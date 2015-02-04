#include <avr/io.h>
#include <util/delay.h>
#include "LCD.h"
#include "DS18B20.h"
#include "Messages.h"
#include "Timers.h"
#include "my_uart.h"
#include "keyboard.h"
#include "menu.h"
#include <avr/pgmspace.h>

/////////Настройки подключения LCD/////////
#define LCD_4bitMode // Раскомментировать эту строку для 4-битного режима
uint8_t LCD_DataMask = 0b01111000;	// Единичками отметить пины [7..4] шины данных LCD
uint16_t LCD_BacklightTimeout = 5*sec;
///////////////////////////////////////////

uint8_t LCD_Pin7;	// Седьмой пин шины данных. Вычисляется автоматически.
uint8_t FSM_Staate; // Переменная состояния КА
char AsciiTemp[10];

void LCD_init() {
/////Инициализация пина подсветки LCD/////////
	LCD_BACKLIGHT_DDR |= (1 << LCD_BACKLIGHT_PIN);
	LCD_BACKLIGHT_PORT &= ~(1 << LCD_BACKLIGHT_PIN); // Выключено при запуске по умолчанию

/////Вычисление пина №7 шины данных///////////
	uint8_t shifted_mask = LCD_DataMask;
	LCD_Pin7 = 3;
	while (shifted_mask != 0b00001111){
		shifted_mask >>= 1;
		LCD_Pin7++;
	}
//////////////////////////////////////////////
#if defined (LCD_4bitMode)
// 4-битный режим
	LCD_DATA_DDR = (LCD_DATA_DDR & ~LCD_DataMask) | LCD_DataMask; // PORTC-шина данных на вывод (единички в ddr)
	LCD_SIGNAL_DDR |= (1 << LCD_A0_PIN) | (1 << LCD_E_PIN) | (1 << LCD_RW_PIN); // пины 0, 1, 2 порта A на вывод

	_delay_ms(100); 			// 	>20ms
//	init
	LCD_SIGNAL_PORT &= ~(1 << LCD_A0_PIN);  // A0 = 0
	LCD_SIGNAL_PORT &= ~(1 << LCD_RW_PIN);  // RW = 0
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);	// E = 0

	LCD_DATA_PORT = 0x30;			 //Выдача байта на шину данных [1]
	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);
	_delay_us(1);
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);
	_delay_ms(10);	// Строб

	LCD_DATA_PORT = 0x30;			 //Выдача байта на шину данных [2]
	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);
	_delay_us(1);
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);
	_delay_us(200);	// Строб

	LCD_DATA_PORT = 0x30;			 //Выдача байта на шину данных [3]
	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);
	_delay_us(1);
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);
	_delay_us(80);	// Строб

	LCD_DATA_PORT = 0x20;			 //Выдача байта на шину данных [4]
	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);
	_delay_us(1);
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);
	_delay_us(100);	// Строб

//	setup
	LCD_WriteCmd(0b00101010);	// Function Set (4bit, 1я страница)
	LCD_WriteCmd(0b00001111);	// Disp On/Off Control (вкл[1]/выкл[0] экрана, режим курсора[11])
	LCD_WriteCmd(0b00000110);	// Entry Mode Set (курсор влево/вправо | разрешение сдвига экрана)
	LCD_WriteCmd(0b00000001);	// Clear Display

//	debug
	LCD_WriteCmd(0b00000010);	// Return Home (курсор в начало)
	LCD_WriteStringFlash(PSTR("4-битный режим"));	// инлайн строчка берется из флеша
	_delay_ms(500);
	LCD_WriteCmd(LCD_CLEAR_SCREEN);

