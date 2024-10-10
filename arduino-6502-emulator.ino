// ToDo: Split into multiple parts
// ToDo: Split debug and no debug code
// ToDo: Precise performance test

// ---------------------- user defined ----------------------
//#define ROM_START	    0xFFEE		// load porta to portb
//#define ROM_START	    0xFFE0		// led runner with interrupt and disable
#define ROM_START	    0xFEB1		// lcd display test
//#define ROM_START	    0xFFCC		// timer 1 one shot test on A5
//#define ROM_START	    0xFFA0		// timer 1 free run test
//#define ROM_START	    0xFFDB		// A5 led test with D2 button

#define RAM_END         0x0220

#define CLOCK_SPEED     1000000

// 1: print reg, 2: print flags, 4: print ram, 8: print instructions texts, 16: wait for enter, 32: print instuction name, 64: print instuction number
// 128: commands throug Serial
//#define DEBUG	        ( 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 )
//#define DEBUG	        ( 16 | 32 | 64 | 128 )
//#define DEBUG	        ( 32 | 64 | 128 )
//#define DEBUG	        ( 128 )

//#define USE_FILE
//#define USE_SCRIPT

//#define IRQB_LOW	    // interrupt continously on GND

//#define VIA_SUPPORT     ( 1 | 2 | 4 )        // 1: port support, 2: timer1 support, 4: extras
#define VIA_SUPPORT     ( 1 | 2 )        // 1: port support, 2: timer1 support, 4: extras

// ---------------------- header ----------------------
#include <assert.h>
#include <avr/sleep.h>
#define VALUE(x) TO_VALUE(x)
#define TO_VALUE(x) #x

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef VIA_SUPPORT
#define VIA_SUPPORT 0
#endif

#ifndef FAST_IO
#define FAST_IO 0
#endif

// ---------------------- bit defines ----------------------

#define BIT(nth_bit)                    (1U << (nth_bit)) //bit(nth_bit)
#define CHECK_BIT(data, bit)            ((data) & BIT(bit)) //bitRead(data, bit)
//#define CHECK_BIT(data, bit)            ((data) & (1UL << (bit))) //bitRead(data, bit)
#define SET_BIT_TO(data, bit, to)       ((data) = (((data) & (~BIT(bit)))) | (((to) ? 1 : 0) << (bit))) //bitWrite(data, bit, to)
//#define SET_BIT_TO(data, bit, to)       ((data) = (((data) & (~(1UL << (bit))))) | ((to) << (bit))) //bitWrite(data, bit, to)
#define SET_BIT(data, bit)              ((data) |= BIT(bit)) //bitClear(data, bit)
#define CLEAR_BIT(data, bit)            ((data) &= ~BIT(bit)) //bitSet(data, bit)
#define CHANGE_BIT(data, bit)           ((data) ^= BIT(bit)) //bitToggle(data, bit)

typedef uint8_t uint4_t;

#define LOW_NIBBLE(data)                (static_cast<uint4_t>((data) & 0x0f))
#define HIGH_NIBBLE(data)               (static_cast<uint4_t>((data) >> 4))
#define LOW_BYTE(data)                  (static_cast<uint8_t>((data) & 0xff))
#define HIGH_BYTE(data)                 (static_cast<uint8_t>((data) >> 8))

#define ADD_BYTE(a, b)                  (static_cast<uint8_t>((a) + (b)))
#define SUB_BYTE(a, b)                  (static_cast<uint8_t>((a) - (b)))

// ---------------------- data defines ----------------------

#define RAM_START             0x0000
#define ROM_END               0xFFFF
#define STACK_START           0x0100
#define ROM_LENGTH            (ROM_END - ROM_START)
#define RAM_LENGTH            (RAM_END - RAM_START)

#define _ROM_START            0x0000                    // real rom start address
#define _ROM_END              (ROM_LENGTH - 1)          // real rom end address
#define _RAM_END              (RAM_LENGTH - 1)          // real ram end address
#define _STACK_END            0xff                      // real stack end address

#define NMIB                  (ROM_END - 1 - 5)         // 6th and 5th last rom byte
#define RESB                  (NMIB + 2)                // 4th and 3rd last rom byte
#define BRK_IRQB              (RESB + 2)                // 2nd and 1st last rom byte

#define MY_PORTA              (RAM_START + 0x0200)      // port A I/O pins
#define MY_PORTB              (RAM_START + 0x0201)      // port B I/O pins
#define MY_DDRB               (RAM_START + 0x0202)      // port B I/O select
#define MY_DDRA               (RAM_START + 0x0203)      // port A I/O select

#define MY_T1CL               (RAM_START + 0x0204)      // timer 1 clock low byte
#define MY_T1CH               (RAM_START + 0x0205)      // timer 1 clock high byte
#define MY_T1LL               (RAM_START + 0x0206)      // timer 1 latch low byte
#define MY_T1LH               (RAM_START + 0x0207)      // timer 1 latch high byte

#define MY_T2CL               (RAM_START + 0x0208)      // timer 1 clock low byte
#define MY_T2CH               (RAM_START + 0x0209)      // timer 1 clock high byte

#define MY_ACR                (RAM_START + 0x020B)      // auxiliary control register
#define MY_IFR                (RAM_START + 0x020D)      // interrupt flag register
#define MY_IER                (RAM_START + 0x020E)      // interrupt enable register

#define MY_SR                 (RAM_START + 0x020A)      // shift register
#define MY_PCR                (RAM_START + 0x020C)      // 
#define MY_PORTA2             (RAM_START + 0x020F)      // no handsake port A I/O pins

#define MY_PULLUPA            (RAM_START + 0x0210)      // port A pins INPUT/INPUT_PULLUP mode
#define MY_PULLUPB            (RAM_START + 0x0211)      // port B pins INPUT/INPUT_PULLUP mode

#define MY_PA0                A0
#define MY_PA1                A1
#define MY_PA2                A2
#define MY_PA3                A3
#define MY_PA4                A4
#define MY_PA5                A5
#define MY_PA6                3
#define MY_PA7                4

#define MY_PB0                5
#define MY_PB1                6
#define MY_PB2                7
#define MY_PB3                8
#define MY_PB4                9
#define MY_PB5                10
#define MY_PB6                11
#define MY_PB7                12

const PROGMEM uint8_t pin_a[] = { MY_PA0, MY_PA1, MY_PA2, MY_PA3, MY_PA4, MY_PA5, MY_PA6, MY_PA7 };
const PROGMEM uint8_t pin_b[] = { MY_PB0, MY_PB1, MY_PB2, MY_PB3, MY_PB4, MY_PB5, MY_PB6, MY_PB7 };

#define WORD_SIZE             2
#define VECTOR_BYTE_COUNT     (3 * WORD_SIZE)

static_assert(!CHECK_BIT(VIA_SUPPORT, 2) || (CHECK_BIT(VIA_SUPPORT, 1) && CHECK_BIT(VIA_SUPPORT, 0)), "VIA \"extras\" needs \"port\" and \"timer1\" support too");

#if VIA_SUPPORT == ( 1 | 2 | 4 )
	#define IO_BYTE_COUNT         16           // 2x 8 pin, 2x 8 pin mode pin, 2x T1C, 2x T1L, 2x T2C, ACR, IFR, IER, SR, PCR, (PORTA2)
#elif VIA_SUPPORT == ( 1 | 2 )
	#define IO_BYTE_COUNT         11           // 2x 8 pin, 2x 8 pin mode pin, 2x T1C, 2x T1L, ACR, IFR, IER
#elif VIA_SUPPORT == 2
	#define IO_BYTE_COUNT         7            // 2x T1C, 2x T1L, ACR, IFR, IER
#elif VIA_SUPPORT == 1
	#define IO_BYTE_COUNT         4            // 2x 8 pin, 2x 8 pin mode pin
#else
	#define IO_BYTE_COUNT         0            // no VIA
#endif

static_assert(RAM_LENGTH >= STACK_START + _STACK_END + IO_BYTE_COUNT, "Not enough RAM for STACK, ZP and IO");
static_assert(ROM_START > RAM_END, "ROM is inside of RAM");
static_assert(ROM_LENGTH >= VECTOR_BYTE_COUNT, "ROM doesn't has enough bytes to store NMIB, RESB, IRQB vectors");

// ToDo different ram sizes for different devices
#define RAM_MAX_SIZE 1900

static_assert(RAM_LENGTH <= RAM_MAX_SIZE, "Don't have enough memory for local variables");

// ---------------------- ram & rom setup ----------------------
uint8_t RAM[RAM_LENGTH];

#if CHECK_BIT(VIA_SUPPORT, 2)
uint8_t RAM_T2LL;
#endif

#if CHECK_BIT(DEBUG, 7)
	bool run = false;
#endif

#if defined(USE_FILE)
	#define INCBIN_PREFIX
	#define INCBIN_STYLE INCBIN_STYLE_SNAKE
	#include <incbin.h>
	#undef INCBIN_PTR_TYPE
	#define ALIGN_MINE 1
	#define INCBIN_PTR_TYPE(NAME, FILENAME, DATA_PTR_TYPE) \
	__asm__(INCBIN_SECTION \
			INCBIN_GLOBAL_LABELS(NAME, DATA) \
			".align " INCBIN_STRINGIZE(ALIGN_MINE) "\n" \
			INCBIN_MANGLE INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME INCBIN_STYLE_STRING(DATA) ":\n" \
			INCBIN_MACRO " \"" FILENAME "\"\n" \
			INCBIN_GLOBAL_LABELS(NAME, END) \
			INCBIN_ALIGN_BYTE \
			INCBIN_MANGLE INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME INCBIN_STYLE_STRING(END) ":\n" \
			INCBIN_APPEND_AFTER_DATA \
				INCBIN_BYTE "1\n" \
			".text\n" \
	); \
	INCBIN_EXTERN_PTR_TYPE(NAME, DATA_PTR_TYPE)
	INCBIN(ROM, "data.bin"); // ROM_data, ROM_end
#elif defined(USE_SCRIPT)
	//bin file
	const PROGMEM uint8_t ROM_data[ROM_LENGTH] = { };
	static_assert(sizeof(ROM_data)/sizeof(ROM_data[0]) == ROM_LENGTH, "ROM has less bytes than ROM capacity");
