;          APPLEMOUSE II SAMPLE PROGRAM

           .ORG $1000

;******************************************************
;*                                                    *
;* AppleMouse II sample program                       *
;*    1. Tests for an AppleMouse card in the system;  *
;*         if none an error message is displayed.     *
;*    2. Turns the Mouse on in passive mode.          *
;*    3. Does continual READMOUSEs and displays       *
;*         the data in the following form:            *
;*                                                    *
;*      X=xxxx   Y=yyyy   B=bb   x,y,b are in hex     *
;*                                                    *
;*    4. When a key on the Apple is pressed the       *
;*         mode is set to 0 (mouse is turned off).    *
;*                                                    *
;*                                                    *
;******************************************************

PRNTAX     = $F941  ;A=1st byte - X=2nd byte
TMP        = $6     ;Zero page tmps are $6/$7
TOP        = $22    ;Top of screen variable
CH         = $24    ;Cursor horizontal position
CV         = $25    ;Cursor vertical position
WARM       = $300   ;Restart ProDOS
VTAB       = $FC22  ;Moves cursor to CH/CV
KEY        = $C000  ;- if key pressed
KEYSTROBE  = $C010  ;Clears keystrobe
CROUT      = $FD8E  ;Output a carriage return
PRBYTE     = $FDDA  ;Prints A reg in Hex
PRBLNK     = $F948  ;Prints 3 blank spaces
RDKEY      = $FD0C  ;Wait for a keypress
COUT       = $FDED  ;Output 1 character to screen
TEXT       = $FB39  ;Set screen for text
HOME       = $FC58  ;Clear screen
XL         = $3B8   ;+Cn=Low byte of abs X pos
YL         = $438   ;+Cn=Low byte of abs Y pos
XH         = $4B8   ;+Cn=High byte of abs X pos
YH         = $538   ;+Cn=High byte of abs Y pos
BUTTON     = $6B8   ; +Cn=Button status
;                 Bit 7 = current status
;                 Bit 6 = status on last READMOUSE

;  Table of offsets to mouse entry points
;
SETMOUSE   = $12
SERVEMOUSE = $13
READMOUSE  = $14
CLEARMOUSE = $15
POSMOUSE   = $16
INITMOUSE  = $19
CLAMPMOUSE = $17
HOMEMOUSE  = $18

;******************************************************
;*  MAIN routine                                      *
;*                                                    *
;******************************************************

BEGIN      = *
           JSR  SETUP      ;Clear screen, type header
           JSR  CHECKSLOTS ;Which slot has the mouse?
            
;   Call INITMOUSE to start mouse card
           LDY  #INITMOUSE ;Pick up offset to offset
           JSR  CALLCARD   ;Call the Mouse card

;   Call SETMOUSE to turn card on
           LDY  #SETMOUSE  ;Pick up offset to offset
           LDA  #$01       ;Set passive mode
           JSR  CALLCARD   ;Call the Mouse card
           BIT  KEYSTROBE  ;Kill any keys hit

MAINLOOP   = *
           LDY  #READMOUSE ;Pick up offset to offset
           LDA  #$88       ;Dont care about A
           JSR  CALLCARD   ;Call the Mouse card

           JSR  PRINTLINE  ;Print current mouse data

           BIT  KEY        ;Test for stop
           BPL  MAINLOOP   ;No keypress: Read again

ALLDONE:   BIT  KEYSTROBE  ;Kill key hit
           LDY  #SETMOUSE  ;Pick up offset to offset
           LDA  #$00       ;Set off mode
           JSR  CALLCARD   ;Call the Mouse card
           JSR  TEXT       ;Set screen to normal
           LDY  #$00       ;Show restart message
RSTMSGLP   = *
           LDA  RSTMSG,Y   ;Pick up char
           BEQ  FINALLY
           JSR  COUT       ;Show it
           INY
           BNE  RSTMSGLP
FINALLY    = *
           JMP  WARM       ;Restart ProDOS
           
CN:        .BYTE 1,0       ;Cn is stored here
N0:        .BYTE 1,0       ;n0 is stored here
TOCARD:    JMP $0000       ;operand modified by prgm

;******************************************************
;*  Subroutines                                       *
;*                                                    *
;******************************************************


;******************************************************
;*  CALLCARD                                          *
;*      Routine which sets up registers for indirect  *
;*        jump to the Mouse firmware                  *
;*      User loads offset to offset in Y reg          *
;*        A contains byte to be passed to firmware    *
;*                                                    *
;******************************************************

CALLCARD   = *
           PHA             ;Save user information
           LDA  (TMP),Y    ;Find offset
           LDX  CN         ;Set up registers for ...
           LDY  N0         ;... what firmware expects
           STA  TOCARD+1   ;Store for ...
           STX  TOCARD+2   ; ...indirect jump
           PLA
           JSR  TOCARD     ;Jump indirect to card
           RTS

