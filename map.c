#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

enum{
	Nobj = 150,
	Ndoor = 64,
	Nstc = 400,
	Narea = 37
};
Tile tiles[Mapa];
Obj *objs, *ofree, *oplr;
Door doors[Ndoor], *doore, pusher;
Static stcs[Nstc], *stce;
uchar plrarea[Narea], conarea[Narea*Narea];
int wspr[] = {SPknife, SPpistol, SPmg, SPgatling};

static int stctype[] = {
	Rnil, Rblock, Rblock, Rblock, Rnil, Rblock, Ralpo, Rblock, Rblock,
	Rnil, Rblock, Rblock, Rblock, Rblock, Rnil, Rnil, Rblock, Rblock,
	Rblock, Rnil, Rkey1, Rkey2, Rblock, Rnil, Rfood, Rstim, Rclip1,
	Rmg, Rchaingun, Rcross, Rchalice, Rbible, Rcrown, R1up, Rgibs, Rblock,
	Rblock, Rblock, Rgibs, Rblock, Rblock, Rnil, Rnil, Rnil, Rnil,
	Rblock, Rblock, Rnil, Rclip2, Rammobox, Rblock, Rspear, Rclip2
};
static Obj opool[Nobj];

static void
spawnstc(Tile *tl, int n)
{
	if(n >= nelem(stctype))
		sysfatal("invalid static object type %d", n);
	if(n > 48 && ver < SDM){
		fprint(2, "spawnstc: ignoring sod only static obj %d\n", n);
		return;
	}
	stce->f = 0;
	switch(stctype[n]){
	case Rnil:
	case Rclip2:
		break;
	case Rblock:
		tl->o = nil;
		tl->to = 1;
		break;
	case Rcross:
	case Rchalice:
	case Rbible:
	case Rcrown:
	case R1up:
		if(!gm.load)
			gm.ntreasure++;
		/* wet floor */
	default:
		stce->f = OFbonus;
		stce->item = stctype[n];
		break;
	}
	stce->tl = tl;
	n = stctype[n] == Rclip2 ? 28 : 2+n;
	stce->spr = sprs + n;
	if(stce->spr == nil)
		sysfatal("spawnstc: missing static sprite %d\n", n);
	if(++stce == stcs+Nstc)
		sysfatal("static object overflow");
}

static void
rconair(int id)
{
	uchar *p, *a;

	a = conarea + id * Narea;
	p = plrarea;
	while(p < plrarea+nelem(plrarea)){
		if(*a && !*p){
			(*p)++;
			rconair(p-plrarea);
		}
		a++, p++;
	}
}
static void
conair(void)
{
	memset(plrarea, 0, sizeof plrarea);
	plrarea[oplr->areaid]++;
	rconair(oplr->areaid);
}

static void
dropening(Door *d)
{
	int δ, a1, a2, x, y;

	δ = d->dopen;
	if(δ == 0){
		a1 = d->tl[d->isvert ? +1 : -Mapdxy].p0 - MTfloor;
		a2 = d->tl[d->isvert ? -1 : +Mapdxy].p0 - MTfloor;
		conarea[a1*Narea + a2]++;
		conarea[a2*Narea + a1]++;
		conair();
		if(plrarea[a1]){
			x = ((d->tl-tiles) % Mapdxy << Dtlshift) + (1<<Dtlshift-1);
			y = ((d->tl-tiles) / Mapdxy << Dtlshift) + (1<<Dtlshift-1);
			sfxatt(Sopendoor, 1, x, y);
		}
	}
	δ += Δtc << 10;
	if(δ >= 0xffff){
		δ = 0xffff;
		d->tc = 0;
		d->φ = DRopen;
		d->tl->o = nil;
		d->tl->to = 0;
	}
	d->dopen = δ;
}

