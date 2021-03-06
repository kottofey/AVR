/*
	           MICRO-MENU

	     (C) Dean Camera, 2007
	    www.fourwalledcubicle.com
	dean_camera@fourwalledcubicle.com

	    Royalty-free for all uses.
	                                  */

#include "menu.h"
#include "Messages.h"
#include "my_uart.h"
#include "DS18B20.h"
#include "Timers.h"
#include "LCD.h"
#include <util/delay.h>
#include "something.h"

//#define MENU_USE_SRAM_BUFFER 128

Menu_Item        Null_Menu = {(void*)0, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0, {"N/A"}};
Menu_Item*       CurrMenuItem;
WriteFuncPtr*    WriteFunc;

// Menus: 			Name		Next		Previous		Parent			Sibling			Select_Func		Enter_Func		Text
MAKE_MENU(			x1,			x2,			x5,				x1,				x1y1,			NULL_FUNC,		NULL_FUNC,		"DS Settings    >");
	MAKE_MENU(		x1y1,		x1y2,		x1y4,			x1,				x1y1z1,			NULL_FUNC,		NULL_FUNC,		"Measure Period >");
		MAKE_MENU(	x1y1z1,		x1y1z2,		x1y1z2,			x1y1,			x1y1z1,			ds_1sec,		NULL_FUNC,		"> 1 sec");
		MAKE_MENU(	x1y1z2,		x1y1z1,		x1y1z1,			x1y1,			x1y1z2,			ds_5sec,		NULL_FUNC,		"> 10 sec");

	MAKE_MENU(		x1y2,		x1y3,		x1y1,			x1,				x1y2z1,			NULL_FUNC,		NULL_FUNC,		"Convert bits   >");
		MAKE_MENU(	x1y2z1,		x1y2z2,		x1y2z4,			x1y2,			x1y2z1,			ds_9bit,		NULL_FUNC,		"> 9 bit");
		MAKE_MENU(	x1y2z2,		x1y2z3,		x1y2z1,			x1y2,			x1y2z2,			ds_10bit,		NULL_FUNC,		"> 10 bit");
		MAKE_MENU(	x1y2z3,		x1y2z4,		x1y2z2,			x1y2,			x1y2z3,			ds_11bit,		NULL_FUNC,		"> 11 bit");
		MAKE_MENU(	x1y2z4,		x1y2z1,		x1y2z3,			x1y2,			x1y2z4,			ds_12bit,		NULL_FUNC,		"> 12 bit");
	MAKE_MENU(		x1y3,		x1y4,		x1y2,			x1,				x1y3,			DS_Start,		NULL_FUNC,		"Start          >");
	MAKE_MENU(		x1y4,		x1y1,		x1y3,			x1,				x1y4,			DS_Stop,		NULL_FUNC,		"Stop           >");

MAKE_MENU(			x2,			x3,			x1,				x2,				x2y1,			NULL_FUNC,		NULL_FUNC,		"LCD Settings   >");
	MAKE_MENU(		x2y1,		x2y2,		x2y5,			x2,				x2y1,			cursor1,		NULL_FUNC,		"> No cursor");
	MAKE_MENU(		x2y2,		x2y3,		x2y1,			x2,				x2y2,			cursor2,		NULL_FUNC,		"> Block, blink");
	MAKE_MENU(		x2y3,		x2y4,		x2y2,			x2,				x2y3,			cursor3,		NULL_FUNC,		"> Line, no blink");
	MAKE_MENU(		x2y4,		x2y5,		x2y3,			x2,				x2y4,			cursor4,		NULL_FUNC,		"> Line, blink");
	MAKE_MENU(		x2y5,		x2y1,		x2y4,			x2,				x2y5,			LCD_ReadCursor,	NULL_FUNC,		"> Cursor to UART");

