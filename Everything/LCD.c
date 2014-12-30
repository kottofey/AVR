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

uint8_t FSM_Staate; // Переменная состояния КА
char AsciiTemp[10];

void LCD_init() {
	DDRC = 0xff; // PORTC-шина данных на вывод
	DDRA |= (1 << 0) | (1 << 1) | (1 << 2); // пины 0, 1, 2 порта A на вывод

	_delay_ms(20); 			// 	>20ms
//	init
	LCD_WriteCmd(0x30);
	LCD_WriteCmd(0x30);
	LCD_WriteCmd(0x30);

//	setup
	LCD_WriteCmd(0b00111010);	// Function Set (4[0]/8[1]bit, 1я страница)
	LCD_WriteCmd(0b00001110);	// Disp On/Off Control (вкл[1]/выкл[0] экрана, режим курсора[10])
	LCD_WriteCmd(0b00000110);	// Entry Mode Set (курсор влево/вправо | разрешение сдвига экрана)
//	LCD_WriteCmd(0b00000001);	// Clear Display

//	debug
	LCD_WriteCmd(0b00000010);	// Return Home (курсор в начало)
	LCD_WriteStringFlash(PSTR("LCD Init OK"));	// инлайн строчка берется из флеша
	_delay_ms(200);
	LCD_WriteCmd(CLEAR_SCREEN);
}

void LCD_WriteCmd(char b) {
	LCD_WriteByte(b, 0);
}

void LCD_WriteData(char b) {
	LCD_WriteByte(b, 1);
}

void LCD_WriteByte(char b, char cd) {
	DDRC = 0b00000000; // PORTC-шина_данных на вход
	DDRA |= (1 << 0) | (1 << 1) | (1 << 2); // пины 0, 1, 2 порта A на вывод

// Чтение флага занятости
	PORTA &= ~(1 << PINA1); //	A0 = 0
	PORTA |= (1 << PINA2); // 	RW = 1

	_delay_us(1); 			// 	>40ns
	PORTA |= (1 << PINA0); 	//	E = 1
	_delay_us(1);			//	>230ns
	while (PINC >= 0x80);	//	Ждать сброса флага занятости на пине PINC7

	PORTA &= ~(1 << PINA0);	//	E = 0
	DDRC = 0xFF;	// шина данных опять на вывод
	PORTA &= ~(1 << PINA2); //	RW = 0

	if (cd == 1) {
		PORTA |= (1 << PINA1);
	}	// A0 = cd (1)
	else {
		PORTA &= ~(1 << PINA1);
	}	// A0 = cd (0)

	PORTC = b;			 //Выдача байта на шину данных
	PORTA |= (1 << 0);
	_delay_us(1);
	PORTA &= ~(1 << 0);
	_delay_us(45);	// Строб
}

void LCD_WriteString(char *data) {
	while (*data) {
		LCD_WriteData(*data);
		data++;
	}
}

void LCD_WriteStringFlash(const char *data) {
	while (pgm_read_byte(data)) {
		LCD_WriteData(pgm_read_byte(data));
		data++;
	}
}

void LCD_GotoXY(char stroka, char simvol) {

	char result = 0;

	if (!stroka)
		result = simvol + 0x80; // 0я строка
	else
		result = simvol + 0xC0; // 1я строка

	LCD_WriteCmd(result);
}

void LCD_MakeSymbol(char addr, char * a0) {
	if (!addr)
	LCD_WriteCmd(addr + 0x40);
	LCD_WriteCmd(addr * 0x08 + 0x40);	// Выбираем адрес символа в CGRAM

	for (int i = 0; i < 8; i++) {
		LCD_WriteData(a0[i]);
	}
}

void LCD_ShowTemp(){
	LCD_WriteCmd(0x01);
	LCD_WriteStringFlash(PSTR("Temp=     "));
	LCD_WriteData(0xB0);
	LCD_WriteStringFlash(PSTR("C"));

	LCD_GotoXY(0,5);
	LCD_WriteString(AsciiTemp);
}

void LCD_InitFSM(){
	FSM_Staate = 0;
}

void LCD_ProcessFSM(){
	if (GetBroadcastMessage(MSG_MENU_EXIT)){
		FSM_Staate = 0;
	}

	switch (FSM_Staate){
		case 0:
			if (GetMessage(MSG_TEMP_CONVERT_COMPLETED)){
				LCD_ShowTemp();
			}
			if (GetMessage(MSG_KEYB_KEY_PRESSED)){
				switch (Keyb_GetScancode()){
					case KEY_1: LCD_WriteCmd(CLEAR_SCREEN); break;
					case KEY_2: LCD_WriteCmd(CURSOR_MOVE_LEFT); break;
					case KEY_3: LCD_WriteCmd(CURSOR_MOVE_RIGHT); break;
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
			LCD_WriteCmd(CLEAR_SCREEN);
			LCD_WriteStringFlash(PSTR("MENU:"));
			LCD_GotoXY(1,0);
			SET_MENU(x1);
			FSM_Staate = 11;
			break;

		case 11:
			if (GetMessage(MSG_KEYB_KEY_PRESSED)){
				LCD_WriteCmd(CLEAR_SCREEN);
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
			LCD_WriteCmd(CLEAR_SCREEN);
			LCD_GotoXY(0,0);
			LCD_WriteStringFlash(PSTR("There was error!"));
			break;
	}
}
