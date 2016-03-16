// WL_AGENT.C

#include "WL_DEF.H"
#pragma hdrstop


/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define MAXMOUSETURN	10


#define MOVESCALE		150l
#define BACKMOVESCALE	100l
#define ANGLESCALE		20

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/



//
// player state info
//
int		running;
s32int		thrustspeed;

u16int	plux,pluy;			// player coordinates scaled to u16int

s16int			anglefrac;
s16int			gotgatgun;	// JR

objtype		*LastAttacker;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


void	T_Player (objtype *ob);
void	T_Attack (objtype *ob);

statetype s_player = {false,0,0,T_Player,NULL,NULL};
statetype s_attack = {false,0,0,T_Attack,NULL,NULL};


s32int	playerxmove,playerymove;

struct atkinf
{
	char	tics,attack,frame;		// attack is 1 for gun, 2 for knife
} attackinfo[4][14] =

{
{ {6,0,1},{6,2,2},{6,0,3},{6,-1,4} },
{ {6,0,1},{6,1,2},{6,0,3},{6,-1,4} },
{ {6,0,1},{6,1,2},{6,3,3},{6,-1,4} },
{ {6,0,1},{6,1,2},{6,4,3},{6,-1,4} },
};


s16int	strafeangle[9] = {0,90,180,270,45,135,225,315,0};

void DrawWeapon (void);
void GiveWeapon (s16int weapon);
void	GiveAmmo (s16int ammo);

//===========================================================================

//----------

void Attack (void);
void Use (void);
void Search (objtype *ob);
void SelectWeapon (void);
void SelectItem (void);

//----------

int TryMove (objtype *ob);
void T_Player (objtype *ob);

void ClipMove (objtype *ob, s32int xmove, s32int ymove);

/*
=============================================================================

						CONTROL STUFF

=============================================================================
*/

/*
======================
=
= CheckWeaponChange
=
= Keys 1-4 change weapons
=
======================
*/

void CheckWeaponChange (void)
{
	s16int	i,buttons;

	if (!gamestate.ammo)		// must use knife with no ammo
		return;

	for (i=wp_knife ; i<=gamestate.bestweapon ; i++)
		if (buttonstate[bt_readyknife+i-wp_knife])
		{
			gamestate.weapon = gamestate.chosenweapon = i;
			DrawWeapon ();
			return;
		}
}


/*
=======================
=
= ControlMovement
=
= Takes controlx,controly, and buttonstate[bt_strafe]
=
= Changes the player's angle and position
=
= There is an angle hack because when going 70 fps, the roundoff becomes
= significant
=
=======================
*/

void ControlMovement (objtype *ob)
{
	s32int	oldx,oldy;
	s16int		angle,maxxmove;
	s16int		angleunits;
	s32int	speed;

	thrustspeed = 0;

	oldx = player->x;
	oldy = player->y;

//
// side to side move
//
	if (buttonstate[bt_strafe])
	{
	//
	// strafing
	//
	//
		if (controlx > 0)
		{
			angle = ob->angle - ANGLES/4;
			if (angle < 0)
				angle += ANGLES;
			Thrust (angle,controlx*MOVESCALE);	// move to left
		}
		else if (controlx < 0)
		{
			angle = ob->angle + ANGLES/4;
			if (angle >= ANGLES)
				angle -= ANGLES;
			Thrust (angle,-controlx*MOVESCALE);	// move to right
		}
	}
	else
	{
	//
	// not strafing
	//
		anglefrac += controlx;
		angleunits = anglefrac/ANGLESCALE;
		anglefrac -= angleunits*ANGLESCALE;
		ob->angle -= angleunits;

		if (ob->angle >= ANGLES)
			ob->angle -= ANGLES;
		if (ob->angle < 0)
			ob->angle += ANGLES;

	}

//
// forward/backwards move
//
	if (controly < 0)
	{
		Thrust (ob->angle,-controly*MOVESCALE);	// move forwards
	}
	else if (controly > 0)
	{
		angle = ob->angle + ANGLES/2;
		if (angle >= ANGLES)
			angle -= ANGLES;
		Thrust (angle,controly*BACKMOVESCALE);		// move backwards
	}

	if (gamestate.victoryflag)		// watching the BJ actor
		return;

//
// calculate total move
//
	playerxmove = player->x - oldx;
	playerymove = player->y - oldy;
}

/*
=============================================================================

					STATUS WINDOW STUFF

=============================================================================
*/


