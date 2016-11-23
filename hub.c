#include <u.h>
#include <libc.h>
#include <thread.h>
#include <keyboard.h>
#include "dat.h"
#include "fns.h"

extern Channel *csc;

int mtc;

typedef struct Score Score;
typedef struct Seq Seq;
typedef struct Item Item;
typedef struct Menu Menu;

struct Score{
	char name[58];
	int n;
	int lvl;
	int ep;
};
static Score sc[] = {
	{"id software-'92", 10000, 1},
	{"Adrian Carmack", 10000, 1},
	{"John Carmack", 10000, 1},
	{"Kevin Cloud", 10000, 1},
	{"Tom Hall", 10000, 1},
	{"John Romero", 10000, 1},
	{"Jay Wilbur", 10000, 1},
};

enum{
	Lload,
	Lintro,
	Ltitle,
	Lcreds,
	Lscore,
	Ldemo,
	Lcam,
	Linter,
	Lwin,
	Ldecay,
	Linctl,
	Lctl,
	Lcur,
	Lesc,
	Lback,
	Lwait,
	Lsfxwait,
	Lack,
	Lmscore,
	Lpants,
	Lquit,
	Ldie
};
struct Seq{
	int dt;
	void (*f)(void);
};
struct Item{
	char *s;
	int c;
	Menu *m;
};
struct Menu{
	void (*init)(void);
	Seq *qs;
	Seq *qe;
	Menu *m;
	Col *c;
	Item *is;
	Item *ie;
	Item *ip;
	int cx;
	int cy;
	int cur;
};
static Menu *mp, ml[];
static Seq *mqp;

static char *ends[] = {
	"Dost thou wish to\nleave with such hasty\nabandon?",
	"Chickening out...\nalready?",
	"Press N for more carnage.\nPress Y to be a weenie.",
	"So, you think you can\nquit this easily, huh?",
	"Press N to save the world.\nPress Y to abandon it in\nits hour of need.",
	"Press N if you are brave.\nPress Y to cower in shame.",
	"Heroes, press N.\nWimps, press Y.",
	"You are at an intersection.\nA sign says, 'Press Y to quit.'\n>",
	"For guns and glory, press N.\nFor work and worry, press Y.",

	"Heroes don't quit, but\ngo ahead and press Y\nif you aren't one.",
	"Press Y to quit,\nor press N to enjoy\nmore violent diversion.",
	"Depressing the Y key means\nyou must return to the\nhumdrum workday world.",
	"Hey, quit or play,\nY or N:\nit's your choice.",
	"Sure you don't want to\nwaste a few more\nproductive hours?",
	"I think you had better\nplay some more. Please\npress N...please?",
	"If you are tough, press N.\nIf not, press Y daintily.",
	"I'm thinkin' that\nyou might wanna press N\nto play more. You do it.",
	"Sure. Fine. Quit.\nSee if we care.\nGet it over with.\nPress Y."
};
static char **quits;

enum{
	Dbg,
	Doff,
	Dbrd,
	Dbrd2,
	Dend
};
static int mcol[Dend] = {[Dbg] 0x2d, 0};

static uchar **demd;
static char *demf;

static void (*mclear)(void);
static void (*stripe)(int);

static void
wlmclear(void)
{
	put(0, 0, Vw, Vh, 0x29);
}
static void
sdmclear(void)
{
	pic(0, 0, pict[Pbackdrop]);
}

static void
wlstripe(int y)
{
	put(0, y, Vw, 24, 0);
	put(0, y+22, 320, 1, 0x2c);
}
static void
sdstripe(int y)
{
	put(0, y, Vw, 22, 0);
	put(0, y+23, 320, 1, 0);
}

static void
outbox(int x, int y, int dx, int dy, int c1, int c2)
{
	put(x, y, dx, 1, c2);
	put(x, y+1, 1, dy-1, c2);
	put(x, y+dy, dx+1, 1, c1);
	put(x+dx, y, 1, dy, c1);
}

