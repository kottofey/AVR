/*
 *	- LCD_Init				Инициализация дисплея
 *	- LCD_WriteCmd			Функция ввода комманды
 *	- LCD_Write_Data		Функция ввода символа в CGRAM или вывода на экран из DDRAM
 *	- LCD_WriteString		Функция ввода строки char
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


////// Управляющие комманды дисплея////////
#define LCD_CLEAR_SCREEN		0x00000001
#define LCD_CURSOR_MOVE_LEFT	0b00010000
#define LCD_CURSOR_MOVE_RIGHT	0b00010100
#define LCD_CURSOR_NOCURSOR		0b00001100
#define LCD_CURSOR_BLOCKCURSOR	0b00001101
#define LCD_CURSOR_LINE_NOBLINK	0b00001110
#define LCD_CURSOR_LINE_BLINK	0b00001111


/////// Прототипы функций//////////////////
void LCD_init();
void LCD_WriteCmd(char b);
void LCD_WriteData(char b);
void LCD_WriteByte(char b, char cd);
void LCD_WriteString(char *data);
void LCD_WriteStringFlash(const char *data);
void LCD_GotoXY(char stroka, char simvol);
void LCD_MakeSymbol(char addr, char * a0); // Символ задается массивом. Программа компактнее, но отжирает больше памяти flash
void LCD_ShowTemp();

void LCD_InitFSM();
void LCD_ProcessFSM();

#endif
