*-------------------------------------------------------*
*	Draw in single plane modes			*	
*-------------------------------------------------------*
both		set	1	; Write in both FastRAM and on screen
longs		equ	1
get		equ	1
mul		equ	1	; Multiply rather than use table
shift		equ	1

only_16		equ	1


	include		"..\pixelmac.dev"
	include		"..\..\vdi.inc"

	xdef		text_area,_text_area

	xref		mode_table

	xref		mreplace,replace	; temporary

	xref		get_colour

  ifeq shift
	xref		dot,lline,rline
  endc
  ifeq mul
	xref		row
  endc


locals      equ     50
length      equ     locals-2
colours     equ     length-4
output      equ     colours-4
h_w         equ     output-4
font_addr   equ     h_w-4
text_addr   equ     font_addr-4
code_low    equ     text_addr-2
screen_addr equ     code_low-4
shadow_addr equ     screen_addr-4
dest_x      equ     shadow_addr-2
wraps       equ     dest_x-4
offset_tab  equ     wraps-4
remaining   equ     offset_tab-2
font_line   equ     remaining-2
normal_font equ     font_line-4
;pixels      equ     offset_tab-2
;mask        equ     pixels-4


	dc.b	0,0,"text_area",0
* In:	a1	virtual workstation
*	a2	offset table
*       d0      string length
*	d3-d4	destination coordinates
**	d6	background and foreground colour
**	d7	logic operation
*	a4	pointer to first character
**	a5	font structure address  
; Needs to do its own clipping
_text_area:
text_area:
	cmp.l		#0,a2
	bne		.must_return
	tst.w		vwk_text_effects(a1)
	bne		.must_return
	cmp.w		#1,vwk_mode(a1)
	beq		.ok
.must_return
	moveq		#0,d0			; Can't deal with this yet
	rts
.ok:
	sub.l		#locals,a7
	move.w		d0,length(a7)           ; String length to stack
	beq		no_draw
	moveq		#0,d0
	move.l		vwk_text_colour(a1),d0

; New check on 2000-12-12
	cmp.l		#$00000001,d0
	beq		.colour_ok
	add.l		#locals,a7
	moveq		#0,d0	; Only black on white supported
	rts
.colour_ok:
;

	bsr		get_colour		; Convert from VDI to TOS
	move.l		d0,colours(a7)		; Colours to stack
	move.w		vwk_mode(a1),d7
	lsl.w		#3,d7
	ext.l		d7
	add.l		#mode_table,d7
	move.l		d7,output(a7)		; Pointer to address of correct output routine on stack

	move.l		vwk_text_current_font(a1),a5

; New check on 2000-12-10
;	tst.w		font_extra_unpacked_format(a5)
;	bne		.has_unpacked
;	add.l		#locals,a7
;	moveq		#0,d0	; Non-unpacked format is buggy
;	rts
;.has_unpacked:
;

	move.w		vwk_text_alignment_vertical(a1),d0
	add.w		d0,d0
	add.w		font_extra_distance(a5,d0.w),d4
	move.w		font_width(a5),d5       ; Source wrap (later high word)
;	move.l		font_data(a5),a0
	sub.l		a0,a0
	move.l		font_table_character(a5),a6
	move.l		a6,offset_tab(a7)

	move.w		font_height(a5),d0      ; d0 - lines to blit (later high word)
	moveq		#0,d1                   ; d1 - source x-coordinate

; Should really always clip!
;	tst.w		vwk_clip_on(a1)
;	beq		no_clip

	move.w		vwk_clip_rectangle_y2(a1),d6
	sub.w		d0,d6
	sub.w		d4,d6
	addq.w		#1,d6                   ; d6 = max_y - (dest_y + font_height - 1)
	bge		.way_down               ; No bottom clipping needed if >=0
	add.w		d6,d0                   ;  else fewer lines to blit
.way_down:
	move.w		vwk_clip_rectangle_y1(a1),d6
	move.w		d6,d7
	sub.w		d4,d7                   ; d7 = min_y - dest_y
	ble		.from_top               ; No top clipping needed if <=0
	sub.w		d7,d0                   ;  else fewer lines to blit
;	mulu.w		d5,d7                   ;  and start on a lower line
;	add.l		d7,a0
	move.w		d7,a0
	move.w          d6,d4
.from_top:
	tst.w           d0
	ble             no_draw

	swap            d0
	swap            d5

	move.w          vwk_clip_rectangle_x2(a1),d0
	cmp.w           d0,d3
	bgt             no_draw

	move.w		length(a7),d2
	moveq		#0,d1
	sub.w		d3,d0
	move.w		vwk_clip_rectangle_x1(a1),d7
	addq.w		#1,d0               ; d0.w width of clip window
	cmp.w		d7,d3
;	bge		clip_done           ; If not within clip window
	bge		first_char          ; If not within clip window
	move.w		d7,d1               ;  calculate distance to first visible pixel
	sub.w		d3,d1
	sub.w		d1,d0
	move.w		d7,d3               ;  and set new destination start x-coordinate

	move.w		font_code_low(a5),d5
;	move.w		length(a7),d2

	move.w		font_flags(a5),d6
	and.w		#8,d6
	bne		monospace_first
.next_char:
	subq.w          #1,d2
	bmi             no_draw
	move.w          (a4)+,d6
	sub.w		d5,d6
	add.w		d6,d6
	move.w		2(a6,d6.w),d7           ; Start of next character
	sub.w		0(a6,d6.w),d7           ; Start of this character
	sub.w		d7,d1
	bgt             .next_char
	beq		first_char
	add.w		d7,d1
	subq.l          #2,a4
	addq.w		#1,d2
first_char:

; New, 2000-12-10
	move.w		font_flags(a5),d6
	and.w		#8,d6
	beq		.count_ok		; Don't bother since that should not happen here!
	moveq		#0,d7
	move.w		vwk_clip_rectangle_x2(a1),d7
	sub.w		d3,d7
	add.w		d1,d7			; Add on outside part of first character
	move.w		font_widest_cell(a5),d6
;	add.w		d6,d7
;	subq.w		#1,d7
;	divu		d6,d7
;	cmp.w		d7,d2
;	ble		.count_ok
;	move.w		d7,d2
;.count_ok:
	move.w		#0,remaining(a7)
;
	addq.w		#1,d7
;
	divu		d6,d7
	cmp.w		d7,d2
	ble		.count_ok
	moveq		#1,d2
	tst.w		d7
	beq		.count_ok
	move.w		d7,d2
	swap		d7
	move.w		d7,remaining(a7)
	move.w		a0,font_line(a7)
	move.l		a5,normal_font(a7)
.count_ok:

	move.w          d2,length(a7)

clip_done:
	sub.l		#$10000,d0		; Height only used via dbra
	move.l          d0,h_w(a7)              ; Height and width to display
