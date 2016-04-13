void CP_ReadThis(void);

#define STARTITEM	newgame

#define ENDGAMESTR	"Are you sure you want\nto end the game you\nare playing? (Y or N):"
#define GAMESVD	"There's already a game\nsaved at this position.\n      Overwrite?"
#define CURGAME	"You are currently in\na game. Continuing will\nerase old game. Ok?"

char far endStrings[9][80]=
{
#ifndef SPEAR
	{"Dost thou wish to\nleave with such hasty\nabandon?"},
	{"Chickening out...\nalready?"},
	{"Press N for more carnage.\nPress Y to be a weenie."},
	{"So, you think you can\nquit this easily, huh?"},
	{"Press N to save the world.\nPress Y to abandon it in\nits hour of need."},
	{"Press N if you are brave.\nPress Y to cower in shame."},
	{"Heroes, press N.\nWimps, press Y."},
	{"You are at an intersection.\nA sign says, 'Press Y to quit.'\n>"},
	{"For guns and glory, press N.\nFor work and worry, press Y."}
#else
	"Heroes don't quit, but\ngo ahead and press Y\nif you aren't one.",
	"Press Y to quit,\nor press N to enjoy\nmore violent diversion.",
	"Depressing the Y key means\nyou must return to the\nhumdrum workday world.",
	"Hey, quit or play,\nY or N:\nit's your choice.",
	"Sure you don't want to\nwaste a few more\nproductive hours?",
	"I think you had better\nplay some more. Please\npress N...please?",
	"If you are tough, press N.\nIf not, press Y daintily.",
	"I'm thinkin' that\nyou might wanna press N\nto play more. You do it.",
	"Sure. Fine. Quit.\nSee if we care.\nGet it over with.\nPress Y."

#endif
};

CP_iteminfo
	MainItems={MENU_X,MENU_Y,10,STARTITEM,24},
	SndItems={SM_X,SM_Y1,12,0,52},
	LSItems={LSM_X,LSM_Y,10,0,24},
	CtlItems={CTL_X,CTL_Y,6,-1,56},
	CusItems={8,CST_Y+13*2,9,-1,0},
	NewEitems={NE_X,NE_Y,11,0,88},
	NewItems={NM_X,NM_Y,4,2,24};

#pragma warn -sus
CP_itemtype far
MainMenu[]=
{
	{1,"New Game",CP_NewGame},
	{1,"Sound",CP_Sound},
	{1,"Control",CP_Control},
	{1,"Load Game",CP_LoadGame},
	{0,"Save Game",CP_SaveGame},
	{1,"Change View",CP_ChangeView},
#ifndef SPEAR
	{2,"Read This!",CP_ReadThis},
#endif
	{1,"View Scores",CP_ViewScores},
	{1,"Back to Demo",0},
	{1,"Quit",0}
},

far SndMenu[]=
{
	{1,"None",0},
	{1,"PC Speaker",0},
	{1,"AdLib/Sound Blaster",0},
	{0,"",0},
	{0,"",0},
	{1,"None",0},
	{1,"Disney Sound Source",0},
	{1,"Sound Blaster",0},
	{0,"",0},
	{0,"",0},
	{1,"None",0},
	{1,"AdLib/Sound Blaster",0}
},

far CtlMenu[]=
{
	{0,"Mouse Enabled",0},
	{0,"Joystick Enabled",0},
	{0,"Use joystick port 2",0},
	{0,"Gravis GamePad Enabled",0},
	{0,"Mouse Sensitivity",MouseSensitivity},
	{1,"Customize controls",CustomControls}
},

#pragma warn +sus

#ifndef SPEAR
far NewEmenu[]=
{
	{1,"Episode 1\n"
	   "Escape from Wolfenstein",0},
	{0,"",0},
	{3,"Episode 2\n"
		   "Operation: Eisenfaust",0},
	{0,"",0},
	{3,"Episode 3\n"
		   "Die, Fuhrer, Die!",0},
	{0,"",0},
	{3,"Episode 4\n"
		  "A Dark Secret",0},
	{0,"",0},
	{3,"Episode 5\n"
		  "Trail of the Madman",0},
	{0,"",0},
	{3,"Episode 6\n"
		  "Confrontation",0}
},
#endif


far NewMenu[]=
{
	{1,"Can I play, Daddy?",0},
	{1,"Don't hurt me.",0},
	{1,"Bring 'em on!",0},
	{1,"I am Death incarnate!",0}
},

far LSMenu[]=
{
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0}
},

far CusMenu[]=
{
	{1,"",0},
	{0,"",0},
	{0,"",0},
	{1,"",0},
	{0,"",0},
	{0,"",0},
	{1,"",0},
	{0,"",0},
	{1,"",0}
}
;


s16int color_hlite[]={
   DEACTIVE,
   HIGHLIGHT,
   READHCOLOR,
   0x67
   },

   color_norml[]={
   DEACTIVE,
   TEXTCOLOR,
   READCOLOR,
   0x6b
   };

s16int EpisodeSelect[6]={1};


s16int SaveGamesAvail[10],StartGame,SoundStatus=1,pickquick;
char SaveGameNames[10][32],SaveName[13]="SAVEGAM?.";


////////////////////////////////////////////////////////////////////
//
// INPUT MANAGER SCANCODE TABLES
//
////////////////////////////////////////////////////////////////////
static u8int
					*ScanNames[] =		// Scan code names with single chars
					{
	"?","?","1","2","3","4","5","6","7","8","9","0","-","+","?","?",
	"Q","W","E","R","T","Y","U","I","O","P","[","]","|","?","A","S",
	"D","F","G","H","J","K","L",";","\"","?","?","?","Z","X","C","V",
	"B","N","M",",",".","/","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","\xf","?","-","\x15","5","\x11","+","?",
	"\x13","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?"
					},	// DEBUG - consolidate these
					far ExtScanCodes[] =	// Scan codes with >1 char names
					{
	1,0xe,0xf,0x1d,0x2a,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,
	0x3f,0x40,0x41,0x42,0x43,0x44,0x57,0x59,0x46,0x1c,0x36,
	0x37,0x38,0x47,0x49,0x4f,0x51,0x52,0x53,0x45,0x48,
	0x50,0x4b,0x4d,0x00
					},
					*ExtScanNames[] =	// Names corresponding to ExtScanCodes
					{
	"Esc","BkSp","Tab","Ctrl","LShft","Space","CapsLk","F1","F2","F3","F4",
	"F5","F6","F7","F8","F9","F10","F11","F12","ScrlLk","Enter","RShft",
	"PrtSc","Alt","Home","PgUp","End","PgDn","Ins","Del","NumLk","Up",
	"Down","Left","Right",""
					};


////////////////////////////////////////////////////////////////////
//
// Wolfenstein Control Panel!  Ta Da!
//
////////////////////////////////////////////////////////////////////
void US_ControlPanel(u8int scancode)
{
	s16int which,i,start;


	if (ingame)
		if (CP_CheckQuick(scancode))
			return;

	StartCPMusic(MENUSONG);
	SetupControlPanel();

	//
	// F-KEYS FROM WITHIN GAME
	//
	switch(scancode)
	{
		case sc_F1:
			#ifndef SPEAR
			HelpScreens();
			#endif
			goto finishup;

		case sc_F2:
			CP_SaveGame(0);
			goto finishup;

		case sc_F3:
			CP_LoadGame(0);
			goto finishup;

		case sc_F4:
			CP_Sound();
			goto finishup;

		case sc_F5:
			CP_ChangeView();
			goto finishup;

		case sc_F6:
			CP_Control();
			goto finishup;

		finishup:
			CleanupControlPanel();
			return;
	}

	DrawMainMenu();
	MenuFadeIn();
	StartGame=0;

	//
	// MAIN MENU LOOP
	//
	do
	{
		which=HandleMenu(&MainItems,&MainMenu[0],NULL);

		#ifdef SPEAR
		#ifndef SPEARDEMO
		//
		// EASTER EGG FOR SPEAR OF DESTINY!
		//
		if (Keyboard[sc_I] && Keyboard[sc_D])
		{
			VW_FadeOut();
			StartCPMusic (18);
			MM_SortMem ();
			ClearMemory ();

			VWB_DrawPic(0,0,Pid1);
			VWB_DrawPic(0,80,Pid2);

			VW_UpdateScreen();

			VL_FadeIn(0,255,Eid,30);	/* sod only */

			while (Keyboard[sc_I] || Keyboard[sc_D]);
			IN_ClearKeysDown();
			IN_Ack();

			VW_FadeOut();

			DrawMainMenu();
			StartCPMusic (MENUSONG);
			MenuFadeIn();
		}
		#endif
		#endif

		switch(which)
		{
			case viewscores:
				if (MainMenu[viewscores].routine == NULL)
					if (CP_EndGame())
						StartGame=1;

				DrawMainMenu();
				MenuFadeIn();
				break;

			case backtodemo:
				MM_SortMem();
				StartGame=1;
				if (!ingame)
					StartCPMusic(INTROSONG);
				VL_FadeOut(0,255,0,0,0,10);
				break;

			case -1:
			case quit:
				CP_Quit();
				break;

			default:
				if (!StartGame)
				{
					DrawMainMenu();
					MenuFadeIn();
				}
		}

	//
	// "EXIT OPTIONS" OR "NEW GAME" EXITS
	//
	} while(!StartGame);

	//
	// DEALLOCATE EVERYTHING
	//
	CleanupControlPanel();

	//
	// CHANGE MAINMENU ITEM
	//
	if (startgame || loadedgame)
	{
		#pragma warn -sus
		MainMenu[viewscores].routine = NULL;
		_fstrcpy(MainMenu[viewscores].string,"End Game");
		#pragma warn +sus
	}
}


