/*
 * Messages.h
 *
 *  Created on: Nov 28, 2014
 *      Author: kottofey
 */
#include <avr/io.h>

#ifndef MESSAGES_H_
#define MESSAGES_H_

#define MAX_MESSAGES 10
#define MAX_BROADCAST_MESSAGES 10

//extern uint8_t Messages[MAX_MESSAGES]; // Контейнер для сообщений
//extern uint8_t BroadcastMessages[MAX_BROADCAST_MESSAGES]; // Контейнер для широковещательных сообщений

// MSG 0 - 7 для DS18b20
#define MSG_TEMP_CONVERT_COMPLETED 0
#define MSG_TEMP_HIGH_ALARM 1
#define MSG_TEMP_LOW_ALARM 2
#define MSG_TEMP_SENSOR_MISSING 3
#define MSG_TEMP_NEGATIVE 4

#define MSG_KEYB_KEY_PRESSED 5

#define MSG_MENU_STARTED 6
#define MSG_MENU_EXIT 7

void InitMessages();
void SendMessage(uint8_t Msg);
void SendBroadcastMessage(uint8_t Msg);
uint8_t GetMessage(uint8_t Msg);
uint8_t GetBroadcastMessage(uint8_t);
void ProcessMessages();

#endif /* MESSAGES_H_ */