;	move.l          a0,font_addr(a7)        ; Start of first font line to draw from
	move.w          a0,d2                   ; Number of lines down to start draw from in font
	move.w          (a4)+,d6
	move.l          a4,text_addr(a7)        ; Address of next character
	move.w		font_code_low(a5),code_low(a7)
	move.w          d3,dest_x(a7)

	move.l		vwk_real_address(a1),a1

  ifne mul
	move.l		wk_screen_mfdb_address(a1),a0
  endc

  ifne mul
	move.w		wk_screen_wrap(a1),d5	; d5 - wraps (source dest)
	mulu.w		d5,d4
	add.l		d4,a0
  endc
  ifeq mul
	lea		row(pc),a0
	move.l		(a0,d4.w*4),a0
  endc

	exg		a0,a1			; Workstation used below

  ifne both
	move.l		wk_screen_shadow_address(a0),d7
        bne             .shadow
        addq.l          #4,output(a7)
	sub.l		a4,a4
	bra		.no_shadow
.shadow:
	move.l		d7,a4
	add.l		d4,a4
.no_shadow:
  endc
	move.l		output(a7),a0
	move.l		(a0),output(a7)

        move.l		d5,wraps(a7)
        move.l		a1,screen_addr(a7)
        move.l		a4,shadow_addr(a7)

	tst.w           font_extra_unpacked_format(a5)	; Quick display applicable?
	bne		display4

no_display4:
	tst.w		remaining(a7)	; Attempt to fix final character
	beq		.no_extra
	addq.w		#1,length(a7)
.no_extra:

	swap		d5			; Not nice that I have to do this
	mulu		d5,d2			; Perhaps there is a better way?
	swap		d5
	add.l		font_data(a5),d2
	move.l		d2,font_addr(a7)

	move.w		d1,d2
	sub.w		code_low(a7),d6
	add.w		d6,d6
	move.w		0(a6,d6.w),d4		; Start of this character
	add.w		d4,d1
	sub.w		2(a6,d6.w),d4
	neg.w		d4
	sub.w		d2,d4
	cmp.w		d4,d0
	bls		.last_char1		; If not last character (clipping)
	move.w		d4,d0			;  blit full character width
.last_char1:
	sub.w		d0,h_w+2(a7)		; Lower free width
	add.w		d3,d4
	move.w		d4,dest_x(a7)

	move.l		font_addr(a7),a0
	move.l		output(a7),a6

;	bra		.first

	bsr		draw_char

.loop:
	subq.w		#1,length(a7)
	ble		no_draw
	move.l		h_w(a7),d0
	tst.w		d0
	beq		no_draw

	move.l		text_addr(a7),a0
	move.w		(a0)+,d6
	move.l		a0,text_addr(a7)
	move.l		shadow_addr(a7),a4
	move.l		screen_addr(a7),a1
	move.w		dest_x(a7),d3
	move.l		wraps(a7),d5

	move.l		offset_tab(a7),a0
	sub.w		code_low(a7),d6
	add.w		d6,d6
	move.w		0(a0,d6.w),d4		; Start of this character
	move.w		d4,d1
	sub.w		2(a0,d6.w),d4
	neg.w		d4
	cmp.w		d4,d0
	bls		.last_char		; If not last character (clipping)
	move.w		d4,d0			;  blit full character width
.last_char:
	sub.w		d0,h_w+2(a7)		; Lower free width
	add.w		d3,d4
	move.w		d4,dest_x(a7)

	move.l		font_addr(a7),a0
	move.l		output(a7),a6

	bsr		draw_char
	bra		.loop

no_clip:
	moveq		#-1,d0
	move.w		font_height(a5),d0
	swap		d0
	swap		d5
  ifne mul
	move.w		wk_screen_wrap(a1),d5	; d5 - wraps (source dest)
  endc
	bra		clip_done

no_draw:
	add.w		#locals,a7

	moveq		#1,d0			; Return as completed
	rts

monospace_first:
	move.w		font_widest_cell(a5),d6
	divu		d6,d1
	sub.w		d1,d2
	ble		no_draw
	add.w		d1,d1
	add.w		d1,a4
	swap		d1
	bra		first_char


;.loop:
;	subq.w		#1,length(a7)
;	ble		.no_draw
;	move.l		h_w(a7),d0
;	tst.w		d0
;	beq		.no_draw
;
;	move.l		text_addr(a7),a0
;	move.w		(a0)+,d6
;	move.l		a0,text_addr(a7)
;	move.l		shadow_addr(a7),a4
;	move.l		screen_addr(a7),a1
;	move.w		dest_x(a7),d3
;	move.l		wraps(a7),d5
;
;	move.l		font_addr(a7),a0
;	sub.w		code_low(a7),d6
;	add.w		d6,d6
;	move.w		0(a6,d6.w),d4		; Start of this character
;	move.w		d4,d1
;	sub.w		2(a6,d6.w),d4
;	neg.w		d4
;	cmp.w		d4,d0
;	bls		.last_char		; If not last character (clipping)
;	move.w		d4,d0			;  blit full character width
;.last_char:
;	sub.w		d0,h_w+2(a7)		; Lower free width
;	add.w		d3,d4
;	move.w		d4,dest_x(a7)
;
;.first:

	dc.b	0,0,"draw_char",0
* In:	a0	font line address
*	a1	screen line address
*	a4	shadow line address
*	a6	drawing routine
*	d0	lines to draw, width
*	d1	source x-coordinate
*	d3	destination x-coordinate
*	d5	source wrap, destination wrap
* XXX:	all
draw_char:
	move.w		d1,d2
	and.w		#$0f,d1			; d1 - bit number in source

	lsr.w		#4,d2
	lsl.w		#1,d2
	add.w		d2,a0			; a0 - start address in source MFDB

	move.w		d3,d4
	and.w		#$0f,d3			; d3 - first bit number in dest MFDB

	lsr.w		#4,d4
	lsl.w		#1,d4
	add.w		d4,a1			; a1 - start address in dest MFDB
  ifne both
	add.w		d4,a4			; a4 - start address in shadow
  endc						; d4 scratch

	add.w		d3,d0
	subq.w		#1,d0
	move.w		d0,d2
	move.w		d0,d4

	lsr.w		#4,d4
	lsl.w		#1,d4
	sub.w		d4,d5
	move.w		d5,a3
	swap		d5
	sub.w		d4,d5
	move.w		d5,a2
	swap		d5			; d5 - wrap-blit

	and.w		#$0f,d2
	addq.w		#1,d2			; d2 - final bit number in dest MFDB

;	pea		.loop
;	move.l		a0,d7
;	move.l		4+output(a7),a0
;	move.l		(a0),-(a7)
;	move.l		d7,a0
;	rts					; Call correct routine

	move.l		4+colours(a7),d7

	jmp		(a6)


**********
*
* Actual drawing routines
*
**********



