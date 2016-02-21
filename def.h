#include "ID_HEADS.H"
#include <MATH.H>
#include <VALUES.H>

#include "WL_MENU.H"

#include "FOREIGN.H"

#ifdef SPEAR
#include "F_SPEAR.H"
#endif

/*
=============================================================================

							MACROS

=============================================================================
*/


#define COLORBORDER(color)		asm{mov	dx,STATUS_REGISTER_1;in al,dx;\
	mov dx,ATR_INDEX;mov al,ATR_OVERSCAN;out dx,al;mov al,color;out	dx,al;\
	mov	al,32;out dx,al};

#define MAPSPOT(x,y,plane)		(*(mapsegs[plane]+farmapylookup[y]+x))

#define SIGN(x) 	((x)>0?1:-1)
#define ABS(x) 		((s16int)(x)>0?(x):-(x))
#define LABS(x) 	((s32int)(x)>0?(x):-(x))

/*
=============================================================================

						 GLOBAL CONSTANTS

=============================================================================
*/

#define MAXACTORS		150				// max number of nazis, etc / map
#define MAXSTATS		400				// max number of lamps, bonus, etc
#define MAXDOORS		64				// max number of sliding doors
#define MAXWALLTILES	64				// max number of wall tiles

//
// tile constants
//

#define	ICONARROWS		90
#define PUSHABLETILE	98
#define EXITTILE		99				// at end of castle
#define AREATILE		107				// first of NUMAREAS floor tiles
#define NUMAREAS		37
#define ELEVATORTILE	21
#define AMBUSHTILE		106
#define	ALTELEVATORTILE	107

#define NUMBERCHARS	9


//----------------

#define EXTRAPOINTS		40000

#define PLAYERSPEED		3000
#define RUNSPEED   		6000

#define	SCREENSEG		0xa000

#define SCREENBWIDE		80

#define HEIGHTRATIO		0.50		// also defined in id_mm.c

#define BORDERCOLOR	3
#define FLASHCOLOR	5
#define FLASHTICS	4


#define PLAYERSIZE		MINDIST			// player radius
#define MINACTORDIST	0x10000l		// minimum dist from player center
										// to any actor center

#define NUMLATCHPICS	100


#define PI	3.141592657

#define GLOBAL1		(1l<<16)
#define TILEGLOBAL  GLOBAL1
#define PIXGLOBAL	(GLOBAL1/64)
#define TILESHIFT		16l
#define UNSIGNEDSHIFT	8

#define ANGLES		360					// must be divisable by 4
#define ANGLEQUAD	(ANGLES/4)
#define FINEANGLES	3600
#define ANG90		(FINEANGLES/4)
#define ANG180		(ANG90*2)
#define ANG270		(ANG90*3)
#define ANG360		(ANG90*4)
#define VANG90		(ANGLES/4)
#define VANG180		(VANG90*2)
#define VANG270		(VANG90*3)
#define VANG360		(VANG90*4)

#define MINDIST		(0x5800l)


#define	MAXSCALEHEIGHT	256				// largest scale on largest view

#define MAXVIEWWIDTH		320

#define MAPSIZE		64					// maps are 64*64 max
#define NORTH	0
#define EAST	1
#define SOUTH	2
#define WEST	3


#define STATUSLINES		40

#define SCREENSIZE		(SCREENBWIDE*208)
#define PAGE1START		0
#define PAGE2START		(SCREENSIZE)
#define PAGE3START		(SCREENSIZE*2u)
#define	FREESTART		(SCREENSIZE*3u)


#define PIXRADIUS		512

#define STARTAMMO		8


// object flag values

#define FL_SHOOTABLE	1
#define FL_BONUS		2
#define FL_NEVERMARK	4
#define FL_VISABLE		8
#define FL_ATTACKMODE	16
#define FL_FIRSTATTACK	32
#define FL_AMBUSH		64
#define FL_NONMARK		128


//
// sprite constants
//

enum	{
		SPR_DEMO,
		SPR_DEATHCAM,
//
// static sprites
//
		SPR_STAT_0,SPR_STAT_1,SPR_STAT_2,SPR_STAT_3,
		SPR_STAT_4,SPR_STAT_5,SPR_STAT_6,SPR_STAT_7,

		SPR_STAT_8,SPR_STAT_9,SPR_STAT_10,SPR_STAT_11,
		SPR_STAT_12,SPR_STAT_13,SPR_STAT_14,SPR_STAT_15,

		SPR_STAT_16,SPR_STAT_17,SPR_STAT_18,SPR_STAT_19,
		SPR_STAT_20,SPR_STAT_21,SPR_STAT_22,SPR_STAT_23,

		SPR_STAT_24,SPR_STAT_25,SPR_STAT_26,SPR_STAT_27,
		SPR_STAT_28,SPR_STAT_29,SPR_STAT_30,SPR_STAT_31,

		SPR_STAT_32,SPR_STAT_33,SPR_STAT_34,SPR_STAT_35,
		SPR_STAT_36,SPR_STAT_37,SPR_STAT_38,SPR_STAT_39,

