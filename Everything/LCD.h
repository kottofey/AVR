/*
 *	- LCD_Init				Инициализация дисплея
 *	- LCD_WriteCmd			Функция ввода комманды
 *	- LCD_Write_Data		Функция ввода символа в CGRAM или вывода на экран из DDRAM
 *	- LCD_WriteString		Функция ввода строки unsigned char
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
 * 	PORTC[0..7] - DB[0..7] (Шина данных) 8-битный режим. Должно быть подключено к одному и тому же порту с пинами идущими по порядку!
 * 	PORTC[4..7] - DB[4..7] (Шина данных) 4-битный режим. Должно быть подключено к одному и тому же порту с пинами идущими по порядку!
 */

#ifndef LCD_H_
#define LCD_H_

///// Настройки подключения LCD /////////////
// Для 4-битного режима используются только пины LCD 4, 5, 6 и 7 из шины данных.
// Чтобы включить 4-битный режим нужно раскомментировать соответствующий дефайн
// и записать единичками пины в маску порта шины данных в LCD.c
#define LCD_DATA_DDR DDRC
#define LCD_DATA_PORT PORTC
#define LCD_DATA_PIN PINC

#define LCD_SIGNAL_DDR DDRC
#define LCD_SIGNAL_PIN PINC
#define LCD_SIGNAL_PORT PORTC

#define LCD_A0_PIN PC0
#define LCD_RW_PIN PC1
#define LCD_E_PIN PC2

#define LCD_BACKLIGHT_PORT PORTC
#define LCD_BACKLIGHT_DDR DDRC
#define LCD_BACKLIGHT_PIN PC7

#define LCD_4bitMode 1 // 1 = 4х-битный режим; 0 = 8ми-битный режим

////// Управляющие комманды дисплея////////
#define LCD_CLEAR_SCREEN		0b00000001
#define LCD_CURSOR_RETURN		0b00000010
#define LCD_CURSOR_MOVE_LEFT	0b00010000
#define LCD_CURSOR_MOVE_RIGHT	0b00010100
#define LCD_CURSOR_NOCURSOR		0b00001100
#define LCD_CURSOR_BLOCKCURSOR	0b00001101
#define LCD_CURSOR_LINE_NOBLINK	0b00001110
#define LCD_CURSOR_LINE_BLINK	0b00001111
#define LCD_ON					0b00001111
#define LCD_OFF					0b00001011

extern uint16_t LCD_BacklightTimeout;
extern uint8_t FSM_Staate;

/////// Прототипы функций//////////////////
void LCD_init();
void LCD_WriteCmd(unsigned char b);
void LCD_WriteData(unsigned char b);
void LCD_WriteByte(unsigned char b, unsigned char cd);
void LCD_WriteString(char *data);
void LCD_WriteStringFlash(const unsigned char *data);
uint8_t LCD_ReadCursor();
void LCD_GotoXY(unsigned char stroka, unsigned char simvol);
void LCD_MakeSymbol(unsigned char addr, unsigned char * a0); // Символ задается массивом. Программа компактнее, но отжирает больше памяти flash
void LCD_ShowTemp();
void LCD_InitFSM();
void LCD_ProcessFSM();
void key2_action();
void key3_action();
void apply_setting();

#endif