;macro	calc_addr dreg,treg
calc_addr macro	dreg,treg
;	move.w		(treg)+,d0
	move.w		(\2)+,d0
	sub.w		code_low(a7),d0
  ifne only_16
	lsl.w		#4,d0
  endc
  ifeq only_16
	mulu		d4,d0
  endc
;	add.w		d0,dreg
	add.w		d0,\1
	endm


	dc.b		0,"display4",0

* In:	a0	font line address
*	a1	screen line address
*	a4	shadow line address
*	a6	drawing routine
*	d0	lines to draw, width
*	d1	source x-coordinate
*	d3	destination x-coordinate
*	d5	source wrap, destination wrap
* XXX:	all
*
* In:	d1	Source x-coordinate
*	d2	Source y-coordinate (starting line in font)
*	d3	Destination x-coordinate
*
display4:
	cmp.l		#mreplace,output(a7)
	beq		.do_display4
	cmp.l		#replace,output(a7)
	beq		.do_display4
	bra		no_display4

.do_display4:
;	move.l		a0,-(a7)

;	move.w		d1,d0
;	and.w		#$0007,d0
;	beq		display_byte
	
;	move.l		a1,-(a7)

	move.l		font_extra_unpacked_data(a5),a0
	add.w		d2,a0
	move.l		a0,font_addr(a7)

;	move.w		font_widest_cell(a5),a1
;	move.w		a1,pixels(a7)
;	move.l		#$03ffffff,d7
;	cmp.w		#6,a1
;	beq		.6_pixels
;	lsr.l		#2,d7
;.6_pixels:
;	move.l		d7,mask(a7)
;
;	move.l		#$00ffffff,mask(a7)
;	move.w		#8,pixels(a7)

;	tst.w		d1
;	beq		fast_draw
	tst.w		d1
	bne		.first_slow
	cmp.w		font_widest_cell(a5),d0
	bge		fast_draw
.first_slow:

	swap		d5			; Not nice that I have to do this
	mulu		d5,d2			; Perhaps there is a better way?
	swap		d5
	move.l		font_data(a5),a0
	add.l		d2,a0

	move.w		d1,d2
	sub.w		code_low(a7),d6
	add.w		d6,d6
	move.w		0(a6,d6.w),d4		; Start of this character
	add.w		d4,d1
	sub.w		2(a6,d6.w),d4
	neg.w		d4
	sub.w		d2,d4
	cmp.w		d4,d0
	bls		.last_char1		; If not last character (clipping)
	move.w		d4,d0			;  blit full character width
.last_char1:
	sub.w		d0,h_w+2(a7)		; Lower free width
	add.w		d3,d4
	move.w		d4,dest_x(a7)

;	move.l		font_addr(a7),a0
	move.l		output(a7),a6

;	bra		.first

	bsr		draw_char

	subq.w		#1,length(a7)
	addq.l		#2,text_addr(a7)	; Silly way to do it!


fast_draw:
	subq.l		#2,text_addr(a7)
	move.w		dest_x(a7),d1

  ifne both
	tst.l		shadow_addr(a7)
	bne		sh_fast_draw
  endc

oldboth	set	both
both	set	0

	move.w		font_widest_cell(a5),d7
	cmp.w		#6,d7
	beq		fast_draw_6

	move.w		length(a7),d7
	and.w		#$fffc,d7
	beq		.d1_st_loop
	lsr.w		#2,d7
	subq.w		#1,d7
	move.w		d1,d6
.d4_loop:
;	move.w		#fnt_hght,d4	; Calculate offset of character
	move.w		h_w(a7),d4	; Height to draw (not necessarily character height)

	move.w		wraps+2(a7),a6	; Try to move this out of the loop!

	move.l		font_addr(a7),a0
	move.l		text_addr(a7),a1
	move.l		a0,a2
	move.l		a0,a3
	move.l		a0,a5
	calc_addr	a0,a1
	calc_addr	a2,a1
	calc_addr	a3,a1
	calc_addr	a5,a1
	move.l		a1,text_addr(a7)
	
;	subq.w		#1,d4

	move.l		screen_addr(a7),a1
  ifne both
	move.l		shadow_addr(a7),a4
  endc

	move.w		d6,d1
	bsr		multi_outchar_8

	add.w		#4*8,d6
	dbra		d7,.d4_loop

	move.w		d6,d1
.d1_st_loop:
	move.w		length(a7),d7
	and.w		#$0003,d7
	beq		.disp4_end
	subq.w		#1,d7

	move.w		wraps+2(a7),a6	; Screen wrap (hopefully)

	move.l		text_addr(a7),a5
	move.l		screen_addr(a7),a2
  ifne both
	move.l		shadow_addr(a7),a3
  endc
	move.w		d1,d6
.d1_loop:
;	move.w		#fnt_hght,d4	; Calculate offset of character
	move.w		h_w(a7),d4	; Height to draw (not necessarily character height)
	move.l		font_addr(a7),a0
	calc_addr	a0,a5

;	subq.w		#1,d4

	move.l		a2,a1
  ifne both
	move.l		a3,a4
  endc

	move.w		d6,d1
	bsr		outchar_8

	addq.w		#8,d6
	dbra		d7,.d1_loop
	
	move.l		a5,text_addr(a7)
	move.w		d6,d1
.disp4_end:

	move.w		remaining(a7),d7; Attempt to fix final character
	beq		.no_more8

	move.w		d1,d3
	move.w		h_w(a7),d0	; Height to draw (not necessarily character height)
	swap		d0
	move.w		d7,d0
	move.w		font_line(a7),d2
	move.l		wraps(a7),d5
	swap		d5			; Not nice that I have to do this
	mulu		d5,d2			; Perhaps there is a better way?
	swap		d5
	move.l		normal_font(a7),a5
	move.l		font_data(a5),a0
	add.l		d2,a0

	move.l		text_addr(a7),a5
	move.w		(a5),d6
	sub.w		code_low(a7),d6
	add.w		d6,d6
	move.l		offset_tab(a7),a6
	move.w		0(a6,d6.w),d1		; Start of this character

	move.l		output(a7),a6
	move.l		screen_addr(a7),a1
  ifne both
	move.l		shadow_addr(a7),a4
  endc

	bsr		draw_char
.no_more8:

;	addq.l		#8,a7		; Remove a1 (row address)
	add.w		#locals,a7

	moveq		#1,d0			; Return as completed
	rts


fast_draw_6:
	move.w		length(a7),d7
	and.w		#$fffc,d7
	beq		.d1_st_loop
	lsr.w		#2,d7
	subq.w		#1,d7
	move.w		d1,d6
.d4_loop:
;	move.w		#fnt_hght,d4	; Calculate offset of character
	move.w		h_w(a7),d4	; Height to draw (not necessarily character height)

	move.w		wraps+2(a7),a6	; Try to move this out of the loop!

	move.l		font_addr(a7),a0
	move.l		text_addr(a7),a1
	move.l		a0,a2
	move.l		a0,a3
	move.l		a0,a5
	calc_addr	a0,a1
	calc_addr	a2,a1
	calc_addr	a3,a1
	calc_addr	a5,a1
	move.l		a1,text_addr(a7)
	
