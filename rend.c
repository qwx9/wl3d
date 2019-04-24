#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

s32int sint[360+90], *cost;
int vwsize;

typedef struct Vis Vis;
typedef struct Scaler Scaler;
enum{
	Nvis = 50<<2,	/* raised from original 50 */
	Nscal = 256+1,
	Wdxy = 64,
	Fineθ = 3600,
	Dfoclen = 0x5700,
	Dglob = 0x10000,
	Dtile = 0x2000,
	Dobj = 0x4000
};
struct Vis{
	s16int vwx;
	s16int vwdy;
	Spr *spr;
};
#define	Pi	3.141592657
static float Rad = (float)Fineθ / 2 / Pi;

static s32int ftan[Fineθ/4];
static s16int midθ;
static int Δvwθ[Vw];
static int prjw, prjh;
static s32int xin, yin;
static int dtx, dty;
static u16int ∂xdown, ∂xup, ∂ydown, ∂yup;

struct Scaler{
	int skip;
	int ps;
	int pe;
	int dx;
};
static Scaler scals[Nscal][Wdxy+1], *sce;
static uchar *scps, *scts;
static int scx, scdx, scdy;
static int waldy[Vw];
static s32int lastin;
static int lasttile, lastside;

static void
scalscol(Scaler *ss, Sprc *c, int dx)
{
	int ps, pe, w;
	uchar n, *d, *sp, *ds;
	Scaler *s, *se;

	ds = pxb + vw.ofs + scx;
	se = ss + c->e;
	while(se != ss){
		sp = c->p;
		s = ss + c->s;
		for(; s<se; s++, sp++){
			if(s->skip)
				continue;
			n = *sp;
			for(ps=s->ps, pe=s->pe; ps<pe; ps++){
				if(ps >= vw.dy)
					break;
				if(ps < 0)
					continue;
				d = ds + ps * Vw;
				w = dx;
				while(w-- > 0)
					*d++ = n;
			}
		}
		c++;
		se = ss + c->e;
	}
}

static void
scalvis(Vis *v)
{
	int x, dx, lx, rx, *lw, *rw;
	Scaler *ss, *s, *se;
	Spr *spr;
	Sprc **c;

	ss = scals[v->vwdy >> 3];
	if(ss == scals[0] || ss > sce)
		return;
	spr = v->spr;
	lx = spr->lx;
	rx = spr->rx;
	scx = v->vwx;
	if(lx < 32){
		s = ss + lx;
		se = ss + 32;
		while(s < se)
			scx -= s++->dx;
	}else{
		s = ss + 32;
		se = ss + lx;
		while(s < se)
			scx += s++->dx;
	}
	c = spr->cs;
	for(s=ss+lx, se=ss+rx+1; s<se && scx<vw.dx; c++, scx+=dx){
		dx = s++->dx;
		if(dx == 0)
			continue;
		if(dx == 1){
			if(scx >= 0 && waldy[scx] < v->vwdy)
				scalscol(ss, *c, 1);
			continue;
		}
		x = scx + dx;
		if(scx < 0){
			if(x <= 0)
				continue;
			dx = x;
			scx = 0;
		}else if(x > vw.dx)
			dx = vw.dx - scx;
		lw = waldy + scx;
		rw = lw + dx - 1;
		if(*lw < v->vwdy){
			if(*rw < v->vwdy){
				scalscol(ss, *c, dx);
				continue;
			}
			while(*rw >= v->vwdy)
				rw--, dx--;
			scalscol(ss, *c, dx);
			break;
		}else{
			if(*rw >= v->vwdy)
				continue;
			while(*lw >= v->vwdy){
				lw++, scx++;
				dx--;
			}
			scalscol(ss, *c, dx);
		}
	}
}

static void
scalcol(void)
{
	int i, n, x, dx;
	uchar c, *ds, *d;
	Scaler *s;

	s = scals[(waldy[scx] & 0xfff8) >> 3];
	if(s > sce)
		s = sce;
	ds = pxb + vw.ofs + scx;
	for(x=0; x<Wdxy; x++, s++){
		if(s->skip)
			continue;
		c = scps[x];
		for(i=s->ps, n=s->pe; i<n; i++){
			if(i >= vw.dy)
				break;
			if(i < 0)
				continue;
			d = ds + i * Vw;
			dx = scdx;
			while(dx-- > 0)
				*d++ = c;
		}
	}
}

