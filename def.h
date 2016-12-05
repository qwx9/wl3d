#define COLORBORDER(color)		asm{mov	dx,STATUS_REGISTER_1;in al,dx;\
	mov dx,ATR_INDEX;mov al,ATR_OVERSCAN;out dx,al;mov al,color;out	dx,al;\
	mov	al,32;out dx,al};

#define MAPSPOT(x,y,plane)		(*(mapsegs[plane]+farmapylookup[y]+x))

#define MAXWALLTILES	64				// max number of wall tiles

//
// tile constants
//

#define	ICONARROWS		90
#define PUSHABLETILE	98
#define EXITTILE		99	// at end of castle
#define ELEVATORTILE	21
#define	ALTELEVATORTILE	107

#define NUMBERCHARS	9

#define PLAYERSPEED		3000
#define RUNSPEED   		6000

#define	SCREENSEG		0xa000

#define SCREENBWIDE		80

#define HEIGHTRATIO		0.50		// also defined in id_mm.c

#define BORDERCOLOR	3
#define FLASHCOLOR	5
#define FLASHTICS	4


#define Dplr		Dmin			// player radius
										// to any actor center

#define NUMLATCHPICS	100

#define UNSIGNEDSHIFT	8

#define ANGLES		360					// must be divisable by 4
#define ANGLEQUAD	(ANGLES/4)
#define FINEANGLES	3600
#define ANG90		(FINEANGLES/4)
#define ANG180		(ANG90*2)
#define ANG270		(ANG90*3)
#define ANG360		(ANG90*4)
#define VANG90		(ANGLES/4)
#define VANG180		(VANG90*2)
#define VANG270		(VANG90*3)
#define VANG360		(VANG90*4)

#define	MAXSCALEHEIGHT	256				// largest scale on largest view

#define MAXVIEWWIDTH		320

#define MAPSIZE		64					// maps are 64*64 max

#define STATUSLINES		40

#define SCREENSIZE		(SCREENBWIDE*208)
#define PAGE1START		0
#define PAGE2START		(SCREENSIZE)
#define PAGE3START		(SCREENSIZE*2u)
#define	FREESTART		(SCREENSIZE*3u)

#define PIXRADIUS		512
#define STARTAMMO		8

#define	PORTTILESWIDE		20      // all drawing takes place inside a
#define UPDATEWIDE			PORTTILESWIDE
#define UPDATEHIGH			PORTTILESHIGH
#define SETFONTCOLOR(f,b) fontcolor=f;backcolor=b;

typedef enum {
	di_north,
	di_east,
	di_south,
	di_west
} controldir_t;

typedef enum {
	ac_badobject = -1,
	ac_no,
	ac_yes,
	ac_allways
} activetype;

typedef struct	statestruct
{
	int	rotate;
	s16int		shapenum;			// a shapenum of -1 means get from ob->temp1
	s16int		tictime;
	void	(*think) (),(*action) ();
	struct	statestruct	*next;
} statetype;

typedef struct statstruct
{
	u8int	tilex,tiley;
	u8int	*visspot;
	s16int		shapenum;			// if shapenum == -1 the obj has been removed
	u8int	flags;
	u8int	item;
} statobj_t;

extern	u8int far	*scalermemory;

extern	s32int		focallength;
extern	u16int	screenofs;

extern	int         startgame;
extern	s16int		mouseadjustment;

extern	int		ingame,fizzlein;
extern	u16int	latchpics[NUMLATCHPICS];
extern	gametype	gamestate;

extern	s32int		spearx,speary;
extern	u16int	spearangle;
extern	int		spearflag;

#define UPDATESIZE			(UPDATEWIDE*UPDATEHIGH)
extern	u8int		update[UPDATESIZE];

extern	s16int			extravbls;

extern	char		far *demoptr, far *lastdemoptr;
extern	uchar *demobuffer;

extern	u16int screenloc[3] = {PAGE1START,PAGE2START,PAGE3START};
extern	u16int freelatch;	// = FREESTART

// the door is the last picture before the sprites
#define SPdoor	(PMSpriteStart-8)
// e.g. sprs-8

extern	s32int 	lasttimecount;
extern	s32int 	frameon;
extern	int	fizzlein;

extern	u16int	wallheight[MAXVIEWWIDTH];

extern	s32int	focallength;
extern	s32int	mindist;

//
// wall optimization variables
//
extern s16int	lastside;		// true for vertical
extern s32int	lastintercept;
extern s16int	lasttilehit;
extern s16int	horizwall[MAXWALLTILES],vertwall[MAXWALLTILES];

//
// derived constants
//
extern	s32int 	scale;
extern	s32int	mindist;

// refresh variables
extern	s32int		postsource;
extern	u16int	scx;
extern	u16int	postwidth;

extern	dirtype opposite[9];
extern	dirtype diagonal[9][9];

typedef struct
{
	u16int	codeofs[65];
	u16int	width[65];
	u8int		code[];
}	t_compscale;

typedef struct
{
	u16int	leftpix,rightpix;
	u16int	dataofs[64];
// table data after dataofs[rightpix-leftpix+1]
}	t_compshape;


extern	t_compscale _seg *scaledirectory[MAXSCALEHEIGHT+1];
extern	s32int			fullscalefarcall[MAXSCALEHEIGHT+1];

extern	u8int		bitmasks1[8][8];
extern	u8int		bitmasks2[8][8];
extern	u16int	wordmasks[8][8];

extern	u8int		mapmasks1[4][8];
extern	u8int		mapmasks2[4][8];
extern	u8int		mapmasks3[4][8];

extern	s16int			maxscale,maxscaleshl2;

extern	int	insetupscaling;

extern	int		running;
extern	u16int	plux,pluy;		// player coordinates scaled to u16int

extern	s16int			anglefrac;
extern	s16int			facecount;

extern	u16int	doorposition[Ndoor],pwallstate;

extern u16int	pwallstate;
extern u16int	pwallpos;			// amount a pushable wall has been moved (0-63)
extern u16int	pwallx,pwally;
extern s16int			pwalldir;
