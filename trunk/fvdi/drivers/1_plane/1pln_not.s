*-------------------------------------------------------*
*	Dummy routines for unimplemented stuff		*	
*-------------------------------------------------------*

* Comment out the XDEFs for functions that are implemented!


* Assembly functions
*
;	xdef		_write_pixel
;	xdef		_read_pixel
;	xdef		_expand_area,expand_area
;	xdef		_blit_area,blit_area
;	xdef		_fill_area,fill_area
	xdef		_fill_polygon,fill_polygon
;	xdef		_line_draw,line_draw
;	xdef		_text_area,text_area
;	xdef		_mouse_draw,mouse_draw
;	xdef		_set_colours,set_colours
;	xdef		_get_colour


* C functions
*
	xdef		_c_write_pixel
	xdef		_c_read_pixel
	xdef		_c_expand_area
	xdef		_c_blit_area
	xdef		_c_fill_area
	xdef		_c_fill_polygon
	xdef		_c_line_draw
	xdef		_c_text_area
	xdef		_c_mouse_draw
	xdef		_c_set_colours
	xdef		_c_get_colour

	text

_write_pixel:
write_pixel:
_read_pixel:
read_pixel:
_line_draw:
line_draw:
_expand_area:
expand_area:
_blit_area:
blit_area:
_fill_area:
fill_area:
_fill_polygon:
fill_polygon:
_text_area:
text_area:
_mouse_draw:
mouse_draw:
_set_colours:
set_colours:
_get_colour:
	rts

_c_write_pixel:
_c_read_pixel:
_c_line_draw:
_c_expand_area:
_c_blit_area:
_c_fill_area:
_c_fill_polygon:
_c_text_area:
_c_mouse_draw:
_c_set_colours:
_c_get_colour:
	rts

	end
