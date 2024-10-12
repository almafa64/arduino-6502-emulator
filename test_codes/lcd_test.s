DDRA = $0203		; port A input/output mode
DDRB = $0202		; port B input/output mode
PORTA = $0200		; port A data
PORTB = $0201		; port B data

E  = %00000100		; led display enable
RW = %00000010		; led display read/write
RS = %00000001		; led display register select

	.org $8000
	
begin:	
	dec DDRA		; set all pins to output on PORTA
	dec DDRB		; set all pins to output on PORTB
	
	sei
	
	lda #%00111000	; 8 bit mode, 2 line, 5x8 dot
	jsr send_inst
	
	lda #%00000110	; increment + shift cursor, dont shift display
	jsr send_inst
	
	lda #%01000000	; set CGRAM to $00
	jsr send_inst
	
write_custom_chars:	; send custom characters
	lda chars, x
	bmi after_customs
	jsr send_char
	inx
	bra write_custom_chars
	
after_customs:
	ldx #0
	
	lda #%00001110	; display on, cursor on, blink off
	jsr send_inst

	lda #%00000001	; clear display
	jsr send_inst

send_loop:
	lda main_msg, x
	beq pre_loop
	jsr send_char
	inx
	bra send_loop
	
pre_loop:
	lda #%00001100	; display on, cursor off, blink off
	jsr send_inst
	
	ldx #0
	cli
loop:
	cpy #1
	bne loop
	sei
	lda #%00000001	; clear display
	jsr send_inst
send_loop2:
	lda alt_msg, x
	beq end
	jsr send_char
	inx
	bra send_loop2
end:
	stp
	
lcd_wait:
	pha
	inc DDRB
lcd_busy:
	lda #RW
	sta PORTA
	lda #RW | E
	sta PORTA
	bit PORTB
	bmi lcd_busy
	
	lda #RW
	sta PORTA
	dec DDRB
	pla
	rts

send_inst:
	jsr lcd_wait
	sta PORTB
	stz PORTA
	lda #E
	sta PORTA
	stz PORTA
	rts

send_char:
	jsr lcd_wait
	sta PORTB
	lda #RS
	sta PORTA
	lda #RS | E
	sta PORTA
	lda #RS
	sta PORTA
	rts

irqb:
	iny
	rti

main_msg:
	; there is a 24 character gap between the two line
	;.asciiz "xxxxxxxxxxxxxxxx                        xxxxxxxxxxxxxxxx"
	;               16                   24                  16
	;.byte    "   "
	;.byte %11110101 ;ü
	;.byte        "dv"
	;.byte %11101111 ;ö
	;.byte           "z"
	;.byte %11101111 ;ö
	;.asciiz           "llek                             otthonomban!"
	
	;.byte    "   K"
	;.byte %11101111 ;ö
	;.byte         "sz"
	;.byte %11101111 ;ö
	;.byte            "n"
	;.byte %11101111 ;ö
	;.byte              "m a                           felk"
	;.byte %11101111 ;ö
	;.byte                                                 "sz"
	;.byte %11101111 ;ö
	;.asciiz                                                   "nte'st!"
	
	;.asciiz  "   Legyszives                               huzz ki!    "
	.byte    "   L"
	.byte $3 ;é (CGRAM)
	.byte         "gysz"
	.byte $4 ;í (CGRAM)
	.byte              "ves                               h"
	.byte $6 ;ú (CGRAM)
	.asciiz                                                "zz ki!    "

chars:
	.byte $05, $0A, $0E, $11, $11, $11, $0E, $00	; ő		;alt -> 0x05, 0x0A, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E
	.byte $05, $0A, $11, $11, $11, $13, $0D, $00	; ű		;alt -> 0x05, 0x0A, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D
	.byte $02, $04, $0E, $01, $0F, $11, $0F, $00	; á		;alt -> 0x02, 0x04, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F
	.byte $02, $04, $0E, $11, $1F, $10, $0E, $00	; é		;alt -> 0x02, 0x04, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0E
	.byte $02, $04, $00, $0C, $04, $04, $0E, $00	; í		;alt -> 0x02, 0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E
	.byte $02, $04, $0E, $11, $11, $11, $0E, $00	; ó		;alt -> 0x02, 0x04, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E
	.byte $02, $04, $11, $11, $11, $13, $0D, $00	; ú		;alt -> 0x02, 0x04, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D
	.byte $FF										; end

alt_msg:
	.byte    "  NE PISZK"
	.byte $2 ;á (CGRAM)
	.asciiz             "LD                           AZT A GOMBOT!!!!"

	.org $fffa
	.word $0000
	.word begin
	.word irqb