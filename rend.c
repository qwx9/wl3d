#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

s32int sint[360+90], *cost;

typedef struct Vis Vis;
struct Vis{
	s16int vwdx;
	s16int vwdy;
	Dat *spr;
};

enum{
	Nvis = 50,
	Fineθ = 3600,
	Dfoclen = 0x5700,
	Dglob = 0x10000,
	Dtile = 0x2000,
	Dobj = 0x4000
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
static int lastside;

static void
scalespr(int, int, int)
{
}

static void
scalevis(Vis *)
{
}

static void
topspr(void)
{
	if(ver < SDM && gm.won){
		if(oplr->s == stt+GSplrcam && mtc & 32)
			scalespr(SPcam, vw.dx/2, vw.dy+1);
		return;
	}
	if(gm.w != -1)
		scalespr(wspr[gm.w] + gm.wfrm, vw.dx/2, vw.dy+1);
	if(gm.record || gm.demo)
		scalespr(SPdemo, vw.dx/2, vw.dy+1);
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
	v->vwdx = vw.mid + cy * prjw / cx;
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
scaleall(void)
{
	Obj *o;
	Tile *tl;
	Static *st;
	Vis viss[Nvis], *v, *w, *e, *m;

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
			e->vwdx = o->vwdx;
			e->vwdy = o->vwdy;
			e->spr = o->s->spr;
			if(e->spr == nil)
				e->spr = sprs + o->sdt;
			if(o->s->rot)
				e->spr += rot(o);
			if(e < viss + nelem(viss)-1)
				e++;
		}else
			o->f &= ~OFvis;
	}
	for(v=viss; v<e; v++){
		for(w=v, m=v; w<=e; w++)
			if(w->vwdy < m->vwdy)
				m = w;
		scalevis(m);
		if(v != m)
			memcpy(m, v, sizeof *m);
	}
	if(e != viss)
		scalevis(e);
}

static s16int
walldy(s32int xin, s32int yin)
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
	s16int p;
	u16int tex;

	tex = yin >> 4 & 0xfc0;
	if(dtx == -1){
		tex = 0xfc0 - tex;
		xin += Dtlglobal;
	}
	p = 0;
	USED(tex, p, tx, ty, tile, xin, i);
#ifdef DICKS
	wallheight[i] = walldy();
	if(lastside==1 && lastintercept == tx && lasttilehit == tile){
		/* in the same wall type as last time, so check for
		 * optimized draw */
		if(tex == (u16int)postsource){
			// wide scale
			postwidth++;
			wallheight[i] = wallheight[i-1];
		}else{
			ScalePost();
			(u16int)postsource = tex;
			postwidth = 1;
			postx = i;
		}
		return;
	}
	/* new wall */
	if(lastside != -1)	/* if not the first scaled post */
		ScalePost();
	lastside = true;
	lastintercept = tx;
	lasttilehit = tile;
	postx = i;
	postwidth = 1;

	if(tile & 0x40){	/* check for adjacent doors */
		if(tiles[ty][tx-dtx].tl & 0x80)
			p = SPdoor+3;
		else
			p = vertwall[tile & ~0x40];
	}else
		p = vertwall[tile];
	*(((u16int *)&postsource)+1) = (u16int)PM_GetPage(p);
	(u16int)postsource = tex;
#endif
}

static void
hwall(int i, int tx, int ty, int tile)
{
	s16int p;
	u16int tex;

	tex = xin >> 4 & 0xfc0;
	if(dty == -1)
		yin += Dtlglobal;
	else
		tex = 0xfc0 - tex;
	p = 0;
	USED(i, tx, ty, tile, tex, p);
#ifdef DICKS
	wallheight[i] = walldy();
	if(lastside == 0 && lastintercept == ty && lasttilehit == tile){
		/* in the same wall type as last time, so check for
		 * optimized draw */
		if(tex == (u16int)postsource){	/* wide scale */
			postwidth++;
			wallheight[i] = wallheight[i-1];
			return;
		}else{
			ScalePost();
			(u16int)postsource = tex;
			postwidth = 1;
			postx = i;
		}
	}else{	/* new wall */
		if(lastside != -1)	/* if not the first scaled post */
			ScalePost();

		lastside = 0;
		lastintercept = ty;
		lasttilehit = tile;
		postx = i;
		postwidth = 1;
		if(tile & 0x40){	/* check for adjacent doors */
			tx = xin >> Dtlshift;
			if(tiles[ty-dty][tx].tl & 0x80)
				p = SPdoor+2;
			else
				p = horizwall[tile & ~0x40];
		}else
			p = horizwall[tile];
		*( ((u16int *)&postsource)+1) = (u16int)PM_GetPage(p);
		(u16int)postsource = tex;
	}
#endif
}

