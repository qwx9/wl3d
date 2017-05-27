#include <u.h>
#include <libc.h>
#include <thread.h>
#include <keyboard.h>
#include "dat.h"
#include "fns.h"

extern Channel *csc;

int qtc;
Score scs[] = {
	{"id software-'92", 10000, 1},
	{"Adrian Carmack", 10000, 1},
	{"John Carmack", 10000, 1},
	{"Kevin Cloud", 10000, 1},
	{"Tom Hall", 10000, 1},
	{"John Romero", 10000, 1},
	{"Jay Wilbur", 10000, 1},
};
char savs[10][32];

typedef struct Fade Fade;
typedef struct Sp Sp;
typedef struct Seq Seq;
typedef struct Item Item;
typedef struct Menu Menu;

enum{
	LMctl,
	LMnew,
	LMdifc,
	LMsnd,
	LMin,
	LMsav,
	LMmsg,

	Lload = 0,
	Lintro,
	Lftitle,
	Ltitle,
	Lcreds,
	Lscore,
	Lfpants,
	Lpants,
	Ldemo,
	Lpsych,
	Lgame,
	Lretry,
	Lmsg,
	Lcont1,
	Lcont2,
	Lgcont,
	Lfdie,
	Ldie,
	Ldie2,
	Lhigh,
	Lcam,
	Lspear,
	Linter,
	Linteri,
	Linterm,
	Linterw,
	Lintere,
	Lsinter,
	Lsdmend,
	Lcolp,
	Lcolp2,
	Lcolp3,
	Lwon,
	Lwon2,
	Lsdepi,
	Lpres,
	Lroll1,
	Lroll2,
	Lroll3,
	Lroll4,
	Lroll5,
	Lroll6,
	Lroll7,
	Ldecay,
	Lctl,
	Ltoctl,
	Lftoctl,
	Lmtoctl,
	Lcur,
	Lftonew,
	Lmtonew,
	Lnewgame,
	Lnctl,
	Ldenied,
	Ldifc1,
	Ldifc2,
	Ldifc3,
	Ldifc4,
	Ldifc5,
	Lfsnd,
	Lmsnd,
	Lsctl,
	Lsndtog,
	Lfin,
	Lmin,
	Lictl,
	Lintog,
	Lfsens,
	Lmsens,
	Lsectl,
	Lfsav,
	Lmsav,
	Lsvctl,
	Lflod,
	Lmlod,
	Lldctl,
	Lfvw,
	Lmvw,
	Lvwctl,
	Lfmscore,
	Lmscore,
	Lend,
	Lcurgame,
	Lquit,
	Lmexit,
	Lexit
};
struct Sp{
	int dt;
	void (*f)(void);
};
struct Fade{
	Col c;
	int dt;
};
struct Seq{
	void (*init)(void);
	Sp *s;
	Sp *e;
	Seq *q;
	Fade *f;
};
static Seq *qsp, *qesc, ql[];
static Sp *qp;

enum{
	DMbg,
	DMoff,
	DMbrd,
	DMbrd2,
	DMend,

	DIreg = 0x17,
	DIsee = 0x4a,
	DIeps = 0x6b,
	DIrhi = 0x13,
	DIshi = 0x47,
	DIepi = 0xd0,
};
static int mcol[DMend] = {[DMbg] 0x2d, 0};

#define SEL(c)		((c) - 4 + (((c) >> 1 ^ (c)) >> 5 & 1))
#define UNSEL(c)	((c) + 4 - (((c) >> 1 ^ (c)) >> 5 & 1))

struct Item{
	char *s;
	int c;
	Seq *q;
	int a;
};
static Item
ictl[] = {
	{"New Game", SEL(DIreg), ql+Lftonew},
	{"Sound", DIreg, ql+Lfsnd},
	{"Control", DIreg, ql+Lfin},
	{"Load Game", DIreg, ql+Lflod},
	{"Save Game", DIreg, ql+Lfsav},
	{"Change View", DIreg, ql+Lfvw},
	{"View Scores", DIreg, ql+Lfmscore},
	{"Back to Demo", DIreg, ql+Lftitle},
	{"Quit", DIreg, ql+Lquit}
},
inew[] = {
	{"Episode 1\nEscape from Wolfenstein", SEL(DIreg), ql+Ldifc1},
	{nil},
	{"Episode 2\nOperation: Eisenfaust", DIreg, ql+Ldifc1},
	{nil},
	{"Episode 3\nDie, Fuhrer, Die!", DIreg, ql+Ldifc1},
	{nil},
	{"Episode 4\nA Dark Secret", DIreg, ql+Ldifc1},
	{nil},
	{"Episode 5\nTrail of the Madman", DIreg, ql+Ldifc1},
	{nil},
	{"Episode 6\nConfrontation", DIreg, ql+Ldifc1},
	{nil}
},
idifc[] = {
	{"Can I play, Daddy?", DIreg, ql+Ldifc4},
	{"Don't hurt me.", DIreg, ql+Ldifc4},
	{"Bring 'em on!", SEL(DIreg), ql+Ldifc4},
	{"I am Death incarnate!", DIreg, ql+Ldifc4}
},
isnd[] = {
	{"None", SEL(DIreg), ql+Lsndtog},
	{"PC Speaker"},
	{"AdLib/Sound Blaster", DIreg, nil, 1},
	{nil},
	{nil},
	{"None", DIreg, ql+Lsndtog},
	{"Disney Sound Source"},
	{"Sound Blaster", DIreg, nil, 1},
	{nil},
	{nil},
	{"None", DIreg, ql+Lsndtog},
	{"AdLib/Sound Blaster", DIreg, nil, 1}
},
iin[] = {
	{"Mouse Enabled", SEL(DIreg), ql+Lintog, 1},
	{"Autorun Enabled", DIreg, ql+Lintog},
	{"Mouse Sensitivity", DIreg, ql+Lfsens}
},
isv[] = {
	{"", SEL(DIreg), nil},
	{"", DIreg, nil},
	{"", DIreg, nil},
	{"", DIreg, nil},
	{"", DIreg, nil},
	{"", DIreg, nil},
	{"", DIreg, nil},
	{"", DIreg, nil},
	{"", DIreg, nil},
	{"", DIreg, nil}
},
imsg[] = {
	{nil}
};
struct Menu{
	Item *p;
	Item *s;
	Item *e;
	int x;
	int y;
	int n;
};
static Menu *mp, ml[] = {
	[LMctl] {ictl, ictl, ictl+nelem(ictl), 72},
	[LMnew] {inew, inew, inew+nelem(inew), 8},
	[LMdifc] {idifc+2, idifc, idifc+nelem(idifc), 48},
	[LMsnd] {isnd, isnd, isnd+nelem(isnd), 48},
	[LMin] {iin, iin, iin+nelem(iin), 24},
	[LMsav] {isv, isv, isv+nelem(isv), 80},
	[LMmsg] {imsg}
};

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

