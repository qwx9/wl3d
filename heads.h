#include "VERSION.H"

//--------------------------------------------------------------------------

extern	char		far signon;

#define	introscn	signon

#ifdef JAPAN
#ifdef JAPDEMO
#include "FOREIGN\JAPAN\GFXV_WJ1.H"
#else
#include "FOREIGN\JAPAN\GFXV_WJ6.H"
#endif
#include "AUDIOWL6.H"
#include "MAPSWL6.H"
#else

#ifndef SPEAR

#include "GFXV_WL6.H"
#include "AUDIOWL6.H"
#include "MAPSWL6.H"

#else

#ifndef SPEARDEMO
#include "GFXV_SOD.H"
#include "AUDIOSOD.H"
#include "MAPSSOD.H"
#else
#include "GFXV_SDM.H"
#include "AUDIOSDM.H"
#include "MAPSSDM.H"
#endif

#endif
#endif
//-----------------


#define GREXT	"VGA"

//
//	ID Engine
//	Types.h - Generic types, #defines, etc.
//	v1.0d1
//

typedef short s16int;
typedef int s32int;

#include "ID_MM.H"
#include "ID_PM.H"
#include "ID_CA.H"
#include "ID_VL.H"
#include "ID_VH.H"
#include "ID_IN.H"
#include "ID_SD.H"
#include "ID_US.H"


void	Quit (char *error);		// defined in user program

//
// replacing refresh manager with custom routines
//

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

#define	EXTENSION	"WLF"