/*
==================
=
= StatusDrawPic
=
==================
*/

void StatusDrawPic (u16int x, u16int y, u16int picnum)
{
	u16int	temp;

	temp = bufferofs;
	bufferofs = 0;

	bufferofs = PAGE1START+(200-STATUSLINES)*SCREENWIDTH;
	LatchDrawPic (x,y,picnum);
	bufferofs = PAGE2START+(200-STATUSLINES)*SCREENWIDTH;
	LatchDrawPic (x,y,picnum);
	bufferofs = PAGE3START+(200-STATUSLINES)*SCREENWIDTH;
	LatchDrawPic (x,y,picnum);

	bufferofs = temp;
}


/*
==================
=
= DrawFace
=
==================
*/

void DrawFace (void)
{
	if (gamestate.health)
	{
		#ifdef SPEAR
		if (godmode)
			StatusDrawPic (17,4,Pgod+gamestate.faceframe);
		else
		#endif
		StatusDrawPic (17,4,Pface1+3*((100-gamestate.health)/16)+gamestate.faceframe);
	}
	else
	{
#ifndef SPEAR
	 if (LastAttacker->obclass == needleobj)
	   StatusDrawPic (17,4,MUTANTBJPIC);
	 else
#endif
	   StatusDrawPic (17,4,Pface8);
	}
}


/*
===============
=
= UpdateFace
=
= Calls draw face if time to change
=
===============
*/

#define FACETICS	70

s16int	facecount;

void	UpdateFace (void)
{

	if (SD_SoundPlaying() == Sgetgatling)
	  return;

	facecount += tics;
	if (facecount > US_RndT())
	{
		gamestate.faceframe = (US_RndT()>>6);
		if (gamestate.faceframe==3)
			gamestate.faceframe = 1;

		facecount = 0;
		DrawFace ();
	}
}



/*
===============
=
= LatchNumber
=
= right justifies and pads with blanks
=
===============
*/

void	LatchNumber (s16int x, s16int y, s16int width, s32int number)
{
	u16int	length,c;
	char	str[20];

	ltoa (number,str,10);

	length = strlen (str);

	while (length<width)
	{
		StatusDrawPic (x,y,Pblank);
		x++;
		width--;
	}

	c= length <= width ? 0 : length-width;

	while (c<length)
	{
		StatusDrawPic (x,y,str[c]-'0'+ Pn0);
		x++;
		c++;
	}
}


/*
===============
=
= DrawHealth
=
===============
*/

void	DrawHealth (void)
{
	LatchNumber (21,16,3,gamestate.health);
}


/*
===============
=
= TakeDamage
=
===============
*/

void	TakeDamage (s16int points,objtype *attacker)
{
	LastAttacker = attacker;

	if (gamestate.victoryflag)
		return;
	if (gamestate.difficulty==gd_baby)
	  points>>=2;

	if (!godmode)
		gamestate.health -= points;

	if (gamestate.health<=0)
	{
		gamestate.health = 0;
		playstate = ex_died;
		killerobj = attacker;
	}

	StartDamageFlash (points);

	gotgatgun=0;

	DrawHealth ();
	DrawFace ();

	//
	// MAKE BJ'S EYES BUG IF MAJOR DAMAGE!
	//
	#ifdef SPEAR
	if (points > 30 && gamestate.health!=0 && !godmode)
	{
		StatusDrawPic (17,4,Pouch);
		facecount = 0;
	}
	#endif

}


/*
===============
=
= HealSelf
=
===============
*/

void	HealSelf (s16int points)
{
	gamestate.health += points;
	if (gamestate.health>100)
		gamestate.health = 100;

	DrawHealth ();
	gotgatgun = 0;	// JR
	DrawFace ();
}


//===========================================================================


/*
===============
=
= DrawLevel
=
===============
*/

void	DrawLevel (void)
{
#ifdef SPEAR
	if (gamestate.mapon == 20)
		LatchNumber (2,16,2,18);
	else
#endif
	LatchNumber (2,16,2,gamestate.mapon+1);
}

//===========================================================================


/*
===============
=
= DrawLives
=
===============
*/

void	DrawLives (void)
{
	LatchNumber (14,16,1,gamestate.lives);
}


/*
===============
=
= GiveExtraMan
=
===============
*/

void	GiveExtraMan (void)
{
	if (gamestate.lives<9)
		gamestate.lives++;
	DrawLives ();
	SD_PlaySound (S1up);
}