////////////////////////
//
// DRAW MAIN MENU SCREEN
//
void DrawMainMenu(void)
{
	ClearMScreen();

	VWB_DrawPic(112,184,Pmouselback);
	DrawStripes(10);
	VWB_DrawPic(84,0,Popt);

	DrawWindow(MENU_X-8,MENU_Y-3,MENU_W,MENU_H,BKGDCOLOR);

	//
	// CHANGE "GAME" AND "DEMO"
	//
	if (ingame)
	{
		_fstrcpy(&MainMenu[backtodemo].string[8],"Game");
		MainMenu[backtodemo].active=2;
	}
	else
	{
		_fstrcpy(&MainMenu[backtodemo].string[8],"Demo");
		MainMenu[backtodemo].active=1;
	}

	DrawMenu(&MainItems,&MainMenu[0]);
	VW_UpdateScreen();
}

////////////////////////////////////////////////////////////////////
//
// READ THIS!
//
////////////////////////////////////////////////////////////////////
void CP_ReadThis(void)
{
	StartCPMusic(0);
	HelpScreens();
	StartCPMusic(MENUSONG);
}

////////////////////////////////////////////////////////////////////
//
// CHECK QUICK-KEYS & QUIT (WHILE IN A GAME)
//
////////////////////////////////////////////////////////////////////
s16int CP_CheckQuick(u16int scancode)
{
	switch(scancode)
	{
		//
		// END GAME
		//
		case sc_F7:
			CA_CacheGrChunk(STARTFONT+1);

			WindowH=160;
			if (Confirm(ENDGAMESTR))
			{
				playstate = ex_died;
				pickquick = gamestate.lives = 0;
			}

			DrawAllPlayBorder();
			WindowH=200;
			fontnumber=0;
			MainMenu[savegame].active = 0;
			return 1;

		//
		// QUICKSAVE
		//
		case sc_F8:
			if (SaveGamesAvail[LSItems.curpos] && pickquick)
			{
				CA_CacheGrChunk(STARTFONT+1);
				fontnumber = 1;
				Message("Saving...");
				CP_SaveGame(1);
				fontnumber=0;
			}
			else
			{
				VW_FadeOut ();

				StartCPMusic(MENUSONG);
				pickquick=CP_SaveGame(0);

				SETFONTCOLOR(0,15);
				IN_ClearKeysDown();
				DrawPlayScreen ();

				if (!startgame && !loadedgame)
				{
					VW_FadeIn ();
					StartMusic ();
				}

				if (loadedgame)
					playstate = ex_abort;
				lasttimecount = TimeCount;

				if (MousePresent)
					Mouse(MDelta);	// Clear accumulated mouse movement

				PM_CheckMainMem ();
			}
			return 1;

		//
		// QUICKLOAD
		//
		case sc_F9:
			if (SaveGamesAvail[LSItems.curpos] && pickquick)
			{
				char string[100]="Load Game called\n\"";


				CA_CacheGrChunk(STARTFONT+1);
				fontnumber = 1;

				strcat(string,SaveGameNames[LSItems.curpos]);
				strcat(string,"\"?");

				if (Confirm(string))
					CP_LoadGame(1);

				DrawAllPlayBorder();
				fontnumber=0;
			}
			else
			{
				VW_FadeOut ();

				StartCPMusic(MENUSONG);
				pickquick=CP_LoadGame(0);

				SETFONTCOLOR(0,15);
				IN_ClearKeysDown();
				DrawPlayScreen ();

				if (!startgame && !loadedgame)
				{
					VW_FadeIn ();
					StartMusic ();
				}

				if (loadedgame)
					playstate = ex_abort;

				lasttimecount = TimeCount;

				if (MousePresent)
					Mouse(MDelta);	// Clear accumulated mouse movement
				PM_CheckMainMem ();
			}
			return 1;

		//
		// QUIT
		//
		case sc_F10:
			CA_CacheGrChunk(STARTFONT+1);

			WindowX=WindowY=0;
			WindowW=320;
			WindowH=160;
			if (Confirm(endStrings[US_RndT()&0x7+(US_RndT()&1)]))
			{
				s16int i;


				VW_UpdateScreen();
				SD_MusicOff();
				SD_StopSound();
				MenuFadeOut();

				//
				// SHUT-UP THE ADLIB
				//
				for (i=1;i<=0xf5;i++)
					alOut(i,0);
				Quit(NULL);
			}

			DrawAllPlayBorder();
			WindowH=200;
			fontnumber=0;
			return 1;
		}

	return 0;
}


////////////////////////////////////////////////////////////////////
//
// END THE CURRENT GAME
//
////////////////////////////////////////////////////////////////////
s16int CP_EndGame(void)
{
	if (!Confirm(ENDGAMESTR))
		return 0;

	pickquick = gamestate.lives = 0;
	playstate = ex_died;

	#pragma warn -sus
	MainMenu[savegame].active = 0;
	MainMenu[viewscores].routine=CP_ViewScores;
	_fstrcpy(MainMenu[viewscores].string,"View Scores");
	#pragma warn +sus

	return 1;
}


////////////////////////////////////////////////////////////////////
//
// VIEW THE HIGH SCORES
//
////////////////////////////////////////////////////////////////////
void CP_ViewScores(void)
{
	fontnumber=0;

#ifdef SPEAR
	StartCPMusic (20);
#else
	StartCPMusic (23);
#endif

	DrawHighScores ();
	VW_UpdateScreen ();
	MenuFadeIn();
	fontnumber=1;

	IN_Ack();

	StartCPMusic(MENUSONG);
	MenuFadeOut();
}


////////////////////////////////////////////////////////////////////
//
// START A NEW GAME
//
////////////////////////////////////////////////////////////////////
void CP_NewGame(void)
{
	s16int which,episode;

#ifndef SPEAR
firstpart:

	DrawNewEpisode();
	do
	{
		which=HandleMenu(&NewEitems,&NewEmenu[0],NULL);
		switch(which)
		{
			case -1:
				MenuFadeOut();
				return;

			default:
				if (!EpisodeSelect[which/2])
				{
					SD_PlaySound (Snoway);
					Message("Please select \"Read This!\"\n"
							"from the Options menu to\n"
							"find out how to order this\n"
							"episode from Apogee.");
					IN_ClearKeysDown();
					IN_Ack();
					DrawNewEpisode();
					which = 0;
				}
				else
				{
					episode = which/2;
					which = 1;
				}
				break;
		}

	} while (!which);

	ShootSnd();

	//
	// ALREADY IN A GAME?
	//
	if (ingame)
		if (!Confirm(CURGAME))
		{
			MenuFadeOut();
			return;
		}

	MenuFadeOut();

#else
	episode = 0;

	//
	// ALREADY IN A GAME?
	//
	DrawNewGame();
	if (ingame)
		if (!Confirm(CURGAME))
		{
			MenuFadeOut();
			return;
		}

#endif

	DrawNewGame();
	which=HandleMenu(&NewItems,&NewMenu[0],DrawNewGameDiff);
	if (which<0)
	{
		MenuFadeOut();
		#ifndef SPEAR
		goto firstpart;
		#else
		return;
		#endif
	}

	ShootSnd();
	NewGame(which,episode);
	StartGame=1;
	MenuFadeOut();

	//
	// CHANGE "READ THIS!" TO NORMAL COLOR
	//
	#ifndef SPEAR
	MainMenu[readthis].active=1;
	#endif

	pickquick = 0;
}


