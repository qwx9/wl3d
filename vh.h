// ID_VH.H


#define WHITE			15			// graphics mode independant colors
#define BLACK			0
#define FIRSTCOLOR		1
#define SECONDCOLOR		12
#define F_WHITE			15
#define F_BLACK			0
#define F_FIRSTCOLOR	1
#define F_SECONDCOLOR	12

//===========================================================================

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


//===========================================================================


extern	pictabletype	_seg *pictable;
extern	pictabletype	_seg *picmtable;
extern	spritetabletype _seg *spritetable;

extern	u8int	fontcolor;
extern	s16int	fontnumber;
extern	s16int	px,py;

//
// Double buffer management routines
//

void VW_InitDoubleBuffer (void);
s16int	 VW_MarkUpdateBlock (s16int x1, s16int y1, s16int x2, s16int y2);
void VW_UpdateScreen (void);

//
// mode independant routines
// coordinates in pixels, rounded to best screen res
// regions marked in double buffer
//

void VWB_DrawTile8 (s16int x, s16int y, s16int tile);
void VWB_DrawPic (s16int x, s16int y, s16int chunknum);
void VWB_Bar (s16int x, s16int y, s16int width, s16int height, s16int color);
void VWB_DrawPropString	 (char far *string);
void VWB_Plot (s16int x, s16int y, s16int color);
void VWB_Hlin (s16int x1, s16int x2, s16int y, s16int color);
void VWB_Vlin (s16int y1, s16int y2, s16int x, s16int color);


//
// wolfenstein EGA compatability stuff
//
extern u8int far gamepal;

void VH_SetDefaultColors (void);

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
void	VW_MeasurePropString (char far *string, u16int *width, u16int *height);
#define EGAMAPMASK(x)	VGAMAPMASK(x)
#define EGAWRITEMODE(x)	VGAWRITEMODE(x)

#define LatchDrawChar(x,y,p) VL_LatchToScreen(latchpics[0]+(p)*16,2,8,x,y)
#define LatchDrawTile(x,y,p) VL_LatchToScreen(latchpics[1]+(p)*64,4,16,x,y)

void LatchDrawPic (u16int x, u16int y, u16int picnum);
void 	LoadLatchMem (void);
int 	FizzleFade (u16int source, u16int dest,
	u16int width,u16int height, u16int frames,int abortable);


#define NUMLATCHPICS	100
extern	u16int	latchpics[NUMLATCHPICS];
extern	u16int freelatch;

