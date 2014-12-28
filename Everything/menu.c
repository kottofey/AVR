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

//#define MENU_USE_SRAM_BUFFER 128

Menu_Item        Null_Menu = {(void*)0, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0, {"N/A"}};
Menu_Item*       CurrMenuItem;
WriteFuncPtr*    WriteFunc;

// Menus: 			Name		Next		Previous		Parent			Sibling			Select_Func		Enter_Func		Text
MAKE_MENU(			x1,			x2,			x3,				x1,				x1y1,			NULL_FUNC,		NULL_FUNC,		"DS18B20 Settings");
	MAKE_MENU(		x1y1,		x1y2,		x1y3,			x1,				x1y1z1,			NULL_FUNC,		NULL_FUNC,		"Measure Period");
		MAKE_MENU(	x1y1z1,		x1y1z2,		x1y1z2,			x1y1,			x1y1z1,			ds_1sec,		NULL_FUNC,		"1 sec");
		MAKE_MENU(	x1y1z2,		x1y1z1,		x1y1z1,			x1y1,			x1y1z2,			ds_5sec,		NULL_FUNC,		"5 sec");

	MAKE_MENU(		x1y2,		x1y3,		x1y1,			x1,				x1y2,			NULL_FUNC,		NULL_FUNC,		"Dummy 1.2");

	MAKE_MENU(		x1y3,		x1y1,		x1y2,			x1,				x1y3,			NULL_FUNC,		NULL_FUNC,		"Dummy 1.3");

MAKE_MENU(			x2,			x3,			x1,				x2,				x2,				NULL_FUNC,		NULL_FUNC,		"Dummy");

MAKE_MENU(			x3,			x1,			x2,				x3,				x3,				menu_exit,		NULL_FUNC,		"Exit");


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

void ds_1sec(){
	UART_TxString("ds_1sec\n");
	return;
}

void ds_5sec(){
	UART_TxString("ds_5sec\n");
	return;
}

void menu_exit(){
	UART_TxString("exit\n");
	return;
}
