/* MCU: ATMega16A
 * F_CPU 8000000L
 *
 * Фьюзы:
 * 	High:	0xd7
 * 	Low:	0xc4
 *
 *
 * В этом файле содержатся следующие функции:
 *
 * 	UART функции:
 * 	- UART_Init				UART инициализация;
 *  - UART_TxChar			UART передача символа;
 *  - UART_TxString			UART передача строки (строку передавать аргументом в функцию);
 *
 *  1wire DS18b20 температурный датчик:
 *  - ds_init				Инициализация\ресет датчика;
 *  - ds_write_byte			Передача команды в датчик;
 *  - ds_read_bit			Функция считывания бита из датчика;
 *  - ds_read_byte			Функция считывания байта из датчика (использует функцию чтения бита в цикле);
 *  - ds_temp				Функция определения и конвертации температуры;
 *  - ds_convert			Функция ковертации температуры в переменную int;
 *  - ds_temp_to_ascii		Функция перевода переменной int в массив char *;
 *  - ds_GetAsciiTemp		Функция занесения ascii значения температуры в глобальный массив temp[10];
 *
 *  Подключение датчика DS18b20:
 *   PD6 - пин данных (подтянут к питанию резистором 4.7кОм)
 *
 *	LCD дисплей
 *	- LCD_Init				Инициализация дисплея
 *	- LCD_WriteCmd			Функция ввода комманды
 *	- LCD_Write_Data		Функция ввода символа в CGRAM или вывода на экран из DDRAM
 *	- LCD_WriteString		Функция ввода строки char
 *	- LCD_GotoXY			Функция перехода на "x" строку (0я строка или 1я строка)
 *							в позицию "y" символо-места (0-15)
 *	- LCD_MakeSymbol		Первым аргументом идет номер ячейки памяти для записи собственного
 *							символа (от 0 до 7), далее идут восемь байт собственно символа,
 *							начиная с верхнего, заканчивая нижним. Первые три бита в байте символа
 *							не имеют значения и могут быть любыми.
 *
 * Подключение LCD:
 * 	PA1 - A0 (Адресный сигнал - выбор между передачей данных и команд управления)
 * 	PA2 - R/W (Выбор режима записи или чтения)
 * 	PA0 - E (Разрешение обращений к индикатору (а также строб данных))
 * 	PORTC[0..7] - DB[0..7] (Шина данных)
 */

/*
 * Todo
 * 	- Сделать настройки для подключения периферии дефайнами, для универсальности
 * 	- Инициализация UART через параметры функции
 * 	- дополнить и изучить функциональность темп. датчика
 * V- сделать прием в UART (!!!)
 * 	- задефайнить настройки LCD, сделать их словами, а не хексом
 * 	- сделать дрыги не дефайном, а функцией. По-моему, так будет лучше...
 * V- Попробовать залить символ в LCD массивом, может будет компактнее в памяти
 * 	- Перевести LCD в 4х-битный режим. ДАЕШЬ МЕНЬШЕ ПРОВОДОВ!!!
 * 	- Повесить вывод температуры на кнопку через внешнее прерывание
 * 	- Сделать софтовый анти-дребезг кнопок (счетчик дребезжания?..)
 * 	- Дополнить отсутствующие комментарии, а то запутаюсь!
 * 	- Залить это дело в GitHub? Опять же, запутаюсь. Да и бэкапы не помешают.
 */



// ИЗМЕНИТЬ ЧАСТОТУ ПРИ ИЗМЕНЕНИИ ЧАСТОТЫ ВО ФЬЮЗАХ!!!!!
#define F_CPU 8000000L	// В самом начале. Шоб ничего не ругалось...

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>


// дефайны для настройки UART

#define BAUD 9600	// Скорость передатчика UART
#define MYUBRR F_CPU/16/BAUD-1 // Вычисление бита UBRR

// дефайны для комманд датчика температуры и его подключение

#define SKIP_ROM 0xCC // Пропустить индентификацию
#define CONVERT_T 0x44 // Измерить температуру
#define READ_SCRATCHPAD 0xBE // Прочитать измеренное

#define DS_PORT PORTD
#define DS_PIN 6
#define DS_DDR DDRD

