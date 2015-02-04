#include <avr/io.h>
#include <avr/interrupt.h>
#include "my_uart.h"
#include <avr/pgmspace.h>

//	Прерывание USART прием байта завершен
ISR (USART_RXC_vect) {
//	char data = UDR;
//	UART_TxChar(data);

}

void UART_Init(unsigned int ubrr) {

	/* Set baud rate */
	UBRRH = (unsigned char) (ubrr >> 8);
	UBRRL = (unsigned char) ubrr;

	/* Enable receiver and transmitter */
	UCSRB = 1 << RXEN | 1 << TXEN | 0 << RXCIE | 0 << TXCIE | 0 << UDRIE;

	/* Set frame format: 1 stop bit, 8data */
	UCSRC = (1 << URSEL) | (0 << USBS) | (3 << UCSZ0);
}

void UART_TxChar(unsigned char data) {	// Передача из МК в провод

	/* Wait for empty transmit buffer */
	while (!( UCSRA & (1 << UDRE)))
		;

	/* Put data into buffer, sends the data */
	UDR = (unsigned int) data;

}

void UART_TxString(unsigned char * data) {
	while (*data) {
		UART_TxChar(*data);
		data++;
	}
}

void UART_TxStringFlash(const unsigned char *data) {
	while (pgm_read_byte(data)) {
		if (pgm_read_byte(data) >= 0xd0) {
			data++;
			UART_TxChar(pgm_read_byte(data) + 0x30);
			data++;
		}
		else {
			UART_TxChar(pgm_read_byte(data));
			data++;
		}
	}
}