static void
drclosing(Door *d)
{
	int δ, a1, a2;

	if(d->tl->to != (d-doors | 0x80) || d->tl == oplr->tl){
		dropen(d);
		return;
	}
	δ = d->dopen - (Δtc << 10);
	if(δ <= 0){
		δ = 0;
		d->φ = DRshut;
		a1 = d->tl[d->isvert ? +1 : -Mapdxy].p0 - MTfloor;
		a2 = d->tl[d->isvert ? -1 : +Mapdxy].p0 - MTfloor;
		conarea[a1*Narea + a2]--;
		conarea[a2*Narea + a1]--;
		conair();
	}
	d->dopen = δ;
}

static void
drclose(Door *d)
{
	int tx, ty;
	Tile *tl;
	Obj *o1, *o2;

	tl = d->tl;
	if(tl->o != nil || tl->to != 0 || tl == oplr->tl)
		return;
	tx = (tl-tiles) % Mapdxy;
	ty = (tl-tiles) / Mapdxy;
	if(d->isvert){
		o1 = tl[-1].o;
		o2 = tl[+1].o;
		if(oplr->ty == ty
		&& (oplr->x + Dmin >> Dtlshift == tx
		|| oplr->x - Dmin >> Dtlshift == tx)
		|| o1 != nil && o1->x + Dmin >> Dtlshift == tx
		|| o2 != nil && o2->x - Dmin >> Dtlshift == tx)
			return;
	}else{
		o1 = tl[-Mapdxy].o;
		o2 = tl[+Mapdxy].o;
		if(oplr->tx == tx
		&& (oplr->y + Dmin >> Dtlshift == ty
		|| oplr->y - Dmin >> Dtlshift == ty)
		|| o1 != nil && o1->y + Dmin >> Dtlshift == ty
		|| o2 != nil && o2->y - Dmin >> Dtlshift == ty)
			return;
	}
	if(plrarea[tl->p0 - MTfloor]){
		tx = (tx << Dtlshift) + (1<<Dtlshift-1);
		ty = (ty << Dtlshift) + (1<<Dtlshift-1);
		sfxatt(Sclosedoor, 1, tx, ty);
	}
	d->φ = DRclosing;
	tl->to = d-doors | 0x80;
}

static void
drwait(Door *d)
{
	d->tc += Δtc;
	if(d->tc >= 300)
		drclose(d);
}

static void
udoors(void)
{
	Door *d;

	if(gm.won)
		return;
	for(d=doors; d<doore; d++)
		switch(d->φ){
		case DRopen: drwait(d); break;
		case DRopening: dropening(d); break;
		case DRclosing: drclosing(d); break;
		}
}

static void
spawndr(Tile *tl, int isvert, int lock)
{
	int n;
	Door *d;

	d = doore;
	n = d - doors;
	if(d >= doors + nelem(doors))
		sysfatal("spawndr: door overflow");
	d->tl = tl;
	d->isvert = isvert;
	d->lock = lock;
	d->φ = DRshut;
	d->dopen = 0;
	tl->o = nil;
	tl->to = n | 0x80;
	tl->tl = n | 0x80;
	if(isvert){
		tl->p0 = tl[-1].p0;
		tl[-Mapdxy].tl |= 0x40;
		tl[+Mapdxy].tl |= 0x40;
	}else{
		tl->p0 = tl[-Mapdxy].p0;
		tl[-1].tl |= 0x40;
		tl[+1].tl |= 0x40;
	}
	doore++;
}

static void
upush(void)
{
	int n;
	Tile *tl;

	if(pusher.φ == 0)
		return;
	n = pusher.φ >> 7;
	pusher.φ += Δtc;
	pusher.dopen = pusher.φ >> 1 & 63;
	if(pusher.φ >> 7 == n)
		return;
	tl = pusher.tl;
	n = tl->tl & 63;
	tl->tl = 0;
	tl->o = nil;
	tl->to = 0;
	tl->p0 = oplr->areaid + MTfloor;
	if(pusher.φ > 256){
		pusher.φ = 0;
		return;
	}
	switch(pusher.isvert){
	case θN: pusher.tl -= Mapdxy; tl -= Mapdxy * 2; break;
	case θE: pusher.tl++; tl += 2; break;
	case θS: pusher.tl += Mapdxy; tl += Mapdxy * 2; break;
	case θW: pusher.tl--; tl -= 2; break;
	}
	if(tl->o != nil || tl->to != 0){
		pusher.φ = 0;
		return;
	}
	tl->to = n;
	tl->tl = n;
	pusher.tl->tl = n | 0xc0;
}

