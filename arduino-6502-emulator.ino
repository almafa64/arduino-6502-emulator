// !!!!! Remove tmp_byte/tmp_word !!!!!
// !!!!! Remove replace goto with #define or function !!!!!
// !!!!! replace RAM[tmp_word] with load_any_byte() !!!!!

// Try out: remove tmp_word and tmp_byte, use local variables instead (more bytes maybe faster???)
// Try out: or only tmp_word and tmp_byte (maybe faster and less bytes???)
// Try out: remove goto-s (lot more bytes but faster)

// ToDo: Split into multiple parts (move loop() out)
// ToDo: Split debug and no debug code (easier to read)
// ToDo: Serial comm from emulator

// ---------------------- user defined ----------------------
//#define ROM_START	    0xFEB1		// lcd display test
#define ROM_START	    0xFFCC		// timer 1 one shot test on A5
//#define ROM_START	    0xFFA0		// timer 1 free run test
//#define ROM_START	    0xFFDB		// A5 led test with D2 button

#define RAM_END         0x0300

/**
*   1: print reg
*   2: print flags
*   4: print ram
*   8: print instructions texts
*   16: wait for enter
*   32: print instuction name
*   64: print instuction number
*   128: commands throug Serial
*/
//#define DEBUG	        ( 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 )
//#define DEBUG	        ( 16 | 32 | 64 | 128 )
//#define DEBUG	        ( 32 | 64 | 128 )
#define DEBUG	        ( 128 )

//#define USE_FILE
//#define USE_SCRIPT // inject.py

//#define IRQB_LOW	    // interrupt continously on GND

//#define VIA_SUPPORT     ( 1 | 2 | 4 )        // 1: port support, 2: timer1 support, 4: extras
#define VIA_SUPPORT     ( 1 | 2 )        // 1: port support, 2: timer1 support, 4: extras
//#define ACIA_SUPPORT    // serial support

// ---------------------- header ----------------------
#include <assert.h>
#include <avr/sleep.h>
#define VALUE(x) _TO_VALUE(x)
#define _TO_VALUE(x) #x

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef VIA_SUPPORT
#define VIA_SUPPORT 0
#endif

#ifdef ACIA_SUPPORT
#undef ACIA_SUPPORT
#define ACIA_SUPPORT 1
#else
#define ACIA_SUPPORT 0
#endif

// ---------------------- bit defines ----------------------

#define BIT(nth_bit)                    (1U << (nth_bit)) //bit(nth_bit)
#define CHECK_BIT(data, bit)            ((data) & BIT(bit)) //bitRead(data, bit)
#define SET_BIT_TO(data, bit, to)       ((data) = (((data) & (~BIT(bit)))) | (((to) ? 1 : 0) << (bit))) //bitWrite(data, bit, to)
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

#define MY_ACIA_DATA          (RAM_START + 0x0212)      // 
#define MY_ACIA_STATUS        (RAM_START + 0x0213)      // 
#define MY_ACIA_CMD           (RAM_START + 0x0214)      // 
#define MY_ACIA_CTRL          (RAM_START + 0x0215)      // 

#define IRQB_PIN              2

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
#define RAM_MAX_SIZE 0x0770

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
	/*const PROGMEM uint8_t ROM_data[] = {	0xCE, 0x03, 0x02, 0xCE, 0x02, 0x02, 0x78, 0xA9, 0x38, 0x20, 0x24, 0xFF,     // lcd test
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
											0x00, 0x00, LOW_BYTE(ROM_START),HIGH_BYTE(ROM_START), 0x4C, 0xFF };*/
	const PROGMEM uint8_t ROM_data[] = {	0xCE, 0x03, 0x02, 0x9C, 0x00, 0x02, 0x9C, 0x0B, 0x02, 0xA2, 0x20, 0x8E,       // timer 1 one shot test
											0x00, 0x02, 0x20, 0xE5, 0xFF, 0x9C, 0x00, 0x02, 0x20, 0xE5, 0xFF, 0x80,
											0xF2, 0xA9, 0x50, 0x8D, 0x04, 0x02, 0xA9, 0xC3, 0x8D, 0x05, 0x02, 0x2C,
											0x0D, 0x02, 0x50, 0xFB, 0xAD, 0x04, 0x02, 0x60, 0x40,
											0x00, 0x00, LOW_BYTE(ROM_START),HIGH_BYTE(ROM_START), 0xF8, 0xFF };
	/*const PROGMEM uint8_t ROM_data[] = {	0xCE, 0x02, 0x02, 0x9C, 0x01, 0x02, 0x9C, 0x0B, 0x02, 0x64, 0x04, 0x20,       // timer 1 free run test
											0xC9, 0xFF, 0x20, 0xB3, 0xFF, 0x80, 0xFB, 0x38, 0xA5, 0x00, 0xE5, 0x04,
											0xC9, 0x19, 0x90, 0x0C, 0xA9, 0x01, 0x4D, 0x01, 0x02, 0x8D, 0x01, 0x02,
											0xA5, 0x00, 0x85, 0x04, 0x60, 0x64, 0x00, 0x64, 0x01, 0x64, 0x02, 0x64,
											0x03, 0xA9, 0x40, 0x8D, 0x0B, 0x02, 0xA9, 0x00, 0x8D, 0x04, 0x02, 0xA9,
											0x01, 0x8D, 0x05, 0x02, 0xA9, 0xC0, 0x8D, 0x0E, 0x02, 0x58, 0x60, 0x2C,
											0x04, 0x02, 0xE6, 0x00, 0xD0, 0x0A, 0xE6, 0x01, 0xD0, 0x06, 0xE6, 0x02,
											0xD0, 0x02, 0xE6, 0x03, 0x40,
											0x00, 0x00, LOW_BYTE(ROM_START),HIGH_BYTE(ROM_START), 0xE7, 0xFF };*/
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