static Fade
	fblk = {{0, 0, 0}, 30},
	ftra = {{0, 0, 0}, 10},
	fctl = {{0xae, 0, 0}, 10},
	focl = {{0, 0x44, 0x44}, 300},
	ficl = {{0, 0x44, 0x44}, 30},
	fecl = {{0, 0x44, 0x44}, 5};

static int irr[4], *irp;
static int iri, iry, irb;

static uchar **demd;
static char *demf;

static void
reset(Seq *p)
{
	qp = p->s;
	qtc = 0;
	if(p == qsp)
		return;
	toss();
	if(qp->f != fadeout)
		pal = pals[C0];
	qsp = p;
	if(p->init != nil)
		p->init();
	if(p->f != nil)
		fadeop(&p->f->c, p->f->dt);
}

static void
pblink(void)
{
	if(mp->n == 0)
		txt(mp->x, mp->y, "_", 0);
	else
		put(mp->x, mp->y, fnt->w['_'], fnt->h, DIreg);
	out();
	mp->n ^= 1;
}

static void
blink(void)
{
	put(mp->x, mp->y, 24, 16, mcol[DMbg]);
	pic(mp->x, mp->y, pict[Pcur1] + mp->n);
	out();
	mp->n ^= 1;
}

static void
iswp(Item *on, Item *off)
{
	on->a = 1;
	on->q = nil;
	off->a = 0;
	off->q = ql+Lsndtog;
}

static void
toggle(void)
{
	Item *i;

	i = mp->s;
	switch(mp - ml){
	case LMsnd:
		if(mp->p->a)
			return;
		switch(mp->p - i){
		case 0: iswp(i, i+2); stopsfx(); sfxon = 0; break;
		case 2: iswp(i+2, i); sfxon++; sfx(Sshoot); break;
		case 5: iswp(i+5, i+7); pcmon = 0; break;
		case 7: iswp(i+7, i+5); pcmon++; break;
		case 10: iswp(i+10, i+11); stopmus(); muson = 0; sfx(Sshoot); break;
		case 11: iswp(i+11, i+10); muson++; mus(Mmenu); break;
		}
		break;
	case LMin:
		switch(mp->p - i){
		tog: mp->p->a ^= 1; break;
		case 0: grabon ^= 1; goto tog;
		case 1: autorun ^= 1; goto tog;
		}
		break;
	}
}

static void
inctl(void)
{
	Item *i, *e;
	Menu *m;

	m = ml+LMctl;
	i = m->s;
	i[4].c = mcol[DMoff];
	i[6].s = "View Scores";
	i[6].q = ql+Lfmscore;
	i[6].c = DIreg;
	i[7].s = "Back to Demo";
	i[7].q = ql+Lftitle;
	i[7].c = DIreg;
	m->p = m->s;
	m->p->c = SEL(DIreg);
	if(ver >= SDM)
		i[0].q = ql+Ldifc1;
	else
		for(i=ml[LMnew].s, e=ml[LMnew].e; i<e && i->q!=ql+Ldenied; i++)
			i->q = ql+Ldifc1;
}

static void
ingctl(void)
{
	Item *i, *e;

	i = ml[LMctl].s;
	i[4].c = DIreg;
	i[6].s = "End Game";
	i[6].q = ql+Lend;
	i[7].s = "Back to Game";
	i[7].q = ql+Lcont1;
	i[7].c = DIsee;
	if(i[0].q == ql+Ldifc1)
		i[0].q = ql+Lcurgame;
	else
		for(i=ml[LMnew].s, e=ml[LMnew].e; i<e && i->q!=ql+Ldenied; i++)
			i->q = ql+Lcurgame;
}

static void
mbox(int x, int y, int dx, int dy)
{
	box(x, y, dx, dy, mcol[DMbg], mcol[DMbrd2], mcol[DMoff]);
}

static void
msg(char *s, int o)
{
	int x, y, w, h, curw;
	char *nl;

	fnt = fnts+1;
	h = txth(s);
	w = txtw(s);
	nl = strrchr(s, '\n');
	curw = txtw(nl != nil ? nl + 1 : s);
	w = w > curw + 10 ? w : curw + 10;
	x = Vw / 2 - w / 2;
	y = (step == gstep ? Vhud : Vh) / 2 - h / 2;
	box(x - 5, y - 5, w + 10, h + 10, DIreg, 0, DIrhi);
	txtnl(x, y, s, 0);
	if(o)
		out();
	mp = ml+LMmsg;
	mp->n = 0;
	mp->x = x + curw;
	mp->y = y + h - fnt->h;
}

static void
quit(void)
{
	msg(quits[nrand(nelem(ends)/2)], 0);
	mp->p->q = ql+Lmexit;
	qesc = ql+Lctl;
}

static void
curgame(void)
{
	msg("You are currently in\na game. Continuing will\nerase old game. Ok?", 0);
	mp->p->q = ql+Ldifc1;
	qesc = ver < SDM ? ql+Lftoctl : ql+Lctl;
}

static void
mend2(void)
{
	view();
	render();
	gm.lives = 0;
	ql[Ldie].q = ql+Ldie2;
}

static void
mend(void)
{
	msg("Are you sure you want\nto end the game you\nare playing? (Y or N):", 0);
	mp->p->q = ql+Lfdie;
	qesc = ql+Lctl;
}

static void
denied(void)
{
	msg("Please select \"Read This!\"\nfrom the Options menu to\n"
		"find out how to order this\nepisode from Apogee.", 1);
	stopsfx();
	sfx(Snoway);
}

static void
sdmend(void)
{
	sfx(S1up);
	msg("This concludes your demo\nof Spear of Destiny! Now,\n"
		"go to your local software\nstore and buy it!", 1);
}

static void
setdifc(void)
{
	int m, d;

	m = ver < SDM ? (ml[LMnew].p - ml[LMnew].s) * 5 : 0;
	d = ml[LMdifc].p - ml[LMdifc].s;
	ginit(nil, m, d);
	ingctl();
	palfill(&fctl.c);
}

