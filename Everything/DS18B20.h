/*
 *  - DS_Init				Инициализация\ресет датчика;
 *  - DS_WriteByte			Передача команды в датчик;
 *  - DS_ReadBit			Функция считывания бита из датчика;
 *  - DS_ReadByte			Функция считывания байта из датчика (использует функцию чтения бита в цикле);
 *  - DS_GetTemp			Функция определения и конвертации температуры;
 *  - DS_ConvertToInt		Функция ковертации температуры в переменную int;
 *  - DS_GetAsciiTemp		Функция занесения ascii значения температуры в глобальный массив temp[10];
 *
 *  Подключение датчика DS18b20:
 *   PD6 - пин данных (подтянут к питанию резистором 4.7кОм)
 */

#ifndef DS18B20_H_
#define DS18B20_H_

#define SKIP_ROM 0xCC // Пропустить индентификацию, если 1-wire устройство одно
#define CONVERT_T 0x44 // Измерить температуру
#define READ_SCRATCHPAD 0xBE // Прочитать из памяти
#define WRITE_SCRATCHPAD 0x4E // Записать в память

#define DS_PORT PORTD
#define DS_PIN 6
#define DS_DDR DDRD

extern uint16_t ds_refresh_period;	// Периодичность замеров температуры
extern uint8_t ds_convert_period;	// Длительность конвертации (зависит от битности значения температуры)
extern char AsciiTemp[10];			// Глобальная переменная для температуры в ASCII формате

int DS_Reset();
void DS_WriteByte(unsigned int byte);
char DS_ReadBit();
char DS_ReadByte();
void DS_MeasureTemp();
uint16_t DS_GetTemp();
void DS_GetAsciiTemp();

void DS_InitFSM();
void DS_ProcessFSM();

#endif