void print_help()
{
	Serial.println(F(
		"a: rAm\no: rOm\ne: rEgister\ns: Status register\ng: Go (dont wait for keypress)\np: Pause (wait for keypress)\nh: Help"
	));
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

static inline bool ier_check(uint8_t bit) { return CHECK_BIT(RAM[MY_IER], bit); }

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
		uint8_t ddr = RAM[MY_DDRA];
		// PORTC A0 - A5
		// PORTD D3 - D4
		uint8_t d = PIND;
		uint8_t c = PINC;
		
		const uint8_t c_mask = 0b00111111;
		const uint8_t d_mask = 0b00011000;
		
		RAM[MY_PORTA] = ((c & c_mask) >> 0) & ~ddr;     // 0-5 -> 0-5
		RAM[MY_PORTA] |= ((d & d_mask) >> 3) & ~ddr;    // 3-4 -> 6-7
	}
	else
	{
		uint8_t ddr = RAM[MY_DDRB];
		// PORTD D5 - D7
		// PORTB D8 - D12
		uint8_t b = PINB;
		uint8_t d = PIND;
		
		const uint8_t b_mask = 0b00011111;
		const uint8_t d_mask = 0b11100000;
		
		RAM[MY_PORTB] = ((b & b_mask) << 3) & ~ddr;     // 0-4 -> 3-7
		RAM[MY_PORTB] |= ((d & d_mask) >> 5) & ~ddr;    // 5-7 -> 0-2
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

static inline uint8_t    load_rom_byte()                                 { return load_rom_byte(tmp_word); }
static inline uint16_t   load_rom_word()                                 { return load_rom_word(tmp_word); }
static inline uint8_t    load_ram_byte()                                 { return load_ram_byte(tmp_word); }
static inline uint16_t   load_ram_word()                                 { return load_ram_word(tmp_word); }
static inline uint8_t    load_any_byte()                                 { return load_any_byte(tmp_word); }
static inline uint16_t   load_any_word()                                 { return load_any_word(tmp_word); }
static inline void       write_ram_byte(uint8_t data)                    { RAM[tmp_word] = data; events_write(tmp_word); }
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

#if DEBUG > 0 || ACIA_SUPPORT > 0
	Serial.begin(9600);
#endif

	pinMode(IRQB_PIN, INPUT_PULLUP);
#ifdef IRQB_LOW
	attachInterrupt(digitalPinToInterrupt(IRQB_PIN), irqb, LOW);
#else
	attachInterrupt(digitalPinToInterrupt(IRQB_PIN), irqb, FALLING);
#endif

	pc = read_next_abs(); //read RESB vector
}

void loop()
{
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
		case 'h': print_help(); break;   // Help
#endif
		case '\n': goto emulator_start;
		}
	}
	if(!run) goto new_msg;