;	subq.w		#1,d4

	move.l		screen_addr(a7),a1
  ifne both
	move.l		shadow_addr(a7),a4
  endc

	move.w		d6,d1
	bsr		multi_outchar_6

	add.w		#4*6,d6
	dbra		d7,.d4_loop

	move.w		d6,d1
.d1_st_loop:
	move.w		length(a7),d7
	and.w		#$0003,d7
	beq		.disp4_end
	subq.w		#1,d7

	move.w		wraps+2(a7),a6	; Screen wrap (hopefully)

	move.l		text_addr(a7),a5
	move.l		screen_addr(a7),a2
  ifne both
	move.l		shadow_addr(a7),a3
  endc
	move.w		d1,d6
.d1_loop:
;	move.w		#fnt_hght,d4	; Calculate offset of character
	move.w		h_w(a7),d4	; Height to draw (not necessarily character height)
	move.l		font_addr(a7),a0
	calc_addr	a0,a5

;	subq.w		#1,d4

	move.l		a2,a1
  ifne both
	move.l		a3,a4
  endc

	move.w		d6,d1
	bsr		outchar_6

	addq.w		#6,d6
	dbra		d7,.d1_loop
	
	move.l		a5,text_addr(a7)
	move.w		d6,d1
.disp4_end:

	move.w		remaining(a7),d7; Attempt to fix final character
	beq		.no_more6

	move.w		d1,d3
	move.w		h_w(a7),d0	; Height to draw (not necessarily character height)
	swap		d0
	move.w		d7,d0
	move.w		font_line(a7),d2
	move.l		wraps(a7),d5
	swap		d5			; Not nice that I have to do this
	mulu		d5,d2			; Perhaps there is a better way?
	swap		d5
	move.l		normal_font(a7),a5
	move.l		font_data(a5),a0
	add.l		d2,a0

	move.l		text_addr(a7),a5
	move.w		(a5),d6
	sub.w		code_low(a7),d6
	add.w		d6,d6
	move.l		offset_tab(a7),a6
	move.w		0(a6,d6.w),d1		; Start of this character

	move.l		output(a7),a6
	move.l		screen_addr(a7),a1
  ifne both
	move.l		shadow_addr(a7),a4
  endc

	bsr		draw_char
.no_more6:

;	addq.l		#8,a7		; Remove a1 (row address)
	add.w		#locals,a7

	moveq		#1,d0			; Return as completed
	rts
	
	
outchar_8:
	move.w		d1,d2		; Calculate column offset
	lsr.w		#4,d2
	add.w		d2,d2
	add.w		d2,a1
  ifne both
	add.w		d2,a4
  endc
	and.w		#$000f,d1	
	beq		even_align_8	; Character at even word....
	cmp.w		#8,d1
	beq		odd_align_8	; ...even byte
	ble		word_align_8	; ...msb of word
no_align_8:
	move.l		#$00ffffff,d0	; ...lsb of word
	ror.l		d1,d0
	subq.w		#8,d1
.loop:
	moveq		#0,d2
	move.b		(a0)+,d2
	ror.l		d1,d2
	swap		d2
  ifne both
	move.l		(a4),d3
  endc
  ifeq both
	move.l		(a1),d3
  endc
	and.l		d0,d3
	or.l		d2,d3
  ifne both
	move.l		d3,(a4)
  endc
	move.l		d3,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

word_align_8:
	move.w		#$00ff,d0
	ror.w		d1,d0
	moveq		#8,d5
	sub.w		d1,d5
.loop:
	moveq		#0,d2
	move.b		(a0)+,d2
	lsl.l		d5,d2
  ifne both
	move.w		(a4),d3
  endc
  ifeq both
	move.w		(a1),d3
  endc
	and.w		d0,d3
	or.w		d2,d3
  ifne both
	move.w		d3,(a4)
  endc
	move.w		d3,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

odd_align_8:
	add.w		#1,a1
  ifne both
	add.w		#1,a4
  endc
even_align_8:
.loop:
;	move.b		(a0)+,(a1)
	move.b		(a0)+,d2
  ifne both
	move.b		d2,(a4)
  endc
	move.b		d2,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts


outchar_6:
	move.w		d1,d2		; Calculate column offset
	lsr.w		#4,d2
	add.w		d2,d2
	add.w		d2,a1
  ifne both
	add.w		d2,a4
  endc
	and.w		#$000f,d1	
	beq		even_align_6	; Character at even word....
	cmp.w		#8,d1
	beq		odd_align_6	; ...even byte
	ble		word_align_6	; ...msb of word
no_align_6:
	move.l		#$03ffffff,d0	; ...lsb of word
	ror.l		d1,d0
	subq.w		#8,d1
.loop:
	moveq		#0,d2
	move.b		(a0)+,d2
	ror.l		d1,d2
	swap		d2
  ifne both
	move.l		(a4),d3
  endc
  ifeq both
	move.l		(a1),d3
  endc
	and.l		d0,d3
	or.l		d2,d3
  ifne both
	move.l		d3,(a4)
  endc
	move.l		d3,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

word_align_6:
	move.w		#$03ff,d0
	ror.w		d1,d0
	moveq		#8,d5
	sub.w		d1,d5
.loop:
	moveq		#0,d2
	move.b		(a0)+,d2
	lsl.l		d5,d2
  ifne both
	move.w		(a4),d3
  endc
  ifeq both
	move.w		(a1),d3
  endc
	and.w		d0,d3
	or.w		d2,d3
  ifne both
	move.w		d3,(a4)
  endc
	move.w		d3,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

odd_align_6:
	add.w		#1,a1
  ifne both
	add.w		#1,a4
  endc
even_align_6:
	moveq		#$03,d0
.loop:
	move.b		(a0)+,d2
  ifne both
	move.b		(a4),d3
  endc
  ifeq both
	move.b		(a1),d3
  endc
	and.b		d0,d3
	or.b		d2,d3
  ifne both
	move.b		d3,(a4)
  endc
	move.b		d3,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts


multi_outchar_8:
	move.w		d1,d2		; Calculate column offset
	lsr.w		#4,d2
	add.w		d2,d2
	add.w		d2,a1
  ifne both
	add.w		d2,a4
  endc
	and.w		#$000f,d1	
	beq		m_even_align_8	; Character at even word....
	subq.w		#8,d1
	beq		m_odd_align_8	; ...even byte
	ble		m_msb_8		; ...msb of word
;	addq.w		#8,d1
	subq.w		#6,a6
m_lsb8:
	moveq		#8,d5		; Fix these later
	sub.w		d1,d5
