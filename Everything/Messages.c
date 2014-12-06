#include "Messages.h"

uint8_t Messages[MAX_MESSAGES];
uint8_t BroadcastMessages[MAX_BROADCAST_MESSAGES];

void InitMessages(){
	for (uint8_t i = 0; i < MAX_MESSAGES; i++){
		Messages[i] = 0;
	}
	for (uint8_t i = 0; i < MAX_BROADCAST_MESSAGES; i++){
		BroadcastMessages[i] = 0;
	}
}

void SendMessage(uint8_t Msg){
	Messages[Msg] = 1;
}

void SendBroadcastMessage(uint8_t Msg){
	BroadcastMessages[Msg] = 1;
}

void ProcessMessages(){
	for (uint8_t i = 0; i < MAX_MESSAGES; i++){
		if (Messages[i] == 2) Messages[i] = 0;
		if (Messages[i] == 1) Messages[i] = 2;
	}
	for (uint8_t i = 0; i < MAX_BROADCAST_MESSAGES; i++){
		if (BroadcastMessages[i] == 2) BroadcastMessages[i] = 0;
		if (BroadcastMessages[i] == 1) BroadcastMessages[i] = 2;
	}
}

uint8_t GetMessage(uint8_t Msg){
	if (Messages[Msg] == 2) {
		Messages[Msg] = 0;
		return 1;
	}
	return 0;
}

uint8_t GetBroadcastMessage(uint8_t Msg){
	if (BroadcastMessages[Msg] == 2){
		return 1;
	}
	return 0;
}
