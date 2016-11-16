	;; example code
	org 8000h		
	mvi c,8h	
	mvi a,0
loop:	add c
	dcr c
	jnz loop
	sta data2
	hlt   ; stop CPU
	
	org 9000h
data1:	db 99h,22h 	
data2:	db ffh,10h,20h
	dw 1234h,cdefh