		SPR_STAT_40,SPR_STAT_41,SPR_STAT_42,SPR_STAT_43,
		SPR_STAT_44,SPR_STAT_45,SPR_STAT_46,SPR_STAT_47,

#ifdef SPEAR
		SPR_STAT_48,SPR_STAT_49,SPR_STAT_50,SPR_STAT_51,
#endif

//
// guard
//
		SPR_GRD_S_1,SPR_GRD_S_2,SPR_GRD_S_3,SPR_GRD_S_4,
		SPR_GRD_S_5,SPR_GRD_S_6,SPR_GRD_S_7,SPR_GRD_S_8,

		SPR_GRD_W1_1,SPR_GRD_W1_2,SPR_GRD_W1_3,SPR_GRD_W1_4,
		SPR_GRD_W1_5,SPR_GRD_W1_6,SPR_GRD_W1_7,SPR_GRD_W1_8,

		SPR_GRD_W2_1,SPR_GRD_W2_2,SPR_GRD_W2_3,SPR_GRD_W2_4,
		SPR_GRD_W2_5,SPR_GRD_W2_6,SPR_GRD_W2_7,SPR_GRD_W2_8,

		SPR_GRD_W3_1,SPR_GRD_W3_2,SPR_GRD_W3_3,SPR_GRD_W3_4,
		SPR_GRD_W3_5,SPR_GRD_W3_6,SPR_GRD_W3_7,SPR_GRD_W3_8,

		SPR_GRD_W4_1,SPR_GRD_W4_2,SPR_GRD_W4_3,SPR_GRD_W4_4,
		SPR_GRD_W4_5,SPR_GRD_W4_6,SPR_GRD_W4_7,SPR_GRD_W4_8,

		SPR_GRD_PAIN_1,SPR_GRD_DIE_1,SPR_GRD_DIE_2,SPR_GRD_DIE_3,
		SPR_GRD_PAIN_2,SPR_GRD_DEAD,

		SPR_GRD_SHOOT1,SPR_GRD_SHOOT2,SPR_GRD_SHOOT3,

//
// dogs
//
		SPR_DOG_W1_1,SPR_DOG_W1_2,SPR_DOG_W1_3,SPR_DOG_W1_4,
		SPR_DOG_W1_5,SPR_DOG_W1_6,SPR_DOG_W1_7,SPR_DOG_W1_8,

		SPR_DOG_W2_1,SPR_DOG_W2_2,SPR_DOG_W2_3,SPR_DOG_W2_4,
		SPR_DOG_W2_5,SPR_DOG_W2_6,SPR_DOG_W2_7,SPR_DOG_W2_8,

		SPR_DOG_W3_1,SPR_DOG_W3_2,SPR_DOG_W3_3,SPR_DOG_W3_4,
		SPR_DOG_W3_5,SPR_DOG_W3_6,SPR_DOG_W3_7,SPR_DOG_W3_8,

		SPR_DOG_W4_1,SPR_DOG_W4_2,SPR_DOG_W4_3,SPR_DOG_W4_4,
		SPR_DOG_W4_5,SPR_DOG_W4_6,SPR_DOG_W4_7,SPR_DOG_W4_8,

		SPR_DOG_DIE_1,SPR_DOG_DIE_2,SPR_DOG_DIE_3,SPR_DOG_DEAD,
		SPR_DOG_JUMP1,SPR_DOG_JUMP2,SPR_DOG_JUMP3,



//
// ss
//
		SPR_SS_S_1,SPR_SS_S_2,SPR_SS_S_3,SPR_SS_S_4,
		SPR_SS_S_5,SPR_SS_S_6,SPR_SS_S_7,SPR_SS_S_8,

		SPR_SS_W1_1,SPR_SS_W1_2,SPR_SS_W1_3,SPR_SS_W1_4,
		SPR_SS_W1_5,SPR_SS_W1_6,SPR_SS_W1_7,SPR_SS_W1_8,

		SPR_SS_W2_1,SPR_SS_W2_2,SPR_SS_W2_3,SPR_SS_W2_4,
		SPR_SS_W2_5,SPR_SS_W2_6,SPR_SS_W2_7,SPR_SS_W2_8,

		SPR_SS_W3_1,SPR_SS_W3_2,SPR_SS_W3_3,SPR_SS_W3_4,
		SPR_SS_W3_5,SPR_SS_W3_6,SPR_SS_W3_7,SPR_SS_W3_8,

		SPR_SS_W4_1,SPR_SS_W4_2,SPR_SS_W4_3,SPR_SS_W4_4,
		SPR_SS_W4_5,SPR_SS_W4_6,SPR_SS_W4_7,SPR_SS_W4_8,

		SPR_SS_PAIN_1,SPR_SS_DIE_1,SPR_SS_DIE_2,SPR_SS_DIE_3,
		SPR_SS_PAIN_2,SPR_SS_DEAD,

		SPR_SS_SHOOT1,SPR_SS_SHOOT2,SPR_SS_SHOOT3,

//
// mutant
//
		SPR_MUT_S_1,SPR_MUT_S_2,SPR_MUT_S_3,SPR_MUT_S_4,
		SPR_MUT_S_5,SPR_MUT_S_6,SPR_MUT_S_7,SPR_MUT_S_8,

		SPR_MUT_W1_1,SPR_MUT_W1_2,SPR_MUT_W1_3,SPR_MUT_W1_4,
		SPR_MUT_W1_5,SPR_MUT_W1_6,SPR_MUT_W1_7,SPR_MUT_W1_8,

