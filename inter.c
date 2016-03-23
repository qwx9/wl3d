// WL_INTER.C

#include "WL_DEF.H"
#pragma hdrstop


//==========================================================================

/*
==================
=
= CLearSplitVWB
=
==================
*/

void ClearSplitVWB (void)
{
	memset (update,0,sizeof(update));
	WindowX = 0;
	WindowY = 0;
	WindowW = 320;
	WindowH = 160;
}


//==========================================================================

#ifdef SPEAR
#ifndef SPEARDEMO
////////////////////////////////////////////////////////
//
// End of Spear of Destiny
//
////////////////////////////////////////////////////////

void EndScreen (s16int palette, s16int screen)
{
	CA_CacheScreen (screen);
	VW_UpdateScreen ();
	VL_FadeIn(0,255,grsegs[palette],30);
	IN_ClearKeysDown ();
	IN_Ack ();
	VW_FadeOut ();
}


void EndSpear(void)
{
	EndScreen (Eend1, Pend1);

	CA_CacheScreen (Pend1+2);
	VW_UpdateScreen ();
	VL_FadeIn(0,255,Eend1+2,30);
	fontnumber = 0;
	fontcolor = 0xd0;
	WindowX = 0;
	WindowW = 320;
	PrintX = 0;
	PrintY = 180;
	US_CPrint ("We owe you a great debt, Mr. Blazkowicz.\n");
	US_CPrint ("You have served your country well.");
	VW_UpdateScreen ();
	IN_StartAck ();
	TimeCount = 0;
	while (!IN_CheckAck () && TimeCount < 700);

	PrintX = 0;
	PrintY = 180;
	VWB_Bar(0,180,320,20,0);
	US_CPrint ("With the spear gone, the Allies will finally\n");
	US_CPrint ("be able to destroy Hitler...");
	VW_UpdateScreen ();
	IN_StartAck ();
	TimeCount = 0;
	while (!IN_CheckAck () && TimeCount < 700);

	VW_FadeOut ();

	EndScreen (Eend1+3, Pend1+3);
	EndScreen (Eend1+4, Pend1+4);
	EndScreen (Eend1+5, Pend1+5);
	EndScreen (Eend1+6, Pend1+6);
	EndScreen (Eend1+7, Pend1+7);
	EndScreen (Eend1+8, Pend1+8);
	EndScreen (Eend1+1, Pend1+1);

	MainMenu[savegame].active = 0;
}
#endif
#endif

//==========================================================================

/*
==================
=
= Victory
=
==================
*/