static void
oswap(Obj *o, Obj *r, int z)
{
	o->p->n = o->n;
	o->n->p = o->p;
	if(z)
		memset(o, 0, sizeof *o);
	o->n = r;
	o->p = r->p;
	r->p->n = o;
	r->p = o;
}

static void
odel(Obj *o)
{
	if(o == oplr)
		sysfatal("odel: player deletion");
	oswap(o, ofree, 1);
}

static void
oinit(int all)
{
	Obj *o, *p;

	memset(opool, 0, sizeof opool);
	objs = opool;
	ofree = opool+1;
	oplr = opool+2;
	objs->n = objs->p = oplr;
	oplr->n = oplr->p = objs;
	p = ofree;
	o = oplr + 1;
	while(o < opool + nelem(opool)){
		o->p = p;
		p->n = o;
		p = o;
		o++;
	}
	ofree->p = p;
	p->n = ofree;

	if(all){
		memset(plrarea, 0, sizeof plrarea);
		memset(conarea, 0, sizeof conarea);
		memset(doors, 0, sizeof doors);
		memset(stcs, 0, sizeof stcs);
		doore = doors;
		stce = stcs;
	}
}

static void
up(Obj *o, Obj **n)
{
	if(o->s->up != nil){
		o->s->up(o);
		*n = o->n;
		if(o->s == nil){
			odel(o);
			return;
		}
	}
	if(o->f & OFnevermark || o->f & OFnomark && o->tl->o != nil)
		return;
	o->tl->o = o;
	o->tl->to = 0;
}

static void
uobj(Obj *o, Obj **n)
{
	if(!o->on && !plrarea[o->areaid])
		return;
	if((o->f & (OFnomark | OFnevermark)) == 0)
		o->tl->o = nil;
	if(o->tc == 0){
		up(o, n);
		return;
	}
	o->tc -= Δtc;
	while(o->tc <= 0){
		if(o->s->act != nil){
			o->s->act(o);
			if(o->s == nil){
				*n = o->n;
				odel(o);
				return;
			}
		}
		o->s = o->s->n;
		*n = o->n;
		if(o->s == nil){
			odel(o);
			return;
		}
		if(o->s->dt == 0){
			o->tc = 0;
			break;
		}
		o->tc += o->s->dt;
	}
	up(o, n);
}

static u16int
unmark(Tile *tl)
{
	u16int n;

	tl->tl = 0;
	n = tl->p0;
	if(tl - 1 < tiles || tl - Mapdxy < tiles
	|| tl + 1 > tiles+nelem(tiles) || tl + Mapdxy > tiles+nelem(tiles))
		sysfatal("unmark: tile out of range");
	if(tl[1].p0 >= MTfloor)
		n = tl[1].p0;
	if(tl[-Mapdxy].p0 >= MTfloor)
		n = tl[-Mapdxy].p0;
	if(tl[Mapdxy].p0 >= MTfloor)
		n = tl[Mapdxy].p0;
	if(tl[-1].p0 >= MTfloor)
		n = tl[-1].p0;
	return n;
}

static void
spawnplr(Tile *tl, int dir)
{
	oplr->s = stt+GSplr;
	oplr->type = Oplr;
	oplr->on++;
	oplr->tl = tl;
	oplr->tx = (tl-tiles) % Mapdxy;
	oplr->ty = (tl-tiles) / Mapdxy;
	osetglobal(oplr);
	oplr->areaid = tl->p0 - MTfloor;
	oplr->θ = (450 - dir * 90) % 360;
	oplr->f |= OFnevermark;
	plrarea[oplr->areaid]++;
}

static void
spawnghost(Tile *tl, State *s)
{
	Obj *o;

	if(ver >= SDM)
		return;
	o = ospawn(tl, s);
	o->type = Oghost;
	o->v = 1500;
	o->θ = θE;
	o->f |= OFambush;
	if(!gm.load)
		gm.nkills++;
}

