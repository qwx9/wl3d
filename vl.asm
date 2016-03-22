SCREENSEG	=	0a000h

	DATASEG

	EXTRN	TimeCount:WORD ; incremented every 70th of a second
	EXTRN	linewidth:WORD

starttime	dw	?

	CODESEG

;==============
;
; VL_SetCRTC
;
;==============

PROC	VL_SetCRTC  crtc:WORD
PUBLIC	VL_SetCRTC

;
; wait for a display signal to make sure the raster isn't in the middle
; of a sync
;
	cli

	mov	dx,STATUS_REGISTER_1

@@waitdisplay:
	in	al,dx
	test	al,1	;1 = display is disabled (HBL / VBL)
	jnz	@@waitdisplay


;
; set CRTC start
;
; for some reason, my XT's EGA card doesn't like word outs to the CRTC
; index...
;
	mov	cx,[crtc]
	mov	dx,CRTC_INDEX
	mov	al,0ch		;start address high register
	out	dx,al
	inc	dx
	mov	al,ch
	out	dx,al
	dec	dx
	mov	al,0dh		;start address low register
	out	dx,al
	mov	al,cl
	inc	dx
	out	dx,al


	sti

	ret

ENDP

;============================================================================
;
; VL_ScreenToScreen
;
; Basic block copy routine.  Copies one block of screen memory to another,
; using write mode 1 (sets it and returns with write mode 0).  bufferofs is
; NOT accounted for.
;
;============================================================================

PROC	VL_ScreenToScreen	source:WORD, dest:WORD, wide:WORD, height:WORD
PUBLIC	VL_ScreenToScreen
USES	SI,DI

	pushf
	cli

	mov	dx,SC_INDEX
	mov	ax,SC_MAPMASK+15*256
	out	dx,ax
	mov	dx,GC_INDEX
	mov	al,GC_MODE
	out	dx,al
	inc	dx
	in	al,dx
	and	al,NOT 3
	or	al,1
	out	dx,al

	popf

	mov	bx,[linewidth]
	sub	bx,[wide]

	mov	ax,SCREENSEG
	mov	es,ax
	mov	ds,ax

	mov	si,[source]
	mov	di,[dest]				;start at same place in all planes
	mov	dx,[height]				;scan lines to draw
	mov	ax,[wide]

@@lineloop:
	mov	cx,ax
	rep	movsb
	add	si,bx
	add	di,bx

	dec	dx
	jnz	@@lineloop

	mov	dx,GC_INDEX+1
	in	al,dx
	and	al,NOT 3
	out	dx,al

	mov	ax,ss
	mov	ds,ax					;restore turbo's data segment

	ret

ENDP