void Victory (void)
{
#ifndef SPEARDEMO
	s32int	sec;
	s16int i,min,kr,sr,tr,x;
	char tempstr[8];

#define RATIOX	6
#define RATIOY	14
#define TIMEX	14
#define TIMEY	8


#ifdef SPEAR
	StartCPMusic (7);

	VWB_Bar(0,0,320,200,VIEWCOLOR);
	VWB_DrawPic (124,44,Pcollapse);
	VW_UpdateScreen ();
	VW_FadeIn ();
	VW_WaitVBL(2*70);
	VWB_DrawPic (124,44,Pcollapse+1);
	VW_UpdateScreen ();
	VW_WaitVBL(105);
	VWB_DrawPic (124,44,Pcollapse+2);
	VW_UpdateScreen ();
	VW_WaitVBL(105);
	VWB_DrawPic (124,44,Pcollapse+3);
	VW_UpdateScreen ();
	VW_WaitVBL(3*70);

	VL_FadeOut (0,255,0,17,17,5);
#endif

#ifndef SPEAR
	StartCPMusic (24);
#else
	StartCPMusic (6);
#endif
	ClearSplitVWB ();

	VWB_Bar (0,0,320,200-STATUSLINES,127);
	Write(18,2,"you win!");

	Write(TIMEX,TIMEY-2,"total time");

	Write(12,RATIOY-2,"averages");

	Write(RATIOX+8,RATIOY,"kill    %");
	Write(RATIOX+4,RATIOY+2,"secret    %");
	Write(RATIOX,  RATIOY+4,"treasure    %");

	VWB_DrawPic (8,4,Pwin);

#ifndef SPEAR
	for (kr = sr = tr = sec = i = 0;i < 8;i++)
#else
	for (kr = sr = tr = sec = i = 0;i < 20;i++)
#endif
	{
		sec += LevelRatios[i].time;
		kr += LevelRatios[i].kill;
		sr += LevelRatios[i].secret;
		tr += LevelRatios[i].treasure;
	}

#ifndef SPEAR
	kr /= 8;
	sr /= 8;
	tr /= 8;
#else
	kr /= 14;
	sr /= 14;
	tr /= 14;
#endif

	min = sec/60;
	sec %= 60;

	if (min > 99)
		min = sec = 99;

	i = TIMEX*8+1;
	VWB_DrawPic(i,TIMEY*8,P0+(min/10));
	i += 2*8;
	VWB_DrawPic(i,TIMEY*8,P0+(min%10));
	i += 2*8;
	Write(i/8,TIMEY,":");
	i += 1*8;
	VWB_DrawPic(i,TIMEY*8,P0+(sec/10));
	i += 2*8;
	VWB_DrawPic(i,TIMEY*8,P0+(sec%10));
	VW_UpdateScreen ();

	itoa(kr,tempstr,10);
	x=RATIOX+24-strlen(tempstr)*2;
	Write(x,RATIOY,tempstr);

	itoa(sr,tempstr,10);
	x=RATIOX+24-strlen(tempstr)*2;
	Write(x,RATIOY+2,tempstr);

	itoa(tr,tempstr,10);
	x=RATIOX+24-strlen(tempstr)*2;
	Write(x,RATIOY+4,tempstr);


#ifndef UPLOAD
#ifndef SPEAR
	//
	// TOTAL TIME VERIFICATION CODE
	//
	if (gamestate.difficulty>=gd_medium)
	{
		VWB_DrawPic (30*8,TIMEY*8,Ptc);
		fontnumber = 0;
		fontcolor = READHCOLOR;
		PrintX = 30*8-3;
		PrintY = TIMEY*8+8;
		PrintX+=4;
		tempstr[0] = (((min/10)^(min%10))^0xa)+'A';
		tempstr[1] = (((sec/10)^(sec%10))^0xa)+'A';
		tempstr[2] = (tempstr[0]^tempstr[1])+'A';
		tempstr[3] = 0;
		US_Print(tempstr);
	}
#endif
#endif


	fontnumber = 1;

	VW_UpdateScreen ();
	VW_FadeIn ();

	IN_Ack();

	VW_FadeOut ();

#ifndef SPEAR
	EndText();
#else
	EndSpear();
#endif

#endif // SPEARDEMO
}


//==========================================================================

/*
==================
=
= PG13
=
==================
*/

void PG13 (void)
{
	VW_FadeOut();
	VWB_Bar(0,0,320,200,0x82);			// background

	VWB_DrawPic (216,110,Ppg13);
	VW_UpdateScreen ();

	VW_FadeIn();
	IN_UserInput(TickBase*7);

	VW_FadeOut ();
}


//==========================================================================

void Write(s16int x,s16int y,char *string)
{
FIXME: use P0+n, Pa+n
 s16int alpha[]={P0,P1,P2,P3,P4,P5,
	P6,P7,P8,P9,Pcolon,0,0,0,0,0,0,Pa,Pb,
	Pc,Pd,Pe,Pf,Pg,Ph,Pi,Pj,Pk,
	Pl,Pm,Pn,Po,Pp,Pq,Pr,Ps,Pt,
	Pu,Pv,Pw,Px,Py,Pz};

 s16int i,ox,nx,ny;
 char ch;


 ox=nx=x*8;
 ny=y*8;
 for (i=0;i<strlen(string);i++)
   if (string[i]=='\n')
   {
	nx=ox;
	ny+=16;
   }
   else
   {
	ch=string[i];
	if (ch>='a')
	  ch-=('a'-'A');
	ch-='0';

	switch(string[i])
	{
	 case '!':
	   VWB_DrawPic(nx,ny,Pexcl);
	   nx+=8;
	   continue;

	 case '\'':
	   VWB_DrawPic(nx,ny,Papo);
	   nx+=8;
	   continue;

	 case ' ': break;
	 case 0x3a:	// ':'

	   VWB_DrawPic(nx,ny,Pcolon);
	   nx+=8;
	   continue;

	 case '%':
	   VWB_DrawPic(nx,ny,Ppercent);
	   break;

	 default:
	   VWB_DrawPic(nx,ny,alpha[ch]);
	}
	nx+=16;
   }
}


