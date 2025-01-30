#include <u.h>
#include <libc.h>
#include <pcm.h>
#include <thread.h>
#include "dat.h"
#include "fns.h"

Dat *imfs;
Sfx *sfxs;
int sfxon, muson, pcmon;
int sfxlck;

typedef struct Snd Snd;
struct Snd{
	int n;
	int att;
	int x;
	int y;
	int lock;
};

enum{
	Rate = 44100,
	PcmHz = 7000,
	ImfHz = 700,
	SfxHz = 140,
	Pdiv = PcmHz / Tb,
	Nsamp = Rate / ImfHz * 4,
	Nbuf = Nsamp * ImfHz / Tb,

	Rwse = 0x01,
	Rnum = 0xa0,
	Roct = 0xb0,
	Ropm = 0xbd,
	Rfed = 0xc0
};
/* channel 0, op 0-1 registers */
static uchar inst[] = {0x20, 0x23, 0x40, 0x43, 0x60, 0x63, 0x80, 0x83, 0xe0, 0xe3};
static uchar *sfxp, *sfxe, *pcm, *pcme, *imf;
static Sfx *sfxd, *pcmd;
static Dat *imfd;
static vlong stc, sdt, mdt;
static Pcmconv *pcmc;
static uchar sbuf[Nbuf], *pcmbuf;
static int bufsz, leftover;
static Channel *musc, *sndc;