		SPR_MUT_W2_1,SPR_MUT_W2_2,SPR_MUT_W2_3,SPR_MUT_W2_4,
		SPR_MUT_W2_5,SPR_MUT_W2_6,SPR_MUT_W2_7,SPR_MUT_W2_8,

		SPR_MUT_W3_1,SPR_MUT_W3_2,SPR_MUT_W3_3,SPR_MUT_W3_4,
		SPR_MUT_W3_5,SPR_MUT_W3_6,SPR_MUT_W3_7,SPR_MUT_W3_8,

		SPR_MUT_W4_1,SPR_MUT_W4_2,SPR_MUT_W4_3,SPR_MUT_W4_4,
		SPR_MUT_W4_5,SPR_MUT_W4_6,SPR_MUT_W4_7,SPR_MUT_W4_8,

		SPR_MUT_PAIN_1,SPR_MUT_DIE_1,SPR_MUT_DIE_2,SPR_MUT_DIE_3,
		SPR_MUT_PAIN_2,SPR_MUT_DIE_4,SPR_MUT_DEAD,

		SPR_MUT_SHOOT1,SPR_MUT_SHOOT2,SPR_MUT_SHOOT3,SPR_MUT_SHOOT4,

//
// officer
//
		SPR_OFC_S_1,SPR_OFC_S_2,SPR_OFC_S_3,SPR_OFC_S_4,
		SPR_OFC_S_5,SPR_OFC_S_6,SPR_OFC_S_7,SPR_OFC_S_8,

		SPR_OFC_W1_1,SPR_OFC_W1_2,SPR_OFC_W1_3,SPR_OFC_W1_4,
		SPR_OFC_W1_5,SPR_OFC_W1_6,SPR_OFC_W1_7,SPR_OFC_W1_8,

		SPR_OFC_W2_1,SPR_OFC_W2_2,SPR_OFC_W2_3,SPR_OFC_W2_4,
		SPR_OFC_W2_5,SPR_OFC_W2_6,SPR_OFC_W2_7,SPR_OFC_W2_8,

		SPR_OFC_W3_1,SPR_OFC_W3_2,SPR_OFC_W3_3,SPR_OFC_W3_4,
		SPR_OFC_W3_5,SPR_OFC_W3_6,SPR_OFC_W3_7,SPR_OFC_W3_8,

		SPR_OFC_W4_1,SPR_OFC_W4_2,SPR_OFC_W4_3,SPR_OFC_W4_4,
		SPR_OFC_W4_5,SPR_OFC_W4_6,SPR_OFC_W4_7,SPR_OFC_W4_8,

		SPR_OFC_PAIN_1,SPR_OFC_DIE_1,SPR_OFC_DIE_2,SPR_OFC_DIE_3,
		SPR_OFC_PAIN_2,SPR_OFC_DIE_4,SPR_OFC_DEAD,

		SPR_OFC_SHOOT1,SPR_OFC_SHOOT2,SPR_OFC_SHOOT3,

#ifndef SPEAR
//
// ghosts
//
		SPR_BLINKY_W1,SPR_BLINKY_W2,SPR_PINKY_W1,SPR_PINKY_W2,
		SPR_CLYDE_W1,SPR_CLYDE_W2,SPR_INKY_W1,SPR_INKY_W2,

//
// hans
//
		SPR_BOSS_W1,SPR_BOSS_W2,SPR_BOSS_W3,SPR_BOSS_W4,
		SPR_BOSS_SHOOT1,SPR_BOSS_SHOOT2,SPR_BOSS_SHOOT3,SPR_BOSS_DEAD,

		SPR_BOSS_DIE1,SPR_BOSS_DIE2,SPR_BOSS_DIE3,

//
// schabbs
//
		SPR_SCHABB_W1,SPR_SCHABB_W2,SPR_SCHABB_W3,SPR_SCHABB_W4,
		SPR_SCHABB_SHOOT1,SPR_SCHABB_SHOOT2,

		SPR_SCHABB_DIE1,SPR_SCHABB_DIE2,SPR_SCHABB_DIE3,SPR_SCHABB_DEAD,
		SPR_HYPO1,SPR_HYPO2,SPR_HYPO3,SPR_HYPO4,

//
// fake
//
		SPR_FAKE_W1,SPR_FAKE_W2,SPR_FAKE_W3,SPR_FAKE_W4,
		SPR_FAKE_SHOOT,SPR_FIRE1,SPR_FIRE2,

		SPR_FAKE_DIE1,SPR_FAKE_DIE2,SPR_FAKE_DIE3,SPR_FAKE_DIE4,
		SPR_FAKE_DIE5,SPR_FAKE_DEAD,

//
// hitler
//
		SPR_MECHA_W1,SPR_MECHA_W2,SPR_MECHA_W3,SPR_MECHA_W4,
		SPR_MECHA_SHOOT1,SPR_MECHA_SHOOT2,SPR_MECHA_SHOOT3,SPR_MECHA_DEAD,

		SPR_MECHA_DIE1,SPR_MECHA_DIE2,SPR_MECHA_DIE3,

		SPR_HITLER_W1,SPR_HITLER_W2,SPR_HITLER_W3,SPR_HITLER_W4,
		SPR_HITLER_SHOOT1,SPR_HITLER_SHOOT2,SPR_HITLER_SHOOT3,SPR_HITLER_DEAD,