static void
difc(void)
{
	Menu *m;
	Item *i, *e;

	mclear();
	pic(112, 184, pict[Pmouselback]);
	if(ver < SDM){
		txt(70, 68, "How tough are you?", DIshi);
		qesc = ql+Lftonew;
	}else{
		pic(70, 68, pict[Pdiffc]);
		qesc = ql+Lftoctl;
	}
	mbox(45, 90, 225, 67);
	m = ml+LMdifc;
	mp = m;
	i = m->s;
	e = m->e;
	do{
		txt(74, 100 + 13 * (i - m->s), i->s, i->c);
	}while(++i < e);
	pic(235, 107, pict[Pbaby] + m->p - m->s);
	m->n = 0;
	m->y = 98 + 13 * (m->p - m->s);
}

static void
newgame(void)
{
	int n;
	Item *i, *e;
	Menu *m;

	mclear();
	pic(112, 184, pict[Pmouselback]);
	mbox(6, 19, 308, 162);
	txtcen(2, "Which episode to play?", DIshi);
	for(n=0; n<6; n++)
		pic(40, 23 + n * 26, pict[Pep1] + n);
	m = ml+LMnew;
	mp = m;
	i = m->s;
	e = m->e;
	do{
		txtnl(98, 23 + 13 * (i - m->s), i->s, i->c);
		i += 2;
	}while(i < e);
	qesc = ql+Lftoctl;
	m->n = 0;
	m->y = 21 + 13 * (m->p - m->s);
}

static void
snd(void)
{
	Item *i, *e;
	Menu *m;

	mclear();
	pic(112, 184, pict[Pmouselback]);
	mbox(40, 17, 250, 45);
	mbox(40, 82, 250, 45);
	mbox(40, 147, 250, 32);
	pic(96, 0, pict[Psfx]);
	pic(96, 65, pict[Ppcm]);
	pic(96, 130, pict[Pmus]);
	m = ml+LMsnd;
	mp = m;
	i = m->s;
	e = m->e;
	do{
		if(i->s == nil)
			continue;
		txt(100, 20 + 13 * (i - m->s), i->s, i->c);
		pic(72, 22 + 13 * (i - m->s), pict[i->a ? Psel : Punsel]);
	}while(++i < e);
	qesc = ql+Lftoctl;
	m->n = 0;
	m->y = 18 + 13 * (m->p - m->s);
}

static void
sens(void)
{
	int n;

	mclear();
	pic(112, 184, pict[Pmouselback]);
	mbox(10, 80, 300, 30);
	txtcen(82, "Adjust Mouse Sensitivity", DIsee);
	txt(14, 95, "Slow", DIreg);
	txt(269, 95, "Fast", DIreg);
	put(60, 97, 200, 10, DIreg);
	outbox(60, 97, 200, 10, 0x0, DIrhi);
	n = 60 + 20 * iri;
	put(n + 1, 98, 19, 9, DIshi);
	outbox(n, 97, 20, 10, 0x0, DIsee);
	qesc = ql+Lfin;
}

static void
in(void)
{
	Menu *m;
	Item *i, *e;

	mclear();
	stripe(10);
	pic(80, 0, pict[Pctl]);
	pic(112, 184, pict[Pmouselback]);
	mbox(16, 65, 284, 45);
	m = ml+LMin;
	mp = m;
	i = m->s;
	e = m->e;
	do{
		txt(80, 70 + 13 * (i - m->s), i->s, i->c);
		if(i < e - 1)
			pic(56, 73 + 13 * (i - m->s), pict[i->a ? Psel : Punsel]);
	}while(++i < e);
	qesc = ql+Lftoctl;
	m->n = 0;
	m->y = 68 + 13 * (m->p - m->s);
	iri = msense;
}

static void
savitem(int i, int c)
{
	outbox(109, 55 + i * 13, 136, 11, c, c);
	fnt = fnts;
	txt(111, 56 + i * 13, savs[i][0] == 0 ? "      - empty -" : savs[i], c);
	fnt = fnts+1;
}

static void
sav(void)
{
	Menu *m;
	Item *i, *e;

	mclear();
	pic(112, 184, pict[Pmouselback]);
	mbox(75, 50, 175, 140);
	stripe(10);
	pic(56, 0, pict[qsp->q == ql+Lsvctl ? Psave : Pload]);
	m = ml+LMsav;
	mp = m;
	i = m->s;
	e = m->e;
	do
		savitem(i - m->s, i->c);
	while(++i < e);
	qesc = ql+Lftoctl;
	m->n = 0;
	m->y = 53 + 13 * (m->p - m->s);
}

static void
mvw(void)
{
	int dx, dy;

	dx = vw.dx;
	dy = vw.dy;
	vw.dx = iri * 16;
	vw.dy = iri * 8;
	viewbox();
	vw.dx = dx;
	vw.dy = dy;
	put(0, Vhud, Vw, 40, 0x7f);
	txtcen(161, "Use arrows to size", DIrhi);
	txtcen(161 + fnt->h, "ENTER to accept", DIrhi);
	txtcen(161 + 2 * fnt->h, "ESC to cancel", DIrhi);
	qesc = ql+Lftoctl;
}

static void
ctl(void)
{
	Menu *m;
	Item *i, *e;

	mclear();
	pic(112, 184, pict[Pmouselback]);
	stripe(10);
	pic(80, 0, pict[Popt]);
	mbox(68, 52, 178, 123);
	fnt = fnts+1;
	m = ml+LMctl;
	mp = m;
	i = m->s;
	e = m->e;
	do
		txt(100, 55 + 13 * (i - m->s), i->s, i->c);
	while(++i < e);
	qesc = ql+Lquit;
	m->n = 0;
	m->y = 53 + 13 * (m->p - m->s);
	if(qsp == ql+Ltoctl)
		stopsfx();
	mus(Mmenu);
	grab(0);
	iri = vwsize;
}

static void
cursfx(void)
{
	sfx(Sdrawgun2);
}

static void
slcur(int dir)
{
	iri += dir;
	switch(qsp - ql){
	case Lsectl:
		if(iri < 0)
			iri = 0;
		else if(iri > 9)
			iri = 9;
		break;
	case Lvwctl:
		if(iri < 4)
			iri = 4;
		else if(iri > 19)
			iri = 19;
		break;
	}
	qsp->init();
	out();
}

static void
slider(void)
{
	Rune r;

	if(nbrecv(csc, &r) <= 0)
		return;
	switch(r){
	case Kleft: case Kdown: slcur(-1); break;
	case Kright: case Kup: slcur(1); break;
	case Kesc: sfx(Sesc); reset(qesc); break;
	case '\n':
	case ' ':
		switch(qsp - ql){
		case Lsectl: msense = iri; break;
		case Lvwctl: vwsize = iri; msg("Thinking...", 1); setvw(); break;
		}
		sfx(Sshoot);
		reset(qesc);
		break;
	}
}