//
// Breathe Mr. BJ!!!
//
void BJ_Breathe(void)
{
	static s16int which=0,max=10;
	s16int pics[2]={Pguy,Pguy2};


	if (TimeCount>max)
	{
		which^=1;
		VWB_DrawPic(0,16,pics[which]);
		VW_UpdateScreen();
		TimeCount=0;
		max=35;
	}
}



/*
==================
=
= LevelCompleted
=
= Entered with the screen faded out
= Still in split screen mode with the status bar
=
= Exit with the screen faded out
=
==================
*/

#ifndef SPEAR
LRstruct LevelRatios[8];
#else
LRstruct LevelRatios[20];
#endif

void LevelCompleted (void)
{
	#define VBLWAIT	30
	#define PAR_AMOUNT	500
	#define PERCENT100AMT	10000
	typedef struct {
			float time;
			char timestr[6];
			} times;

	s16int	x,i,min,sec,ratio,kr,sr,tr;
	u16int	temp;
	char tempstr[10];
	s32int bonus,timeleft=0;
	times parTimes[]=
	{
#ifndef SPEAR
	 //
	 // Episode One Par Times
	 //
	 {1.5,	"01:30"},
	 {2,	"02:00"},
	 {2,	"02:00"},
	 {3.5,	"03:30"},
	 {3,	"03:00"},
	 {3,	"03:00"},
	 {2.5,	"02:30"},
	 {2.5,	"02:30"},
	 {0,	"??:??"},	// Boss level
	 {0,	"??:??"},	// Secret level

	 //
	 // Episode Two Par Times
	 //
	 {1.5,	"01:30"},
	 {3.5,	"03:30"},
	 {3,	"03:00"},
	 {2,	"02:00"},
	 {4,	"04:00"},
	 {6,	"06:00"},
	 {1,	"01:00"},
	 {3,	"03:00"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 //
	 // Episode Three Par Times
	 //
	 {1.5,	"01:30"},
	 {1.5,	"01:30"},
	 {2.5,	"02:30"},
	 {2.5,	"02:30"},
	 {3.5,	"03:30"},
	 {2.5,	"02:30"},
	 {2,	"02:00"},
	 {6,	"06:00"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 //
	 // Episode Four Par Times
	 //
	 {2,	"02:00"},
	 {2,	"02:00"},
	 {1.5,	"01:30"},
	 {1,	"01:00"},
	 {4.5,	"04:30"},
	 {3.5,	"03:30"},
	 {2,	"02:00"},
	 {4.5,	"04:30"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 //
	 // Episode Five Par Times
	 //
	 {2.5,	"02:30"},
	 {1.5,	"01:30"},
	 {2.5,	"02:30"},
	 {2.5,	"02:30"},
	 {4,	"04:00"},
	 {3,	"03:00"},
	 {4.5,	"04:30"},
	 {3.5,	"03:30"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 //
	 // Episode Six Par Times
	 //
	 {6.5,	"06:30"},
	 {4,	"04:00"},
	 {4.5,	"04:30"},
	 {6,	"06:00"},
	 {5,	"05:00"},
	 {5.5,	"05:30"},
	 {5.5,	"05:30"},
	 {8.5,	"08:30"},
	 {0,	"??:??"},
	 {0,	"??:??"}
#else
	 //
	 // SPEAR OF DESTINY TIMES
	 //
	 {1.5,	"01:30"},
	 {3.5,	"03:30"},
	 {2.75,	"02:45"},
	 {3.5,	"03:30"},
	 {0,	"??:??"},	// Boss 1
	 {4.5,	"04:30"},
	 {3.25,	"03:15"},
	 {2.75,	"02:45"},
	 {4.75,	"04:45"},
	 {0,	"??:??"},	// Boss 2
	 {6.5,	"06:30"},
	 {4.5,	"04:30"},
	 {2.75,	"02:45"},
	 {4.5,	"04:30"},
	 {6,	"06:00"},
	 {0,	"??:??"},	// Boss 3
	 {6,	"06:00"},
	 {0,	"??:??"},	// Boss 4
	 {0,	"??:??"},	// Secret level 1
	 {0,	"??:??"},	// Secret level 2
#endif
	};

	ClearSplitVWB ();			// set up for double buffering in split screen
	VWB_Bar (0,0,320,200-STATUSLINES,127);
	StartCPMusic(16);

//
// do the intermission
//
	IN_ClearKeysDown();
	IN_StartAck();

	VWB_DrawPic(0,16,Pguy);

#ifndef SPEAR
	if (mapon<8)
#else
	if (mapon != 4 &&
		mapon != 9 &&
		mapon != 15 &&
		mapon < 17)
#endif
	{
	 Write(14,2,"floor\ncompleted");

	 Write(14,7,"bonus     0");
	 Write(16,10,"time");
	 Write(16,12," par");

	 Write(9,14,"kill ratio    %");
	 Write(5,16,"secret ratio    %");
	 Write(1,18,"treasure ratio    %");

	 Write(26,2,itoa(gamestate.mapon+1,tempstr,10));

	 Write(26,12,parTimes[gamestate.episode*10+mapon].timestr);

	 //
	 // PRINT TIME
	 //
	 sec=gamestate.TimeCount/70;

	 if (sec > 99*60)		// 99 minutes max
	   sec = 99*60;

	 if (gamestate.TimeCount<parTimes[gamestate.episode*10+mapon].time*4200)
		timeleft=(parTimes[gamestate.episode*10+mapon].time*4200)/70-sec;

	 min=sec/60;
	 sec%=60;

	 i=26*8;
	 VWB_DrawPic(i,10*8,P0+(min/10));
	 i+=2*8;
	 VWB_DrawPic(i,10*8,P0+(min%10));
	 i+=2*8;
	 Write(i/8,10,":");
	 i+=1*8;
	 VWB_DrawPic(i,10*8,P0+(sec/10));
	 i+=2*8;
	 VWB_DrawPic(i,10*8,P0+(sec%10));

	 VW_UpdateScreen ();
	 VW_FadeIn ();


	 //
	 // FIGURE RATIOS OUT BEFOREHAND
	 //
	 kr = sr = tr = 0;
	 if (gamestate.killtotal)
		kr=(gamestate.killcount*100)/gamestate.killtotal;
	 if (gamestate.secrettotal)
		sr=(gamestate.secretcount*100)/gamestate.secrettotal;
	 if (gamestate.treasuretotal)
		tr=(gamestate.treasurecount*100)/gamestate.treasuretotal;


	 //
	 // PRINT TIME BONUS
	 //
	 bonus=timeleft*PAR_AMOUNT;
	 if (bonus)
	 {
	  for (i=0;i<=timeleft;i++)
	  {
	   ltoa((s32int)i*PAR_AMOUNT,tempstr,10);
	   x=36-strlen(tempstr)*2;
	   Write(x,7,tempstr);
	   if (!(i%(PAR_AMOUNT/10)))
		 SD_PlaySound(Sendb1);
	   VW_UpdateScreen();
	   while(SD_SoundPlaying())
		 BJ_Breathe();
	   if (IN_CheckAck())
		 goto done;
	  }

	  VW_UpdateScreen();
	  SD_PlaySound(Sendb2);
	  while(SD_SoundPlaying())
		BJ_Breathe();
	 }


	 #define RATIOXX		37
	 //
	 // KILL RATIO
	 //
	 ratio=kr;
	 for (i=0;i<=ratio;i++)
	 {
	  itoa(i,tempstr,10);
	  x=RATIOXX-strlen(tempstr)*2;
	  Write(x,14,tempstr);
	  if (!(i%10))
		SD_PlaySound(Sendb1);
	  VW_UpdateScreen ();
	  while(SD_SoundPlaying())
		BJ_Breathe();

	  if (IN_CheckAck())
		goto done;
	 }
	 if (ratio==100)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   bonus+=PERCENT100AMT;
	   ltoa(bonus,tempstr,10);
	   x=(RATIOXX-1)-strlen(tempstr)*2;
	   Write(x,7,tempstr);
	   VW_UpdateScreen();
	   SD_PlaySound(S100);
	 }
	 else
	 if (!ratio)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   SD_PlaySound(Snobonus);
	 }
	 else
	 SD_PlaySound(Sendb2);

	 VW_UpdateScreen();
	 while(SD_SoundPlaying())
	   BJ_Breathe();


	 //
	 // SECRET RATIO
	 //
	 ratio=sr;
	 for (i=0;i<=ratio;i++)
	 {
	  itoa(i,tempstr,10);
	  x=RATIOXX-strlen(tempstr)*2;
	  Write(x,16,tempstr);
	  if (!(i%10))
		SD_PlaySound(Sendb1);
	  VW_UpdateScreen ();
	  while(SD_SoundPlaying())
		BJ_Breathe();
	  BJ_Breathe();

	  if (IN_CheckAck())
		goto done;
	 }
	 if (ratio==100)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   bonus+=PERCENT100AMT;
	   ltoa(bonus,tempstr,10);
	   x=(RATIOXX-1)-strlen(tempstr)*2;
	   Write(x,7,tempstr);
	   VW_UpdateScreen();
	   SD_PlaySound(S100);
	 }
	 else
	 if (!ratio)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   SD_PlaySound(Snobonus);
	 }
	 else
	   SD_PlaySound(Sendb2);
	 VW_UpdateScreen();
	 while(SD_SoundPlaying())
	   BJ_Breathe();


	 //
	 // TREASURE RATIO
	 //
	 ratio=tr;
	 for (i=0;i<=ratio;i++)
	 {
	  itoa(i,tempstr,10);
	  x=RATIOXX-strlen(tempstr)*2;
	  Write(x,18,tempstr);
	  if (!(i%10))
		SD_PlaySound(Sendb1);
	  VW_UpdateScreen ();
	  while(SD_SoundPlaying())
		BJ_Breathe();
	  if (IN_CheckAck())
		goto done;
	 }
	 if (ratio==100)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   bonus+=PERCENT100AMT;
	   ltoa(bonus,tempstr,10);
	   x=(RATIOXX-1)-strlen(tempstr)*2;
	   Write(x,7,tempstr);
	   VW_UpdateScreen();
	   SD_PlaySound(S100);
	 }
	 else
	 if (!ratio)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   SD_PlaySound(Snobonus);
	 }
	 else
	 SD_PlaySound(Sendb2);
	 VW_UpdateScreen();
	 while(SD_SoundPlaying())
	   BJ_Breathe();


	 //
	 // JUMP STRAIGHT HERE IF KEY PRESSED
	 //
	 done:

	 itoa(kr,tempstr,10);
	 x=RATIOXX-strlen(tempstr)*2;
	 Write(x,14,tempstr);

	 itoa(sr,tempstr,10);
	 x=RATIOXX-strlen(tempstr)*2;
	 Write(x,16,tempstr);

	 itoa(tr,tempstr,10);
	 x=RATIOXX-strlen(tempstr)*2;
	 Write(x,18,tempstr);

	 bonus=(s32int)timeleft*PAR_AMOUNT+
		   (PERCENT100AMT*(kr==100))+
		   (PERCENT100AMT*(sr==100))+
		   (PERCENT100AMT*(tr==100));

	 GivePoints(bonus);
	 ltoa(bonus,tempstr,10);
	 x=36-strlen(tempstr)*2;
	 Write(x,7,tempstr);

	 //
	 // SAVE RATIO INFORMATION FOR ENDGAME
	 //
	 LevelRatios[mapon].kill=kr;
	 LevelRatios[mapon].secret=sr;
	 LevelRatios[mapon].treasure=tr;
	 LevelRatios[mapon].time=min*60+sec;
	}
	else
	{
#ifdef SPEAR
#ifndef SPEARDEMO
	  switch(mapon)
	  {
	   case 4: Write(14,4," trans\n grosse\ndefeated!"); break;
	   case 9: Write(14,4,"barnacle\nwilhelm\ndefeated!"); break;
	   case 15: Write(14,4,"ubermutant\ndefeated!"); break;
	   case 17: Write(14,4," death\n knight\ndefeated!"); break;
	   case 18: Write(13,4,"secret tunnel\n    area\n  completed!"); break;
	   case 19: Write(13,4,"secret castle\n    area\n  completed!"); break;
	  }
#endif
#else
	  Write(14,4,"secret floor\n completed!");
#endif

	  Write(10,16,"15000 bonus!");

	  VW_UpdateScreen();
	  VW_FadeIn();

	  GivePoints(15000);
	}


	DrawScore();
	VW_UpdateScreen();

	TimeCount=0;
	IN_StartAck();
	while(!IN_CheckAck())
	  BJ_Breathe();