static void
box(int x, int y, int dx, int dy, int col, int out, int out2)
{
	put(x+1, y+1, dx-1, dy-1, col);
	outbox(x, y, dx, dy, out, out2);
}

static void
viewbox(void)
{
	int x, y;

	x = Vhud - vw.dx / 2 - 1;
	y = (Vhud - vw.dy) / 2 - 1;
	put(0, 0, 320, Vhud, 0x7f);
	box(x, y, vw.dx+1, vw.dy+1, 0, 0x7d, 0);
	put(x, y+vw.dy+1, 1, 1, 0x7c);
}

static void
view(void)
{
	viewbox();
	pic(0, Vhud, pict[Pstat]);
	hudf();
	hudh();
	hudl();
	hudm();
	huda();
	hudk();
	hudw();
	hudp();
}

static void
fixedw(char *s)
{
	char c;

	while(c = *s, c != 0)
		*s++ = c - '0' + 129;
}

static void
reset(Menu *m)
{
	Seq *q;

	q = m->qs;
	mqp = q;
	mtc = 0;
	if(m != mp){
		toss();
		if(q->f != fadeout && mp != ml+Lpants)
			pal = pals[C0];
		mp = m;
		if(m->init != nil)
			m->init();
		if(m->c != nil)
			fadeop(m->c, q->dt);
	}
}

static void
blink(void)
{
	Menu *m;

	m = mp;
	if(m == ml+Lctl){
		put(m->cx, m->cy, 24, 16, mcol[Dbg]);
		pic(m->cx, m->cy, pict[Pcur1]+m->cur);
	}else if(m == ml+Lquit){
		if(m->cur == 0)
			txt(m->cx, m->cy, "_", 0);
		else
			put(m->cx, m->cy, fnt->w['_'], fnt->h, 0x17);
	}
	out();
	m->cur ^= 1;
}

static void
ask(void)
{
	Rune r;

	if(nbrecv(csc, &r) <= 0)
		return;
	if(r == 'y'){
		sfx(Sshoot);
		reset(ml+Ldie);
	}
	else if(r == 'n' || r == Kesc){
		sfx(Sesc);
		reset(ml+Lctl);
	}
}

static void
quit(void)
{
	int x, y, w, h, curw;
	char *s, *nl;

	s = quits[nrand(nelem(ends)/2)];
	h = txth(s);
	w = txtw(s);
	nl = strrchr(s, '\n');
	curw = txtw(nl != nil ? nl+1 : s);
	w = w > curw+10 ? w : curw+10;
	y = 200/2 - h/2;
	x = Vhud - w/2;

	box(x-5, y-5, w+10, h+10, 0x17, 0, 0x13);
	txtnl(x, y, s, 0);
	mp->cx = x+curw;
	mp->cy = y + h - fnt->h;
}

static void
ctl(void)
{
	Menu *m;
	Item *i, *s, *e;

	mclear();
	pic(112, 184, pict[Pmouselback]);
	stripe(10);
	pic(80, 0, pict[Popt]);
	box(68, 52, 178, 6+13*9, mcol[Dbg], mcol[Dbrd2], mcol[Doff]);

	fnt = fnts+1;
	m = ml+Lctl;
	s = i = m->is;
	e = m->ie;
	do
		txt(100, 55+13*(i-s), i->s, i->c);
	while(++i < e);
	m->cur = 0;
	m->cx = 72;
	m->cy = 53+13*(m->ip-s);
	if(mp == m)
		mus(Mmenu);
}

static void
cursfx(void)
{
	sfx(Sdrawgun2);
}

