///////////////////////////////////////////////////////////////////////////
//
//	USL_XORICursor() - XORs the I-bar text cursor. Used by US_LineInput()
//
///////////////////////////////////////////////////////////////////////////
static void
USL_XORICursor(s16int x,s16int y,char *s,u16int cursor)
{
	static	int	status;		// VGA doesn't XOR...
	char	buf[MaxString];
	s16int		temp;
	u16int	w,h;

	strcpy(buf,s);
	buf[cursor] = '\0';
	USL_MeasureString(buf,&w,&h);

	px = x + w - 1;
	py = y;
	if (status^=1)
		USL_DrawString("\x80");
	else
	{
		temp = fontcolor;
		fontcolor = backcolor;
		USL_DrawString("\x80");
		fontcolor = temp;
	}

}

///////////////////////////////////////////////////////////////////////////
//
//	US_LineInput() - Gets a line of user input at (x,y), the string defaults
//		to whatever is pointed at by def. Input is restricted to maxchars
//		chars or maxwidth pixels wide. If the user hits escape (and escok is
//		true), nothing is copied into buf, and false is returned. If the
//		user hits return, the current string is copied into buf, and true is
//		returned
//
///////////////////////////////////////////////////////////////////////////
int
US_LineInput(s16int x,s16int y,char *buf,char *def,int escok,
				s16int maxchars,s16int maxwidth)
{
	int		redraw,
				cursorvis,cursormoved,
				done,result;
	u8int	sc;
	char		c,
				s[MaxString],olds[MaxString];
	u16int		i,
				cursor,
				w,h,
				len,temp;
	u32int	lasttime;

	if (def)
		strcpy(s,def);
	else
		*s = '\0';
	*olds = '\0';
	cursor = strlen(s);
	cursormoved = redraw = true;

	cursorvis = done = false;
	lasttime = TimeCount;
	LastASCII = key_None;
	LastScan = sc_None;

	while (!done)
	{
		if (cursorvis)
			USL_XORICursor(x,y,s,cursor);

	asm	pushf
	asm	cli

		sc = LastScan;
		LastScan = sc_None;
		c = LastASCII;
		LastASCII = key_None;

	asm	popf

		switch (sc)
		{
		case sc_LeftArrow:
			if (cursor)
				cursor--;
			c = key_None;
			cursormoved = true;
			break;
		case sc_RightArrow:
			if (s[cursor])
				cursor++;
			c = key_None;
			cursormoved = true;
			break;
		case sc_Home:
			cursor = 0;
			c = key_None;
			cursormoved = true;
			break;
		case sc_End:
			cursor = strlen(s);
			c = key_None;
			cursormoved = true;
			break;

		case sc_Return:
			strcpy(buf,s);
			done = true;
			result = true;
			c = key_None;
			break;
		case sc_Escape:
			if (escok)
			{
				done = true;
				result = false;
			}
			c = key_None;
			break;

		case sc_BackSpace:
			if (cursor)
			{
				strcpy(s + cursor - 1,s + cursor);
				cursor--;
				redraw = true;
			}
			c = key_None;
			cursormoved = true;
			break;
		case sc_Delete:
			if (s[cursor])
			{
				strcpy(s + cursor,s + cursor + 1);
				redraw = true;
			}
			c = key_None;
			cursormoved = true;
			break;

		case 0x4c:	// Keypad 5
		case sc_UpArrow:
		case sc_DownArrow:
		case sc_PgUp:
		case sc_PgDn:
		case sc_Insert:
			c = key_None;
			break;
		}

		if (c)
		{
			len = strlen(s);
			USL_MeasureString(s,&w,&h);

			if
			(
				isprint(c)
			&&	(len < MaxString - 1)
			&&	((!maxchars) || (len < maxchars))
			&&	((!maxwidth) || (w < maxwidth))
			)
			{
				for (i = len + 1;i > cursor;i--)
					s[i] = s[i - 1];
				s[cursor++] = c;
				redraw = true;
			}
		}

		if (redraw)
		{
			px = x;
			py = y;
			temp = fontcolor;
			fontcolor = backcolor;
			USL_DrawString(olds);
			fontcolor = temp;
			strcpy(olds,s);

			px = x;
			py = y;
			USL_DrawString(s);

			redraw = false;
		}

		if (cursormoved)
		{
			cursorvis = false;
			lasttime = TimeCount - TickBase;

			cursormoved = false;
		}
		if (TimeCount - lasttime > TickBase / 2)
		{
			lasttime = TimeCount;

			cursorvis ^= true;
		}
		if (cursorvis)
			USL_XORICursor(x,y,s,cursor);

		VW_UpdateScreen();
	}

	if (cursorvis)
		USL_XORICursor(x,y,s,cursor);
	if (!result)
	{
		px = x;
		py = y;
		USL_DrawString(olds);
	}
	VW_UpdateScreen();

	IN_ClearKeysDown();
	return(result);
}
