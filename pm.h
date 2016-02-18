//
//	ID_PM.H
//	Header file for Id Engine's Page Manager
//

//	NOTE! PMPageSize must be an even divisor of EMSPageSize, and >= 1024
#define	EMSPageSize		16384
#define	EMSPageSizeSeg	(EMSPageSize >> 4)
#define	EMSPageSizeKB	(EMSPageSize >> 10)
#define	EMSFrameCount	4
#define	PMPageSize		4096
#define	PMPageSizeSeg	(PMPageSize >> 4)
#define	PMPageSizeKB	(PMPageSize >> 10)
#define	PMEMSSubPage	(EMSPageSize / PMPageSize)

#define	PMMinMainMem	10			// Min acceptable # of pages from main
#define	PMMaxMainMem	100			// Max number of pages in main memory

#define	PMThrashThreshold	1	// Number of page thrashes before panic mode
#define	PMUnThrashThreshold	5	// Number of non-thrashing frames before leaving panic mode

typedef	enum
		{
			pml_Unlocked,
			pml_Locked
		} PMLockType;

typedef	enum
		{
			pmba_Unused = 0,
			pmba_Used = 1,
			pmba_Allocated = 2
		} PMBlockAttr;

typedef	struct
		{
			u32int	offset;		// Offset of chunk into file
			u16int		length;		// Length of the chunk

			s16int			xmsPage;	// If in XMS, (xmsPage * PMPageSize) gives offset into XMS handle

			PMLockType	locked;		// If set, this page can't be purged
			s16int			emsPage;	// If in EMS, logical page/offset into page
			s16int			mainPage;	// If in Main, index into handle array

			u32int	lastHit;	// Last frame number of hit
		} PageListStruct;

typedef	struct
		{
			s16int			baseEMSPage;	// Base EMS page for this phys frame
			u32int	lastHit;		// Last frame number of hit
		} EMSListStruct;

extern	int			XMSPresent,EMSPresent;
extern	u16int			XMSPagesAvail,EMSPagesAvail;

extern	u16int			ChunksInFile,
						PMSpriteStart,PMSoundStart;
extern	PageListStruct	far *PMPages;

#define	PM_GetSoundPage(v)	PM_GetPage(PMSoundStart + (v))
#define	PM_GetSpritePage(v)	PM_GetPage(PMSpriteStart + (v))

#define	PM_LockMainMem()	PM_SetMainMemPurge(0)
#define	PM_UnlockMainMem()	PM_SetMainMemPurge(3)


extern	char	PageFileName[13];


extern	void	PM_Startup(void),
				PM_Shutdown(void),
				PM_Reset(void),
				PM_Preload(int (*update)(u16int current,u16int total)),
				PM_NextFrame(void),
				PM_SetPageLock(s16int pagenum,PMLockType lock),
				PM_SetMainPurge(s16int level),
				PM_CheckMainMem(void);
extern	uchar *PM_GetPageAddress(s16int pagenum),
				PM_GetPage(s16int pagenum);		// Use this one to cache page

void PM_SetMainMemPurge(s16int level);