//
// done
//
#ifdef SPEARDEMO
	if (gamestate.mapon == 1)
	{
		SD_PlaySound (S1up);
		Message ("This concludes your demo\n"
				 "of Spear of Destiny! Now,\n"
				 "go to your local software\n"
				 "store and buy it!");

		IN_ClearKeysDown();
		IN_Ack();
	}
#endif

	VW_FadeOut ();
	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
	}
	bufferofs = temp;
}



//==========================================================================


/*
=================
=
= PreloadGraphics
=
= Fill the cache up
=
=================
*/

int PreloadUpdate(u16int current, u16int total)
{
	u16int w = WindowW - 10;


	VWB_Bar(WindowX + 5,WindowY + WindowH - 3,w,2,BLACK);
	w = ((s32int)w * current) / total;
	if (w)
	{
	 VWB_Bar(WindowX + 5,WindowY + WindowH - 3,w,2,0x37); //SECONDCOLOR);
	 VWB_Bar(WindowX + 5,WindowY + WindowH - 3,w-1,1,0x32);

	}
	VW_UpdateScreen();
	return false;
}

void PreloadGraphics(void)
{
	DrawLevel ();
	ClearSplitVWB ();			// set up for double buffering in split screen

	VWB_Bar (0,0,320,200-STATUSLINES,127);

	LatchDrawPic (20-14,80-3*8,Ppsyched);

	WindowX = 160-14*8;
	WindowY = 80-3*8;
	WindowW = 28*8;
	WindowH = 48;
	VW_UpdateScreen();
	VW_FadeIn ();

	PM_Preload (PreloadUpdate);
	IN_UserInput (70);
	VW_FadeOut ();

	DrawPlayBorder ();
	VW_UpdateScreen ();
}

