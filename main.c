char            str[80],str2[20];
s16int				tedlevelnum;
int         tedlevel;
s16int  dirangle[9] = {0,ANGLES/8,2*ANGLES/8,3*ANGLES/8,4*ANGLES/8,
	5*ANGLES/8,6*ANGLES/8,7*ANGLES/8,ANGLES};

u16int        screenofs;

int         startgame;
s16int             mouseadjustment;

char	configname[13]="CONFIG.";


void ReadConfig(void)
{
	s16int                     file;
	SDMode          sd;
	SMMode          sm;
	SDSMode         sds;


	if ( (file = open(configname,O_BINARY | O_RDONLY)) != -1)
	{
	//
	// valid config file
	//
		read(file,Scores,sizeof(HighScore) * MaxScores);

		read(file,&sd,sizeof(sd));
		read(file,&sm,sizeof(sm));
		read(file,&sds,sizeof(sds));

		read(file,&mouseenabled,sizeof(mouseenabled));
		read(file,&joystickenabled,sizeof(joystickenabled));
		read(file,&joypadenabled,sizeof(joypadenabled));
		read(file,&joystickprogressive,sizeof(joystickprogressive));
		read(file,&joystickport,sizeof(joystickport));

		read(file,&dirscan,sizeof(dirscan));
		read(file,&buttonscan,sizeof(buttonscan));
		read(file,&buttonmouse,sizeof(buttonmouse));
		read(file,&buttonjoy,sizeof(buttonjoy));

		read(file,&vw.size,sizeof(vw.size));
		read(file,&mouseadjustment,sizeof(mouseadjustment));

		close(file);

		if (sd == sdm_AdLib && !AdLibPresent && !SoundBlasterPresent)
		{
			sd = sdm_PC;
			sd = smm_Off;
		}

		if ((sds == sds_SoundBlaster && !SoundBlasterPresent) ||
			(sds == sds_SoundSource && !SoundSourcePresent))
			sds = sds_Off;

		if (!MousePresent)
			mouseenabled = false;

		MainMenu[6].active=1;
		MainItems.curpos=0;
	}
	else
	{
	//
	// no config file, so select by hardware
	//
		if (SoundBlasterPresent || AdLibPresent)
		{
			sd = sdm_AdLib;
			sm = smm_AdLib;
		}
		else
		{
			sd = sdm_PC;
			sm = smm_Off;
		}

		if (SoundBlasterPresent)
			sds = sds_SoundBlaster;
		else if (SoundSourcePresent)
			sds = sds_SoundSource;
		else
			sds = sds_Off;

		if (MousePresent)
			mouseenabled = true;

		vw.size = 15;
		mouseadjustment=5;
	}

	SD_SetMusicMode (sm);
	SD_SetSoundMode (sd);
	SD_SetDigiDevice (sds);

}

void WriteConfig(void)
{
	s16int                     file;

	file = open(configname,O_CREAT | O_BINARY | O_WRONLY,
				S_IREAD | S_IWRITE | S_IFREG);

	if (file != -1)
	{
		write(file,Scores,sizeof(HighScore) * MaxScores);

		write(file,&SoundMode,sizeof(SoundMode));
		write(file,&MusicMode,sizeof(MusicMode));
		write(file,&DigiMode,sizeof(DigiMode));

		write(file,&mouseenabled,sizeof(mouseenabled));
		write(file,&joystickenabled,sizeof(joystickenabled));
		write(file,&joypadenabled,sizeof(joypadenabled));
		write(file,&joystickprogressive,sizeof(joystickprogressive));
		write(file,&joystickport,sizeof(joystickport));

		write(file,&dirscan,sizeof(dirscan));
		write(file,&buttonscan,sizeof(buttonscan));
		write(file,&buttonmouse,sizeof(buttonmouse));
		write(file,&buttonjoy,sizeof(buttonjoy));

		write(file,&vw.size,sizeof(vw.size));
		write(file,&mouseadjustment,sizeof(mouseadjustment));

		close(file);
	}
}