static void
spawnboss(Tile *tl, int type)
{
	int hp, θ;
	State *s;
	Obj *o;

	/* bug: pcmon checks cut demo playback short before cam can be called
	 * if they were recorded with pcmon=0 */
	θ = θnil;
	s = nil;
	hp = 0;
	switch(type){
	wlonly:
		if(ver >= SDM){
			fprint(2, "spawnboss: non-wl6 obj type %d\n", type);
			return;
		}
		break;
	sdonly:
		if(ver < SDM){
			fprint(2, "spawnboss: non-sod obj type %d\n", type);
			return;
		}
		break;
	case Ohans:
		s = stt+GShans;
		hp = gm.difc<GDhard ? 850 + gm.difc * 100 : 1200;
		θ = θS;
		goto wlonly;
	case Oschb:
		stt[GSschbdie2].dt = pcmon ? 140 : 5;
		s = stt+GSschb;
		hp = gm.difc<GDeasy ? 850 : gm.difc<GDmed ? 950
			: gm.difc<GDhard ? 1550 : 2400;
		θ = θS;
		goto wlonly;
	case Ogretel:
		s = stt+GSgretel;
		hp = gm.difc<GDhard ? 850 + gm.difc * 100 : 1200;
		θ = θN;
		goto wlonly;
	case Ootto:
		stt[GSottodie2].dt = pcmon ? 140 : 5;
		s = stt+GSotto;
		hp = gm.difc<GDhard ? 850 + gm.difc * 100 : 1200;
		θ = θN;
		goto wlonly;
	case Ofett:
		stt[GSfettdie2].dt = pcmon ? 140 : 5;
		s = stt+GSfett;
		hp = gm.difc<GDhard ? 850 + gm.difc * 100 : 1200;
		θ = θS;
		goto wlonly;
	case Ofake:
		stt[GShitlerdie2].dt = pcmon ? 140 : 5;
		s = stt+GSfake;
		hp = 200 + 100 * gm.difc;
		θ = θN;
		goto wlonly;
	case Omech:
		stt[GShitlerdie2].dt = pcmon ? 140 : 5;
		s = stt+GSmech;
		hp = gm.difc<GDeasy ? 800 : gm.difc<GDmed ? 950
			: gm.difc<GDhard ? 1050 : 1200;
		θ = θS;
		goto wlonly;
	case Otrans:
		if(pcmon)
			stt[GStransdie2].dt = 105;
		s = stt+GStrans;
		hp = gm.difc<GDhard ? 850 + gm.difc * 100 : 1200;
		goto sdonly;
	case Owilh:
		if(pcmon)
			stt[GSwilhdie2].dt = 70;
		s = stt+GSwilh;
		hp = gm.difc<GDhard ? 950 + gm.difc * 100 : 1300;
		goto sdonly;
	case Ouber:
		if(pcmon)
			stt[GSuberdie2].dt = 70;
		s = stt+GSuber;
		hp = gm.difc<GDhard ? 1050 + gm.difc * 100 : 1400;
		goto sdonly;
	case Oknight:
		if(pcmon)
			stt[GSknightdie2].dt = 105;
		s = stt+GSknight;
		hp = gm.difc<GDhard ? 1250 + 100 * gm.difc : 1600;
		goto sdonly;
	case Ospectre:
		s = stt+GSspectrewait1;
		hp = gm.difc<GDhard ? 5 * (1 + gm.difc) : 25;
		goto sdonly;
	case Oangel:
		if(pcmon)
			stt[GSangeldie2].dt = 105;
		s = stt+GSangel;
		hp = gm.difc<GDhard ? 1450 + 100 * gm.difc : 2000;
		goto sdonly;
	}
	o = ospawn(tl, s);
	o->type = type;
	o->hp = hp;
	o->θ = θ;
	o->f |= OFshootable | OFambush;
	if(!gm.load)
		gm.nkills++;
}