#define pin_low() {DS_PORT &= ~(1 << DS_PIN);}
#define pin_high() {DS_PORT |= (1 << DS_PIN);}
#define pin_read() {DS_DDR &= ~(1 << DS_PIN);}
#define pin_write() {DS_DDR |= (1 << DS_PIN);}
////////////////////////////////////////////////////////////



void UART_Init(unsigned int ubrr);
void UART_TxChar(char data);
void UART_TxString(char * data);

int DS_Init();
void DS_WriteByte(unsigned int byte);
char DS_ReadBit();
char DS_ReadByte();
unsigned int DS_Temp();
unsigned int DS_Convert(unsigned int td);
void DS_GetAsciiTemp();
char temp[10]="";	// Глобальная переменная для температуры в ASCII формате

void LCD_init();
void LCD_WriteCmd(char b);
void LCD_WriteData(char b);
void LCD_WriteByte(char b, char cd);
void LCD_WriteString(char *data);
void LCD_GotoXY(char stroka, char simvol);
void LCD_MakeSymbol(char addr, char * a0); // Символ задается массивом. Программа компактнее, но отжирает больше памяти flash


/*	здесь прерывание по опустошению буфера уарт
#define BUF_SIZE 17
char buffer[BUF_SIZE] = "0123456789ABCDEF\n";
uint8_t buffer_index=0;


ISR (USART_UDRE_vect){
	buffer_index++;

	if (buffer_index == BUF_SIZE){
		UCSRB &= ~(1<<UDRIE);
	}
	else{
		UDR = buffer[buffer_index];
	}
//	UCSRB &= ~(1<<UDRIE);
}
*/


//	Прерывание USART прием байта завершен
ISR (USART_RXC_vect){
	char data=UDR;
	UART_TxChar(data);
//	LCD_WriteData(data);

//	Реакция на определенный символ отправленный в USART
	if (data == 't'){
		UART_TxString("\nt=");
		UART_TxString(temp);
		UART_TxChar(0xB0);
		UART_TxString("C\n");
	}
	else{
		UART_TxString("\nHaha!\n");
	}
}


int main(void){

		UART_Init(MYUBRR);
		LCD_init();

		DS_Init();
		DS_WriteByte(SKIP_ROM);
		DS_WriteByte(0x4E);
		DS_WriteByte(0x00);
		DS_WriteByte(0x00);
		DS_WriteByte(0b01111111);
		DS_Init();

		char ds_address[7]="";

		DS_WriteByte(0x33);
		for(int i=0; i<8; i++){
			ds_address[i] = DS_ReadByte();
		}
		DS_Init();

		for(int ii=0; ii<8; ii++){
			UART_TxChar(ds_address[ii]);

		}


		sei();	// Разрешаем прерывания


		while(1){

//		UART_TxString("t=");
//		UART_TxString(temp);
//		UART_TxChar(0xB0);
//		UART_TxString("C\n");

		DS_GetAsciiTemp();

		LCD_WriteCmd(0x01);
		LCD_WriteString("Temp=");
		LCD_WriteString(temp);
		LCD_WriteData(0xB0);
		LCD_WriteData('C');
	}
	return 0;
}



//	Функции для UART

void UART_Init(unsigned int ubrr){

	/* Set baud rate */
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;

	/* Enable receiver and transmitter */
	UCSRB = 1<<RXEN | 1<<TXEN | 1<<RXCIE | 0<<TXCIE | 0<<UDRIE;

	/* Set frame format: 1 stop bit, 8data */
	UCSRC = (1<<URSEL)|(0<<USBS)|(3<<UCSZ0);

}

void UART_TxChar(char data) {	// Передача из МК в провод

/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) );

/* Put data into buffer, sends the data */
	UDR = (unsigned int) data;

}

void UART_TxString(char * data){

	int i = 0;
	while (data[i] != 0){
			UART_TxChar(data[i]);
			i++;
			}
			i=0;
}

//	Функции для DS18b20

int DS_Init() {
	pin_write();

	pin_low();
	_delay_us(480);
	pin_high();
	_delay_us(60); //ждем пока он сообразит

	pin_read();
	char i = (PIND & 0b01000000) >> 6;	// проверяем ножку на сигнал присутствия датчика


	   if (i == 0) {	// датчик обнаружен, ножка прижата к нулю
//			UART_Tx_Char('Y');
		    _delay_us(480);
			return 1;
	   }
	   else {	// датчик не обнаружен, ножка осталась подтянута к питанию

			UART_TxString("Temperature sensor missing!\n");
		   _delay_us(480);
			return 0;
		}
}