static void
movcur(Menu *m, Item *p, int dir)
{
	Item *i;

	p->c = 0x17;
	i = p;
	do{
		i += dir;
		if(i < m->is)
			i = m->ie-1;
		else if(i == m->ie)
			i = m->is;
	}while(i->c != 0x17);
	i->c = 0x13;
	m->ip = i;
	m->cur = 0;
	if(i != p+dir){
		cursfx();
		reset(ml+Lctl);
		ctl();
		return;
	}
	put(m->cx, m->cy, 24, 16, mcol[Dbg]);
	m->cy += dir * 6;
	blink();
	sfx(Sdrawgun1);
	reset(ml+Lcur);
}

static void
cwalk(void)
{
	Rune r;
	Menu *m;
	Item *i;
	static int p;

	if(nbrecv(csc, &r) <= 0)
		return;
	m = mp;
	i = m->ip;
	switch(r){
	case Kup: movcur(m, i, -1); break;
	case Kdown: movcur(m, i, 1); break;
	case Kesc: sfx(Sesc); reset(ml+Lquit); break;
	case 'i': p++; break;
	case 'd':
		if(ver == SOD && p == 1)
			reset(ml+Lpants);
		break;
	case '\n':
		m = i->m;
		if(m == nil)
			break;
		if(m != ml+Lquit && m != ml+Lesc)
			sfx(Sshoot);
		reset(m);
	}
	if(r != 'i')
		p = 0;
}

static void
inctl(void)
{
	Item *i;
	Menu *m;

	m = ml+Lctl;
	if(m->ip != nil)
		m->ip->c = 0x17;
	i = m->is;
	m->ip = i;
	i[0].c = 0x13;
	i[4].c = mcol[Doff];
	grab(0);
	ctl();
	stopsfx();
}

static void
skipstep(void)
{
	if(nbrecv(csc, nil) > 0){
		mqp++;
		mtc = 0;
	}
}

static void
skiploop(void)
{
	if(nbrecv(csc, nil) > 0)
		reset(ml+Ldecay);
}

static void
ack(void)
{
	if(nbrecv(csc, nil) > 0)
		reset(ml+Lback);
}

static void
swait(void)
{
	if(lastsfx() < 0){
		reset(ml+Linter);
		step = mstep;
	}
}

static void
pants(void)
{
	pic(0, 0, pict[Pid1]);
	pic(0, 80, pict[Pid2]);
	palpic(exts[Eid]);
	fadeop(mp->c, mp->qs->dt);
	mus(Mnazjazz);
}

static void
iwin(void)
{
}
static void
win(void)
{
}

static void
iscore(void)
{
}
static void
inter(void)
{
}

static void
gcont(void)
{
	step = gstep;
	gm.end = 0;
	gm.fizz = 0;
}

static void
camtxt2(void)
{
	put(0, 56, Vw, 16, 0x7f);
	viewbox();
}
static void
camtxt(void)
{
	fizzop(-1, 0);
	pictxt(0, 56, "LET\'S SEE THAT AGAIN!");
	out();
}

static void
indem(void)
{
	initg(0, *demd++);
	if(demd >= epis)
		demd = dems;
	view();
}

