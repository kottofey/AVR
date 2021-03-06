/*
 * 	- UART_Init				UART инициализация;
 *  - UART_TxChar			UART передача символа;
 *  - UART_TxString			UART передача строки из оперативки;
 *  - UART_TxStringFlash	UART передача строки из флеша;
 */


#ifndef MY_UART_H_
#define MY_UART_H_

// дефайны для настройки UART
#define BAUD 9600	// Скорость передатчика UART
#define MYUBRR F_CPU/16/BAUD-1 // Вычисление бита UBRR

void UART_Init(unsigned int ubrr);
void UART_TxChar(unsigned char data);
void UART_TxString(unsigned char *data);
void UART_TxStringFlash(const unsigned char *data);

#endif