#ifndef SPEAR
/////////////////////
//
// DRAW NEW EPISODE MENU
//
void DrawNewEpisode(void)
{
	s16int i;

	ClearMScreen();
	VWB_DrawPic(112,184,Pmouselback);

	DrawWindow(NE_X-4,NE_Y-4,NE_W+8,NE_H+8,BKGDCOLOR);
	SETFONTCOLOR(READHCOLOR,BKGDCOLOR);
	PrintY=2;
	WindowX=0;
	US_CPrint("Which episode to play?");

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	DrawMenu(&NewEitems,&NewEmenu[0]);

	for (i=0;i<6;i++)
		VWB_DrawPic(NE_X+32,NE_Y+i*26,Pep1+i);

	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}
#endif

/////////////////////
//
// DRAW NEW GAME MENU
//
void DrawNewGame(void)
{
	ClearMScreen();
	VWB_DrawPic(112,184,Pmouselback);

	SETFONTCOLOR(READHCOLOR,BKGDCOLOR);
	PrintX=NM_X+20;
	PrintY=NM_Y-32;

#ifndef SPEAR
	US_Print("How tough are you?");
#else
	VWB_DrawPic (PrintX,PrintY,Pdiffc);
#endif

	DrawWindow(NM_X-5,NM_Y-10,NM_W,NM_H,BKGDCOLOR);

	DrawMenu(&NewItems,&NewMenu[0]);
	DrawNewGameDiff(NewItems.curpos);
	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}


////////////////////////
//
// DRAW NEW GAME GRAPHIC
//
void DrawNewGameDiff(s16int w)
{
	VWB_DrawPic(NM_X+185,NM_Y+7,w+Pbaby);
}


////////////////////////////////////////////////////////////////////
//
// HANDLE SOUND MENU
//
////////////////////////////////////////////////////////////////////
void CP_Sound(void)
{
	s16int which,i;

	DrawSoundMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which=HandleMenu(&SndItems,&SndMenu[0],NULL);
		//
		// HANDLE MENU CHOICES
		//
		switch(which)
		{
			//
			// SOUND EFFECTS
			//
			case 0:
				if (SoundMode!=sdm_Off)
				{
					SD_WaitSoundDone();
					SD_SetSoundMode(sdm_Off);
					DrawSoundMenu();
				}
				break;
			case 1:
				if (SoundMode!=sdm_PC)
				{
					SD_WaitSoundDone();
					SD_SetSoundMode(sdm_PC);
					CA_LoadAllSounds();
					DrawSoundMenu();
					ShootSnd();
				}
				break;
			case 2:
				if (SoundMode!=sdm_AdLib)
				{
					SD_WaitSoundDone();
					SD_SetSoundMode(sdm_AdLib);
					CA_LoadAllSounds();
					DrawSoundMenu();
					ShootSnd();
				}
				break;

			//
			// DIGITIZED SOUND
			//
			case 5:
				if (DigiMode!=sds_Off)
				{
					SD_SetDigiDevice(sds_Off);
					DrawSoundMenu();
				}
				break;
			case 6:
				if (DigiMode!=sds_SoundSource)
				{
					SD_SetDigiDevice(sds_SoundSource);
					DrawSoundMenu();
					ShootSnd();
				}
				break;
			case 7:
				if (DigiMode!=sds_SoundBlaster)
				{
					SD_SetDigiDevice(sds_SoundBlaster);
					DrawSoundMenu();
					ShootSnd();
				}
				break;

			//
			// MUSIC
			//
			case 10:
				if (MusicMode!=smm_Off)
				{
					SD_SetMusicMode(smm_Off);
					DrawSoundMenu();
					ShootSnd();
				}
				break;
			case 11:
				if (MusicMode!=smm_AdLib)
				{
					SD_SetMusicMode(smm_AdLib);
					DrawSoundMenu();
					ShootSnd();
					StartCPMusic(MENUSONG);
				}
				break;
		}
	} while(which>=0);

	MenuFadeOut();
}


//////////////////////
//
// DRAW THE SOUND MENU
//
void DrawSoundMenu(void)
{
	s16int i,on;

	//
	// DRAW SOUND MENU
	//
	ClearMScreen();
	VWB_DrawPic(112,184,Pmouselback);

	DrawWindow(SM_X-8,SM_Y1-3,SM_W,SM_H1,BKGDCOLOR);
	DrawWindow(SM_X-8,SM_Y2-3,SM_W,SM_H2,BKGDCOLOR);
	DrawWindow(SM_X-8,SM_Y3-3,SM_W,SM_H3,BKGDCOLOR);

	//
	// IF NO ADLIB, NON-CHOOSENESS!
	//
	if (!AdLibPresent && !SoundBlasterPresent)
	{
		SndMenu[2].active=SndMenu[10].active=SndMenu[11].active=0;
	}

	if (!SoundSourcePresent)
		SndMenu[6].active=0;

	if (!SoundBlasterPresent)
		SndMenu[7].active=0;

	if (!SoundSourcePresent && !SoundBlasterPresent)
		SndMenu[5].active=0;

	DrawMenu(&SndItems,&SndMenu[0]);
	VWB_DrawPic(100,SM_Y1-20,Psfx);
	VWB_DrawPic(100,SM_Y2-20,Ppcm);
	VWB_DrawPic(100,SM_Y3-20,Pmus);

	for (i=0;i<SndItems.amount;i++)
		if (SndMenu[i].string[0])
		{
			//
			// DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
			//
			on=0;
			switch(i)
			{
				//
				// SOUND EFFECTS
				//
				case 0: if (SoundMode==sdm_Off) on=1; break;
				case 1: if (SoundMode==sdm_PC) on=1; break;
				case 2: if (SoundMode==sdm_AdLib) on=1; break;

				//
				// DIGITIZED SOUND
				//
				case 5: if (DigiMode==sds_Off) on=1; break;
				case 6: if (DigiMode==sds_SoundSource) on=1; break;
				case 7: if (DigiMode==sds_SoundBlaster) on=1; break;

				//
				// MUSIC
				//
				case 10: if (MusicMode==smm_Off) on=1; break;
				case 11: if (MusicMode==smm_AdLib) on=1; break;
			}

			if (on)
				VWB_DrawPic(SM_X+24,SM_Y1+i*13+2,Psel);
			else
				VWB_DrawPic(SM_X+24,SM_Y1+i*13+2,Punsel);
		}

	DrawMenuGun(&SndItems);
	VW_UpdateScreen();
}


//
// DRAW LOAD/SAVE IN PROGRESS
//
void DrawLSAction(s16int which)
{
	#define LSA_X	96
	#define LSA_Y	80
	#define LSA_W	130
	#define LSA_H	42

	DrawWindow(LSA_X,LSA_Y,LSA_W,LSA_H,TEXTCOLOR);
	DrawOutline(LSA_X,LSA_Y,LSA_W,LSA_H,0,HIGHLIGHT);
	VWB_DrawPic(LSA_X+8,LSA_Y+5,Pread1);

	fontnumber=1;
	SETFONTCOLOR(0,TEXTCOLOR);
	PrintX=LSA_X+46;
	PrintY=LSA_Y+13;

	if (!which)
		US_Print("Loading...");
	else
		US_Print("Saving...");

	VW_UpdateScreen();
}


////////////////////////////////////////////////////////////////////
//
// LOAD SAVED GAMES
//
////////////////////////////////////////////////////////////////////
s16int CP_LoadGame(s16int quick)
{
	s16int handle,which,exit=0;
	char name[13];


	strcpy(name,SaveName);

	//
	// QUICKLOAD?
	//
	if (quick)
	{
		which=LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			name[7]=which+'0';
			handle=open(name,O_BINARY);
			lseek(handle,32,SEEK_SET);
			loadedgame=true;
			LoadTheGame(handle,0,0);
			loadedgame=false;
			close(handle);

			DrawFace ();
			DrawHealth ();
			DrawLives ();
			DrawLevel ();
			DrawAmmo ();
			DrawKeys ();
			DrawWeapon ();
			DrawScore ();
			return 1;
		}
	}

	DrawLoadSaveScreen(0);

	do
	{
		which=HandleMenu(&LSItems,&LSMenu[0],TrackWhichGame);
		if (which>=0 && SaveGamesAvail[which])
		{
			ShootSnd();
			name[7]=which+'0';

			handle=open(name,O_BINARY);
			lseek(handle,32,SEEK_SET);

			DrawLSAction(0);
			loadedgame=true;

			LoadTheGame(handle,LSA_X+8,LSA_Y+5);
			close(handle);

			StartGame=1;
			ShootSnd();
			//
			// CHANGE "READ THIS!" TO NORMAL COLOR
			//
			#ifndef SPEAR
			MainMenu[readthis].active=1;
			#endif

			exit=1;
			break;
		}

	} while(which>=0);

	MenuFadeOut();

	return exit;
}