#else
// 8-битный режим
	LCD_DATA_DDR = 0xff; // PORTC-шина данных на вывод (единички в ddr)
	LCD_SIGNAL_DDR |= (1 << LCD_A0_PIN) | (1 << LCD_E_PIN) | (1 << LCD_RW_PIN); // пины 0, 1, 2 порта A на вывод
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);	// E = 0

		_delay_ms(20); 			// 	>20ms
	//	init
		LCD_SIGNAL_PORT &= ~(1 << LCD_A0_PIN); //	A0 = 0
		LCD_SIGNAL_PORT &= ~(1 << LCD_RW_PIN); // 	RW = 0

		LCD_DATA_PORT = 0x30;			 //Выдача байта на шину данных [1]
		LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);
		_delay_us(1);
		LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);
		_delay_us(45);	// Строб

		LCD_DATA_PORT = 0x30;			 //Выдача байта на шину данных [2]
		LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);
		_delay_us(1);
		LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);
		_delay_us(45);	// Строб

		LCD_DATA_PORT = 0x30;			 //Выдача байта на шину данных [3]
		LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);
		_delay_us(1);
		LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);
		_delay_us(45);	// Строб

	//	setup
		LCD_WriteCmd(0b00111010);	// Function Set (8bit, 1я страница)
		LCD_WriteCmd(0b00001110);	// Disp On/Off Control (вкл[1]/выкл[0] экрана, режим курсора[10])
		LCD_WriteCmd(0b00000110);	// Entry Mode Set (курсор влево/вправо | разрешение сдвига экрана)
		LCD_WriteCmd(0b00000001);	// Clear Display

	//	debug
		LCD_WriteCmd(0b00000010);	// Return Home (курсор в начало)
		LCD_WriteStringFlash(PSTR("8-битный режим"));	// инлайн строчка берется из флеша
		_delay_ms(200);
		LCD_WriteCmd(LCD_CLEAR_SCREEN);
#endif
}

void LCD_WriteCmd(unsigned char b) {
	LCD_WriteByte(b, 0);
}

void LCD_WriteData(unsigned char b) {
	LCD_WriteByte(b, 1);
}

void LCD_WriteByte(unsigned char b, unsigned char cd) {
#if defined(LCD_4bitMode)
// 4-битный режим
	LCD_DATA_DDR = (LCD_DATA_DDR | LCD_DataMask) ^ LCD_DataMask; // шина_данных на вход (ноли по маске не трогая остальные биты)
	LCD_SIGNAL_DDR |= (1 << LCD_A0_PIN) | (1 << LCD_E_PIN) | (1 << LCD_RW_PIN); // пины A0, E, RW на вывод

// Чтение флага занятости
	LCD_SIGNAL_PORT &= ~(1 << LCD_A0_PIN); //	A0 = 0
	LCD_SIGNAL_PORT |= (1 << LCD_RW_PIN); // 	RW = 1

	_delay_us(1); 			// 	>40ns
	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN); 	//	E = 1
	_delay_us(1);			//	>230ns
	while ( ((LCD_DATA_PIN & LCD_DataMask) >> LCD_Pin7) );	//	Ждать сброса флага занятости на пине 7 LCD

	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);	//	E = 0
	LCD_DATA_DDR = (LCD_DATA_DDR | LCD_DataMask);	// шина данных опять на вывод (единички по маске не трогая остальные биты)
	LCD_SIGNAL_PORT &= ~(1 << LCD_RW_PIN); //	RW = 0

	if (cd == 1) {
		LCD_SIGNAL_PORT |= (1 << LCD_A0_PIN);
	}	// A0 = cd (1)
	else {
		LCD_SIGNAL_PORT &= ~(1 << LCD_A0_PIN);
	}	// A0 = cd (0)

	uint8_t port_backup = LCD_DATA_PORT;
	uint8_t nibble = ( (b >> 4) << (LCD_Pin7 - 3)); // старший ниббл в нужной позиции
	port_backup = (port_backup & ~LCD_DataMask) | nibble; // обнуляем шину данных инвертированной маской, а потом вставляем ниббл
	LCD_DATA_PORT = port_backup;

	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);
	_delay_us(1);
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);
	_delay_us(45);	// Строб

	nibble = ( (b & 0x0F) << (LCD_Pin7 - 3)); // младший ниббл в нужной позиции
	port_backup = (port_backup & ~LCD_DataMask) | nibble; // обнуляем шину данных инвертированной маской, а потом вставляем ниббл
	LCD_DATA_PORT = port_backup;

	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);
	_delay_us(1);
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);
	_delay_us(45);	// Строб

#else
// 8-битный режим
	LCD_DATA_DDR = 0b00000000; // PORTC-шина_данных на вход
	LCD_SIGNAL_DDR |= (1 << LCD_A0_PIN) | (1 << LCD_E_PIN) | (1 << LCD_RW_PIN); // пины 0, 1, 2 порта A на вывод

