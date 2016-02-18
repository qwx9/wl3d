// ID_CA.H
//===========================================================================

#define NUMMAPS		60
#define MAPPLANES	2

#define UNCACHEGRCHUNK(chunk)	{MM_FreePtr(&grsegs[chunk]);grneeded[chunk]&=~ca_levelbit;}

//===========================================================================

typedef	struct
{
	s32int		planestart[3];
	u16int	planelength[3];
	u16int	width,height;
	char		name[16];
} maptype;

//===========================================================================

extern	char		audioname[13];

extern	u8int 		_seg	*tinf;
extern	s16int			mapon;

extern	u16int	_seg	*mapsegs[MAPPLANES];
extern	maptype		_seg	*mapheaderseg[NUMMAPS];
extern	u8int		_seg	*audiosegs[NUMSNDCHUNKS];
extern	void		_seg	*grsegs[NUMCHUNKS];

extern	u8int		far	grneeded[NUMCHUNKS];
extern	u8int		ca_levelbit,ca_levelnum;

extern	char		*titleptr[8];

extern	s16int			profilehandle,debughandle;

extern	char		extension[5],
			gheadname[10],
			gfilename[10],
			gdictname[10],
			mheadname[10],
			mfilename[10],
			aheadname[10],
			afilename[10];

extern s32int		_seg *grstarts;	// array of offsets in egagraph, -1 for sparse
extern s32int		_seg *audiostarts;	// array of offsets in audio / audiot
//
// hooks for custom cache dialogs
//
extern	void	(*drawcachebox)		(char *title, u16int numcache);
extern	void	(*updatecachebox)	(void);
extern	void	(*finishcachebox)	(void);

//===========================================================================

// just for the score box reshifting

void CAL_ShiftSprite (u16int segment,u16int source,u16int dest,
	u16int width, u16int height, u16int pixshift);

//===========================================================================

void CA_OpenDebug (void);
void CA_CloseDebug (void);
int CA_FarRead (s16int handle, u8int far *dest, s32int length);
int CA_FarWrite (s16int handle, u8int far *source, s32int length);
int CA_ReadFile (char *filename, uchar **ptr);
int CA_LoadFile (char *filename, uchar **ptr);
int CA_WriteFile (char *filename, void far *ptr, s32int length);

s32int CA_RLEWCompress (u16int huge *source, s32int length, u16int huge *dest,
  u16int rlewtag);

void CA_RLEWexpand (u16int huge *source, u16int huge *dest,s32int length,
  u16int rlewtag);

void CA_Startup (void);
void CA_Shutdown (void);

void CA_SetGrPurge (void);
void CA_CacheAudioChunk (s16int chunk);
void CA_LoadAllSounds (void);

void CA_UpLevel (void);
void CA_DownLevel (void);

void CA_SetAllPurge (void);

void CA_ClearMarks (void);
void CA_ClearAllMarks (void);

#define CA_MarkGrChunk(chunk)	grneeded[chunk]|=ca_levelbit

void CA_CacheGrChunk (s16int chunk);
void CA_CacheMap (s16int mapnum);

void CA_CacheMarks (void);

void CA_CacheScreen (s16int chunk);