///////////////////////////////////
//
// HIGHLIGHT CURRENT SELECTED ENTRY
//
void TrackWhichGame(s16int w)
{
	static s16int lastgameon=0;

	PrintLSEntry(lastgameon,TEXTCOLOR);
	PrintLSEntry(w,HIGHLIGHT);

	lastgameon=w;
}


////////////////////////////
//
// DRAW THE LOAD/SAVE SCREEN
//
void DrawLoadSaveScreen(s16int loadsave)
{
	#define DISKX	100
	#define DISKY	0

	s16int i;


	ClearMScreen();
	fontnumber=1;
	VWB_DrawPic(112,184,Pmouselback);
	DrawWindow(LSM_X-10,LSM_Y-5,LSM_W,LSM_H,BKGDCOLOR);
	DrawStripes(10);

	if (!loadsave)
		VWB_DrawPic(60,0,Pload);
	else
		VWB_DrawPic(60,0,Psave);

	for (i=0;i<10;i++)
		PrintLSEntry(i,TEXTCOLOR);

	DrawMenu(&LSItems,&LSMenu[0]);
	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}


///////////////////////////////////////////
//
// PRINT LOAD/SAVE GAME ENTRY W/BOX OUTLINE
//
void PrintLSEntry(s16int w,s16int color)
{
	SETFONTCOLOR(color,BKGDCOLOR);
	DrawOutline(LSM_X+LSItems.indent,LSM_Y+w*13,LSM_W-LSItems.indent-15,11,color,color);
	PrintX=LSM_X+LSItems.indent+2;
	PrintY=LSM_Y+w*13+1;
	fontnumber=0;

	if (SaveGamesAvail[w])
		US_Print(SaveGameNames[w]);
	else
		US_Print("      - empty -");

	fontnumber=1;
}


////////////////////////////////////////////////////////////////////
//
// SAVE CURRENT GAME
//
////////////////////////////////////////////////////////////////////
s16int CP_SaveGame(s16int quick)
{
	s16int handle,which,exit=0;
	u16int nwritten;
	char name[13],input[32];


	strcpy(name,SaveName);

	//
	// QUICKSAVE?
	//
	if (quick)
	{
		which=LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			name[7]=which+'0';
			unlink(name);
			handle=creat(name,S_IREAD|S_IWRITE);

			strcpy(input,&SaveGameNames[which][0]);

			_dos_write(handle,(void far *)input,32,&nwritten);
			lseek(handle,32,SEEK_SET);
			SaveTheGame(handle,0,0);
			close(handle);

			return 1;
		}
	}

	DrawLoadSaveScreen(1);

	do
	{
		which=HandleMenu(&LSItems,&LSMenu[0],TrackWhichGame);
		if (which>=0)
		{
			//
			// OVERWRITE EXISTING SAVEGAME?
			//
			if (SaveGamesAvail[which])
				if (!Confirm(GAMESVD))
				{
					DrawLoadSaveScreen(1);
					continue;
				}
				else
				{
					DrawLoadSaveScreen(1);
					PrintLSEntry(which,HIGHLIGHT);
					VW_UpdateScreen();
				}

			ShootSnd();

			strcpy(input,&SaveGameNames[which][0]);
			name[7]=which+'0';

			fontnumber=0;
			if (!SaveGamesAvail[which])
				VWB_Bar(LSM_X+LSItems.indent+1,LSM_Y+which*13+1,LSM_W-LSItems.indent-16,10,BKGDCOLOR);
			VW_UpdateScreen();

			if (US_LineInput(LSM_X+LSItems.indent+2,LSM_Y+which*13+1,input,input,true,31,LSM_W-LSItems.indent-30))
			{
				SaveGamesAvail[which]=1;
				strcpy(&SaveGameNames[which][0],input);

				unlink(name);
				handle=creat(name,S_IREAD|S_IWRITE);
				_dos_write(handle,(void far *)input,32,&nwritten);
				lseek(handle,32,SEEK_SET);

				DrawLSAction(1);
				SaveTheGame(handle,LSA_X+8,LSA_Y+5);

				close(handle);

				ShootSnd();
				exit=1;
			}
			else
			{
				VWB_Bar(LSM_X+LSItems.indent+1,LSM_Y+which*13+1,LSM_W-LSItems.indent-16,10,BKGDCOLOR);
				PrintLSEntry(which,HIGHLIGHT);
				VW_UpdateScreen();
				SD_PlaySound(Sesc);
				continue;
			}

			fontnumber=1;
			break;
		}

	} while(which>=0);

	MenuFadeOut();

	return exit;
}

////////////////////////////////////////////////////////////////////
//
// DEFINE CONTROLS
//
////////////////////////////////////////////////////////////////////
void CP_Control(void)
{
	#define CTL_SPC	70
	enum {MOUSEENABLE,JOYENABLE,USEPORT2,PADENABLE,MOUSESENS,CUSTOMIZE};
	s16int i,which;

	DrawCtlScreen();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which=HandleMenu(&CtlItems,&CtlMenu[0],NULL);
		switch(which)
		{
			case MOUSEENABLE:
				mouseenabled^=1;
				_CX=_DX=CENTER;
				Mouse(4);
				DrawCtlScreen();
				CusItems.curpos=-1;
				ShootSnd();
				break;

			case JOYENABLE:
				joystickenabled^=1;
				DrawCtlScreen();
				CusItems.curpos=-1;
				ShootSnd();
				break;

			case USEPORT2:
				joystickport^=1;
				DrawCtlScreen();
				ShootSnd();
				break;

			case PADENABLE:
				joypadenabled^=1;
				DrawCtlScreen();
				ShootSnd();
				break;

			case MOUSESENS:
			case CUSTOMIZE:
				DrawCtlScreen();
				MenuFadeIn();
				WaitKeyUp();
				break;
		}
	} while(which>=0);

	MenuFadeOut();
}


////////////////////////////////
//
// DRAW MOUSE SENSITIVITY SCREEN
//
void DrawMouseSens(void)
{
	ClearMScreen();
	VWB_DrawPic(112,184,Pmouselback);
	DrawWindow(10,80,300,30,BKGDCOLOR);

	WindowX=0;
	WindowW=320;
	PrintY=82;
	SETFONTCOLOR(READCOLOR,BKGDCOLOR);
	US_CPrint("Adjust Mouse Sensitivity");

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	PrintX=14;
	PrintY=95;
	US_Print("Slow");
	PrintX=269;
	US_Print("Fast");

	VWB_Bar(60,97,200,10,TEXTCOLOR);
	DrawOutline(60,97,200,10,0,HIGHLIGHT);
	DrawOutline(60+20*mouseadjustment,97,20,10,0,READCOLOR);
	VWB_Bar(61+20*mouseadjustment,98,19,9,READHCOLOR);

	VW_UpdateScreen();
	MenuFadeIn();
}


///////////////////////////
//
// ADJUST MOUSE SENSITIVITY
//
void MouseSensitivity(void)
{
	ControlInfo ci;
	s16int exit=0,oldMA;


	oldMA=mouseadjustment;
	DrawMouseSens();
	do
	{
		ReadAnyControl(&ci);
		switch(ci.dir)
		{
			case dir_North:
			case dir_West:
				if (mouseadjustment)
				{
					mouseadjustment--;
					VWB_Bar(60,97,200,10,TEXTCOLOR);
					DrawOutline(60,97,200,10,0,HIGHLIGHT);
					DrawOutline(60+20*mouseadjustment,97,20,10,0,READCOLOR);
					VWB_Bar(61+20*mouseadjustment,98,19,9,READHCOLOR);
					VW_UpdateScreen();
					SD_PlaySound(Sdrawgun1);
					while(Keyboard[sc_LeftArrow]);
					WaitKeyUp();
				}
				break;

			case dir_South:
			case dir_East:
				if (mouseadjustment<9)
				{
					mouseadjustment++;
					VWB_Bar(60,97,200,10,TEXTCOLOR);
					DrawOutline(60,97,200,10,0,HIGHLIGHT);
					DrawOutline(60+20*mouseadjustment,97,20,10,0,READCOLOR);
					VWB_Bar(61+20*mouseadjustment,98,19,9,READHCOLOR);
					VW_UpdateScreen();
					SD_PlaySound(Sdrawgun1);
					while(Keyboard[sc_RightArrow]);
					WaitKeyUp();
				}
				break;
		}

		if (ci.button0 || Keyboard[sc_Space] || Keyboard[sc_Enter])
			exit=1;
		else
		if (ci.button1 || Keyboard[sc_Escape])
			exit=2;

	} while(!exit);

	if (exit==2)
	{
		mouseadjustment=oldMA;
		SD_PlaySound(Sesc);
	}
	else
		SD_PlaySound(Sshoot);

	WaitKeyUp();
	MenuFadeOut();
}