#else
	/*const PROGMEM uint8_t ROM_data[] = {	0xCE, 0x02, 0x02, 0xAD, 0x00, 0x02, 0x8D, 0x01, 0x02, 0x50, 0xF8,             // load porta to portb
											0x00, 0x00, LOW_BYTE(ROM_START),HIGH_BYTE(ROM_START), 0x00, 0x00 };*/
	/*const PROGMEM uint8_t ROM_data[] = {	0xCE, 0x03, 0x02, 0x1A, 0x8D, 0x00, 0x02, 0x2A, 0xD0, 0x01, 0x2A, 0xC8,       // led runner porta
											0xF0, 0xF6, 0xD0, 0xFB, 0xA9, 0xFF, 0x8D, 0x00, 0x02, 0xE8, 0xD0, 0xFD,
											0x40,
											0x00, 0x00, LOW_BYTE(ROM_START),HIGH_BYTE(ROM_START), 0xF0, 0xFF };*/
	const PROGMEM uint8_t ROM_data[] = {	0xCE, 0x03, 0x02, 0xCE, 0x02, 0x02, 0x78, 0xA9, 0x38, 0x20, 0x24, 0xFF,     // lcd test
											0xA9, 0x06, 0x20, 0x24, 0xFF, 0xA9, 0x40, 0x20, 0x24, 0xFF, 0xBD, 0x87,
											0xFF, 0x30, 0x06, 0x20, 0x36, 0xFF, 0xE8, 0x80, 0xF5, 0xA2, 0x00, 0xA9,
											0x0E, 0x20, 0x24, 0xFF, 0xA9, 0x01, 0x20, 0x24, 0xFF, 0xBD, 0x4E, 0xFF,
											0xF0, 0x06, 0x20, 0x36, 0xFF, 0xE8, 0x80, 0xF5, 0xA9, 0x0C, 0x20, 0x24,
											0xFF, 0xA2, 0x00, 0x58, 0xC0, 0x01, 0xD0, 0xFC, 0x78, 0xA9, 0x01, 0x20,
											0x24, 0xFF, 0xBD, 0xC0, 0xFF, 0xF0, 0x06, 0x20, 0x36, 0xFF, 0xE8, 0x80,
											0xF5, 0xDB, 0x48, 0xEE, 0x02, 0x02, 0xA9, 0x02, 0x8D, 0x00, 0x02, 0xA9,
											0x06, 0x8D, 0x00, 0x02, 0x2C, 0x01, 0x02, 0x30, 0xF1, 0xA9, 0x02, 0x8D,
											0x00, 0x02, 0xCE, 0x02, 0x02, 0x68, 0x60, 0x20, 0x07, 0xFF, 0x8D, 0x01,
											0x02, 0x9C, 0x00, 0x02, 0xA9, 0x04, 0x8D, 0x00, 0x02, 0x9C, 0x00, 0x02,
											0x60, 0x20, 0x07, 0xFF, 0x8D, 0x01, 0x02, 0xA9, 0x01, 0x8D, 0x00, 0x02,
											0xA9, 0x05, 0x8D, 0x00, 0x02, 0xA9, 0x01, 0x8D, 0x00, 0x02, 0x60, 0xC8,
											0x40, 0x20, 0x20, 0x20, 0x4C, 0x03, 0x67, 0x79, 0x73, 0x7A, 0x04, 0x76,
											0x65, 0x73, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
											0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
											0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x68, 0x06, 0x7A,
											0x7A, 0x20, 0x6B, 0x69, 0x21, 0x20, 0x20, 0x20, 0x20, 0x00, 0x05, 0x0A,
											0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00, 0x05, 0x0A, 0x11, 0x11, 0x11, 0x13,
											0x0D, 0x00, 0x02, 0x04, 0x0E, 0x01, 0x0F, 0x11, 0x0F, 0x00, 0x02, 0x04,
											0x0E, 0x11, 0x1F, 0x10, 0x0E, 0x00, 0x02, 0x04, 0x00, 0x0C, 0x04, 0x04,
											0x0E, 0x00, 0x02, 0x04, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00, 0x02, 0x04,
											0x11, 0x11, 0x11, 0x13, 0x0D, 0x00, 0xFF, 0x20, 0x20, 0x4E, 0x45, 0x20,
											0x50, 0x49, 0x53, 0x5A, 0x4B, 0x02, 0x4C, 0x44, 0x20, 0x20, 0x20, 0x20,
											0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
											0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x41,
											0x5A, 0x54, 0x20, 0x41, 0x20, 0x47, 0x4F, 0x4D, 0x42, 0x4F, 0x54, 0x21,
											0x21, 0x21, 0x21, 0x00,
											0x00, 0x00, LOW_BYTE(ROM_START),HIGH_BYTE(ROM_START), 0x4C, 0xFF };
	/*const PROGMEM uint8_t ROM_data[] = {	0xCE, 0x03, 0x02, 0x9C, 0x00, 0x02, 0x9C, 0x0B, 0x02, 0xA2, 0x20, 0x8E,       // timer 1 one shot test
											0x00, 0x02, 0x20, 0xE5, 0xFF, 0x9C, 0x00, 0x02, 0x20, 0xE5, 0xFF, 0x80,
											0xF2, 0xA9, 0x50, 0x8D, 0x04, 0x02, 0xA9, 0xC3, 0x8D, 0x05, 0x02, 0x2C,
											0x0D, 0x02, 0x50, 0xFB, 0xAD, 0x04, 0x02, 0x60, 0x40,
											0x00, 0x00, LOW_BYTE(ROM_START),HIGH_BYTE(ROM_START), 0xF8, 0xFF };*/
	/*const PROGMEM uint8_t ROM_data[] = {	0xCE, 0x02, 0x02, 0x9C, 0x01, 0x02, 0x9C, 0x0B, 0x02, 0x64, 0x04, 0x20,       // timer 1 free run test
											0xC9, 0xFF, 0x20, 0xB3, 0xFF, 0x80, 0xFB, 0x38, 0xA5, 0x00, 0xE5, 0x04,
											0xC9, 0x19, 0x90, 0x0C, 0xA9, 0x01, 0x4D, 0x01, 0x02, 0x8D, 0x01, 0x02,
											0xA5, 0x00, 0x85, 0x04, 0x60, 0x64, 0x00, 0x64, 0x01, 0x64, 0x02, 0x64,
											0x03, 0xA9, 0x40, 0x8D, 0x0B, 0x02, 0xA9, 0x00, 0x8D, 0x04, 0x02, 0xA9,
											0x01, 0x8D, 0x05, 0x02, 0xA9, 0xC0, 0x8D, 0x0E, 0x02, 0x58, 0x60, 0x2C,
											0x04, 0x02, 0xE6, 0x00, 0xD0, 0x0A, 0xE6, 0x01, 0xD0, 0x06, 0xE6, 0x02,
											0xD0, 0x02, 0xE6, 0x03, 0x40,
											0x00, 0x00, LOW_BYTE(ROM_START),HIGH_BYTE(ROM_START), 0xE7, 0xFF };*/
	/*const PROGMEM uint8_t ROM_data[] = {	0xCE, 0x03, 0x02, 0xCE, 0x02, 0x02, 0x78, 0xAD, 0x02, 0x02, 0x29, 0xFE,       // A5 led test with D2 button
											0x8D, 0x02, 0x02, 0xAD, 0x01, 0x02, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x8D,
											0x00, 0x02, 0x80, 0xF3, 0xC8, 0x40,
											0x00, 0x00, LOW_BYTE(ROM_START),HIGH_BYTE(ROM_START), 0xF7, 0xFF };*/
	static_assert(sizeof(ROM_data) <= ROM_LENGTH, "ROM has less bytes than ROM capacity");
#endif

#define RELATIVE_ROM_ADDRESS(address) ((address) - ROM_START)
#define ABSOLUTE_ROM_ADDRESS(address) ((address) + ROM_START)

// ---------------------- register setup ----------------------

enum Flags
{
	No         = 0 << 0,
	Carry      = 1 << 0,
	Zero       = 1 << 1,
	Interrupt  = 1 << 2,
	Decimal    = 1 << 3,
	Break      = 1 << 4,
	Unused     = 1 << 5,
	Overflow   = 1 << 6,
	Negative   = 1 << 7,
};

enum FlagBits
{
	CarryBit,
	ZeroBit,
	InterruptBit,
	DecimalBit,
	BreakBit,
	UnusedBit,
	OverflowBit,
	NegativeBit,
};

uint8_t a, x, y;
uint8_t s = 0xff; // set stack pointer to last byte
uint8_t p = Unused | Break;
uint16_t pc = RESB;

uint8_t op_code;

// ---------------------- debug functions ----------------------
#if DEBUG != 0
void print_hex_byte(uint8_t num, bool print_zero = true)
{
	uint4_t numH = HIGH_NIBBLE(num);
	uint4_t numL = LOW_NIBBLE(num);
	if(print_zero) Serial.print(F("0x"));
	Serial.print(static_cast<char>(numH + ((numH >= 10) ? 'A' - 10 : '0')));
	Serial.print(static_cast<char>(numL + ((numL >= 10) ? 'A' - 10 : '0')));
}
void print_hex_word(uint16_t num, bool print_zero = true)
{
	print_hex_byte(HIGH_BYTE(num), print_zero);
	print_hex_byte(LOW_BYTE(num), false);
}

void print_bin(uint8_t num, bool print_zero = true)
{
	if(print_zero) Serial.print(F("0b"));
	for(int8_t i = 7; 0 <= i; --i)
	{
		Serial.print(CHECK_BIT(num, i) > 0);
	}
}

void print_ram()
{
	Serial.print(F("Offset(h) "));
	for(uint8_t i = 0; i < 32; ++i)
	{
		print_hex_byte(i, false);
		Serial.print(F("  "));
	}
	for(uint16_t y2 = 0, n = RAM_LENGTH / 32; y2 <= n; ++y2)
	{
		if(y2 == n && RAM_LENGTH % 32 == 0) break;
		Serial.print(F("\n0000"));
		uint16_t y_address = y2 * 32;
		print_hex_word(y_address, false);
		for(uint8_t x2 = 0; x2 < 32; ++x2)
		{
			uint16_t address = x2 + y_address;
			if(address >= RAM_LENGTH) break;
			
			Serial.print(F("  "));
			print_hex_byte(RAM[address], false);
		}
	}
	Serial.println();
}

void print_rom()
{
	Serial.print(F("Offset(h) "));
	for(uint8_t i = 0; i < 32; ++i)
	{
		print_hex_byte(i, false);
		Serial.print(F("  "));
	}
	for(uint16_t y2 = 0, n = ROM_LENGTH / 32; y2 <= n; ++y2)
	{
		if(y2 == n && ROM_LENGTH % 32 == 0) break;
		Serial.print(F("\n0000"));
		uint16_t y_address = y2 * 32;
		print_hex_word(y_address, false);
		for(uint8_t x2 = 0; x2 < 32; ++x2)
		{
			uint16_t address = x2 + y_address;
			if(address >= ROM_LENGTH) break;
			
			Serial.print(F("  "));
			print_hex_byte(pgm_read_byte(ROM_data + address), false);
		}
	}
	Serial.println();
}

