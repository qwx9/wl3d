#include <u.h>
#include <libc.h>
#include <thread.h>
#include <keyboard.h>
#include "dat.h"
#include "fns.h"

extern Channel *csc;

typedef struct Score Score;
typedef struct Menu Menu;
typedef struct Seq Seq;

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

struct Seq{
	int dt;
	void (*f)(void);
};
struct Menu{
	void (*draw)(void);
	int nq;
	Seq *sq;
	Menu *m;
};
static Menu *mp;
static Seq *msq;
static int tc;

static void waitkb(void);
static void menuk(void);
static void menu(void);
static void pants(void);
static void demo(void);
static void scores(void);
static void creds(void);
static void title(void);
static void intro(void);

static Seq menuq[] = {{1, menuk}};
static Seq mfadeq[] = {{30, fade}};
static Seq pantsq[] = {{30, fade}, {600*Tb, waitkb}, {30, fade}};
static Seq demoq[] = {{1, demo}};
static Seq loopq[] = {{30, fade}, {10*Tb, waitkb}, {30, fade}};
static Seq titleq[] = {{30, fade}, {15*Tb, waitkb}, {30, fade}};
static Seq introq[] = {{30, fade}, {7*Tb, waitkb}, {30, fade}};
enum{
	Lmenu,
	Lmfade,
	Lpants,
	Ldemo,
	Lscores,
	Lcreds,
	Ltitle,
	Lintro
};
static Menu ml[] = {
	[Lmenu] {menu, nelem(menuq), menuq, ml+Lmenu},
	[Lmfade] {nil, nelem(mfadeq), mfadeq, ml+Lmenu},
	[Lpants] {pants, nelem(pantsq), pantsq, ml+Lmenu},
	[Ldemo] {demo, nelem(demoq), demoq, ml+Ltitle},
	[Lscores] {scores, nelem(loopq), loopq, ml+Ldemo},
	[Lcreds] {creds, nelem(loopq), loopq, ml+Lscores},
	[Ltitle] {title, nelem(titleq), titleq, ml+Lcreds},
	[Lintro] {intro, nelem(introq), introq, ml+Ltitle},
};

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
fixedw(char *s)
{
	char c;

	while(c = *s, c != 0)
		*s++ = c - '0' + 129;
}

static void
reset(Menu *m, int pal0)
{
	mp = m;
	tc = 0;
	if(pal0)
		pal = pals[C0];
	msq = m->sq;
	if(mp->draw != nil){
		mp->draw();
		fadeop(msq->dt, 0, 0);
	}else
		fadeop(msq->dt, 0, 1);
}

static void
waitkb(void)
{
	if(nbrecv(csc, nil) > 0)
		reset(ml+Lmfade, 0);
}

static void
menuk(void)
{
	Rune r;

	while(nbrecv(csc, &r) != 0);
}

static void
pants(void)
{
	pic(0, 0, pict[Pid1]);
	pic(0, 80, pict[Pid2]);
	palpic(exts[Eid].p);
	playmus(Mnazjazz);
}

static void
menu(void)
{
	playmus(Mmenu);
	grab(0);
	step = estep;
}

static void
demo(void)
{
	step = dstep;
}

static void
scores(void)
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
creds(void)
{
	pic(0, 0, pict[Pcreds]);
}

static void
title(void)
{
	pic(0, 0, pict[Ptitle1]);
	playmus(Mintro);
}
static void
sdtitle(void)
{
	pic(0, 0, pict[Ptitle1]);
	pic(0, 80, pict[Ptitle2]);
	palpic(exts[Etitpal].p);
	playmus(Mtower);
}

static void
intro(void)
{
	fill(0x82);
	pic(216, 110, pict[Ppg13]);
	playmus(Mintro);
}
static void
sdintro(void)
{
	fill(0x82);
	pic(216, 110, pict[Ppg13]);
	playmus(Mtower);
}

int
estep(void)
{
	return -1;
}

int
mstep(void)
{
	tc++;
	msq->f();
	if(tc == msq->dt){
		if(++msq == mp->sq + mp->nq)
			reset(mp->m, 1);
		tc = 0;
	}
	return 0;
}

void
init(int nointro)
{
	clear = wlclear;
	stripe = wlstripe;
	if(ver >= SDM){
		clear = sdclear;
		stripe = sdstripe;
		ml[Lintro].draw = sdintro;
		ml[Ltitle].draw = sdtitle;
		ml[Lscores].draw = sdscores;
	}
	reset(nointro ? ml+Lmenu : ml+Lintro, 0);
	cson++;
}