.loop:
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	lsl.l		d1,d2
	move.b		(a0)+,d2
	ror.l		d1,d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
  ifne both
	move.w		2(a4),d2
  endc
  ifeq both
	move.w		2(a1),d2
  endc
	rol.l		d1,d2
	swap		d2
	move.b		(a3)+,d2
	lsl.l		#8,d2
	move.b		(a5)+,d2
	swap		d2
	move.b		(a2)+,d2
	swap		d2
	rol.l		d5,d2
  ifne both
	move.l		d2,(a4)+
  endc
	move.l		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts
	
m_even_align_8:			; Characters word aligned
	subq.w		#4,a6
.loop:				; 8 pixels wide characters
;	move.b		(a0)+,(a1)+
;	move.b		(a2)+,(a1)+
;	move.b		(a3)+,(a1)+
;	move.b		(a5)+,(a1)+
	move.b		(a0)+,d2
	lsl.w		#8,d2
	move.b		(a2)+,d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
	move.b		(a3)+,d2
	lsl.w		#8,d2
	move.b		(a5)+,d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

m_odd_align_8:			; Characters byte aligned (not word)
	subq.w		#4,a6
	addq.l		#1,a1
  ifne both
	addq.l		#1,a4
  endc
.loop:				; 8 pixels wide characters
;	move.b		(a0)+,(a1)+
;	move.b		(a2)+,(a1)+
;	move.b		(a3)+,(a1)+
;	move.b		(a5)+,(a1)+
	move.b		(a0)+,d2
  ifne both
	move.b		d2,(a4)+
  endc
	move.b		d2,(a1)+
	move.b		(a2)+,d2
	lsl.w		#8,d2
	move.b		(a3)+,d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
	move.b		(a5)+,d2
  ifne both
	move.b		d2,(a4)+
  endc
	move.b		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

m_msb_8:
	neg.w		d1
	subq.w		#6,a6
	moveq		#8,d5
	sub.w		d1,d5	
.loop:				; 8 pixels wide characters
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	lsr.w		d1,d2
	move.b		(a0)+,d2
	swap		d2
	move.b		(a2)+,d2
	lsl.w		#8,d2
	move.b		(a3)+,d2
	lsl.l		d1,d2
	swap		d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
  ifne both
	move.w		2(a4),d2
  endc
  ifeq both
	move.w		2(a1),d2
  endc
	swap		d2
	lsl.l		d5,d2
	move.b		(a5)+,d2
	ror.l		d5,d2
	swap		d2
  ifne both
	move.l		d2,(a4)+
  endc
	move.l		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts


multi_outchar_6:
	move.w		d1,d2		; Calculate column offset
	lsr.w		#4,d2
	add.w		d2,d2
	add.w		d2,a1
  ifne both
	add.w		d2,a4
  endc
	and.w		#$000f,d1	
	beq		m_even_align_6	; Character at even word....
	subq.w		#8,d1
	beq		m_odd_align_6	; ...even byte
	ble		m_msb_6		; ...msb of word
;	addq.w		#8,d1
	subq.w		#6,a6
	move.w		d1,d3
	addq.w		#2,d3
	moveq		#4,d5
	moveq		#$03,d0
	sub.w		d1,d5
	beq		m_lsb64
	ble		m_lsb6h
m_lsb6l:
.loop:
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	lsl.l		d1,d2
	move.b		(a0)+,d2
	lsl.l		#6,d2
	move.b		(a2)+,d2
	lsl.l		#6,d2
	move.b		(a3)+,d2
	lsl.l		d5,d2
	swap		d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
  ifne both
	move.w		2(a4),d2
  endc
  ifeq both
	move.w		2(a1),d2
  endc
	rol.l		d3,d2
	swap		d2
	and.b		d0,d2
	or.b		(a5)+,d2
	swap		d2
	ror.l		d3,d2
  ifne both
	move.l		d2,(a4)+
  endc
	move.l		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

m_lsb64:
.loop:
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	lsl.l		#4,d2
	move.b		(a0)+,d2
	lsl.l		#6,d2
	move.b		(a2)+,d2
	lsl.l		#6,d2
	move.b		(a3)+,d2
	swap		d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
  ifne	both
	move.w		2(a4),d2
  endc
  ifeq both
	move.w		2(a1),d2
  endc
	rol.l		#6,d2
	swap		d2
	and.b		d0,d2
	or.b		(a5)+,d2
	swap		d2
	ror.l		#6,d2
  ifne both
	move.l		d2,(a4)+
  endc
	move.l		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

m_lsb6h:
	addq.w		#6,d5
	subq.w		#6,d3
.loop:
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	lsl.l		d1,d2
	move.b		(a0)+,d2
	lsl.l		#6,d2
	move.b		(a2)+,d2
	lsl.l		d5,d2
	swap		d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
  ifne both
	move.w		2(a4),d2
  endc
  ifeq both
	move.w		2(a1),d2
  endc
	rol.l		d3,d2
	swap		d2
	move.b		(a3)+,d2
	rol.l		#6,d2
	and.b		d0,d2
	or.b		(a5)+,d2
	rol.l		#4,d2
	rol.l		d5,d2
  ifne both
	move.l		d2,(a4)+
  endc
	move.l		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

m_even_align_6:			; 6 pixels wide characters
.loop:
;	moveq		#0,d2
	move.b		(a0)+,d2
	lsl.w		#6,d2
	or.b		(a2)+,d2
	lsl.l		#6,d2
	or.b		(a3)+,d2
	lsl.l		#6,d2
	or.b		(a5)+,d2
	lsl.l		#6,d2
  ifne both
	move.b		3(a4),d2
  endc
  ifeq both
	move.b		3(a1),d2
  endc
  ifne both
	move.l		d2,(a4)
  endc
	move.l		d2,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts
	
m_odd_align_6:			; 6 pixels wide characters
.loop:
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	move.b		(a0)+,d2
	lsl.l		#6,d2
	or.b		(a2)+,d2
	lsl.l		#6,d2
	or.b		(a3)+,d2
;	rol.l		#6,d2
	lsl.l		#4,d2		; These two lines should be better
	rol.l		#2,d2
	or.b		(a5)+,d2
	ror.l		#2,d2
  ifne both
	move.l		d2,(a4)
  endc
  ifeq both
	move.l		d2,(a1)
  endc
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts
	
m_msb_6:				; Characters start in msb byte
	neg.w		d1		; Correct ?
.loop:				; 6 pixels wide characters
  ifne both
	move.l		(a4),d2
  endc
  ifeq both
	move.l		(a1),d2
  endc
	ror.l		d1,d2
	clr.w		d2
	swap		d2
	move.b		(a0)+,d2
	lsl.l		#6,d2
	or.b		(a2)+,d2
	lsl.l		#6,d2
	or.b		(a3)+,d2
	rol.l		#6,d2
	or.b		(a5)+,d2
	ror.l		#2,d2
	rol.l		d1,d2
  ifne both
	move.l		d2,(a4)
  endc
  ifeq both
	move.l		d2,(a1)
  endc
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

both	set	oldboth

  ifne both
