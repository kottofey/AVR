#include <avr/io.h>
#include <util/delay.h>
#include "DS18B20.h"
#include "my_uart.h"
#include "Timers.h"
#include "Messages.h"
#include "LCD.h"
#include <avr/pgmspace.h>

#define pin_low() {DS_PORT &= ~(1 << DS_PIN_NUMBER);}	// Пин в ноль
#define pin_high() {DS_PORT |= (1 << DS_PIN_NUMBER);}	// Пин в единицу
#define pin_read() {DS_DDR &= ~(1 << DS_PIN_NUMBER);}	// Пин на чтение
#define pin_write() {DS_DDR |= (1 << DS_PIN_NUMBER);}	// Пин на запись

char AsciiTemp[10]; // Строка с температурой в ASCII формате
char FSM_State, _FSM_State; // Состояние автомата
uint16_t ds_refresh_period = 1*sec;	// Периодичность замеров температуры
uint8_t ds_convert_period = 188; 	// 9bit = 24, 10bit = 47, 11bit = 94, 12bit = 188
uint8_t crc8;

const char dscrc_table[] PROGMEM = {
0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
157,195, 33,127,252,162, 64, 30, 95, 1,227,189, 62, 96,130,220,
35,125,159,193, 66, 28,254,160,225,191, 93, 3,128,222, 60, 98,
190,224, 2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89, 7,
219,133,103, 57,186,228, 6, 88, 25, 71,165,251,120, 38,196,154,
101, 59,217,135, 4, 90,184,230,167,249, 27, 69,198,152,122, 36,
248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91, 5,231,185,
140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
202,148,118, 40,171,245, 23, 73, 8, 86,180,234,105, 55,213,139,
87, 9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

int DS_Reset() {
//	asm("cli");
	pin_write();
	pin_low();
	_delay_us(480);
	pin_high();
	_delay_us(60); //ждем пока оно сообразит
	pin_read();
	char i = (DS_PIN & (1 << DS_PIN_NUMBER));	// проверяем ножку на сигнал присутствия датчика

	   if (i == 0) {	// датчик обнаружен, ножка прижата к нулю
//			UART_TxChar('Y');
		    _delay_us(480);
//		    asm("sei");
		    return 1;
	   }
	   else {	// датчик не обнаружен, ножка осталась подтянута к питанию

//			UART_TxChar('N');
		   _delay_us(480);
//		   asm("sei");
		   return 0;
		}
}

void DS_WriteBit(unsigned int bit){
	if (bit){
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
}

void DS_WriteByte(unsigned int byte){
//	asm("cli");
	for (int i = 0; i < 8; i++){
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
//	asm("sei");
}

char DS_ReadBit(){
//	asm("cli");
	char bit;
	pin_write();
	pin_low();		// Прижимаем к нулю
	_delay_us(3);	// на 3 мкс, начало таймслота
	pin_high();		// Отпускаем шину
	_delay_us(7);	// Ждем до чтения бита (данные действительны в течение 15 мкс после начала таймслота)

	pin_read(); // пин PD6 на чтение
	bit = (DS_PIN & (1 << DS_PIN_NUMBER)) >> DS_PIN_NUMBER;	// Читаем бит. Сдвиг вправо на n бит чтобы получить чистую ноль или единицу
	_delay_us(60);	// Ждем до конца таймслота
//	asm("sei");
	return bit;
}

char DS_ReadByte(){
	char data = 0;
	for (uint8_t i = 0; i < 8; i++){
		data |= DS_ReadBit() << i;
	}
	return data;
}

void DS_MeasureTemp(){
	DS_Reset();
	DS_WriteByte(SKIP_ROM);
	DS_WriteByte(CONVERT_T);	// Запуск измерения температуры
}

uint16_t DS_GetTemp(){
	char ls = 0;	// Less Significant byte из датчика
	char ms = 0;	// Most Significant byte из датчика
	uint16_t DSTemp = 0;	// Собственно температура в int

	DS_Reset(); // Посылаем Reset и ждем сигнал Presence от датчика
	DS_WriteByte(SKIP_ROM);	// Один датчик
	DS_WriteByte(READ_SCRATCHPAD); //передать байты из памяти мастеру (у 18b20 в первых двух содержится температура)

	ls = DS_ReadByte(); //читаем байт LS
	ms = DS_ReadByte(); //читаем байт MS
	DS_Reset();	// Reset ибо кроме первых двух байт температуры нам пока ничего не надо

	DSTemp = ((ms << 8) | ls); //укладываем биты в последовательности MS потом LS

// Конвертируем в int XXXY, где XXX - целая часть, а Y - десятичная часть
	int16_t IntTemp; // Переменная для конвертации температуры в целочисленное значение
	IntTemp = (DSTemp & 0x0FFF); // обнуляем биты знака (с 15 по 12). В итоге получаем 12 бит

// Дальше идет магия... Здесь используется способ преобразования без плавающей запятой.
// То есть, результат будет представлен умноженным на 10. Посленяя цифра - десятичная доля
// Для этого надо разделить значение из датчика на цену младшего разряда. в 12-бит младший разряд представлен 2^(-4) = 0.0625
// То есть, надо умножить на 0.0625 или разделить на 16, что является одним и те же.
// Сдвиг байта вправо равносилен делению на степень двойки равную числу сдвигов
//
// Определяем знак. Если знак минус (старший бит равен единице), то инвертируем байт и добавляем единичку.
// Это есть операция нахождения обратного кода. Она используется для представления отрицательных чисел в двоичном коде
// Байт инвертируется, добавляется единица, а старший-знаковый бит откидывается. В итоге будет модуль отрицательного числа.

		if (IntTemp & 2048){	/*Если температура отрицательная*/
			SendMessage(MSG_TEMP_NEGATIVE);	// Выставляем флаг, что температура орицательна. Чтобы потом вывести знак минуса на экран.
			IntTemp ^= 0xFFF;	// Ксорим 12 бит (инвертируем то бишь)
			IntTemp++;			// Добавляем единичку. Получили модуль значения температуры.
			IntTemp = (IntTemp >> 1) + (IntTemp >> 3);	// Делим на 16

			return IntTemp;
		}
		else {
			IntTemp = (IntTemp >> 1) + (IntTemp >> 3); // Просто и с радостью делим на 16 ибо температура положительна!
			return IntTemp;
		}
}

void DS_GetAsciiTemp(){
	uint16_t int_temp = DS_GetTemp();
	int buf_size = 4;

//	Ниже в комментарии перед циклом вставляем доп. символы в конец строки.
//	На данный момент в строке содержится 5 символов "xхх.х", которые задаются
//	в цикле ниже, где х - температура, целая и десятичная часть, точка вставляется
//	по пути в цикле. После температуры вставляются значок градуса с ASCII
//	кодом '0xB0' и символ перевода строки '\n'. От размера буфера (количества
//	знаков в температуре, включая целую и десятичную часть) вставка символов в
//	конец строки не зависит и все вставляется корректно.
//
//	Дабы не плодить сущности, эта фигня не включена в код. Легче и ЛУЧШЕ после
//	вывода на UART температуры следом вывести отдельно знак переноса строки итд
//	и не забивать переменную лишней белибердой.
//
//	В самом начале цикла идет проверка флага отрицательной температуры. Если флаг выставлен в 1,
//	то вместо первого символа нуля будет знак минуса. Трехзначные отрицательные температуры нам
//	мерять не зачем, так что знак минуса не помешает. Вот как бы только раздобыть отрицательную
//	температуру для проверки... Морозилка?.. Надо припаять датчик к проводу в термоусадки чтоб
//	не засовывать всю макетку в морозилку.
//
//	*(AsciiTemp + (i+3) ) = '\n';
//	*(AsciiTemp + (i+2) ) = 'C';
//	*(AsciiTemp + (i+1) ) = 0xB0;

	for(int i = buf_size; i >= 0; i--){
		if (GetMessage(MSG_TEMP_NEGATIVE)){
			*(AsciiTemp + i) = '-';
			i--;
		}
		if (i==buf_size-1){ // Вставляем десятичную точку перед десятой долей градуса
			*(AsciiTemp+i)='.';
			i--;
		}

		*(AsciiTemp + i) = int_temp%10 | 0x30;
		int_temp = int_temp/10;
	}
}

void DS_ReadROM(){
	char ds_rom[8];
	uint8_t crc_result = 0;

	do {
		DS_Reset();
		DS_WriteByte(READ_ROM);
		for (int i = 7; i >= 0; i--){
			ds_rom[i] = DS_ReadByte();
		}
		crc_result = DS_CheckCRC(ds_rom);
	} while (crc_result);

	for (int i=0; i < 8; i++){
		UART_TxChar(ds_rom[i]);
	}

	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(500);
	return;
}

void DS_Start(){
	_FSM_State = 0;
	return;
}

void DS_Stop(){
	_FSM_State = 99;
	return;
}

///// Функция проверяет CRC переданный в аргументе. Возвращается ноль если CRC совпадает
uint8_t DS_CheckCRC(char *crc_to_check){
	crc8 = 0;
	for (int i = 7; i >= 0; i--){
		crc8 = (char)pgm_read_byte( &(dscrc_table[ crc8 ^ crc_to_check[i] ]) );
	}
	return crc8;
}

//////////////////// Конечный Автомат //////////////////////

void DS_InitFSM(){
	FSM_State = 0;
}

void DS_ProcessFSM(){
	if (GetBroadcastMessage(MSG_MENU_STARTED)){
			_FSM_State = FSM_State;	// Сохраняем состояние автомата во временную переменную
			FSM_State = 0xFF;	// Вход в меню, работа КА приостанавливается
		}
		else if (GetBroadcastMessage(MSG_MENU_EXIT)){
			FSM_State = _FSM_State;	// Выход из меню. Возвращаемся к месту где остановились при входе в меню
		}

	switch (FSM_State){
		case 0:
//			UART_TxString("DS0\n");
			FSM_State = 1;
			StartTimer(TIMER_TEMP_CONVERT);
			break;

		case 1:
//			UART_TxString("DS1\n");
			if (GetTimer(TIMER_TEMP_CONVERT) >= ds_refresh_period ){
				DS_MeasureTemp();
				ResetTimer(TIMER_TEMP_CONVERT);
				FSM_State = 2;
			}
			break;

		case 2:
//			UART_TxString("DS2\n");
			if (GetTimer(TIMER_TEMP_CONVERT) >= ds_convert_period ){	// Через секунду гарантировано можно забирать значение при любой разрядности датчика
				DS_GetAsciiTemp();
				SendMessage(MSG_TEMP_CONVERT_COMPLETED);
				StopTimer(TIMER_TEMP_CONVERT);
				FSM_State = 0;
			}
			break;
		case 99:	// Приостановка измерений
			break;

	}
}