void NewGame (s16int difficulty,s16int episode)
{
	→ initg, w/o difficulty, map
}

void DiskFlopAnim(s16int x,s16int y)
{
 static char which=0;
 if (!x && !y)
   return;
 VWB_DrawPic(x,y,Pread1+which);
 VW_UpdateScreen();
 which^=1;
}


s32int DoChecksum(u8int far *source,u16int size,s32int checksum)
{
 u16int i;

 for (i=0;i<size-1;i++)
   checksum += source[i]^source[i+1];

 return checksum;
}

int SaveTheGame(s16int file,s16int x,s16int y)
{
	struct diskfree_t dfree;
	s32int avail,size,checksum;
	objtype *ob,nullobj;


	if (_dos_getdiskfree(0,&dfree))
	  Quit("Error in _dos_getdiskfree call");

	avail = (s32int)dfree.avail_clusters *
			dfree.bytes_per_sector *
			dfree.sectors_per_cluster;

	size = 0;
	for (ob = player; ob ; ob=ob->next)
	  size += sizeof(*ob);
	size += sizeof(nullobj);

	size += sizeof(gamestate) +
			sizeof(LRstruct)*8 +
			sizeof(tilemap) +
			sizeof(actorat) +
			sizeof(laststatobj) +
			sizeof(statobjlist) +
			sizeof(doorposition) +
			sizeof(pwallstate) +
			sizeof(pwallx) +
			sizeof(pwally) +
			sizeof(pwalldir) +
			sizeof(pwallpos);

	if (avail < size)
	{
	 Message("There is not enough space\n"
		"on your disk to Save Game!");
	 return false;
	}

	checksum = 0;


	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)&gamestate,sizeof(gamestate));
	checksum = DoChecksum((u8int far *)&gamestate,sizeof(gamestate),checksum);

	DiskFlopAnim(x,y);
#ifdef SPEAR
	CA_FarWrite (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*20);
	checksum = DoChecksum((u8int far *)&LevelRatios[0],sizeof(LRstruct)*20,checksum);
#else
	CA_FarWrite (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*8);
	checksum = DoChecksum((u8int far *)&LevelRatios[0],sizeof(LRstruct)*8,checksum);
#endif

	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)tilemap,sizeof(tilemap));
	checksum = DoChecksum((u8int far *)tilemap,sizeof(tilemap),checksum);
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)actorat,sizeof(actorat));
	checksum = DoChecksum((u8int far *)actorat,sizeof(actorat),checksum);

	CA_FarWrite (file,(void far *)conarea,sizeof(conarea));
	CA_FarWrite (file,(void far *)plrarea,sizeof(plrarea));

	for (ob = player ; ob ; ob=ob->next)
	{
	 DiskFlopAnim(x,y);
	 CA_FarWrite (file,(void far *)ob,sizeof(*ob));
	}
	nullobj.active = ac_badobject;          // end of file marker
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)&nullobj,sizeof(nullobj));



	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)&laststatobj,sizeof(laststatobj));
	checksum = DoChecksum((u8int far *)&laststatobj,sizeof(laststatobj),checksum);
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)statobjlist,sizeof(statobjlist));
	checksum = DoChecksum((u8int far *)statobjlist,sizeof(statobjlist),checksum);

	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)doorposition,sizeof(doorposition));
	checksum = DoChecksum((u8int far *)doorposition,sizeof(doorposition),checksum);
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)doorobjlist,sizeof(doorobjlist));
	checksum = DoChecksum((u8int far *)doorobjlist,sizeof(doorobjlist),checksum);

	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void far *)&pwallstate,sizeof(pwallstate));
	checksum = DoChecksum((u8int far *)&pwallstate,sizeof(pwallstate),checksum);
	CA_FarWrite (file,(void far *)&pwallx,sizeof(pwallx));
	checksum = DoChecksum((u8int far *)&pwallx,sizeof(pwallx),checksum);
	CA_FarWrite (file,(void far *)&pwally,sizeof(pwally));
	checksum = DoChecksum((u8int far *)&pwally,sizeof(pwally),checksum);
	CA_FarWrite (file,(void far *)&pwalldir,sizeof(pwalldir));
	checksum = DoChecksum((u8int far *)&pwalldir,sizeof(pwalldir),checksum);
	CA_FarWrite (file,(void far *)&pwallpos,sizeof(pwallpos));
	checksum = DoChecksum((u8int far *)&pwallpos,sizeof(pwallpos),checksum);

	//
	// WRITE OUT CHECKSUM
	//
	CA_FarWrite (file,(void far *)&checksum,sizeof(checksum));

	return(true);
}