static void
movcur(Item *p, int dir)
{
	Item *i;

	p->c = UNSEL(p->c);
	i = p;
	do{
		i += dir;
		if(i < mp->s)
			i = mp->e - 1;
		else if(i == mp->e)
			i = mp->s;
	}while(i->c == mcol[DMoff] || i->c == 0);
	i->c = SEL(i->c);
	mp->p = i;
	mp->n = 0;
	if(i != p + dir){
		cursfx();
		reset(qsp);
		if(qsp->init != nil)
			qsp->init();
		return;
	}
	put(mp->x, mp->y, 24, 16, mcol[DMbg]);
	mp->y += dir * 6;
	blink();
	sfx(Sdrawgun1);
	ql[Lcur].q = qsp;
	reset(ql+Lcur);
}

static void
cwalk(void)
{
	static char s[UTFmax*2], *sp = s;
	Rune r;
	Seq *q;
	Item *i;

	if(nbrecv(csc, &r) <= 0)
		return;
	i = mp->p;
	switch(r){
	case Kup: movcur(i, -1); break;
	case Kdown: movcur(i, 1); break;
	case Kesc: sfx(Sesc); reset(qesc); break;
	case '\n':
	case ' ':
		q = i->q;
		if(q == nil)
			break;
		if(q != ql+Lquit && q != ql+Lftitle)
			sfx(Sshoot);
		reset(q);
		break;
	default:
		if(ver < SOD)
			break;
		sp += runetochar(sp, &r);
		if(strncmp(s, "id", sp-s) == 0){
			if(sp - s == 2){
				reset(ql+Lfpants);
				sp = s;
			}
		}else
			sp = s;
	}
}

static void
ask(void)
{
	Rune r;

	if(nbrecv(csc, &r) <= 0)
		return;
	if(r == 'y'){
		sfx(Sshoot);
		reset(mp->p->q);
	}else if(r == 'n' || r == Kesc){
		sfx(Sesc);
		reset(qesc);
	}
}

static void
skip(void)
{
	if(nbrecv(csc, nil) > 0)
		reset(ql+Ldecay);
}
static void
swait(void)
{
	if(lastsfx() >= 0)
		qtc = 0;
	else
		sfxlck = 0;
}
static void
nbwait(void)
{
	if(nbrecv(csc, nil) > 0)
		qtc = qp->dt;
}
static void
bwait(void)
{
	qtc = nbrecv(csc, nil) > 0 ? qp->dt : 0;
}

static void
pants(void)
{
	pic(0, 0, pict[Pid1]);
	pic(0, 80, pict[Pid2]);
	palpic(exts[Eid]);
	mus(Mnazjazz);
}

static void
roll(void)
{
	pic(0, 0, pict[Pend1]+iri);
	palpic(exts[Eend1+iri]);
	iri++;
	if(iri == 9)
		iri = 1;
}
static void
pres3(void)
{
	put(0, 180, Vw, 20, 0);
	txtcen(180, "With the spear gone, the Allies will finally", DIepi);
	/* bug: typo "by able to" */
	txtcen(180 + fnt->h, "be able to destroy Hitler...", DIepi);
	out();
	iri = 3;
}
static void
pres2(void)
{
	txtcen(180, "We owe you a great debt, Mr. Blazkowicz.", DIepi);
	txtcen(180 + fnt->h, "You have served your country well.", DIepi);
	out();
}
static void
pres1(void)
{
	pic(0, 0, pict[Pend1]+2);
	palpic(exts[Eend1+2]);
	fnt = fnts;
}
static void
sdepi(void)
{
	pic(0, 0, pict[Pend1]);
	palpic(exts[Eend1]);
}

static void
won(void)
{
	int n;
	char a[10];

	put(0, 0, Vw, Vhud, 0x7f);
	pic(8, 4, pict[Pwin]);
	pictxt(144, 16, "YOU WIN!");
	pictxt(112, 48, "TOTAL TIME");
	if(gm.eptm > 99 * 60)
		gm.eptm = 99 * 60 + 99;
	sprint(a, "%02d:%02d", gm.eptm / 60, gm.eptm % 60);
	pictxt(113, 64, a);
	pictxt(96, 96, "AVERAGES");
	n = ver < SDM ? 8 : 14;
	sprint(a, "KILL %3d%%", (gm.epk / n));
	pictxt(112, 112, a);
	sprint(a, "SECRET %3d%%", (gm.eps / n));
	pictxt(80, 128, a);
	sprint(a, "TREASURE %3d%%", (gm.ept / n));
	pictxt(48, 144, a);
	if(ver == WL6 && gm.difc >= GDmed){
		pic(240, 64, pict[Ptc]);
		fnt = fnts;
		n = gm.eptm / 60;
		a[0] = 'A' + (n / 10 ^ n % 10 ^ 0xa);
		n = gm.eptm % 60;
		a[1] = 'A' + (n / 10 ^ n % 10 ^ 0xa);
		a[2] = 'A' + (a[0] ^ a[1]);
		a[3] = 0;
		txt(241, 72, a, DIshi);
	}
	fnt = fnts+1;
	mus(ver < SDM ? Mwon : Msdwon);
	grab(0);
}

static void
colp(void)
{
	pic(124, 44, pict[Pcollapse] + iri++);
	out();
}
static void
incolp(void)
{
	iri = 1;
	fill(0x7f);
	pic(124, 44, pict[Pcollapse]);
	mus(Mend);
}

static void
breathe(void)
{
	static int n, t;

	if(++t > 35){
		t = 0;
		n ^= 1;
		pic(0, 16, pict[n ? Pguy2 : Pguy]);
		out();
	}
}

static void
sinterw(void)
{
	breathe();
	if(nbrecv(csc, nil) <= 0)
		qtc = 0;
}

static void
siscore(void)
{
	givep(15000);
	gm.oldpt = gm.pt;
}
static void
sinter(void)
{
	put(0, 0, Vw, Vhud, 0x7f);
	pic(0, 16, pict[Pguy]);
	if(ver < SDM)
		pictxt(112, 32, "SECRET FLOOR\n COMPLETED!");
	else{
		switch(gm.map){
		case 4: pictxt(112, 32, " TRANS\n GROSSE\nDEFEATED!"); break;
		case 9: pictxt(112, 32, "BARNACLE\nWILHELM\nDEFEATED!"); break;
		case 15: pictxt(112, 32, "UBERMUTANT\nDEFEATED!"); break;
		case 17: pictxt(112, 32, " DEATH\n KNIGHT\nDEFEATED!"); break;
		case 18: pictxt(104, 32, "SECRET TUNNEL\n    AREA\n  COMPLETED!"); break;
		case 19: pictxt(104, 32, "SECRET CASTLE\n    AREA\n  COMPLETED!"); break;
		}
	}
	if(gm.map == 17){	/* solely for crmskp */
		gm.com = GMup;
		gm.map = 19;
	}
	pictxt(80, 128, "15000 BONUS!");
	mus(Minter);
}

