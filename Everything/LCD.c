#include <avr/io.h>
#include <util/delay.h>
#include "LCD.h"
#include "DS18B20.h"
#include "Messages.h"
#include "Timers.h"
#include "my_uart.h"

uint8_t FSM_State; // Переменная состояния КА
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
	LCD_WriteCmd(0b00001111);	// Disp On/Off Control (вкл[1]/выкл[0] экрана, режим курсора[11])
	LCD_WriteCmd(0b00000110);	// Entry Mode Set (курсор влево/вправо | разрешение сдвига экрана)
//	LCD_WriteCmd(0b00000001);	// Clear Display

//	debug
	LCD_WriteCmd(0b00000010);	// Return Home (курсор в начало)
	LCD_WriteString("LCD Init OK");
	_delay_ms(200);
	LCD_WriteCmd(0x01);
}

void LCD_WriteCmd(char b) {
//	asm("cli");
	LCD_WriteByte(b, 0);
//	asm("sei");
}

void LCD_WriteData(char b) {
//	asm("cli");
	LCD_WriteByte(b, 1);
//	asm("sei");
}

void LCD_WriteByte(char b, char cd) {
	asm("cli");
	DDRC = 0b00000000; // PORTC-шина_данных на вход
	DDRA |= (1 << 0) | (1 << 1) | (1 << 2); // пины 0, 1, 2 порта A на вывод

// Чтение флага занятости
	PORTA &= ~(1 << PINA1); //	A0 = 0
	PORTA |= (1 << PINA2); // 	RW = 1

	_delay_us(1); 			// 	>40ns
	PORTA |= (1 << PINA0); 	//	E = 1
	_delay_us(1);			//	>230ns
	while (PINC >= 0x80)
		;	//	Ждать сброса флага занятости на пине PINC7

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
	asm("sei");
}

void LCD_WriteString(char *data) {
	int i = 0;
	while (data[i] != 0) {
		LCD_WriteData(data[i]);
		i++;
	}
	i = 0;
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
	//Значок молнии
	if (!addr)
		LCD_WriteCmd(addr + 0x40);
	LCD_WriteCmd(addr * 0x08 + 0x40);	// Выбираем адрес символа в CGRAM

	for (int i = 0; i < 8; i++) {
		LCD_WriteData(a0[i]);
	}
}

void LCD_ShowTemp(){
	LCD_GotoXY(0,5);
	LCD_WriteString(AsciiTemp);
}

void LCD_InitFSM(){
	uint8_t FSM_State = 0;
}

void LCD_ProcessFSM(){
	switch(FSM_State){
		case 0:
			if (GetMessage(MSG_TEMP_CONVERT_COMPLETED)){
				LCD_ShowTemp();
			}
			break;
	}
}