sh_fast_draw:
	move.w		font_widest_cell(a5),d7
	cmp.w		#6,d7
	beq		sh_fast_draw_6

	move.w		length(a7),d7
	and.w		#$fffc,d7
	beq		.d1_st_loop
	lsr.w		#2,d7
	subq.w		#1,d7
	move.w		d1,d6
.d4_loop:
;	move.w		#fnt_hght,d4	; Calculate offset of character
	move.w		h_w(a7),d4	; Height to draw (not necessarily character height)

	move.w		wraps+2(a7),a6	; Try to move this out of the loop!

	move.l		font_addr(a7),a0
	move.l		text_addr(a7),a1
	move.l		a0,a2
	move.l		a0,a3
	move.l		a0,a5
	calc_addr	a0,a1
	calc_addr	a2,a1
	calc_addr	a3,a1
	calc_addr	a5,a1
	move.l		a1,text_addr(a7)
	
;	subq.w		#1,d4

	move.l		screen_addr(a7),a1
  ifne both
	move.l		shadow_addr(a7),a4
  endc

	move.w		d6,d1
	bsr		sh_multi_outchar_8

	add.w		#4*8,d6
	dbra		d7,.d4_loop

	move.w		d6,d1
.d1_st_loop:
	move.w		length(a7),d7
	and.w		#$0003,d7
	beq		.disp4_end
	subq.w		#1,d7

	move.w		wraps+2(a7),a6	; Screen wrap (hopefully)

	move.l		text_addr(a7),a5
	move.l		screen_addr(a7),a2
  ifne both
	move.l		shadow_addr(a7),a3
  endc
	move.w		d1,d6
.d1_loop:
;	move.w		#fnt_hght,d4	; Calculate offset of character
	move.w		h_w(a7),d4	; Height to draw (not necessarily character height)
	move.l		font_addr(a7),a0
	calc_addr	a0,a5

;	subq.w		#1,d4

	move.l		a2,a1
  ifne both
	move.l		a3,a4
  endc

	move.w		d6,d1
	bsr		sh_outchar_8

	addq.w		#8,d6
	dbra		d7,.d1_loop
	
	move.l		a5,text_addr(a7)
	move.w		d6,d1
.disp4_end:

	move.w		remaining(a7),d7; Attempt to fix final character
	beq		.no_more8

	move.w		d1,d3
	move.w		h_w(a7),d0	; Height to draw (not necessarily character height)
	swap		d0
	move.w		d7,d0
	move.w		font_line(a7),d2
	move.l		wraps(a7),d5
	swap		d5			; Not nice that I have to do this
	mulu		d5,d2			; Perhaps there is a better way?
	swap		d5
	move.l		normal_font(a7),a5
	move.l		font_data(a5),a0
	add.l		d2,a0

	move.l		text_addr(a7),a5
	move.w		(a5),d6
	sub.w		code_low(a7),d6
	add.w		d6,d6
	move.l		offset_tab(a7),a6
	move.w		0(a6,d6.w),d1		; Start of this character

	move.l		output(a7),a6
	move.l		screen_addr(a7),a1
  ifne both
	move.l		shadow_addr(a7),a4
  endc

	bsr		draw_char
.no_more8:

;	addq.l		#8,a7		; Remove a1 (row address)
	add.w		#locals,a7

	moveq		#1,d0			; Return as completed
	rts


sh_fast_draw_6:
	move.w		length(a7),d7
	and.w		#$fffc,d7
	beq		.d1_st_loop
	lsr.w		#2,d7
	subq.w		#1,d7
	move.w		d1,d6
.d4_loop:
;	move.w		#fnt_hght,d4	; Calculate offset of character
	move.w		h_w(a7),d4	; Height to draw (not necessarily character height)

	move.w		wraps+2(a7),a6	; Try to move this out of the loop!

	move.l		font_addr(a7),a0
	move.l		text_addr(a7),a1
	move.l		a0,a2
	move.l		a0,a3
	move.l		a0,a5
	calc_addr	a0,a1
	calc_addr	a2,a1
	calc_addr	a3,a1
	calc_addr	a5,a1
	move.l		a1,text_addr(a7)
	
;	subq.w		#1,d4

	move.l		screen_addr(a7),a1
  ifne both
	move.l		shadow_addr(a7),a4
  endc

	move.w		d6,d1
	bsr		sh_multi_outchar_6

	add.w		#4*6,d6
	dbra		d7,.d4_loop

	move.w		d6,d1
.d1_st_loop:
	move.w		length(a7),d7
	and.w		#$0003,d7
	beq		.disp4_end
	subq.w		#1,d7

	move.w		wraps+2(a7),a6	; Screen wrap (hopefully)

	move.l		text_addr(a7),a5
	move.l		screen_addr(a7),a2
  ifne both
	move.l		shadow_addr(a7),a3
  endc
	move.w		d1,d6
.d1_loop:
;	move.w		#fnt_hght,d4	; Calculate offset of character
	move.w		h_w(a7),d4	; Height to draw (not necessarily character height)
	move.l		font_addr(a7),a0
	calc_addr	a0,a5

;	subq.w		#1,d4

	move.l		a2,a1
  ifne both
	move.l		a3,a4
  endc

	move.w		d6,d1
	bsr		sh_outchar_6

	addq.w		#6,d6
	dbra		d7,.d1_loop
	
	move.l		a5,text_addr(a7)
	move.w		d6,d1
.disp4_end:

	move.w		remaining(a7),d7; Attempt to fix final character
	beq		.no_more6

	move.w		d1,d3
	move.w		h_w(a7),d0	; Height to draw (not necessarily character height)
	swap		d0
	move.w		d7,d0
	move.w		font_line(a7),d2
	move.l		wraps(a7),d5
	swap		d5			; Not nice that I have to do this
	mulu		d5,d2			; Perhaps there is a better way?
	swap		d5
	move.l		normal_font(a7),a5
	move.l		font_data(a5),a0
	add.l		d2,a0

	move.l		text_addr(a7),a5
	move.w		(a5),d6
	sub.w		code_low(a7),d6
	add.w		d6,d6
	move.l		offset_tab(a7),a6
	move.w		0(a6,d6.w),d1		; Start of this character

	move.l		output(a7),a6
	move.l		screen_addr(a7),a1
  ifne both
	move.l		shadow_addr(a7),a4
  endc

	bsr		draw_char
.no_more6:

;	addq.l		#8,a7		; Remove a1 (row address)
	add.w		#locals,a7

	moveq		#1,d0			; Return as completed
	rts
	
	
sh_outchar_8:
	move.w		d1,d2		; Calculate column offset
	lsr.w		#4,d2
	add.w		d2,d2
	add.w		d2,a1
  ifne both
	add.w		d2,a4
  endc
	and.w		#$000f,d1	
	beq		sh_even_align_8	; Character at even word....
	cmp.w		#8,d1
	beq		sh_odd_align_8	; ...even byte
	ble		sh_word_align_8	; ...msb of word