emulator_start:
#endif

	op_code = read_next_byte();
	
	cli();

	switch (op_code){
	case 0x00:                                                          // brk
		//BRK does set the interrupt-disable I flag like an IRQ does, and if you have the CMOS 6502 (65C02), it will also clear the decimal D flag.
		//Note that BRK, although it is a one-byte instruction, needs an extra byte of padding after it.
		//This is because the return address it puts on the stack will cause the RTI to put the program counter back not to the very next byte after the BRK,
		//but to the second byte after it.
		write_stack_word(pc + 1);
		write_stack_byte(p);
		p = (p & (~Decimal)) | Interrupt;
		pc = load_rom_word(BRK_IRQB);
		break;
	case 0xDB:                                                          // stp
		detachInterrupt(digitalPinToInterrupt(2));
		sleep_enable();
		sleep_cpu();
		break;
	case 0xCB:                                                          // wai
		sei();
		sleep_enable();
		sleep_cpu();
		break;
	case 0xA9:                                                          // lda #
		a = read_next_imm();
		lda_bit_set();
		break;
	case 0xA5:                                                          // lda zp
		read_next_zp();
		a = load_ram_byte();
		lda_bit_set();
		break;
	case 0xB5:                                                          // lda zp, x
		read_next_zp_x();
		a = load_any_byte();
		lda_bit_set();
		break;
	case 0xAD:                                                          // lda abs
		read_next_abs();
		a = load_any_byte();
		lda_bit_set();
		break;
	case 0xBD:                                                          // lda abs, x
		read_next_abs_x();
		a = load_any_byte();
		lda_bit_set();
		break;
	case 0xB9:                                                          // lda abs, y
		read_next_abs_y();
		a = load_any_byte();
		lda_bit_set();
		break;
	case 0xA1:                                                          // lda (zp, x)
		read_next_zp_idx_ind();
		a = load_any_byte();
		lda_bit_set();
		break;
	case 0xB1:                                                          // lda (zp), y
		read_next_zp_ind_idx();
		a = load_any_byte();
		lda_bit_set();
		break;
	case 0xB2:                                                          // lda (zp)    *
		read_next_zp_ind();
		a = load_any_byte();
		lda_bit_set();
		break;
	case 0xA2:                                                          // ldx #
		x = read_next_imm();
		ldx_bit_set();
		break;
	case 0xA6:                                                          // ldx zp
		read_next_zp();
		x = load_ram_byte();
		ldx_bit_set();
		break;
	case 0xB6:                                                          // ldx zp, y
		read_next_zp_y();
		x = load_any_byte();
		ldx_bit_set();
		break;
	case 0xAE:                                                          // ldx abs
		read_next_abs();
		x = load_any_byte();
		ldx_bit_set();
		break;
	case 0xBE:                                                          // ldx abs, y
		read_next_abs_y();
		x = load_any_byte();
		ldx_bit_set();
		break;
	case 0xA0:                                                          // ldy #
		y = read_next_imm();
		ldy_bit_set();
		break;
	case 0xA4:                                                          // ldy zp
		read_next_zp();
		y = load_ram_byte();
		ldy_bit_set();
		break;
	case 0xB4:                                                          // ldy zp, x
		read_next_zp_x();
		y = load_any_byte();
		ldy_bit_set();
		break;
	case 0xAC:                                                          // ldy abs
		read_next_abs();
		y = load_any_byte();
		ldy_bit_set();
		break;
	case 0xBC:                                                          // ldy abs, x
		read_next_abs_x();
		y = load_any_byte();
		ldy_bit_set();
		break;
	case 0x85:                                                          // sta zp
		read_next_zp();
		write_ram_byte(a);
		break;
	case 0x95:                                                          // sta zp, x
		read_next_zp_x();
		write_ram_byte(a);
		break;
	case 0x8D:                                                          // sta abs
		read_next_abs();
		write_any_byte(a);
		break;
	case 0x9D:                                                          // sta abs, x
		read_next_abs_x();
		write_any_byte(a);
		break;
	case 0x99:                                                          // sta abs, y
		read_next_abs_y();
		write_any_byte(a);
		break;
	case 0x81:                                                          // sta (zp, x)
		read_next_zp_idx_ind();
		write_any_byte(a);
		break;
	case 0x91:                                                          // sta (zp), y
		read_next_zp_ind_idx();
		write_any_byte(a);
		break;
	case 0x92:                                                          // sta (zp)     *
		read_next_zp_ind();
		write_any_byte(a);
		break;
	case 0x64:                                                          // stz zp       *
		read_next_zp();
		write_any_byte(0);
		break;
	case 0x74:                                                          // stz zp, x    *
		read_next_zp_x();
		write_ram_byte(0);
		break;
	case 0x9C:                                                          // stz abs      *
		read_next_abs();
		write_any_byte(0);
		break;
	case 0x9E:                                                          // stz abs, x   *
		read_next_abs_x();
		write_any_byte(0);
		break;
	case 0x86:                                                          // stx zp
		read_next_zp();
		write_ram_byte(x);
		break;
	case 0x96:                                                          // stx zp, y
		read_next_zp_y();
		write_any_byte(x);
		break;
	case 0x8E:                                                          // stx abs
		read_next_abs();
		write_any_byte(x);
		break;
	case 0x84:                                                          // sty zp
		read_next_zp();
		write_ram_byte(y);
		break;
	case 0x94:                                                          // sty zp, x
		read_next_zp_x();
		write_ram_byte(y);
		break;
	case 0x8C:                                                          // sty abs
		read_next_abs();
		write_any_byte(y);
		break;
	case 0x0F:                                                          // bbr0 *
	case 0x1F:                                                          // bbr1 *
	case 0x2F:                                                          // bbr2 *
	case 0x3F:                                                          // bbr3 *
	case 0x4F:                                                          // bbr4 *
	case 0x5F:                                                          // bbr5 *
	case 0x6F:                                                          // bbr6 *
	case 0x7F:                                                          // bbr7 *
		if(!CHECK_BIT(load_ram_byte(read_next_zp()), HIGH_NIBBLE(op_code)))
			pc += static_cast<int8_t>(read_next_imm());
		else
			++pc;
		break;
	case 0x8F:                                                          // bbs0 *
	case 0x9F:                                                          // bbs1 *
	case 0xAF:                                                          // bbs2 *
	case 0xBF:                                                          // bbs3 *
	case 0xCF:                                                          // bbs4 *
	case 0xDF:                                                          // bbs5 *
	case 0xEF:                                                          // bbs6 *
	case 0xFF:                                                          // bbs7 *
		if(CHECK_BIT(load_ram_byte(read_next_zp()), HIGH_NIBBLE(op_code - 0x80)))
			pc += static_cast<int8_t>(read_next_imm());
		else
			++pc;
		break;
	case 0xB0:                                                          // bcs
		if(CHECK_BIT(p, CarryBit)) 
			pc += static_cast<int8_t>(read_next_imm());
		else
			++pc;
		break;
	case 0x90:                                                          // bcc
		if(!CHECK_BIT(p, CarryBit)) 
			pc += static_cast<int8_t>(read_next_imm());
		else
			++pc;
		break;
	case 0xF0:                                                          // beq
		if(CHECK_BIT(p, ZeroBit)) 
			pc += static_cast<int8_t>(read_next_imm());
		else
			++pc;
		break;
	case 0xD0:                                                          // bne
		if(!CHECK_BIT(p, ZeroBit)) 
			pc += static_cast<int8_t>(read_next_imm());
		else
			++pc;
		break;
	case 0x30:                                                          // bmi
		if(CHECK_BIT(p, NegativeBit)) 
			pc += static_cast<int8_t>(read_next_imm());
		else
			++pc;
		break;
	case 0x10:                                                          // bpl
		if(!CHECK_BIT(p, NegativeBit)) 
			pc += static_cast<int8_t>(read_next_imm());
		else
			++pc;
		break;
	case 0x70:                                                          // bvs
		if(CHECK_BIT(p, OverflowBit)) 
			pc += static_cast<int8_t>(read_next_imm());
		else
			++pc;
		break;
	case 0x50:                                                          // bvc
		if(!CHECK_BIT(p, OverflowBit))
			pc += static_cast<int8_t>(read_next_imm());
		else
			++pc;
		break;
	case 0x80:                                                          // bra
		pc += static_cast<int8_t>(read_next_imm());
		break;
	case 0x48:                                                          // pha
		write_stack_byte(a);
		break;
	case 0x08:                                                          // php
		write_stack_byte(p);
		break;
	case 0xDA:                                                          // phx *
		write_stack_byte(x);
		break;
	case 0x5A:                                                          // phy *
		write_stack_byte(y);
		break;
	case 0x68:                                                          // pla
		a = load_stack_byte();
		pla_bit_set();
		break;
	case 0x28:                                                          // plp
		p = load_stack_byte() | Break | Unused;
		break;
	case 0xFA:                                                          // plx *
		x = load_stack_byte();
		plx_bit_set();
		break;
	case 0x7A:                                                          // ply *
		y = load_stack_byte();
		ply_bit_set();
		break;
	case 0x9A:                                                          // txs
		s = x;
		break;
	case 0xBA:                                                          // tsx
		x = s;
		tsx_bit_set();
		break;
	case 0x8A:                                                          // txa
		a = x;
		txa_bit_set();
		break;
	case 0xAA:                                                          // tax
		x = a;
		tax_bit_set();
		break;
	case 0xA8:                                                          // tay
		y = a;
		tay_bit_set();
		break;
	case 0x98:                                                          // tya
		a = y;
		tya_bit_set();
		break;
	case 0x07:                                                          // rmb0 *
	case 0x17:                                                          // rmb1 *
	case 0x27:                                                          // rmb2 *
	case 0x37:                                                          // rmb3 *
	case 0x47:                                                          // rmb4 *
	case 0x57:                                                          // rmb5 *
	case 0x67:                                                          // rmb6 *
	case 0x77:                                                          // rmb7 *
		if(is_ram(read_next_zp()))
			CLEAR_BIT(RAM[tmp_word], HIGH_NIBBLE(op_code));
		break;
	case 0x87:                                                          // smb0 *
	case 0x97:                                                          // smb1 *
	case 0xA7:                                                          // smb2 *
	case 0xB7:                                                          // smb3 *
	case 0xC7:                                                          // smb4 *
	case 0xD7:                                                          // smb5 *
	case 0xE7:                                                          // smb6 *
	case 0xF7:                                                          // smb7 *
		if(is_ram(read_next_zp()))
			SET_BIT(RAM[tmp_word], HIGH_NIBBLE(op_code - 0x80));
		break;
	case 0x18:                                                          // clc
		CLEAR_BIT(p, CarryBit);
		break;
	case 0xD8:                                                          // cld
		CLEAR_BIT(p, DecimalBit);
		break;
	case 0x58:                                                          // cli
		CLEAR_BIT(p, InterruptBit);
		break;
	case 0xB8:                                                          // clv
		CLEAR_BIT(p, OverflowBit);
		break;
	case 0x38:                                                          // sec
		SET_BIT(p, CarryBit);
		break;
	case 0xF8:                                                          // sed
		SET_BIT(p, DecimalBit);
		break;
	case 0x78:                                                          // sei
		SET_BIT(p, InterruptBit);
		break;
	case 0x3A:                                                          // dec *
		dec_bit_set(--a);
		break;
	case 0xCA:                                                          // dex
		dec_bit_set(--x);
		break;
	case 0x88:                                                          // dey
		dec_bit_set(--y);
		break;
	case 0xC6:                                                          // dec zp
		read_next_zp();
		dec_bit_set(--RAM[tmp_word]);
		events_write(tmp_word);
		break;
	case 0xD6:                                                          // dec zp, x
		read_next_zp_x();
		dec_bit_set(--RAM[tmp_word]);
		events_write(tmp_word);
		break;
	case 0xCE:                                                          // dec abs
		read_next_abs();
		if(!is_ram()) break;
		dec_bit_set(--RAM[tmp_word]);
		events_write(tmp_word);
		break;
	case 0xDE:                                                          // dec abs, x
		read_next_abs_x();
		if(!is_ram()) break;
		dec_bit_set(--RAM[tmp_word]);
		events_write(tmp_word);
		break;
	case 0x1A:                                                          // inc A *
		inc_bit_set(++a);
		break;
	case 0xE8:                                                          // inx
		inc_bit_set(++x);
		break;
	case 0xC8:                                                          // iny
		inc_bit_set(++y);
		break;
	case 0xE6:                                                          // inc zp
		read_next_zp();
		inc_bit_set(++RAM[tmp_word]);
		events_write(tmp_word);
		break;
	case 0xF6:                                                          // inc zp, x
		read_next_zp_x();
		inc_bit_set(++RAM[tmp_word]);
		events_write(tmp_word);
		break;
	case 0xEE:                                                          // inc abs
		read_next_abs();
		if(!is_ram()) break;
		inc_bit_set(++RAM[tmp_word]);
		events_write(tmp_word);
		break;
	case 0xFE:                                                          // inc abs, x
		read_next_abs_x();
		if(!is_ram()) break;
		inc_bit_set(++RAM[tmp_word]);
		events_write(tmp_word);
		break;
	case 0x6C:                                                          // jmp (abs)
		tmp_word = load_any_word(read_next_abs());
		pc = tmp_word;
		break;
	case 0x7C:                                                          // jmp (abs, x)
		tmp_word = load_any_word(read_next_abs_x());
		pc = tmp_word;
		break;
	case 0x20:                                                          // jsr abs
		write_stack_word(pc + 2);
	case 0x4C:                                                          // jmp abs
		read_next_abs();
		pc = tmp_word;
		break;
	case 0x40:                                                          // rti
		//https://www.masswerk.at/6502/6502_instruction_set.html#RTI
		p = load_stack_byte() | Unused | Break;
	case 0x60:                                                          // rts
		pc = load_stack_word();
		break;
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
	cpy_end:
		cpy_bit_set();
		break;
	cpx_end:
		cpx_bit_set();
		break;
	cmp_end:
		cmp_bit_set();
		break;
	case 0x0A:                                                          // asl a
		SET_BIT_TO(p, CarryBit, CHECK_BIT(a, 7));
		asl_bit_set(a <<= 1);
		break;
	case 0x06:                                                          // asl zp
		read_next_zp();
		if(!is_ram()) break;
		SET_BIT_TO(p, CarryBit, CHECK_BIT(RAM[tmp_word], 7));
		asl_bit_set((RAM[tmp_word] <<= 1));
		events_write(tmp_word);
		break;
	case 0x16:                                                          // asl zp, x
		read_next_zp_x();
		SET_BIT_TO(p, CarryBit, CHECK_BIT(RAM[tmp_word], 7));
		asl_bit_set((RAM[tmp_word] <<= 1));
		events_write(tmp_word);
		break;
	case 0x0E:                                                          // asl abs
		read_next_abs();
		if(!is_ram()) break;
		SET_BIT_TO(p, CarryBit, CHECK_BIT(RAM[tmp_word], 7));
		asl_bit_set((RAM[tmp_word] <<= 1));
		events_write(tmp_word);
		break;
	case 0x1E:                                                          // asl abs, x
		read_next_abs_x();
		if(!is_ram()) break;
		SET_BIT_TO(p, CarryBit, CHECK_BIT(RAM[tmp_word], 7));
		asl_bit_set((RAM[tmp_word] <<= 1));
		events_write(tmp_word);
		break;
	case 0x4A:                                                          // lsr a
		SET_BIT_TO(p, CarryBit, CHECK_BIT(a, 0));
		lsr_bit_set(a >>= 1);
		break;
	case 0x46:                                                          // lsr zp
		read_next_zp();
		SET_BIT_TO(p, CarryBit, CHECK_BIT(RAM[tmp_word], 0));
		lsr_bit_set((RAM[tmp_word] >>= 1));
		events_write(tmp_word);
		break;
	case 0x56:                                                          // lsr zp, x
		read_next_zp_x();
		SET_BIT_TO(p, CarryBit, CHECK_BIT(RAM[tmp_word], 0));
		lsr_bit_set((RAM[tmp_word] >>= 1));
		events_write(tmp_word);
		break;
	case 0x4E:                                                          // lsr abs
		read_next_abs();
		if(!is_ram()) break;
		SET_BIT_TO(p, CarryBit, CHECK_BIT(RAM[tmp_word], 0));
		lsr_bit_set((RAM[tmp_word] >>= 1));
		events_write(tmp_word);
		break;
	case 0x5E:                                                          // lsr abs, x
		read_next_abs_x();
		if(!is_ram()) break;
		SET_BIT_TO(p, CarryBit, CHECK_BIT(RAM[tmp_word], 0));
		lsr_bit_set((RAM[tmp_word] >>= 1));
		events_write(tmp_word);
		break;
	case 0x2A:                                                          // rol a
		SET_BIT_TO(p, UnusedBit, CHECK_BIT(p, CarryBit)); // XD
		SET_BIT_TO(p, CarryBit, CHECK_BIT(a, 7));
		a = (a << 1) | (CHECK_BIT(p, UnusedBit) >> UnusedBit);
		rol_bit_set(a);
		break;
	case 0x26:                                                          // rol zp
		read_next_zp();
		goto rol_mem;
	case 0x36:                                                          // rol zp, x
		read_next_zp_x();
		goto rol_mem;
	case 0x2E:                                                          // rol abs
		read_next_abs();
		goto rol_mem;
	case 0x3E:                                                          // rol abs, x
		read_next_abs_x();
	rol_mem:
		if(!is_ram()) break;
		SET_BIT_TO(p, UnusedBit, CHECK_BIT(p, CarryBit)); // XD
		SET_BIT_TO(p, CarryBit, CHECK_BIT(load_ram_byte(), 7));
		write_ram_byte((load_ram_byte() << 1) | (CHECK_BIT(p, UnusedBit) >> UnusedBit));
		rol_bit_set(load_ram_byte());
		break;
	case 0x6A:                                                          // ror a
		SET_BIT_TO(p, UnusedBit, CHECK_BIT(p, CarryBit)); // XD
		SET_BIT_TO(p, CarryBit, CHECK_BIT(a, 0));
		a = (a >> 1) | (CHECK_BIT(p, UnusedBit) << (7 - UnusedBit));
		ror_bit_set(a);
		break;
	case 0x66:                                                          // ror zp
		read_next_zp();
		goto ror_mem;
	case 0x76:                                                          // ror zp, x
		read_next_zp_x();
		goto ror_mem;
	case 0x6E:                                                          // ror abs
		read_next_abs();
		goto ror_mem;
	case 0x7E:                                                          // ror abs, x
		read_next_abs_x();
	ror_mem:
		if(!is_ram()) break;
		SET_BIT_TO(p, UnusedBit, CHECK_BIT(p, CarryBit)); // XD
		SET_BIT_TO(p, CarryBit, CHECK_BIT(load_ram_byte(), 0));
		write_ram_byte((load_ram_byte() >> 1) | (CHECK_BIT(p, UnusedBit) << (7 - UnusedBit)));
		ror_bit_set(load_ram_byte());
		break;
	case 0x89:                                                          // bit # *
		read_next_imm();
		bit_bit_set();
		break;
	case 0x24:                                                          // bit zp
		read_next_zp();
		tmp_word = load_ram_byte();
		bit_bit_set();
		break;
	case 0x34:                                                          // bit zp, x *
		read_next_zp_x();
		tmp_word = load_ram_byte();
		bit_bit_set();
		break;
	case 0x2C:                                                          // bit abs
		read_next_abs();
		tmp_word = load_any_byte();
		bit_bit_set();
		break;
	case 0x3C:                                                          // bit abs, x *
		read_next_abs_x();
		tmp_word = load_any_byte();
		bit_bit_set();
		break;
	case 0x14:                                                          // trb zp *
	case 0x04:                                                          // tsb zp *
		read_next_zp();
		tsb_trb_bit_set(load_ram_byte());
		write_ram_byte((op_code > 0x10) ? ((~a) & load_ram_byte()) : (a | load_ram_byte()));
		break;
	case 0x1C:                                                          // trb abs *
	case 0x0C:                                                          // tsb abs *
		read_next_abs();
		tsb_trb_bit_set(load_any_byte());
		write_any_byte((op_code > 0x10) ? ((~a) & load_ram_byte()) : (a | load_ram_byte()));
		break;
	case 0x29:                                                          // and #
		read_next_imm();
		a &= tmp_word;
		and_bit_set();
		break;
	case 0x25:                                                          // and zp
		read_next_zp();
		tmp_word = load_ram_byte();
		a &= tmp_word;
		and_bit_set();
		break;
	case 0x35:                                                          // and zp, x
		read_next_zp_x();
		tmp_word = load_ram_byte();
		a &= tmp_word;
		and_bit_set();
		break;
	case 0x2D:                                                          // and abs
		read_next_abs();
		tmp_word = load_any_byte();
		a &= tmp_word;
		and_bit_set();
		break;
	case 0x3D:                                                          // and abs, x
		read_next_abs_x();
		tmp_word = load_any_byte();
		a &= tmp_word;
		and_bit_set();
		break;
	case 0x39:                                                          // and abs, y
		read_next_abs_y();
		tmp_word = load_any_byte();
		a &= tmp_word;
		and_bit_set();
		break;
	case 0x21:                                                          // and (zp, x)
		read_next_zp_idx_ind();
		tmp_word = load_any_byte();
		a &= tmp_word;
		and_bit_set();
		break;
	case 0x31:                                                          // and (zp), y
		read_next_zp_ind_idx();
		tmp_word = load_any_byte();
		a &= tmp_word;
		and_bit_set();
		break;
	case 0x32:                                                          // and (zp)    *
		read_next_zp_ind();
		tmp_word = load_any_byte();
		a &= tmp_word;
		and_bit_set();
		break;
	case 0x49:                                                          // eor #
		a ^= read_next_imm();
		eor_bit_set();
		break;
	case 0x45:                                                          // eor zp
		read_next_zp();
		tmp_word = load_ram_byte();
		a ^= tmp_word;
		eor_bit_set();
		break;
	case 0x55:                                                          // eor zp, x
		read_next_zp_x();
		tmp_word = load_ram_byte();
		a ^= tmp_word;
		eor_bit_set();
		break;
	case 0x4D:                                                          // eor abs
		read_next_abs();
		tmp_word = load_any_byte();
		a ^= tmp_word;
		eor_bit_set();
		break;
	case 0x5D:                                                          // eor abs, x
		read_next_abs_x();
		tmp_word = load_any_byte();
		a ^= tmp_word;
		eor_bit_set();
		break;
	case 0x59:                                                          // eor abs, y
		read_next_abs_y();
		tmp_word = load_any_byte();
		a ^= tmp_word;
		eor_bit_set();
		break;
	case 0x41:                                                          // eor (zp, x)
		read_next_zp_idx_ind();
		tmp_word = load_any_byte();
		a ^= tmp_word;
		eor_bit_set();
		break;
	case 0x51:                                                          // eor (zp), y
		read_next_zp_ind_idx();
		tmp_word = load_any_byte();
		a ^= tmp_word;
		eor_bit_set();
		break;
	case 0x52:                                                          // eor (zp)    *
		read_next_zp_ind();
		tmp_word = load_any_byte();
		a ^= tmp_word;
		eor_bit_set();
		break;
	case 0x09:                                                          // ora #
		read_next_imm();
		a |= tmp_word;
		ora_bit_set();
		break;
	case 0x05:                                                          // ora zp
		read_next_zp();
		tmp_word = load_ram_byte();
		a |= tmp_word;
		ora_bit_set();
		break;
	case 0x15:                                                          // ora zp, x
		read_next_zp_x();
		tmp_word = load_ram_byte();
		a |= tmp_word;
		ora_bit_set();
		break;
	case 0x0D:                                                          // ora abs
		read_next_abs();
		tmp_word = load_any_byte();
		a |= tmp_word;
		ora_bit_set();
		break;
	case 0x1D:                                                          // ora abs, x
		read_next_abs_x();
		tmp_word = load_any_byte();
		a |= tmp_word;
		ora_bit_set();
		break;
	case 0x19:                                                          // ora abs, y
		read_next_abs_y();
		tmp_word = load_any_byte();
		a |= tmp_word;
		ora_bit_set();
		break;
	case 0x11:                                                          // ora (zp), y
		read_next_zp_idx_ind();
		tmp_word = load_any_byte();
		a |= tmp_word;
		ora_bit_set();
		break;
	case 0x01:                                                          // ora (zp, x)
		read_next_zp_ind_idx();
		tmp_word = load_any_byte();
		a |= tmp_word;
		ora_bit_set();
		break;
	case 0x12:                                                          // ora (zp)    *
		read_next_zp_ind();
		tmp_word = load_any_byte();
		a |= tmp_word;
		ora_bit_set();
		break;
	case 0x69:                                                          // adc #
		read_next_imm();
		goto adc_end;
	case 0x65:                                                          // adc zp
		read_next_zp();
		goto adc_mem;
	case 0x75:                                                          // adc zp, x
		read_next_zp_x();
		goto adc_mem;
	case 0x6D:                                                          // adc abs
		read_next_abs();
		goto adc_mem;
	case 0x7D:                                                          // adc abs, x
		read_next_abs_x();
		goto adc_mem;
	case 0x79:                                                          // adc abs, y
		read_next_abs_y();
		goto adc_mem;
	case 0x61:                                                          // adc (zp, x)
		read_next_zp_idx_ind();
		goto adc_mem;
	case 0x71:                                                          // adc (zp), y
		read_next_zp_ind_idx();
		goto adc_mem;
	case 0x72:                                                          // adc (zp)    *
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
		break;
	case 0xE9:                                                          // sbc #
		read_next_imm();
		goto sbc_end;
	case 0xE5:                                                          // sbc zp
		read_next_zp();
		goto sbc_mem;
	case 0xF5:                                                          // sbc zp, x
		read_next_zp_x();
		goto sbc_mem;
	case 0xED:                                                          // sbc abs
		read_next_abs();
		goto sbc_mem;
	case 0xFD:                                                          // sbc abs, x
		read_next_abs_x();
		goto sbc_mem;
	case 0xF9:                                                          // sbc abs, y
		read_next_abs_y();
		goto sbc_mem;
	case 0xF1:                                                          // sbc (zp), y
		read_next_zp_idx_ind();
		goto sbc_mem;
	case 0xE1:                                                          // sbc (zp, x)
		read_next_zp_ind_idx();
		goto sbc_mem;
	case 0xF2:                                                          // sbc (zp)    *
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
		break;
	default:                                                          // nop
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
}