int LoadTheGame(s16int file,s16int x,s16int y)
{
	s32int checksum,oldchecksum;
	objtype *ob,nullobj;


	checksum = 0;

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)&gamestate,sizeof(gamestate));
	checksum = DoChecksum((u8int far *)&gamestate,sizeof(gamestate),checksum);

	DiskFlopAnim(x,y);
#ifdef SPEAR
	CA_FarRead (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*20);
	checksum = DoChecksum((u8int far *)&LevelRatios[0],sizeof(LRstruct)*20,checksum);
#else
	CA_FarRead (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*8);
	checksum = DoChecksum((u8int far *)&LevelRatios[0],sizeof(LRstruct)*8,checksum);
#endif

	DiskFlopAnim(x,y);
	initmap ();

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)tilemap,sizeof(tilemap));
	checksum = DoChecksum((u8int far *)tilemap,sizeof(tilemap),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)actorat,sizeof(actorat));
	checksum = DoChecksum((u8int far *)actorat,sizeof(actorat),checksum);

	CA_FarRead (file,(void far *)conarea,sizeof(conarea));
	CA_FarRead (file,(void far *)plrarea,sizeof(plrarea));



	oinit ();
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)player,sizeof(*player));

	while (1)
	{
	 DiskFlopAnim(x,y);
		CA_FarRead (file,(void far *)&nullobj,sizeof(nullobj));
		if (nullobj.active == ac_badobject)
			break;
		onew ();
	 // don't copy over the links
		memcpy (new,&nullobj,sizeof(nullobj)-4);
	}



	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)&laststatobj,sizeof(laststatobj));
	checksum = DoChecksum((u8int far *)&laststatobj,sizeof(laststatobj),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)statobjlist,sizeof(statobjlist));
	checksum = DoChecksum((u8int far *)statobjlist,sizeof(statobjlist),checksum);

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)doorposition,sizeof(doorposition));
	checksum = DoChecksum((u8int far *)doorposition,sizeof(doorposition),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)doorobjlist,sizeof(doorobjlist));
	checksum = DoChecksum((u8int far *)doorobjlist,sizeof(doorobjlist),checksum);

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)&pwallstate,sizeof(pwallstate));
	checksum = DoChecksum((u8int far *)&pwallstate,sizeof(pwallstate),checksum);
	CA_FarRead (file,(void far *)&pwallx,sizeof(pwallx));
	checksum = DoChecksum((u8int far *)&pwallx,sizeof(pwallx),checksum);
	CA_FarRead (file,(void far *)&pwally,sizeof(pwally));
	checksum = DoChecksum((u8int far *)&pwally,sizeof(pwally),checksum);
	CA_FarRead (file,(void far *)&pwalldir,sizeof(pwalldir));
	checksum = DoChecksum((u8int far *)&pwalldir,sizeof(pwalldir),checksum);
	CA_FarRead (file,(void far *)&pwallpos,sizeof(pwallpos));
	checksum = DoChecksum((u8int far *)&pwallpos,sizeof(pwallpos),checksum);

	CA_FarRead (file,(void far *)&oldchecksum,sizeof(oldchecksum));

	if (oldchecksum != checksum)
	{
		Message("Your Save Game file is,\n"
		"shall we say, \"corrupted\".\n"
		"But I'll let you go on and\n"
		"play anyway....");

		IN_ClearKeysDown();
		IN_Ack();

		gm.score = 0;
		gm.lives = 1;
		gm.w = gm.bestw = gm.lastw = WPpistol;
		gm.ammo = 8;
	}

	return true;
}