static void
spawndeadgd(Tile *tl)
{
	Obj *o;

	o = ospawn(tl, stt+GSgddie4);
	o->type = Oinert;
}

static void
spawnguy(Tile *tl, int type, int dir, int patrol)
{
	int hp;
	Obj *o;
	State *s;

	s = nil;
	hp = 0;
	switch(type){
	case Ogd:
		s = stt + (patrol ? GSgdwalk1 : GSgd);
		hp = 25;
		break;
	case Oofc:
		s = stt + (patrol ? GSofcwalk1 : GSofc);
		hp = 50;
		break;
	case Omut:
		s = stt + (patrol ? GSmutwalk1 : GSmut);
		hp = gm.difc > GDmed ? 65 : gm.difc > GDbaby ? 55 : 45;
		break;
	case Oss:
		s = stt + (patrol ? GSsswalk1 : GSss);
		hp = 100;
		break;
	case Odog:
		/* bug: unhandled case causing object pool corruption */
		if(!patrol)
			sysfatal("spawnguy: unhandled spawn type");
		s = stt + GSdogwalk1;
		hp = 1;
		break;
	}
	o = ospawn(tl, s);
	if(patrol){
		tl->o = nil;
		tl->to = 0;
		switch(dir){
		case 0: tl++; o->tx++; break;
		case 1: tl -= Mapdxy; o->ty--; break;
		case 2: tl--; o->tx--; break;
		case 3: tl += Mapdxy; o->ty++; break;
		}
		tl->to = 0;
		tl->o = o;
		o->tl = tl;
	}
	o->type = type;
	o->f |= OFshootable;
	o->hp = hp;
	o->v = type == Odog ? 1500 : 512;
	o->θ = dir * 90;

	if(!gm.load)
		gm.nkills++;
	if(patrol){
		o->Δr = Dtlglobal;
		o->on++;
	}else if(tl->p0 == MTambush){
		tl->p0 = unmark(tl);
		o->f |= OFambush;
		o->areaid = tl->p0 - MTfloor;
	}
}