static void
score(void)
{
	int x, y;
	char a[16], b[16];
	Score *s;

	mclear();
	stripe(10);
	pic(48, 0, pict[Pscores]);
	pic(32, 68, pict[Pname]);
	pic(160, 68, pict[Plvl]);
	pic(224, 68, pict[Phigh]);

	fnt = fnts;
	for(s=sc, y=76; s<sc+nelem(sc); s++, y+=16){
		txt(32, y, s->name, 0xf);

		sprint(a, "%d", s->lvl);
		fixedw(a);
		x = 176 - txtw(a);
		if(ver == WL6){
			sprint(b, "E%d/L", s->ep+1);
			x += txt(x-6, y, b, 0xf) - 6;
		}
		txt(x, y, a, 0xf);

		sprint(a, "%d", s->n);
		fixedw(a);
		txt(264 - txtw(a), y, a, 0xf);
	}
	if(mp == ml+Lmscore)
		mus(Mroster);
}
static void
sdscore(void)
{
	int y;
	char a[16];
	Score *s;

	mclear();
	pic(0, 0, pict[Pscores]);

	fnt = fnts+1;
	for(s=sc, y=76; s<sc+nelem(sc); s++, y+=16){
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
	if(mp == ml+Lmscore)
		mus(Maward);
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
	mus(Mintro);
}
static void
sdtitle(void)
{
	pic(0, 0, pict[Ptitle1]);
	pic(0, 80, pict[Ptitle2]);
	palpic(exts[Etitpal]);
	mus(Mtower);
}

static void
intro(void)
{
	fill(0x82);
	pic(216, 110, pict[Ppg13]);
}

static void
die(void)
{
	threadexitsall(nil);
}

static Item ictl[] = {
	{"New Game", 0x17},
	{"Sound", 0x17},
	{"Control", 0x17},
	{"Load Game", 0x17},
	{"Save Game", 0x17},
	{"Change View", 0x17},
	{"View Scores", 0x17, ml+Lmscore},
	{"Back to Demo", 0x17, ml+Lesc},
	{"Quit", 0x17, ml+Lquit}
};

static Col fblk, fmenu = { 0xae, 0, 0 };
static Seq *mqp,
	introq[] = {{30, fadein}, {7*Tb, skiploop}, {30, fadeout}},
	titleq[] = {{30, fadein}, {15*Tb, skiploop}, {30, fadeout}},
	loopq[] = {{30, fadein}, {10*Tb, skiploop}, {30, fadeout}},
	scoreq[] = {{30, fadein}, {10*Tb, skiploop}, {30, fadeout}},
	demoq[] = {{30, fadein}, {1, demo}, {41, fizz}, {1, gcont}, {30, fadeout}},
	camq[] = {{100, nil}, {144, fizz}, {0, camtxt}, {300, skipstep}, {0, camtxt2}, {41, fizz}, {1, gcont}, {100, nil}, {30, fadeout}},
	interq[] = {{30, fadein}, {0, iscore}, {30, fadeout}},
	winq[] = {{30, fadein}, {0, iwin}, {30, fadeout}},
	decq[] = {{30, fadeout}},
	inctlq[] = {{10, fadein}},
	ctlq[] = {{0, blink}, {70, cwalk}, {0, blink}, {8, cwalk}},
	curq[] = {{8, nil}, {0, cursfx}},
	escq[] = {{10, fadeout}},
	backq[] = {{10, fadeout}, {0, ctl}, {10, fadein}},
	waitq[] = {{1, skiploop}},
	swaitq[] = {{1, swait}},
	ackq[] = {{1, ack}},
	mscoreq[] = {{10, fadeout}, {0, score}, {10, fadein}},
	pantsq[] = {{30, fadeout}, {0, pants}, {30, fadein}},
	quitq[] = {{0, blink}, {10, ask}},
	dieq[] = {{10, fadeout}, {1, die}};

static Menu *mp, ml[] = {
	[Lload] {nil, decq, decq+nelem(decq), ml+Lintro, &fblk},
	[Lintro] {intro, introq, introq+nelem(introq), ml+Ltitle, &fblk},
	[Ltitle] {title, titleq, titleq+nelem(titleq), ml+Lcreds, &fblk},
	[Lcreds] {creds, loopq, loopq+nelem(loopq), ml+Lscore, &fblk},
	[Lscore] {score, loopq, loopq+nelem(loopq), ml+Ldemo, &fblk},
	[Ldemo] {indem, demoq, demoq+nelem(demoq), nil, &fblk},
	[Lcam] {nil, camq, camq+nelem(camq), nil},
	[Linter] {inter, interq, interq+nelem(interq), nil, &fblk},
	[Lwin] {win, winq, winq+nelem(winq), nil, &fblk},
	[Ldecay] {nil, decq, decq+nelem(decq), ml+Linctl},
	[Linctl] {inctl, inctlq, inctlq+nelem(inctlq), ml+Lctl, &fblk},
	[Lctl] {ctl, ctlq, ctlq+nelem(ctlq), ml+Lctl, nil, ictl, ictl+nelem(ictl)},
	[Lcur] {nil, curq, curq+nelem(curq), ml+Lctl},
	[Lesc] {nil, escq, escq+nelem(escq), ml+Ltitle, &fblk},
	[Lback] {nil, backq, backq+nelem(backq), ml+Lctl, &fmenu},
	[Lwait] {nil, waitq, waitq+nelem(waitq), ml+Lwait},
	[Lsfxwait] {nil, swaitq, swaitq+nelem(swaitq), ml+Lsfxwait},
	[Lack] {nil, ackq, ackq+nelem(ackq), ml+Lack},
	[Lmscore] {nil, mscoreq, mscoreq+nelem(mscoreq), ml+Lack, &fmenu},
	[Lpants] {nil, pantsq, pantsq+nelem(pantsq), ml+Lwait, &fblk},
	[Lquit] {quit, quitq, quitq+nelem(quitq), ml+Lquit},
	[Ldie] {nil, dieq, dieq+nelem(dieq), nil, &fmenu}
};

static void
dend(void)
{
	gm.demo = gm.record = 0;
	pal = pals[Cfad];
	if(demf != nil){
		if(demexit)
			threadexitsall(nil);
		free(demf);
		demf = nil;
		demd = dems;
	}
}
void
gend(void)
{
	switch(gm.end){
	case EDfizz:
		fizzop(-1, 1);
		put((Vw - vw.dx) / 2, (Vhud - vw.dy) / 2, vw.dx, vw.dy, 0);
		out();
		break;
	enddem:
	case EDdem:
		dend();
		mp->m = ml+Ltitle;
		break;
	case EDcam:
		fizzop(0x7f, 1);
		reset(ml+Lcam);
		mp->m = gm.demo || gm.record ? ml+Ltitle : ml+Lwin;
		gm.fizz++;
		break;
	case EDcam2:
		if(gm.demo || gm.record)
			dend();
		else
			pal = pals[Cfad];
		break;
	case EDkey:
		dend();
		mp->m = ml+Linctl;
		break;
	case EDdie:
		if(gm.demo || gm.record)
			goto enddem;
		break;
	case EDup:
	case EDsetec:
	case EDwon:
		if(gm.demo || gm.record)
			goto enddem;
		mp->m = ml+Lsfxwait;
		break;
	}
	step = mstep;
}

void
mstep(void)
{
	Menu *m;
	Seq *q;

rep:
	m = mp;
	q = mqp;
	mtc += Δtc;
	if(q->f != nil)
		q->f();
	if(mtc >= q->dt){
		if(++mqp == m->qe)
			reset(m->m);
		mtc = 0;
	}
	if(q->dt == 0)
		goto rep;
}

void
init(char *f)
{
	tab();
	mclear = wlmclear;
	stripe = wlstripe;
	quits = ends;
	if(ver >= SDM){
		mclear = sdmclear;
		stripe = sdstripe;
		ml[Ltitle].init = sdtitle;
		ml[Lscore].init = sdscore;
		mscoreq[1].f = sdscore;
		fmenu = (Col){0, 0, 0xce};
		mcol[Dbg] = 0x9d;
		quits += nelem(ends)/2;
	}
	mcol[Doff] = mcol[Dbg] ^ 6;
	mcol[Dbrd] = mcol[Dbg] ^ 4;
	mcol[Dbrd2] = mcol[Dbg] ^ 14;
	demd = dems;
	reset(ml+Lload);
	setvw(15);
	if(f != nil){
		demf = demof(f);
		demd = (uchar **)&demf;
		mp->m = ml+Ldemo;
	}
	mus(ver<SDM ? Mintro : Mtower);
}
