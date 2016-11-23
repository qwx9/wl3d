#define OP_RETF	0xcb

/*
=============================================================================

						  GLOBALS

=============================================================================
*/

t_compscale _seg *scaledirectory[MAXSCALEHEIGHT+1];
s32int			fullscalefarcall[MAXSCALEHEIGHT+1];

s16int			maxscale,maxscaleshl2;

/*
=============================================================================

						  LOCALS

=============================================================================
*/

t_compscale 	_seg *work;
u16int BuildCompScale (s16int height, uchar **finalspot);

s16int			stepbytwo;

//===========================================================================

/*
==============
=
= BadScale
=
==============
*/

void far BadScale (void)
{
	Quit ("BadScale called!");
}


void SetupScaling (s16int maxscaleheight)
{
	s16int		i,x,y;
	u8int	far *dest;

The dynamically compiled scaling routines are now a Bad Thing.  
On uncached machines (the original target) they are the fastest 
possible way to scale walls, but on modern processors you just 
wind up thrashing the code cash and wrecking performance.  
A simple looping texture mapper would be faster on 486+ machines.

	maxscaleheight/=2;			// one scaler every two pixels

	maxscale = maxscaleheight-1;
	maxscaleshl2 = maxscale<<2;

//
// free up old scalers
//
	for (i=1;i<MAXSCALEHEIGHT;i++)
	{
		if (scaledirectory[i])
			MM_FreePtr (&(uchar *)scaledirectory[i]);
		if (i>=stepbytwo)
			i += 2;
	}
	memset (scaledirectory,0,sizeof(scaledirectory));

//
// build the compiled scalers
//
	stepbytwo = vw.dy/2;	// save space by double stepping
	MM_GetPtr (&(uchar *)work,20000);

	for (i=1;i<=maxscaleheight;i++)
	{
		BuildCompScale (i*2,&(uchar *)scaledirectory[i]);
		if (i>=stepbytwo)
			i+= 2;
	}
	MM_FreePtr (&(uchar *)work);

//
// compact memory and lock down scalers
//
	for (i=1;i<=maxscaleheight;i++)
	{
		MM_SetLock (&(uchar *)scaledirectory[i],true);
		fullscalefarcall[i] = (u16int)scaledirectory[i];
		fullscalefarcall[i] <<=16;
		fullscalefarcall[i] += scaledirectory[i]->codeofs[0];
		if (i>=stepbytwo)
		{
			scaledirectory[i+1] = scaledirectory[i];
			fullscalefarcall[i+1] = fullscalefarcall[i];
			scaledirectory[i+2] = scaledirectory[i];
			fullscalefarcall[i+2] = fullscalefarcall[i];
			i+=2;
		}
	}
	scaledirectory[0] = scaledirectory[1];
	fullscalefarcall[0] = fullscalefarcall[1];

//
// check for oversize wall drawing
//
	for (i=maxscaleheight;i<MAXSCALEHEIGHT;i++)
		fullscalefarcall[i] = (uintptr)BadScale;
}

//===========================================================================

/*
========================
=
= BuildCompScale
=
= Builds a compiled scaler object that will scale a 64 tall object to
= the given height (centered vertically on the screen)
=
= height should be even
=
= Call with
= ---------
= DS:SI		Source for scale
= ES:DI		Dest for scale
=
= Calling the compiled scaler only destroys AL
=
========================
*/

u16int BuildCompScale (s16int height, uchar **finalspot)
{
	a simple looping texture mapper would be better, simpler, faster

	u8int		far *code;

	s16int			i;
	s32int		fix,step;
	u16int	src,totalscaled,totalsize;
	s16int			startpix,endpix,toppix;


	step = ((s32int)height<<16) / 64;
	code = &work->code[0];
	toppix = (vw.dy-height)/2;
	fix = 0;

	for (src=0;src<=64;src++)
	{
		startpix = fix>>16;
		fix += step;
		endpix = fix>>16;

		if (endpix>startpix)
			work->width[src] = endpix-startpix;
		else
			work->width[src] = 0;

//
// mark the start of the code
//
		work->codeofs[src] = FP_OFF(code);

//
// compile some code if the source pixel generates any screen pixels
//
		startpix+=toppix;
		endpix+=toppix;

		if (startpix == endpix || endpix < 0 || startpix >= vw.dy || src == 64)
			continue;

	//
	// mov al,[si+src]
	//
		*code++ = 0x8a;
		*code++ = 0x44;
		*code++ = src;

		for (;startpix<endpix;startpix++)
		{
			if (startpix >= vw.dy)
				break;						// off the bottom of the view area
			if (startpix < 0)
				continue;					// not into the view area

		//
		// mov [es:di+heightofs],al
		//
			*code++ = 0x26;
			*code++ = 0x88;
			*code++ = 0x85;
			*((u16int far *)code)++ = startpix*SCREENBWIDE;
		}

	}

//
// retf
//
	*code++ = 0xcb;

	totalsize = FP_OFF(code);
	MM_GetPtr (finalspot,totalsize);
	_fmemcpy ((u8int _seg *)(*finalspot),(u8int _seg *)work,totalsize);

	return totalsize;
}