void	CheckHighScore (s32int score,u16int other)
{
	u16int		i,j;
	s16int			n;
	HighScore	myscore;

	strcpy(myscore.name,"");
	myscore.score = score;
	myscore.episode = gamestate.episode;
	myscore.completed = other;

	for (i = 0,n = -1;i < MaxScores;i++)
	{
		if
		(
			(myscore.score > Scores[i].score)
		||	(
				(myscore.score == Scores[i].score)
			&& 	(myscore.completed > Scores[i].completed)
			)
		)
		{
			for (j = MaxScores;--j > i;)
				Scores[j] = Scores[j - 1];
			Scores[i] = myscore;
			n = i;
			break;
		}
	}

#ifdef SPEAR
	StartCPMusic (20);
#else
	StartCPMusic (23);
#endif
	DrawHighScores ();

	VW_FadeIn ();

	if (n != -1)
	{
	//
	// got a high score
	//
		PrintY = 76 + (16 * n);
#ifndef SPEAR
		PrintX = 4*8;
		backcolor = BORDCOLOR;
		fontcolor = 15;
		US_LineInput(PrintX,PrintY,Scores[n].name,nil,true,MaxHighName,100);
#else
		PrintX = 16;
		fontnumber = 1;
		VWB_Bar (PrintX-2,PrintY-2,145,15,0x9c);
		VW_UpdateScreen ();
		backcolor = 0x9c;
		fontcolor = 15;
		US_LineInput(PrintX,PrintY,Scores[n].name,nil,true,MaxHighName,130);
#endif
	}
	else
	{
		IN_ClearKeysDown ();
		IN_UserInput(500);
	}

}