void print_reg()
{
	Serial.print(F("a: "));
	print_hex_byte(a);
	Serial.print(F(" x: "));
	print_hex_byte(x);
	Serial.print(F(" y: "));
	print_hex_byte(y);
	Serial.print(F(" s: "));
	print_hex_byte(s);
	Serial.print(F(" pc: "));
	print_hex_word(pc);
	Serial.println();
}

void print_status()
{
	Serial.println(F("N V - B D I Z C"));
	for(int8_t i = 7; 0 <= i; --i)
	{
		Serial.print(CHECK_BIT(p, i) > 0);
		Serial.print(F(" "));
	}
	Serial.print(F("- "));
	print_hex_byte(p);
	Serial.println();
}
#endif

// ---------------------- ram & rom functions ----------------------
uint16_t tmp_word;
uint8_t tmp_byte;

static inline bool is_ram(uint16_t address = tmp_word) { return (address <= RAM_END); }
static inline bool is_rom(uint16_t address = tmp_word) { return (address >= ROM_START); }

#if CHECK_BIT(VIA_SUPPORT, 1)

enum EventFlagBits{
	T1_StartBit,
	T2_StartBit,
};

enum IFRBits{
	IFR_CA2_Bit,
	IFR_CA1_Bit,
	IFR_SR_Bit,
	IFR_CB2_Bit,
	IFR_CB1_Bit,
	IFR_T2_Bit,
	IFR_T1_Bit,
	IFR_IRQ_Bit
};

enum IERBits{
	IER_CA2_Bit,
	IER_CA1_Bit,
	IER_SR_Bit,
	IER_CB2_Bit,
	IER_CB1_Bit,
	IER_T2_Bit,
	IER_T1_Bit,
	IER_SetClear_Bit
};

enum ACRBits{
	ACR_PA_Bit,
	ACR_PB_Bit,
	ACR_SRC1_Bit,
	ACR_SRC2_Bit,
	ACR_SRC3_Bit,
	ACR_T2C_Bit,
	ACR_T1C1_Bit,
	ACR_T1C2_Bit
};

uint8_t eventFlags = 0;

static inline uint16_t load_ram_word_dev(uint16_t address)                  { return (RAM[address + 1] << 8) | RAM[address]; }
static inline void     write_ram_word_dev(uint16_t address, uint16_t data)  { RAM[address + 1] = HIGH_BYTE(data); RAM[address] = LOW_BYTE(data); }

static inline bool ier_check(uint8_t bit) { return CHECK_BIT(RAM[MY_IER], bit) ? 1 : 0; }

static inline bool t1_is_zero() { return load_ram_word_dev(MY_T1CL) == 0; }
static inline void t1_end_start()
{
	if(t1_is_zero() && CHECK_BIT(eventFlags, T1_StartBit))
	{
		CLEAR_BIT(RAM[MY_IFR], IFR_T1_Bit);
		CLEAR_BIT(eventFlags, T1_StartBit);
	}
}

static inline void t2_end_start()
{
	if(CHECK_BIT(RAM[MY_IFR], IFR_T2_Bit) && CHECK_BIT(eventFlags, T2_StartBit))
	{
		CLEAR_BIT(RAM[MY_IFR], IFR_T2_Bit);
		CLEAR_BIT(eventFlags, T2_StartBit);
	}
}
#endif

#if CHECK_BIT(VIA_SUPPORT, 0)
static void WRITE_PORTS(bool is_pina)
{
	if(is_pina)
	{
		uint8_t port = RAM[MY_PORTA];
		uint8_t ddr = RAM[MY_DDRA];
		// PORTC A0 - A5
		// PORTD D3 - D4
		uint8_t d = PORTD;
		uint8_t c = PORTC;
		
		const uint8_t c_mask = 0b11000000;
		const uint8_t d_mask = 0b11100111;
		
		c &= c_mask;
		c |= ((ddr & port) >> 0) & ~c_mask;     // A0-A5 -> A0-A5
		
		d &= d_mask;
		d |= ((ddr & port) >> 3) & ~d_mask;     // 6-7 -> 3-4

		PORTD = d;
		PORTC = c;
	}
	else
	{
		uint8_t port = RAM[MY_PORTB];
		uint8_t ddr = RAM[MY_DDRB];
		// PORTD D5 - D7
		// PORTB D8 - D12
		uint8_t d = PORTD;
		uint8_t b = PORTB;
		
		const uint8_t b_mask = 0b11100000;
		const uint8_t d_mask = 0b00011111;
		
		d &= d_mask;
		d |= ((ddr & port) << 5) & ~d_mask;     // 0-2 -> 5-7
	
		b &= b_mask;
		b |= ((ddr & port) >> 3) & ~b_mask;     // 3-7 -> 0-4
		
		PORTD = d;
		PORTB = b;
	}
}

static void READ_PORTS(bool is_pina)
{
	if(is_pina)
	{
		uint8_t* port = &RAM[MY_PORTA];
		uint8_t ddr = RAM[MY_DDRA];
		// PORTC A0 - A5
		// PORTD D3 - D4
		uint8_t d = PIND;
		uint8_t c = PINC;
		
		const uint8_t c_mask = 0b00111111;
		const uint8_t d_mask = 0b00011000;
		
		*port = ((c & c_mask) >> 0) & ~ddr;     // 0-5 -> 0-5
		*port |= ((d & d_mask) >> 3) & ~ddr;    // 3-4 -> 6-7
	}
	else
	{
		uint8_t* port = &RAM[MY_PORTB];
		uint8_t ddr = RAM[MY_DDRB];
		// PORTD D5 - D7
		// PORTB D8 - D12
		uint8_t b = PINB;
		uint8_t d = PIND;
		
		const uint8_t b_mask = 0b00011111;
		const uint8_t d_mask = 0b11100000;
		
		*port = ((b & b_mask) << 3) & ~ddr;     // 0-4 -> 3-7
		*port |= ((d & d_mask) >> 5) & ~ddr;    // 5-7 -> 0-2
	}
}

static void DDR_PORTS(bool is_pina)
{
	if(is_pina)
	{
		uint8_t pullup = RAM[MY_PULLUPA];
		uint8_t ddr = RAM[MY_DDRA];
		// PORTC A0 - A5
		// PORTD D3 - D4
		
		uint8_t d = DDRD, pd = PORTD;
		uint8_t c = DDRC, pc = PORTC;
		
		const uint8_t c_mask = 0b11000000;
		const uint8_t d_mask = 0b11100111;
		
		c &= c_mask;
		c |= (ddr >> 0) & ~c_mask;              // A0-A5 -> A0-A5
		
		d &= d_mask;
		d |= (ddr >> 3) & ~d_mask;              // 6-7 -> 3-4

		pc |= (pullup >> 0) & ~c_mask;          // A0-A5 -> A0-A5
		pd |= (pullup >> 3) & ~d_mask;          // 6-7 -> 3-4

		DDRD = d;
		DDRC = c;
		PORTD = pd;
		PORTC = pc;
	}
	else
	{
		uint8_t pullup = RAM[MY_PULLUPB];
		uint8_t ddr = RAM[MY_DDRB];
		// PORTD D5 - D7
		// PORTB D8 - D12
		uint8_t d = DDRD, pd = PORTD;
		uint8_t b = DDRB, pb = PORTB;
		
		const uint8_t b_mask = 0b11100000;
		const uint8_t d_mask = 0b00011111;
		
		d &= d_mask;
		d |= (ddr << 5) & ~d_mask;              // 0-2 -> 5-7

		b &= b_mask;
		b |= (ddr >> 3) & ~b_mask;              // 3-7 -> 0-4
		
		pb |= (pullup >> 0) & ~b_mask;          // A0-A5 -> A0-A5
		pd |= (pullup >> 3) & ~d_mask;          // 6-7 -> 3-4
		
		DDRD = d;
		DDRB = b;
		PORTD = pd;
		PORTB = pb;
	}
}
#endif

void events_read(uint16_t address)
{
	switch(address)
	{
#if CHECK_BIT(VIA_SUPPORT, 1)
	case MY_T1CL:
		t1_end_start();
		break;
#if CHECK_BIT(VIA_SUPPORT, 2)
	case MY_T2CL:
		t2_end_start();
		break;
#endif
#endif
#if CHECK_BIT(VIA_SUPPORT, 0)
	case MY_PORTA:
		READ_PORTS(true);
		break;
	case MY_PORTB:
		READ_PORTS(false);
		break;
#endif
	}
}

void events_write(uint16_t address)
{
	switch(address)
	{
#if CHECK_BIT(VIA_SUPPORT, 1)
	case MY_T1CL:
		if(!CHECK_BIT(eventFlags, T1_StartBit)) RAM[MY_T1LL] = RAM[MY_T1CL];
		break;
	case MY_T1CH:
		if(!CHECK_BIT(eventFlags, T1_StartBit)) 
		{
			SET_BIT(eventFlags, T1_StartBit);
			RAM[MY_T1CL] = RAM[MY_T1LL];
			RAM[MY_T1LH] = RAM[MY_T1CH];
		}
		break;
	case MY_T1LH:
		t1_end_start();
		break;
#if CHECK_BIT(VIA_SUPPORT, 2)
	case MY_T2CL:
		if(!CHECK_BIT(eventFlags, T2_StartBit)) RAM_T2LL = RAM[MY_T2CL];
		break;
	case MY_T2CH:
		CLEAR_BIT(RAM[MY_IFR], IFR_T2_Bit);
		SET_BIT(eventFlags, T2_StartBit);
		RAM[MY_T2CL] = RAM_T2LL;
		break;
#endif
#endif
#if CHECK_BIT(VIA_SUPPORT, 0)
	case MY_PORTA:
	{
		WRITE_PORTS(true);
		break;
	}
	case MY_PORTB:
	{
		WRITE_PORTS(false);
		break;
	}
	case MY_DDRA:
		DDR_PORTS(true);
		break;
	case MY_DDRB:
		DDR_PORTS(false);
		break;
#endif
	}
}

static inline uint8_t    load_rom_byte(uint16_t address)                 { events_read(address); return pgm_read_byte(ROM_data + RELATIVE_ROM_ADDRESS(address)); }
static inline uint16_t   load_rom_word(uint16_t address)                 { events_read(address); events_read(address + 1); return pgm_read_word(ROM_data + RELATIVE_ROM_ADDRESS(address)); }
static inline uint8_t    load_ram_byte(uint16_t address)                 { events_read(address); return RAM[address]; }
static inline uint16_t   load_ram_word(uint16_t address)                 { return (load_ram_byte(address + 1) << 8) | load_ram_byte(address); }
static inline uint8_t    load_any_byte(uint16_t address)                 { return (is_rom(address) ? load_rom_byte(address) : load_ram_byte(address)); }
static inline uint16_t   load_any_word(uint16_t address)                 { return (is_rom(address) ? load_rom_word(address) : load_ram_word(address)); }
static inline void       write_ram_byte(uint16_t address, uint8_t data)  { RAM[address] = data; events_write(address); }
static inline void       write_any_byte(uint16_t address, uint8_t data)  { if(is_ram(address)) write_ram_byte(address, data); }