;******************************************************
;*  SETUP                                             *
;*      Clears the text screen, prints header         *
;*                                                    *
;******************************************************
SETUP      = *
           JSR  TEXT       ;Set screen to text
           JSR  HOME       ;Clear screen
           LDY  #$00       ;Put up message on screen
HDRLOOP    = *
           LDA  HEADER,Y
           BEQ  BEGIN1     ;0 is terminator
           JSR  COUT       ;Output it to screen
           INY
           BNE  HDRLOOP    ;Go for more
BEGIN1     = *
           LDA  #$03       ;Set top of screen
           STA  TOP
           LDA  #$00       ;Tab down and set new top
           STA  CH
           LDA  #$03
           STA  CV
           JSR  VTAB
           RTS

;******************************************************
;*                                                    *
;*  CHECKSLOTS                                        *
;*       Routine to check the 7 peripheral slots      *
;*         for the signature bytes of the Mouse card  *
;*         $Cn0C = $20                                *
;*         $CnFB = $D6                                *
;*       Returns values for $Cn and $n0               *
;*         in user defined location CN and N0         *
;*                                                    *
;******************************************************
CHECKSLOTS = *
           LDX  #$07       ;Test 7 slots only
           LDA  #$00       ;Lo byte is zip
           STA  TMP
           LDA  #$C8       ;Max slot+1
           STA  TMP+1
LOOP1      = *
           DEC  TMP+1      ;Decrement Cn
           DEX             ;Decrement # to do
           BMI  EXIT       ;Error exit if here
           LDY  #$0C       ;1st ID byte is here
           LDA  (TMP),Y    ;What do we have?
           CMP  #$20       ;1st ID byte is $20
           BNE  LOOP1      ;If no match try next card
           LDY  #$FB       ;Last ID byte is here
           LDA  (TMP),Y    ;What do we have?
           CMP  #$D6       ;Must be a 'V' to match
           BNE  LOOP1      ;If no match try next card
           LDA  TMP+1      ;If match fall to here
           STA  TOCARD+2   ;Save for later
           STA  CN
           ASL             ;Make $n0 for Y
           ASL
           ASL
           ASL
           STA  N0         ;Save for later
           BNE  START      ;Go start program

EXIT       = *             ;Error loop exit
           JSR  TEXT       ;Set text mode
           JSR  HOME       ;Clear the screen
           LDY  #$00       ;Set index to 00
ERRLOOP    = *
           LDA  NOCARDMSG,Y
           BEQ  WARMSTART
           JSR  COUT
           INY             ;Go to next char to show
           BNE  ERRLOOP
WARMSTART  = *
           JMP  WARM       ; Restart ProDOS
START:     RTS

;******************************************************
;*  PRINTLINE                                         *
;*     Prints a line of Mouse data                    *
;*                                                    *
;******************************************************
PRINTLINE  = *
           LDA  #'X'+$80
           JSR  COUT       ;Print an X
           LDA  #'='+$80
           JSR  COUT       ;Print an '='
           LDX  TOCARD+2
           LDY  XH,X
           LDA  XL,X
           TAX
           TYA             ;Set high value in A
           JSR  PRNTAX     ;Print X value
           JSR  PRBLNK     ;Print 3 blanks
           LDA  #'Y'+$80
           JSR  COUT       ;Print a Y
           LDA  #'='+$80
           JSR  COUT       ;Print '='
           LDX  TOCARD+2   ;Pick up $Cn
           LDY  YH,X
           LDA  YL,X
           TAX
           TYA             ;Set high value in A
           JSR  PRNTAX     ;Print Y value
           JSR  PRBLNK     ;Print 3 blanks
           LDA  #'B'+$80
           JSR  COUT       ;Print a B
           LDA  #'='+$80
           JSR  COUT       ;Print '='
           LDX  TOCARD+2   ;Pick up $Cn
           LDA  BUTTON,X
           JSR  PRBYTE     ;Print button status
           JSR  CROUT      ;Go to next line
           RTS

;******************************************************
;*  Messages                                          *
;*                                                    *
;******************************************************

           .MACRO   ASCHI STR
           .REPEAT  .STRLEN (STR), C
           .BYTE    .STRAT (STR, C) | $80
           .ENDREP
           .ENDMACRO
NOCARDMSG  = *
           .BYTE $8D,$8D
           ASCHI "THERE IS NO APPLEMOUSE CARD INSTALLED!"
           .BYTE $8D,$87,$87,$87,$00

HEADER     = *
           ASCHI "***** APPLEMOUSE TEST ROUTINE *****"
           .BYTE $8D
           ASCHI "           PRESS ANY KEY TO STOP."
           .BYTE $8D,$00
           
RSTMSG     =*
           ASCHI "TYPE CALL 4096 AND RETURN TO RESTART."
           .BYTE $8D,$00