// Чтение флага занятости
	LCD_SIGNAL_PORT &= ~(1 << LCD_A0_PIN); //	A0 = 0
	LCD_SIGNAL_PORT |= (1 << LCD_RW_PIN); // 	RW = 1

	_delay_us(1); 			// 	>40ns
	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN); 	//	E = 1
	_delay_us(1);			//	>230ns
	while (LCD_DATA_PIN >= 0x80);	//	Ждать сброса флага занятости на пине 7 LCD

	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);	//	E = 0
	LCD_DATA_DDR = 0xFF;	// шина данных опять на вывод
	LCD_SIGNAL_PORT &= ~(1 << LCD_RW_PIN); //	RW = 0

	if (cd == 1) {
		LCD_SIGNAL_PORT |= (1 << LCD_A0_PIN);
	}	// A0 = cd (1)
	else {
		LCD_SIGNAL_PORT &= ~(1 << LCD_A0_PIN);
	}	// A0 = cd (0)

	LCD_DATA_PORT = b;			 //Выдача байта на шину данных
	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);
	_delay_us(1);
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);
	_delay_us(45);	// Строб
#endif
}

void LCD_WriteString(char *data) {
	while (*data) {
		if (*data >= 0xd0){	// Проврка на кириллицу в кодировке UTF-8
			if (*data == 0xd1) {	// проверка промежутка "п-я". В UTF8 он оторван и начинается с 0xD180, что соответствует строчной букве "п"
				data++;
				LCD_WriteData(*data + 0x70);
				data++;
			}
			else { // промежуток "А-Я" и "а-о"
				data++;
				LCD_WriteData(*data + 0x30);
				data++;
			}
		}
		else { // для латиницы
			LCD_WriteData(*data);
			data++;
		}
	}
}

void LCD_WriteStringFlash(const unsigned char *data) {
	while (pgm_read_byte(data)) {
		if (pgm_read_byte(data) >= 0xd0) {	// Проврка на кириллицу в кодировке UTF-8
			if (pgm_read_byte(data) == 0xd1){	// проверка промежутка "п-я". В UTF8 он оторван и начинается с 0xD180, что соответствует строчной букве "п"
				data++;
				LCD_WriteData(pgm_read_byte(data) + 0x70);
				data++;
			}
			else {// промежуток "А-Я" и "а-о"
				data++;
				LCD_WriteData(pgm_read_byte(data) + 0x30);
				data++;
			}
		}
		else { // Для латиницы
			LCD_WriteData(pgm_read_byte(data));
			data++;
		}
	}
}

uint8_t LCD_ReadCursor(){
	uint8_t byte = 0, hNibble = 0, lNibble = 0;
	LCD_SIGNAL_PORT |= (1 << LCD_A0_PIN); //	A0 = 1
	LCD_SIGNAL_PORT |= (1 << LCD_RW_PIN); // 	RW = 1
	LCD_DATA_DDR = (LCD_DATA_DDR | LCD_DataMask); // шина_данных на выход (единицы по маске не трогая остальные биты)
	_delay_us(1); 	// Время предустановки адреса >40ns

#if defined (LCD_4bitMode)
// 4-битный режим
	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);	// E = 1
	_delay_us(1);
	hNibble = ( (LCD_DATA_PIN & LCD_DataMask) >> (LCD_Pin7 - 3) );	// Записываем старший ниббл
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);	// E = 0
	_delay_us(45);	// Строб

	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);	// E = 1
	_delay_us(1);
	lNibble = ( (LCD_DATA_PIN & LCD_DataMask) >> (LCD_Pin7 - 3) );	// Записываем младший ниббл
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);	// E = 0
	_delay_us(45);	// Строб

	byte = ( (hNibble << 4) | lNibble); // Укладываем нибблы в байт, сначала старший, затем младший

	UART_TxChar((unsigned char)hNibble);
	UART_TxChar((unsigned char)lNibble);
	UART_TxChar((unsigned char)byte);

#else
// 8-битный режим
	LCD_SIGNAL_PORT |= (1 << LCD_E_PIN);	// E = 1
	_delay_us(1);
	byte = LCD_DATA_PIN;	// Записываем байт
	LCD_SIGNAL_PORT &= ~(1 << LCD_E_PIN);	// E = 0
	_delay_us(45);	// Строб
#endif

	return byte;
}

void LCD_GotoXY(unsigned char stroka, unsigned char simvol) {

	unsigned char result = 0;

	if (!stroka)
		result = simvol + 0x80; // 0я строка
	else
		result = simvol + 0xC0; // 1я строка

	LCD_WriteCmd(result);
}

void LCD_MakeSymbol(unsigned char addr, unsigned char * a0) {
	if (!addr)
	LCD_WriteCmd(addr + 0x40);
	LCD_WriteCmd(addr * 0x08 + 0x40);	// Выбираем адрес символа в CGRAM

	for (int i = 0; i < 8; i++) {
		LCD_WriteData(a0[i]);
	}
}