static void
topspr(void)
{
	if(ver < SDM && gm.won){
		if(oplr->s == stt+GSplrcam && qtc & 32)
			scalspr(SPcam, vw.dx/2, vw.dy+1);
		return;
	}
	if(gm.w != WPnone)
		scalspr(wspr[gm.w] + gm.wfrm, vw.dx/2, vw.dy+1);
	if(gm.record || gm.demo)
		scalspr(SPdemo, vw.dx/2, vw.dy+1);
}

static int
rot(Obj *o)
{
	int θ;

	θ = oplr->θ + (vw.mid - o->vwdx) / 8 + 180 + 360 - o->θ + 45/2;
	θ %= 360;
	if(o->s->rot == 2)
		return 4 * (θ / 180);
	return θ / 45;
}

static int
projtl(Tile *tl, Vis *v)
{
	int x, y, cx, cy;

	x = ((tl-tiles) % Mapdxy << Dtlshift) + 0x8000 - vw.x;
	y = ((tl-tiles) / Mapdxy << Dtlshift) + 0x8000 - vw.y;
	cx = ffs(x, vw.cos);
	cx -= ffs(y, vw.sin) + Dtile;
	cy = ffs(y, vw.cos);
	cy += ffs(x, vw.sin);
	if(cx < Dmin){
		v->vwdy = 0;
		return 0;
	}
	v->vwx = vw.mid + cy * prjw / cx;
	/* low 3 bits are fractional */
	v->vwdy = prjh / (cx >> 8);
	return cx < Dtlglobal && abs(cy) < Dtlglobal / 2;
}

static void
projob(Obj *o)
{
	int x, y, cx, cy;

	x = o->x - vw.x;
	y = o->y - vw.y;
	cx = ffs(x, vw.cos);
	cx -= ffs(y, vw.sin) + Dobj;
	cy = ffs(x, vw.sin);
	cy += ffs(y, vw.cos);
	o->vwx = cx;
	if(cx < Dmin){
		o->vwdy = 0;
		return;
	}
	o->vwdx = vw.mid + cy * prjw / cx;
	o->vwdy = prjh / (cx >> 8);
}

static void
scalobj(void)
{
	int i, n, min;
	Obj *o;
	Tile *tl;
	Static *st;
	Vis viss[Nvis], *v, *e, *m;

	memset(viss, 0, sizeof viss);
	e = viss;
	for(st=stcs; st<stce; st++){
		if(st->tl == nil || st->tl->vis == 0)
			continue;
		if(projtl(st->tl, e) && st->f & OFbonus){
			bonus(st);
			continue;
		}
		if(e->vwdy == 0)
			continue;
		e->spr = st->spr;
		if(e < viss + nelem(viss)-1)
			e++;
	}
	for(o=oplr->n; o!=objs; o=o->n){
		if(o->s->spr == nil)
			continue;
		tl = tiles + o->ty * Mapdxy + o->tx;
		if(tl[0].vis
		|| tl[-1].vis && tl[-1].tl == 0
		|| tl[+1].vis && tl[+1].tl == 0
		|| tl[-Mapdxy-1].vis && tl[-Mapdxy-1].tl == 0
		|| tl[-Mapdxy].vis && tl[-Mapdxy].tl == 0
		|| tl[-Mapdxy+1].vis && tl[-Mapdxy+1].tl == 0
		|| tl[Mapdxy+1].vis && tl[Mapdxy+1].tl == 0
		|| tl[Mapdxy].vis && tl[Mapdxy].tl == 0
		|| tl[Mapdxy-1].vis && tl[Mapdxy-1].tl == 0){
			o->on = 1;
			projob(o);
			if(o->vwdy == 0)
				continue;
			o->f |= OFvis;
			e->vwx = o->vwdx;
			e->vwdy = o->vwdy;
			e->spr = o->s->spr;
			if(o->s->rot)
				e->spr += rot(o);
			if(e < viss + nelem(viss)-1)
				e++;
		}else
			o->f &= ~OFvis;
	}
	for(i=0, n=e-viss; i<n; i++){
		min = 32000;
		v = m = viss;
		while(v < e){
			if(v->vwdy < min){
				min = v->vwdy;
				m = v;
			}
			v++;
		}
		scalvis(m);
		m->vwdy = 32000;
	}
}