void DS_WriteByte(unsigned int byte){
	int i;

	for (i=0; i<8; i++){
		if (byte & 0b00000001){
			pin_write();
			pin_low();
			_delay_us(15);
			pin_high();
			_delay_us(45);	// Ждем окончания таймслота
			_delay_us(3);	// Восстановление между таймслотами не менее 1 мкс
		}
		else	{
			pin_write();
			pin_low();
			_delay_us(100);
			pin_high();
			_delay_us(3);	// Восстановление между таймслотами
		}
		byte >>= 1;
	}

}

char DS_ReadBit(){
	char bit;

	pin_write();
	pin_low();		// Прижимаем к нулю
	_delay_us(3);	// на 3 мкс, начало таймслота
	pin_high();		// Отпускаем шину
	_delay_us(7);	// Ждем до чтения бита (данные действительны в течение 15 мкс после начала таймслота)

	pin_read(); // пин PD6 на чтение
	bit = (PIND & 0b01000000) >> 6;	// Читаем бит. Сдвиг вправо на 6 бит чтобы получить чистую ноль или единицу
//	UART_Tx_Char(PIND & 0b01000000);
	_delay_us(60);	// Ждем до конца таймслота

	return bit;
}

char DS_ReadByte(){
	char data = 0;
	int i;
	for (i=0; i < 8; i++)
		{
		data |= DS_ReadBit() << i;
	}
	return data;
}

unsigned int DS_Temp(){
	char ls = 0;
	char ms = 0;
	unsigned int ds18_temp = 0;


	DS_Init();
	DS_WriteByte(SKIP_ROM);
	DS_WriteByte(CONVERT_T);
	_delay_ms(750); //преобразование в 12 битном режиме занимает 750ms

	DS_Init(); //снова посылаем Reset и Presence
	DS_WriteByte(SKIP_ROM);
	DS_WriteByte(READ_SCRATCHPAD); //передать байты мастеру (у 18b20 в первых двух содержится температура)

	ls = DS_ReadByte(); //читаем байт LS
	ms = DS_ReadByte(); //читаем байт MS

	DS_Init();	// Reset ибо больше двух байт температуры нам ничего не надо

//	UART_Tx_Char('@');

	ds18_temp = ((ms << 8) | ls); //укладываем биты в последовательности MS потом LS
//	UART_Tx_Char(ms); UART_Tx_Char(ls);
	return ds18_temp; //возвращаем int (MS,LS)
}

unsigned int DS_Convert(unsigned int td){
	unsigned int a, b, dat;
	dat = (td & 4095); // обрезаем биты знака (с 15 по 12). В итоге получаем 12 бит

	if (dat & 2048){	/*Если температура отрицательная*/
		a = dat >> 1;
		b = dat >> 3;
		dat = a + b;
		dat *= -1;
		return dat;
	}

	else {
	a = dat >> 1;
	b = dat >> 3;
	dat = a + b;
	return dat;
	}
}

void DS_GetAsciiTemp()
{
	int int_temp = DS_Convert(DS_Temp());

	int buf_size = 4;

//	Ниже в комментарии перед циклом вставляем доп. символы в конец строки.
//	На данный момент в строке содержится 5 символов "xхх.х", которые задаются
//	в цикле ниже, где х - температура, целая и десятичная часть, точка вставляется
//	по пути в цикле. После температуры вставляются значок градуса с ASCII
//	кодом '0xB0' и символ перевода строки '\n'. От размера буфера (количества
//	знаков в температуре, включая целую и десятичную часть) вставка символов в
//	конец строки не зависит и вставляется корректно.
//
//	Дабы не пложить сущности, эта фигня не включена в код. Легче и ЛУЧШЕ после
//	вывода на UART температуры следом вывести отдельно знак переноса строки итд
//	и не забивать переменную лишней белибердой.

//	*(temp + (i+3) ) = '\n';
//	*(temp + (i+2) ) = 'C';
//	*(temp + (i+1) ) = 0xb0;

	for(int i = buf_size; i >= 0; i--){

		if (i==buf_size-1){*(temp+i)='.'; i--;}

		*(temp + i) = int_temp%10 | 0x30;
		int_temp = int_temp/10;
	}

}