static void
interw(void)
{
	breathe();
	if(nbrecv(csc, nil) > 0)
		reset(ver == SDM && gm.map == 1 ? ql+Lsdmend : ql+Lintere);
}

static void
intere(void)
{
	givep(irb);
	gm.oldpt = gm.pt;
	qsp->q = ql+Linterw;
}

static void
interskip(void)
{
	int n;
	char a[10];

	irb = irr[0] * 500;
	irb += 10000 * ((irr[1] == 100) + (irr[2] == 100) + (irr[3] == 100));
	n = sprint(a, "%d", irb);
	pictxt((36 - n * 2) * 8, 56, a);
	n = sprint(a, "%d", irr[1]);
	pictxt((37 - n * 2) * 8, 112, a);
	n = sprint(a, "%d", irr[2]);
	pictxt((37 - n * 2) * 8, 128, a);
	n = sprint(a, "%d", irr[3]);
	pictxt((37 - n * 2) * 8, 144, a);
	intere();
}

static void
interb(void)
{
	if(nbrecv(csc, nil) > 0)
		interskip();
	else if(lastsfx() >= 0){
		breathe();
		qtc = 0;
	}
}

static void
interm(void)
{
	int n;
	char a[10];

	if(nbrecv(csc, nil) > 0){
		interskip();
		return;
	}
	if(*irp == 100){
		stopsfx();
		irb += 10000;
		n = sprint(a, "%d", irb);
		pictxt((36 - n * 2) * 8, 56, a);
		out();
		sfx(S100);
	}else if(*irp == 0)
		sfx(Snobonus);
	else
		sfx(Sendb2);
	iri = 0;
	qsp->q = ql+Linteri;
	qsp->q->q = ql+Linteri;
	switch(++irp - irr){
	case 1: iry = 112; break;
	case 2: iry = 128; break;
	case 3: iry = 144; break;
	case 4: intere(); break;
	}
}

static void
interi(void)
{
	int n;
	char a[10];

	if(nbrecv(csc, nil) > 0){
		interskip();
		return;
	}
	n = sprint(a, "%d", iri);
	pictxt((37 - n * 2) * 8, iry, a);
	sfx(Sendb1);
	out();
	iri += 10;
	if(iri == *irp + 10){
		ql[Linterm].s[0].dt = *irp == 100 || *irp == 0 ? 30 : 0;
		qsp->q = ql+Linterm;
	}else if(iri >= *irp)
		iri = *irp;
}

static void
interp(void)
{
	int m;
	char a[10];

	m = sprint(a, "%d", iri * 500);
	pictxt((18 - m) * 16, 56, a);
	sfx(Sendb1);
	out();
	iri += 50;
	if(iri == *irp + 50){
		sfx(Sendb2);
		iri = 0;
		irb += iri * 500;
		iry = 112;
		irp++;
		qsp->s[0].f = interi;
	}else if(iri >= *irp)
		iri = *irp;
}

static void
inter(void)
{
	static int wlpar[] = {
		90, 120, 120, 210, 180, 180, 150, 150, 0, 0,
		90, 210, 180, 120, 240, 360, 60, 180, 0, 0,
		90, 90, 150, 150, 210, 150, 120, 360, 0, 0,
		120, 120, 90, 60, 270, 210, 120, 270, 0, 0,
		150, 90, 150, 150, 240, 180, 270, 210, 0, 0,
		390, 240, 270, 360, 300, 330, 330, 510, 0, 0
	},
	sdpar[] = {
		 90, 210, 165, 210, 0, 270, 195, 165, 285, 0,
		 390, 270, 165, 270, 360, 0, 360, 0, 0, 0
	};
	int s, p;
	char a[10];

	put(0, 0, Vw, Vhud, 0x7f);
	pic(0, 16, pict[Pguy]);
	pictxt(112, 16, "FLOOR\nCOMPLETED");
	sprint(a, "%d", ver<SDM ? gm.map % 10 + 1 : gm.map + 1);
	pictxt(208, 16, a);
	pictxt(112, 56, "BONUS     0");
	pictxt(128, 80, "TIME");
	pictxt(128, 96, " PAR");
	p = ver < SDM ? wlpar[gm.map] : sdpar[gm.map];
	sprint(a, "%02d:%2d", p / 60, p % 60);
	pictxt(208, 96, a);
	pictxt(72, 112, "KILL RATIO    %");
	pictxt(40, 128, "SECRET RATIO    %");
	pictxt(8, 144, "TREASURE RATIO    %");
	s = gm.tc / Tb;
	if(s > 99 * 60)
		s = 99 * 60;
	sprint(a, "%02d:%02d", s / 60, s % 60);
	pictxt(208, 80, a);
	gm.eptm += s;
	memset(irr, 0, sizeof irr);
	s = p - s;
	irr[0] = s > 0 ? s : 0;
	if(gm.ktot)
		irr[1] = gm.kp * 100 / gm.ktot;
	if(gm.stot)
		irr[2] = gm.sp * 100 / gm.stot;
	if(gm.ttot)
		irr[3] = gm.tp * 100 / gm.ttot;
	gm.epk += irr[1];
	gm.eps += irr[2];
	gm.ept += irr[3];
	irp = irr;
	iri = 0;
	irb = 0;
	if(irr[0] == 0){
		ql[Linteri].s[0].f = interi;
		iry = 112;
		irp++;
	}else
		ql[Linteri].s[0].f = interp;
	ql[Linteri].q = ql+Linteri;
	mus(Minter);
}

static void
gcont(void)
{
	kb = 0;
	mΔx = mΔy = 0;
	mb = 0;
	if(!gm.demo){
		kbon++;
		grab(1);
	}
	step = gstep;
}

static void
camtxt2(void)
{
	camwarp();
	qtc = 32;
	render();
	fizzop(-1, 1);
	put(0, 0, Vw, Vhud, 0x7f);
	viewbox();
}
static void
camtxt(void)
{
	pictxt(0, 56, "LET\'S SEE THAT AGAIN!");
	out();
}

static void
cont(void)
{
	view();
	render();
	mapmus();
}

static void
ingam(void)
{
	greset();
	view();
}

