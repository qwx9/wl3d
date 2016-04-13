//
// WL_MENU.H
//
#ifdef SPEAR

#define BORDCOLOR	0x99
#define BORD2COLOR	0x93
#define DEACTIVE	0x9b
#define BKGDCOLOR	0x9d

#define MenuFadeOut()	VL_FadeOut(0,255,0,0,51,10)

#else

#define BORDCOLOR	0x29
#define BORD2COLOR	0x23
#define DEACTIVE	0x2b
#define BKGDCOLOR	0x2d

#define MenuFadeOut()	VL_FadeOut(0,255,43,0,0,10)

#endif

#define READCOLOR	0x4a
#define READHCOLOR	0x47
#define VIEWCOLOR	0x7f
#define TEXTCOLOR	0x17
#define HIGHLIGHT	0x13
#define MenuFadeIn()	VL_FadeIn(0,255,&gamepal,10)


#ifndef SPEAR
#define INTROSONG	7
#else
#define INTROSONG	23
#endif

#define SENSITIVE	60
#define CENTER		SENSITIVE*2

#define MENU_X	76
#define MENU_Y	55
#define MENU_W	178
#ifndef SPEAR
#define MENU_H	13*10+6
#else
#define MENU_H	13*9+6
#endif

#define SM_X	48
#define SM_W	250

#define SM_Y1	20
#define SM_H1	4*13-7
#define SM_Y2	SM_Y1+5*13
#define SM_H2	4*13-7
#define SM_Y3	SM_Y2+5*13
#define SM_H3	3*13-7

#define CTL_X	24
#define CTL_Y	70
#define CTL_W	284
#define CTL_H	13*7-7

#define LSM_X	85
#define LSM_Y	55
#define LSM_W	175
#define LSM_H	10*13+10

#define NM_X	50
#define NM_Y	100
#define NM_W	225
#define NM_H	13*4+15

#define NE_X	10
#define NE_Y	23
#define NE_W	320-NE_X*2
#define NE_H	200-NE_Y*2

#define CST_X		20
#define CST_Y		48
#define CST_START	60
#define CST_SPC	60


//
// TYPEDEFS
//
typedef struct {
		s16int x,y,amount,curpos,indent;
		} CP_iteminfo;

typedef struct {
		s16int active;
		char string[36];
		void (* routine)(s16int temp1);
		} CP_itemtype;

typedef struct {
		s16int allowed[4];
		} CustomCtrls;

extern CP_itemtype far MainMenu[],far NewEMenu[];
extern CP_iteminfo MainItems;

//
// FUNCTION PROTOTYPES
//
void SetupControlPanel(void);
void CleanupControlPanel(void);

void DrawMenu(CP_iteminfo *item_i,CP_itemtype far *items);
s16int  HandleMenu(CP_iteminfo *item_i,
		CP_itemtype far *items,
		void (*routine)(s16int w));
void ClearMScreen(void);
void DrawWindow(s16int x,s16int y,s16int w,s16int h,s16int wcolor);
void DrawOutline(s16int x,s16int y,s16int w,s16int h,s16int color1,s16int color2);
void WaitKeyUp(void);
void ReadAnyControl(ControlInfo *ci);
void TicDelay(s16int count);
void StartCPMusic(s16int song);
s16int  Confirm(char far *string);
void Message(char far *string);
void CheckPause(void);
void ShootSnd(void);
void CheckSecretMissions(void);
void BossKey(void);

void DrawGun(CP_iteminfo *item_i,CP_itemtype far *items,s16int x,s16int *y,s16int which,s16int basey,void (*routine)(s16int w));
void DrawHalfStep(s16int x,s16int y);
void EraseGun(CP_iteminfo *item_i,CP_itemtype far *items,s16int x,s16int y,s16int which);
void SetTextColor(CP_itemtype far *items,s16int hlight);
void DrawMenuGun(CP_iteminfo *iteminfo);
void DrawStripes(s16int y);

void DefineMouseBtns(void);
void DefineKeyBtns(void);
void DefineKeyMove(void);
void EnterCtrlData(s16int index,CustomCtrls *cust,void (*DrawRtn)(s16int),void (*PrintRtn)(s16int),s16int type);

void DrawMainMenu(void);
void DrawSoundMenu(void);
void DrawLoadSaveScreen(s16int loadsave);
void DrawNewEpisode(void);
void DrawNewGame(void);
void DrawChangeView(s16int view);
void DrawMouseSens(void);
void DrawCtlScreen(void);
void DrawCustomScreen(void);
void DrawLSAction(s16int which);
void DrawCustMouse(s16int hilight);
void DrawCustKeybd(s16int hilight);
void DrawCustKeys(s16int hilight);
void PrintCustMouse(s16int i);
void PrintCustKeybd(s16int i);
void PrintCustKeys(s16int i);

void PrintLSEntry(s16int w,s16int color);
void TrackWhichGame(s16int w);
void DrawNewGameDiff(s16int w);
void FixupCustom(s16int w);

void CP_NewGame(void);
void CP_Sound(void);
s16int  CP_LoadGame(s16int quick);
s16int  CP_SaveGame(s16int quick);
void CP_Control(void);
void CP_ChangeView(void);
void CP_ExitOptions(void);
void CP_Quit(void);
void CP_ViewScores(void);
s16int  CP_EndGame(void);
s16int  CP_CheckQuick(u16int scancode);
void CustomControls(void);
void MouseSensitivity(void);

//
// VARIABLES
//
extern s16int SaveGamesAvail[10],StartGame,SoundStatus;
extern char SaveGameNames[10][32],SaveName[13];

enum {MOUSE,JOYSTICK,KEYBOARDBTNS,KEYBOARDMOVE};	// FOR INPUT TYPES

enum
{
	newgame,
	soundmenu,
	control,
	loadgame,
	savegame,
	changeview,

#ifndef GOODTIMES
#ifndef SPEAR
	readthis,
#endif
#endif

	viewscores,
	backtodemo,
	quit
} menuitems;

//
// WL_INTER
//
typedef struct {
		s16int kill,secret,treasure;
		s32int time;
		} LRstruct;

extern LRstruct LevelRatios[];

void Write (s16int x,s16int y,char *string);