static inline uint8_t    load_rom_byte()                                 { return load_rom_byte(tmp_word); }
static inline uint16_t   load_rom_word()                                 { return load_rom_word(tmp_word); }
static inline uint8_t    load_ram_byte()                                 { return load_ram_byte(tmp_word); }
static inline uint16_t   load_ram_word()                                 { return load_ram_word(tmp_word); }
static inline uint8_t    load_any_byte()                                 { return load_any_byte(tmp_word); }
static inline uint16_t   load_any_word()                                 { return load_any_word(tmp_word); }
static inline void       write_ram_byte(uint8_t data)                    { write_ram_byte(tmp_word, data); }
static inline void       write_any_byte(uint8_t data)                    { if(is_ram()) write_ram_byte(data); }

static inline uint8_t    load_stack_byte()                               { return RAM[STACK_START + ++s]; }
static inline uint16_t   load_stack_word()                               { return load_stack_byte() | (load_stack_byte() << 8); }
static inline void       write_stack_byte(uint8_t data)                  { RAM[STACK_START + s--] = data; }
static inline void       write_stack_word(uint16_t data)
{ 
	write_stack_byte(HIGH_BYTE(data));
	write_stack_byte(LOW_BYTE(data));
}

static inline uint8_t    read_next_byte() { return (tmp_word = load_any_byte(pc++)); }
static inline uint16_t   read_next_word()
{
	pc += 2;
	return load_any_word(pc - 2);
}

static inline uint8_t  read_next_imm()        { return read_next_byte(); }
static inline uint8_t  read_next_zp()         { return read_next_byte(); }
static inline uint8_t  read_next_zp_x()       { return (tmp_word = ADD_BYTE(read_next_zp(), x)); }
static inline uint8_t  read_next_zp_y()       { return (tmp_word = ADD_BYTE(read_next_zp(), y)); }
static inline uint16_t read_next_zp_ind()     { return (tmp_word = load_ram_word(read_next_zp())); }
static inline uint16_t read_next_zp_idx_ind() { return (tmp_word = load_ram_word(ADD_BYTE(read_next_zp(), x))); }
static inline uint16_t read_next_zp_ind_idx() { return (tmp_word = load_ram_word(read_next_zp()) + y); }
static inline uint16_t read_next_abs()        { return (tmp_word = read_next_word()); }
static inline uint16_t read_next_abs_x()      { return (tmp_word = read_next_abs() + x); }
static inline uint16_t read_next_abs_y()      { return (tmp_word = read_next_abs() + y); }

static inline void set_flags(uint8_t data, uint8_t flags_to_set)
{
	if(CHECK_BIT(flags_to_set, ZeroBit))        SET_BIT_TO(p, ZeroBit, data == 0);
	if(CHECK_BIT(flags_to_set, NegativeBit))    SET_BIT_TO(p, NegativeBit, CHECK_BIT(data, NegativeBit));
	if(CHECK_BIT(flags_to_set, OverflowBit))    SET_BIT_TO(p, OverflowBit, CHECK_BIT(data, OverflowBit));
} 

// BIT_SETS
static inline void lda_bit_set() { set_flags(a, Negative | Zero); }
static inline void ldx_bit_set() { set_flags(x, Negative | Zero); }
static inline void ldy_bit_set() { set_flags(y, Negative | Zero); }

static inline void pla_bit_set() { set_flags(a, Negative | Zero); }
static inline void plx_bit_set() { set_flags(x, Negative | Zero); }
static inline void ply_bit_set() { set_flags(y, Negative | Zero); }

static inline void tsx_bit_set() { set_flags(x, Negative | Zero); }
static inline void txa_bit_set() { set_flags(a, Negative | Zero); }
static inline void tax_bit_set() { set_flags(x, Negative | Zero); }
static inline void tay_bit_set() { set_flags(y, Negative | Zero); }
static inline void tya_bit_set() { set_flags(a, Negative | Zero); }

static inline void dec_bit_set(uint8_t data) { set_flags(data, Negative | Zero); }
static inline void inc_bit_set(uint8_t data) { set_flags(data, Negative | Zero); }

static inline void cmp_bit_set() { set_flags(SUB_BYTE(a, tmp_word), Negative | Zero); SET_BIT_TO(p, CarryBit, a >= tmp_word); }
static inline void cpx_bit_set() { set_flags(SUB_BYTE(x, tmp_word), Negative | Zero); SET_BIT_TO(p, CarryBit, x >= tmp_word); }
static inline void cpy_bit_set() { set_flags(SUB_BYTE(y, tmp_word), Negative | Zero); SET_BIT_TO(p, CarryBit, y >= tmp_word); }

static inline void asl_bit_set(uint8_t data) { set_flags(data, Negative | Zero); }
static inline void lsr_bit_set(uint8_t data) { set_flags(data, Negative | Zero); }
static inline void rol_bit_set(uint8_t data) { set_flags(data, Negative | Zero); }
static inline void ror_bit_set(uint8_t data) { set_flags(data, Negative | Zero); }

static inline void bit_bit_set() { set_flags(tmp_word, Overflow | Negative); set_flags(a & tmp_word, Zero); }

static inline void tsb_trb_bit_set(uint8_t data) { set_flags(a & data, Zero); }

static inline void and_bit_set() { set_flags(a, Negative | Zero); }
static inline void ora_bit_set() { set_flags(a, Negative | Zero); }
static inline void eor_bit_set() { set_flags(a, Negative | Zero); }

static inline void adc_bit_set() { set_flags(a, Negative | Zero); }
static inline void sbc_bit_set() { set_flags(a, Negative | Zero); }

#if CHECK_BIT(DEBUG, 5)
	#if CHECK_BIT(DEBUG, 6)
		#define OP_WORD(txt) Serial.println(F(txt")"))
	#else
		#define OP_WORD(txt) Serial.println(F(txt))
	#endif
#else
	#define OP_WORD(txt)
#endif

// ---------------------- Runnable ----------------------

void irqb()
{
	sleep_disable();
	if(!CHECK_BIT(p, InterruptBit))
	{
		write_stack_word(pc);
		write_stack_byte(p & (~Break));
		p = (p & (~Decimal)) | Interrupt;
		pc = load_rom_word(BRK_IRQB);
	}
}

void setup()
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
#if DEBUG > 0
	Serial.begin(9600);
#endif
	pinMode(2, INPUT_PULLUP);
#ifdef IRQB_LOW
	attachInterrupt(digitalPinToInterrupt(2), irqb, LOW);
#else
	attachInterrupt(digitalPinToInterrupt(2), irqb, FALLING);
#endif
	pc = read_next_abs(); //read RESB vector
}

void loop()
{
#if CHECK_BIT(DEBUG, 2)
	bool do_print_ram = false;
#endif

#if CHECK_BIT(DEBUG, 4) || CHECK_BIT(DEBUG, 7)
new_msg:
	while(!run && !Serial.available()) {}
	for(; 0 < Serial.available();)
	{
		switch(Serial.read())
		{
#if CHECK_BIT(DEBUG, 7)
		case 'a': print_ram(); break;    // rAm
		case 'o': print_rom(); break;    // rOm
		case 'e': print_reg(); break;    // rEgister
		case 's': print_status(); break; // Status register
		case 'g': run = true; break;     // Go (dont wait for keypress)
		case 'p': run = false; break;    // Pause (wait for keypress)
#endif
		case '\n': goto emulator_start;
		}
	}
	if(!run) goto new_msg;
emulator_start:
#endif

op_code = read_next_byte();
#if CHECK_BIT(DEBUG, 6)
	Serial.print(F("running: "));
	print_hex_byte(op_code);
#endif
#if CHECK_BIT(DEBUG, 5) && CHECK_BIT(DEBUG, 6)
	Serial.print(F(" ("));
#endif

	cli();

	switch (op_code){
	case 0x00:                                                          // brk
		OP_WORD("BRK");
		//BRK does set the interrupt-disable I flag like an IRQ does, and if you have the CMOS 6502 (65C02), it will also clear the decimal D flag.
		//Note that BRK, although it is a one-byte instruction, needs an extra byte of padding after it.
		//This is because the return address it puts on the stack will cause the RTI to put the program counter back not to the very next byte after the BRK,
		//but to the second byte after it.
		write_stack_word(pc + 1);
		write_stack_byte(p);
		p = (p & (~Decimal)) | Interrupt;
		pc = load_rom_word(BRK_IRQB);
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("Breaked to "));
	print_hex_word(pc);
	Serial.println();
#endif
		break;
	case 0xDB:                                                          // stp
		OP_WORD("STP");
#if CHECK_BIT(DEBUG, 5)
	delay(100);
#endif
#if CHECK_BIT(DEBUG, 3)
	Serial.println(F("Please press reset to wake me up from sleep\nZZZzzz..."));
	delay(1000);
#endif
		detachInterrupt(digitalPinToInterrupt(2));
		sleep_enable();
		sleep_cpu();
		break;
	case 0xCB:                                                          // wai
		OP_WORD("WAI");
#if CHECK_BIT(DEBUG, 5)
	delay(100);
#endif
		sei();
		sleep_enable();
		sleep_cpu();
		break;
	case 0xA9:                                                          // lda #
		OP_WORD("LDA #");
		a = read_next_imm();
		goto lda_end;
	case 0xA5:                                                          // lda zp
		OP_WORD("LDA ZP");
		read_next_zp();
		goto lda_mem;
	case 0xB5:                                                          // lda zp, x
		OP_WORD("LDA ZP, X");
		read_next_zp_x();
		goto lda_mem;
	case 0xAD:                                                          // lda abs
		OP_WORD("LDA ABS");
		read_next_abs();
		goto lda_mem;
	case 0xBD:                                                          // lda abs, x
		OP_WORD("LDA ABS, X");
		read_next_abs_x();
		goto lda_mem;
	case 0xB9:                                                          // lda abs, y
		OP_WORD("LDA ABS, Y");
		read_next_abs_y();
		goto lda_mem;
	case 0xA1:                                                          // lda (zp, x)
		OP_WORD("LDA (ZP, X)");
		read_next_zp_idx_ind();
		goto lda_mem;
	case 0xB1:                                                          // lda (zp), y
		OP_WORD("LDA (ZP), Y");
		read_next_zp_ind_idx();
		goto lda_mem;
	case 0xB2:                                                          // lda (zp)    *
		OP_WORD("LDA (ZP)");
		read_next_zp_ind();
	lda_mem:
		a = load_any_byte();
	lda_end:
		lda_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("loaded "));
		print_hex_byte(a);
		Serial.print(F(" into A"));
		if(op_code != 0xA9) goto lda_ldx_ldy_msg;
		Serial.println();