//===========================================================================

/*
===============
=
= DrawScore
=
===============
*/

void	DrawScore (void)
{
	LatchNumber (6,16,6,gamestate.score);
}

/*
===============
=
= GivePoints
=
===============
*/

void	GivePoints (s32int points)
{
	gamestate.score += points;
	while (gamestate.score >= gamestate.nextextra)
	{
		gamestate.nextextra += EXTRAPOINTS;
		GiveExtraMan ();
	}
	DrawScore ();
}

//===========================================================================

/*
==================
=
= DrawWeapon
=
==================
*/

void DrawWeapon (void)
{
	StatusDrawPic (32,8,Pknife+gamestate.weapon);
}


/*
==================
=
= DrawKeys
=
==================
*/

void DrawKeys (void)
{
	if (gamestate.keys & 1)
		StatusDrawPic (30,4,Pgkey);
	else
		StatusDrawPic (30,4,Pnokey);

	if (gamestate.keys & 2)
		StatusDrawPic (30,20,Pskey);
	else
		StatusDrawPic (30,20,Pnokey);
}



/*
==================
=
= GiveWeapon
=
==================
*/

void GiveWeapon (s16int weapon)
{
	GiveAmmo (6);

	if (gamestate.bestweapon<weapon)
		gamestate.bestweapon = gamestate.weapon
		= gamestate.chosenweapon = weapon;

	DrawWeapon ();
}


//===========================================================================

/*
===============
=
= DrawAmmo
=
===============
*/

void	DrawAmmo (void)
{
	LatchNumber (27,16,2,gamestate.ammo);
}


/*
===============
=
= GiveAmmo
=
===============
*/

void	GiveAmmo (s16int ammo)
{
	if (!gamestate.ammo)				// knife was out
	{
		if (!gamestate.attackframe)
		{
			gamestate.weapon = gamestate.chosenweapon;
			DrawWeapon ();
		}
	}
	gamestate.ammo += ammo;
	if (gamestate.ammo > 99)
		gamestate.ammo = 99;
	DrawAmmo ();
}

//===========================================================================

/*
==================
=
= GiveKey
=
==================
*/

void GiveKey (s16int key)
{
	gamestate.keys |= (1<<key);
	DrawKeys ();
}



/*
=============================================================================

							MOVEMENT

=============================================================================
*/


/*
===================
=
= GetBonus
=
===================
*/
void GetBonus (statobj_t *check)
{
	switch (check->itemnumber)
	{
	case	bo_firstaid:
		if (gamestate.health == 100)
			return;

		SD_PlaySound (Shealth2);
		HealSelf (25);
		break;

	case	bo_key1:
	case	bo_key2:
	case	bo_key3:
	case	bo_key4:
		GiveKey (check->itemnumber - bo_key1);
		SD_PlaySound (Sgetkey);
		break;

	case	bo_cross:
		SD_PlaySound (Sbonus1);
		GivePoints (100);
		gamestate.treasurecount++;
		break;
	case	bo_chalice:
		SD_PlaySound (Sbonus2);
		GivePoints (500);
		gamestate.treasurecount++;
		break;
	case	bo_bible:
		SD_PlaySound (Sbonus3);
		GivePoints (1000);
		gamestate.treasurecount++;
		break;
	case	bo_crown:
		SD_PlaySound (Sbonus4);
		GivePoints (5000);
		gamestate.treasurecount++;
		break;

	case	bo_clip:
		if (gamestate.ammo == 99)
			return;

		SD_PlaySound (Sgetammo);
		GiveAmmo (8);
		break;
	case	bo_clip2:
		if (gamestate.ammo == 99)
			return;

		SD_PlaySound (Sgetammo);
		GiveAmmo (4);
		break;

#ifdef SPEAR
	case	bo_25clip:
		if (gamestate.ammo == 99)
		  return;

		SD_PlaySound (Sammobox);
		GiveAmmo (25);
		break;
#endif

	case	bo_machinegun:
		SD_PlaySound (Sgetmg);
		GiveWeapon (wp_machinegun);
		break;
	case	bo_chaingun:
		SD_PlaySound (Sgetgatling);
		GiveWeapon (wp_chaingun);

		StatusDrawPic (17,4,Pgat);
		facecount = 0;
		gotgatgun = 1;
		break;

	case	bo_fullheal:
		SD_PlaySound (S1up);
		HealSelf (99);
		GiveAmmo (25);
		GiveExtraMan ();
		gamestate.treasurecount++;
		break;

	case	bo_food:
		if (gamestate.health == 100)
			return;

		SD_PlaySound (Shealth1);
		HealSelf (10);
		break;

	case	bo_alpo:
		if (gamestate.health == 100)
			return;

		SD_PlaySound (Shealth1);
		HealSelf (4);
		break;

	case	bo_gibs:
		if (gamestate.health >10)
			return;

		SD_PlaySound (Sslurp);
		HealSelf (1);
		break;

	case	bo_spear:
		spearflag = true;
		spearx = player->x;
		speary = player->y;
		spearangle = player->angle;
		playstate = ex_completed;
	}

	StartBonusFlash ();
	check->shapenum = -1;			// remove from list
}