		SPR_HITLER_DIE1,SPR_HITLER_DIE2,SPR_HITLER_DIE3,SPR_HITLER_DIE4,
		SPR_HITLER_DIE5,SPR_HITLER_DIE6,SPR_HITLER_DIE7,

//
// giftmacher
//
		SPR_GIFT_W1,SPR_GIFT_W2,SPR_GIFT_W3,SPR_GIFT_W4,
		SPR_GIFT_SHOOT1,SPR_GIFT_SHOOT2,

		SPR_GIFT_DIE1,SPR_GIFT_DIE2,SPR_GIFT_DIE3,SPR_GIFT_DEAD,
#endif
//
// Rocket, smoke and small explosion
//
		SPR_ROCKET_1,SPR_ROCKET_2,SPR_ROCKET_3,SPR_ROCKET_4,
		SPR_ROCKET_5,SPR_ROCKET_6,SPR_ROCKET_7,SPR_ROCKET_8,

		SPR_SMOKE_1,SPR_SMOKE_2,SPR_SMOKE_3,SPR_SMOKE_4,
		SPR_BOOM_1,SPR_BOOM_2,SPR_BOOM_3,

//
// Angel of Death's DeathSparks(tm)
//
#ifdef SPEAR
		SPR_HROCKET_1,SPR_HROCKET_2,SPR_HROCKET_3,SPR_HROCKET_4,
		SPR_HROCKET_5,SPR_HROCKET_6,SPR_HROCKET_7,SPR_HROCKET_8,

		SPR_HSMOKE_1,SPR_HSMOKE_2,SPR_HSMOKE_3,SPR_HSMOKE_4,
		SPR_HBOOM_1,SPR_HBOOM_2,SPR_HBOOM_3,

		SPR_SPARK1,SPR_SPARK2,SPR_SPARK3,SPR_SPARK4,
#endif

#ifndef SPEAR
//
// gretel
//
		SPR_GRETEL_W1,SPR_GRETEL_W2,SPR_GRETEL_W3,SPR_GRETEL_W4,
		SPR_GRETEL_SHOOT1,SPR_GRETEL_SHOOT2,SPR_GRETEL_SHOOT3,SPR_GRETEL_DEAD,

		SPR_GRETEL_DIE1,SPR_GRETEL_DIE2,SPR_GRETEL_DIE3,

//
// fat face
//
		SPR_FAT_W1,SPR_FAT_W2,SPR_FAT_W3,SPR_FAT_W4,
		SPR_FAT_SHOOT1,SPR_FAT_SHOOT2,SPR_FAT_SHOOT3,SPR_FAT_SHOOT4,

		SPR_FAT_DIE1,SPR_FAT_DIE2,SPR_FAT_DIE3,SPR_FAT_DEAD,

//
// bj
//
		SPR_BJ_W1,SPR_BJ_W2,SPR_BJ_W3,SPR_BJ_W4,
		SPR_BJ_JUMP1,SPR_BJ_JUMP2,SPR_BJ_JUMP3,SPR_BJ_JUMP4,
#else
//
// THESE ARE FOR 'SPEAR OF DESTINY'
//

//
// Trans Grosse
//
		SPR_TRANS_W1,SPR_TRANS_W2,SPR_TRANS_W3,SPR_TRANS_W4,
		SPR_TRANS_SHOOT1,SPR_TRANS_SHOOT2,SPR_TRANS_SHOOT3,SPR_TRANS_DEAD,

		SPR_TRANS_DIE1,SPR_TRANS_DIE2,SPR_TRANS_DIE3,

//
// Wilhelm
//
		SPR_WILL_W1,SPR_WILL_W2,SPR_WILL_W3,SPR_WILL_W4,
		SPR_WILL_SHOOT1,SPR_WILL_SHOOT2,SPR_WILL_SHOOT3,SPR_WILL_SHOOT4,

		SPR_WILL_DIE1,SPR_WILL_DIE2,SPR_WILL_DIE3,SPR_WILL_DEAD,

//
// UberMutant
//
		SPR_UBER_W1,SPR_UBER_W2,SPR_UBER_W3,SPR_UBER_W4,
		SPR_UBER_SHOOT1,SPR_UBER_SHOOT2,SPR_UBER_SHOOT3,SPR_UBER_SHOOT4,

		SPR_UBER_DIE1,SPR_UBER_DIE2,SPR_UBER_DIE3,SPR_UBER_DIE4,
		SPR_UBER_DEAD,

//
// Death Knight
//
		SPR_DEATH_W1,SPR_DEATH_W2,SPR_DEATH_W3,SPR_DEATH_W4,
		SPR_DEATH_SHOOT1,SPR_DEATH_SHOOT2,SPR_DEATH_SHOOT3,SPR_DEATH_SHOOT4,

		SPR_DEATH_DIE1,SPR_DEATH_DIE2,SPR_DEATH_DIE3,SPR_DEATH_DIE4,
		SPR_DEATH_DIE5,SPR_DEATH_DIE6,SPR_DEATH_DEAD,

//
// Ghost
//
		SPR_SPECTRE_W1,SPR_SPECTRE_W2,SPR_SPECTRE_W3,SPR_SPECTRE_W4,
		SPR_SPECTRE_F1,SPR_SPECTRE_F2,SPR_SPECTRE_F3,SPR_SPECTRE_F4,

//
// Angel of Death
//
		SPR_ANGEL_W1,SPR_ANGEL_W2,SPR_ANGEL_W3,SPR_ANGEL_W4,
		SPR_ANGEL_SHOOT1,SPR_ANGEL_SHOOT2,SPR_ANGEL_TIRED1,SPR_ANGEL_TIRED2,

