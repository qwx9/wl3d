#include <u.h>
#include <libc.h>
#include <bio.h>
#include "dat.h"
#include "fns.h"

/* what bullshit. */
uchar *pict;
static uchar picts[4][Pend]={ {
	3, 15, 8, 9, 10, 11, 24, 27, 34, 35, 23, 7, 12, 13, 14, 36, 16, 21, 25,
	26, 87, 37, 129, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
	54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
	72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 84, 0, 0, 83, 85, 86, 0, 0,
	88, 92, 93, 94, 95, 96, 106, 127, 128, 0, 0, 0, 130, 131
	},{
	14, 27, 20, 21, 22, 23, 36, 39, 46, 47, 35, 19, 24, 25, 26, 48, 28, 33,
	37, 38, 99, 49, 141, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
	65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
	83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 96, 0, 0, 95, 97, 98, 0,
	0, 100, 104, 105, 106, 107, 108, 118, 139, 140, 0, 0, 0, 142, 143
	},{
	0, 1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 22, 24, 25,
	26, 27, 0, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
	61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 0, 73, 74, 75, 0, 0, 76,
	80, 81, 82, 83, 84, 94, 115, 116, 117, 120, 122, 123, 124
	},{
	0, 1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 22, 24, 25,
	26, 27, 28, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
	66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 87, 88, 89, 90, 91,
	98, 102, 103, 104, 105, 106, 116, 137, 138, 139, 142, 144, 145, 146
	}
};
static uchar pcmt[2][46]={ {
	Sgd, Sdogbark, Sclosedoor, Sopendoor, Smg, Spistol, Sgatling, Sss, Shans,
	Shansdeath, Shansfire, Sssfire, Sscream1, Sscream2, Send, Spushwall,
	Sdogdeath, Smutdeath, Shitlerdeath, Seva, Sssdeath, Sgdfire, Sslurp,
	Sfake, Sschbdeath, Sschb, Shitler, Soffc, Soffcdeath, Sdogfire, Slvlend,
	Smechwalk, Syeah, Smechadeath, Sscream4, Sscream5, Sottodeath, Sotto,
	Sfett, Sscream6, Sscream7, Sscream8, Sscream9, Sgretel, Sgreteldeath,
	Sfettdeath
	},{
	Sgd, Sdogbark, Sclosedoor, Sopendoor, Smg, Spistol, Sgatling, Sss,
	Shansfire, Sssfire, Sscream1, Sscream2, Send, Spushwall, Sdogdeath,
	Smutdeath, Sssdeath, Sgdfire, Sslurp, Soffc, Soffcdeath, Sdogfire,
	Slvlend, Sscream4, Sscream5, Sscream6, Sscream7, Sscream8, Sscream9,
	Strans, Stransdeath, Swilh, Swilhdeath, Suberdeath, Sknight,
	Sknightdeath, Sangel, Sangeldeath, Sgetgatling, Spear
	}
};

enum{
	WL6,
	WL1,
	SDM,
	SOD,
	Nplane = 2,
	Mapdxy = 64,
	Planesz = Mapdxy * Mapdxy * Nplane,
	Mapsz = Planesz * Nplane
};
static int ver;
static Dat *snds;
static uchar *swpb, *sndb, *mapb;
static int alofs;
static u16int rlewtag;

#define	GBIT16(p)	((p)[0]|((p)[1]<<8))

static Biobuf *
bopen(char *f, int m)
{
	char s[16];
	Biobuf *bf;

	snprint(s, sizeof s, "%s%s", f, ext);
	bf = Bopen(s, m);
	if(bf == nil)
		sysfatal("bopen: %r");
	Blethal(bf, nil);
	return bf;
}

static long
eread(Biobuf *bf, void *u, long n)
{
	if(Bread(bf, u, n) != n)
		sysfatal("eread: short read: %r");
	return n;
}

static u8int
get8(Biobuf *bf)
{
	u8int v;

	eread(bf, &v, 1);
	return v;
}

