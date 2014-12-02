#include <avr/io.h>
#include <util/delay.h>
#include "keyboard.h"
#include "my_uart.h"
#include "LCD.h"


//////////////////Прерывание по переполнению перенесено в главнцю функцию
//ISR (TIMER2_OVF_vect){
//	unsigned char scancode = Keyb_Scan();
//	if (scancode){
////		LCD_WriteData(scancode);
////		UART_TxChar(scancode);
////		_delay_ms(1000);
//		switch (scancode){
//		case '1': LCD_WriteCmd(0b00010000); while ((PINA & 0b00011000) != 0b00011000); break;
//		case '2': LCD_WriteCmd(0b00010100); while ((PINA & 0b00011000) != 0b00011000); break;
//		case '3': LCD_WriteString("Up"); while ((PINA & 0b00011000) != 0b00011000); break;
//		case '4': LCD_WriteString("Down"); while ((PINA & 0b00011000) != 0b00011000); break;
//		}
//	}
//}
////////////////////////////

// Инициализация клавиатуры
void Keyb_Init(){
	DDRA &= ~(1 << PA3); DDRA &= ~(1 << PA4);	// Пины PA3 и PA4 на вход c подтяжкой
	PORTA |= (1 << PA4) | (1 << PA4);			// источают ток если посажено на землю (+5V)

	DDRA |= (1 << PA5) | (1 << PA6);	// Пины PA5 и PA6 на выход
	PORTA |= (1 << PA5) | (1 << PA6);	// Пины 5 и 6 высокие (пока что не-GND)
}

uint8_t Keyb_Scan(){
//	asm("cli");

	unsigned char key_mask[2]= {0b11011111, 0b10111111};
	unsigned char scan_mask = 0b00011000;
	unsigned char scancode = 0;

	for (uint8_t i = 0; i < 2; i++){
		uint8_t bit = PORTA;
		bit = (bit & scan_mask) | key_mask[i];
		PORTA = bit; UART_TxChar((PINA & 0b00111100) >> 3);
		_delay_ms(30); // антидребезг
		if ( (PINA != 0x17) & (PINA != 0x1B) ){ //если есть нажатие
			switch( (PINA & 0b01111000) >> 3 ){
				case 0b1001: scancode = '1'; break;
				case 0b1010: scancode = '2'; break;
				case 0b0101: scancode = '3'; break;
				case 0b0110: scancode = '4'; break;
				default: scancode = '0'; break;
			}
		}
	}
//	asm("sei");
	return scancode;
}

void Keyb_Action(){
//	asm("cli");
	uint8_t scancode = Keyb_Scan();
	if (scancode){
			switch (scancode){
			case '1': LCD_WriteCmd(0b00010000); while ((PINA & 0b00011000) != 0b00011000); break;
			case '2': LCD_WriteCmd(0b00010100); while ((PINA & 0b00011000) != 0b00011000); break;
			case '3': LCD_WriteString("One"); 	while ((PINA & 0b00011000) != 0b00011000); break;
			case '4': LCD_WriteString("Two"); 	while ((PINA & 0b00011000) != 0b00011000); break;
			}
		}
//	asm("sei");
}

