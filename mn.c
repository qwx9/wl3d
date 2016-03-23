#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

typedef struct Score Score;
struct Score{
	char name[58];
	int n;
	int lvl;
	int ep;
};
static Score score[] = {
	{"id software-'92", 10000, 1},
	{"Adrian Carmack", 10000, 1},
	{"John Carmack", 10000, 1},
	{"Kevin Cloud", 10000, 1},
	{"Tom Hall", 10000, 1},
	{"John Romero", 10000, 1},
	{"Jay Wilbur", 10000, 1},
};

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
fixedw(char *s)
{
	char c;

	while(c = *s, c != 0)
		*s++ = c - '0' + 129;
}

static void
wlscores(void)
{
	int x, y;
	char a[16], b[16];
	Score *s;

	clear();
	stripe(10);
	pic(48, 0, pict[Pscores]);
	pic(32, 68, pict[Pname]);
	pic(160, 68, pict[Plvl]);
	pic(224, 68, pict[Phigh]);

	fnt = fnts;
	for(s=score, y=76; s<score+nelem(score); s++, y+=16){
		txt(32, y, s->name, 0xf);

		sprint(a, "%d", s->lvl);
		fixedw(a);
		x = 176 - txtw(a);
		if(ver < WL1){
			sprint(b, "E%d/L", s->ep+1);
			x += txt(x-6, y, b, 0xf) - 6;
		}
		txt(x, y, a, 0xf);

		sprint(a, "%d", s->n);
		fixedw(a);
		txt(264 - txtw(a), y, a, 0xf);
	}
}
static void
sdscores(void)
{
	int y;
	char a[16];
	Score *s;

	clear();
	pic(0, 0, pict[Pscores]);

	fnt = fnts+1;
	for(s=score, y=76; s<score+nelem(score); s++, y+=16){
		txt(16, y, s->name, 0x13);

		if(s->lvl == 21)
			pic(176, y-1, pict[Pspear]);
		else{
			sprint(a, "%d", s->lvl);
			txt(194 - txtw(a), y, a, 0xf);
		}

		sprint(a, "%d", s->n);
		txt(292 - txtw(a), y, a, 0xf);
	}
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