///////////////////////////
//
// DRAW CONTROL MENU SCREEN
//
void DrawCtlScreen(void)
{
 s16int i,x,y;

 ClearMScreen();
 DrawStripes(10);
 VWB_DrawPic(80,0,Pctl);
 VWB_DrawPic(112,184,Pmouselback);
 DrawWindow(CTL_X-8,CTL_Y-5,CTL_W,CTL_H,BKGDCOLOR);
 WindowX=0;
 WindowW=320;
 SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);

/*
 if (JoysPresent[0])
   CtlMenu[1].active=
   CtlMenu[2].active=
   CtlMenu[3].active=1;
*/

 CtlMenu[2].active=CtlMenu[3].active=joystickenabled;

 if (MousePresent)
 {
  CtlMenu[4].active=
  CtlMenu[0].active=1;
 }

 CtlMenu[4].active=mouseenabled;


 DrawMenu(&CtlItems,&CtlMenu[0]);


 x=CTL_X+CtlItems.indent-24;
 y=CTL_Y+3;
 if (mouseenabled)
   VWB_DrawPic(x,y,Psel);
 else
   VWB_DrawPic(x,y,Punsel);

 y=CTL_Y+16;
 if (joystickenabled)
   VWB_DrawPic(x,y,Psel);
 else
   VWB_DrawPic(x,y,Punsel);

 y=CTL_Y+29;
 if (joystickport)
   VWB_DrawPic(x,y,Psel);
 else
   VWB_DrawPic(x,y,Punsel);

 y=CTL_Y+42;
 if (joypadenabled)
   VWB_DrawPic(x,y,Psel);
 else
   VWB_DrawPic(x,y,Punsel);

 //
 // PICK FIRST AVAILABLE SPOT
 //
 if (CtlItems.curpos<0 || !CtlMenu[CtlItems.curpos].active)
   for (i=0;i<6;i++)
	 if (CtlMenu[i].active)
	 {
	  CtlItems.curpos=i;
	  break;
	 }

 DrawMenuGun(&CtlItems);
 VW_UpdateScreen();
}


////////////////////////////////////////////////////////////////////
//
// CUSTOMIZE CONTROLS
//
////////////////////////////////////////////////////////////////////
enum {FIRE,STRAFE,RUN,OPEN};
char mbarray[4][3]={"b0","b1","b2","b3"},
	   order[4]={RUN,OPEN,FIRE,STRAFE};


void CustomControls(void)
{
 s16int which;


 DrawCustomScreen();
 do
 {
  which=HandleMenu(&CusItems,&CusMenu[0],FixupCustom);
  switch(which)
  {
   case 0:
	 DefineMouseBtns();
	 DrawCustMouse(1);
	 break;
   case 3:
	 // joystick
	 break;
   case 6:
	 DefineKeyBtns();
	 DrawCustKeybd(0);
	 break;
   case 8:
	 DefineKeyMove();
	 DrawCustKeys(0);
  }
 } while(which>=0);



 MenuFadeOut();
}


////////////////////////
//
// DEFINE THE MOUSE BUTTONS
//
void DefineMouseBtns(void)
{
 CustomCtrls mouseallowed={0,1,1,1};
 EnterCtrlData(2,&mouseallowed,DrawCustMouse,PrintCustMouse,MOUSE);
}


////////////////////////
//
// DEFINE THE KEYBOARD BUTTONS
//
void DefineKeyBtns(void)
{
 CustomCtrls keyallowed={1,1,1,1};
 EnterCtrlData(8,&keyallowed,DrawCustKeybd,PrintCustKeybd,KEYBOARDBTNS);
}


////////////////////////
//
// DEFINE THE KEYBOARD BUTTONS
//
void DefineKeyMove(void)
{
	CustomCtrls keyallowed={1,1,1,1};
	EnterCtrlData(10,&keyallowed,DrawCustKeys,PrintCustKeys,KEYBOARDMOVE);
}


////////////////////////
//
// ENTER CONTROL DATA FOR ANY TYPE OF CONTROL
//
enum {FWRD,RIGHT,BKWD,LEFT};
s16int moveorder[4]={LEFT,RIGHT,FWRD,BKWD};

void EnterCtrlData(s16int index,CustomCtrls *cust,void (*DrawRtn)(s16int),void (*PrintRtn)(s16int),s16int type)
{
 s16int j,exit,tick,redraw,which,x,picked;
 ControlInfo ci;


 ShootSnd();
 PrintY=CST_Y+13*index;
 IN_ClearKeysDown();
 exit=0;
 redraw=1;
 //
 // FIND FIRST SPOT IN ALLOWED ARRAY
 //
 for (j=0;j<4;j++)
   if (cust->allowed[j])
   {
	which=j;
	break;
   }

 do
 {
  if (redraw)
  {
   x=CST_START+CST_SPC*which;
   DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);

   DrawRtn(1);
   DrawWindow(x-2,PrintY,CST_SPC,11,TEXTCOLOR);
   DrawOutline(x-2,PrintY,CST_SPC,11,0,HIGHLIGHT);
   SETFONTCOLOR(0,TEXTCOLOR);
   PrintRtn(which);
   PrintX=x;
   SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
   VW_UpdateScreen();
   WaitKeyUp();
   redraw=0;
  }

  ReadAnyControl(&ci);

  if (type==MOUSE || type==JOYSTICK)
	if (IN_KeyDown(sc_Enter)||IN_KeyDown(sc_Control)||IN_KeyDown(sc_Alt))
	{
	 IN_ClearKeysDown();
	 ci.button0=ci.button1=false;
	}

  //
  // CHANGE BUTTON VALUE?
  //
  if ((ci.button0|ci.button1|ci.button2|ci.button3)||
	  ((type==KEYBOARDBTNS||type==KEYBOARDMOVE) && LastScan==sc_Enter))
  {
   tick=TimeCount=picked=0;
   SETFONTCOLOR(0,TEXTCOLOR);

   do
   {
	s16int button,result=0;


	if (type==KEYBOARDBTNS||type==KEYBOARDMOVE)
	  IN_ClearKeysDown();

	//
	// FLASH CURSOR
	//
	if (TimeCount>10)
	{
	 switch(tick)
	 {
	  case 0:
	VWB_Bar(x,PrintY+1,CST_SPC-2,10,TEXTCOLOR);
	break;
	  case 1:
	PrintX=x;
	US_Print("?");
	SD_PlaySound(Shitwall);
	 }
	 tick^=1;
	 TimeCount=0;
	 VW_UpdateScreen();
	}

	//
	// WHICH TYPE OF INPUT DO WE PROCESS?
	//
	switch(type)
	{
	 case MOUSE:
	   Mouse(3);
	   button=_BX;
	   switch(button)
	   {
	case 1: result=1; break;
	case 2: result=2; break;
	case 4: result=3; break;
	   }

	   if (result)
	   {
	s16int z;


	for (z=0;z<4;z++)
	  if (order[which]==buttonmouse[z])
	  {
	   buttonmouse[z]=bt_nobutton;
	   break;
	  }

	buttonmouse[result-1]=order[which];
	picked=1;
	SD_PlaySound(Shitdoor);
	   }
	   break;

	 case JOYSTICK:
	   if (ci.button0) result=1;
	   else
	   if (ci.button1) result=2;
	   else
	   if (ci.button2) result=3;
	   else
	   if (ci.button3) result=4;

	   if (result)
	   {
	s16int z;


	for (z=0;z<4;z++)
	  if (order[which]==buttonjoy[z])
	  {
	   buttonjoy[z]=bt_nobutton;
	   break;
	  }

	buttonjoy[result-1]=order[which];
	picked=1;
	SD_PlaySound(Shitdoor);
	   }
	   break;

	 case KEYBOARDBTNS:
	   if (LastScan)
	   {
	buttonscan[order[which]]=LastScan;
	picked=1;
	ShootSnd();
	IN_ClearKeysDown();
	   }
	   break;

	 case KEYBOARDMOVE:
	   if (LastScan)
	   {
	dirscan[moveorder[which]]=LastScan;
	picked=1;
	ShootSnd();
	IN_ClearKeysDown();
	   }
	   break;
	}

	//
	// EXIT INPUT?
	//
	if (IN_KeyDown(sc_Escape))
	{
	 picked=1;
	 continue;
	}

   } while(!picked);

   SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
   redraw=1;
   WaitKeyUp();
   continue;
  }

  if (ci.button1 || IN_KeyDown(sc_Escape))
	exit=1;

  //
  // MOVE TO ANOTHER SPOT?
  //
  switch(ci.dir)
  {
   case dir_West:
	 do
	 {
	  which--;
	  if (which<0)
	which=3;
	 } while(!cust->allowed[which]);
	 redraw=1;
	 SD_PlaySound(Sdrawgun1);
	 while(ReadAnyControl(&ci),ci.dir!=dir_None);
	 IN_ClearKeysDown();
	 break;

   case dir_East:
	 do
	 {
	  which++;
	  if (which>3)
	which=0;
	 } while(!cust->allowed[which]);
	 redraw=1;
	 SD_PlaySound(Sdrawgun1);
	 while(ReadAnyControl(&ci),ci.dir!=dir_None);
	 IN_ClearKeysDown();
	 break;
   case dir_North:
   case dir_South:
	 exit=1;
  }
 } while(!exit);

 SD_PlaySound(Sesc);
 WaitKeyUp();
 DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);
}