static void
psych2(void)
{
	int n;

	n = (qtc - 1) * 16 + 6;
	if(n > 214 || nbrecv(csc, nil) > 0){
		n = 214;
		qp = qsp->e - 1;
		qtc = 0;
	}
	put(53, 101, n, 2, 0x37);
	put(53, 101, n-1, 1, 0x32);
	out();
}

static void
psych(void)
{
	ingam();
	mapmus();
	put(0, 0, Vw, Vhud, 0x7f);
	pic(48, 56, pict[Ppsyched]);
}

static void
indem(void)
{
	ginit(*demd++, -1, 0);
	if(demd >= epis)
		demd = dems;
	view();
}

static void
fixedw(char *s)
{
	char c;

	while(c = *s, c != 0)
		*s++ = c - '0' + 129;
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
	for(s=scs, y=76; s<scs+nelem(scs); s++, y+=16){
		txt(32, y, s->name, 0xf);
		sprint(a, "%d", s->lvl);
		fixedw(a);
		x = 176 - txtw(a);
		if(ver == WL6){
			sprint(b, "E%d/L", s->ep + 1);
			x += txt(x - 6, y, b, 0xf) - 6;
		}
		txt(x, y, a, 0xf);
		sprint(a, "%d", s->n);
		fixedw(a);
		txt(264 - txtw(a), y, a, 0xf);
	}
	if(qsp != ql+Lscore)
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
	for(s=scs, y=76; s<scs+nelem(scs); s++, y+=16){
		txt(16, y, s->name, DIrhi);
		if(s->lvl == 21)
			pic(176, y-1, pict[Pspear]);
		else{
			sprint(a, "%d", s->lvl);
			txt(194 - txtw(a), y, a, 0xf);
		}
		sprint(a, "%d", s->n);
		txt(292 - txtw(a), y, a, 0xf);
	}
	if(qsp != ql+Lscore)
		mus(Maward);
}