void SetupWalls (void)	/* map tile values to scaled pics */
{
	s16int     i;

	for (i=1;i<MAXWALLTILES;i++)
	{
		horizwall[i]=(i-1)*2;
		vertwall[i]=(i-1)*2+1;
	}
}

#define	PORTTILESHIGH		13		// non displayed port of this size

void InitGame (void)
{
	s16int                     i,x,y;
	u16int        *blockstart;

	mapon = -1;

	for (i=0;i<MAPSIZE;i++)
	{
		farmapylookup[i] = i*64;
	}

	for (i=0;i<PORTTILESHIGH;i++)
		uwidthtable[i] = UPDATEWIDE*i;

	blockstart = &blockstarts[0];
	for (y=0;y<UPDATEHIGH;y++)
		for (x=0;x<UPDATEWIDE;x++)
			*blockstart++ = SCREENWIDTH*16*y+x*TILEWIDTH;

	updateptr = &update[0];

	bufferofs = 0;
	displayofs = 0;
	ReadConfig ();

	IntroScreen ();

	LoadLatchMem ();
	SetupWalls ();

	NewViewSize (vw.size);

	InitRedShifts ();

	displayofs = PAGE1START;
	bufferofs = PAGE2START;
}

int SetViewSize (u16int width, u16int height)
{
	→ setvw()
}

void ShowViewSize (s16int width)
{
	s16int     oldwidth,oldheight;

	oldwidth = vw.dx;
	oldheight = vw.dy;

	vw.dx = width*16;
	vw.dy = width*16*HEIGHTRATIO;
	DrawPlayBorder ();

	vw.dy = oldheight;
	vw.dx = oldwidth;
}

void NewViewSize (s16int width)
{
	vw.size = width;
	SetViewSize (width*16,width*16*HEIGHTRATIO);
}

void    DemoLoop (void)
{
	static s16int LastDemo;
	s16int     i,level;
	s32int nsize;
	uchar *nullblock;

//
// check for launch from ted
//
	/* → if warping to map [tedlevel] */
	if (tedlevel)
	{
		NoWait = true;
		NewGame(1,0);

		/* → set difficulty level 1-4 if parameter passed as
		 * gamestate.difficulty */

#ifndef SPEAR
		gamestate.episode = tedlevelnum/10;
		gamestate.mapon = tedlevelnum%10;
#else
		gamestate.episode = 0;
		gamestate.mapon = tedlevelnum;
#endif
		GameLoop();
		Quit (NULL);
	}

	StartCPMusic(INTROSONG);
	// pg13

	while (1)
	{
		p = dems;
		while (!NoWait)
		{
			/* title loop */
			PlayDemo(p++);
			if(p >= epis)
				p = dems;
			if (gm.φ == ex_abort)
				break;
			StartCPMusic(INTROSONG);
		}

		VW_FadeOut ();
		if (Keyboard[sc_Tab] && debug)
			RecordDemo ();
		else
			US_ControlPanel (0);

		if (startgame || gm.load)
		{
			GameLoop ();
			VW_FadeOut();
			StartCPMusic(INTROSONG);
		}
	}
}

void main (void)
{
	if (wl6)
	{
		NewEmenu[2].active =
		NewEmenu[4].active =
		NewEmenu[6].active =
		NewEmenu[8].active =
		NewEmenu[10].active =
		EpisodeSelect[1] =
		EpisodeSelect[2] =
		EpisodeSelect[3] =
		EpisodeSelect[4] =
		EpisodeSelect[5] = 1;
	}

	InitGame ();

	DemoLoop();
}

