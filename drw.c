#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

Fnt fnts[2], *fnt;
Pic *pics;
uchar **exts, **dems, **epis, **wals;
Spr *sprs;

int scale, npx;
uchar *px, pxb[Va], *fzd, fzb[Vw*Vhud];
View vw;
void (*mclear)(void);
void (*stripe)(int);

static Col *fcol;
static u32int *fref;
static int fi, fo, fdt;
static int fzc, fzdx, fzdy, fzdn, fzn, fzout;

static uchar wl6ceil[] = {
	0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0xbf,
	0x4e, 0x4e, 0x4e, 0x1d, 0x8d, 0x4e, 0x1d, 0x2d, 0x1d, 0x8d,
	0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x2d, 0xdd, 0x1d, 0x1d, 0x98,
	0x1d, 0x9d, 0x2d, 0xdd, 0xdd, 0x9d, 0x2d, 0x4d, 0x1d, 0xdd,
	0x7d, 0x1d, 0x2d, 0x2d, 0xdd, 0xd7, 0x1d, 0x1d, 0x1d, 0x2d,
	0x1d, 0x1d, 0x1d, 0x1d, 0xdd, 0xdd, 0x7d, 0xdd, 0xdd, 0xdd,
};
static uchar sodceil[] = {
	0x6f, 0x4f, 0x1d, 0xde, 0xdf, 0x2e, 0x7f, 0x9e, 0xae, 0x7f,
	0x1d, 0xde, 0xdf, 0xde, 0xdf, 0xde, 0xe1, 0xdc, 0x2e, 0x1d, 0xdc
};

static void
hudnp(int x, int y, int dx, int n)
{
	char s[20], *p;

	memset(s, 0, sizeof s);
	p = s;
	if(n == 0)
		p++;
	while(n > 0 && p < s + sizeof s)
		*p++ = n % 10, n /= 10;
	n = dx - (p - s);
	if(n > 0)
		memset(p, -1, n);	/* Pblank == Pn0 - 1, always */
	p += n - 1;
	while(p >= s){
		pic(x, y, pict[Pn0] + *p--);
		x += 8;
	}
}

void
out(void)
{
	int n;
	u32int c;
	uchar *s, *d, *w;

	d = px;
	s = pxb;
	n = scale * 3;
	while(s < pxb + sizeof pxb){
		c = pal[*s++];
		w = d + n;
		while(d < w){
			*d++ = c;
			*d++ = c>>8;
			*d++ = c>>16;
		}
	}
	flush();
}

void
put(int x, int y, int dx, int dy, int c)
{
	uchar *d;

	d = pxb + x + y*Vw;
	while(dy-- > 0){
		memset(d, c, dx);
		d += Vw;
	}	
}

void
pput(int x, int y, int dx, int dy, uchar *s)
{
	uchar *d;

	d = pxb + x + y*Vw;
	while(dy-- > 0){
		memcpy(d, s, dx);
		s += dx;
		d += Vw;
	}	
}

void
fill(int c)
{
	memset(pxb, c, sizeof pxb);
}

void
clear(void)
{
	int n;
	uchar c, *p;

	c = ver < SDM ? wl6ceil[gm.map] : sodceil[gm.map];
	p = pxb + vw.ofs;
	n = 0;
	while(n++ < vw.dy){
		memset(p, c, vw.dx);
		p += Vw;
		if(n == vw.dy/2)
			c = 0x19;
	}
}

void
fizz(void)
{
	int i, x, y, ofs;

	for(i=0; i<fzdn*Δtc; i++){
		y = fzn - 1 & 0xff;
		x = fzn >> 8;
		fzn = fzn >> 1 ^ (fzn & 1 ? 0x12000 : 0);
		if(x >= fzdx || y >= fzdy)
			continue;
		ofs = y * Vw + x;
		fzd[ofs] = fzout ? fzc : fzb[ofs];
		if(fzn == 1){
			fzdn = 0;
			break;
		}
	}
	out();
}