void LCD_ShowTemp(){
	LCD_WriteCmd(0x01);
	LCD_WriteStringFlash(PSTR("Темп=     "));
	LCD_WriteData(0xB0);
	LCD_WriteStringFlash(PSTR("C"));

	LCD_GotoXY(0,5);
	LCD_WriteString(AsciiTemp);
}

void LCD_InitFSM(){
	FSM_Staate = 0;
	StartTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);
}

void LCD_ProcessFSM(){
	if (GetBroadcastMessage(MSG_MENU_EXIT)){
		FSM_Staate = 0;
			LCD_GotoXY(1,15);
			LCD_WriteData(0x17); _delay_ms(200);
	}

	switch (FSM_Staate){
		case 0:
			// Проверка таймаута подсветки. Выключаем таймер и подсветку, если таймаут.
			if (GetTimer(TIMER_LCD_BACKLIGHT_TIMEOUT) >= LCD_BacklightTimeout){
				LCD_BACKLIGHT_PORT &= ~(1 << LCD_BACKLIGHT_PIN);
				StopTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);
			}

			if (GetMessage(MSG_TEMP_CONVERT_COMPLETED)){
				LCD_ShowTemp();
			}
			if (GetMessage(MSG_KEYB_KEY_PRESSED)){
				// По нажатию клавиши заново стартует таймер подсветки и она включается на время LCD_BACKLIGHT_TIMEOUT (указано в настройках)
				StartTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);
				ResetTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);
				LCD_BACKLIGHT_PORT |= (1 << LCD_BACKLIGHT_PIN);

				switch (Keyb_GetScancode()){
					case KEY_1: LCD_ReadCursor(); break;
					case KEY_2: LCD_WriteCmd(LCD_CURSOR_MOVE_LEFT); break;
					case KEY_3: LCD_WriteCmd(LCD_CURSOR_MOVE_RIGHT); break;
					case KEY_4: FSM_Staate=10; SendBroadcastMessage(MSG_MENU_STARTED); break;	// Вход в меню
					case KEY_1_2: LCD_WriteData('5'); break;
					case KEY_1_3: LCD_WriteData('6'); break;
//					case KEY_1_4: FSM_Staate = 20; break;	// DEBUG
					case KEY_2_3: LCD_WriteData('8'); break;
					case KEY_2_4: LCD_WriteData('9'); break;
					case KEY_3_4: LCD_WriteData('A'); break;
					case KEY_1_2_3: LCD_WriteData('B'); break;
				}
			}
			break;

		case 10:	// Входим в меню
			// Проверка таймаута подсветки. Выключаем ее, если таймаут.
			if (GetTimer(TIMER_LCD_BACKLIGHT_TIMEOUT) >= LCD_BacklightTimeout){
				LCD_BACKLIGHT_PORT &= ~(1 << LCD_BACKLIGHT_PIN);
				StopTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);
			}

			LCD_WriteCmd(LCD_CLEAR_SCREEN);
			LCD_WriteStringFlash(PSTR("MENU:"));
			LCD_GotoXY(1,0);
			SET_MENU(x1);
			FSM_Staate = 11;
			break;

		case 11:
			// Проверка таймаута подсветки. Выключаем ее, если таймаут.
			if (GetTimer(TIMER_LCD_BACKLIGHT_TIMEOUT) >= LCD_BacklightTimeout){
				LCD_BACKLIGHT_PORT &= ~(1 << LCD_BACKLIGHT_PIN);
				StopTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);
			}

			if (GetMessage(MSG_KEYB_KEY_PRESSED)){
// По нажатию клавиши заново стартует таймер подсветки и она включается на время LCD_BACKLIGHT_TIMEOUT (указано в настройках)
				StartTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);
				ResetTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);
				LCD_BACKLIGHT_PORT |= (1 << LCD_BACKLIGHT_PIN);

				LCD_WriteCmd(LCD_CLEAR_SCREEN);
				LCD_WriteStringFlash(PSTR("MENU:"));
				LCD_GotoXY(1,0);
				switch (Keyb_GetScancode()){
					case KEY_1: SET_MENU(PARENT); break;
					case KEY_2: SET_MENU(PREVIOUS); break;
					case KEY_3: SET_MENU(NEXT); break;
					case KEY_4: SET_MENU(SIBLING); break;
					case KEY_1_4: GO_MENU_FUNC(SELECTFUNC); break; // Если выбран пункт "EXIT", то восстанавливаем работу автомата
				}
			}
			break;
		default:
			LCD_WriteCmd(LCD_CLEAR_SCREEN);
			LCD_WriteStringFlash(PSTR("There was error!"));
			break;
	}
}
