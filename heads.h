#define	PORTTILESWIDE		20      // all drawing takes place inside a
#define	PORTTILESHIGH		13		// non displayed port of this size

#define UPDATEWIDE			PORTTILESWIDE
#define UPDATEHIGH			PORTTILESHIGH

#define	MAXTICS				10
#define DEMOTICS			4

#define	UPDATETERMINATE	0x0301

extern	u16int	mapwidth,mapheight,tics;
extern	int		compatability;

extern	u8int		*updateptr;
extern	u16int	uwidthtable[UPDATEHIGH];
extern	u16int	blockstarts[UPDATEWIDE*UPDATEHIGH];

extern	u8int		fontcolor,backcolor;

#define SETFONTCOLOR(f,b) fontcolor=f;backcolor=b;