		SPR_ANGEL_DIE1,SPR_ANGEL_DIE2,SPR_ANGEL_DIE3,SPR_ANGEL_DIE4,
		SPR_ANGEL_DIE5,SPR_ANGEL_DIE6,SPR_ANGEL_DIE7,SPR_ANGEL_DEAD,

#endif

//
// player attack frames
//
		SPR_KNIFEREADY,SPR_KNIFEATK1,SPR_KNIFEATK2,SPR_KNIFEATK3,
		SPR_KNIFEATK4,

		SPR_PISTOLREADY,SPR_PISTOLATK1,SPR_PISTOLATK2,SPR_PISTOLATK3,
		SPR_PISTOLATK4,

		SPR_MACHINEGUNREADY,SPR_MACHINEGUNATK1,SPR_MACHINEGUNATK2,MACHINEGUNATK3,
		SPR_MACHINEGUNATK4,

		SPR_CHAINREADY,SPR_CHAINATK1,SPR_CHAINATK2,SPR_CHAINATK3,
		SPR_CHAINATK4,

		};


/*
=============================================================================

						   GLOBAL TYPES

=============================================================================
*/

typedef enum {
	di_north,
	di_east,
	di_south,
	di_west
} controldir_t;

typedef enum {
	dr_normal,
	dr_lock1,
	dr_lock2,
	dr_lock3,
	dr_lock4,
	dr_elevator
} door_t;

typedef enum {
	ac_badobject = -1,
	ac_no,
	ac_yes,
	ac_allways
} activetype;

typedef enum {
	nothing,
	playerobj,
	inertobj,
	guardobj,
	officerobj,
	ssobj,
	dogobj,
	bossobj,
	schabbobj,
	fakeobj,
	mechahitlerobj,
	mutantobj,
	needleobj,
	fireobj,
	bjobj,
	ghostobj,
	realhitlerobj,
	gretelobj,
	giftobj,
	fatobj,
	rocketobj,

	spectreobj,
	angelobj,
	transobj,
	uberobj,
	willobj,
	deathobj,
	hrocketobj,
	sparkobj
} classtype;

typedef enum {
	dressing,
	block,
	bo_gibs,
	bo_alpo,
	bo_firstaid,
	bo_key1,
	bo_key2,
	bo_key3,
	bo_key4,
	bo_cross,
	bo_chalice,
	bo_bible,
	bo_crown,
	bo_clip,
	bo_clip2,
	bo_machinegun,
	bo_chaingun,
	bo_food,
	bo_fullheal,
	bo_25clip,
	bo_spear
} stat_t;

typedef enum {
	east,
	northeast,
	north,
	northwest,
	west,
	southwest,
	south,
	southeast,
	nodir
} dirtype;


#define NUMENEMIES		22
typedef enum {
	en_guard,
	en_officer,
	en_ss,
	en_dog,
	en_boss,
	en_schabbs,
	en_fake,
	en_hitler,
	en_mutant,
	en_blinky,
	en_clyde,
	en_pinky,
	en_inky,
	en_gretel,
	en_gift,
	en_fat,
	en_spectre,
	en_angel,
	en_trans,
	en_uber,
	en_will,
	en_death
} enemy_t;


typedef struct	statestruct
{
	int	rotate;
	s16int		shapenum;			// a shapenum of -1 means get from ob->temp1
	s16int		tictime;
	void	(*think) (),(*action) ();
	struct	statestruct	*next;
} statetype;


//---------------------
//
// trivial actor structure
//
//---------------------

typedef struct statstruct
{
	u8int	tilex,tiley;
	u8int	*visspot;
	s16int		shapenum;			// if shapenum == -1 the obj has been removed
	u8int	flags;
	u8int	itemnumber;
} statobj_t;


//---------------------
//
// door actor structure
//
//---------------------

typedef struct doorstruct
{
	u8int	tilex,tiley;
	int	vertical;
	u8int	lock;
	enum	{dr_open,dr_closed,dr_opening,dr_closing}	action;
	s16int		ticcount;
} doorobj_t;


//--------------------
//
// thinking actor structure
//
//--------------------

typedef struct objstruct
{
	activetype	active;
	s16int			ticcount;
	classtype	obclass;
	statetype	*state;

	u8int		flags;				//	FL_SHOOTABLE, etc

	s32int		distance;			// if negative, wait for that door to open
	dirtype		dir;

	s32int 		x,y;
	u16int	tilex,tiley;
	u8int		areanumber;

	s16int	 		viewx;
	u16int	viewheight;
	s32int		transx,transy;		// in global coord

	s16int 		angle;
	s16int			hitpoints;
	s32int		speed;

	s16int			temp1,temp2,temp3;
	struct		objstruct	*next,*prev;
} objtype;


#define NUMBUTTONS	8
enum	{
	bt_nobutton=-1,
	bt_attack=0,
	bt_strafe,
	bt_run,
	bt_use,
	bt_readyknife,
	bt_readypistol,
	bt_readymachinegun,
	bt_readychaingun
};


