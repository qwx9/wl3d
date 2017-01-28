int		ingame,fizzlein;
u16int	latchpics[NUMLATCHPICS];

#define MAXDEMOSIZE	8192

void StartDemoRecord (s16int levelnumber)
{
	MM_GetPtr (&demobuffer,MAXDEMOSIZE);
	MM_SetLock (&demobuffer,true);
	demoptr = (char far *)demobuffer;
	lastdemoptr = demoptr+MAXDEMOSIZE;

	*demoptr = levelnumber;
	demoptr += 4;				// leave space for length
	gm.record = true;
}

char	demoname[13] = "DEMO?.";

void FinishDemoRecord (void)
{
	s32int	length,level;

	gm.record = false;

	length = demoptr - (char far *)demobuffer;

	demoptr = ((char far *)demobuffer)+1;
	*(u16int far *)demoptr = length;

	CenterWindow(24,3);	/* No. */
	PrintY+=6;
	US_Print(" Demo number (0-9):");
	VW_UpdateScreen();

	if (US_LineInput (px,py,str,NULL,true,2,0))
	{
		level = atoi (str);
		if (level>=0 && level<=9)
		{
			demoname[4] = '0'+level;
			CA_WriteFile (demoname,(void far *)demobuffer,length);
		}
	}
	MM_FreePtr (&demobuffer);
}

void RecordDemo (void)
{
	s16int level,esc;

	CenterWindow(26,3);	/* No. */
	PrintY+=6;
	CA_CacheGrChunk(STARTFONT);
	fontnumber=0;
	US_Print("  Demo which level(1-10):");
	VW_UpdateScreen();
	VW_FadeIn ();
	esc = !US_LineInput (px,py,str,NULL,true,2,0);
	if (esc)
		return;

	level = atoi (str);
	level--;

	SETFONTCOLOR(0,15);
	VW_FadeOut ();

#ifndef SPEAR
	NewGame (GDhard,level/10);
	gamestate.mapon = level%10;
#else
	NewGame (GDhard,0);
	gamestate.mapon = level;
#endif

	StartDemoRecord (level);

	view ();
	VW_FadeIn ();

	startgame = false;
	gm.record = true;

	initmap ();
	mapmus ();
	PM_CheckMainMem ();
	fizzlein = true;

	PlayLoop ();

	gm.demo = false;

	stopmus ();
	VW_FadeOut ();

	FinishDemoRecord ();
}

#define DEATHROTATE 2

void GameLoop (void)
{
	s16int i,xl,yl,xh,yh;
	char num[20];
	int	died;

restartgame:
	SETFONTCOLOR(0,15);
	view ();
	died = false;
restart:
	do
	{
		if (!gm.load)
		  gm.pt = gamestate.oldscore;
		hudp();

		startgame = false;
		if (gm.load)
			gm.load = false;
		else
			initmap ();

		ingame = true;
		mapmus ();
		PM_CheckMainMem ();
		if (!died)
			PreloadGraphics ();
		else
			died = false;

		fizzlein = true;
		hudm ();

startplayloop:
		PlayLoop ();

		stopmus ();
		ingame = false;

		if (gm.record && gm.φ != ex_warped)
			FinishDemoRecord ();

		if (startgame || gm.load)
			goto restartgame;

		switch (gm.φ)
		{
		case ex_completed:
		case ex_secretlevel:
			break;
		case ex_died:
			return;
		case ex_victorious:
			return;
		}
	} while (1);
}