static void
high(void)
{
	if(ver < SDM)
		score();
	else
		sdscore();
	inctl();
	grab(0);
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
exit(void)
{
	threadexitsall(nil);
}

static Sp
	loadq[] = {{30, fadeout}},
	introq[] = {{30, fadein}, {7*Tb, skip}, {30, fadeout}},
	titleq[] = {{30, fadein}, {15*Tb, skip}, {30, fadeout}},
	loopq[] = {{30, fadein}, {10*Tb, skip}, {30, fadeout}},
	scoreq[] = {{30, fadein}, {10*Tb, skip}, {30, fadeout}},
	pantsq[] = {{30, fadein}, {1, bwait}, {30, fadeout}},
	psychq[] = {{30, fadein}, {14, psych2}, {70, nbwait}, {30, fadeout}},
	gamq[] = {{30, fadein}, {1, game}, {41, fizz}, {1, gcont}, {30, fadeout}},
	gamsq[] = {{1, game}, {41, fizz}, {1, gcont}, {30, fadeout}},
	contq[] = {{30, fadein}, {1, gcont}, {30, fadeout}},
	msgq[] = {{1, bwait}, {1, gcont}, {30, fadeout}},
	gcontq[] = {{1, gcont}, {30, fadeout}},
	dieq[] = {{1, dieturn}, {144, fizz}, {100, nbwait}, {1, swait}},
	fdieq[] = {{0, ctl}, {1, blink}, {30, fadeout}, {1, mend2}, {30, fadein}},
	camq[] = {{100, nil}, {144, fizz}, {0, camtxt}, {300, nbwait}, {0, camtxt2}, {41, fizz}, {1, gcont}, {100, nil}, {0, stopmus}, {30, fadeout}},
	spq[] = {{1, swait}, {0, mapmus}, {1, gcont}, {30, fadeout}},
	interq[] = {{1, swait}, {30, fadein}},
	interiq[] = {{0, nil}, {1, interb}},
	intermq[] = {{0, nil}, {1, interm}, {1, interb}},
	interwq[] = {{1, interw}},
	intereq[] = {{0, nextmap}, {30, fadeout}},
	sinterq[] = {{1, swait}, {30, fadein}, {0, siscore}, {1, sinterw}, {0, nextmap}, {30, fadeout}},
	sdmq[] = {{1, bwait}, {30, fadeout}},
	colpq[] = {{300, fadeout}},
	colp2q[] = {{30, fadein}, {2*Tb, nil}, {0, colp}, {105, nil}, {0, colp}, {105, nil}, {0, colp}, {3*Tb, nil}},
	colp3q[] = {{5, fadeout}},
	wonq[] = {{1, swait}, {30, fadein}, {1, bwait}},
	won2q[] = {{0, nil}, {30, fadeout}},
	sdeq[] = {{30, fadein}, {1, bwait}, {30, fadeout}},
	presq[] = {{30, fadein}, {0, pres2}, {10*Tb, nbwait}, {0, pres3}, {10*Tb, nbwait}, {30, fadeout}},
	highq[] = {{30, fadein}, {1, bwait}, {30, fadeout}},
	escq[] = {{10, fadeout}},
	toctlq[] = {{10, fadein}},
	ctlq[] = {{0, blink}, {70, cwalk}, {0, blink}, {8, cwalk}},
	slq[] = {{1, slider}},
	curq[] = {{8, nil}, {0, cursfx}},
	togq[] = {{1, toggle}},
	mscoreq[] = {{10, fadein}, {1, bwait}, {10, fadeout}},
	quitq[] = {{0, pblink}, {10, ask}},
	ackq[] = {{1, bwait}},
	exitq[] = {{1, exit}};
static Seq ql[] = {
	[Lload] {nil, loadq, loadq+nelem(loadq), ql+Lintro, &fblk},
	[Lintro] {intro, introq, introq+nelem(introq), ql+Ltitle, &fblk},
	[Lftitle] {nil, escq, escq+nelem(escq), ql+Ltitle, &ftra},
	[Ltitle] {title, titleq, titleq+nelem(titleq), ql+Lcreds, &fblk},
	[Lcreds] {creds, loopq, loopq+nelem(loopq), ql+Lscore, &fblk},
	[Lscore] {score, loopq, loopq+nelem(loopq), ql+Ldemo, &fblk},
	[Lfpants] {nil, loadq, loadq+nelem(loadq), ql+Lpants, &fblk},
	[Lpants] {pants, pantsq, pantsq+nelem(pantsq), ql+Ltoctl, &fblk},
	[Ldemo] {indem, gamq, gamq+nelem(gamq), nil, &fblk},
	[Lpsych] {psych, psychq, psychq+nelem(psychq), ql+Lgame, &fblk},
	[Lgame] {view, gamq, gamq+nelem(gamq), nil, &fblk},
	[Lretry] {ingam, gamsq, gamsq+nelem(gamsq), nil},
	[Lmsg] {nil, msgq, msgq+nelem(msgq), nil, &fblk},
	[Lcont1] {nil, escq, escq+nelem(escq), ql+Lcont2, &ftra},
	[Lcont2] {cont, contq, contq+nelem(contq), nil, &fblk},
	[Lgcont] {cont, gcontq, gcontq+nelem(gcontq), nil, &fblk},
	[Lfdie] {nil, fdieq, fdieq+nelem(fdieq), ql+Ldie, &fblk},
	[Ldie] {die, dieq, dieq+nelem(dieq), nil},
	[Ldie2] {nil, loadq, loadq+nelem(loadq), ql+Lhigh, &fblk},
	[Lhigh] {high, highq, highq+nelem(highq), ql+Ltitle, &fblk},
	[Lcam] {nil, camq, camq+nelem(camq), nil, &fblk},
	[Lspear] {spshunt, spq, spq+nelem(spq), nil, &fblk},
	[Linter] {inter, interq, interq+nelem(interq), ql+Linteri, &fblk},
	[Linteri] {nil, interiq, interiq+nelem(interiq), nil},
	[Linterm] {nil, intermq, intermq+nelem(intermq), ql+Linteri},
	[Linterw] {nil, interwq, interwq+nelem(interwq), ql+Linterw},
	[Lintere] {nil, intereq, intereq+nelem(intereq), ql+Lpsych, &fblk},
	[Lsinter] {sinter, sinterq, sinterq+nelem(sinterq), ql+Lpsych, &fblk},
	[Lsdmend] {sdmend, sdmq, sdmq+nelem(sdmq), ql+Lhigh, &fblk},
	[Lcolp] {nil, colpq, colpq+nelem(colpq), ql+Lcolp2, &focl},
	[Lcolp2] {incolp, colp2q, colp2q+nelem(colp2q), ql+Lcolp3, &ficl},
	[Lcolp3] {nil, colp3q, colp3q+nelem(colp3q), ql+Lwon, &fecl},
	[Lwon] {won, wonq, wonq+nelem(wonq), ql+Lwon2, &fblk},
	[Lwon2] {nil, won2q, won2q+nelem(won2q), ql+Lhigh, &fblk},
	[Lsdepi] {sdepi, sdeq, sdeq+nelem(sdeq), ql+Lpres, &fblk},
	[Lpres] {pres1, presq, presq+nelem(presq), ql+Lroll1, &fblk},
	[Lroll1] {roll, sdeq, sdeq+nelem(sdeq), ql+Lroll2, &fblk},
	[Lroll2] {roll, sdeq, sdeq+nelem(sdeq), ql+Lroll3, &fblk},
	[Lroll3] {roll, sdeq, sdeq+nelem(sdeq), ql+Lroll4, &fblk},
	[Lroll4] {roll, sdeq, sdeq+nelem(sdeq), ql+Lroll5, &fblk},
	[Lroll5] {roll, sdeq, sdeq+nelem(sdeq), ql+Lroll6, &fblk},
	[Lroll6] {roll, sdeq, sdeq+nelem(sdeq), ql+Lroll7, &fblk},
	[Lroll7] {roll, sdeq, sdeq+nelem(sdeq), ql+Lhigh, &fblk},
	[Ldecay] {nil, loadq, loadq+nelem(loadq), ql+Ltoctl},
	[Ltoctl] {ctl, toctlq, toctlq+nelem(toctlq), ql+Lctl, &ftra},
	[Lftoctl] {nil, escq, escq+nelem(escq), ql+Lmtoctl, &fctl},
	[Lmtoctl] {ctl, toctlq, toctlq+nelem(toctlq), ql+Lctl, &fctl},
	[Lctl] {ctl, ctlq, ctlq+nelem(ctlq), ql+Lctl},
	[Lcur] {nil, curq, curq+nelem(curq), ql+Lctl},
	[Lftonew] {nil, escq, escq+nelem(escq), ql+Lmtonew, &fctl},
	[Lmtonew] {newgame, toctlq, toctlq+nelem(toctlq), ql+Lnctl, &fctl},
	[Lnewgame] {newgame, toctlq, toctlq+nelem(toctlq), ql+Lnctl, &fctl},
	[Lnctl] {newgame, ctlq, ctlq+nelem(ctlq), ql+Lnctl},
	[Ldenied] {denied, ackq, ackq+nelem(ackq), ql+Lnctl},
	[Ldifc1] {nil, escq, escq+nelem(escq), ql+Ldifc2, &fctl},
	[Ldifc2] {difc, toctlq, toctlq+nelem(toctlq), ql+Ldifc3, &fctl},
	[Ldifc3] {difc, ctlq, ctlq+nelem(ctlq), ql+Ldifc3},
	[Ldifc4] {nil, escq, escq+nelem(escq), ql+Ldifc5, &fctl},
	[Ldifc5] {setdifc, loadq, loadq+nelem(loadq), ql+Lpsych, &fblk},
	[Lfsnd] {nil, escq, escq+nelem(escq), ql+Lmsnd, &fctl},
	[Lmsnd] {snd, toctlq, toctlq+nelem(toctlq), ql+Lsctl, &fctl},
	[Lsctl] {snd, ctlq, ctlq+nelem(ctlq), ql+Lsctl},
	[Lsndtog] {nil, togq, togq+nelem(togq), ql+Lsctl},
	[Lfin] {nil, escq, escq+nelem(escq), ql+Lmin, &fctl},
	[Lmin] {in, toctlq, toctlq+nelem(toctlq), ql+Lictl, &fctl},
	[Lictl] {in, ctlq, ctlq+nelem(ctlq), ql+Lictl},
	[Lintog] {nil, togq, togq+nelem(togq), ql+Lictl},
	[Lfsav] {nil, escq, escq+nelem(escq), ql+Lmsav, &fctl},
	[Lmsav] {sav, toctlq, toctlq+nelem(toctlq), ql+Lsvctl, &fctl},
	[Lsvctl] {sav, ctlq, ctlq+nelem(ctlq), ql+Lsvctl},
	[Lflod] {nil, escq, escq+nelem(escq), ql+Lmlod, &fctl},
	[Lmlod] {sav, toctlq, toctlq+nelem(toctlq), ql+Lldctl, &fctl},
	[Lldctl] {sav, ctlq, ctlq+nelem(ctlq), ql+Lldctl},
	[Lfsens] {nil, escq, escq+nelem(escq), ql+Lmsens, &fctl},
	[Lmsens] {sens, toctlq, toctlq+nelem(toctlq), ql+Lsectl, &fctl},
	[Lsectl] {sens, slq, slq+nelem(slq), ql+Lsectl},
	[Lfvw] {nil, escq, escq+nelem(escq), ql+Lmvw, &fctl},
	[Lmvw] {mvw, toctlq, toctlq+nelem(toctlq), ql+Lvwctl, &fctl},
	[Lvwctl] {mvw, slq, slq+nelem(slq), ql+Lvwctl},
	[Lfmscore] {nil, escq, escq+nelem(escq), ql+Lmscore, &fctl},
	[Lmscore] {score, mscoreq, mscoreq+nelem(mscoreq), ql+Lmtoctl, &fctl},
	[Lend] {mend, quitq, quitq+nelem(quitq), ql+Lend},
	[Lcurgame] {curgame, quitq, quitq+nelem(quitq), ql+Lcurgame},
	[Lquit] {quit, quitq, quitq+nelem(quitq), ql+Lquit},
	[Lmexit] {nil, escq, escq+nelem(escq), ql+Lexit, &fctl},
	[Lexit] {nil, exitq, exitq+nelem(exitq)}
};

static void
initseqs(void)
{
	Item *i;

	mclear = wlmclear;
	stripe = wlstripe;
	quits = ends;
	if(ver == WL1){
		for(i=ml[LMnew].s+2; i<ml[LMnew].e; i+=2){
			i->c = DIeps;
			i->q = ql+Ldenied;
		}
	}
	if(ver >= SDM){
		mclear = sdmclear;
		stripe = sdstripe;
		ql[Ltitle].init = sdtitle;
		ql[Lscore].init = sdscore;
		ql[Lmscore].init = sdscore;
		ql[Lwon].f = &ficl;
		ql[Lwon2].q = ql+Lsdepi;
		fctl.c = (Col){0, 0, 0xce};
		mcol[DMbg] = 0x9d;
		quits += nelem(ends)/2;
	}
	mcol[DMoff] = mcol[DMbg] ^ 6;
	mcol[DMbrd] = mcol[DMbg] ^ 4;
	mcol[DMbrd2] = mcol[DMbg] ^ 14;
	ml[LMsnd].s[1].c = mcol[DMoff];
	ml[LMsnd].s[6].c = mcol[DMoff];
}

static void
cfg(void)
{
	muson = sfxon = pcmon = 1;
	grabon++;
	autorun = 0;
	msense = 5;
	vwsize = 15;
	/* fs.c: load config file and read values */
	if(msense < 0)
		msense = 0;
	else if(msense > 9)
		msense = 9;
	if(vwsize < 4)
		vwsize = 4;
	else if(vwsize > 19)
		vwsize = 19;
	setvw();
}

static void
sqend(void)
{
	if(!gm.demo && !gm.record)
		return;
	qsp->q = gm.end == EDkey ? ql+Ltoctl : ql+Ltitle;
	if(demf != nil){
		free(demf);
		demf = nil;
		demd = dems;
		if(demexit)
			qsp->q = ql+Lexit;
	}
	gm.demo = gm.record = 0;
}
static void
edfizz(void)
{
	fizzop(-1, 1);
	put((Vw - vw.dx) / 2, (Vhud - vw.dy) / 2, vw.dx, vw.dy,
		qsp == ql+Lretry ? 4 : 0);
	out();
}
static void
edcam(void)
{
	gm.won++;
	out();
	fizzop(0x7f, 0);
	reset(ql+Lcam);
	qsp->q = ql+Lwon;
}
static void
eddie(void)
{
	u32int *p;

	p = pal;
	reset(ql+Ldie);
	pal = p;
	if(gm.lives >= 0)
		qsp->q = ql+Lretry;
	else
		qsp->q = ql+Ldie2;
}
static void
edup(void)
{
	gm.keys = 0;
	hudk();
	if(ver < SDM && gm.map % 10 == 9 || ver >= SDM
	&& (gm.map == 4 || gm.map == 9 || gm.map == 15 || gm.map >= 17)){
		gm.com = GMret;
		qsp->q = ql+Lsinter;
	}else if(gm.end == EDsetec){
		gm.com = GMsetec;
		qsp->q = ql+Linter;
	}else{
		gm.com = GMup;
		qsp->q = ql+Linter;
	}
}
static void
edspear(void)
{
	reset(ql+Lspear);
	qp->dt = pcmon ? 150 : 1;
	qp->f = pcmon ? nil : swait;
}
static void
edwon(void)
{
	stopmus();
	qsp->q = ql+Lwon;
	if(ver == SOD)
		reset(ql+Lcolp);
}
void
gend(void)
{
	kbon = 0;
	switch(gm.end){
	case EDfizz: edfizz(); break;
	case EDcam: edcam(); break;
	case EDcam2: scalspr(SPcam, vw.dx / 2, vw.dy + 1); sqend(); break;
	case EDdem: sqend(); break;
	case EDkey: qsp->q = ql+Ltoctl; sqend(); break;
	case EDdie: eddie(); break;
	case EDup: /* wet floor */
	case EDsetec: edup(); sqend(); break;
	case EDspear: edspear(); break;
	case EDwon: edwon(); sqend(); break;
	case EDmsg:;
	}
	gm.end = 0;
	pal = pals[Cfad];
	qtc = 0;
	step = qstep;
}

int
quickkey(Rune r)
{
	switch(r){
	case KF|1:
	case KF|2:
	case KF|3:
	case KF|4:
	case KF|5:
	case KF|6:
	case KF|7:
	case KF|8:
	case KF|9:
	default: return 0;
	case KF|10: reset(ql+Lquit); qesc = ql+Lgcont; break;
	}
	gm.end = EDmsg;
	grab(0);
	return 1;
}

void
qstep(void)
{
	Sp *p;

rep:
	p = qp;
	qtc += Δtc;
	if(p->f != nil)
		p->f();
	if(p != qp)
		return;
	if(qtc >= p->dt){
		if(++qp == qsp->e){
			reset(qsp->q);
			return;
		}
		qtc = 0;
	}
	if(p->dt == 0)
		goto rep;
}

void
init(char *f, int m, int d)
{
	srand(time(nil));
	cfg();
	initseqs();
	inctl();
	demd = dems;
	reset(ql+Lload);
	if(m != -1){
		if(d > GDhard)
			d = GDhard;
		qsp->q = ql+Lpsych;
		ginit(nil, m, d);
		ingctl();
		return;
	}
	if(f != nil){
		demf = demof(f);
		demd = (uchar **)&demf;
		qsp->q = ql+Ldemo;
	}
	mus(ver<SDM ? Mintro : Mtower);
}
