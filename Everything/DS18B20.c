#include <avr/io.h>
#include <util/delay.h>
#include "DS18B20.h"
#include "my_uart.h"
#include "Timers.h"
#include "Messages.h"

#define pin_low() {DS_PORT &= ~(1 << DS_PIN);}	// Пин в ноль
#define pin_high() {DS_PORT |= (1 << DS_PIN);}	// Пин в единицу
#define pin_read() {DS_DDR &= ~(1 << DS_PIN);}	// Пин на чтение
#define pin_write() {DS_DDR |= (1 << DS_PIN);}	// Пин на запись

char AsciiTemp[10]; // Строка с температурой в ASCII формате
char FSM_State, _FSM_State; // Состояние автомата
uint16_t ds_refresh_period = 0*sec;	// Периодичность замеров температуры
uint8_t ds_convert_period = 188; 	// 9bit = 24, 10bit = 47, 11bit = 94, 12bit = 188

int DS_Reset() {
//	asm("cli");
	pin_write();
	pin_low();
	_delay_us(480);
	pin_high();
	_delay_us(60); //ждем пока оно сообразит
	pin_read();
	char i = (PIND & 0b01000000) >> 6;	// проверяем ножку на сигнал присутствия датчика

	   if (i == 0) {	// датчик обнаружен, ножка прижата к нулю
//			UART_TxChar('Y');
		    _delay_us(480);
//		    asm("sei");
		    return 1;
	   }
	   else {	// датчик не обнаружен, ножка осталась подтянута к питанию

//			UART_TxString("Temperature sensor missing!\n");
		   _delay_us(480);
//		   asm("sei");
		   return 0;
		}
}

void DS_WriteByte(unsigned int byte){
//	asm("cli");
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
	bit = (PIND & 0b01000000) >> 6;	// Читаем бит. Сдвиг вправо на 6 бит чтобы получить чистую ноль или единицу
	_delay_us(60);	// Ждем до конца таймслота
//	asm("sei");
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
	IntTemp = (DSTemp & 4095); // обрезаем биты знака (с 15 по 12). В итоге получаем 12 бит

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

	}
}
