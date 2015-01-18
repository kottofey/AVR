/*
	           MICRO-MENU

	     (C) Dean Camera, 2007
	    www.fourwalledcubicle.com
	dean_camera@fourwalledcubicle.com

	    Royalty-free for all uses.
	                                  */

#ifndef MENU_H
#define MENU_H

#include <avr/pgmspace.h>

// Typedefs:
typedef void (*FuncPtr)(void);
typedef void (*WriteFuncPtr)(const char*);

typedef const struct PROGMEM{
	void       *Next;
	void       *Previous;
	void       *Parent;
	void       *Sibling;
	FuncPtr     SelectFunc;
	FuncPtr     EnterFunc;
	const char  Text[];
} Menu_Item;

// Externs:
extern WriteFuncPtr*    WriteFunc;
extern Menu_Item        Null_Menu;
extern Menu_Item*       CurrMenuItem;

// Defines and Macros:
#define NULL_ENTRY Null_Menu
#define NULL_FUNC  (void*)0
#define NULL_TEXT  0x00

#define PREVIOUS   *((Menu_Item*)pgm_read_word(&CurrMenuItem->Previous))
#define NEXT       *((Menu_Item*)pgm_read_word(&CurrMenuItem->Next))
#define PARENT     *((Menu_Item*)pgm_read_word(&CurrMenuItem->Parent))
#define SIBLING    *((Menu_Item*)pgm_read_word(&CurrMenuItem->Sibling))
#define SELECTFUNC *((FuncPtr*)pgm_read_word(&CurrMenuItem->SelectFunc))
#define ENTERFUNC  *((FuncPtr*)pgm_read_word(&CurrMenuItem->EnterFunc))

#define MAKE_MENU(Name, Next, Previous, Parent, Sibling, SelectFunc, EnterFunc, Text) \
    extern Menu_Item Next;     \
	extern Menu_Item Previous; \
	extern Menu_Item Parent;   \
	extern Menu_Item Sibling;  \
	Menu_Item Name = {(void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Sibling, (FuncPtr)SelectFunc, (FuncPtr)EnterFunc, { Text }}

#define SET_MENU_WRITE_FUNC(x) \
	WriteFunc = (WriteFuncPtr*)&x;

#define SET_MENU(x) \
	MenuChange((Menu_Item*)&x);

#define GO_MENU_FUNC(x)  \
	MenuFunc((FuncPtr*)&x)

#define EXTERN_MENU(Name) \
    extern Menu_Item Name;

// Prototypes:
void MenuChange(Menu_Item* NewMenu);
void MenuFunc(FuncPtr* Function);
void ds_1sec();
void ds_5sec();
void ds_9bit();
void ds_10bit();
void ds_11bit();
void ds_12bit();
void menu_exit();
void cursor1();
void cursor2();
void cursor3();
void cursor4();

EXTERN_MENU(x1);
EXTERN_MENU(x2);
EXTERN_MENU(x3);

EXTERN_MENU(x1y1);
EXTERN_MENU(x1y2);
EXTERN_MENU(x1y3);

EXTERN_MENU(x2y1);
EXTERN_MENU(x2y2);

EXTERN_MENU(x1y1z1);
EXTERN_MENU(x1y1z2);

#endif
