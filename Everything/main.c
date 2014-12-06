/* MCU: ATMega16A
 * F_CPU 8000000L
 *
 * Фьюзы:
 * 	High:	0xd7
 * 	Low:	0xc4
 */

/*
 * Todo
 * 	- Сделать настройки для подключения периферии дефайнами, для универсальности
 * 	- Инициализация UART через параметры функции
 * 	- дополнить и изучить функциональность темп. датчика
 * V- сделать прием в UART (!!!)
 * 	- задефайнить настройки LCD, сделать их словами, а не хексом
 * 	- сделать дрыги не дефайном, а функцией. По-моему, так будет лучше...
 * V- Попробовать залить символ в LCD массивом, может будет компактнее в памяти
 * 	- Перевести LCD в 4х-битный режим. ДАЕШЬ МЕНЬШЕ ПРОВОДОВ!!!
 * N- Повесить вывод температуры на кнопку через внешнее прерывание
 * 	- Сделать софтовый анти-дребезг кнопок (счетчик дребезжания?..)
 * V- Дополнить отсутствующие комментарии, а то запутаюсь!
 * V- Залить это дело в GitHub? Опять же, запутаюсь. Да и бэкапы не помешают.
 *  - Переделать под конечный автомат
 */

#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "Timers.h"		// Библиотека таймеров Конечного Автомата (КА)
#include "Messages.h"	// Библиотека сообщений КА
#include "keyboard.h"	// Библиотека клавиатуры КА

#include "my_uart.h"
#include "DS18B20.h"
#include "LCD.h"

ISR (TIMER2_OVF_vect){
	ProcessTimers();		// добавляем единичку ко всем виртуальным таймерам по переполнению и по условию (для секунд-минут итд)
}



int main(void) {
	UART_Init(MYUBRR);
	LCD_init();
	InitTimers();
	InitMessages();
	DS_InitFSM();
	LCD_InitFSM();
	Keyb_InitFSM();

	// Инициализация аппаратного таймера
	TCCR2 = (1 << CS22) | (1 << CS21) | (0 << CS20);	// Prescaler 256, один тик длится 0.032мс при частоте камня 8МГц
	TCNT2 = (255 - 125);	// Обнуляем счетчик. Начинаем тикать 125 раз.
							// За 125 тиков до переполнения пройдет 4мс. Одна секунда длится 250 переполнений.
	TIMSK = 1 << TOIE2;		// Запуск прерывания по переполнению таймера

	LCD_WriteCmd(0x01);
	LCD_WriteString("Temp=");
	LCD_GotoXY(0,10);
	LCD_WriteData(0xB0);
	LCD_WriteData('C');


	asm("sei");	// Разрешаем прерывания

	while (1) {
		LCD_ProcessFSM();
		DS_ProcessFSM();
		Keyb_ProcessFSM();
		ProcessMessages();
	}
	return 0;	// Мы никогда не достигнем этого места...
}
