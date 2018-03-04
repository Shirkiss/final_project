; !!! label is a key word !!!
.entry	r3
.extern	PRTSTR
; !!! label allready defined !!!
.entry PRTSTR
; !!! missing parameters !!!
STRADD:	.data
;!!! missing " sign !!! 
STR: .string abcdef"
LASTCHAR: .data	0
LEN: .data 0
K: .data 0

; !!! too many args !!!
jsr/0,0,0 COUNT
; !!! iligal dbl !!!
jsr/0,3	COUNT
; !!! iligal type !!!
jsr/2,0 PRTSTR
; !!! a lot of spaces, thats o.k !!!
mov/1/1/1,0 STR{7},									r3
dec/1/1/1,0 LASTCHAR{*K}
; !!! too many args !!!
inc/0,1 K b
; !!! missing parameters!!!
jsr/0,0

