// ID_VL.H

// wolf compatability

#define MS_Quit	Quit

void Quit (char *error);

//===========================================================================


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


//===========================================================================

#define SCREENSEG		0xa000

#define SCREENWIDTH		80			// default screen width in bytes
#define MAXSCANLINES	200			// size of ylookup table

#define CHARWIDTH		2
#define TILEWIDTH		4

//===========================================================================

extern	u16int	bufferofs;			// all drawing is reletive to this
extern	u16int	displayofs,pelpan;	// last setscreen coordinates

extern	u16int	screenseg;			// set to 0xa000 for asm convenience

extern	u16int	linewidth;
extern	u16int	ylookup[MAXSCANLINES];

extern	int		screenfaded;
extern	u16int	bordercolor;

//===========================================================================

//
// VGA hardware routines
//

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


void VL_Startup (void);
void VL_Shutdown (void);

void VL_SetVGAPlane (void);
void VL_SetTextMode (void);
void VL_DePlaneVGA (void);
void VL_SetVGAPlaneMode (void);
void VL_ClearVideo (u8int color);

void VL_SetLineWidth (u16int width);
void VL_SetSplitScreen (s16int linenum);

void VL_WaitVBL (s16int vbls);
void VL_CrtcStart (s16int crtc);
void VL_SetScreen (s16int crtc, s16int pelpan);

void VL_FillPalette (s16int red, s16int green, s16int blue);
void VL_SetColor	(s16int color, s16int red, s16int green, s16int blue);
void VL_GetColor	(s16int color, s16int *red, s16int *green, s16int *blue);
void VL_SetPalette (u8int far *palette);
void VL_GetPalette (u8int far *palette);
void VL_FadeOut (s16int start, s16int end, s16int red, s16int green, s16int blue, s16int steps);
void VL_FadeIn (s16int start, s16int end, u8int far *palette, s16int steps);
void VL_ColorBorder (s16int color);

void VL_Plot (s16int x, s16int y, s16int color);
void VL_Hlin (u16int x, u16int y, u16int width, u16int color);
void VL_Vlin (s16int x, s16int y, s16int height, s16int color);
void VL_Bar (s16int x, s16int y, s16int width, s16int height, s16int color);

void VL_MungePic (u8int far *source, u16int width, u16int height);
void VL_DrawPicBare (s16int x, s16int y, u8int far *pic, s16int width, s16int height);
void VL_MemToLatch (u8int far *source, s16int width, s16int height, u16int dest);
void VL_ScreenToScreen (u16int source, u16int dest,s16int width, s16int height);
void VL_MemToScreen (u8int far *source, s16int width, s16int height, s16int x, s16int y);
void VL_MaskedToScreen (u8int far *source, s16int width, s16int height, s16int x, s16int y);

void VL_DrawTile8String (char *str, char far *tile8ptr, s16int printx, s16int printy);
void VL_DrawLatch8String (char *str, u16int tile8ptr, s16int printx, s16int printy);
void VL_SizeTile8String (char *str, s16int *width, s16int *height);
void VL_DrawPropString (char *str, u16int tile8ptr, s16int printx, s16int printy);
void VL_SizePropString (char *str, s16int *width, s16int *height, char far *font);

void VL_TestPaletteSet (void);