void
fizzop(int c, int save)
{
	if(save)
		memcpy(fzb, pxb + vw.ofs, (vw.dy-1) * Vw + vw.dx-1);
	fzdx = vw.dx;
	fzdy = vw.dy;
	fzd = pxb + vw.ofs;
	if(c < 0){
		fzdn = Va / 20;
		fzout = 0;
	}else{
		if(gm.won){
			fzd = pxb;
			fzdx = Vw;
			fzdy = Vhud;
		}
		fzdn = Va / 70;
		fzout = 1;
		fzc = c;
	}
	fzn = 1;
}

void
fadeout(void)
{
	int i, u, v, w;
	u32int p, *s, *d;

	i = fo + Δtc;
	if(i > fdt)
		i = fdt;
	fo = i;
	s = fref;
	d = pal;
	while(d < pal + nelem(pals[0])){
		p = *s++;
		u = p & 0xff;
		v = p>>8 & 0xff;
		w = p>>16 & 0xff;
		u = u + (fcol->b-u) * i/fdt;
		v = v + (fcol->g-v) * i/fdt;
		w = w + (fcol->r-w) * i/fdt;
		*d++ = w<<16 | v<<8 | u;
	}
	out();
}

void
fadein(void)
{
	int i, u, v, w;
	u32int p, *s, *d;

	i = fi + Δtc;
	if(i > fdt)
		i = fdt;
	fi = i;
	s = fref;
	d = pal;
	while(d < pal + nelem(pals[0])){
		p = *s++;
		u = p & 0xff;
		v = p>>8 & 0xff;
		w = p>>16 & 0xff;
		u = fcol->b + (u-fcol->b) * i/fdt;
		v = fcol->g + (v-fcol->g) * i/fdt;
		w = fcol->r + (w-fcol->r) * i/fdt;
		*d++ = w<<16 | v<<8 | u;
	}
	out();
}

void
fadeop(Col *c, int dt)
{
	fi = 0;
	fo = 0;
	fdt = dt;
	fcol = c;
	fref = pal;
	pal = pals[Cfad];
}

void
palfill(Col *c)
{
	u32int v, *p;

	p = pals[Csod];
	v = c->r << 16 | c->g << 8 | c->b;
	while(p < pals[Cend])
		*p++ =  v;
	pal = pals[Csod];
}

void
palpic(uchar *s)
{
	u32int *p;

	p = pals[Csod];
	while(p < pals[Csod] + nelem(pals[0])){
		*p++ = s[0]*255/63 << 16 | s[1]*255/63 << 8 | s[2]*255/63;
		s += 3;
	}
	pal = pals[Csod];
}

int
txt(int x, int y, char *t, int col)
{
	int h, w;
	uchar c, *d, *s, *p, *e, *q;

	h = fnt->h;
	p = fnt->p;
	c = *t++;
	d = pxb + x + y*Vw;
	x = 0;
	while(c != 0){
		w = fnt->w[c];
		s = p + fnt->ofs[c];
		e = s + w*h;
		while(s < e){
			q = s + w;
			while(s < q){
				c = *s++;
				if(c != 0)
					*d = col;
				d++;
			}
			d += Vw-w;
		}
		d -= Vw*h - w;
		x += w;
		c = *t++;
	}
	return x;
}

int
txtnl(int x, int y, char *t, int col)
{
	int n;
	char *s, *m;

	n = 0;
	m = strdup(t);
	if(m == nil)
		sysfatal("txtnl: %r");
	s = strtok(m, "\n");
	while(s != nil){
		n += txt(x, y, s, col);
		s = strtok(nil, "\n");
		y += fnt->h;
	}
	free(m);
	return n;
}

int
txth(char *t)
{
	int h, n;

	h = fnt->h;
	n = h;
	while(*t != 0)
		if(*t++ == '\n')
			n += h;
	return n;
}

