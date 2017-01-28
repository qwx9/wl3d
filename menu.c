s16int SaveGamesAvail[10],StartGame,SoundStatus=1,pickquick;
char SaveGameNames[10][32],SaveName[13]="SAVEGAM?.";

void US_ControlPanel(u8int scancode)
{
	if (ingame)
		if (CP_CheckQuick(scancode))
			return;
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
			return;
	}
}

void CP_ReadThis(void)
{
	StartCPMusic(0);
	HelpScreens();
	StartCPMusic(MENUSONG);
}

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
				gm.φ = ex_died;
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
				view ();

				if (!startgame && !gm.load)
				{
					VW_FadeIn ();
					mapmus ();
				}

				if (gm.load)
					gm.φ = ex_abort;
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
				view ();

				if (!startgame && !gm.load)
				{
					VW_FadeIn ();
					mapmus ();
				}

				if (gm.load)
					gm.φ = ex_abort;

				lasttimecount = TimeCount;

				if (MousePresent)
					Mouse(MDelta);	// Clear accumulated mouse movement
				PM_CheckMainMem ();
			}
			return 1;
		}

	return 0;
}

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
					sfx (Snoway);
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
			gm.load=true;
			LoadTheGame(handle,0,0);
			gm.load=false;
			close(handle);

			hudf ();
			hudh ();
			hudl ();
			hudm ();
			huda ();
			hudk ();
			hudw ();
			hudp ();
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
			gm.load=true;

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

void TrackWhichGame(s16int w)
{
	static s16int lastgameon=0;

	PrintLSEntry(lastgameon,TEXTCOLOR);
	PrintLSEntry(w,HIGHLIGHT);

	lastgameon=w;
}

void DrawLoadSaveScreen(s16int loadsave)
{
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
				if (!Confirm("There's already a game\nsaved at this position.\n      Overwrite?"))
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
				sfx(Sesc);
				continue;
			}

			fontnumber=1;
			break;
		}

	} while(which>=0);

	MenuFadeOut();

	return exit;
}

void SetupControlPanel(void)
{
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
}

void StartCPMusic(s16int song)
{
	SD_MusicOff();
	SD_mapmus((MusicGroup far *)audiosegs[STARTMUSIC + song]);
}

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