static void
vdoor(int i, int tile)
{
	USED(i, xin, yin, tile);
#ifdef DICKS
	Door *d;
	u16int tex, p;

	wallheight[i] = walldy(xin, yin);
	d = doors + (tile & 0x7f);
	tex = yin - d->dopen >> 4 & 0xfc0;
	if(lasttilehit == tile){
		/* in the same door as last time, so check for optimized draw */
		if(tex == (u16int)postsource){
			/* wide scale */
			postwidth++;
			wallheight[i] = wallheight[i-1];
			return;
		}else{
			ScalePost ();
			(u16int)postsource = tex;
			postwidth = 1;
			postx = i;
		}
	}else{
		if (lastside != -1)	/* if not the first scaled post */
			ScalePost ();		/* draw last post */
		/* first pixel in this door */
		lastside = 2;
		lasttile = tile;
		postx = i;
		postwidth = 1;
		switch(d->lock){
		case DRunlk: p = SPdoor; break;
		case DRlock1:
		case DRlock2:
		case DRlock3:
		case DRlock4: p = SPdoor+6; break;
		case DRup: p = SPdoor+4; break;
		}
		*( ((u16int *)&postsource)+1) = (u16int)PM_GetPage(p+1);
		(u16int)postsource = tex;
	}
#endif
}

static void
hdoor(int i, int tile)
{
	USED(i, xin, yin, tile);
#ifdef DICKS
	Door *d;
	u16int tex, p;

	wallheight[i] = walldy(xin, yin);
	d = doors + (tile & 0x7f);
	tex = xin - d->dopen >> 4 & 0xfc0;
	if(lasttilehit == tile){
		/* in the same door as last time, so check for optimized draw */
		if(tex == (u16int)postsource){	/* wide scale */
			postwidth++;
			wallheight[i] = wallheight[i-1];
			return;
		}else{
			ScalePost();
			(u16int)postsource = tex;
			postwidth = 1;
			postx = i;
		}
	}else{
		if(lastside != -1)	/* if not the first scaled post */
			ScalePost();	/* draw last post */
		/* first pixel in this door */
		lastside = 2;
		lasttile = tile;
		postx = i;
		postwidth = 1;
		switch(d->lock){
		case DRunlk: p = SPdoor; break;
		case DRlock1:
		case DRlock2:
		case DRlock3:
		case DRlock4: p = SPdoor+6; break;
		case DRup: p = SPdoor+4; break;
		}
		*( ((u16int *)&postsource)+1) = (u16int)PM_GetPage(p);
		(u16int)postsource = tex;
	}
#endif
}

static void
vpush(int i, int tile)
{
	s16int p;
	u16int tex, ofs;

	tex = yin >> 4 & 0xfc0;
	ofs = pusher.dopen << 10;
	if(dtx == -1){
		xin += Dtlglobal - ofs;
		tex = 0xfc0 - tex;
	}else
		xin += ofs;
	p = 0;
	USED(i, xin, tile, p, tex, ofs);
#ifdef DICKS
	wallheight[i] = walldy(xin, yin);
	if(lasttilehit == tile){
		/* in the same wall type as last time, so check for
		 * optimized draw */
		if(tex == (u16int)postsource){	/* wide scale */
			postwidth++;
			wallheight[i] = wallheight[i-1];
			return;
		}else{
			ScalePost();
			(u16int)postsource = tex;
			postwidth = 1;
			postx = i;
		}
	}else{	/* new wall */
		if(lastside != -1)	// if not the first scaled post
			ScalePost ();
		lasttile = tile;
		postx = i;
		postwidth = 1;
		p = vertwall[tile&63];
		*( ((u16int *)&postsource)+1) = (u16int)PM_GetPage(p);
		(u16int)postsource = tex;
	}
#endif
}