#define NUMWEAPONS	5
typedef enum	{
	wp_knife,
	wp_pistol,
	wp_machinegun,
	wp_chaingun
} weapontype;


typedef enum	{
	gd_baby,
	gd_easy,
	gd_medium,
	gd_hard
};

//---------------
//
// gamestate structure
//
//---------------

typedef	struct
{
	s16int			difficulty;
	s16int			mapon;
	s32int		oldscore,score,nextextra;
	s16int			lives;
	s16int			health;
	s16int			ammo;
	s16int			keys;
	weapontype		bestweapon,weapon,chosenweapon;

	s16int			faceframe;
	s16int			attackframe,attackcount,weaponframe;

	s16int			episode,secretcount,treasurecount,killcount,
				secrettotal,treasuretotal,killtotal;
	s32int		TimeCount;
	s32int		killx,killy;
	int		victoryflag;		// set during victory animations
} gametype;


typedef	enum	{
	ex_stillplaying,
	ex_completed,
	ex_died,
	ex_warped,
	ex_resetgame,
	ex_loadedgame,
	ex_victorious,
	ex_abort,
	ex_demodone,
	ex_secretlevel
} exit_t;


/*
=============================================================================

						 WL_MAIN DEFINITIONS

=============================================================================
*/

extern	int		MS_CheckParm (char far *string);

extern	char		str[80],str2[20];
extern	s16int			tedlevelnum;
extern	int		tedlevel;
extern	int		nospr;
extern	int		IsA386;

extern	u8int far	*scalermemory;

extern	s32int		focallength;
extern	u16int	viewangles;
extern	u16int	screenofs;
extern	s16int		    viewwidth;
extern	s16int			viewheight;
extern	s16int			centerx;
extern	s16int			shootdelta;

extern	s16int			dirangle[9];

extern	int         startgame,loadedgame,virtualreality;
extern	s16int		mouseadjustment;
//
// math tables
//
extern	s16int			pixelangle[MAXVIEWWIDTH];
extern	s32int		far finetangent[FINEANGLES/4];
extern	s32int 		far sintable[],far *costable;

//
// derived constants
//
extern	s32int 	scale,maxslope;
extern	s32int	heightnumerator;
extern	s16int		minheightdiv;

extern	char	configname[13];



void		HelpScreens (void);
void		OrderingInfo (void);
void		TEDDeath(void);
void		Quit (char *error);
void 		CalcProjection (s32int focal);
int		SetViewSize (u16int width, u16int height);
void		NewGame (s16int difficulty,s16int episode);
void 		NewViewSize (s16int width);
int 	LoadTheGame(s16int file,s16int x,s16int y);
int		SaveTheGame(s16int file,s16int x,s16int y);
void 		ShowViewSize (s16int width);
void		ShutdownId (void);


/*
=============================================================================

						 WL_GAME DEFINITIONS

=============================================================================
*/


extern	int		ingame,fizzlein;
extern	u16int	latchpics[NUMLATCHPICS];
extern	gametype	gamestate;
extern	s16int			doornum;

extern	char		demoname[13];

extern	s32int		spearx,speary;
extern	u16int	spearangle;
extern	int		spearflag;


void 	DrawPlayBorder (void);
void 	ScanInfoPlane (void);
void	SetupGameLevel (void);
void 	NormalScreen (void);
void 	DrawPlayScreen (void);
void 	FizzleOut (void);
void 	GameLoop (void);
void ClearMemory (void);
void PlayDemo (s16int demonumber);
void RecordDemo (void);
void DrawAllPlayBorder (void);
void	DrawHighScores(void);
void DrawAllPlayBorderSides (void);


// JAB
#define	PlaySoundLocTile(s,tx,ty)	PlaySoundLocGlobal(s,(((s32int)(tx) << TILESHIFT) + (1L << (TILESHIFT - 1))),(((s32int)ty << TILESHIFT) + (1L << (TILESHIFT - 1))))
#define	PlaySoundLocActor(s,ob)		PlaySoundLocGlobal(s,(ob)->x,(ob)->y)
void	PlaySoundLocGlobal(u16int s,s32int gx,s32int gy);
void UpdateSoundLoc(void);


/*
=============================================================================

						 WL_PLAY DEFINITIONS

=============================================================================
*/

#ifdef SPEAR
extern	s32int		funnyticount;		// FOR FUNNY BJ FACE
#endif

extern	exit_t		playstate;

extern	int		madenoise;

extern	objtype 	objlist[MAXACTORS],*new,*obj,*player,*lastobj,
					*objfreelist,*killerobj;
extern	statobj_t	statobjlist[MAXSTATS],*laststatobj;
extern	doorobj_t	doorobjlist[MAXDOORS],*lastdoorobj;

extern	u16int	farmapylookup[MAPSIZE];
extern	u8int		*nearmapylookup[MAPSIZE];

extern	u8int		tilemap[MAPSIZE][MAPSIZE];	// wall values only
extern	u8int		spotvis[MAPSIZE][MAPSIZE];
extern	objtype		*actorat[MAPSIZE][MAPSIZE];

#define UPDATESIZE			(UPDATEWIDE*UPDATEHIGH)
extern	u8int		update[UPDATESIZE];

extern	int		singlestep,godmode,noclip;
extern	s16int			extravbls;

