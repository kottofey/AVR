#include "Timers.h"

uint16_t Timers[MAX_TIMERS];
uint8_t TimersState[MAX_TIMERS];

void InitTimers(){
	for (uint8_t i=0; i < MAX_TIMERS; i++){
		Timers[i] = 0;
		TimersState[i] = 0;
	}
}

uint16_t GetTimer(uint8_t Timer){
	return Timers[Timer];
}

void ResetTimer(uint8_t Timer){
	Timers[Timer] = 0;
}

void StartTimer(uint8_t Timer){
	if (TimersState[Timer] == TIMER_STOPPED){
		Timers[Timer] = 0;
		TimersState[Timer] = TIMER_RUNNING;
	}
}

void StopTimer(uint8_t Timer){
	TimersState[Timer] = TIMER_STOPPED;
}

void PauseTimer(uint8_t Timer){
	if (TimersState[Timer] == TIMER_RUNNING){
		TimersState[Timer] = TIMER_PAUSED;
	}
}

void ResumeTimer(uint8_t Timer){
	if (TimersState[Timer] == TIMER_PAUSED){
		TimersState[Timer] = TIMER_RUNNING;
	}
}

void ProcessTimers(){
	for (uint8_t i = 4; i < MAX_TIMERS; i++){ // Начинаем с 4го таймера, так как таймеры "секунды-минуты-часы-дни" обновляются по условию
		if (TimersState[i] == TIMER_RUNNING){
			Timers[i]++;
		}
	}

	if (GetTimer(TIMER_TICK) == sec)	{ Timers[TIMER_SEC]++; ResetTimer(TIMER_TICK); }
	if (GetTimer(TIMER_SEC) == 60)		{ Timers[TIMER_MIN]++;  ResetTimer(TIMER_SEC); }
	if (GetTimer(TIMER_MIN) == 60)		{ Timers[TIMER_HOUR]++; ResetTimer(TIMER_MIN); }
	if (GetTimer(TIMER_HOUR) == 24)		{ Timers[TIMER_DAY]++; ResetTimer(TIMER_HOUR); }

	TCNT2 = (255 - 125);	// Начинаем отсчитывать заново 125 тиков
}
