#include <avr/pgmspace.h>
#include <util/delay.h>
#include "DS18B20.h"
#include "my_uart.h"
#include "something.h"
#include "Messages.h"
#include "menu.h"
#include "LCD.h"

uint8_t ROM_NO[8]; // ROM Bit
uint8_t LastDiscrepancy; // last discrepancy
uint8_t LastFamilyDiscrepancy; // last family discrepancy
uint8_t LastDeviceFlag; // Done flag
uint8_t crc8;

uint8_t First(){
	// Reset Search State
	LastDiscrepancy = 0;
	LastDeviceFlag = 0;
	LastFamilyDiscrepancy = 0;

	return DS_SearchROM();	// 1 - device found and ROM in buffer; 0 - device not found, end of search
}

uint8_t Next(){
	return DS_SearchROM();	// 1 - device found and ROM in buffer; 0 - device not found, end of search
}

uint8_t DS_SearchROM(){
	uint8_t id_bit_number;
	uint8_t last_zero;
	uint8_t rom_byte_number;
	uint8_t search_result;
	uint8_t id_bit, cmp_id_bit;
	uint8_t rom_byte_mask;
	uint8_t search_direction;

	// Init for search
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;
	crc8 = 0;

	if (!LastDeviceFlag){
		if (!DS_Reset()){	// If devices are present
			//Reset Search
			LastDiscrepancy = 0;
			LastDeviceFlag = 0;
			LastFamilyDiscrepancy = 0;
			return 0;
		}
		DS_WriteByte(SEARCH_ROM);

		do {
			id_bit = DS_ReadBit();
			cmp_id_bit = DS_ReadBit();

			// Check for no-device on 1-wire
			if ((id_bit == 1) && (cmp_id_bit == 1))
				break;
			else {
				// 10 or 01
				if (id_bit != cmp_id_bit)
					search_direction = id_bit;
				// 00 - discrepancy
				else {
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < LastDiscrepancy)
						search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
					else
						// if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == LastDiscrepancy);

					// if 0 was picked then record its position in LastZero
					if (search_direction == 0){
						last_zero = id_bit_number;

						// Check last discrepancy in family
						if (last_zero < 9)
							LastFamilyDiscrepancy = last_zero;
					}
				}
				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				// Здесь выбранный бит записывается в контейнер ROM по маске
				// там бегает единичка поочередно для каждого бита текущего байта
				// Если только единичка выйдет за пределы 8-байтной переменнной
				// значит байт найден и надо "объединичить" маску и перейти к следующему байту
				if (search_direction == 1)
					ROM_NO[rom_byte_number] |= rom_byte_mask;
				else
					ROM_NO[rom_byte_number] &= ~rom_byte_mask;

				DS_WriteBit(search_direction);	// Запись выбранного бита направления поиска
				id_bit_number++;
				rom_byte_mask <<= 1;

				// if mask is zero then go to next byte in ROM and reset mask
				if (rom_byte_mask == 0){
					docrc8(ROM_NO[rom_byte_number]); // Accumulate CRC
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		} while (rom_byte_number < 8); // делать пока все 8 байт не будут прочитаны

		if (!((id_bit_number < 65) || (crc8 != 0))){ // check crc for found ROM
			// search succesfull
			LastDiscrepancy = last_zero;

			// check for last device
			if (LastDiscrepancy == 0)
				LastDeviceFlag = 1;
			search_result = 1;
		}
	}
	// if no device found then reset counters so next 'search' will be like a first
	if (!search_result || !ROM_NO[0]){
		LastDiscrepancy = 0;
		LastDeviceFlag = 0;
		LastFamilyDiscrepancy = 0;
		search_result = 0;
	}

	return search_result;	// 1 - device found and ROM in buffer; 0 - device not found, end of search
}

void DS_SearchAllDevices(){
	uint8_t result = First();
//	UART_TxChar(result);

	while (result){
		for (int i=0;i<8; i++) {
			UART_TxChar(ROM_NO[i]);
			result = First();
		}
	}
//	SendBroadcastMessage(MSG_MENU_EXIT);
//	LCD_GotoXY(1,15);
//	LCD_WriteData(0x17); _delay_ms(500);
}

uint8_t docrc8(uint8_t x){
	crc8 = (char)pgm_read_byte( &(dscrc_table[crc8 ^ x]) );
	return crc8;
}



/*/// функция для отладки
void CheckCRC(){

//	char romm[8] = {0x3B, 0x00, 0x00, 0x04, 0x7B, 0xA2, 0x01, 0x28}; // Правильный ROM
//	char romm[8] = {0xA2, 0x00, 0x00, 0x00, 0x01, 0xB8, 0x1C, 0x02}; // Правильный ROM
	char romm[8] = {0x02, 0x1C, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xA2}; // неправильный ROM
	DS_CalcCRC(romm);
	if(!DS_CalcCRC(romm)) UART_TxStringFlash(PSTR("CRC OK!\n"));
	else UART_TxStringFlash(PSTR("CRC does not match!\n"));
}
/*/////////////////