#endif
		break;
	case 0xA2:                                                          // ldx #
		OP_WORD("LDX #");
		x = read_next_imm();
		goto ldx_end;
	case 0xA6:                                                          // ldx zp
		OP_WORD("LDX ZP");
		read_next_zp();
		goto ldx_mem;
	case 0xB6:                                                          // ldx zp, y
		OP_WORD("LDX ZP, Y");
		read_next_zp_y();
		goto ldx_mem;
	case 0xAE:                                                          // ldx abs
		OP_WORD("LDX ABS");
		read_next_abs();
		goto ldx_mem;
	case 0xBE:                                                          // ldx abs, y
		OP_WORD("LDX ABS");
		read_next_abs_y();
	ldx_mem:
		x = load_any_byte();
	ldx_end:
		ldx_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("loaded "));
		print_hex_byte(x);
		Serial.print(F(" into X"));
		if(op_code != 0xA2) goto lda_ldx_ldy_msg;
		Serial.println();
#endif
		break;
	case 0xA0:                                                          // ldy #
		OP_WORD("LDY #");
		y = read_next_imm();
		goto ldy_end;
	case 0xA4:                                                          // ldy zp
		OP_WORD("LDY ZP");
		read_next_zp();
		goto ldy_mem;
	case 0xB4:                                                          // ldy zp, x
		OP_WORD("LDY ZP, X");
		read_next_zp_x();
		goto ldy_mem;
	case 0xAC:                                                          // ldy abs
		OP_WORD("LDY ABS");
		read_next_abs();
		goto ldy_mem;
	case 0xBC:                                                          // ldy abs, x
		OP_WORD("LDY ABS, X");
		read_next_abs_x();
	ldy_mem:
		y = load_any_byte();
	ldy_end:
		ldy_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("loaded "));
		print_hex_byte(y);
		Serial.print(F(" into Y"));
		if(op_code != 0xA0)
		{
		lda_ldx_ldy_msg:
			Serial.print(F(" from "));
			print_hex_word(tmp_word);
		}
		Serial.println();
#endif
		break;
	case 0x85:                                                          // sta zp
		OP_WORD("STA ZP");
		read_next_zp();
		goto sta_end;
	case 0x95:                                                          // sta zp, x
		OP_WORD("STA ZP, X");
		read_next_zp_x();
		goto sta_end;
	case 0x8D:                                                          // sta abs
		OP_WORD("STA ABS");
		read_next_abs();
		goto sta_abs;
	case 0x9D:                                                          // sta abs, x
		OP_WORD("STA ABS, X");
		read_next_abs_x();
		goto sta_abs;
	case 0x99:                                                          // sta abs, y
		OP_WORD("STA ABS, Y");
		read_next_abs_y();
		goto sta_abs;
	case 0x81:                                                          // sta (zp, x)
		OP_WORD("STA (ZP, X)");
		read_next_zp_idx_ind();
		goto sta_abs;
	case 0x91:                                                          // sta (zp), y
		OP_WORD("STA (ZP), Y");
		read_next_zp_ind_idx();
		goto sta_abs;
	case 0x92:                                                          // sta (zp)     *
		OP_WORD("STA (ZP)");
		read_next_zp_ind();
	sta_abs:
	sta_end:
		write_any_byte(a);
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("stored A ("));
		print_hex_byte(a);
		goto sta_stx_sty_msg;
#endif
		break;
	case 0x64:                                                          // stz zp       *
		OP_WORD("STZ ZP");
		read_next_zp();
		goto stz_end;
	case 0x74:                                                          // stz zp, x    *
		OP_WORD("STZ ZP, X");
		read_next_zp_x();
		goto stz_end;
	case 0x9C:                                                          // stz abs      *
		OP_WORD("STZ ABS");
		read_next_abs();
		goto stz_abs;
	case 0x9E:                                                          // stz abs, x   *
		OP_WORD("STZ ABS, X");
		read_next_abs_x();
	stz_abs:
	stz_end:
		write_any_byte(0);
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("stored 0 (0"));
		goto sta_stx_sty_msg;
#endif
		break;
	case 0x86:                                                          // stx zp
		OP_WORD("STX ZP");
		read_next_zp();
		goto stx_end;
	case 0x96:                                                          // stx zp, y
		OP_WORD("STX ZP, Y");
		read_next_zp_y();
		goto stx_end;
	case 0x8E:                                                          // stx abs
		OP_WORD("STX ABS");
		if(!is_ram(read_next_abs())) break;
	stx_end:
		write_ram_byte(x);
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("stored X ("));
		print_hex_byte(x);
		goto sta_stx_sty_msg;
#endif
		break;
	case 0x84:                                                          // sty zp
		OP_WORD("STY ZP");
		read_next_zp();
		goto sty_end;
	case 0x94:                                                          // sty zp, x
		OP_WORD("STY ZP, X");
		read_next_zp_x();
		goto sty_end;
	case 0x8C:                                                          // sty abs
		OP_WORD("STY ABS");
		if(!is_ram(read_next_abs())) break;
	sty_end:
		write_ram_byte(y);
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("stored Y ("));
		print_hex_byte(y);
	sta_stx_sty_msg:
		Serial.print(F(") into "));
		print_hex_word(tmp_word);
		Serial.println();
#endif
		break;
	case 0x0F:                                                          // bbr0 *
	case 0x1F:                                                          // bbr1 *
	case 0x2F:                                                          // bbr2 *
	case 0x3F:                                                          // bbr3 *
	case 0x4F:                                                          // bbr4 *
	case 0x5F:                                                          // bbr5 *
	case 0x6F:                                                          // bbr6 *
	case 0x7F:                                                          // bbr7 *
		OP_WORD("BBR");
		if(!CHECK_BIT(load_ram_byte(read_next_zp()), op_code >> 4)) goto branch;
		goto no_branch;
	case 0x8F:                                                          // bbs0 *
	case 0x9F:                                                          // bbs1 *
	case 0xAF:                                                          // bbs2 *
	case 0xBF:                                                          // bbs3 *
	case 0xCF:                                                          // bbs4 *
	case 0xDF:                                                          // bbs5 *
	case 0xEF:                                                          // bbs6 *
	case 0xFF:                                                          // bbs7 *
		OP_WORD("BBS");
		if(CHECK_BIT(load_ram_byte(read_next_zp()), (op_code - 0x80) >> 4)) goto branch;
		goto no_branch;
	case 0xB0:                                                          // bcs
		OP_WORD("BCS");
		if(CHECK_BIT(p, CarryBit)) goto branch;
		goto no_branch;
	case 0x90:                                                          // bcc
		OP_WORD("BCC");
		if(!CHECK_BIT(p, CarryBit)) goto branch;
		goto no_branch;
	case 0xF0:                                                          // beq
		OP_WORD("BEQ");
		if(CHECK_BIT(p, ZeroBit)) goto branch;
		goto no_branch;
	case 0xD0:                                                          // bne
		OP_WORD("BNE");
		if(!CHECK_BIT(p, ZeroBit)) goto branch;
		goto no_branch;
	case 0x30:                                                          // bmi
		OP_WORD("BMI");
		if(CHECK_BIT(p, NegativeBit)) goto branch;
		goto no_branch;
	case 0x10:                                                          // bpl
		OP_WORD("BPL");
		if(!CHECK_BIT(p, NegativeBit)) goto branch;
		goto no_branch;
	case 0x70:                                                          // bvs
		OP_WORD("BVS");
		if(CHECK_BIT(p, OverflowBit)) goto branch;
		goto no_branch;
	case 0x50:                                                          // bvc
		OP_WORD("BVC");
		if(!CHECK_BIT(p, OverflowBit)) goto branch;
	no_branch:
		++pc;
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.println(F("didn't branch"));
#endif
		break;
	case 0x80:                                                          // bra
		OP_WORD("BRA");
	branch:
		pc += static_cast<int8_t>(read_next_imm());
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("branched to "));
		print_hex_word(pc);
		Serial.println();
#endif
		break;
	case 0x48:                                                          // pha
		OP_WORD("PHA");
		write_stack_byte(a);
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("pushed A ("));
		print_hex_byte(a);
		goto store_stack_msg;
#endif
		break;
	case 0x08:                                                          // php
		OP_WORD("PHP");
		write_stack_byte(p);
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("pushed P ("));
		print_bin(p);
		goto store_stack_msg;
#endif
		break;
	case 0xDA:                                                          // phx *
		OP_WORD("PHX");
		write_stack_byte(x);
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("pushed X ("));
		print_hex_byte(x);
		goto store_stack_msg;
#endif
		break;
	case 0x5A:                                                          // phy *
		OP_WORD("PHY");
		write_stack_byte(y);
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("pushed Y ("));
		print_hex_byte(y);
	store_stack_msg:
		Serial.print(F(") into stack at S ("));
		print_hex_byte(s + 1);
		Serial.println(F(")"));
#endif
		break;
	case 0x68:                                                          // pla
		OP_WORD("PLA");
		a = load_stack_byte();
		pla_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("loaded A"));
		goto load_stack_msg;
#endif
		break;
	case 0x28:                                                          // plp
		OP_WORD("PLP");
		p = load_stack_byte() | Break | Unused;
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("loaded flags"));
		goto load_stack_msg;
#endif
		break;
	case 0xFA:                                                          // plx *
		OP_WORD("PLX");
		x = load_stack_byte();
		plx_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("loaded X"));
		goto load_stack_msg;
#endif
		break;
	case 0x7A:                                                          // ply *
		OP_WORD("PLY");
		y = load_stack_byte();
		ply_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("loaded Y"));
	load_stack_msg:
		Serial.print(F(" from stack at S ("));
		print_hex_byte(s);
		Serial.println(F(")"));
#endif
		break;
	case 0x9A:                                                          // txs
		OP_WORD("TXS");
		s = x;
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("transfered X ("));
		print_hex_byte(x);
		Serial.println(F(") to S"));
#endif
		break;
	case 0xBA:                                                          // tsx
		OP_WORD("TSX");
		x = s;
		tsx_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("transfered S ("));
		print_hex_byte(s);
		Serial.println(F(") to X"));
#endif
		break;
	case 0x8A:                                                          // txa
		OP_WORD("TXA");
		a = x;
		txa_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("transfered X ("));
		print_hex_byte(x);
		Serial.println(F(") to A"));
#endif
		break;
	case 0xAA:                                                          // tax
		OP_WORD("TAX");
		x = a;
		tax_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("transfered A ("));
		print_hex_byte(a);
		Serial.println(F(") to X"));
#endif
		break;
	case 0xA8:                                                          // tay
		OP_WORD("TAY");
		y = a;
		tay_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("transfered A ("));
		print_hex_byte(a);
		Serial.println(F(") to Y"));