/*
===================
=
= TryMove
=
= returns true if move ok
= debug: use pointers to optimize
===================
*/

int TryMove (objtype *ob)
{
	s16int			xl,yl,xh,yh,x,y;
	objtype		*check;
	s32int		deltax,deltay;

	xl = (ob->x-PLAYERSIZE) >>TILESHIFT;
	yl = (ob->y-PLAYERSIZE) >>TILESHIFT;

	xh = (ob->x+PLAYERSIZE) >>TILESHIFT;
	yh = (ob->y+PLAYERSIZE) >>TILESHIFT;

//
// check for solid walls
//
	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = actorat[x][y];
			if (check && check<objlist)
				return false;
		}

//
// check for actors
//
	if (yl>0)
		yl--;
	if (yh<MAPSIZE-1)
		yh++;
	if (xl>0)
		xl--;
	if (xh<MAPSIZE-1)
		xh++;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = actorat[x][y];
			if (check > objlist
			&& (check->flags & FL_SHOOTABLE) )
			{
				deltax = ob->x - check->x;
				if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
					continue;
				deltay = ob->y - check->y;
				if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
					continue;

				return false;
			}
		}

	return true;
}


/*
===================
=
= ClipMove
=
===================
*/

void ClipMove (objtype *ob, s32int xmove, s32int ymove)
{
	s32int	basex,basey;

	basex = ob->x;
	basey = ob->y;

	ob->x = basex+xmove;
	ob->y = basey+ymove;
	if (TryMove (ob))
		return;

	if (noclip && ob->x > 2*TILEGLOBAL && ob->y > 2*TILEGLOBAL &&
	ob->x < (((s32int)(mapwidth-1))<<TILESHIFT)
	&& ob->y < (((s32int)(mapheight-1))<<TILESHIFT) )
		return;		// walk through walls

	if (!SD_SoundPlaying())
		SD_PlaySound (Shitwall);

	ob->x = basex+xmove;
	ob->y = basey;
	if (TryMove (ob))
		return;

	ob->x = basex;
	ob->y = basey+ymove;
	if (TryMove (ob))
		return;

	ob->x = basex;
	ob->y = basey;
}

//==========================================================================

/*
===================
=
= VictoryTile
=
===================
*/

void VictoryTile (void)
{
#ifndef SPEAR
	SpawnBJVictory ();
#endif

	gamestate.victoryflag = true;
}


/*
===================
=
= Thrust
=
===================
*/

void Thrust (s16int angle, s32int speed)
{
	s32int xmove,ymove;
	s32int	slowmax;
	u16int	offset;


	//
	// ZERO FUNNY COUNTER IF MOVED!
	//
	#ifdef SPEAR
	if (speed)
		funnyticount = 0;
	#endif

	thrustspeed += speed;
//
// moving bounds speed
//
	if (speed >= MINDIST*2)
		speed = MINDIST*2-1;

	xmove = FixedByFrac(speed,costable[angle]);
	ymove = -FixedByFrac(speed,sintable[angle]);

	ClipMove(player,xmove,ymove);

	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;

	offset = farmapylookup[player->tiley]+player->tilex;
	player->areanumber = *(mapsegs[0] + offset) -AREATILE;

	if (*(mapsegs[1] + offset) == EXITTILE)
		VictoryTile ();
}


/*
=============================================================================

								ACTIONS

=============================================================================
*/


/*
===============
=
= Cmd_Fire
=
===============
*/

void Cmd_Fire (void)
{
	buttonheld[bt_attack] = true;

	gamestate.weaponframe = 0;

	player->state = &s_attack;

	gamestate.attackframe = 0;
	gamestate.attackcount =
		attackinfo[gamestate.weapon][gamestate.attackframe].tics;
	gamestate.weaponframe =
		attackinfo[gamestate.weapon][gamestate.attackframe].frame;
}