static u16int
get16(Biobuf *bf)
{
	u16int v;

	v = get8(bf);
	return v | get8(bf)<<8;
}

static u32int
get24(Biobuf *bf)
{
	u32int v;

	v = get16(bf);
	return v | get8(bf)<<16;
}

static u32int
get32(Biobuf *bf)
{
	u32int v;

	v = get16(bf);
	return v | get16(bf)<<16;
}

static vlong
bsize(Biobuf *bf)
{
	vlong n;
	Dir *d;

	d = dirfstat(Bfildes(bf));
	if(d == nil)
		sysfatal("bstat: %r");
	n = d->length;
	free(d);
	return n;
}

static int
unrlew(u16int *d, u16int *s)
{
	u16int n, v, *e;
	s++;
	e = d + Planesz/Nplane;
	while(d < e){
		v = *s++;
		if(v == rlewtag){
			n = *s++;
			v = *s++;
			while(n-- > 0)
				*d++ = v;
		}else
			*d++ = v;
	}
	return Planesz;
}

static int
uncarmack(Biobuf *bf, u16int *u, u32int ofs)
{
	int len;
	u16int v, a[Planesz/2], *p, *cp, *e;
	u8int n;

	Bseek(bf, ofs, 0);
	p = a;
	e = a + get16(bf) / 2;
	while(p < e){
		v = get16(bf);
		n = v & 0xff;
		switch(v >> 8){
		tag:
			*p++ = v | get8(bf);
			break;
		copy:
			while(n-- > 0)
				*p++ = *cp++;
			break;
		case 0xa7:
			if(n == 0)
				goto tag;
			cp = p - get8(bf);
			if(cp < a)
				sysfatal("uncarmack: bad offset");
			goto copy;
		case 0xa8:
			if(n == 0)
				goto tag;
			cp = a + get16(bf);
			if(cp > p)
				sysfatal("uncarmack: bad offset");
			goto copy;
		default:
			*p++ = v;
		}
	}
	len = unrlew(u, a);
	if(len != Planesz)
		sysfatal("uncarmack: truncated lump");
	return len;
}

static void
unhuff(Biobuf* bf, u16int hf[], uchar *u, int len)
{
	int k;
	uchar *e, b;
	u16int *h, v;

	h = hf+2*254;
	e = u+len;
	k = 1;
	b = get8(bf);
	while(u < e){
		v = h[b & k ? 1 : 0];
		k <<= 1;
		if(k & 0x100){
			b = get8(bf);
			k = 1;
		}
		if(v < 256){
			*u++ = v & 0xff;
			h = hf+2*254;
		}else
			h = hf+2*(v-256);
	}
}

static void
packpcm(Biobuf *bf, Dat *e, u16int so, u16int po)
{
	u16int n;
	Dat *p, *s;

	p = s = wals+po;
	while(++p < e){
		Bseek(bf, 2, 1);
		n = get16(bf);
		while(s->sz < n)
			s->sz += p++->sz;
		while(p->sz == 0 && p < e-1)
			p++;
		s++;
		s->sz = p->sz;
		s->p = p->p;
	}
	n = s-wals;
	wals = erealloc(wals, n * sizeof *wals);
	sprs = wals + so;
	spre = wals + po;
}

static void
vswap(void)
{
	u16int n, v, w;
	u32int *o, *p;
	uchar *u;
	Dat *s, *e;
	Biobuf *bf;

	bf = bopen("vswap.", OREAD);
	n = get16(bf);
	wals = emalloc((n-1) * sizeof *wals);
	e = wals + n-1;
	v = get16(bf);
	w = get16(bf);
	p = o = emalloc(n * sizeof *o);

	while(p < o+n)
		*p++ = get32(bf);
	for(s=wals; s<wals+n-1; s++)
		s->sz = get16(bf);
	u = swpb = emalloc(bsize(bf) - Bseek(bf, 0, 1));
	Bseek(bf, 2, 1);
	for(p=o, s=wals; s<e; s++, p++){
		if(s->sz == 0)
			continue;
		Bseek(bf, *p, 0);
		s->p = u;
		u += eread(bf, u, s->sz);
	}
	Bseek(bf, *p, 0);
	free(o);
	swpb = erealloc(swpb, u-swpb);

	packpcm(bf, e-1, v, w);
	Bterm(bf);
}

