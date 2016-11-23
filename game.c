int		ingame,fizzlein;
u16int	latchpics[NUMLATCHPICS];

s32int		spearx,speary;
u16int	spearangle;
int		spearflag;

//
// ELEVATOR BACK MAPS - REMEMBER (-1)!!
//
s16int ElevatorBackTo[]={1,1,7,3,5,3};

void DrawPlayBorderSides (void)
{
	s16int	xl,yl;

	xl = 160-vw.dx/2;
	yl = (200-STATUSLINES-vw.dy)/2;

	VWB_Bar (0,0,xl-1,200-STATUSLINES,127);
	VWB_Bar (xl+vw.dx+1,0,xl-2,200-STATUSLINES,127);

	VWB_Vlin (yl-1,yl+vw.dy,xl-1,0);
	VWB_Vlin (yl-1,yl+vw.dy,xl+vw.dx,125);
}

void DrawAllPlayBorderSides (void)
{
	u16int	i,temp;

	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorderSides ();
	}
	bufferofs = temp;
}

void DrawAllPlayBorder (void)
{
	u16int	i,temp;

	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
	}
	bufferofs = temp;
}

void DrawPlayBorder (void)
{
	s16int	xl,yl;

	VWB_Bar (0,0,320,200-STATUSLINES,127);

	xl = 160-vw.dx/2;
	yl = (200-STATUSLINES-vw.dy)/2;
	VWB_Bar (xl,yl,vw.dx,vw.dy,0);

	VWB_Hlin (xl-1,xl+vw.dx,yl-1,0);
	VWB_Hlin (xl-1,xl+vw.dx,yl+vw.dy,125);
	VWB_Vlin (yl-1,yl+vw.dy,xl-1,0);
	VWB_Vlin (yl-1,yl+vw.dy,xl+vw.dx,125);
	VWB_Plot (xl-1,yl+vw.dy,124);
}

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

void Died (void)
{
	float	fangle;
	s32int	dx,dy;
	s16int		iangle,curangle,clockwise,counter,change;

	gamestate.weapon = -1;			// take away weapon
	sfx (Sdie);
//
// swing around to face attacker
//
	dx = killer->x - player->x;
	dy = player->y - killer->y;

	fangle = atan2(dy,dx);			// returns -pi to pi
	if (fangle<0)
		fangle = Fpi*2+fangle;

	iangle = fangle/(Fpi*2)*ANGLES;

	if (player->angle > iangle)
	{
		counter = player->angle - iangle;
		clockwise = ANGLES-player->angle + iangle;
	}
	else
	{
		clockwise = iangle - player->angle;
		counter = player->angle + ANGLES-iangle;
	}

	curangle = player->angle;

	if (clockwise<counter)
	{
	//
	// rotate clockwise
	//
		if (curangle>iangle)
			curangle -= ANGLES;
		do
		{
			change = tics*DEATHROTATE;
			if (curangle + change > iangle)
				change = iangle-curangle;

			curangle += change;
			player->angle += change;
			if (player->angle >= ANGLES)
				player->angle -= ANGLES;

			render ();
			ttic ();
		} while (curangle != iangle);
	}
	else
	{
	//
	// rotate counterclockwise
	//
		if (curangle<iangle)
			curangle += ANGLES;
		do
		{
			change = -tics*DEATHROTATE;
			if (curangle + change < iangle)
				change = iangle-curangle;

			curangle += change;
			player->angle += change;
			if (player->angle < 0)
				player->angle += ANGLES;

			render ();
			ttic ();
		} while (curangle != iangle);
	}

//
// fade to red
//
	pal = pals[C0];

	bufferofs += screenofs;
	VW_Bar (0,0,vw.dx,vw.dy,4);
	IN_ClearKeysDown ();
	FizzleFade(bufferofs,displayofs+screenofs,vw.dx,vw.dy,70,false);
	bufferofs -= screenofs;
	IN_UserInput(100);
	SD_WaitSoundDone ();

	if (tedlevel == false)	// SO'S YA DON'T GET KILLED WHILE LAUNCHING!
	  gamestate.lives--;

	if (gamestate.lives > -1)
	{
		gm.hp = 100;
		gm.w = gm.bestw = gm.lastw = WPpistol;
		gm.ammo = STARTAMMO;
		gm.keys = 0;
		atkfrm = 0;
		atktc = 0;
		gm.wfrm = 0;
		hudk();
		hudw();
		huda();
		hudh();
		hudf();
		hudl();
	}

}

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

