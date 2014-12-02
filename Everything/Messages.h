/*
 * Messages.h
 *
 *  Created on: Nov 28, 2014
 *      Author: kottofey
 */
#include <avr/io.h>

#ifndef MESSAGES_H_
#define MESSAGES_H_

#define MAX_MESSAGES 24

extern uint8_t Messages[MAX_MESSAGES]; // Контейнер для сообщений

// MSG 0 - 7 для DS18b20
#define MSG_TEMP_NEGATIVE 0
#define MSG_TEMP_CONVERT_COMPLETED 0
#define MSG_TEMP_HIGH_ALARM 1
#define MSG_TEMP_LOW_ALARM 2
#define MSG_TEMP_SENSOR_MISSING 3


#define MSG_KEYB_KEY_PRESSED 8

void InitMessages();
void SendMessage(uint8_t Msg);
char GetMessage(uint8_t Msg);
void ProcessMessages();

#endif /* MESSAGES_H_ */