static void
gamemaps(void)
{
	int n;
	u32int v, p0, p1;
	uchar *u;
	Biobuf *hed, *dat;

	hed = bopen("maphead.", OREAD);
	dat = bopen("gamemaps.", OREAD);
	n = ver==WL6 ? 60 : ver==WL1 ? 10 : ver==SDM ? 2 : 20;
	rlewtag = get16(hed);
	maps = emalloc(n * sizeof *maps);
	mapb = emalloc(n * Mapsz);

	for(mape=maps, u=mapb; mape<maps+n; mape++){
		v = get32(hed);
		if(v == 0xffffffff)
			sysfatal("sparse map %zud", mape-maps);
		Bseek(dat, v, 0);
		p0 = get32(dat);
		p1 = get32(dat);
		Bseek(dat, 10, 1);
		if(get16(dat) != Mapdxy || get16(dat) != Mapdxy)
			sysfatal("invalid map size");
		mape->p = u;
		u += uncarmack(dat, (u16int*)u, p0);
		u += uncarmack(dat, (u16int*)u, p1);
		mape->sz = u - mape->p;
	}
	Bterm(hed);
	Bterm(dat);
}

static void
swap(Sfx *a, Sfx *b)
{
	Sfx c;

	memcpy(&c, a, sizeof c);
	memcpy(a, b, sizeof c);
	memcpy(b, &c, sizeof c);
}

static void
mungesfx(void)
{
	uchar *p, *e;
	Dat *pcm;

	if(ver >= SDM){
		swap(sfxs+Sscream4, sfxs+Shansdeath);
		swap(sfxs+Sscream5, sfxs+Shitlerdeath);
		swap(sfxs+Sscream7, sfxs+Seva);
		swap(sfxs+Sscream8, sfxs+Shans);
		swap(sfxs+Sscream6, sfxs+Smechadeath);
		swap(sfxs+Sscream9, sfxs+Sschbdeath);
	}
	p = pcmt[ver<SDM ? 0 : 1];
	e = p + (ver==WL6 ? 46 : ver==WL1 ? 21 : ver==SDM ? 26 : 40);
	for(pcm=spre; p<e; p++, pcm++)
		if(*p != Send)
			sfxs[*p].pcm = pcm;
	sfxs[Sscream3].pcm = sfxs[ver<SDM ? Sscream2 : Sscream4].pcm;	/* why */
}

static void
audiot(void)
{
	int n, c;
	u32int v, w;
	uchar *u;
	Sfx *s;
	Biobuf *hed, *dat;

	hed = bopen("audiohed.", OREAD);
	dat = bopen("audiot.", OREAD);
	n = ver < SDM ? Send : Ssend;
	sfxs = emalloc(n * sizeof *sfxs);
	sfxe = sfxs + n;
	u = sndb = emalloc(bsize(dat));
	v = get32(hed);
	for(c=0, s=sfxs; s<sfxe; s++){
		w = get32(hed);
		Bseek(dat, v, 0);
		if(c++ < n){
			s->pc.sz = w-v;
			s->pc.p = u;
		}else{
			s->al.sz = w-v;
			s->al.p = u;
		}
		u += eread(dat, u, w-v);
		v = w;
		if(c == n)
			s = sfxs-1;
	}

	Bseek(hed, (n-1)*4, 1);
	n = ver < SDM ? 27 : 24;
	imfs = emalloc(n * sizeof *imfs);
	v = get32(hed);
	for(imfe=imfs; imfe<imfs+n; imfe++){
		w = get32(hed);
		Bseek(dat, v, 0);
		imfe->sz = w-v;
		imfe->p = u;
		u += eread(dat, u, w-v);
		v = w;
	}
	Bterm(hed);
	Bterm(dat);

	mungesfx();
}