sh_no_align_8:
	move.l		#$00ffffff,d0	; ...lsb of word
	ror.l		d1,d0
	subq.w		#8,d1
.loop:
	moveq		#0,d2
	move.b		(a0)+,d2
	ror.l		d1,d2
	swap		d2
  ifne both
	move.l		(a4),d3
  endc
  ifeq both
	move.l		(a1),d3
  endc
	and.l		d0,d3
	or.l		d2,d3
  ifne both
	move.l		d3,(a4)
  endc
	move.l		d3,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

sh_word_align_8:
	move.w		#$00ff,d0
	ror.w		d1,d0
	moveq		#8,d5
	sub.w		d1,d5
.loop:
	moveq		#0,d2
	move.b		(a0)+,d2
	lsl.l		d5,d2
  ifne both
	move.w		(a4),d3
  endc
  ifeq both
	move.w		(a1),d3
  endc
	and.w		d0,d3
	or.w		d2,d3
  ifne both
	move.w		d3,(a4)
  endc
	move.w		d3,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

sh_odd_align_8:
	add.w		#1,a1
  ifne both
	add.w		#1,a4
  endc
sh_even_align_8:
.loop:
;	move.b		(a0)+,(a1)
	move.b		(a0)+,d2
  ifne both
	move.b		d2,(a4)
  endc
	move.b		d2,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts


sh_outchar_6:
	move.w		d1,d2		; Calculate column offset
	lsr.w		#4,d2
	add.w		d2,d2
	add.w		d2,a1
  ifne both
	add.w		d2,a4
  endc
	and.w		#$000f,d1	
	beq		sh_even_align_6	; Character at even word....
	cmp.w		#8,d1
	beq		sh_odd_align_6	; ...even byte
	ble		sh_word_align_6	; ...msb of word
sh_no_align_6:
	move.l		#$03ffffff,d0	; ...lsb of word
	ror.l		d1,d0
	subq.w		#8,d1
.loop:
	moveq		#0,d2
	move.b		(a0)+,d2
	ror.l		d1,d2
	swap		d2
  ifne both
	move.l		(a4),d3
  endc
  ifeq both
	move.l		(a1),d3
  endc
	and.l		d0,d3
	or.l		d2,d3
  ifne both
	move.l		d3,(a4)
  endc
	move.l		d3,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

sh_word_align_6:
	move.w		#$03ff,d0
	ror.w		d1,d0
	moveq		#8,d5
	sub.w		d1,d5
.loop:
	moveq		#0,d2
	move.b		(a0)+,d2
	lsl.l		d5,d2
  ifne both
	move.w		(a4),d3
  endc
  ifeq both
	move.w		(a1),d3
  endc
	and.w		d0,d3
	or.w		d2,d3
  ifne both
	move.w		d3,(a4)
  endc
	move.w		d3,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

sh_odd_align_6:
	add.w		#1,a1
  ifne both
	add.w		#1,a4
  endc
sh_even_align_6:
	moveq		#$03,d0
.loop:
	move.b		(a0)+,d2
  ifne both
	move.b		(a4),d3
  endc
  ifeq both
	move.b		(a1),d3
  endc
	and.b		d0,d3
	or.b		d2,d3
  ifne both
	move.b		d3,(a4)
  endc
	move.b		d3,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts


sh_multi_outchar_8:
	move.w		d1,d2		; Calculate column offset
	lsr.w		#4,d2
	add.w		d2,d2
	add.w		d2,a1
  ifne both
	add.w		d2,a4
  endc
	and.w		#$000f,d1	
	beq		sh_m_even_align_8	; Character at even word....
	subq.w		#8,d1
	beq		sh_m_odd_align_8	; ...even byte
	ble		sh_m_msb_8		; ...msb of word
;	addq.w		#8,d1
	subq.w		#6,a6
sh_m_lsb8:
	moveq		#8,d5		; Fix these later
	sub.w		d1,d5
.loop:
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	lsl.l		d1,d2
	move.b		(a0)+,d2
	ror.l		d1,d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
  ifne both
	move.w		2(a4),d2
  endc
  ifeq both
	move.w		2(a1),d2
  endc
	rol.l		d1,d2
	swap		d2
	move.b		(a3)+,d2
	lsl.l		#8,d2
	move.b		(a5)+,d2
	swap		d2
	move.b		(a2)+,d2
	swap		d2
	rol.l		d5,d2
  ifne both
	move.l		d2,(a4)+
  endc
	move.l		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts
	
sh_m_even_align_8:			; Characters word aligned
	subq.w		#4,a6
.loop:				; 8 pixels wide characters
;	move.b		(a0)+,(a1)+
;	move.b		(a2)+,(a1)+
;	move.b		(a3)+,(a1)+
;	move.b		(a5)+,(a1)+
	move.b		(a0)+,d2
	lsl.w		#8,d2
	move.b		(a2)+,d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
	move.b		(a3)+,d2
	lsl.w		#8,d2
	move.b		(a5)+,d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

sh_m_odd_align_8:			; Characters byte aligned (not word)
	subq.w		#4,a6
	addq.l		#1,a1
  ifne both
	addq.l		#1,a4
  endc
.loop:				; 8 pixels wide characters
;	move.b		(a0)+,(a1)+
;	move.b		(a2)+,(a1)+
;	move.b		(a3)+,(a1)+
;	move.b		(a5)+,(a1)+
	move.b		(a0)+,d2
  ifne both
	move.b		d2,(a4)+
  endc
	move.b		d2,(a1)+
	move.b		(a2)+,d2
	lsl.w		#8,d2
	move.b		(a3)+,d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
	move.b		(a5)+,d2
  ifne both
	move.b		d2,(a4)+
  endc
	move.b		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

sh_m_msb_8:
	neg.w		d1
	subq.w		#6,a6
	moveq		#8,d5
	sub.w		d1,d5	
.loop:				; 8 pixels wide characters
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	lsr.w		d1,d2
	move.b		(a0)+,d2
	swap		d2
	move.b		(a2)+,d2
	lsl.w		#8,d2
	move.b		(a3)+,d2
	lsl.l		d1,d2
	swap		d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
  ifne both
	move.w		2(a4),d2
  endc
  ifeq both
	move.w		2(a1),d2
  endc
	swap		d2
	lsl.l		d5,d2
	move.b		(a5)+,d2
	ror.l		d5,d2
	swap		d2
  ifne both
	move.l		d2,(a4)+
  endc
	move.l		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts


sh_multi_outchar_6:
	move.w		d1,d2		; Calculate column offset
	lsr.w		#4,d2
	add.w		d2,d2
	add.w		d2,a1
  ifne both
	add.w		d2,a4
  endc
	and.w		#$000f,d1	
	beq		sh_m_even_align_6	; Character at even word....
	subq.w		#8,d1
	beq		sh_m_odd_align_6	; ...even byte
	ble		sh_m_msb_6		; ...msb of word
