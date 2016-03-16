#define	MaxX	320
#define	MaxY	200

#define	MaxHelpLines	500

#define	MaxHighName	57
#define	MaxScores	7
typedef	struct
		{
			char	name[MaxHighName + 1];
			s32int	score;
			u16int	completed,episode;
		} HighScore;

#define	MaxGameName		32
#define	MaxSaveGames	6
typedef	struct
		{
			char	signature[4];
			u16int	*oldtest;
			int	present;
			char	name[MaxGameName + 1];
		} SaveGame;

#define	MaxString	128	// Maximum input string size

typedef	struct
		{
			s16int	x,y,
				w,h,
				px,py;
		} WindowRec;	// Record used to save & restore screen windows

typedef	enum
		{
			gd_Continue,
			gd_Easy,
			gd_Normal,
			gd_Hard
		} GameDiff;

//	Hack import for TED launch support
extern	int		tedlevel;
extern	s16int			tedlevelnum;
extern	void		TEDDeath(void);

extern	int		ingame,		// Set by game code if a game is in progress
					abortgame,	// Set if a game load failed
					loadedgame,	// Set if the current game was loaded
					NoWait,
					HighScoresDirty;
extern	char		*abortprogram;	// Set to error msg if program is dying
extern	GameDiff	restartgame;	// Normally gd_Continue, else starts game
extern	u16int		PrintX,PrintY;	// Current printing location in the window
extern	u16int		WindowX,WindowY,// Current location of window
					WindowW,WindowH;// Current size of window

extern	int		Button0,Button1,
					CursorBad;
extern	s16int			CursorX,CursorY;

extern	void		(*USL_MeasureString)(char far *,u16int *,u16int *),
					(*USL_DrawString)(char far *);

extern	int		(*USL_SaveGame)(s16int),(*USL_LoadGame)(s16int);
extern	void		(*USL_ResetGame)(void);
extern	SaveGame	Games[MaxSaveGames];
extern	HighScore	Scores[];

#define	US_HomeWindow()	{PrintX = WindowX; PrintY = WindowY;}

extern	void	US_Startup(void),
				US_Setup(void),
				US_Shutdown(void),
				US_InitRndT(int randomize),
				US_SetLoadSaveHooks(int (*load)(s16int),
									int (*save)(s16int),
									void (*reset)(void)),
				US_TextScreen(void),
				US_UpdateTextScreen(void),
				US_FinishTextScreen(void),
				US_DrawWindow(u16int x,u16int y,u16int w,u16int h),
				US_CenterWindow(u16int,u16int),
				US_SaveWindow(WindowRec *win),
				US_RestoreWindow(WindowRec *win),
				US_ClearWindow(void),
				US_SetPrintRoutines(void (*measure)(char far *,u16int *,u16int *),
									void (*print)(char far *)),
				US_PrintCentered(char far *s),
				US_CPrint(char far *s),
				US_CPrintLine(char far *s),
				US_Print(char far *s),
				US_PrintUnsigned(u32int n),
				US_PrintSigned(s32int n),
				US_StartCursor(void),
				US_ShutCursor(void),
				US_CheckHighScore(s32int score,u16int other),
				US_DisplayHighScores(s16int which);
extern	int	US_UpdateCursor(void),
				US_LineInput(s16int x,s16int y,char *buf,char *def,int escok,
								s16int maxchars,s16int maxwidth);
extern	s16int		US_RndT(void);

		void	USL_PrintInCenter(char far *s,Rct r);
		char 	*USL_GiveSaveName(u16int game);