////////////////////////
//
// FIXUP GUN CURSOR OVERDRAW SHIT
//
void FixupCustom(s16int w)
{
	static s16int lastwhich=-1;
	s16int y=CST_Y+26+w*13;


	VWB_Hlin(7,32,y-1,DEACTIVE);
	VWB_Hlin(7,32,y+12,BORD2COLOR);
#ifndef SPEAR
	VWB_Hlin(7,32,y-2,BORDCOLOR);
	VWB_Hlin(7,32,y+13,BORDCOLOR);
#else
	VWB_Hlin(7,32,y-2,BORD2COLOR);
	VWB_Hlin(7,32,y+13,BORD2COLOR);
#endif

	switch(w)
	{
		case 0: DrawCustMouse(1); break;
		case 3: /* joystick */ break;
		case 6: DrawCustKeybd(1); break;
		case 8: DrawCustKeys(1);
	}


	if (lastwhich>=0)
	{
		y=CST_Y+26+lastwhich*13;
		VWB_Hlin(7,32,y-1,DEACTIVE);
		VWB_Hlin(7,32,y+12,BORD2COLOR);
#ifndef SPEAR
		VWB_Hlin(7,32,y-2,BORDCOLOR);
		VWB_Hlin(7,32,y+13,BORDCOLOR);
#else
		VWB_Hlin(7,32,y-2,BORD2COLOR);
		VWB_Hlin(7,32,y+13,BORD2COLOR);
#endif

		if (lastwhich!=w)
			switch(lastwhich)
			{
				case 0: DrawCustMouse(0); break;
				case 3: /* joystick */ break;
				case 6: DrawCustKeybd(0); break;
				case 8: DrawCustKeys(0);
			}
	}

	lastwhich=w;
}


////////////////////////
//
// DRAW CUSTOMIZE SCREEN
//
void DrawCustomScreen(void)
{
	s16int i;

	ClearMScreen();
	WindowX=0;
	WindowW=320;
	VWB_DrawPic(112,184,Pmouselback);
	DrawStripes(10);
	VWB_DrawPic(80,0,Pcustom);

	//
	// MOUSE
	//
	SETFONTCOLOR(READCOLOR,BKGDCOLOR);
	WindowX=0;
	WindowW=320;

#ifndef SPEAR
	PrintY=CST_Y;
	US_CPrint("Mouse\n");
#else
	PrintY = CST_Y+13;
	VWB_DrawPic (128,48,Pmouse);
#endif

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	PrintX=CST_START;
	US_Print("Run");
	PrintX=CST_START+CST_SPC*1;
	US_Print("Open");
	PrintX=CST_START+CST_SPC*2;
	US_Print("Fire");
	PrintX=CST_START+CST_SPC*3;
	US_Print("Strafe\n");

	DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);
	DrawCustMouse(0);
	US_Print("\n");


	//
	// JOYSTICK/PAD
	//
#ifndef SPEAR
	SETFONTCOLOR(READCOLOR,BKGDCOLOR);
	US_CPrint("Joystick/Gravis GamePad\n");
#else
	PrintY += 13;
	VWB_DrawPic (40,88,Pjs);
#endif

#ifdef SPEAR
	VWB_DrawPic (112,120,Pkb);
#endif

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	PrintX=CST_START;
	US_Print("Run");
	PrintX=CST_START+CST_SPC*1;
	US_Print("Open");
	PrintX=CST_START+CST_SPC*2;
	US_Print("Fire");
	PrintX=CST_START+CST_SPC*3;
	US_Print("Strafe\n");
	DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);
	//DrawCustJoy(0);
	US_Print("\n");


	//
	// KEYBOARD
	//
#ifndef SPEAR
	SETFONTCOLOR(READCOLOR,BKGDCOLOR);
	US_CPrint("Keyboard\n");
#else
	PrintY += 13;
#endif
	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	PrintX=CST_START;
	US_Print("Run");
	PrintX=CST_START+CST_SPC*1;
	US_Print("Open");
	PrintX=CST_START+CST_SPC*2;
	US_Print("Fire");
	PrintX=CST_START+CST_SPC*3;
	US_Print("Strafe\n");
	DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);
	DrawCustKeybd(0);
	US_Print("\n");


	//
	// KEYBOARD MOVE KEYS
	//
	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	PrintX=CST_START;
	US_Print("Left");
	PrintX=CST_START+CST_SPC*1;
	US_Print("Right");
	PrintX=CST_START+CST_SPC*2;
	US_Print("Frwd");
	PrintX=CST_START+CST_SPC*3;
	US_Print("Bkwrd\n");
	DrawWindow(5,PrintY-1,310,13,BKGDCOLOR);
	DrawCustKeys(0);
	//
	// PICK STARTING POINT IN MENU
	//
	if (CusItems.curpos<0)
		for (i=0;i<CusItems.amount;i++)
			if (CusMenu[i].active)
			{
				CusItems.curpos=i;
				break;
			}


	VW_UpdateScreen();
	MenuFadeIn();
}


void PrintCustMouse(s16int i)
{
	s16int j;

	for (j=0;j<4;j++)
		if (order[i]==buttonmouse[j])
		{
			PrintX=CST_START+CST_SPC*i;
			US_Print(mbarray[j]);
			break;
		}
}

void DrawCustMouse(s16int hilight)
{
	s16int i,color;


	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	if (!mouseenabled)
	{
		SETFONTCOLOR(DEACTIVE,BKGDCOLOR);
		CusMenu[0].active=0;
	}
	else
		CusMenu[0].active=1;

	PrintY=CST_Y+13*2;
	for (i=0;i<4;i++)
		PrintCustMouse(i);
}

void PrintCustKeybd(s16int i)
{
	PrintX=CST_START+CST_SPC*i;
	US_Print(IN_GetScanName(buttonscan[order[i]]));
}

void DrawCustKeybd(s16int hilight)
{
	s16int i,color;


	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	PrintY=CST_Y+13*8;
	for (i=0;i<4;i++)
		PrintCustKeybd(i);
}

void PrintCustKeys(s16int i)
{
	PrintX=CST_START+CST_SPC*i;
	US_Print(IN_GetScanName(dirscan[moveorder[i]]));
}

void DrawCustKeys(s16int hilight)
{
	s16int i,color;


	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	PrintY=CST_Y+13*10;
	for (i=0;i<4;i++)
		PrintCustKeys(i);
}


////////////////////////////////////////////////////////////////////
//
// CHANGE SCREEN VIEWING SIZE
//
////////////////////////////////////////////////////////////////////
void CP_ChangeView(void)
{
	s16int exit=0,oldview,newview;
	ControlInfo ci;


	WindowX=WindowY=0;
	WindowW=320;
	WindowH=200;
	newview=oldview=viewwidth/16;
	DrawChangeView(oldview);

	do
	{
		CheckPause();
		ReadAnyControl(&ci);
		switch(ci.dir)
		{
		case dir_South:
		case dir_West:
			newview--;
			if (newview<4)
				newview=4;
			ShowViewSize(newview);
			VW_UpdateScreen();
			SD_PlaySound(Shitwall);
			TicDelay(10);
			break;

		case dir_North:
		case dir_East:
			newview++;
			if (newview>19)
				newview=19;
			ShowViewSize(newview);
			VW_UpdateScreen();
			SD_PlaySound(Shitwall);
			TicDelay(10);
			break;
		}

		if (ci.button0 || Keyboard[sc_Enter])
			exit=1;
		else
		if (ci.button1 || Keyboard[sc_Escape])
		{
			viewwidth=oldview*16;
			SD_PlaySound(Sesc);
			MenuFadeOut();
			return;
		}

	} while(!exit);


	if (oldview!=newview)
	{
		SD_PlaySound (Sshoot);
		Message("Thinking...");
		NewViewSize(newview);
	}

	ShootSnd();
	MenuFadeOut();
}


