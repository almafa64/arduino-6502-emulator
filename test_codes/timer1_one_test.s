DDRA = $0203		; port A input/output mode
DDRB = $0202		; port B input/output mode
PORTA = $0200		; port A data
PORTB = $0201		; port B data

E  = %00000100		; led display enable
RW = %00000010		; led display read/write
RS = %00000001		; led display register select

T1CL = $0204		; t1 clock low byte
T1CH = $0205		; t1 clock high byte

ACR = $020B
IFR = $020D

	.org $8000
	
begin:
	dec DDRA
	stz PORTA
	stz ACR
	ldx #%00100000	; A5 pin

loop:
	stx PORTA
	jsr delay
	stz PORTA
	jsr delay
	bra loop

delay:
	lda #$50
	sta T1CL
	lda #$c3
	sta T1CH
delay1:
	bit IFR
	bvc delay1
	lda T1CL
	rts

irqb:
	rti

	.org $fffa
	.word $0000
	.word begin
	.word irqb