static void
spawn(Tile *tl)
{
	int n, difc;

	n = tl->p1;
	difc = GDeasy;
	switch(n){
	case 19: case 20: case 21: case 22:
		spawnplr(tl, n-19);
		break;
	case 23: case 24: case 25: case 26: case 27: case 28: case 29: case 30:
	case 31: case 32: case 33: case 34: case 35: case 36: case 37: case 38:
	case 39: case 40: case 41: case 42: case 43: case 44: case 45: case 46:
	case 47: case 48: case 49: case 50: case 51: case 52: case 53: case 54:
	case 55: case 56: case 57: case 58: case 59: case 60: case 61: case 62:
	case 63: case 64: case 65: case 66: case 67: case 68: case 69: case 70:
	case 71: case 72: case 73: case 74:
		spawnstc(tl, n-23);
		break;
	case 98:
		if(!gm.load)
			gm.nsecret++;
		break;
	case 180: case 181: case 182: case 183: difc++;	n-=36; /* wet floor */
	case 144: case 145: case 146: case 147: difc++;	n-=36; /* wet floor */
	case 108: case 109: case 110: case 111:
		if(difc <= gm.difc)
			spawnguy(tl, Ogd, n-108, 0);
		break;
	case 184: case 185: case 186: case 187: difc++;	n-=36; /* wet floor */
	case 148: case 149: case 150: case 151: difc++;	n-=36; /* wet floor */
	case 112: case 113: case 114: case 115:
		if(difc <= gm.difc)
			spawnguy(tl, Ogd, n-112, 1);
		break;
	case 188: case 189: case 190: case 191: difc++;	n-=36; /* wet floor */
	case 152: case 153: case 154: case 155: difc++;	n-=36; /* wet floor */
	case 116: case 117: case 118: case 119:
		if(difc <= gm.difc)
			spawnguy(tl, Oofc, n-116, 0);
		break;
	case 192: case 193: case 194: case 195: difc++;	n-=36; /* wet floor */
	case 156: case 157: case 158: case 159: difc++;	n-=36; /* wet floor */
	case 120: case 121: case 122: case 123:
		if(difc <= gm.difc)
			spawnguy(tl, Oofc, n-120, 1);
		break;
	case 198: case 199: case 200: case 201: difc++;	n-=36; /* wet floor */
	case 162: case 163: case 164: case 165: difc++;	n-=36; /* wet floor */
	case 126: case 127: case 128: case 129:
		if(difc <= gm.difc)
			spawnguy(tl, Oss, n-126, 0);
		break;
	case 202: case 203: case 204: case 205: difc++;	n-=36; /* wet floor */
	case 166: case 167: case 168: case 169: difc++;	n-=36; /* wet floor */
	case 130: case 131: case 132: case 133:
		if(difc <= gm.difc)
			spawnguy(tl, Oss, n-130, 1);
		break;
	case 206: case 207: case 208: case 209: difc++;	n-=36; /* wet floor */
	case 170: case 171: case 172: case 173: difc++;	n-=36; /* wet floor */
	case 134: case 135: case 136: case 137:
		if(difc <= gm.difc)
			spawnguy(tl, Odog, n-134, 0);
		break;
	case 210: case 211: case 212: case 213: difc++;	n-=36; /* wet floor */
	case 174: case 175: case 176: case 177: difc++;	n-=36; /* wet floor */
	case 138: case 139: case 140: case 141:
		if(difc <= gm.difc)
			spawnguy(tl, Odog, n-138, 1);
		break;
	case 252: case 253: case 254: case 255: difc++;	n-=18; /* wet floor */
	case 234: case 235: case 236: case 237: difc++;	n-=18; /* wet floor */
	case 216: case 217: case 218: case 219:
		if(difc <= gm.difc)
			spawnguy(tl, Omut, n-216, 0);
		break;
	case 256: case 257: case 258: case 259: difc++;	n-=18; /* wet floor */
	case 238: case 239: case 240: case 241: difc++;	n-=18; /* wet floor */
	case 220: case 221: case 222: case 223:
		if(difc <= gm.difc)
			spawnguy(tl, Omut, n-220, 1);
		break;
	case 224: case 225: case 226: case 227:
		spawnghost(tl, stt+GSgh1chase1+n-224);
		break;
	case 106: spawnboss(tl, Ospectre); break;
	case 107: spawnboss(tl, Oangel); break;
	case 124: spawndeadgd(tl); break;
	case 125: spawnboss(tl, Otrans); break;
	case 142: spawnboss(tl, Ouber); break;
	case 143: spawnboss(tl, Owilh); break;
	case 160: spawnboss(tl, Ofake); break;
	case 161: spawnboss(tl, Oknight); break;
	case 178: spawnboss(tl, Omech); break;
	case 179: spawnboss(tl, Ofett); break;
	case 196: spawnboss(tl, Oschb); break;
	case 197: spawnboss(tl, Ogretel); break;
	case 214: spawnboss(tl, Ohans); break;
	case 215: spawnboss(tl, Ootto); break;
	}
}

void
drop(Tile *tl, int n)
{
	int sn, *sti;
	Static *s;

	for(sti=stctype; sti<stctype+nelem(stctype); sti++)
		if(*sti == n)
			break;
	if(sti >= stctype+nelem(stctype))
		sysfatal("drop: unknown item type");
	for(s=stcs; s<stcs+nelem(stcs); s++)
		if(s->tl == nil){
			if(s == stce)
				stce++;
			break;
		}
	if(s >= stcs+nelem(stcs))
		return;
	s->tl = tl;
	sn = n == Rclip2 ? 28 : 2+(sti-stctype);
	s->spr = sprs + sn;
	if(s->spr == nil)
		sysfatal("drop: missing static sprite %d\n", sn);
	s->f = OFbonus;
	s->item = n;
}

void
dropen(Door *d)
{
	if(d->φ == DRopen)
		d->tc = 0;
	else
		d->φ = DRopening;
}