#ifdef SPEAR
		if (gamestate.mapon == 20)	// give them the key allways
		{
			gamestate.keys |= 1;
			hudk ();
		}
#endif

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

#ifdef SPEAR
		if (spearflag)
		{
			SD_StopSound();
			sfx(Sspear);
			if (DigiMode != sds_Off)
			{
				s32int lasttimecount = TimeCount;

				while(TimeCount < lasttimecount+150)
				//while(DigiPlaying!=false)
					SD_Poll();
			}
			else
				SD_WaitSoundDone();

			gamestate.oldscore = gm.pt;
			gamestate.mapon = 20;
			initmap ();
			mapmus ();
			PM_CheckMainMem ();
			oplr->x = spearx;
			oplr->y = speary;
			oplr->tx = spearx >> Dtlshift;
			oplr->ty = speary >> Dtlshift;
			oplr->θ2 = spearangle;
			oplr->areaid = oplr->tl->p0 - MTfloor;
			spearflag = false;
			goto startplayloop;
		}
#endif

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
			gamestate.keys = 0;
			hudk ();
			VW_FadeOut ();


			LevelCompleted ();		// do the intermission
#ifdef SPEARDEMO
			if (gamestate.mapon == 1)
			{
				died = true;			// don't "get psyched!"

				VW_FadeOut ();

				CheckHighScore (gm.pt,gamestate.mapon+1);

				#pragma warn -sus
				_fstrcpy(MainMenu[viewscores].string,"View Scores");
				MainMenu[viewscores].routine = CP_ViewScores;
				#pragma warn +sus

				return;
			}
#endif

			gamestate.oldscore = gm.pt;

#ifndef SPEAR
			//
			// COMING BACK FROM SECRET LEVEL
			//
			if (gamestate.mapon == 9)
				gamestate.mapon = ElevatorBackTo[gamestate.episode];	// back from secret
			else
			//
			// GOING TO SECRET LEVEL
			//
			if (gm.φ == ex_secretlevel)
				gamestate.mapon = 9;
#else

#define FROMSECRET1		3
#define FROMSECRET2		11

			//
			// GOING TO SECRET LEVEL
			//
			if (gm.φ == ex_secretlevel)
				switch(gamestate.mapon)
				{
				 case FROMSECRET1: gamestate.mapon = 18; break;
				 case FROMSECRET2: gamestate.mapon = 19; break;
				}
			else
			//
			// COMING BACK FROM SECRET LEVEL
			//
			if (gamestate.mapon == 18 || gamestate.mapon == 19)
				switch(gamestate.mapon)
				{
				 case 18: gamestate.mapon = FROMSECRET1+1; break;
				 case 19: gamestate.mapon = FROMSECRET2+1; break;
				}
#endif
			else
			//
			// GOING TO NEXT LEVEL
			//
				gamestate.mapon++;


			break;

		case ex_died:
			Died ();
			died = true;			// don't "get psyched!"

			if (gamestate.lives > -1)
				break;				// more lives left

			VW_FadeOut ();

			CheckHighScore (gm.pt,gamestate.mapon+1);

			#pragma warn -sus
			_fstrcpy(MainMenu[viewscores].string,"View Scores");
			MainMenu[viewscores].routine = CP_ViewScores;
			#pragma warn +sus

			return;

		case ex_victorious:

#ifndef SPEAR
			VW_FadeOut ();
#else
			VL_FadeOut (0,255,0,17,17,300);
#endif
			Victory ();

			CheckHighScore (gm.pt,gamestate.mapon+1);

			#pragma warn -sus
			_fstrcpy(MainMenu[viewscores].string,"View Scores");
			MainMenu[viewscores].routine = CP_ViewScores;
			#pragma warn +sus

			return;
		}
	} while (1);
}
