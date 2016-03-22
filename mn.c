#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

static void (*clear)(void);
static void (*stripe)(int);
static void (*scores)(void);
static void (*title)(void);

static void
wlclear(void)
{
	put(0, 0, Vw, Vh, nil, 0x29);
}
static void
sdclear(void)
{
	pic(0, 0, Pbackdrop);
}

static void
wlstripe(int y)
{
	put(0, y, Vw, 24, nil, 0);
	put(0, y+22, 320, 1, nil, 0x2c);
}
static void
sdstripe(int y)
{
	put(0, y, Vw, 22, nil, 0);
	put(0, y+23, 320, 1, nil, 0);
}

static void
pants(void)
{
	fadeout(45);
	playmus(18);
	pic(0, 0, pict[Pid1]);
	pic(0, 80, pict[Pid2]);
	palpic(exts[Eid].p);
	fadein(30);
	/* wait for input */
	delay(15);
	fadeout(45);
	pal = pals[C0];
	/* draw main menu */
}

static void
wlscores(void)
{
	clear();
	stripe(10);
	pic(48, 0, pict[Pscores]);
	pic(4*8, 68, pict[Pname]);
	pic(20*8, 68, pict[Plvl]);
	pic(28*8, 68, pict[Phigh]);
}
static void
sdscores(void)
{
	clear();
	pic(0, 0, pict[Pscores]);
}

static void
wltitle(void)
{
	pic(0, 0, pict[Ptitle1]);
}
static void
sdtitle(void)
{
	pic(0, 0, pict[Ptitle1]);
	pic(0, 80, pict[Ptitle2]);
	palpic(exts[Etitpal].p);
}

static void
intro(void)
{
	fill(0x82);
	pic(216, 110, pict[Ppg13]);
	fadein(40);
	delay(7);
	fadeout(40);
}

void
init(void)
{
	if(ver < SDM){
		clear = wlclear;
		stripe = wlstripe;
		scores = wlscores;
		title = wltitle;
	}else{
		clear = sdclear;
		stripe = sdstripe;
		scores = sdscores;
		title = sdtitle;
	}
	if(!nointro)
		intro();
}

void
demos(void)
{
	for(;;){
		if(nointro)
			goto stop;
		title();
		fadein(30);
		delay(15);
		fadeout(40);
		pal = pals[C0];

		pic(0, 0, pict[Pcreds]);
		fadein(30);
		delay(10);
		fadeout(40);

		scores();
		fadein(30);
		delay(10);
		fadeout(40);

		continue;
	stop:
		break;
	}
}
