exit_t		gm.φ;
s16int			DebugOk;
objtype 	objlist[Nobj],*new,*obj,*player,*lastobj,
			*objfreelist,*killer;
u16int	farmapylookup[MAPSIZE];
int		onestep,godmode,noclip;
s16int			extravbls;
u8int		tilemap[MAPSIZE][MAPSIZE];	// wall values only
u8int		spotvis[MAPSIZE][MAPSIZE];
objtype		*actorat[MAPSIZE][MAPSIZE];

//
// replacing refresh manager
//
u16int	mapwidth,mapheight,tics;
int		compatability;
u8int		*updateptr;
u16int	mapwidthtable[64];
u16int	uwidthtable[UPDATEHIGH];
u16int	blockstarts[UPDATEWIDE*UPDATEHIGH];
u8int		update[UPDATESIZE];

//
// control info
//
int		mouseenabled,joystickenabled,joypadenabled,joystickprogressive;
s16int			joystickport;
s16int			dirscan[4] = {sc_UpArrow,sc_RightArrow,sc_DownArrow,sc_LeftArrow};
s16int			buttonscan[NUMBUTTONS] =
			{sc_Control,sc_Alt,sc_RShift,sc_Space,sc_1,sc_2,sc_3,sc_4};
s16int			buttonmouse[4]={bt_attack,bt_strafe,bt_use,bt_nobutton};
s16int			buttonjoy[4]={bt_attack,bt_strafe,bt_use,bt_run};

int		buttonheld[NUMBUTTONS];

char		far *demoptr, far *lastdemoptr;
uchar *demobuffer;

int		buttonstate[NUMBUTTONS];

objtype dummyobj;

#define BASETURN		35
#define RUNTURN			70
#define JOYSCALE		2

void CheckKeys (void)
{
	s16int		i;
	u8int	scan;
	u16int	temp;


	if (screenfaded || gm.demo)	// don't do anything with a faded screen
		return;

	scan = LastScan;


	#ifdef SPEAR
	//
	// SECRET CHEAT CODE: TAB-G-F10
	//
	if (Keyboard[sc_Tab] &&
		Keyboard[sc_G] &&
		Keyboard[sc_F10])
	{
		WindowH = 160;
		if (godmode)
		{
			Message ("God mode OFF");
			sfx (Snobonus);
		}
		else
		{
			Message ("God mode ON");
			sfx (Sendb2);
		}

		IN_Ack();
		godmode ^= 1;
		DrawAllPlayBorderSides ();
		IN_ClearKeysDown();
		return;
	}
	#endif


	//
	// SECRET CHEAT CODE: 'MLI'
	//
	if (Keyboard[sc_M] &&
		Keyboard[sc_L] &&
		Keyboard[sc_I])
	{
		gamestate.health = 100;
		gamestate.ammo = 99;
		gamestate.keys = 3;
		gm.pt = 0;
		gm.lvltc += 42000L;
		givew (WPgatling);

		hudw();
		hudh();
		hudk();
		huda();
		hudp();

		ClearMemory ();
		ClearSplitVWB ();
		VW_ScreenToScreen (displayofs,bufferofs,80,160);

		Message("You now have 100% Health,\n"
			"99 Ammo and both Keys!\n\n"
			"Note that you have basically\n"
			"eliminated your chances of\n"
			"getting a high score!");
		PM_CheckMainMem ();
		IN_ClearKeysDown();
		IN_Ack();

		DrawAllPlayBorder ();
	}

	//
	// OPEN UP DEBUG KEYS
	//
	if (Keyboard[sc_BackSpace] &&
		Keyboard[sc_LShift] &&
		Keyboard[sc_Alt] &&
		debug)
	{
	 ClearMemory ();
	 ClearSplitVWB ();
	 VW_ScreenToScreen (displayofs,bufferofs,80,160);

	 Message("Debugging keys are\nnow available!");
	 PM_CheckMainMem ();
	 IN_ClearKeysDown();
	 IN_Ack();

	 DrawAllPlayBorderSides ();
	 DebugOk=1;
	}

	//
	// TRYING THE KEEN CHEAT CODE!
	//
	if (Keyboard[sc_B] &&
		Keyboard[sc_A] &&
		Keyboard[sc_T])
	{
	 ClearMemory ();
	 ClearSplitVWB ();
	 VW_ScreenToScreen (displayofs,bufferofs,80,160);

	 Message("Commander Keen is also\n"
			 "available from Apogee, but\n"
			 "then, you already know\n"
			 "that - right, Cheatmeister?!");

	 PM_CheckMainMem ();
	 IN_ClearKeysDown();
	 IN_Ack();

	 DrawAllPlayBorder ();
	}

//
// pause key weirdness can't be checked as a scan code
//
	if (Paused)
	{
		bufferofs = displayofs;
		LatchDrawPic (20-4,80-2*8,Ppause);
		SD_MusicOff();
		IN_Ack();
		IN_ClearKeysDown ();
		SD_MusicOn();
		Paused = false;
		if (MousePresent)
			Mouse(MDelta);	// Clear accumulated mouse movement
		return;
	}


//
// F1-F7/ESC to enter control panel
//
	if (
		scan == sc_F10 ||
		scan == sc_F9 ||
		scan == sc_F7 ||
		scan == sc_F8)			// pop up quit dialog
	{
		ClearMemory ();
		ClearSplitVWB ();
		VW_ScreenToScreen (displayofs,bufferofs,80,160);
		US_ControlPanel(scan);

		 DrawAllPlayBorderSides ();

		if (scan == sc_F9)
		  mapmus ();

		PM_CheckMainMem ();
		SETFONTCOLOR(0,15);
		IN_ClearKeysDown();
		return;
	}

	if ( (scan >= sc_F1 && scan <= sc_F9) || scan == sc_Escape)
	{
		stopmus ();
		ClearMemory ();
		VW_FadeOut ();

		US_ControlPanel(scan);

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
		return;
	}

//
// TAB-? debug keys
//
	if (Keyboard[sc_Tab] && DebugOk)
	{
		CA_CacheGrChunk (STARTFONT);
		fontnumber=0;
		SETFONTCOLOR(0,15);
		DebugKeys();
		if (MousePresent)
			Mouse(MDelta);	// Clear accumulated mouse movement
		lasttimecount = TimeCount;
		return;
	}

}