static s16int
walldy(void)
{
	s32int cx, dy;

	cx = ffs(xin - vw.x, vw.cos);
	dy = cx - ffs(yin - vw.y, vw.sin);
	if(dy < Dmin)
		dy = Dmin;
	return prjh / (dy >> 8);
}

static void
vwall(int i, int tx, int ty, int tile)
{
	int n;
	u16int tex;

	tex = yin >> 4 & 0xfc0;
	if(dtx == -1){
		tex = 0xfc0 - tex;
		xin += Dtlglobal;
	}
	waldy[i] = walldy();
	if(lastside == 1 && lastin == tx && lasttile == tile){
		if(scps == scts + tex){
			scdx++;
			waldy[i] = waldy[i-1];
		}else{
			scalcol();
			scps = scts + tex;
			scdx = 1;
			scx = i;
		}
		return;
	}
	if(lastside != -1)
		scalcol();
	lastside = 1;
	lastin = tx;
	lasttile = tile;
	scx = i;
	scdx = 1;
	if(tile & 0x40){
		if(tiles[ty*Mapdxy + tx-dtx].tl & 0x80)
			n = drofs + 3;
		else
			n = ((tile & ~0x40) - 1) * 2 + 1;
	}else
		n = (tile - 1) * 2 + 1;
	scts = wals[n];
	if(scts == nil)
		sysfatal("sparse wall %d\n", n);
	scps = scts + tex;
}

static void
hwall(int i, int tx, int ty, int tile)
{
	int n;
	u16int tex;

	tex = xin >> 4 & 0xfc0;
	if(dty == -1)
		yin += Dtlglobal;
	else
		tex = 0xfc0 - tex;
	waldy[i] = walldy();
	if(lastside == 0 && lastin == ty && lasttile == tile){
		if(scps == scts + tex){
			scdx++;
			waldy[i] = waldy[i-1];
		}else{
			scalcol();
			scps = scts + tex;
			scdx = 1;
			scx = i;
		}
		return;
	}
	if(lastside != -1)
		scalcol();
	lastside = 0;
	lastin = ty;
	lasttile = tile;
	scx = i;
	scdx = 1;
	if(tile & 0x40){
		tx = xin >> Dtlshift;
		if(tiles[(ty-dty)*Mapdxy + tx].tl & 0x80)
			n = drofs + 2;
		else
			n = ((tile & ~0x40) - 1) * 2;
	}else
		n = (tile - 1) * 2;
	scts = wals[n];
	if(scts == nil)
		sysfatal("sparse wall %d\n", n);
	scps = scts + tex;
}

static void
vdoor(int i, int tile)
{
	int n;
	u16int tex;
	Door *d;

	waldy[i] = walldy();
	d = doors + (tile & 0x7f);
	tex = yin - d->dopen >> 4 & 0xfc0;
	if(lasttile == tile){
		if(scps == scts + tex){
			scdx++;
			waldy[i] = waldy[i-1];
		}else{
			scalcol();
			scps = scts + tex;
			scdx = 1;
			scx = i;
		}
		return;
	}
	if(lastside != -1)
		scalcol();
	lastside = 2;
	lasttile = tile;
	scx = i;
	scdx = 1;
	n = 1;
	switch(d->lock){
	case DRunlk: n += drofs; break;
	case DRlock1:
	case DRlock2:
	case DRlock3:
	case DRlock4: n += drofs + 6; break;
	case DRup: n += drofs + 4; break;
	}
	scts = wals[n];
	if(scts == nil)
		sysfatal("sparse wall %d\n", n);
	scps = scts + tex;
}

