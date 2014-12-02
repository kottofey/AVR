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
 * 	PORTC[0..7] - DB[0..7] (Шина данных)
 */

#ifndef LCD_H_
#define LCD_H_

void LCD_init();
void LCD_WriteCmd(char b);
void LCD_WriteData(char b);
void LCD_WriteByte(char b, char cd);
void LCD_WriteString(char *data);
void LCD_GotoXY(char stroka, char simvol);
void LCD_MakeSymbol(char addr, char * a0); // Символ задается массивом. Программа компактнее, но отжирает больше памяти flash
void LCD_ShowTemp();

void LCD_InitFSM();
void LCD_ProcessFSM();

#endif