;	addq.w		#8,d1
	subq.w		#6,a6
	move.w		d1,d3
	addq.w		#2,d3
	moveq		#4,d5
	moveq		#$03,d0
	sub.w		d1,d5
	beq		sh_m_lsb64
	ble		sh_m_lsb6h
sh_m_lsb6l:
.loop:
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	lsl.l		d1,d2
	move.b		(a0)+,d2
	lsl.l		#6,d2
	move.b		(a2)+,d2
	lsl.l		#6,d2
	move.b		(a3)+,d2
	lsl.l		d5,d2
	swap		d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
  ifne both
	move.w		2(a4),d2
  endc
  ifeq both
	move.w		2(a1),d2
  endc
	rol.l		d3,d2
	swap		d2
	and.b		d0,d2
	or.b		(a5)+,d2
	swap		d2
	ror.l		d3,d2
  ifne both
	move.l		d2,(a4)+
  endc
	move.l		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

sh_m_lsb64:
.loop:
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	lsl.l		#4,d2
	move.b		(a0)+,d2
	lsl.l		#6,d2
	move.b		(a2)+,d2
	lsl.l		#6,d2
	move.b		(a3)+,d2
	swap		d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
  ifne both
	move.w		2(a4),d2
  endc
  ifeq both
	move.w		2(a1),d2
  endc
	rol.l		#6,d2
	swap		d2
	and.b		d0,d2
	or.b		(a5)+,d2
	swap		d2
	ror.l		#6,d2
  ifne both
	move.l		d2,(a4)+
  endc
	move.l		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

sh_m_lsb6h:
	addq.w		#6,d5
	subq.w		#6,d3
.loop:
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	lsl.l		d1,d2
	move.b		(a0)+,d2
	lsl.l		#6,d2
	move.b		(a2)+,d2
	lsl.l		d5,d2
	swap		d2
  ifne both
	move.w		d2,(a4)+
  endc
	move.w		d2,(a1)+
  ifne both
	move.w		2(a4),d2
  endc
  ifeq both
	move.w		2(a1),d2
  endc
	rol.l		d3,d2
	swap		d2
	move.b		(a3)+,d2
	rol.l		#6,d2
	and.b		d0,d2
	or.b		(a5)+,d2
	rol.l		#4,d2
	rol.l		d5,d2
  ifne both
	move.l		d2,(a4)+
  endc
	move.l		d2,(a1)+
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts

sh_m_even_align_6:			; 6 pixels wide characters
.loop:
;	moveq		#0,d2
	move.b		(a0)+,d2
	lsl.w		#6,d2
	or.b		(a2)+,d2
	lsl.l		#6,d2
	or.b		(a3)+,d2
	lsl.l		#6,d2
	or.b		(a5)+,d2
	lsl.l		#6,d2
  ifne both
	move.b		3(a4),d2
  endc
  ifeq both
	move.b		3(a1),d2
  endc
  ifne both
	move.l		d2,(a4)
  endc
	move.l		d2,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts
	
sh_m_odd_align_6:			; 6 pixels wide characters
.loop:
  ifne both
	move.w		(a4),d2
  endc
  ifeq both
	move.w		(a1),d2
  endc
	move.b		(a0)+,d2
	lsl.l		#6,d2
	or.b		(a2)+,d2
	lsl.l		#6,d2
	or.b		(a3)+,d2
;	rol.l		#6,d2
	lsl.l		#4,d2		; These two lines should be better
	rol.l		#2,d2
	or.b		(a5)+,d2
	ror.l		#2,d2
  ifne both
	move.l		d2,(a4)
  endc
	move.l		d2,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts
	
sh_m_msb_6:				; Characters start in msb byte
	neg.w		d1		; Correct ?
.loop:				; 6 pixels wide characters
  ifne both
	move.l		(a4),d2
  endc
  ifeq both
	move.l		(a1),d2
  endc
	ror.l		d1,d2
	clr.w		d2
	swap		d2
	move.b		(a0)+,d2
	lsl.l		#6,d2
	or.b		(a2)+,d2
	lsl.l		#6,d2
	or.b		(a3)+,d2
	rol.l		#6,d2
	or.b		(a5)+,d2
	ror.l		#2,d2
	rol.l		d1,d2
  ifne both
	move.l		d2,(a4)
  endc
	move.l		d2,(a1)
	add.w		a6,a1
  ifne both
	add.w		a6,a4
  endc
	dbra		d4,.loop
	rts
  endc


  ifne 0
* ---
* Totally aligned 8 pixel output
* ---
display_byte:
;	move.b		mask(a7),d0
;	bne		dispb_end

	move.l		screen_addr(a7),a1
  ifne both
	move.l		shadow_addr(a7),a4
  endc

	move.w		d1,d2		; Calculate column offset
	lsr.w		#3,d2
	add.w		d2,a1
  ifne both
	add.w		d2,a4
  endc
	move.l		a1,d6
	move.w		d7,d5
	and.w		#$fffc,d5
	beq		db1_st_loop
	lsr.w		#2,d5
	subq.w		#1,d5
	subq.w		#4,a6
db4_loop:
;	move.w		#fnt_hght,d4	; Calculate offset of character
	move.w		h_w(a7),d4	; Height to draw (not necessarily character height)

	move.l		font_addr(a7),a0
	move.l		text_addr(a7),a1
	move.l		a0,a2
	move.l		a0,a3
	move.l		a0,a5
	calc_addr	a0,a1
	calc_addr	a2,a1
	calc_addr	a3,a1
	calc_addr	a5,a1
	move.l		a1,text_addr(a7)
	
	subq.w		#1,d4

	move.l		d6,a1
	addq.l		#4,d6
db_out_loop:			; 8 pixels wide characters
	move.b		(a0)+,(a1)+
	move.b		(a2)+,(a1)+
	move.b		(a3)+,(a1)+
	move.b		(a5)+,(a1)+
	add.w		a6,a1
	dbra		d4,db_out_loop

	dbra		d5,db4_loop
	addq.w		#4,a6
	
db1_st_loop:
	and.w		#$0003,d7
	beq		dispb_end
	subq.w		#1,d7
	move.l		text_addr(a7),a5
db1_loop:
;	move.w		#fnt_hght,d4	; Calculate offset of character
	move.w		h_w(a7),d4	; Height to draw (not necessarily character height)

	move.l		font_addr(a7),a0

	calc_addr	a0,a5
	subq.w		#1,d4

	move.l		d6,a1
	addq.l		#1,d6
db1_out_loop:
	move.b		(a0)+,(a1)
	add.w		a6,a1
	dbra		d4,db1_out_loop

;	addq.w		#pixels,d1
	dbra		d7,db1_loop

	move.l		a5,text_addr(a7)	
dispb_end:
;	addq.l		#4,a7
	rts
  endc

	end
