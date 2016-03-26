#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

Fnt fnts[2], *fnt;
Pic *pics, *pice;
Dat *exts, *exte;
Dat *dems, *deme, *epis;
Dat *wals, *sprs, *spre;

int scale, npx;
uchar *px;
static uchar pxb[Va];
static void (*ffp)(void);
static int fi, fdt, fr, fg, fb;
static u32int *fref;

static void
fadeout(void)
{
	int u, v, w;
	u32int p, *s, *d, *e;

	s = fref;
	d = pal;
	e = d+nelem(pals[0]);
	while(d < e){
		p = *s++;
		u = p & 0xff;
		v = p>>8 & 0xff;
		w = p>>16 & 0xff;
		u = u + (fb-u) * fi/fdt;
		v = v + (fg-v) * fi/fdt;
		w = w + (fr-w) * fi/fdt;
		*d++ = w<<16 | v<<8 | u;
	}
}

static void
fadein(void)
{
	int u, v, w;
	u32int p, *s, *d, *e;

	s = fref;
	d = pal;
	e = d+nelem(pals[0]);
	while(d < e){
		p = *s++;
		u = (p & 0xff) * fi/fdt;
		v = (p>>8 & 0xff) * fi/fdt;
		w = (p>>16 & 0xff) * fi/fdt;
		*d++ = w<<16 | v<<8 | u;
	}
}

void
fade(void)
{
	ffp();
	out();
	if(fi == fdt && ffp == fadein){
		ffp = fadeout;
		fi = 0;
	}
	fi++;
}

void
fadeop(int dt, u32int c, int noin)
{
	fdt = dt;
	fb = (c & 0xff) * 255 / 63;
	fg = (c>>8 & 0xff) * 255 / 63;
	fr = (c>>16 & 0xff) * 255 / 63;
	fref = pal;
	pal = pals[Cfad];
	fi = 1;
	ffp = noin ? fadeout : fadein;
}

void
palpic(uchar *s)
{
	u32int *p, *e;

	p = pal = pals[Csod];
	e = p + nelem(pals[0]);
	while(p < e){
		*p++ = s[0]*255/63<<16 | s[1]*255/63<<8 | s[2]*255/63;
		s += 3;
	}
}

void
out(void)
{
	int n;
	u32int c;
	uchar *s, *e, *d, *w;

	d = px;
	s = pxb;
	e = s + sizeof pxb;
	n = scale * 3;
	while(s < e){
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
put(int x, int y, int dx, int dy, uchar *s, int c)
{
	uchar *d;

	d = pxb + x + y*Vw;
	if(s != nil)
		while(dy-- > 0){
			memcpy(d, s, dx);
			s += dx;
			d += Vw;
		}
	else
		while(dy-- > 0){
			memset(d, c, dx);
			d += Vw;
		}	
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
	char *s;

	n = 0;
	s = strtok(t, "\n");
	while(s != nil){
		n += txt(x, y, s, col);
		s = strtok(nil, "\n");
		y += fnt->h;
	}
	return n;
}

int
txtw(char *t)
{
	int n;

	n = 0;
	while(*t != 0)
		n += fnt->w[(uchar)*t++];
	return n;
}

void
fill(int c)
{
	memset(pxb, c, sizeof pxb);
}

void
pic(int x, int y, int n)
{
	Pic *p;

	p = pics+n;
	put(x, y, p->x, p->y, p->p, 0);
}
