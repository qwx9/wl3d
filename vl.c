u16int	bufferofs;
u16int	displayofs,pelpan;

#define SC_INDEX			0x3C4
#define SC_RESET			0
#define SC_CLOCK			1
#define SC_MAPMASK			2
#define SC_CHARMAP			3
#define SC_MEMMODE			4

#define CRTC_INDEX			0x3D4
#define CRTC_H_TOTAL		0
#define CRTC_H_DISPEND		1
#define CRTC_H_BLANK		2
#define CRTC_H_ENDBLANK		3
#define CRTC_H_RETRACE		4
#define CRTC_H_ENDRETRACE 	5
#define CRTC_V_TOTAL		6
#define CRTC_OVERFLOW		7
#define CRTC_ROWSCAN		8
#define CRTC_MAXSCANLINE 	9
#define CRTC_CURSORSTART 	10
#define CRTC_CURSOREND		11
#define CRTC_STARTHIGH		12
#define CRTC_STARTLOW		13
#define CRTC_CURSORHIGH		14
#define CRTC_CURSORLOW		15
#define CRTC_V_RETRACE		16
#define CRTC_V_ENDRETRACE 	17
#define CRTC_V_DISPEND		18
#define CRTC_OFFSET			19
#define CRTC_UNDERLINE		20
#define CRTC_V_BLANK		21
#define CRTC_V_ENDBLANK		22
#define CRTC_MODE			23
#define CRTC_LINECOMPARE 	24


#define GC_INDEX			0x3CE
#define GC_SETRESET			0
#define GC_ENABLESETRESET 	1
#define GC_COLORCOMPARE		2
#define GC_DATAROTATE		3
#define GC_READMAP			4
#define GC_MODE				5
#define GC_MISCELLANEOUS 	6
#define GC_COLORDONTCARE 	7
#define GC_BITMASK			8

#define ATR_INDEX			0x3c0
#define ATR_MODE			16
#define ATR_OVERSCAN		17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN			19
#define ATR_COLORSELECT		20

#define	STATUS_REGISTER_1    0x3da

#define PEL_WRITE_ADR		0x3c8
#define PEL_READ_ADR		0x3c7
#define PEL_DATA			0x3c9

#define SCREENSEG		0xa000

#define SCREENWIDTH		80			// default screen width in bytes
#define MAXSCANLINES	200			// size of ylookup table

#define TILEWIDTH		4

extern	u16int	bufferofs;			// all drawing is reletive to this
extern	u16int	displayofs,pelpan;	// last setscreen coordinates

extern	u16int	screenseg;			// set to 0xa000 for asm convenience

extern	u16int	linewidth;
extern	u16int	ylookup[MAXSCANLINES];

extern	int		screenfaded;
extern	u16int	bordercolor;

#define VGAWRITEMODE(x) asm{\
cli;\
mov dx,GC_INDEX;\
mov al,GC_MODE;\
out dx,al;\
inc dx;\
in al,dx;\
and al,252;\
or al,x;\
out dx,al;\
sti;}

#define VGAMAPMASK(x) asm{cli;mov dx,SC_INDEX;mov al,SC_MAPMASK;mov ah,x;out dx,ax;sti;}
#define VGAREADMAP(x) asm{cli;mov dx,GC_INDEX;mov al,GC_READMAP;mov ah,x;out dx,ax;sti;}

u16int	screenseg=SCREENSEG;		// set to 0xa000 for asm convenience

u16int	linewidth;
u16int	ylookup[MAXSCANLINES];

int		screenfaded;
u16int	bordercolor;

int		fastpalette;				// if true, use outsb to set

u8int	pixmasks[4] = {1,2,4,8};
u8int	leftmasks[4] = {15,14,12,8};
u8int	rightmasks[4] = {1,3,7,15};

void VL_Plot (s16int x, s16int y, s16int color)
{
	u8int mask;

	mask = pixmasks[x&3];
	VGAMAPMASK(mask);
	*(u8int far *)MK_FP(SCREENSEG,bufferofs+(ylookup[y]+(x>>2))) = color;
	VGAMAPMASK(15);
}

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
