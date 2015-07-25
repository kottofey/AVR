#include <avr/io.h>
#include <util/delay.h>
#include "keyboard.h"
#include "my_uart.h"
#include "LCD.h"
#include "Timers.h"
#include "Messages.h"

uint8_t FSM_Statee; // Состояние Конечного Автомата
uint8_t _scancode = 0;	// предыдущее состояние кнопки
uint8_t scancode = 0;	// скан-код нажатой кнопки

// Инициализация клавиатуры
void Keyb_InitFSM(){
	DDRD &= ~(1 << PD3); DDRD &= ~(1 << PD4);	// Пины PD3 и PD4 на вход c подтяжкой
	PORTD |= (1 << PD3) | (1 << PD4);			// источник тока если посажено на землю (+5V)

	DDRD |= (1 << PD5) | (1 << PD6);	// Пины PD5 и PD6 на выход
	PORTD |= (1 << PD5) | (1 << PD6);	// Пины 5 и 6 высокие (пока что не-GND)

	StartTimer(TIMER_KEYB);	// Запускаем таймер, он понадобится для антидребезга и расчета задержек для повторов
	FSM_Statee = 0;
}

uint8_t Keyb_Scan(){
	uint8_t key_mask[2]= {0b11011111, 0b10111111};
	uint8_t scan_mask = 0b00011000;
	uint8_t pina, pina1, pina2;	// Промежуточные переменные для формирования скан-кода

	for (uint8_t i = 0; i < 2; i++){
		uint8_t bit = PORTD;
		bit = (bit & scan_mask) | key_mask[i];
		PORTD = bit;
		asm("nop"); asm("nop"); asm("nop"); asm("nop"); // задержка чтоб ножки успели выставить нужный уровень
		pina = ((PIND & 0b01111000) >> 3); // Вычленяем текущее состояние ножек клавиатуры
		if (i == 0){pina1 = pina;}	// Первая часть скан-кода
		else {pina2 = pina;}		// Вторая часть скан-кода
	}
	scancode = (pina1 << 4) | pina2;	// Укладываем первую и вторую часть скан-кода по порядку
	return scancode;
}

uint8_t Keyb_GetScancode(){	// Возвращает код нажатой клавиши и 0xFF если ничего не нажато
	switch (scancode){		// Список комбинаций вычислен брутфорсом :)
	case 0xB7: return KEY_0; break;
	case 0xB5: return KEY_1; break;
	case 0xB6: return KEY_2; break;
	case 0x97: return KEY_3; break;
	case 0xA7: return KEY_4; break;
	case 0xB4: return KEY_1_2; break;
	case 0x95: return KEY_1_3; break;
	case 0xA5: return KEY_1_4; break;
	case 0x96: return KEY_2_3; break;
	case 0xA6: return KEY_2_4; break;
	case 0x87: return KEY_3_4; break;
	case 0x84: return KEY_1_2_3; break;
	}
	return 0xFF;
}

void Keyb_ProcessFSM(){
	// Проверка таймаута подсветки. Выключаем таймер и подсветку, если таймаут.
	if (GetTimer(TIMER_LCD_BACKLIGHT_TIMEOUT) >= LCD_BacklightTimeout){
		LCD_BACKLIGHT_PORT &= ~(1 << LCD_BACKLIGHT_PIN);
		StopTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);
	}

	switch (FSM_Statee){
		case 0:
//			UART_TxString("KB0\n");
			Keyb_Scan();
			if(scancode != 0xB7){	// Вроде как есть нажатие! Обнуляем таймер антидребезга!
				_scancode = scancode;
				ResetTimer(TIMER_KEYB);
				StartTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);
				FSM_Statee = 1;
			}
			break;
		case 1:
//			UART_TxString("KB1\n");
			if (GetTimer(TIMER_KEYB) >= DEBOUNCE){ // задержка на дребезг
				FSM_Statee = 2;
			}
			break;
		case 2:
//			UART_TxString("KB2\n");
			Keyb_Scan();
			if (scancode == _scancode){ // Если нажатие всё еще есть, то идем дальше
				SendMessage(MSG_KEYB_KEY_PRESSED);
				ResetTimer(TIMER_KEYB);
				ResetTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);		// По нажатию клавиши сбрасывается таймер подсветки
				LCD_BACKLIGHT_PORT |= (1 << LCD_BACKLIGHT_PIN);	// и она включается на время LCD_BACKLIGHT_TIMEOUT (указано в настройках)
				FSM_Statee = 3;
			}
			else FSM_Statee = 0; // Если нажатия нет, переходим к началу
			break;
		case 3:
//			UART_TxString("KB3\n");
			Keyb_Scan();
			if (scancode == _scancode){
				if (GetTimer(TIMER_KEYB) >= FIRST_REPEAT_DELAY){
					ResetTimer(TIMER_KEYB);
					ResetTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);	// Подсветка все еще нужна
					SendMessage(MSG_KEYB_KEY_PRESSED);
					FSM_Statee = 4;
				};
			}
			else FSM_Statee = 0;
			break;
		case 4:
//			UART_TxString("KB4\n");
			Keyb_Scan();
			if (scancode == _scancode){
				if (GetTimer(TIMER_KEYB) >= REPEAT_DELAY){
					ResetTimer(TIMER_KEYB);
					ResetTimer(TIMER_LCD_BACKLIGHT_TIMEOUT);	// Подсветка все еще нужна
					SendMessage(MSG_KEYB_KEY_PRESSED);
				};
			}
			else {
				FSM_Statee = 0;
			}
			break;
	} // Конец switch
}
