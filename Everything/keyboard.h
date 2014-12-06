#ifndef KEYBOARD_H_
#define KETBOARD_H_

#define DEBOUNCE 5	// 5 * 4мс = 20мс, антидребезг
#define FIRST_REPEAT_DELAY 125 // 125 * 4мс = 500мс, задержка перед первым повтором если кнопка не отпущена
#define REPEAT_DELAY 25 // 25 * 4мс = 100мс, частота повторений при удержании кнопки

#define KEY_0		0
#define KEY_1 		1
#define KEY_2 		2
#define KEY_3 		3
#define KEY_4 		4
#define KEY_1_2		5
#define KEY_1_3		6
#define KEY_1_4		7
#define KEY_2_3		8
#define KEY_2_4		9
#define KEY_3_4		10
#define KEY_1_2_3	11


uint8_t Keyb_Scan();
void Keyb_Action();
uint8_t Keyb_GetScancode();
void Keyb_InitFSM();
void Keyb_ProcessFSM();

#endif