//	Функции для LCD

 void LCD_init() {
	DDRC = 0xff; // PORTC-шина данных на вывод
	DDRA = 0xff; // пины 5, 4, 3 и, на всякий случай, 6 порта D на вывод

	PORTA = 0x00;
	_delay_ms(20); 			// 	>20ms
//	init
	LCD_WriteCmd(0x30);
	LCD_WriteCmd(0x30);
	LCD_WriteCmd(0x30);

//	setup
	LCD_WriteCmd(0b00111010);	// Function Set (4[0]/8[1]bit, 1я страница)
	LCD_WriteCmd(0b00001111);	// Disp On/Off Control (вкл[1]/выкл[0] экрана, режим курсора[11])
	LCD_WriteCmd(0b00000110);	// Entry Mode Set (курсор влево/вправо | разрешение сдвига экрана)
//	LCD_WriteCmd(0b00000001);	// Clear Display

//	debug
	LCD_WriteCmd(0b00000010);	// Return Home (курсор в начало)
	LCD_WriteString("LCD Init OK");
	_delay_ms(200);
	LCD_WriteCmd(0x01);
}

void LCD_WriteCmd(char b) { LCD_WriteByte(b,0); }

void LCD_WriteData(char b) { LCD_WriteByte(b,1); }

void LCD_WriteByte(char b, char cd) {
	DDRC = 0b00000000; // PORTC-шина_данных на вход
	DDRA = 0b00000111;

// Чтение флага занятости
	PORTA &= ~(1 << PINA1); //	A0 = 0
	PORTA |=  (1 << PINA2); // 	RW = 1

	_delay_us(1); 			// 	>40ns
	PORTA |= (1 << PINA0); 	//	E = 1
	_delay_us(1);			//	>230ns
	while(PINC >= 0x80);	//	Ждать сброса флага занятости на пине PINC7

	PORTA &= ~(1 << PINA0);	//	E = 0
	DDRC = 0xFF;	// шина данных опять на вывод
	PORTA &= ~(1 << PINA2); //	RW = 0

	if (cd == 1) {PORTA |= (1 << PINA1);}	// A0 = cd (1)
	else 		 {PORTA &=~(1 << PINA1);}	// A0 = cd (0)

	PORTC = b;			 //Выдача байта на шину данных
	PORTA |= _BV(0); _delay_us(1); PORTA &= ~_BV(0); _delay_us(45);	// Строб
}

void LCD_WriteString(char *data){
	int i = 0;
	while (data[i] != 0){
		LCD_WriteData(data[i]);
		i++;
	}
	i=0;
}

void LCD_GotoXY(char stroka, char simvol){

	char result=0;

	if (!stroka) 	result = simvol + 0x80 - 0x01; // 0я строка
	else			result = simvol + 0xC0 - 0x01; // 1я строка

	LCD_WriteCmd(result);
}


void LCD_MakeSymbol(char addr, char * a0){ //, char a1, char a2, char a3, char a4, char a5, char a6, char a7){
	//Значок молнии
		if (!addr) LCD_WriteCmd(addr+0x40);
		LCD_WriteCmd(addr*0x08+0x40);	// Выбираем адрес символа в CGRAM

		for (int i=0; i<8; i++)
			{
			LCD_WriteData(a0[i]);
		}
}


/*
void LCD_MakeSymbol(char addr, char a0, char a1, char a2, char a3, char a4, char a5, char a6, char a7){
	//Значок молнии
		if (!addr) LCD_WriteCmd(addr+0x40);
		LCD_WriteCmd(addr*0x08+0x40);	// Выбираем адрес символа в CGRAM

		// Пишем 8 байт символа в CGRAM
		// Первые три бита незначащие и не повлияют на результат
		// Размер символа 5х8 точек

		LCD_WriteData(a0);	// Первый "верхний" байт
		LCD_WriteData(a1);
		LCD_WriteData(a2);
		LCD_WriteData(a3);
		LCD_WriteData(a4);
		LCD_WriteData(a5);
		LCD_WriteData(a6);
		LCD_WriteData(a7);	// последний "нижний" байт
}
*/
