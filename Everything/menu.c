/*
/	           MICRO-MENU

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
#include "keyboard.h"
#include <avr/pgmspace.h>
#include <stdlib.h>

// Измерение температуры датчиком занимает определенное количество времени. Через 1 сек можно гарантировано забаирать
// измерянное значение. Чтобы ускорить вывод температуры в реальном времени (мгновенно после окончания замера датчиком),
// применяется переменная ds_convert_period, которая равна соответственно 9bit = 24, 10bit = 47, 11bit = 94, 12bit = 188
// Это время в тиках программного таймера для конвертации DS18b20 (взято из даташита). Один тик программного таймера = 4мс при
// частоте процессора 8МГц и предделителя системного таймера 1/256. НЕ ПУТАТЬ С ТИКОМ СИСТЕМНОГО ТАЙМЕРА ПРОЦЕССОРА! ОН ДЛИТСЯ 0.032мс!

unsigned char current_setting;
unsigned char current_setting_selected = 0;

const settings Setting[] PROGMEM = {
		{ " bit",	{0x1F, 0x3F, 0x5F, 0x7F},	{"9 ", "10", "11", "12"},		{24, 47, 94, 188}	},	// DS_BitsSetting
		{ " sec",	{0, 1, 10, 30},				{"0 ", "1 ", "10", "30"},		{}					},	// DS_PeriodSetting
		{ " sec",	{1, 3, 5, 10},				{"1 ", "3 ", "5 ", "10"},		{}					},	// LCD_BacklightSetting
		{ "",		{0x0C, 0x0D, 0x0E, 0x0F},	{"NoC", "Blk", "LiN", "LiB"},	{}					}	// LCD_CursorSetting
};

//#define MENU_USE_SRAM_BUFFER 128

Menu_Item        Null_Menu = {(void*)0, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0, {0x00}};
Menu_Item*       CurrMenuItem;
WriteFuncPtr*    WriteFunc;

// Menus: 			Name		Next		Previous		Parent			Sibling			Select_Func			Enter_Func				Text
MAKE_MENU(			x1,			x2,			x5,				x1,				x1y1,			NULL_FUNC,			NULL_FUNC,				"1.Установки DS >");

	MAKE_MENU(		x1y1,		x1y2,		x1y2,			x1,				x1y1z1,			NULL_FUNC,			Enter_DS_PeriodSetting,	"1.1.Период изм.>");
		MAKE_MENU(	x1y1z1,		x1y1z1,		x1y1z1,			x1y1,			NULL_ENTRY,		NULL_FUNC,			setting_change,			NULL_TEXT);

	MAKE_MENU(		x1y2,		x1y1,		x1y1,			x1,				x1y2z1,			NULL_FUNC,			Enter_DS_BitsSetting,	"1.2.Разрешение >");
		MAKE_MENU(	x1y2z1,		x1y2z1,		x1y2z1,			x1y2,			NULL_ENTRY,		NULL_FUNC,		 	setting_change,			NULL_TEXT);

MAKE_MENU(			x2,			x3,			x1,				MenuExit,		x2y1,			NULL_FUNC,			NULL_FUNC,				"2.Установки LCD>");

	MAKE_MENU(		x2y1,		x2y2,		x2y2,			x2,				x2y1z1,			NULL_FUNC,			Enter_LCD_CursorSetting,"2.1.Реж.курсора>");
		MAKE_MENU(	x2y1z1,		x2y1z1,		x2y1z1,			x2y1,			NULL_ENTRY,		NULL_FUNC,			setting_change,			NULL_TEXT);

	MAKE_MENU(		x2y2,		x2y1,		x2y1,			x2,				x2y2z1,			NULL_FUNC,			Enter_LCD_BacklightSetting,"2.2.Подсветка   ");
		MAKE_MENU(	x2y2z1,		x2y2z1,		x2y2z1,			x2y2,			NULL_ENTRY,		NULL_FUNC,			setting_change,			NULL_TEXT);

MAKE_MENU(			x3,			x4,			x2,				MenuExit,		x3,				ShowAllROM,			NULL_FUNC,			"3.ShowAllROM    ");

MAKE_MENU(			x4,			x5,			x3,				MenuExit,		x4,				DS_ReadROM,			NULL_FUNC,				"4.ROM > EEPROM");

MAKE_MENU(			x5,			x1,			x4,				MenuExit,		x5,				menu_exit,			NULL_FUNC,				"5.Выход           ");

MAKE_MENU(			MenuExit,	NULL_ENTRY,	NULL_ENTRY,		NULL_ENTRY,		NULL_ENTRY,		NULL_FUNC,			menu_exit,				NULL_TEXT);

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


void Enter_DS_BitsSetting(){	// Установка разрешения датчика температуры [0]
	current_setting = DS_BitsSetting;
	return;
}


void Enter_DS_PeriodSetting(){	// Период измерений температуры
	current_setting = DS_PeriodSetting;
	return;
}


void Enter_LCD_BacklightSetting(){
	current_setting = LCD_BacklightSetting;
	return;
}


void Enter_LCD_CursorSetting(){
	current_setting = LCD_CursorSetting;
	return;
}

void setting_change(){
	FSM_Staate = 12;	// Переводим КА в состояние изменения настройки
	LCD_WriteCmd(LCD_CLEAR_SCREEN);
	LCD_WriteStringFlash(PSTR("<CHANGE> OK<-+->"));	// Вывод верхней строки-инструкции по кнопкам
	LCD_GotoXY(1,0);
	LCD_WriteStringFlash(Setting[current_setting].item1_text[current_setting_selected]);	// Вывод текущей настройки
	LCD_WriteStringFlash(Setting[current_setting].text);
	return;
}


void menu_exit(){
	SendBroadcastMessage(MSG_MENU_EXIT);
	return;
}


void sleep(){
	SendBroadcastMessage(MSG_MENU_EXIT);
	LCD_WriteCmd(LCD_CLEAR_SCREEN);
	LCD_WriteStringFlash(PSTR("хр-р-р-р..."));
	_delay_ms(1000);
	GICR |= (1 << INT1);	// Разрешаем прерывание INT1
	LCD_BACKLIGHT_PORT &= ~(1 << LCD_BACKLIGHT_PIN); // Выключаем подсветку
	LCD_WriteCmd(LCD_OFF);	// Выключаем LCD
	asm("sleep");			// Засыпаем! А просыпаемся от прерывания INT1!
	GICR &= ~(1 << INT1);	// Запрещаем прерывание INT1
	LCD_WriteCmd(LCD_ON);	// Включаем обратно дисплей
}