/*
=======================
=
= ScaleLine
=
= linescale should have the high word set to the segment of the scaler
=
=======================
*/

extern	s16int			slinex,slinewidth;
extern	u16int	far *linecmds;
extern	s32int		linescale;
extern	u16int	maskword;

u8int	mask1,mask2,mask3;


void near ScaleLine (void)
{
asm	mov	cx,WORD PTR [linescale+2]
asm	mov	es,cx						// segment of scaler

asm	mov bp,WORD PTR [linecmds]
asm	mov	dx,SC_INDEX+1				// to set SC_MAPMASK

asm	mov	bx,[slinex]
asm	mov	di,bx
asm	shr	di,2						// X in bytes
asm	add	di,[bufferofs]
asm	and	bx,3
asm	shl	bx,3
asm	add	bx,[slinewidth]				// bx = (pixel*8+pixwidth)
asm	mov	al,BYTE [mapmasks3-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ds,WORD PTR [linecmds+2]
asm	or	al,al
asm	jz	notthreebyte				// scale across three bytes
asm	jmp	threebyte
notthreebyte:
asm	mov	al,BYTE PTR ss:[mapmasks2-1+bx]	// -1 because pixwidth of 1 is first
asm	or	al,al
asm	jnz	twobyte						// scale across two bytes

//
// one byte scaling
//
asm	mov	al,BYTE PTR ss:[mapmasks1-1+bx]	// -1 because pixwidth of 1 is first
asm	out	dx,al						// set map mask register

scalesingle:

asm	mov	bx,[ds:bp]					// table location of rtl to patch
asm	or	bx,bx
asm	jz	linedone					// 0 signals end of segment list
asm	mov	bx,[es:bx]
asm	mov	dl,[es:bx]					// save old value
asm	mov	BYTE PTR es:[bx],OP_RETF	// patch a RETF in
asm	mov	si,[ds:bp+4]				// table location of entry spot
asm	mov	ax,[es:si]
asm	mov	WORD PTR ss:[linescale],ax	// call here to start scaling
asm	mov	si,[ds:bp+2]				// corrected top of shape for this segment
asm	add	bp,6						// next segment list

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	call ss:[linescale]				// scale the segment of pixels

asm	mov	es,cx						// segment of scaler
asm	mov	BYTE PTR es:[bx],dl			// unpatch the RETF
asm	jmp	scalesingle					// do the next segment


//
// done
//
linedone:
asm	mov	ax,ss
asm	mov	ds,ax
return;

//
// two byte scaling
//
twobyte:
asm	mov	ss:[mask2],al
asm	mov	al,BYTE PTR ss:[mapmasks1-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ss:[mask1],al

scaledouble:

asm	mov	bx,[ds:bp]					// table location of rtl to patch
asm	or	bx,bx
asm	jz	linedone					// 0 signals end of segment list
asm	mov	bx,[es:bx]
asm	mov	cl,[es:bx]					// save old value
asm	mov	BYTE PTR es:[bx],OP_RETF	// patch a RETF in
asm	mov	si,[ds:bp+4]				// table location of entry spot
asm	mov	ax,[es:si]
asm	mov	WORD PTR ss:[linescale],ax	// call here to start scaling
asm	mov	si,[ds:bp+2]				// corrected top of shape for this segment
asm	add	bp,6						// next segment list

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	mov	al,ss:[mask1]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	inc	di
asm	mov	al,ss:[mask2]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	dec	di

asm	mov	es,WORD PTR ss:[linescale+2] // segment of scaler
asm	mov	BYTE PTR es:[bx],cl			// unpatch the RETF
asm	jmp	scaledouble					// do the next segment


//
// three byte scaling
//
threebyte:
asm	mov	ss:[mask3],al
asm	mov	al,BYTE PTR ss:[mapmasks2-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ss:[mask2],al
asm	mov	al,BYTE PTR ss:[mapmasks1-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ss:[mask1],al

scaletriple:

asm	mov	bx,[ds:bp]					// table location of rtl to patch
asm	or	bx,bx
asm	jz	linedone					// 0 signals end of segment list
asm	mov	bx,[es:bx]
asm	mov	cl,[es:bx]					// save old value
asm	mov	BYTE PTR es:[bx],OP_RETF	// patch a RETF in
asm	mov	si,[ds:bp+4]				// table location of entry spot
asm	mov	ax,[es:si]
asm	mov	WORD PTR ss:[linescale],ax	// call here to start scaling
asm	mov	si,[ds:bp+2]				// corrected top of shape for this segment
asm	add	bp,6						// next segment list

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	mov	al,ss:[mask1]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	inc	di
asm	mov	al,ss:[mask2]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	inc	di
asm	mov	al,ss:[mask3]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	dec	di
asm	dec	di

asm	mov	es,WORD PTR ss:[linescale+2] // segment of scaler
asm	mov	BYTE PTR es:[bx],cl			// unpatch the RETF
asm	jmp	scaletriple					// do the next segment


}


/*
=======================
=
= scalevis
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/

static	s32int		longtemp;

void scalevis (s16int xcenter, s16int shapenum, u16int height)
{
	t_compshape	_seg *shape;
	t_compscale _seg *comptable;
	u16int	scale,srcx,stopx,tempx;	/* /!\ scale shadow */
	s16int			t;
	u16int	far *cmdptr;
	int		leftvis,rightvis;


	shape = PM_GetSpritePage (shapenum);

	scale = height>>3;						// low three bits are fractional
	if (!scale || scale>maxscale)
		return;								// too close or far away
	comptable = scaledirectory[scale];

	*(((u16int *)&linescale)+1)=(u16int)comptable;	// seg of far call
	*(((u16int *)&linecmds)+1)=(u16int)shape;		// seg of shape

//
// scale to the left (from pixel 31 to shape->leftpix)
//
	srcx = 32;
	slinex = xcenter;
	stopx = shape->leftpix;
	cmdptr = &shape->dataofs[31-stopx];

	while ( --srcx >=stopx && slinex>0)
	{
		(u16int)linecmds = *cmdptr--;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		if (slinewidth == 1)
		{
			slinex--;
			if (slinex<vw.dx)
			{
				if (wallheight[slinex] >= height)
					continue;		// obscured by closer wall
				ScaleLine ();
			}
			continue;
		}

		//
		// handle multi pixel lines
		//
		if (slinex>vw.dx)
		{
			slinex -= slinewidth;
			slinewidth = vw.dx-slinex;
			if (slinewidth<1)
				continue;		// still off the right side
		}
		else
		{
			if (slinewidth>slinex)
				slinewidth = slinex;
			slinex -= slinewidth;
		}


		leftvis = (wallheight[slinex] < height);
		rightvis = (wallheight[slinex+slinewidth-1] < height);

		if (leftvis)
		{
			if (rightvis)
				ScaleLine ();
			else
			{
				while (wallheight[slinex+slinewidth-1] >= height)
					slinewidth--;
				ScaleLine ();
			}
		}
		else
		{
			if (!rightvis)
				continue;		// totally obscured

			while (wallheight[slinex] >= height)
			{
				slinex++;
				slinewidth--;
			}
			ScaleLine ();
			break;			// the rest of the shape is gone
		}
	}


//
// scale to the right
//
	slinex = xcenter;
	stopx = shape->rightpix;
	if (shape->leftpix<31)
	{
		srcx = 31;
		cmdptr = &shape->dataofs[32-shape->leftpix];
	}
	else
	{
		srcx = shape->leftpix-1;
		cmdptr = &shape->dataofs[0];
	}
	slinewidth = 0;

	while ( ++srcx <= stopx && (slinex+=slinewidth)<vw.dx)
	{
		(u16int)linecmds = *cmdptr++;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		if (slinewidth == 1)
		{
			if (slinex>=0 && wallheight[slinex] < height)
			{
				ScaleLine ();
			}
			continue;
		}

		//
		// handle multi pixel lines
		//
		if (slinex<0)
		{
			if (slinewidth <= -slinex)
				continue;		// still off the left edge

			slinewidth += slinex;
			slinex = 0;
		}
		else
		{
			if (slinex + slinewidth > vw.dx)
				slinewidth = vw.dx-slinex;
		}


		leftvis = (wallheight[slinex] < height);
		rightvis = (wallheight[slinex+slinewidth-1] < height);

		if (leftvis)
		{
			if (rightvis)
			{
				ScaleLine ();
			}
			else
			{
				while (wallheight[slinex+slinewidth-1] >= height)
					slinewidth--;
				ScaleLine ();
				break;			// the rest of the shape is gone
			}
		}
		else
		{
			if (rightvis)
			{
				while (wallheight[slinex] >= height)
				{
					slinex++;
					slinewidth--;
				}
				ScaleLine ();
			}
			else
				continue;		// totally obscured
		}
	}
}



/*
=======================
=
= scalespr
=
= NO CLIPPING, height in pixels
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/

void scalespr (s16int xcenter, s16int shapenum, u16int height)
{
	t_compshape	_seg *shape;
	t_compscale _seg *comptable;
	u16int	scale,srcx,stopx,tempx;	/* /!\ scale shadow */
	s16int			t;
	u16int	far *cmdptr;
	int		leftvis,rightvis;


	shape = PM_GetSpritePage (shapenum);

	scale = height>>1;
	comptable = scaledirectory[scale];

	*(((u16int *)&linescale)+1)=(u16int)comptable;	// seg of far call
	*(((u16int *)&linecmds)+1)=(u16int)shape;		// seg of shape

//
// scale to the left (from pixel 31 to shape->leftpix)
//
	srcx = 32;
	slinex = xcenter;
	stopx = shape->leftpix;
	cmdptr = &shape->dataofs[31-stopx];

	while ( --srcx >=stopx )
	{
		(u16int)linecmds = *cmdptr--;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		slinex -= slinewidth;
		ScaleLine ();
	}


//
// scale to the right
//
	slinex = xcenter;
	stopx = shape->rightpix;
	if (shape->leftpix<31)
	{
		srcx = 31;
		cmdptr = &shape->dataofs[32-shape->leftpix];
	}
	else
	{
		srcx = shape->leftpix-1;
		cmdptr = &shape->dataofs[0];
	}
	slinewidth = 0;

	while ( ++srcx <= stopx )
	{
		(u16int)linecmds = *cmdptr++;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		ScaleLine ();
		slinex+=slinewidth;
	}
}




//
// bit mask tables for drawing scaled strips up to eight pixels wide
//
// down here so the STUPID inline assembler doesn't get confused!
//


u8int	mapmasks1[4][8] = {
{1 ,3 ,7 ,15,15,15,15,15},
{2 ,6 ,14,14,14,14,14,14},
{4 ,12,12,12,12,12,12,12},
{8 ,8 ,8 ,8 ,8 ,8 ,8 ,8} };

u8int	mapmasks2[4][8] = {
{0 ,0 ,0 ,0 ,1 ,3 ,7 ,15},
{0 ,0 ,0 ,1 ,3 ,7 ,15,15},
{0 ,0 ,1 ,3 ,7 ,15,15,15},
{0 ,1 ,3 ,7 ,15,15,15,15} };

u8int	mapmasks3[4][8] = {
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0},
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1},
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,3},
{0 ,0 ,0 ,0 ,0 ,1 ,3 ,7} };


u16int	wordmasks[8][8] = {
{0x0080,0x00c0,0x00e0,0x00f0,0x00f8,0x00fc,0x00fe,0x00ff},
{0x0040,0x0060,0x0070,0x0078,0x007c,0x007e,0x007f,0x807f},
{0x0020,0x0030,0x0038,0x003c,0x003e,0x003f,0x803f,0xc03f},
{0x0010,0x0018,0x001c,0x001e,0x001f,0x801f,0xc01f,0xe01f},
{0x0008,0x000c,0x000e,0x000f,0x800f,0xc00f,0xe00f,0xf00f},
{0x0004,0x0006,0x0007,0x8007,0xc007,0xe007,0xf007,0xf807},
{0x0002,0x0003,0x8003,0xc003,0xe003,0xf003,0xf803,0xfc03},
{0x0001,0x8001,0xc001,0xe001,0xf001,0xf801,0xfc01,0xfe01} };

s16int			slinex,slinewidth;
u16int	far *linecmds;
s32int		linescale;
u16int	maskword;