#endif
		break;
	case 0x98:                                                          // tya
		OP_WORD("TYA");
		a = y;
		tya_bit_set();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("transfered Y ("));
		print_hex_byte(y);
		Serial.println(F(") to A"));
#endif
		break;
	case 0x07:                                                          // rmb0 *
	case 0x17:                                                          // rmb1 *
	case 0x27:                                                          // rmb2 *
	case 0x37:                                                          // rmb3 *
	case 0x47:                                                          // rmb4 *
	case 0x57:                                                          // rmb5 *
	case 0x67:                                                          // rmb6 *
	case 0x77:                                                          // rmb7 *
		OP_WORD("RMB");
		if(is_ram(read_next_zp())) CLEAR_BIT(RAM[tmp_word], op_code >> 4);
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
		break;
	case 0x87:                                                          // smb0 *
	case 0x97:                                                          // smb1 *
	case 0xA7:                                                          // smb2 *
	case 0xB7:                                                          // smb3 *
	case 0xC7:                                                          // smb4 *
	case 0xD7:                                                          // smb5 *
	case 0xE7:                                                          // smb6 *
	case 0xF7:                                                          // smb7 *
		OP_WORD("SMB");
		if(is_ram(read_next_zp())) SET_BIT(RAM[tmp_word], (op_code - 0x80) >> 4);
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
		break;
	case 0x18:                                                          // clc
		OP_WORD("CLC");
		CLEAR_BIT(p, CarryBit);
		break;
	case 0xD8:                                                          // cld
		OP_WORD("CLD");
		CLEAR_BIT(p, DecimalBit);
		break;
	case 0x58:                                                          // cli
		OP_WORD("CLI");
		CLEAR_BIT(p, InterruptBit);
		break;
	case 0xB8:                                                          // clv
		OP_WORD("CLV");
		CLEAR_BIT(p, OverflowBit);
		break;
	case 0x38:                                                          // sec
		OP_WORD("SEC");
		SET_BIT(p, CarryBit);
		break;
	case 0xF8:                                                          // sed
		OP_WORD("SED");
		SET_BIT(p, DecimalBit);
		break;
	case 0x78:                                                          // sei
		OP_WORD("SEI");
		SET_BIT(p, InterruptBit);
		break;
	case 0x3A:                                                          // dec *
		OP_WORD("DEC A");
		dec_bit_set(--a);
		break;
	case 0xCA:                                                          // dex
		OP_WORD("DEX");
		dec_bit_set(--x);
		break;
	case 0x88:                                                          // dey
		OP_WORD("DEY");
		dec_bit_set(--y);
		break;
	case 0xC6:                                                          // dec zp
		OP_WORD("DEC ZP");
		read_next_zp();
		goto dec_end;
	case 0xD6:                                                          // dec zp, x
		OP_WORD("DEC ZP, X");
		read_next_zp_x();
		goto dec_end;
	case 0xCE:                                                          // dec abs
		OP_WORD("DEC ABS");
		read_next_abs();
		goto dec_abs;
	case 0xDE:                                                          // dec abs, x
		OP_WORD("DEC ABS, X");
		read_next_abs_x();
	dec_abs:
		if(!is_ram()) break;
	dec_end:
		dec_bit_set(--RAM[tmp_word]);
		events_write(tmp_word);
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("decremented "));
		goto inc_dec_msg;
#endif
		break;
	case 0x1A:                                                          // inc A *
		OP_WORD("INC A");
		inc_bit_set(++a);
		break;
	case 0xE8:                                                          // inx
		OP_WORD("INX");
		inc_bit_set(++x);
		break;
	case 0xC8:                                                          // iny
		OP_WORD("INY");
		inc_bit_set(++y);
		break;
	case 0xE6:                                                          // inc zp
		OP_WORD("INC ZP");
		read_next_zp();
		goto inc_end;
	case 0xF6:                                                          // inc zp, x
		OP_WORD("INC ZP, X");
		read_next_zp_x();
		goto inc_end;
	case 0xEE:                                                          // inc abs
		OP_WORD("INC ABS");
		read_next_abs();
		goto inc_abs;
	case 0xFE:                                                          // inc abs, x
		OP_WORD("INC ABS, X");
		read_next_abs_x();
	inc_abs:
		if(!is_ram()) break;
	inc_end:
		inc_bit_set(++RAM[tmp_word]);
		events_write(tmp_word);
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("incremented "));
	inc_dec_msg:
		print_hex_word(tmp_word);
		Serial.println();
#endif
		break;
	case 0x6C:                                                          // jmp (abs)
		OP_WORD("JMP (ABS)");
		tmp_word = load_any_word(read_next_abs());
		goto jmp_jsr_end;
	case 0x7C:                                                          // jmp (abs, x)
		OP_WORD("JMP (ABS, X)");
		tmp_word = load_any_word(read_next_abs_x());
		goto jmp_jsr_end;
	case 0x4C:                                                          // jmp abs
		OP_WORD("JMP ABS");
		goto jmp_jsr_read;
	case 0x20:                                                          // jsr abs
		OP_WORD("JSR");
		write_stack_word(pc + 2);
	jmp_jsr_read:
		read_next_abs();
	jmp_jsr_end:
		pc = tmp_word;
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("Jumped to "));
		print_hex_word(pc);
		Serial.println();
#endif
		break;
	case 0x40:                                                          // rti
		OP_WORD("RTI");
		//https://www.masswerk.at/6502/6502_instruction_set.html#RTI
		p = load_stack_byte() | Unused | Break;
		goto rts_rti_end;
	case 0x60:                                                          // rts
		OP_WORD("RTS");
	rts_rti_end:
		pc = load_stack_word();
#if CHECK_BIT(DEBUG, 3) > 0
		Serial.print(F("Returned back to "));
		print_hex_word(pc);
		Serial.println();
#endif
		break;
#if CHECK_BIT(DEBUG, 5) > 0
	case 0xC9:                                                          // cmp #
		OP_WORD("CMP #");
		read_next_imm();
		goto cmp_end;
	case 0xC5:                                                          // cmp zp
		OP_WORD("CMP ZP");
		read_next_zp();
		goto cmp_mem;
	case 0xD5:                                                          // cmp zp, x
		OP_WORD("CMP ZP, X");
		read_next_zp_x();
		goto cmp_mem;
	case 0xCD:                                                          // cmp abs
		OP_WORD("CMP ABS");
		read_next_abs();
		goto cmp_mem;
	case 0xDD:                                                          // cmp abs, x
		OP_WORD("CMP ABS, x");
		read_next_abs_x();
		goto cmp_mem;
	case 0xD9:                                                          // cmp abs, y
		OP_WORD("CMP ABS, y");
		read_next_abs_y();
		goto cmp_mem;
	case 0xC1:                                                          // cmp (zp, x)
		OP_WORD("CMP (ZP, X)");
		read_next_zp_idx_ind();
		goto cmp_mem;
	case 0xD1:                                                          // cmp (zp), y
		OP_WORD("CMP (ZP), Y");
		read_next_zp_ind_idx();
		goto cmp_mem;
	case 0xD2:                                                          // cmp (zp)     *
		OP_WORD("CMP (ZP)");
		read_next_zp_ind();
	cmp_mem:
		tmp_word = load_any_byte();
		goto cmp_end;
	case 0xE0:                                                          // cpx #
	case 0xC0:                                                          // cpy #
		OP_WORD("CPX # = 0xE0, CPY #");
		read_next_imm();
		goto cpx_cpy_branch;
	case 0xE4:                                                          // cpx zp
	case 0xC4:                                                          // cpy zp
		OP_WORD("CPX ZP = 0xE4, CPY ZP");
		read_next_zp();
		goto cpx_cpy_mem;
	case 0xEC:                                                          // cpx abs
	case 0xCC:                                                          // cpy abs
		OP_WORD("CPX ABS = 0xEC, CPY ABS");
		read_next_abs();
	cpx_cpy_mem:
		tmp_word = load_any_byte();
	cpx_cpy_branch:
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("compared "));
#endif
		if(op_code >= 0xE0) goto cpx_end;
		else goto cpy_end;
#else
	case 0xC1:                                                          // cmp (zp, x)
	case 0xD1:                                                          // cmp (zp), y
	case 0xD2:                                                          // cmp (zp)     *
		tmp_word = load_ram_word(ADD_BYTE(read_next_zp(), ((op_code == 0xC1) ? x : 0))) + ((op_code == 0xD1) ? y : 0);
		goto cpx_cpy_cmp_mem;
	case 0xC9:                                                          // cmp #
	case 0xE0:                                                          // cpx #
	case 0xC0:                                                          // cpy #
		read_next_imm();
		goto cpx_cpy_cmp_branch;
	case 0xE4:                                                          // cpx zp
	case 0xC4:                                                          // cpy zp
	case 0xC5:                                                          // cmp zp
	case 0xD5:                                                          // cmp zp, x
		tmp_word = ADD_BYTE(read_next_zp(), ((op_code == 0xD5) ? x : 0));
		goto cpx_cpy_cmp_mem;
	case 0xEC:                                                          // cpx abs
	case 0xCC:                                                          // cpy abs
	case 0xCD:                                                          // cmp abs
	case 0xDD:                                                          // cmp abs, x
	case 0xD9:                                                          // cmp abs, y
		tmp_word = read_next_abs() + ((op_code == 0xDD) ? x : ((op_code == 0xD9) ? y : 0));
	cpx_cpy_cmp_mem:
		tmp_word = load_any_byte();
	cpx_cpy_cmp_branch:
		if (op_code == 0xD2 || CHECK_BIT(op_code, 0)) goto cmp_end;
		else if(op_code >= 0xE0) goto cpx_end;
		else goto cpy_end;
#endif
	cpy_end:
		cpy_bit_set();
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("Y ("));
	print_hex_byte(y);
	goto cpx_cpy_cmp_end;
#endif
		break;
	cpx_end:
		cpx_bit_set();
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("X ("));
	print_hex_byte(x);
	goto cpx_cpy_cmp_end;
#endif
		break;
	cmp_end:
		cmp_bit_set();
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("A ("));
	print_hex_byte(a);
cpx_cpy_cmp_end:
	Serial.print(F(") to "));
	print_hex_byte(LOW_BYTE(tmp_word));
	Serial.println();
#endif
		break;
	case 0x0A:                                                          // asl a
		OP_WORD("ASL A");
		SET_BIT_TO(p, CarryBit, CHECK_BIT(a, 7));
		asl_bit_set(a <<= 1);
		goto asl_end;
	case 0x06:                                                          // asl zp
		OP_WORD("ASL ZP");
		read_next_zp();
		goto asl_mem;
	case 0x16:                                                          // asl zp, x
		OP_WORD("ASL ZP, X");
		read_next_zp_x();
		goto asl_mem;
	case 0x0E:                                                          // asl abs
		OP_WORD("ASL ABS");
		read_next_abs();
		goto asl_mem;
	case 0x1E:                                                          // asl abs, x
		OP_WORD("ASL ABS, X");
		read_next_abs_x();
	asl_mem:
		if(!is_ram()) break;
		SET_BIT_TO(p, CarryBit, CHECK_BIT(RAM[tmp_word], 7));
		asl_bit_set((RAM[tmp_word] <<= 1));
		events_write(tmp_word);
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
	asl_end:
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("Shifted left "));
	goto asl_lsr_ror_rol_msg;