static void
hdoor(int i, int tile)
{
	int n;
	u16int tex;
	Door *d;

	waldy[i] = walldy();
	d = doors + (tile & 0x7f);
	tex = xin - d->dopen >> 4 & 0xfc0;
	if(lasttile == tile){
		if(scps == scts + tex){
			scdx++;
			waldy[i] = waldy[i-1];
		}else{
			scalcol();
			scps = scts + tex;
			scdx = 1;
			scx = i;
		}
		return;
	}
	if(lastside != -1)
		scalcol();
	lastside = 2;
	lasttile = tile;
	scx = i;
	scdx = 1;
	n = 0;
	switch(d->lock){
	case DRunlk: n = drofs; break;
	case DRlock1:
	case DRlock2:
	case DRlock3:
	case DRlock4: n = drofs + 6; break;
	case DRup: n = drofs + 4; break;
	}
	scts = wals[n];
	if(scts == nil)
		sysfatal("sparse wall %d\n", n);
	scps = scts + tex;
}

static void
vpush(int i, int tile)
{
	int n;
	u16int tex, ofs;

	tex = yin >> 4 & 0xfc0;
	ofs = pusher.dopen << 10;
	if(dtx == -1){
		xin += Dtlglobal - ofs;
		tex = 0xfc0 - tex;
	}else
		xin += ofs;
	waldy[i] = walldy();
	if(lasttile == tile){
		if(scps == scts + tex){
			scdx++;
			waldy[i] = waldy[i-1];
		}else{
			scalcol();
			scps = scts + tex;
			scdx = 1;
			scx = i;
		}
		return;
	}
	if(lastside != -1)
		scalcol();
	lasttile = tile;
	scx = i;
	scdx = 1;
	n = ((tile & 63) - 1) * 2 + 1;
	scts = wals[n];
	if(scts == nil)
		sysfatal("sparse wall %d\n", n);
	scps = scts + tex;
}

static void
hpush(int i, int tile)
{
	int n;
	u16int tex, ofs;

	tex = xin >> 4 & 0xfc0;
	ofs = pusher.dopen << 10;
	if(dty == -1)
		yin += Dtlglobal - ofs;
	else{
		tex = 0xfc0 - tex;
		yin += ofs;
	}
	waldy[i] = walldy();
	if(lasttile == tile){
		if(scps == scts + tex){
			scdx++;
			waldy[i] = waldy[i-1];
		}else{
			scalcol();
			scps = scts + tex;
			scdx = 1;
			scx = i;
		}
		return;
	}
	if(lastside != -1)
		scalcol();
	lasttile = tile;
	scx = i;
	scdx = 1;
	n = ((tile & 63) - 1) * 2;
	scts = wals[n];
	if(scts == nil)
		sysfatal("sparse wall %d\n", n);
	scps = scts + tex;
}