//===========================================================================

/*
===============
=
= Cmd_Use
=
===============
*/

void Cmd_Use (void)
{
	objtype 	*check;
	s16int			checkx,checky,doornum,dir;
	int		elevatorok;


//
// find which cardinal direction the player is facing
//
	if (player->angle < ANGLES/8 || player->angle > 7*ANGLES/8)
	{
		checkx = player->tilex + 1;
		checky = player->tiley;
		dir = di_east;
		elevatorok = true;
	}
	else if (player->angle < 3*ANGLES/8)
	{
		checkx = player->tilex;
		checky = player->tiley-1;
		dir = di_north;
		elevatorok = false;
	}
	else if (player->angle < 5*ANGLES/8)
	{
		checkx = player->tilex - 1;
		checky = player->tiley;
		dir = di_west;
		elevatorok = true;
	}
	else
	{
		checkx = player->tilex;
		checky = player->tiley + 1;
		dir = di_south;
		elevatorok = false;
	}

	doornum = tilemap[checkx][checky];
	if (*(mapsegs[1]+farmapylookup[checky]+checkx) == PUSHABLETILE)
	{
	//
	// pushable wall
	//

		PushWall (checkx,checky,dir);
		return;
	}
	if (!buttonheld[bt_use] && doornum == ELEVATORTILE && elevatorok)
	{
	//
	// use elevator
	//
		buttonheld[bt_use] = true;

		tilemap[checkx][checky]++;		// flip switch
		if (*(mapsegs[0]+farmapylookup[player->tiley]+player->tilex) == ALTELEVATORTILE)
			playstate = ex_secretlevel;
		else
			playstate = ex_completed;
		SD_PlaySound (Slvlend);
		SD_WaitSoundDone();
	}
	else if (!buttonheld[bt_use] && doornum & 0x80)
	{
		buttonheld[bt_use] = true;
		OperateDoor (doornum & ~0x80);
	}
	else
		SD_PlaySound (Snope);

}

/*
=============================================================================

						   PLAYER CONTROL

=============================================================================
*/



/*
===============
=
= SpawnPlayer
=
===============
*/

void SpawnPlayer (s16int tilex, s16int tiley, s16int dir)
{
	player->obclass = playerobj;
	player->active = true;
	player->tilex = tilex;
	player->tiley = tiley;
	player->areanumber =
		*(mapsegs[0] + farmapylookup[player->tiley]+player->tilex);
	player->x = ((s32int)tilex<<TILESHIFT)+TILEGLOBAL/2;
	player->y = ((s32int)tiley<<TILESHIFT)+TILEGLOBAL/2;
	player->state = &s_player;
	player->angle = (1-dir)*90;
	if (player->angle<0)
		player->angle += ANGLES;
	player->flags = FL_NEVERMARK;
	Thrust (0,0);				// set some variables

	InitAreas ();
}


//===========================================================================

/*
===============
=
= T_KnifeAttack
=
= Update player hands, and try to do damage when the proper frame is reached
=
===============
*/

void	KnifeAttack (objtype *ob)
{
	objtype *check,*closest;
	s32int	dist;

	SD_PlaySound (Sknife);
// actually fire
	dist = 0x7fffffff;
	closest = NULL;
	for (check=ob->next ; check ; check=check->next)
		if ( (check->flags & FL_SHOOTABLE)
		&& (check->flags & FL_VISABLE)
		&& abs (check->viewx-centerx) < shootdelta
		)
		{
			if (check->transx < dist)
			{
				dist = check->transx;
				closest = check;
			}
		}

	if (!closest || dist> 0x18000l)
	{
	// missed

		return;
	}

// hit something
	DamageActor (closest,US_RndT() >> 4);
}



