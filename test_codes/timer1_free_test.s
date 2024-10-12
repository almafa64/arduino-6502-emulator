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
IER = $020E

ticks = $00
toggle_time = $04

	.org $8000
	
begin:
	dec DDRB
	stz PORTB
	stz ACR
	stz toggle_time
	jsr init_timer

loop:
	jsr update_led
	bra loop

update_led:
	sec
	lda ticks
	sbc toggle_time
	cmp #25		; have 250ms elapsed
	bcc exit_update_led
	lda #$01
	eor PORTB
	sta PORTB	; toggle led
	lda ticks
	sta toggle_time
exit_update_led:
	rts

init_timer:
	stz ticks + 0
	stz ticks + 1
	stz ticks + 2
	stz ticks + 3
	lda #%01000000
	sta ACR
	lda #$0E
	sta T1CL
	lda #$27
	sta T1CH
	lda #%11000000
	sta IER
	cli
	rts

irqb:
	bit T1CL
	inc ticks + 0
	bne end_irq
	inc ticks + 1
	bne end_irq
	inc ticks + 2
	bne end_irq
	inc ticks + 3
end_irq:
	rti

	.org $fffa
	.word $0000
	.word begin
	.word irqb