static void
raytrace(void)
{
	int i, θ, tx, ty;
	u8int n;
	s16int in, xinh, yinh;
	u16int dr, x, y, ∂x, ∂y, tilehit;
	s32int rs, dx, dy;

	vw.θ = oplr->θ;
	midθ = vw.θ * (Fineθ / 360);
	vw.sin = sint[vw.θ];
	vw.cos = cost[vw.θ];
	vw.x = oplr->x - ffs(Dfoclen, vw.cos);
	vw.y = oplr->y + ffs(Dfoclen, vw.sin);
	vw.tx = vw.x >> Dtlshift;
	vw.ty = vw.y >> Dtlshift;
	∂xdown = vw.x & Dtlglobal - 1;
	∂xup = Dtlglobal - ∂xdown;
	∂ydown = vw.y & Dtlglobal - 1;
	∂yup = Dtlglobal - ∂ydown;
	lastside = -1;
	i = 0;
	tilehit = 0;
loop:
	θ = (midθ + Δvwθ[i]) % Fineθ;
	if(θ < 0)
		θ += Fineθ;
	if(θ < 900){
		dtx = 1;
		dty = -1;
		dx = ftan[900-1-θ];
		dy = -ftan[θ];
		∂x = ∂xup;
		∂y = ∂ydown;
	}else if(θ < 1800){
		dtx = -1;
		dty = -1;
		dx = -ftan[θ-900];
		dy = -ftan[1800-1-θ];
		∂x = ∂xdown;
		∂y = ∂ydown;
	}else if(θ < 2700){
		dtx = -1;
		dty = 1;
		dx = -ftan[2700-1-θ];
		dy = ftan[θ-1800];
		∂x = ∂xdown;
		∂y = ∂yup;
	}else{
		dtx = 1;
		dty = 1;
		dx = ftan[θ-2700];
		dy = ftan[3600-1-θ];
		∂x = ∂xup;
		∂y = ∂yup;
	}
	yin = ffs(dy, ∂x) + vw.y;
	yinh = yin >> 16;
	tx = vw.tx + dtx;
	x = yinh * Mapdxy + tx;
	xin = ffs(dx, ∂y) + vw.x;
	xinh = xin >> 16;
	ty = vw.ty + dty;
	y = ty * Mapdxy + xinh;

vcheck:
	if(dty * (yinh - ty) >= 0)
		goto hentry;
ventry:
	n = tiles[x].tl;
	if(n == 0){
vpass:
		tiles[x].vis = 1;
		tx += dtx;
		rs = (yinh << 16 | yin & 0xffff) + dy;
		yinh = rs >> 16;
		yin = yin & 0xffff0000 | rs & 0xffff;
		x = yinh * Mapdxy + tx;
		goto vcheck;
	}
	tilehit = tilehit & 0xff00 | n;
	if(~n & 1<<7){
		xin = tx << 16;
		ty = yinh;
		yin = yinh << 16 | yin & 0xffff;
		vwall(i, tx, ty, tilehit);
		goto next;
	}
	yin = yinh << 16 | yin & 0xffff;
	if(n & 1<<6){
		rs = ((s32int)pusher.dopen * dy >> 6) + yin;	/* sar */
		in = rs >> Dtlshift;
		if(in != yinh)
			goto vpass;
		yin = rs;
		xin = tx << 16;
		vpush(i, tilehit);
		goto next;
	}
	rs = yin + (dy >> 1);	/* sar */
	in = rs >> Dtlshift;
	dr = rs & 0xffff;
	if(yinh != in || dr < doors[n&0x7f].dopen)
		goto vpass;
	yin = yinh << 16 | dr;
	xin = tx << 16 | 0x8000;	/* intercept in middle of tile */
	vdoor(i, tilehit);
	goto next;

hcheck:
	if(dtx * (xinh - tx) >= 0)
		goto ventry;
hentry:
	n = tiles[y].tl;
	if(n == 0){
hpass:
		tiles[y].vis = 1;
		ty += dty;
		rs = (xinh << 16 | xin & 0xffff) + dx;
		xinh = rs >> 16;
		xin = xin & 0xffff0000 | rs & 0xffff;
		y = ty * Mapdxy + xinh;
		goto hcheck;
	}
	tilehit = tilehit & 0xff00 | n;
	if(~n & 1<<7){
		xin = xinh << 16 | xin & 0xffff;
		tx = xinh;
		yin = ty << 16;
		hwall(i, tx, ty, tilehit);
		goto next;
	}
	yin = yinh << 16 | yin & 0xffff;
	if(n & 1<<6){
		rs = ((s32int)pusher.dopen * dx >> 6) + (xinh << 16 | xin & 0xffff);	/* sar */
		in = rs >> Dtlshift;
		if(in != xinh)
			goto hpass;
		xin = rs;
		yin = ty << 16;
		hpush(i, tilehit);
		goto next;
	}
	rs = (xinh << 16 | xin & 0xffff) + (dx >> 1);	/* sar */
	in = rs >> Dtlshift;
	dr = rs & 0xffff;
	if(xinh != in || dr < doors[n&0x7f].dopen)
		goto hpass;
	xin = xinh << 16 | dr;
	yin = ty << 16 | 0x8000;	/* intercept in middle of tile */
	hdoor(i, tilehit);

next:
	if(++i < vw.dx)
		goto loop;
	scalcol();
}

