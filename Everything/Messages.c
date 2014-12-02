#include "Messages.h"

uint8_t Messages[MAX_MESSAGES];

void InitMessages(){
	for (uint8_t i = 0; i < MAX_MESSAGES; i++){
		Messages[i] = 0;
	}
}

void SendMessage(uint8_t Msg){
	Messages[Msg] = 1;
}

void ProcessMessages(){
	for (uint8_t i = 0; i < MAX_MESSAGES; i++){
		if (Messages[i] == 2) Messages[i] = 0;
		if (Messages[i] == 1) Messages[i] = 2;
	}
}

char GetMessage(uint8_t Msg){
	if (Messages[Msg] == 2) {
		Messages[Msg] = 0;
		return 1;
	}
	return 0;
}
