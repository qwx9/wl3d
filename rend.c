#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

Fnt fnts[2], *fnt;
Pic *pics, *pice;
uchar **exts, **dems, **epis;
Dat *wals, *sprs;

int scale, npx;
uchar *px;
static uchar pxb[Va];
static Col *fcol;
static u32int *fref;
static int fi, fo, fdt;

void
fadeout(void)
{
	int i, t, u, v, w;
	u32int p, *s, *d, *e;
	Col *c;

	i = fo++;
	t = fdt;
	c = fcol;
	s = fref;
	d = pal;
	e = d+nelem(pals[0]);
	while(d < e){
		p = *s++;
		u = p & 0xff;
		v = p>>8 & 0xff;
		w = p>>16 & 0xff;
		u = u + (c->b-u) * i/t;
		v = v + (c->g-v) * i/t;
		w = w + (c->r-w) * i/t;
		*d++ = w<<16 | v<<8 | u;
	}
	out();
}

void
fadein(void)
{
	int i, t, u, v, w;
	u32int p, *s, *d, *e;
	Col *c;

	i = fi++;
	t = fdt;
	c = fcol;
	s = fref;
	d = pal;
	e = d+nelem(pals[0]);
	while(d < e){
		p = *s++;
		u = p & 0xff;
		v = p>>8 & 0xff;
		w = p>>16 & 0xff;
		u = c->b + (u-c->b) * i/t;
		v = c->g + (v-c->g) * i/t;
		w = c->r + (w-c->r) * i/t;
		*d++ = w<<16 | v<<8 | u;
	}
	out();
}

void
fadeop(Col *c, int dt)
{
	fi = 0;
	fo = 1;
	fdt = dt;
	fcol = c;
	fref = pal;
	pal = pals[Cfad];
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