static void
calcscal(Scaler *s, int dy)
{
	int i, step, fix, top, ps, pe;

	top = (vw.dy - dy) / 2;
	step = (dy << 16) / 64;
	fix = 0;
	for(i=0; i<nelem(scals[0]); i++, s++){
		ps = (fix >> 16) + top;
		fix += step;
		pe = (fix >> 16) + top;
		s->dx = pe > ps ? pe - ps : 0;
		s->skip = ps == pe || pe < 0 || ps >= vw.dy || i == nelem(scals[0])-1;
		if(s->skip)
			continue;
		s->ps = ps;
		s->pe = pe;
	}
}
static void
scaltab(int maxdy)
{
	int dy;
	Scaler (*s)[nelem(scals[0])];

	dy = 1;
	memset(scals, 0, sizeof scals);
	s = scals + 1;
	while(dy <= maxdy)
		calcscal(*s++, dy++ * 2);
	memcpy(scals, scals+1, sizeof *scals);
	sce = scals[dy-1];
}

void
scalspr(int n, int x, int dy)
{
	int dx, lx, rx;
	Scaler *ss, *s, *se;
	Spr *spr;
	Sprc **c;

	spr = sprs + n;
	if(spr == nil)
		sysfatal("scalspr: missing sprite %d\n", n);
	dy >>= 1;
	ss = scals[dy];
	lx = spr->lx;
	rx = spr->rx;
	scx = x;
	if(lx < 32){
		s = ss + lx;
		se = ss + 32;
		while(s < se)
			scx -= s++->dx;
	}else{
		s = ss + 32;
		se = ss + lx;
		while(s < se)
			scx += s++->dx;
	}
	c = spr->cs;
	for(s=ss+lx, se=ss+rx+1; s<se; c++, scx+=dx){
		dx = s++->dx;
		if(dx != 0)
			scalscol(ss, *c, dx);
	}
}

s32int
ffs(s32int a, s32int b)
{
	int s;
	u32int r;
	uvlong h, l;

	s = 0;
	if(a < 0){
		a = -a;
		s ^= 1;
	}
	if(b < 0)
		s ^= 1;
	b &= 0xffff;
	h = a >> 16;
	l = a & 0xffff;
	r = h * b + (l * b >> 16 & 0xffff);
	return s ? -r : r;
}

void
render(void)
{
	Tile *tl;

	for(tl=tiles; tl<tiles+nelem(tiles); tl++)
		tl->vis = 0;
	clear();
	raytrace();
	scalobj();
	topspr();
}

void
setvw(void)
{
	int i, an, dx, *p, *q, *e;
	double dface;

	vw.dx = vwsize * 16 & ~15;
	vw.dy = vwsize * 16 / 2 & ~1;
	vw.mid = vw.dx / 2 - 1;
	vw.Δhit = vw.dx / 10;
	vw.ofs = Vw * (160 - vw.dy) / 2 + (Vw - vw.dx) / 2;
	scaltab((vw.dx * 1.5) / 2);

	dx = vw.dx / 2;
	dface = Dfoclen + Dmin;
	i = 0;
	p = Δvwθ + dx;
	e = p + dx;
	q = p - 1;
	while(p < e){
		/* start 0.5px over so vw.θ bisects two middle pixels */
		an = (float)atan(i++ * Dglob / vw.dx / dface) * Rad;
		*p++ = -an;
		*q-- = an;
	}
	prjw = dx * dface / (Dglob/2);
	prjh = prjw * Dtlglobal >> 6;
}

void
tab(void)
{
	s32int i, *f, *fe;
	float a;
	double t;

	i = 0;
	f = ftan;
	fe = ftan + nelem(ftan) - 1;
	while(f < fe){
		t = tan((i++ + 0.5) / (double)Rad);
		*f++ = t * Dtlglobal;
		*fe-- = 1/t * Dtlglobal;
	}

	/* low word: fraction; high word: mbz except 1<<31, sign bit */
	a = 0;
	f = fe = sint;
	while(f < sint + 91){
		i = Dtlglobal * sin(a);
		f[0] = f[360] = fe[180] = i;
		fe[360] = f[180] = i | 1<<31;
		a += Pi / 2 / 90;
		f++, fe--;
	}
	cost = sint + 90;
}