#endif
		break;
	case 0x4A:                                                          // lsr a
		OP_WORD("LSR A");
		SET_BIT_TO(p, CarryBit, CHECK_BIT(a, 0));
		lsr_bit_set(a >>= 1);
		goto lsr_end;
	case 0x46:                                                          // lsr zp
		OP_WORD("LSR ZP");
		read_next_zp();
		goto lsr_mem;
	case 0x56:                                                          // lsr zp, x
		OP_WORD("LSR ZP, X");
		read_next_zp_x();
		goto lsr_mem;
	case 0x4E:                                                          // lsr abs
		OP_WORD("LSR ABS");
		read_next_abs();
		goto lsr_mem;
	case 0x5E:                                                          // lsr abs, x
		OP_WORD("LSR ABS, X");
		read_next_abs_x();
	lsr_mem:
		if(!is_ram()) break;
		SET_BIT_TO(p, CarryBit, CHECK_BIT(RAM[tmp_word], 0));
		lsr_bit_set((RAM[tmp_word] >>= 1));
		events_write(tmp_word);
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
	lsr_end:
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("Shifted right "));
	goto asl_lsr_ror_rol_msg;
#endif
		break;
	case 0x2A:                                                          // rol a
		OP_WORD("ROL A");
		SET_BIT_TO(p, UnusedBit, CHECK_BIT(p, CarryBit)); // XD
		SET_BIT_TO(p, CarryBit, CHECK_BIT(a, 7));
		a = (a << 1) | (CHECK_BIT(p, UnusedBit) >> UnusedBit);
		rol_bit_set(a);
		goto rol_end;
	case 0x26:                                                          // rol zp
		OP_WORD("ROL ZP");
		read_next_zp();
		goto rol_mem;
	case 0x36:                                                          // rol zp, x
		OP_WORD("ROL ZP, X");
		read_next_zp_x();
		goto rol_mem;
	case 0x2E:                                                          // rol abs
		OP_WORD("ROL ABS");
		read_next_abs();
		goto rol_mem;
	case 0x3E:                                                          // rol abs, x
		OP_WORD("ROL ABS, X");
		read_next_abs_x();
	rol_mem:
		if(!is_ram()) break;
		SET_BIT_TO(p, UnusedBit, CHECK_BIT(p, CarryBit)); // XD
		SET_BIT_TO(p, CarryBit, CHECK_BIT(load_ram_byte(), 7));
		write_ram_byte((load_ram_byte() << 1) | (CHECK_BIT(p, UnusedBit) >> UnusedBit));
		rol_bit_set(load_ram_byte());
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
	rol_end:
		SET_BIT(p, UnusedBit);
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("Rotated left "));
	goto asl_lsr_ror_rol_msg;
#endif
		break;
	case 0x6A:                                                          // ror a
		OP_WORD("ROR A");
		SET_BIT_TO(p, UnusedBit, CHECK_BIT(p, CarryBit)); // XD
		SET_BIT_TO(p, CarryBit, CHECK_BIT(a, 0));
		a = (a >> 1) | (CHECK_BIT(p, UnusedBit) << (7 - UnusedBit));
		ror_bit_set(a);
		goto ror_end;
	case 0x66:                                                          // ror zp
		OP_WORD("ROR ZP");
		read_next_zp();
		goto ror_mem;
	case 0x76:                                                          // ror zp, x
		OP_WORD("ROR ZP, X");
		read_next_zp_x();
		goto ror_mem;
	case 0x6E:                                                          // ror abs
		OP_WORD("ROR ABS");
		read_next_abs();
		goto ror_mem;
	case 0x7E:                                                          // ror abs, x
		OP_WORD("ROR ABS, X");
		read_next_abs_x();
	ror_mem:
		if(!is_ram()) break;
		SET_BIT_TO(p, UnusedBit, CHECK_BIT(p, CarryBit)); // XD
		SET_BIT_TO(p, CarryBit, CHECK_BIT(load_ram_byte(), 0));
		write_ram_byte((load_ram_byte() >> 1) | (CHECK_BIT(p, UnusedBit) << (7 - UnusedBit)));
		ror_bit_set(load_ram_byte());
#if CHECK_BIT(DEBUG, 2) > 0
		do_print_ram = true;
#endif
	ror_end:
		SET_BIT(p, UnusedBit);
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("Rotated right "));
asl_lsr_ror_rol_msg:
	if(LOW_NIBBLE(op_code) == 0xA) Serial.println(F("A"));
	else 
	{ 
		print_hex_byte(tmp_word);
		Serial.println();
	}
#endif
		break;
	case 0x89:                                                          // bit # *
		OP_WORD("BIT #");
		read_next_imm();
		goto bit_end;
	case 0x24:                                                          // bit zp
		OP_WORD("BIT ZP");
		read_next_zp();
		goto bit_mem;
	case 0x34:                                                          // bit zp, x *
		OP_WORD("BIT ZP, X");
		read_next_zp_x();
		goto bit_mem;
	case 0x2C:                                                          // bit abs
		OP_WORD("BIT ABS");
		read_next_abs();
		goto bit_mem;
	case 0x3C:                                                          // bit abs, x *
		OP_WORD("BIT ABS, X");
		read_next_abs_x();
	bit_mem:
		tmp_word = load_any_byte();
	bit_end:
		bit_bit_set();
		break;
	case 0x14:                                                          // trb zp *
	case 0x04:                                                          // tsb zp *
		OP_WORD("(TSB = 0x04, TRB = 0x14) ZP");
		read_next_zp();
		goto trb_tsb_end;
	case 0x1C:                                                          // trb abs *
	case 0x0C:                                                          // tsb abs *
		OP_WORD("(TSB = 0x04, TRB = 0x14) ABS");
		read_next_abs();
	trb_tsb_end:
		tsb_trb_bit_set(load_any_byte());
		write_any_byte((op_code > 0x10) ? ((~a) & load_ram_byte()) : (a | load_ram_byte()));
#if CHECK_BIT(DEBUG, 2)
	do_print_ram = true;
#endif
		break;
	case 0x29:                                                          // and #
		OP_WORD("AND #");
		read_next_imm();
		goto and_end;
	case 0x25:                                                          // and zp
		OP_WORD("AND ZP");
		read_next_zp();
		goto and_mem;
	case 0x35:                                                          // and zp, x
		OP_WORD("AND ZP, X");
		read_next_zp_x();
		goto and_mem;
	case 0x2D:                                                          // and abs
		OP_WORD("AND ABS");
		read_next_abs();
		goto and_mem;
	case 0x3D:                                                          // and abs, x
		OP_WORD("AND ABS, X");
		read_next_abs_x();
		goto and_mem;
	case 0x39:                                                          // and abs, y
		OP_WORD("AND ABS, Y");
		read_next_abs_y();
		goto and_mem;
	case 0x21:                                                          // and (zp, x)
		OP_WORD("AND (ZP, X)");
		read_next_zp_idx_ind();
		goto and_mem;
	case 0x31:                                                          // and (zp), y
		OP_WORD("AND (ZP), Y");
		read_next_zp_ind_idx();
		goto and_mem;
	case 0x32:                                                          // and (zp)    *
		OP_WORD("AND (ZP)");
		read_next_zp_ind();
	and_mem:
		tmp_word = load_any_byte();
	and_end:
		a &= tmp_word;
		and_bit_set();
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("AND-ed A ("));
	print_hex_byte((~a) ^ LOW_BYTE(tmp_word));
	goto and_eor_ora_msg;
#endif
		break;
	case 0x49:                                                          // eor #
		OP_WORD("EOR #");
		a ^= read_next_imm();
		goto eor_end;
	case 0x45:                                                          // eor zp
		OP_WORD("EOR ZP");
		read_next_zp();
		goto eor_mem;
	case 0x55:                                                          // eor zp, x
		OP_WORD("EOR ZP, X");
		read_next_zp_x();
		goto eor_mem;
	case 0x4D:                                                          // eor abs
		OP_WORD("EOR ABS");
		read_next_abs();
		goto eor_mem;
	case 0x5D:                                                          // eor abs, x
		OP_WORD("EOR ABS, X");
		read_next_abs_x();
		goto eor_mem;
	case 0x59:                                                          // eor abs, y
		OP_WORD("EOR ABS, Y");
		read_next_abs_y();
		goto eor_mem;
	case 0x41:                                                          // eor (zp, x)
		OP_WORD("EOR (ZP, X)");
		read_next_zp_idx_ind();
		goto eor_mem;
	case 0x51:                                                          // eor (zp), y
		OP_WORD("EOR (ZP), Y");
		read_next_zp_ind_idx();
		goto eor_mem;
	case 0x52:                                                          // eor (zp)    *
		OP_WORD("EOR (ZP)");
		read_next_zp_ind();
	eor_mem:
		tmp_word = load_any_byte();
		a ^= tmp_word;
	eor_end:
		eor_bit_set();
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("XOR-ed A ("));
	print_hex_byte(a ^ LOW_BYTE(tmp_word));
	goto and_eor_ora_msg;
#endif
		break;
	case 0x09:                                                          // ora #
		OP_WORD("ORA #");
		read_next_imm();
		goto ora_end;
	case 0x05:                                                          // ora zp
		OP_WORD("ORA ZP");
		read_next_zp();
		goto ora_mem;
	case 0x15:                                                          // ora zp, x
		OP_WORD("ORA ZP, X");
		read_next_zp_x();
		goto ora_mem;
	case 0x0D:                                                          // ora abs
		OP_WORD("ORA ABS");
		read_next_abs();
		goto ora_mem;
	case 0x1D:                                                          // ora abs, x
		OP_WORD("ORA ABS, X");
		read_next_abs_x();
		goto ora_mem;
	case 0x19:                                                          // ora abs, y
		OP_WORD("ORA ABS, Y");
		read_next_abs_y();
		goto ora_mem;
	case 0x11:                                                          // ora (zp), y
		OP_WORD("ORA (ZP, X)");
		read_next_zp_idx_ind();
		goto ora_mem;
	case 0x01:                                                          // ora (zp, x)
		OP_WORD("ORA (ZP), Y");
		read_next_zp_ind_idx();
		goto ora_mem;
	case 0x12:                                                          // ora (zp)    *
		OP_WORD("ORA (ZP)");
		read_next_zp_ind();
	ora_mem:
		tmp_word = load_any_byte();
	ora_end:
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("OR-ed A ("));
	print_hex_byte(a);
	and_eor_ora_msg:
	Serial.print(F(") with "));
	print_hex_byte(LOW_BYTE(tmp_word));
	Serial.println();
	if(op_code > 0x1F) break;