static u8int ratt[][30] = {
	{8,8,8,8,8,8,8,7,7,7,7,7,7,6,0,0,0,0,0,1,3,5,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,7,7,7,7,7,6,4,0,0,0,0,0,2,4,6,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,7,7,7,7,6,6,4,1,0,0,0,1,2,4,6,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,7,7,7,7,6,5,4,2,1,0,1,2,3,5,7,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,7,7,7,6,5,4,3,2,2,3,3,5,6,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,7,7,7,6,6,5,4,4,4,4,5,6,7,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,7,7,7,6,6,5,5,5,6,6,7,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,7,7,7,6,6,7,7,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8}
}, latt[][30] = {
	{8,8,8,8,8,8,8,8,5,3,1,0,0,0,0,0,6,7,7,7,7,7,7,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,6,4,2,0,0,0,0,0,4,6,7,7,7,7,7,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,6,4,2,1,0,0,0,1,4,6,6,7,7,7,7,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,7,5,3,2,1,0,1,2,4,5,6,7,7,7,7,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,6,5,3,3,2,2,3,4,5,6,7,7,7,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,7,6,5,4,4,4,4,5,6,6,7,7,7,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,7,6,6,5,5,5,6,6,7,7,7,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,7,7,6,6,7,7,7,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
	{8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8}
};
static int atton, attx, atty, lvol, rvol;

static void
stopal(void)
{
	if(sfxd == nil)
		return;
	opl2wr(Roct, 0);
	sfxp = sfxe = nil;
	sfxd = nil;
}

static void
imfcmd(void)
{
	int r, v, dt;
	uchar *u;

	dt = 0;
	while(dt == 0){
		u = imf;
		r = u[0];
		v = u[1];
		dt = u[3]<<8 | u[2];
		opl2wr(r, v);
		imf += 4;
		if(imf >= imfd->e)
			imf = imfd->p;
	}
	mdt = stc + dt;
}

static void
alcmd(void)
{
	int v;

	if(sfxp >= sfxe){
		stopal();
		return;
	}
	v = *sfxp++;
	if(v != 0){
		opl2wr(Rnum, v);
		opl2wr(Roct, sfxd->blk);
	}else
		opl2wr(Roct, 0);
	sdt = stc + ImfHz / SfxHz;
}

static void
setvol(void)
{
	int x, y, ax, ay;
	s32int f;

	if(!atton){
		lvol = rvol = 16;
		return;
	}
	ax = attx - vw.x;
	ay = atty - vw.y;
	f = ffs(ax, vw.cos);
	x = f - ffs(ay, vw.sin) >> Dtlshift;
	f = ffs(ax, vw.sin);
	y = f + ffs(ay, vw.cos) >> Dtlshift;
	if(x < 0)
		x = -x;
	if(x >= nelem(latt))
		x = nelem(latt) - 1;
	if(y < -nelem(latt))
		y = -nelem(latt);
	else if(y >= nelem(latt))
		y = nelem(latt) - 1;
	lvol = 16 - latt[x][nelem(latt)+y];
	rvol = 16 - ratt[x][nelem(latt)+y];
}

static void
killsfx(void)
{
	stopal();
	pcm = pcme = nil;
	pcmd = nil;
}

int
lastsfx(void)
{
	if(pcm < pcme)
		return pcmd - sfxs;
	else if(sfxd != nil)
		return sfxd - sfxs;
	return -1;
}

static void
killmus(void)
{
	int i;

	killsfx();
	if(!muson && !sfxon)
		return;
	for(i=Roct+1; i<Roct+9; i++)
		opl2wr(i, 0);
	imf = nil;
	imfd = nil;
}

static void
playmus(int n)
{
	Dat *d;

	d = imfs+n;
	if(!muson || d == imfd)
		return;
	killmus();
	if(n < 0)
		return;
	mdt = stc;
	imfd = d;
	imf = d->p;
}

static uchar *
mix(uchar *s, uchar *p, int n)
{
	uchar *e;
	s32int v, w;

	for(e=p+n; p<e; p+=2, s+=4){
		w = (s16int)(p[0] | p[1] << 8);
		v = w * lvol / 16;
		v += (s16int)(s[0] | s[1] << 8);
		if(v > 32767)
			v = 32767;
		else if(v < -32768)
			v = -32768;
		s[0] = v;
		s[1] = v >> 8;
		v = w * rvol / 16;
		v += (s16int)(s[2] | s[3] << 8);
		if(v > 32767)
			v = 32767;
		else if(v < -32768)
			v = -32768;
		s[2] = v;
		s[3] = v >> 8;
	}
	return s;
}

static int
opl2step(void)
{
	uchar *p, *e;

	if(!muson && !sfxon)
		return -1;
	for(p=sbuf, e=p+sizeof sbuf; p<e; stc++){
		if(stc == sdt && sfxd != nil)
			alcmd();
		if(stc == mdt && imfd != nil)
			imfcmd();
		p = opl2out(p, Nsamp);
	}
	return 0;
}

static void
pcmstep(void)
{
	int n, m;
	uchar *e, *p, *s;
	static int leftover;
	static uchar *l;

	s = sbuf;
	m = sizeof sbuf / 2;
	if(leftover){
		s = mix(s, l, leftover);
		m -= leftover;
		leftover = 0;
	}
	p = pcm;
	e = pcme;
	if(p == e)
		return;
	if(p + Pdiv < e)
		e = p + Pdiv;
	setvol();
	n = pcmconv(pcmc, p, pcmbuf, e - p);
	if(n > m){
		leftover = n - m;
		l = pcmbuf + m;
	}
	mix(s, pcmbuf, m);
	pcm = e;
}

static void
playsfx(Snd *snd)
{
	Sfx *s;
	uchar *r, *i;

	if(sfxlck)
		return;
	if(snd->n < 0){
		killsfx();
		return;
	}
	s = sfxs + snd->n;
	if(pcmon && s->pcm != nil){
		if(pcm < pcme && s->pri < pcmd->pri)
			return;
		pcmd = s;
		pcm = s->pcm->p;
		pcme = s->pcm->e;
		if(atton = snd->att){
			attx = snd->x;
			atty = snd->y;
		}
	}else if(sfxon){
		if(sfxd != nil && s->pri < sfxd->pri)
			return;
		stopal();
		sfxd = s;
		sfxp = s->p;
		sfxe = s->e;
		sdt = stc;
		i = s->inst;
		r = inst;
		while(r < inst + sizeof inst)
			opl2wr(*r++, *i++);
	}
	if(snd->lock)
		sfxlck = 1;
}

void
mus(int n)
{
	send(musc, &n);
}

void
stopmus(void)
{
	int n;

	n = -1;
	send(musc, &n);
}

void
sfxatt(int n, int att, int x, int y)
{
	Snd s;

	s = (Snd){n, att, x, y, 0},
	send(sndc, &s);
}

void
locksfx(int n)
{
	Snd s;

	s = (Snd){n, 0, 0, 0, 1},
	send(sndc, &s);
}

void
sfx(int n)
{
	sfxatt(n, 0, 0, 0);
}

void
stopsfx(void)
{
	sfxatt(-1, 0, 0, 0);
}

static void
sndproc(void *)
{
	int fd, n, m;
	Pcmdesc i, o;
	Snd snd;

	if((fd = open("/dev/audio", OWRITE)) < 0){
		fprint(2, "initsnd: %r\n");
		return;
	}
	if(mkpcmdesc("u8c1r7000", &i) < 0
	|| mkpcmdesc("s16c1r44100", &o) < 0)
		sysfatal("mkpcmdesc: %r");
	if((pcmc = allocpcmconv(&i, &o)) == nil)
		sysfatal("allocpcmconv: %r");
	if((n = pcmratio(pcmc, Pdiv)) < 0)
		sysfatal("pcmratio: %r");
	pcmbuf = emalloc(n);
	opl2init(Rate);
	opl2wr(Rwse, 0x20);
	opl2wr(Ropm, 0);
	opl2wr(Rfed, 0);
	enum{
		Amus,
		Asfx,
	};
	Alt a[] = {
		[Amus] {musc, &m, CHANRCV},
		[Asfx] {sndc, &snd, CHANRCV},
		{nil, nil, CHANNOBLK},
	};
	for(;;){
		switch(alt(a)){
		default: break;
		case Amus: playmus(m); break;
		case Asfx: playsfx(&snd); break;
		}
		if(opl2step() < 0){
			if(!pcmon)
				continue;
			memset(sbuf, 0, sizeof sbuf);
		}
		pcmstep();
		if(write(fd, sbuf, sizeof sbuf) != sizeof sbuf)
			sysfatal("sndproc: %r");
	}
}

void
initsnd(void)
{
	if((sndc = chancreate(sizeof(Snd), 1)) == nil
	|| (musc = chancreate(sizeof(int), 1)) == nil)
		sysfatal("chancreate: %r");
	if(proccreate(sndproc, nil, mainstacksize) < 0)
		sysfatal("proccreate: %r");
}