/////////////////////////////
//
// DRAW THE CHANGEVIEW SCREEN
//
void DrawChangeView(s16int view)
{
	VWB_Bar(0,160,320,40,VIEWCOLOR);
	ShowViewSize(view);

	PrintY=161;
	WindowX=0;
	WindowY=320;
	SETFONTCOLOR(HIGHLIGHT,BKGDCOLOR);

	US_CPrint("Use arrows to size\n");
	US_CPrint("ENTER to accept\n");
	US_CPrint("ESC to cancel");
	VW_UpdateScreen();

	MenuFadeIn();
}


////////////////////////////////////////////////////////////////////
//
// QUIT THIS INFERNAL GAME!
//
////////////////////////////////////////////////////////////////////
void CP_Quit(void)
{
	s16int i;

	if (Confirm(endStrings[US_RndT()&0x7+(US_RndT()&1)]))
	{
		VW_UpdateScreen();
		SD_MusicOff();
		SD_StopSound();
		MenuFadeOut();
		//
		// SHUT-UP THE ADLIB
		//
		for (i=1;i<=0xf5;i++)
			alOut(i,0);
		Quit(NULL);
	}

	DrawMainMenu();
}

////////////////////////////////////////////////////////////////////
//
// Draw a window for a menu
//
////////////////////////////////////////////////////////////////////
void DrawWindow(s16int x,s16int y,s16int w,s16int h,s16int wcolor)
{
	VWB_Bar(x,y,w,h,wcolor);
	DrawOutline(x,y,w,h,BORD2COLOR,DEACTIVE);
}


void DrawOutline(s16int x,s16int y,s16int w,s16int h,s16int color1,s16int color2)
{
	VWB_Hlin(x,x+w,y,color2);
	VWB_Vlin(y,y+h,x,color2);
	VWB_Hlin(x,x+w,y+h,color1);
	VWB_Vlin(y,y+h,x+w,color1);
}


////////////////////////////////////////////////////////////////////
//
// Setup Control Panel stuff - graphics, etc.
//
////////////////////////////////////////////////////////////////////
void SetupControlPanel(void)
{
	struct ffblk f;
	char name[13];
	s16int which,i;


	//
	// CACHE GRAPHICS & SOUNDS
	//

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	fontnumber=1;
	WindowH=200;

	if (!ingame)
		CA_LoadAllSounds();
	else
		MainMenu[savegame].active=1;

	//
	// SEE WHICH SAVE GAME FILES ARE AVAILABLE & READ STRING IN
	//
	strcpy(name,SaveName);
	if (!findfirst(name,&f,0))
		do
		{
			which=f.ff_name[7]-'0';
			if (which<10)
			{
				s16int handle;
				char temp[32];

				SaveGamesAvail[which]=1;
				handle=open(f.ff_name,O_BINARY);
				read(handle,temp,32);
				close(handle);
				strcpy(&SaveGameNames[which][0],temp);
			}
		} while(!findnext(&f));

	//
	// CENTER MOUSE
	//
	_CX=_DX=CENTER;
	Mouse(4);
}


////////////////////////////////////////////////////////////////////
//
// Clean up all the Control Panel stuff
//
////////////////////////////////////////////////////////////////////
void CleanupControlPanel(void)
{
	fontnumber = 0;
}


////////////////////////////////////////////////////////////////////
//
// Handle moving gun around a menu
//
////////////////////////////////////////////////////////////////////
s16int HandleMenu(CP_iteminfo *item_i,CP_itemtype far *items,void (*routine)(s16int w))
{
	char key;
	static s16int redrawitem=1,lastitem=-1;
	s16int i,x,y,basey,exit,which,shape,timer;
	ControlInfo ci;


	which=item_i->curpos;
	x=item_i->x&-8;
	basey=item_i->y-2;
	y=basey+which*13;

	VWB_DrawPic(x,y,Pcur1);
	SetTextColor(items+which,1);
	if (redrawitem)
	{
		PrintX=item_i->x+item_i->indent;
		PrintY=item_i->y+which*13;
		US_Print((items+which)->string);
	}
	//
	// CALL CUSTOM ROUTINE IF IT IS NEEDED
	//
	if (routine)
		routine(which);
	VW_UpdateScreen();

	shape=Pcur1;
	timer=8;
	exit=0;
	TimeCount=0;
	IN_ClearKeysDown();


	do
	{
		//
		// CHANGE GUN SHAPE
		//
		if (TimeCount>timer)
		{
			TimeCount=0;
			if (shape==Pcur1)
			{
				shape=Pcur2;
				timer=8;
			}
			else
			{
				shape=Pcur1;
				timer=70;
			}
			VWB_DrawPic(x,y,shape);
			if (routine)
				routine(which);
			VW_UpdateScreen();
		}

		CheckPause();

		//
		// SEE IF ANY KEYS ARE PRESSED FOR INITIAL CHAR FINDING
		//
		key=LastASCII;
		if (key)
		{
			s16int ok=0;

			if (key>='a')
				key-='a'-'A';

			for (i=which+1;i<item_i->amount;i++)
				if ((items+i)->active && (items+i)->string[0]==key)
				{
					EraseGun(item_i,items,x,y,which);
					which=i;
					DrawGun(item_i,items,x,&y,which,basey,routine);
					ok=1;
					IN_ClearKeysDown();
					break;
				}

			//
			// DIDN'T FIND A MATCH FIRST TIME THRU. CHECK AGAIN.
			//
			if (!ok)
			{
				for (i=0;i<which;i++)
					if ((items+i)->active && (items+i)->string[0]==key)
					{
						EraseGun(item_i,items,x,y,which);
						which=i;
						DrawGun(item_i,items,x,&y,which,basey,routine);
						IN_ClearKeysDown();
						break;
					}
			}
		}

		//
		// GET INPUT
		//
		ReadAnyControl(&ci);
		switch(ci.dir)
		{
			////////////////////////////////////////////////
			//
			// MOVE UP
			//
			case dir_North:

			EraseGun(item_i,items,x,y,which);

			//
			// ANIMATE HALF-STEP
			//
			if (which && (items+which-1)->active)
			{
				y-=6;
				DrawHalfStep(x,y);
			}

			//
			// MOVE TO NEXT AVAILABLE SPOT
			//
			do
			{
				if (!which)
					which=item_i->amount-1;
				else
					which--;
			} while(!(items+which)->active);

			DrawGun(item_i,items,x,&y,which,basey,routine);
			//
			// WAIT FOR BUTTON-UP OR DELAY NEXT MOVE
			//
			TicDelay(20);
			break;

			////////////////////////////////////////////////
			//
			// MOVE DOWN
			//
			case dir_South:

			EraseGun(item_i,items,x,y,which);
			//
			// ANIMATE HALF-STEP
			//
			if (which!=item_i->amount-1 && (items+which+1)->active)
			{
				y+=6;
				DrawHalfStep(x,y);
			}

			do
			{
				if (which==item_i->amount-1)
					which=0;
				else
					which++;
			} while(!(items+which)->active);

			DrawGun(item_i,items,x,&y,which,basey,routine);

			//
			// WAIT FOR BUTTON-UP OR DELAY NEXT MOVE
			//
			TicDelay(20);
			break;
		}

		if (ci.button0 ||
			Keyboard[sc_Space] ||
			Keyboard[sc_Enter])
				exit=1;

		if (ci.button1 ||
			Keyboard[sc_Escape])
				exit=2;

	} while(!exit);


	IN_ClearKeysDown();

	//
	// ERASE EVERYTHING
	//
	if (lastitem!=which)
	{
		VWB_Bar(x-1,y,25,16,BKGDCOLOR);
		PrintX=item_i->x+item_i->indent;
		PrintY=item_i->y+which*13;
		US_Print((items+which)->string);
		redrawitem=1;
	}
	else
		redrawitem=0;

	if (routine)
		routine(which);
	VW_UpdateScreen();

	item_i->curpos=which;

	lastitem=which;
	switch(exit)
	{
		case 1:
			//
			// CALL THE ROUTINE
			//
			if ((items+which)->routine!=NULL)
			{
				ShootSnd();
				MenuFadeOut();
				(items+which)->routine(0);
			}
			return which;

		case 2:
			SD_PlaySound(Sesc);
			return -1;
	}

	return 0; // JUST TO SHUT UP THE ERROR MESSAGES!
}


//
// ERASE GUN & DE-HIGHLIGHT STRING
//
void EraseGun(CP_iteminfo *item_i,CP_itemtype far *items,s16int x,s16int y,s16int which)
{
	VWB_Bar(x-1,y,25,16,BKGDCOLOR);
	SetTextColor(items+which,0);

	PrintX=item_i->x+item_i->indent;
	PrintY=item_i->y+which*13;
	US_Print((items+which)->string);
	VW_UpdateScreen();
}


