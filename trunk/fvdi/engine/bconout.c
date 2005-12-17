/*
 * fVDI Bconout(con) redirection
 *
 * $Id: bconout.c,v 1.1 2005-12-17 01:08:31 standa Exp $
 *
 * Copyright 1993, Johan Klockars, 2005 Standa Opichal 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

void bconout_char(long ch)
{
	static short esc = 0;
	static short inverse = 0;
	static short params = 0;
	static char str[] = " ";

	ch &= 0xff;

	if (esc) {
		switch (ch) {
			case 'p':
				inverse = 1;
				break;
			case 'q':
				inverse = 0;
				break;
			case 'K':
				; /* fallthrough: delete to end of line (I guess) */
			case 'Y':
				params = 2;
				break;
			
		}

		esc = 0;
		return;
	}

	if (params > 0) {
		params--;
		return;
	}

	switch (ch) {
#if 0
		static int x = 0;
		static int y = 0;

		case 10:
			y += cell_h;
			if (y > height - cell_h) {
				intin[0] = 3;
				ptsin[0] = 0;
				if (!scroll_height)
					ptsin[1] = cell_h;
				else
					ptsin[1] = height - 1 - (scroll_height - 1) * cell_h;
				ptsin[2] = width - 1;
				ptsin[3] = height - 1;
				ptsin[4] = 0;
				if (!scroll_height)
					ptsin[5] = 0;
				else
					ptsin[5] = height - 1 - scroll_height * cell_h;
				ptsin[6] = width - 1;
				ptsin[7] = height - 1 - cell_h;
				*(long *)&control.dummy[0] = 0;       /* Should really point to MFDB with addr 0 */
				*(long *)&control.dummy[2] = 0;
				sdo_vdi(VDI, handle, 109, 0, 4, 1);    /* vro_cpyfm */
				ptsin[0] = 0;
				ptsin[1] = height - cell_h;
				ptsin[2] = width - 1;
				ptsin[3] = height - 1;
				sdo_vdi(VDI, handle, 114, 0, 2, 0);    /* vr_recfl */
				y = height - cell_h;
			}
			break;
		case 9:
			x = (x + 8) & 0xfff8;
			break;
		case 13:
			x = 0;
			break;
		case 27: /* ESC */
			esc = 1;
			break;
		default:
			intin[0] = ch;
			ptsin[0] = x;
			ptsin[1] = y;
			sdo_vdi(VDI, handle, 8, 0, 1, 1);
			x += cell_w;
			break;
#else
		default:
			*str = (ch == 32 && inverse) ? '_' : ch;
			puts(str);
#endif
	}
}

