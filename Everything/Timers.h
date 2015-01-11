/*
 * Timers.h
 *
 *  Created on: Nov 28, 2014
 *      Author: kottofey
 */
#include <avr/io.h>

#ifndef TIMERS_H_
#define TIMERS_H_

#define MAX_TIMERS 10
extern uint16_t Timers[MAX_TIMERS]; // Контейнер для таймеров (16 бит хватит ровно на 4 минуты, если один период системного таймера 4 мс)
extern uint8_t TimersState[MAX_TIMERS];	// Контейнер для состояния таймеров: запущен/остановлен/на паузе

// Определение длительности секунды. Таймер переполнится через 130 тиков
// Справедливо для частоты процессора 8МГц, предделителя таймера 1/256. Один период(переполнение) таймера длится 4мс.
// 1 сек длится 250 периодов.
// Для других частот и предделителей надо пересчитать период таймера и сколько переполнений случается за одну секунду.
#define sec 250
#define min 60*sec
#define hour 60*min
#define day 24*hour

// Перечисление таймеров
#define TIMER_SEC 0
#define TIMER_MIN 1
#define TIMER_HOUR 2
#define TIMER_DAY 3
#define TIMER_TICK 4
#define TIMER_KEYB 5
#define TIMER_TEMP_CONVERT 6

// Состояния для таймеров
#define TIMER_STOPPED 0
#define TIMER_RUNNING 1
#define TIMER_PAUSED 2

void InitTimers();
uint16_t GetTimer(uint8_t Timer);
void ResetTimer(uint8_t Timer);
void StartTimer(uint8_t Timer);
void StopTimer(uint8_t Timer);
void PauseTimer(uint8_t Timer);
void ResumeTimer(uint8_t Timer);
void ProcessTimers();

#endif /* TIMERS_H_ */