static void
piched(Biobuf *dat, Biobuf *aux, u16int hf[])
{
	u32int v, n;
	uchar *u, *p;

	v = get24(aux);
	Bseek(dat, v, 0);
	n = get32(dat);
	p = u = emalloc(n);
	n /= 4;
	pics = emalloc(n * sizeof *pics);
	unhuff(dat, hf, u, n);
	for(pice=pics; pice<pics+n; pice++){
		pice->x = GBIT16(p), p+=2;
		pice->y = GBIT16(p), p+=2;
	}
	free(u);
}

static void
getpics(Biobuf *dat, Biobuf *aux, u16int hf[])
{
	u32int v, n;
	uchar *u;
	Pic *s;

	for(s=pics; s<pice; s++){
		v = get24(aux);
		Bseek(dat, v, 0);
		n = get32(dat);
		u = emalloc(n);
		unhuff(dat, hf, u, n);
		s->p = u;
		s->sz = n;
	}
	pict = picts[ver];
}

static void
getfnts(Biobuf *dat, Biobuf *aux, u16int hf[])
{
	s16int *o;
	u32int v, n;
	uchar *u, *p;
	char *w;
	Fnt *f;

	for(f=fnts; f<fnts+2; f++){
		v = get24(aux);
		Bseek(dat, v, 0);
		n = get32(dat);
		p = u = emalloc(n);
		unhuff(dat, hf, u, n);
		f->h = GBIT16(p), p+=2;
		for(o=f->ofs; o < f->ofs+nelem(f->ofs); o++)
			*o = GBIT16(p), p+=2;
		for(w=f->w; w < f->w+nelem(f->w); w++)
			*w = *p++;
		n -= p-u;
		f->p = emalloc(n);
		f->sz = n;
		memcpy(f->p, p, n);
		free(u);
	}
}

static void
getexts(Biobuf *dat, Biobuf *aux, u16int hf[])
{
	int n, m;
	uchar *u;
	u32int v;

	n = (bsize(aux) - Bseek(aux, 0, 1)) / 3 - 1;
	exts = emalloc(n * sizeof *exts);
	for(exte=exts; exte<exts+n; exte++){
		v = get24(aux);
		Bseek(dat, v, 0);
		m = get32(dat);
		u = emalloc(m);
		unhuff(dat, hf, u, m);
		exte->p = u;
		exte->sz = m;
	}
	dems = exts + (ver==WL6 || ver==SDM ? 3 : ver==SOD ? 13 : 0);
	deme = dems + (ver==WL6 || ver==SOD ? 4 : ver==SDM ? 1 : 0);
	epis = exts + (ver==WL6 ? 7 : 17);
}

static void
gfx(void)
{
	u16int hf[512], *h;
	Biobuf *dat, *aux;

	aux = bopen("vgadict.", OREAD);
	for(h=hf; h<hf+nelem(hf); h++)
		*h = get16(aux);
	Bterm(aux);

	aux = bopen("vgahead.", OREAD);
	dat = bopen("vgagraph.", OREAD);
	piched(dat, aux, hf);
	getfnts(dat, aux, hf);
	getpics(dat, aux, hf);
	get24(aux);	/* ignore bullshit tile lump full of lies */
	getexts(dat, aux, hf);
	Bterm(aux);
	Bterm(dat);
}

static void
version(void)
{
	if(strcmp(ext, "wl6") == 0)
		ver = WL6;
	else if(strcmp(ext, "wl1") == 0)
		ver = WL1;
	else if(strcmp(ext, "sdm") == 0)
		ver = SDM;
	else
		ver = SOD;
}

void
dat(char *dir)
{
	rfork(RFNAMEG);
	if(bind(".", dir, MBEFORE|MCREATE) < 0 || chdir(dir) < 0)
		fprint(2, "dat: %r\n");
	version();
	vswap();
	gamemaps();
	if(ver == SOD)
		ext = "sod";
	audiot();
	gfx();
}
