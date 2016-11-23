#define WHITE			15			// graphics mode independant colors
#define BLACK			0
#define FIRSTCOLOR		1
#define SECONDCOLOR		12
#define F_WHITE			15
#define F_BLACK			0
#define F_FIRSTCOLOR	1
#define F_SECONDCOLOR	12

#define MAXSHIFTS	1

typedef struct
{
	s16int width,height;
} pictabletype;

typedef struct
{
	s16int height;
	s16int location[256];
	char width[256];
} fontstruct;

extern	pictabletype	_seg *pictable;
extern	pictabletype	_seg *picmtable;
extern	spritetabletype _seg *spritetable;

extern	u8int	fontcolor;
extern	s16int	fontnumber;
extern	s16int	px,py;

#define VW_SetCRTC		VL_SetCRTC
#define VW_SetScreen	VL_SetScreen
#define VW_Bar			VL_Bar
#define VW_Plot			VL_Plot
#define VW_Hlin(x,z,y,c)	VL_Hlin(x,y,(z)-(x)+1,c)
#define VW_Vlin(y,z,x,c)	VL_Vlin(x,y,(z)-(y)+1,c)
#define VW_DrawPic		VH_DrawPic
#define VW_ColorBorder	VL_ColorBorder
#define VW_WaitVBL		VL_WaitVBL
#define VW_FadeIn()		VL_FadeIn(0,255,&gamepal,30);
#define VW_FadeOut()	VL_FadeOut(0,255,0,0,0,30);
#define VW_ScreenToScreen	VL_ScreenToScreen
#define VW_SetDefaultColors	VH_SetDefaultColors
#define EGAMAPMASK(x)	VGAMAPMASK(x)
#define EGAWRITEMODE(x)	VGAWRITEMODE(x)
#define LatchDrawChar(x,y,p) VL_LatchToScreen(latchpics[0]+(p)*16,2,8,x,y)
#define LatchDrawTile(x,y,p) VL_LatchToScreen(latchpics[1]+(p)*64,4,16,x,y)
#define NUMLATCHPICS	100

#define PIXTOBLOCK		4		// 16 pixels to an update block

u8int	update[UPDATEHIGH][UPDATEWIDE];

//==========================================================================

pictabletype	_seg *pictable;


s16int	px,py;
u8int	fontcolor,backcolor;
s16int	fontnumber;
s16int bufferwidth,bufferheight;


/*
=============================================================================

				Double buffer management routines

=============================================================================
*/


/*
=======================
=
= VW_MarkUpdateBlock
=
= Takes a pixel bounded block and marks the tiles in bufferblocks
= Returns 0 if the entire block is off the buffer screen
=
=======================
*/

s16int VW_MarkUpdateBlock (s16int x1, s16int y1, s16int x2, s16int y2)
{
	s16int	x,y,xt1,yt1,xt2,yt2,nextline;
	u8int *mark;

	xt1 = x1>>PIXTOBLOCK;
	yt1 = y1>>PIXTOBLOCK;

	xt2 = x2>>PIXTOBLOCK;
	yt2 = y2>>PIXTOBLOCK;

	if (xt1<0)
		xt1=0;
	else if (xt1>=UPDATEWIDE)
		return 0;

	if (yt1<0)
		yt1=0;
	else if (yt1>UPDATEHIGH)
		return 0;

	if (xt2<0)
		return 0;
	else if (xt2>=UPDATEWIDE)
		xt2 = UPDATEWIDE-1;

	if (yt2<0)
		return 0;
	else if (yt2>=UPDATEHIGH)
		yt2 = UPDATEHIGH-1;

	mark = updateptr + uwidthtable[yt1] + xt1;
	nextline = UPDATEWIDE - (xt2-xt1) - 1;

	for (y=yt1;y<=yt2;y++)
	{
		for (x=xt1;x<=xt2;x++)
			*mark++ = 1;			// this tile will need to be updated

		mark += nextline;
	}

	return 1;
}

void VWB_DrawTile8 (s16int x, s16int y, s16int tile)
{
	if (VW_MarkUpdateBlock (x,y,x+7,y+7))
		LatchDrawChar(x,y,tile);
}

void VWB_Plot (s16int x, s16int y, s16int color)
{
	if (VW_MarkUpdateBlock (x,y,x,y))
		VW_Plot(x,y,color);
}

void VWB_Vlin (s16int y1, s16int y2, s16int x, s16int color)
{
	if (VW_MarkUpdateBlock (x,y1,x,y2))
		VW_Vlin(y1,y2,x,color);
}

void VW_UpdateScreen (void)
{
	VH_UpdateScreen ();
}


/*
=============================================================================

						WOLFENSTEIN STUFF

=============================================================================
*/

/*
=====================
=
= LatchDrawPic
=
=====================
*/

void LatchDrawPic (u16int x, u16int y, u16int picnum)
{
	u16int wide, height, source;

	wide = pictable[picnum-STARTPICS].width;
	height = pictable[picnum-STARTPICS].height;
	source = latchpics[2+picnum-LATCHPICS_LUMP_START];

	VL_LatchToScreen (source,wide/4,height,x*8,y);
}


//==========================================================================

/*
===================
=
= LoadLatchMem
=
===================
*/

void LoadLatchMem (void)
{
	s16int	i,j,p,m,width,height,start,end;
	u8int	far *src;
	u16int	destoff;

//
// tile 8s
//
	→ fuck this
	latchpics[0] = freelatch;
	src = (u8int _seg *)grsegs[STARTTILE8];
	destoff = freelatch;

	for (i=0;i<NUMTILE8;i++)
	{
		VL_MemToLatch (src,8,8,destoff);
		src += 64;
		destoff +=16;
	}

//
// pics
//
	start = LATCHPICS_LUMP_START;
	end = LATCHPICS_LUMP_END;

	for (i=start;i<=end;i++)
	{
		latchpics[2+i-start] = destoff;
		width = pictable[i-STARTPICS].width;
		height = pictable[i-STARTPICS].height;
		VL_MemToLatch (grsegs[i],width,height,destoff);
		destoff += width/4 *height;
	}

	EGAMAPMASK(15);
}