#endif
		a |= tmp_word;
		ora_bit_set();
		break;
	case 0x69:                                                          // adc #
		OP_WORD("ADC #");
		read_next_imm();
		goto adc_end;
	case 0x65:                                                          // adc zp
		OP_WORD("ADC ZP");
		read_next_zp();
		goto adc_mem;
	case 0x75:                                                          // adc zp, x
		OP_WORD("ADC ZP, X");
		read_next_zp_x();
		goto adc_mem;
	case 0x6D:                                                          // adc abs
		OP_WORD("ADC ABS");
		read_next_abs();
		goto adc_mem;
	case 0x7D:                                                          // adc abs, x
		OP_WORD("ADC ABS, X");
		read_next_abs_x();
		goto adc_mem;
	case 0x79:                                                          // adc abs, y
		OP_WORD("ADC ABS, Y");
		read_next_abs_y();
		goto adc_mem;
	case 0x61:                                                          // adc (zp, x)
		OP_WORD("ADC (ZP, X)");
		read_next_zp_idx_ind();
		goto adc_mem;
	case 0x71:                                                          // adc (zp), y
		OP_WORD("ADC (ZP), Y");
		read_next_zp_ind_idx();
		goto adc_mem;
	case 0x72:                                                          // adc (zp)    *
		OP_WORD("ADC (ZP)");
		read_next_zp_ind();
	adc_mem:
		tmp_word = load_any_byte();
	adc_end:
		if(CHECK_BIT(p, DecimalBit))
		{
			// http://www.6502.org/tutorials/decimal_mode.html#A
			tmp_byte = LOW_NIBBLE(a) + LOW_NIBBLE(tmp_word) + CHECK_BIT(p, CarryBit);   // 1a. AL = (A & $0F) + (B & $0F) + C
			if(tmp_byte >= 0x0A) tmp_byte = LOW_NIBBLE(tmp_byte + 0x06) + 0x10;         // 1b. If AL >= $0A, then AL = ((AL + $06) & $0F) + $10
			tmp_byte = (a & 0xF0) + (tmp_word & 0xF0) + tmp_byte;                       // 1c. A = (A & $F0) + (B & $F0) + AL
			if(tmp_byte >= 0xA0)                                                        // 1e. If (A >= $A0), then A = A + $60
			{
				tmp_byte += 0x60;
				SET_BIT(p, CarryBit);
			}
			else CLEAR_BIT(p, CarryBit);
		}
		else
		{
			tmp_byte = a + tmp_word + CHECK_BIT(p, CarryBit);
			SET_BIT_TO(p, CarryBit, tmp_byte < a);
		}
		SET_BIT_TO(p, OverflowBit, !CHECK_BIT(tmp_word ^ a, 7) && CHECK_BIT(tmp_word ^ tmp_byte, 7)); //(M^result)&(N^result)&0x80 , !((M^N) & 0x80) && ((M^result) & 0x80)
		a = tmp_byte;
		adc_bit_set();
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("Added"));
	print_hex_byte(LOW_BYTE(tmp_word));
	Serial.print(F("to A ("));
	print_hex_byte(a);
	Serial.println(')');
#endif
		break;
	case 0xE9:                                                          // sbc #
		OP_WORD("SBC #");
		read_next_imm();
		goto sbc_end;
	case 0xE5:                                                          // sbc zp
		OP_WORD("SBC ZP");
		read_next_zp();
		goto sbc_mem;
	case 0xF5:                                                          // sbc zp, x
		OP_WORD("SBC ZP, X");
		read_next_zp_x();
		goto sbc_mem;
	case 0xED:                                                          // sbc abs
		OP_WORD("SBC ABS");
		read_next_abs();
		goto sbc_mem;
	case 0xFD:                                                          // sbc abs, x
		OP_WORD("SBC ABS, X");
		read_next_abs_x();
		goto sbc_mem;
	case 0xF9:                                                          // sbc abs, y
		OP_WORD("SBC ABS, Y");
		read_next_abs_y();
		goto sbc_mem;
	case 0xF1:                                                          // sbc (zp), y
		OP_WORD("SBC (ZP, X)");
		read_next_zp_idx_ind();
		goto sbc_mem;
	case 0xE1:                                                          // sbc (zp, x)
		OP_WORD("SBC (ZP), Y");
		read_next_zp_ind_idx();
		goto sbc_mem;
	case 0xF2:                                                          // sbc (zp)    *
		OP_WORD("SBC (ZP)");
		read_next_zp_ind();
	sbc_mem:
		tmp_word = load_any_byte();
	sbc_end:
		if(CHECK_BIT(p, DecimalBit))
		{
			// http://www.6502.org/tutorials/decimal_mode.html#A
			tmp_byte = LOW_NIBBLE(a) - LOW_NIBBLE(tmp_word) - !CHECK_BIT(p, CarryBit);   // AL = (A & $0F) - (B & $0F) + C-1
			if(tmp_byte >= 0x80) tmp_byte = LOW_NIBBLE(tmp_byte - 0x06) - 0x10;          // If AL < 0, then AL = ((AL - $06) & $0F) - $10
			tmp_byte = (a & 0xF0) - (tmp_word & 0xF0) + tmp_byte;                        // A = (A & $F0) - (B & $F0) + AL
			if(tmp_byte >= 0x80)                                                         // If A < 0, then A = A - $60
			{
				tmp_byte -= 0x60;
				CLEAR_BIT(p, CarryBit);
			}
			else SET_BIT(p, CarryBit);
		}
		else
		{
			tmp_byte = a - tmp_word - !CHECK_BIT(p, CarryBit);
			SET_BIT_TO(p, CarryBit, tmp_byte <= a);
		}
		SET_BIT_TO(p, OverflowBit, CHECK_BIT(tmp_word ^ a, 7) && !CHECK_BIT(tmp_word ^ tmp_byte, 7)); //(M^result)&(N^result)&0x80 , !((M^N) & 0x80) && ((M^result) & 0x80)
		a = tmp_byte;
		sbc_bit_set();
#if CHECK_BIT(DEBUG, 3)
	Serial.print(F("Subbed"));
	print_hex_byte(LOW_BYTE(tmp_word));
	Serial.print(F("from A ("));
	print_hex_byte(a);
	Serial.println(')');
#endif
		break;
	default:                                                          // nop
		OP_WORD("UNSPECIFIED / NOP");
		break;
	}
	
	sei();

#if CHECK_BIT(VIA_SUPPORT, 1) // T1, IFR, ACR basics only

	if(CHECK_BIT(eventFlags, T1_StartBit))
	{
		if(!CHECK_BIT(RAM[MY_ACR], ACR_T1C1_Bit))           // timer 1 one-shot mode
		{
			if(RAM[MY_T1CH] == 0x00 && RAM[MY_T1CL] == 0x00)
			{
				SET_BIT(RAM[MY_IFR], IFR_T1_Bit);
				if(ier_check(IER_T1_Bit)) irqb();
				#if CHECK_BIT(VIA_SUPPORT, 0)
					if(CHECK_BIT(RAM[MY_ACR], ACR_T1C2_Bit))
					{
						SET_BIT(RAM[MY_DDRB], 7);
						SET_BIT(RAM[MY_PORTB], 7);
					}
				#endif
			}
			else
			{
				if(--RAM[MY_T1CL] == 0xFF) --RAM[MY_T1CH];
				#if CHECK_BIT(VIA_SUPPORT, 0)
					if(CHECK_BIT(RAM[MY_ACR], ACR_T1C2_Bit))
					{
						SET_BIT(RAM[MY_DDRB], 7);
						CLEAR_BIT(RAM[MY_PORTB], 7);
					}
				#endif
			}
		}
		else                                                // timer 1 free run mode
		{
			if(--RAM[MY_T1CL] == 0xFF) --RAM[MY_T1CH];
			
			if(RAM[MY_T1CH] == 0x00 && RAM[MY_T1CL] == 0x00)
			{
				SET_BIT(RAM[MY_IFR], IFR_T1_Bit);
				if(ier_check(IER_T1_Bit)) irqb();
				#if CHECK_BIT(VIA_SUPPORT, 0)
					if(CHECK_BIT(RAM[MY_ACR], ACR_T1C2_Bit))
					{
						SET_BIT(RAM[MY_DDRB], 7);
						CHANGE_BIT(RAM[MY_PORTB], 7);
					}
				#endif
				RAM[MY_T1CH] = RAM[MY_T1LH];
				RAM[MY_T1CL] = RAM[MY_T1LL];
			}
		}
	}
#if CHECK_BIT(VIA_SUPPORT, 2)
	if(CHECK_BIT(eventFlags, T2_StartBit))
	{
		if(!CHECK_BIT(RAM[MY_ACR], ACR_T2C_Bit))            // timer 2 one-shot mode
		{
			if(--RAM[MY_T2CL] == 0xFF) --RAM[MY_T2CH];
			else if(RAM[MY_T2CH] == 0x00 && RAM[MY_T2CL] == 0x00 && !CHECK_BIT(RAM[MY_IFR], IFR_T2_Bit))
			{
				SET_BIT(RAM[MY_IFR], IFR_T2_Bit);
				if(ier_check(IER_T2_Bit)) irqb();
			}
		}
	#if CHECK_BIT(VIA_SUPPORT, 0)
		else                                                // timer 2 counting mode
		{
		#ifdef PULLUP
			pinMode(MY_PB6, INPUT_PULLUP);
			if(!digitalRead(MY_PB6) && --RAM[MY_T2CL] == 0xFF) --RAM[MY_T2CH];
		#else
			pinMode(MY_PB6, INPUT);
			if(digitalRead(MY_PB6) && --RAM[MY_T2CL] == 0xFF) --RAM[MY_T2CH];
		#endif
			else if(RAM[MY_T2CH] == 0x00 && RAM[MY_T2CL] == 0x00 && !CHECK_BIT(RAM[MY_IFR], IFR_T2_Bit))
			{
				SET_BIT(RAM[MY_IFR], IFR_T2_Bit);
				if(ier_check(IER_T2_Bit)) irqb();
			}
		}
	#endif
	}
#endif
	SET_BIT_TO(RAM[MY_IFR], IFR_IRQ_Bit, RAM[MY_IFR] & (RAM[MY_IER] & 0b01111111));
#endif

	#if CHECK_BIT(DEBUG, 0)
		print_status();
	#endif
	#if CHECK_BIT(DEBUG, 1)
		print_reg();
	#endif
	#if CHECK_BIT(DEBUG, 2)
		if(do_print_ram) print_ram();
	#endif
}