int
txtw(char *t)
{
	int n, m;

	n = m = 0;
	while(*t != 0){
		if(*t == '\n'){
			if(n > m)
				m = n;
			n = 0;
		}else
			n += fnt->w[(uchar)*t];
		t++;
	}
	return n > m ? n : m;
}

void
txtcen(int y, char *t, int c)
{
	int n;

	n = txtw(t);
	txt((Vw - n) / 2, y, t, c);
}

void
pic(int x, int y, int n)
{
	Pic *p;

	p = pics+n;
	pput(x, y, p->x, p->y, p->p);
}

void
pictxt(int x0, int y, char *t)
{
	int p, x, n;
	char c;

	x = x0;
	for(;;){
		c = *t++;
		n = 16;
		switch(c){
		case 0: return;
		case '\n': y += 16; x = x0 - 16; goto skip;
		case ' ': goto skip;
		case '!': p = pict[Pexcl]; n = 8; break;
		case '\'': p = pict[Papo]; n = 8; break;
		case ':': p = pict[Pcolon]; n = 8; break;
		case '%': p = pict[Ppercent]; break;
		default: p = pict[P0] + c - (c >= 'A' ? 'A'-11 : '0'); break;
		}
		pic(x, y, p);
skip:
		x += n;
	}
}

void
wlmclear(void)
{
	put(0, 0, Vw, Vh, 0x29);
}
void
sdmclear(void)
{
	pic(0, 0, pict[Pbackdrop]);
}

void
wlstripe(int y)
{
	put(0, y, Vw, 24, 0);
	put(0, y+22, 320, 1, 0x2c);
}
void
sdstripe(int y)
{
	put(0, y, Vw, 22, 0);
	put(0, y+23, 320, 1, 0);
}

void
outbox(int x, int y, int dx, int dy, int c1, int c2)
{
	put(x, y, dx, 1, c2);
	put(x, y+1, 1, dy-1, c2);
	put(x, y+dy, dx+1, 1, c1);
	put(x+dx, y, 1, dy, c1);
}

void
box(int x, int y, int dx, int dy, int col, int out, int out2)
{
	put(x+1, y+1, dx-1, dy-1, col);
	outbox(x, y, dx, dy, out, out2);
}

void
disking(void)
{
	static int n;

	pic(104, 85, pict[Pread1] + n);
	out();
	n ^= 1;
}

void
viewbox(void)
{
	int x, y;

	x = Vhud - vw.dx / 2 - 1;
	y = (Vhud - vw.dy) / 2 - 1;
	put(0, 0, 320, Vhud, 0x7f);
	box(x, y, vw.dx+1, vw.dy+1, 0, 0x7d, 0);
	put(x, y+vw.dy+1, 1, 1, 0x7c);
}

void
hudf(void)
{
	int p;

	if(gm.hp > 0){
		p = ver >= SDM && god ? pict[Pgod]
			: pict[Pface1] + 3 * (100 - gm.hp >> 4);
		p += gm.facefrm;
	}else
		p = gm.mut ? pict[Pmut] : pict[Pface8];
	pic(136, 164, p);
}

void
hudh(void)
{
	hudnp(168, 176, 3, gm.hp);
}

void
hudl(void)
{
	hudnp(112, 176, 1, gm.lives);
}

void
hudm(void)
{
	int n;

	n = ver == SOD ? (gm.map == 20 ? 18 : gm.map + 1) : gm.map % 10 + 1;
	hudnp(16, 176, 2, n);
}

void
huda(void)
{
	hudnp(216, 176, 2, gm.ammo);
}

void
hudk(void)
{
	pic(240, 164, pict[gm.keys & 1 ? Pgkey : Pnokey]);
	pic(240, 180, pict[gm.keys & 2 ? Pskey : Pnokey]);
}

void
hudw(void)
{
	pic(256, 168, pict[Pknife]+gm.w);
}

void
hudp(void)
{
	hudnp(48, 176, 6, gm.pt);
}

void
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