static void
hpush(int i, int tile)
{
	s16int p;
	u16int tex, ofs;

	tex = xin >> 4 & 0xfc0;
	ofs = pusher.dopen << 10;
	if(dty == -1)
		yin += Dtlglobal - ofs;
	else{
		tex = 0xfc0 - tex;
		yin += ofs;
	}
	p = 0;
	USED(i, yin, tile, p, tex, ofs);
#ifdef DICKS
	wallheight[i] = walldy(xin, yin);
	if(lasttilehit == tile){
		/* in the same wall type as last time, so check for
		 * optimized draw */
		if(tex == (u16int)postsource){	/* wide scale */
			postwidth++;
			wallheight[i] = wallheight[i-1];
			return;
		}else{
			ScalePost();
			(u16int)postsource = tex;
			postwidth = 1;
			postx = i;
		}
	}else{	/* new wall */
		if(lastside != -1)	/* if not the first scaled post */
			ScalePost();
		lasttile = tile;
		postx = i;
		postwidth = 1;
		p = horizwall[tile&63];
		*( ((u16int *)&postsource)+1) = (u16int)PM_GetPage(p);
		(u16int)postsource = tex;
	}
#endif
}

static void
raytrace(void)
{
	int i, θ, tx, ty;
	u8int n;
	s16int in, xinh, yinh;
	u16int dr, x, y, ∂x, ∂y, tilehit;
	s32int rs, dx, dy;

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
		tiles[x].vis++;
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
		tiles[y].vis++;
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
}

static void
ScalePost(void)
{
#ifdef DICKS
	ax = SCREENSEG;
	es = ax;
	bx = postx >> 1;

	asm mov	bp,WORD PTR [wallheight+bx]	// fractional height (low 3 bits frac)
	asm and	bp,0xfff8			// bp = heightscaler*4
	asm shr	bp,1
	asm cmp	bp,[maxscaleshl2]
	asm jle	heightok
	asm mov	bp,[maxscaleshl2]
heightok:
	asm add	bp,OFFSET fullscalefarcall

	// scale a byte wide strip of wall
	asm mov	bx,[postx]
	asm mov	di,bx
	asm shr	di,2		// X in bytes
	asm add	di,[bufferofs]

	asm and	bx,3
	asm shl	bx,3		// bx = pixel*8+pixwidth
	asm add	bx,[postwidth]

	asm mov	al,BYTE PTR [mapmasks1-1+bx]	// -1 because no widths of 0
	asm mov	dx,SC_INDEX+1
	asm out	dx,al		// set bit mask register
	asm lds	si,DWORD PTR [postsource]
	asm call DWORD PTR [bp]	// scale the line of pixels

	asm mov	al,BYTE PTR [ss:mapmasks2-1+bx]   // -1 because no widths of 0
	asm or	al,al
	asm jz	nomore

	// draw a second byte for vertical strips that cross two bytes
	asm inc	di
	asm out	dx,al			// set bit mask register
	asm call DWORD PTR [bp]		// scale the line of pixels

	asm mov al,BYTE PTR [ss:mapmasks3-1+bx]	// -1 because no widths of 0
	asm or	al,al
	asm jz	nomore

	// draw a third byte for vertical strips that cross three bytes
	asm inc	di
	asm out	dx,al			// set bit mask register
	asm call DWORD PTR [bp]		// scale the line of pixels

nomore:
	asm mov	ax,ss
	asm mov	ds,ax
#endif
}

static void
walls(void)
{
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

	lastside = -1;	// the first pixel is on a new wall
	raytrace();
	ScalePost();	// no more optimization on last post
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
	walls();
	scaleall();
	topspr();
}

void
initscal(void)
{
	int i, an, dx, *p, *q, *e;
	double dface;

	dx = vw.dx / 2;
	dface = Dfoclen + Dmin;
	prjw = dx * dface / (Dglob/2);
	prjh = prjw * Dtlglobal >> 6;

	i = 0;
	p = Δvwθ + dx;
	e = p + dx;
	q = p - 1;
	dx = vw.dx;
	while(p < e){
		/* start 0.5px over so vw.θ bisects two middle pixels */
		an = (float)atan(i++ * Dglob / dx / dface) * Rad;
		*p++ = -an;
		*q-- = an;
	}

	//SetupScaling(vw.dx * 1.5);
}

void
setvw(int n)
{
	vw.size = n;
	vw.dx = n * 16 & ~15;
	vw.dy = n * 16 / 2 & ~1;
	vw.mid = vw.dx / 2 - 1;
	vw.Δhit = vw.dx / 10;
	vw.ofs = Vw * (160 - vw.dy) / 2 + (Vw - vw.dx) / 2;
	initscal();
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