void
druse(Door *d)
{
	if(d->lock > DRunlk && d->lock < DRup && ~gm.keys & 1<<d->lock-DRlock1){
		sfx(Snoway);
		return;
	}
	switch(d->φ){
	case DRshut: case DRclosing: dropen(d); break;
	case DRopen: case DRopening: drclose(d); break;
	}
}

void
osetglobal(Obj *o)
{
	if(o->tx > Mapdxy || o->ty > Mapdxy)
		sysfatal("object %d,%d out of bounds", o->tx, o->ty);
	o->x = (o->tx << Dtlshift) + Dtlglobal / 2;
	o->y = (o->ty << Dtlshift) + Dtlglobal / 2;
}

void
ostate(Obj *o, State *s)
{
	o->s = s;
	o->tc = s->dt;
}

Obj *
onew(void)
{
	Obj *o;

	if(ofree->p == ofree)
		sysfatal("onew: object list overflow");
	o = ofree->p;
	oswap(o, objs, 0);
	return o;
}

Obj *
ospawn(Tile *tl, State *s)
{
	Obj *o;

	o = onew();
	tl->o = o;
	tl->to = 0;
	o->tl = tl;
	o->s = s;
	o->tx = (tl-tiles) % Mapdxy;
	o->ty = (tl-tiles) / Mapdxy;
	osetglobal(o);
	o->areaid = tl->p0 - MTfloor;
	o->tc = s->dt != 0 ? rnd() % s->dt : 0;
	return o;
}

void
uworld(void)
{
	Obj *o, *n;

	udoors();
	upush();
	for(o=oplr; o!=objs; o=n){
		n = o->n;
		uobj(o, &n);
	}
}

void
mapmus(void)
{
	static char wlmus[] = {
		3, 11, 9, 12, 3, 11, 9, 12, 2, 0,
		8, 18, 17, 4, 8, 18, 4, 17, 2, 1,
		6, 20, 22, 21, 6, 20, 22, 21, 19, 26,
		3, 11, 9, 12, 3, 11, 9, 12, 2, 0,
		8, 18, 17, 4, 8, 18, 4, 17, 2, 1,
		6, 20, 22, 21, 6, 20, 22, 21, 19, 15
	}, sdmus[] = {
		4, 0, 2, 22, 15, 1, 5, 9, 10, 15,
		8, 3, 12, 11, 13, 15, 21, 15, 18, 0, 17
	};

	mus(ver < SDM ? wlmus[gm.map] : sdmus[gm.map]);
}

void
initmap(void)
{
	u16int *p0, *p1, *s;
	Tile *tl;

	oinit(1);
	memset(tiles, 0, sizeof tiles);
	p0 = s = readmap(gm.map);
	p1 = p0 + Mapa;
	for(tl=tiles; tl<tiles+nelem(tiles); tl++){
		tl->p0 = *p0++;
		tl->p1 = *p1++;
		if(tl->p0 < MTfloor){
			tl->tl = tl->p0;
			tl->to = tl->p0;
		}
	}
	free(s);
	for(tl=tiles; tl<tiles+nelem(tiles); tl++)
		if(tl->p0 > 89 && tl->p0 < 102)
			spawndr(tl, ~tl->p0 & 1, (tl->p0 - 90) / 2);
	for(tl=tiles; tl<tiles+nelem(tiles); tl++)
		spawn(tl);
	for(tl=tiles; tl<tiles+nelem(tiles); tl++)
		if(tl->p0 == MTambush){
			if(tl->to == MTambush)
				tl->to = 0;
			tl->p0 = unmark(tl);
		}
}

void
sodmap(void)
{
	int *w;
	State *s;

	stctype[15] = Rblock;
	stctype[40] = Rnil;
	stctype[44] = Rblock;
	stctype[48] = Rblock;
	for(s=stt+GSgd; s<stt+GShans; s++)
		s->spr += 4;
	for(s=stt+GSmissile; s<stt+GSrocket; s++)
		s->spr -= SPmissile1 - SPofcfire3 - 1;
	for(w=wspr; w<wspr+nelem(wspr); w++)
		*w += SPangeldead - SPbjjump4;
}