//
// DRAW HALF STEP OF GUN TO NEXT POSITION
//
void DrawHalfStep(s16int x,s16int y)
{
	VWB_DrawPic(x,y,Pcur1);
	VW_UpdateScreen();
	SD_PlaySound(Sdrawgun1);
	TimeCount=0;
	while(TimeCount<8);
}


//
// DRAW GUN AT NEW POSITION
//
void DrawGun(CP_iteminfo *item_i,CP_itemtype far *items,s16int x,s16int *y,s16int which,s16int basey,void (*routine)(s16int w))
{
	VWB_Bar(x-1,*y,25,16,BKGDCOLOR);
	*y=basey+which*13;
	VWB_DrawPic(x,*y,Pcur1);
	SetTextColor(items+which,1);

	PrintX=item_i->x+item_i->indent;
	PrintY=item_i->y+which*13;
	US_Print((items+which)->string);

	//
	// CALL CUSTOM ROUTINE IF IT IS NEEDED
	//
	if (routine)
		routine(which);
	VW_UpdateScreen();
	SD_PlaySound(Sdrawgun2);
}

////////////////////////////////////////////////////////////////////
//
// DELAY FOR AN AMOUNT OF TICS OR UNTIL CONTROLS ARE INACTIVE
//
////////////////////////////////////////////////////////////////////
void TicDelay(s16int count)
{
	ControlInfo ci;


	TimeCount=0;
	do
	{
		ReadAnyControl(&ci);
	} while(TimeCount<count && ci.dir!=dir_None);
}


////////////////////////////////////////////////////////////////////
//
// Draw a menu
//
////////////////////////////////////////////////////////////////////
void DrawMenu(CP_iteminfo *item_i,CP_itemtype far *items)
{
	s16int i,which=item_i->curpos;


	WindowX=PrintX=item_i->x+item_i->indent;
	WindowY=PrintY=item_i->y;
	WindowW=320;
	WindowH=200;

	for (i=0;i<item_i->amount;i++)
	{
		SetTextColor(items+i,which==i);

		PrintY=item_i->y+i*13;
		if ((items+i)->active)
			US_Print((items+i)->string);
		else
		{
			SETFONTCOLOR(DEACTIVE,BKGDCOLOR);
			US_Print((items+i)->string);
			SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
		}

		US_Print("\n");
	}
}

void SetTextColor(CP_itemtype far *items,s16int hlight)
{
	if (hlight)
		{SETFONTCOLOR(color_hlite[items->active],BKGDCOLOR);}
	else
		{SETFONTCOLOR(color_norml[items->active],BKGDCOLOR);}
}


////////////////////////////////////////////////////////////////////
//
// WAIT FOR CTRLKEY-UP OR BUTTON-UP
//
////////////////////////////////////////////////////////////////////
void WaitKeyUp(void)
{
	ControlInfo ci;
	while(ReadAnyControl(&ci),	ci.button0|
								ci.button1|
								ci.button2|
								ci.button3|
								Keyboard[sc_Space]|
								Keyboard[sc_Enter]|
								Keyboard[sc_Escape]);
}


////////////////////////////////////////////////////////////////////
//
// READ KEYBOARD, JOYSTICK AND MOUSE FOR INPUT
//
////////////////////////////////////////////////////////////////////
void ReadAnyControl(ControlInfo *ci)
{
	s16int mouseactive=0;


	IN_ReadControl(0,ci);

	if (mouseenabled)
	{
		s16int mousey,mousex;


		// READ MOUSE MOTION COUNTERS
		// RETURN DIRECTION
		// HOME MOUSE
		// CHECK MOUSE BUTTONS

		Mouse(3);
		mousex=_CX;
		mousey=_DX;

		if (mousey<CENTER-SENSITIVE)
		{
			ci->dir=dir_North;
			_CX=_DX=CENTER;
			Mouse(4);
			mouseactive=1;
		}
		else
		if (mousey>CENTER+SENSITIVE)
		{
			ci->dir=dir_South;
			_CX=_DX=CENTER;
			Mouse(4);
			mouseactive=1;
		}

		if (mousex<CENTER-SENSITIVE)
		{
			ci->dir=dir_West;
			_CX=_DX=CENTER;
			Mouse(4);
			mouseactive=1;
		}
		else
		if (mousex>CENTER+SENSITIVE)
		{
			ci->dir=dir_East;
			_CX=_DX=CENTER;
			Mouse(4);
			mouseactive=1;
		}

		if (IN_MouseButtons())
		{
			ci->button0=IN_MouseButtons()&1;
			ci->button1=IN_MouseButtons()&2;
			ci->button2=IN_MouseButtons()&4;
			ci->button3=false;
			mouseactive=1;
		}
	}
}


////////////////////////////////////////////////////////////////////
//
// DRAW DIALOG AND CONFIRM YES OR NO TO QUESTION
//
////////////////////////////////////////////////////////////////////
s16int Confirm(char far *string)
{
	s16int xit=0,i,x,y,tick=0,time,whichsnd[2]={Sesc,Sshoot};


	Message(string);
	IN_ClearKeysDown();

	//
	// BLINK CURSOR
	//
	x=PrintX;
	y=PrintY;
	TimeCount=0;

	do
	{
		if (TimeCount>=10)
		{
			switch(tick)
			{
				case 0:
					VWB_Bar(x,y,8,13,TEXTCOLOR);
					break;
				case 1:
					PrintX=x;
					PrintY=y;
					US_Print("_");
			}
			VW_UpdateScreen();
			tick^=1;
			TimeCount=0;
		}
	} while(!Keyboard[sc_Y] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);

	if (Keyboard[sc_Y])
	{
		xit=1;
		ShootSnd();
	}

	while(Keyboard[sc_Y] || Keyboard[sc_N] || Keyboard[sc_Escape]);

	IN_ClearKeysDown();
	SD_PlaySound(whichsnd[xit]);
	return xit;
}

////////////////////////////////////////////////////////////////////
//
// PRINT A MESSAGE IN A WINDOW
//
////////////////////////////////////////////////////////////////////
void Message(char far *string)
{
	s16int h=0,w=0,mw=0,i,x,y,time;
	fontstruct _seg *font;

	fontnumber=1;
	h=font->height;
	for (i=0;i<_fstrlen(string);i++)
		if (string[i]=='\n')
		{
			if (w>mw)
				mw=w;
			w=0;
			h+=font->height;
		}
		else
			w+=font->width[string[i]];

	if (w+10>mw)
		mw=w+10;

	PrintY=(WindowH/2)-h/2;
	PrintX=WindowX=160-mw/2;

	DrawWindow(WindowX-5,PrintY-5,mw+10,h+10,TEXTCOLOR);
	DrawOutline(WindowX-5,PrintY-5,mw+10,h+10,0,HIGHLIGHT);
	SETFONTCOLOR(0,TEXTCOLOR);
	US_Print(string);
	VW_UpdateScreen();
}

void StartCPMusic(s16int song)
{
	SD_MusicOff();
	SD_StartMusic((MusicGroup far *)audiosegs[STARTMUSIC + song]);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetScanName() - Returns a string containing the name of the
//		specified scan code
//
///////////////////////////////////////////////////////////////////////////
u8int *
IN_GetScanName(u8int scan)
{
	u8int		**p;
	u8int	far *s;

	for (s = ExtScanCodes,p = ExtScanNames;*s;p++,s++)
		if (*s == scan)
			return(*p);

	return(ScanNames[scan]);
}


///////////////////////////////////////////////////////////////////////////
//
// CHECK FOR PAUSE KEY (FOR MUSIC ONLY)
//
///////////////////////////////////////////////////////////////////////////
void CheckPause(void)
{
	if (Paused)
	{
		switch(SoundStatus)
		{
			case 0: SD_MusicOn(); break;
			case 1: SD_MusicOff(); break;
		}

		SoundStatus^=1;
		VW_WaitVBL(3);
		IN_ClearKeysDown();
		Paused=false;
 }
}


///////////////////////////////////////////////////////////////////////////
//
// DRAW GUN CURSOR AT CORRECT POSITION IN MENU
//
///////////////////////////////////////////////////////////////////////////
void DrawMenuGun(CP_iteminfo *iteminfo)
{
	s16int x,y;


	x=iteminfo->x;
	y=iteminfo->y+iteminfo->curpos*13-2;
	VWB_DrawPic(x,y,Pcur1);
}


///////////////////////////////////////////////////////////////////////////
//
// DRAW SCREEN TITLE STRIPES
//
///////////////////////////////////////////////////////////////////////////

void ShootSnd(void)
{
	SD_PlaySound(Sshoot);
}
