u16int	bufferofs;
u16int	displayofs,pelpan;

u16int	screenseg=SCREENSEG;		// set to 0xa000 for asm convenience

u16int	linewidth;
u16int	ylookup[MAXSCANLINES];

int		screenfaded;
u16int	bordercolor;

int		fastpalette;				// if true, use outsb to set

//===========================================================================

// asm
void VL_SetCRTC (s16int crtc);


/*
=============================================================================

						PALETTE OPS

		To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/

/*
==================
=
= VL_ColorBorder
=
==================
*/

void VL_ColorBorder (s16int color)
{
	_AH=0x10;
	_AL=1;
	_BH=color;
	geninterrupt (0x10);
	bordercolor = color;
}



/*
=============================================================================

							PIXEL OPS

=============================================================================
*/

u8int	pixmasks[4] = {1,2,4,8};
u8int	leftmasks[4] = {15,14,12,8};
u8int	rightmasks[4] = {1,3,7,15};


/*
=================
=
= VL_Plot
=
=================
*/

void VL_Plot (s16int x, s16int y, s16int color)
{
	u8int mask;

	mask = pixmasks[x&3];
	VGAMAPMASK(mask);
	*(u8int far *)MK_FP(SCREENSEG,bufferofs+(ylookup[y]+(x>>2))) = color;
	VGAMAPMASK(15);
}


/*
=================
=
= VL_Vlin
=
=================
*/

void VL_Vlin (s16int x, s16int y, s16int height, s16int color)
{
	u8int	far *dest,mask;

	mask = pixmasks[x&3];
	VGAMAPMASK(mask);

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2));

	while (height--)
	{
		*dest = color;
		dest += linewidth;
	}

	VGAMAPMASK(15);
}

/*
============================================================================

							MEMORY OPS

============================================================================
*/

/*
=================
=
= VL_MemToLatch
=
=================
*/

void VL_MemToLatch (u8int far *source, s16int width, s16int height, u16int dest)
{
	u16int	count;
	u8int	plane,mask;

	count = ((width+3)/4)*height;
	mask = 1;
	for (plane = 0; plane<4 ; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;

asm	mov	cx,count
asm mov ax,SCREENSEG
asm mov es,ax
asm	mov	di,[dest]
asm	lds	si,[source]
asm	rep movsb
asm mov	ax,ss
asm	mov	ds,ax

		source+= count;
	}
}


//===========================================================================


/*
=================
=
= VL_MemToScreen
=
= Draws a block of data to the screen.
=
=================
*/

void VL_MemToScreen (u8int far *source, s16int width, s16int height, s16int x, s16int y)
{
	u8int    far *screen,far *dest,mask;
	s16int		plane;

	width>>=2;
	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2) );
	mask = 1 << (x&3);

	for (plane = 0; plane<4; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;
		if (mask == 16)
			mask = 1;

		screen = dest;
		for (y=0;y<height;y++,screen+=linewidth,source+=width)
			_fmemcpy (screen,source,width);
	}
}

//==========================================================================


/*
=================
=
= VL_MaskedToScreen
=
= Masks a block of main memory to the screen.
=
=================
*/

void VL_MaskedToScreen (u8int far *source, s16int width, s16int height, s16int x, s16int y)
{
	u8int    far *screen,far *dest,mask;
	u8int	far *maskptr;
	s16int		plane;

	width>>=2;
	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2) );
//	mask = 1 << (x&3);

//	maskptr = source;

	for (plane = 0; plane<4; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;
		if (mask == 16)
			mask = 1;

		screen = dest;
		for (y=0;y<height;y++,screen+=linewidth,source+=width)
			_fmemcpy (screen,source,width);
	}
}

//==========================================================================

/*
=================
=
= VL_LatchToScreen
=
=================
*/

void VL_LatchToScreen (u16int source, s16int width, s16int height, s16int x, s16int y)
{
	VGAWRITEMODE(1);
	VGAMAPMASK(15);

asm	mov	di,[y]				// dest = bufferofs+ylookup[y]+(x>>2)
asm	shl	di,1
asm	mov	di,[WORD PTR ylookup+di]
asm	add	di,[bufferofs]
asm	mov	ax,[x]
asm	shr	ax,2
asm	add	di,ax

asm	mov	si,[source]
asm	mov	ax,[width]
asm	mov	bx,[linewidth]
asm	sub	bx,ax
asm	mov	dx,[height]
asm	mov	cx,SCREENSEG
asm	mov	ds,cx
asm	mov	es,cx

drawline:
asm	mov	cx,ax
asm	rep movsb
asm	add	di,bx
asm	dec	dx
asm	jnz	drawline

asm	mov	ax,ss
asm	mov	ds,ax

	VGAWRITEMODE(0);
}