//
// control info
//
extern	int		mouseenabled,joystickenabled,joypadenabled,joystickprogressive;
extern	s16int			joystickport;
extern	s16int			dirscan[4];
extern	s16int			buttonscan[NUMBUTTONS];
extern	s16int			buttonmouse[4];
extern	s16int			buttonjoy[4];

extern	int		buttonheld[NUMBUTTONS];

extern	s16int			viewsize;

//
// curent user input
//
extern	s16int			controlx,controly;		// range from -100 to 100
extern	int		buttonstate[NUMBUTTONS];

extern	int		demorecord,demoplayback;
extern	char		far *demoptr, far *lastdemoptr;
extern	uchar *demobuffer;



void	InitRedShifts (void);
void 	FinishPaletteShifts (void);

void	CenterWindow(u16int w,u16int h);
void 	InitActorList (void);
void 	GetNewActor (void);
void 	RemoveObj (objtype *gone);
void 	PollControls (void);
void 	StopMusic(void);
void 	StartMusic(void);
void	PlayLoop (void);
void StartDamageFlash (s16int damage);
void StartBonusFlash (void);

/*
=============================================================================

							WL_INTER

=============================================================================
*/

void IntroScreen (void);
void PreloadGraphics(void);
void LevelCompleted (void);
void	CheckHighScore (s32int score,u16int other);
void Victory (void);
void ClearSplitVWB (void);


/*
=============================================================================

							WL_DEBUG

=============================================================================
*/

s16int DebugKeys (void);
void PicturePause (void);


/*
=============================================================================

						 WL_DRAW DEFINITIONS

=============================================================================
*/

extern	u16int screenloc[3];
extern	u16int freelatch;

extern	s32int 	lasttimecount;
extern	s32int 	frameon;
extern	int	fizzlein;

extern	u16int	wallheight[MAXVIEWWIDTH];

extern	s32int	tileglobal;
extern	s32int	focallength;
extern	s32int	mindist;

//
// math tables
//
extern	s16int			pixelangle[MAXVIEWWIDTH];
extern	s32int		far finetangent[FINEANGLES/4];
extern	s32int 		far sintable[],far *costable;

//
// derived constants
//
extern	s32int 	scale;
extern	s32int	heightnumerator,mindist;

//
// refresh variables
//
extern	s32int	viewx,viewy;			// the focal point
extern	s16int		viewangle;
extern	s32int	viewsin,viewcos;

extern	s32int		postsource;
extern	u16int	postx;
extern	u16int	postwidth;


extern	s16int		horizwall[],vertwall[];

extern	u16int	pwallpos;


s32int	FixedByFrac (s32int a, s32int b);
void	TransformActor (objtype *ob);
void	BuildTables (void);
void	ClearScreen (void);
s16int		CalcRotate (objtype *ob);
void	DrawScaleds (void);
void	CalcTics (void);
void	FixOfs (void);
void	ThreeDRefresh (void);
void  FarScalePost (void);

/*
=============================================================================

						 WL_STATE DEFINITIONS

=============================================================================
*/
#define TURNTICS	10
#define SPDPATROL	512
#define SPDDOG		1500


extern	dirtype opposite[9];
extern	dirtype diagonal[9][9];


void	InitHitRect (objtype *ob, u16int radius);
void	SpawnNewObj (u16int tilex, u16int tiley, statetype *state);
void	NewState (objtype *ob, statetype *state);

int TryWalk (objtype *ob);
void 	SelectChaseDir (objtype *ob);
void 	SelectDodgeDir (objtype *ob);
void	SelectRunDir (objtype *ob);
void	MoveObj (objtype *ob, s32int move);
int SightPlayer (objtype *ob);

void	KillActor (objtype *ob);
void	DamageActor (objtype *ob, u16int damage);

int CheckLine (objtype *ob);
int	CheckSight (objtype *ob);


/*
=============================================================================

						 WL_SCALE DEFINITIONS

=============================================================================
*/


#define COMPSCALECODESTART	(65*4)		// offset to start of code in comp scaler

typedef struct
{
	u16int	codeofs[65];
	u16int	width[65];
	u8int		code[];
}	t_compscale;

typedef struct
{
	u16int	leftpix,rightpix;
	u16int	dataofs[64];
// table data after dataofs[rightpix-leftpix+1]
}	t_compshape;


extern	t_compscale _seg *scaledirectory[MAXSCALEHEIGHT+1];
extern	s32int			fullscalefarcall[MAXSCALEHEIGHT+1];

extern	u8int		bitmasks1[8][8];
extern	u8int		bitmasks2[8][8];
extern	u16int	wordmasks[8][8];

extern	u8int		mapmasks1[4][8];
extern	u8int		mapmasks2[4][8];
extern	u8int		mapmasks3[4][8];

extern	s16int			maxscale,maxscaleshl2;

extern	int	insetupscaling;

void SetupScaling (s16int maxscaleheight);
void ScaleShape (s16int xcenter, s16int shapenum, u16int height);
void SimpleScaleShape (s16int xcenter, s16int shapenum, u16int height);

/*
=============================================================================

						 WL_AGENT DEFINITIONS

=============================================================================
*/

//
// player state info
//
extern	int		running;
extern	s32int		thrustspeed;
extern	u16int	plux,pluy;		// player coordinates scaled to u16int

extern	s16int			anglefrac;
extern	s16int			facecount;

