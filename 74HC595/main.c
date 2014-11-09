/*
 * 		Закидываем в регистр 74HC595 цифры для вывода на 7-ми сегментные индикаторы.
 * 		Каждый такой индикатор подключен к своему регистру.
 * 		Регистры соединены последовательно, то есть выход Q7' предыдущего подключен
 * 		ко входу DS следующего. Выходы Q[0..7] подключены к анодам индикатора по порядку
 * 		(Q0 -> A, Q1 -> B, Q2 -> C...) Reset посажен на +5V, вывод OE посажен на землю.
 *
 * 		Есть проверка на НЕ-цифры в аргументе функции. Если там НЕ-цифра, то выводится знак "-"
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


//	Config
#define DATA_PORT PORTB
#define DATA_PIN 2
#define DATA_DDR DDRB

#define SHIFT_PORT PORTB
#define SHIFT_PIN 3
#define SHIFT_DDR DDRB

#define STORE_PORT PORTB
#define STORE_PIN 0
#define STORE_DDR DDRB

#define HC595_QTY 3			// Сколько регистров подключено
//	Config end


#define data_high() DATA_PORT|=_BV(DATA_PIN);
#define data_low() DATA_PORT&=~_BV(DATA_PIN);
#define shift() {SHIFT_PORT |= _BV(SHIFT_PIN); SHIFT_PORT &=~_BV(SHIFT_PIN);}
#define store() {STORE_PORT |= _BV(STORE_PIN); STORE_PORT &=~_BV(STORE_PIN);}

const volatile char digits[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
volatile unsigned int edgecount = 0, count = 0;

int HC595_WriteAscii(char *i, uint8_t qty);
void int_to_str(int i, char * str);

ISR (INT0_vect) {
	edgecount++;		// счетчик дребезга
}

ISR (TIM0_OVF_vect){	// будет тикать 1.2млн раз в секунду с prescaler 8 при частоте 9.6МГц
	TCNT0 = 6;			// а переполняться будет 1200000 / (256-6) = 4800 раз в секунду, начинаем считать с шести
	count++;			// Сколько раз переполнился счетчик
}


int main(){

//	Инициализация регистра(-ов)
	DATA_DDR |= 1 << DATA_PIN; DATA_PORT = 0;
	SHIFT_DDR |= 1 << SHIFT_PIN; SHIFT_PORT = 0;
	STORE_DDR |= 1 << STORE_PIN; STORE_PORT = 0;

//	Инициализация прерываний
	MCUCR = 0b00000010;		// Прерывание на INT0 по падающему фронту
	GIMSK |= (1 << INT0);	// разрешаем внешнее прерывание на ножке INT0

//	Инициализация таймера
	TCCR0B = 0b00000010;	// предделитель таймера 8
	TIMSK0 |= 1 << TOIE0; 	// разрешение прерывания по переполнению
	GTCCR |= 1 << PSR10;	// ресет предделителя таймера

//	sei();					// глобальное разрешение прерываний

	char str[3] = "000";
	int i = 123;

	int_to_str(i, str);
	HC595_WriteAscii(str, 3);

//	HC595_WriteAscii("123", HC595_QTY);
	while (1) {
		if (count == 4800){
//			int_to_str(i, str);
//			HC595_WriteAscii(str, 3);
			count = 0;
			edgecount = 0;
		}
	}

return 0;
}


int HC595_WriteAscii(char *i, uint8_t qty){
// Надо добавить проверку на соответствие входных данных и количества подключенных регистров

	i += (qty-1);	// Начинаем с конца и проталкиваем его (конец) вперед. В итоге конец окажется в конце!

	while (qty--){		// цикл на количество подключенных регистров. qty - оно же HC595_QTY задается дефайном в настройках

		uint8_t counter = 8;					// счетчик битов
		uint8_t byte_place = *i - 0x30;			// переводим цифру в номер ячейки массива с цифрами
		char lcd_digit = digits[byte_place];	// Присваиваем переменной значение соответствующего байта массива с цифрами

		if ( ((byte_place+0x30) < 0x30) || ((byte_place+0x30) > 0x39) ){	// проверка на НЕ-цифры
			lcd_digit = 0x40;	// Если в массиве таки попалась НЕ-цифра, то выводится знак минуса
		}

		while(counter--){		// Цикл на запихивание байта в текущий регистр

			if (lcd_digit & 0x80){	// Определяем старший бит и дергаем ножкой DS
				data_high();		// если он 1
			}
			else{
				data_low();			// если он 0
			}
			shift();				// сдвигаем регистр
			lcd_digit <<= 1;		// сдвигаем байт влево чтобы подготовить следующий бит
		}
		i = i - 1;					// Переходим к следующей цифре во входном массиве
	}
	store();
	return 0;
}

void int_to_str(int i, char *str){
	*str = (i % 100) + 0x30; str++;
//	*str = ( (i - (i%100)*100 ) % 10) + 0x30; str++;
	*str = ( i - ( (i % 10) * 10) ) + 0x30;
}