void	GunAttack (objtype *ob)
{
	objtype *check,*closest,*oldclosest;
	s16int		damage;
	s16int		dx,dy,dist;
	s32int	viewdist;

	switch (gamestate.weapon)
	{
	case wp_pistol:
		SD_PlaySound (Spistol);
		break;
	case wp_machinegun:
		SD_PlaySound (Smg);
		break;
	case wp_chaingun:
		SD_PlaySound (Sgatling);
		break;
	}

	madenoise = true;

//
// find potential targets
//
	viewdist = 0x7fffffffl;
	closest = NULL;

	while (1)
	{
		oldclosest = closest;

		for (check=ob->next ; check ; check=check->next)
			if ( (check->flags & FL_SHOOTABLE)
			&& (check->flags & FL_VISABLE)
			&& abs (check->viewx-centerx) < shootdelta
			)
			{
				if (check->transx < viewdist)
				{
					viewdist = check->transx;
					closest = check;
				}
			}

		if (closest == oldclosest)
			return;						// no more targets, all missed

	//
	// trace a line from player to enemey
	//
		if (CheckLine(closest))
			break;

	}

//
// hit something
//
	dx = abs(closest->tilex - player->tilex);
	dy = abs(closest->tiley - player->tiley);
	dist = dx>dy ? dx:dy;

	if (dist<2)
		damage = US_RndT() / 4;
	else if (dist<4)
		damage = US_RndT() / 6;
	else
	{
		if ( (US_RndT() / 12) < dist)		// missed
			return;
		damage = US_RndT() / 6;
	}

	DamageActor (closest,damage);
}

//===========================================================================

/*
===============
=
= VictorySpin
=
===============
*/

void VictorySpin (void)
{
	s32int	desty;

	if (player->angle > 270)
	{
		player->angle -= tics * 3;
		if (player->angle < 270)
			player->angle = 270;
	}
	else if (player->angle < 270)
	{
		player->angle += tics * 3;
		if (player->angle > 270)
			player->angle = 270;
	}

	desty = (((s32int)player->tiley-5)<<TILESHIFT)-0x3000;

	if (player->y > desty)
	{
		player->y -= tics*4096;
		if (player->y < desty)
			player->y = desty;
	}
}


//===========================================================================

/*
===============
=
= T_Attack
=
===============
*/

void	T_Attack (objtype *ob)
{
	struct	atkinf	*cur;

	UpdateFace ();

	if (gamestate.victoryflag)		// watching the BJ actor
	{
		VictorySpin ();
		return;
	}

	if ( buttonstate[bt_use] && !buttonheld[bt_use] )
		buttonstate[bt_use] = false;

	if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
		buttonstate[bt_attack] = false;

	ControlMovement (ob);
	if (gamestate.victoryflag)		// watching the BJ actor
		return;

	plux = player->x >> UNSIGNEDSHIFT;			// scale to fit in u16int
	pluy = player->y >> UNSIGNEDSHIFT;
	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;

//
// change frame and fire
//
	gamestate.attackcount -= tics;
	while (gamestate.attackcount <= 0)
	{
		cur = &attackinfo[gamestate.weapon][gamestate.attackframe];
		switch (cur->attack)
		{
		case -1:
			ob->state = &s_player;
			if (!gamestate.ammo)
			{
				gamestate.weapon = wp_knife;
				DrawWeapon ();
			}
			else
			{
				if (gamestate.weapon != gamestate.chosenweapon)
				{
					gamestate.weapon = gamestate.chosenweapon;
					DrawWeapon ();
				}
			};
			gamestate.attackframe = gamestate.weaponframe = 0;
			return;

		case 4:
			if (!gamestate.ammo)
				break;
			if (buttonstate[bt_attack])
				gamestate.attackframe -= 2;
		case 1:
			if (!gamestate.ammo)
			{	// can only happen with chain gun
				gamestate.attackframe++;
				break;
			}
			GunAttack (ob);
			gamestate.ammo--;
			DrawAmmo ();
			break;

		case 2:
			KnifeAttack (ob);
			break;

		case 3:
			if (gamestate.ammo && buttonstate[bt_attack])
				gamestate.attackframe -= 2;
			break;
		}

		gamestate.attackcount += cur->tics;
		gamestate.attackframe++;
		gamestate.weaponframe =
			attackinfo[gamestate.weapon][gamestate.attackframe].frame;
	}

}



//===========================================================================

/*
===============
=
= T_Player
=
===============
*/

void	T_Player (objtype *ob)
{
	if (gamestate.victoryflag)		// watching the BJ actor
	{
		VictorySpin ();
		return;
	}

	UpdateFace ();
	CheckWeaponChange ();

	if ( buttonstate[bt_use] )
		Cmd_Use ();

	if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
		Cmd_Fire ();

	ControlMovement (ob);
	if (gamestate.victoryflag)		// watching the BJ actor
		return;


	plux = player->x >> UNSIGNEDSHIFT;			// scale to fit in u16int
	pluy = player->y >> UNSIGNEDSHIFT;
	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;
}


