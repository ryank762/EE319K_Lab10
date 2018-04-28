; Print.s
; Student names: Ryan Kim
; Last modification date: 3/28/2018
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

Digit0	EQU		0
Digit1	EQU		4
Digit2	EQU		8
Digit3	EQU		12
Digit4	EQU		16
Digit5	EQU		20
Digit6	EQU		24
Digit7	EQU		28
Digit8	EQU		32
Digit9	EQU		36	 
  
;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
	PUSH	{R0-R12, LR}
	MOV		R2, R0					; save input
	MOV		R6, #0					; init R6
	MOV		R7, #0					; flag which is set when first nonzero digit is detected
; allocation phase
	SUB		SP, #40					; allocate stack
; access phase
	BL		Space_Find
	MOV		R4, R0					; save output of Space_Find
	SUB		R4, #1
	MOV		R8, R4					; R8 = (# of digits)-1
OutDec_Again
	MOV		R1, R4
	MOV		R0, #10
	BL		Exponentiation
	MOV		R5, R0					; save output of Exp
	MOV		R1, R0					; AAPCS parameter passing
	MOV		R0, R2					; AAPCS parameter passing
	BL		Division_Algorithm		; get digit
	STR		R0, [SP, R6]
	MUL		R0, R5
	SUB		R2, R0					; "remove" most recently pushed digit from number
	ADD		R6, #4					; pointer to next element on stack
	SUBS	R4, #1					; reduce number of remaining digits to push by 1
	BGE		OutDec_Again			; repeat until all digits are pushed to stack
	MOV		R6, #4
	MUL		R8, R6					; R8 = 4*[(# of digits)-]
	MOV		R6, #0					; init R6 to 0
Print_Again
	CMP		R6, R8
	BEQ		PrintOnesDigit			; always print ones digit at the end no matter what
	LDR		R0, [SP, R6]			; pop element on stack
	CMP		R7, #0					; see if we still need to check for extraneous zeroes
	BGT		Print_Now				; 0 = keep checking, 1 = print whatever is remaining
	CMP		R0, #0
	BEQ		Skip_Print				; check if popped element is a number between 0-9
	CMP		R0, #9	
	BGT		Skip_Print				; check if popped element is a number between 0-9
	MOV		R7, #1
Print_Now
	ADD		R0, #0x30				; ASCII value for number
	BL		ST7735_OutChar			; print to LCD
	MOV		R7, #1					; set flag to 1 to skip checking for extraneous zeroes
Skip_Print
	ADD		R6, #4					; update pointer
	B		Print_Again
PrintOnesDigit
	LDR		R0, [SP, R8]			; pop ones digit
	ADD		R0, #0x30				; ASCII value for number
	BL		ST7735_OutChar			; print to LCD
Print_Done
; deallocation phase
	ADD		SP, #40					; deallocate stack
	POP		{R0-R12, LR}			; restore regs
	BX		LR						; return

;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
	PUSH	{R0-R12, LR}
; allocation phase
	SUB		SP, #16					; allocate stack
	MOV		R4, R0					; save input
	MOV		R1, #0x2E				; period in ASCII
; access phase
	STR		R1, [SP, #Digit3]
	BL		Space_Find
	CMP		R0, #4					; either 4 digits or more
	BGT		OutFix_Over
; access thousands digit
	MOV		R0, R4					; load input
	MOV		R1, #1000				; AAPCS parameter passing
	BL		Division_Algorithm		; determine digit
	ADD		R0, #0x30
	STR		R0, [SP, #Digit4]		; push digit to stack
	SUB		R0, #0x30
	MOV		R1, #1000
	MUL		R0, R1
	SUB		R4, R0
; access hundreds digit
	MOV		R0, R4					; AAPCS parameter passing
	MOV		R1, #100				; AAPCS parameter passing
	BL		Division_Algorithm		; determine digit
	ADD		R0, #0x30
	STR		R0, [SP, #Digit2]		; push digit to stack
	SUB		R0, #0x30
	MOV		R1, #100
	MUL		R0, R1
	SUB 	R4, R0
; access tens digit
	MOV		R0, R4					; AAPCS parameter passing
	MOV		R1, #10					; AAPCS parameter passing
	BL		Division_Algorithm		; determine digit
	ADD		R0, #0x30
	STR		R0, [SP, #Digit1]		; push digit to stack
	SUB		R0, #0x30
	MOV		R1, #10
	MUL		R0, R1
	SUB		R4, R0
; access ones digit
	ADD		R4, #0x30
	STR		R4, [SP, #Digit0]		; push digit to stack
	B		OutFix_Done
OutFix_Over
	MOV		R0, #0x2A				; asterisk in ASCII
	STR		R0, [SP, #Digit0]
	STR		R0, [SP, #Digit1]
	STR		R0, [SP, #Digit2]
	STR		R0, [SP, #Digit4]		; push asterisks into stack
OutFix_Done
	LDR		R0, [SP, #Digit4]		; pop digit
	BL		ST7735_OutChar
	LDR		R0, [SP, #Digit3]		; pop decimal point
	BL		ST7735_OutChar
	LDR		R0, [SP, #Digit2]		; pop digit
	BL		ST7735_OutChar
	LDR		R0, [SP, #Digit1]		; pop digit
	BL		ST7735_OutChar
	LDR		R0, [SP, #Digit0]		; pop digit
	BL		ST7735_OutChar
; deallocation phase
	ADD		SP, #16					; deallocate stack
	POP		{R0-R12, LR}			; restore regs
	BX		LR						; return
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

Space_Find
; Determines the number of digits of input
; Input:	R0 32-bit unsigned number
; Output:	R0 number of digits of input
;
	PUSH	{R2-R12, LR}			; save regs
	MOV		R2, R0					; save input
	MOV		R0, #1					; number of digits of input
	MOV		R3, #10					; comparison parameter
	MOV		R4, #10					; fixed number
Space_Again
	CMP		R3, R2
	BHI		Space_Done				; done if R2<R3
	ADD		R0, #1
	CMP		R0, #10					; 10 digits is maximum
	BGE		Space_Done				; loop at most 10 times
	MUL		R3, R4					; check next digit
	B		Space_Again
Space_Done
	POP		{R2-R12, LR}			; restore regs
	BX		LR						; return
;

Division_Algorithm
; Applies the division algorithm
; Inputs:	R0 dividend
;			R1 divisor
; Outputs:	R0 quotient
;			R1 remainder
;
	PUSH	{R2-R12, LR}			; save regs
	MOV		R2, R0					; copy of dividend
	MOV		R3, R1					; copy of divisor
	MOV		R0, #0					; quotient = 0 initially
Division_Again
	CMP		R3, R2				
	BHI		Division_Done			; done if dividend < divisor
	ADD		R0, #1					; add 1 to quotient
	SUB		R2, R3					; subtract from dividend
	B		Division_Again
Division_Done
	MOV		R1, R2					; lowest positive dividend is remainder
	POP		{R2-R12, LR}			; restore regs
	BX		LR						; return
;

Exponentiation
; Raises R0 to the R1 power
; Inputs:	R0 base
;			R1 exponent
; Outputs:	R0 = R0^R1
;
	PUSH	{R2-R12, LR}			; save regs
	MOV		R2, R0					; save base
	CMP		R1, #0					; loop R1 times
	BEQ		Exp_Zero
Exp_Again
	SUBS	R1, #1					; decrement R1
	BEQ		Exp_Done
	MUL		R0, R2					; R0*10
	B		Exp_Again
Exp_Zero
	MOV		R0, #1					; if R1=0, then R0^0 = 1
Exp_Done
	POP		{R2-R12, LR}			; restore regs
	BX		LR						; return
;

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file                          ; end of file