MAKE_MENU(			x3,			x4,			x2,				x3,				x3,				DS_SearchAllDevices,NULL_FUNC,	"DS SEARCHROM");
MAKE_MENU(			x4,			x5,			x3,				x4,				x4,				DS_ReadROM,		NULL_FUNC,		"READ_ROM to UART");
MAKE_MENU(			x5,			x1,			x4,				x5,				x5,				menu_exit,		NULL_FUNC,		"Exit");


//Functions
void MenuChange(Menu_Item* NewMenu)
{
	if ((void*)NewMenu == (void*)&NULL_ENTRY)
		return;

	CurrMenuItem = NewMenu;

	#if defined(MENU_USE_SRAM_BUFFER)
		#if (MENU_USE_SRAM_BUFFER < 1)
		  #error Menu SRAM Buffer Size not Defined!
		#endif

		char Buffer[MENU_USE_SRAM_BUFFER];
		strcpy_P(Buffer, CurrMenuItem->Text);

		((WriteFuncPtr)WriteFunc)((const char*)Buffer);
	#else
		((WriteFuncPtr)WriteFunc)((const char*)CurrMenuItem->Text);
	#endif

	GO_MENU_FUNC(ENTERFUNC);
}

void MenuFunc(FuncPtr* Function)
{
	if ((void*)Function == (void*)NULL_FUNC)
	  return;
	((FuncPtr)Function)();
}


void ds_9bit(){
	DS_Reset();
	DS_WriteByte(SKIP_ROM);
	DS_WriteByte(WRITE_SCRATCHPAD);
	DS_WriteByte(0x00);			// Th - байт
	DS_WriteByte(0x00);			// Tl - байт
	DS_WriteByte(0b00011111);	// Config - байт
	ds_convert_period = 24;		// 9bit=24, 10bit=47, 11bit=94, 12bit=188
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}

void ds_10bit(){
	DS_Reset();
	DS_WriteByte(SKIP_ROM);
	DS_WriteByte(WRITE_SCRATCHPAD);
	DS_WriteByte(0x00); // Th - байт
	DS_WriteByte(0x00); // Tl - байт
	DS_WriteByte(0b00111111); // Config - байт
	ds_convert_period = 47;		// 9bit=24, 10bit=47, 11bit=94, 12bit=188
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}

void ds_11bit(){
	DS_Reset();
	DS_WriteByte(SKIP_ROM);
	DS_WriteByte(WRITE_SCRATCHPAD);
	DS_WriteByte(0x00);			// Th - байт
	DS_WriteByte(0x00);			// Tl - байт
	DS_WriteByte(0b01011111);	// Config - байт
	ds_convert_period = 94;		// 9bit=24, 10bit=47, 11bit=94, 12bit=188
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}

void ds_12bit(){
	DS_Reset();
	DS_WriteByte(SKIP_ROM);
	DS_WriteByte(WRITE_SCRATCHPAD);
	DS_WriteByte(0x00);			// Th - байт
	DS_WriteByte(0x00);			// Tl - байт
	DS_WriteByte(0b01111111);	// Config - байт
	ds_convert_period = 0xBC;	// 9bit = 0x18(24 ticks), 10bit = 0x2F(47 ticks), 11bit = 0x5E(94 ticks), 12bit = 0xBC(188 ticks), 1tick = 4ms
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}

void ds_1sec(){
	ds_refresh_period = 1;
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}

void ds_5sec(){
	ds_refresh_period = 10*sec;
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}

void menu_exit(){
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}

void cursor1(){
// Курсора нет, ничего не мигает
	LCD_WriteCmd(LCD_CURSOR_NOCURSOR);
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}

void cursor2(){
// Курсора нет, мигает весь символ
	LCD_WriteCmd(LCD_CURSOR_BLOCKCURSOR);
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}

void cursor3(){
// Курсор в виде черты и не мигает
	LCD_WriteCmd(LCD_CURSOR_LINE_NOBLINK);
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}

void cursor4(){
	// Курсор в виде черты и мигает
	LCD_WriteCmd(LCD_CURSOR_LINE_BLINK);
	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(200);
	return;
}