void	SpawnPlayer (s16int tilex, s16int tiley, s16int dir);
void 	DrawFace (void);
void	DrawHealth (void);
void	TakeDamage (s16int points,objtype *attacker);
void	HealSelf (s16int points);
void	DrawLevel (void);
void	DrawLives (void);
void	GiveExtraMan (void);
void	DrawScore (void);
void	GivePoints (s32int points);
void	DrawWeapon (void);
void	DrawKeys (void);
void	GiveWeapon (s16int weapon);
void	DrawAmmo (void);
void	GiveAmmo (s16int ammo);
void	GiveKey (s16int key);
void	GetBonus (statobj_t *check);

void	Thrust (s16int angle, s32int speed);

/*
=============================================================================

						 WL_ACT1 DEFINITIONS

=============================================================================
*/

extern	doorobj_t	doorobjlist[MAXDOORS],*lastdoorobj;
extern	s16int			doornum;

extern	u16int	doorposition[MAXDOORS],pwallstate;

extern	u8int		far areaconnect[NUMAREAS][NUMAREAS];

extern	int		areabyplayer[NUMAREAS];

extern u16int	pwallstate;
extern u16int	pwallpos;			// amount a pushable wall has been moved (0-63)
extern u16int	pwallx,pwally;
extern s16int			pwalldir;


void InitDoorList (void);
void InitStaticList (void);
void SpawnStatic (s16int tilex, s16int tiley, s16int type);
void SpawnDoor (s16int tilex, s16int tiley, int vertical, s16int lock);
void MoveDoors (void);
void MovePWalls (void);
void OpenDoor (s16int door);
void PlaceItemType (s16int itemtype, s16int tilex, s16int tiley);
void PushWall (s16int checkx, s16int checky, s16int dir);
void OperateDoor (s16int door);
void InitAreas (void);

/*
=============================================================================

						 WL_ACT2 DEFINITIONS

=============================================================================
*/

#define s_nakedbody s_static10

extern	statetype s_grddie1;
extern	statetype s_dogdie1;
extern	statetype s_ofcdie1;
extern	statetype s_mutdie1;
extern	statetype s_ssdie1;
extern	statetype s_bossdie1;
extern	statetype s_schabbdie1;
extern	statetype s_fakedie1;
extern	statetype s_mechadie1;
extern	statetype s_hitlerdie1;
extern	statetype s_greteldie1;
extern	statetype s_giftdie1;
extern	statetype s_fatdie1;

extern	statetype s_spectredie1;
extern	statetype s_angeldie1;
extern	statetype s_transdie0;
extern	statetype s_uberdie0;
extern	statetype s_willdie1;
extern	statetype s_deathdie1;


extern	statetype s_grdchase1;
extern	statetype s_dogchase1;
extern	statetype s_ofcchase1;
extern	statetype s_sschase1;
extern	statetype s_mutchase1;
extern	statetype s_bosschase1;
extern	statetype s_schabbchase1;
extern	statetype s_fakechase1;
extern	statetype s_mechachase1;
extern	statetype s_gretelchase1;
extern	statetype s_giftchase1;
extern	statetype s_fatchase1;

extern	statetype s_spectrechase1;
extern	statetype s_angelchase1;
extern	statetype s_transchase1;
extern	statetype s_uberchase1;
extern	statetype s_willchase1;
extern	statetype s_deathchase1;

extern	statetype s_blinkychase1;
extern	statetype s_hitlerchase1;

extern	statetype s_grdpain;
extern	statetype s_grdpain1;
extern	statetype s_ofcpain;
extern	statetype s_ofcpain1;
extern	statetype s_sspain;
extern	statetype s_sspain1;
extern	statetype s_mutpain;
extern	statetype s_mutpain1;

extern	statetype s_deathcam;

extern	statetype s_schabbdeathcam2;
extern	statetype s_hitlerdeathcam2;
extern	statetype s_giftdeathcam2;
extern	statetype s_fatdeathcam2;

void SpawnStand (enemy_t which, s16int tilex, s16int tiley, s16int dir);
void SpawnPatrol (enemy_t which, s16int tilex, s16int tiley, s16int dir);
void KillActor (objtype *ob);

void	US_ControlPanel(u8int);

void SpawnDeadGuard (s16int tilex, s16int tiley);
void SpawnBoss (s16int tilex, s16int tiley);
void SpawnGretel (s16int tilex, s16int tiley);
void SpawnTrans (s16int tilex, s16int tiley);
void SpawnUber (s16int tilex, s16int tiley);
void SpawnWill (s16int tilex, s16int tiley);
void SpawnDeath (s16int tilex, s16int tiley);
void SpawnAngel (s16int tilex, s16int tiley);
void SpawnSpectre (s16int tilex, s16int tiley);
void SpawnGhosts (s16int which, s16int tilex, s16int tiley);
void SpawnSchabbs (s16int tilex, s16int tiley);
void SpawnGift (s16int tilex, s16int tiley);
void SpawnFat (s16int tilex, s16int tiley);
void SpawnFakeHitler (s16int tilex, s16int tiley);
void SpawnHitler (s16int tilex, s16int tiley);

/*
=============================================================================

						 WL_TEXT DEFINITIONS

=============================================================================
*/

extern	void	HelpScreens(void);
extern	void	EndText(void);
