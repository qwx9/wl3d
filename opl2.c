#include <u.h>
#include <libc.h>

typedef struct Envelope Envelope;
typedef struct Phase Phase;
typedef struct Op Op;
typedef struct Chan Chan;

enum{
	Rwse = 0x01,
		Mwse = 1<<5,	/* wave selection enable */
	Rcsm = 0x08,
		Mnse = 1<<6,	/* note selection enable */
	Rctl = 0x20,
		Mame = 1<<7,	/* enable amplitude modulation */
		Mvbe = 1<<6,	/* enable vibrato */
		Msse = 1<<5,	/* enable sustain */
		Mkse = 1<<4,	/* enable keyboard scaling */
		Mmfq = 15<<0,	/* modulator freq multiple */
	Rsca = 0x40,
		Mlvl = 63<<0,	/* total level */
		Mscl = 3<<6,	/* scaling level */
	Ratk = 0x60,
		Mdec = 15<<0,	/* decay rate */
		Matk = 15<<4,	/* attack rate */
	Rsus = 0x80,
		Mrel = 15<<0,	/* release rate */
		Msus = 15<<4,	/* sustain level */
	Rnum = 0xa0,		/* f number lsb */
	Roct = 0xb0,
		Mmsb = 3<<0,	/* f number msb */
		Moct = 7<<2,
		Mkon = 1<<5,
	Ropm = 0xbd,
		Mamp = 1<<7,	/* amplitude mod depth */
		Mvib = 1<<6,	/* vibrato depth */
		Mrms = 63<<0,
		Mrhy = 1<<5,	/* rhythm enable */
		Mbas = 1<<4,
		Msna = 1<<3,
		Mtom = 1<<2,
		Mcym = 1<<1,
		Mhat = 1<<0,
	Rfed = 0xc0,
		Mmod = 1<<0,	/* enable operator modulation */
		Mfed = 7<<1,	/* feedback strength */
	Rwav = 0xe0,
		Mwav = 3<<0,

	Sfreq = 16,
	Sfreqd = Sfreq - 10,
	Seg = 16,
	Slfo = 24,
	Stm = 16,

	Mfreq = (1 << Sfreq) - 1,

	Lampb = 12,
	Lsignb = 2,
	Ly = Lampb * Lsignb,
	Lx = 256,
	Lxy = Lx * Ly,

	Sinb = 10,
	Sinlen = 1<<Sinb,
	Sinm = Sinlen - 1,
	Nsines = 4,

	Erstep = 8,
	Equiet = Lxy >> 4,
	Eb = 10,
	Elen = 1 << Eb,
	Nerates = 16 + 16 * 4 + 16,	/* infinite, regular, dummy */
	Nlfoa = 210,
	Attmax = (1 << Eb-1) - 1,
	Attmin = 0
};
#define Clk	3579545.0
#define Estep	(128.0/Elen)

enum{
	Eoff,
	Erel,
	Esus,
	Edec,
	Eatk,
};
struct Envelope{
	int state;
	int key;
	int son;
	int sus;
	int atksh;
	int atk;
	int decsh;
	int dec;
	int relsh;
	int rel;
	int lvl0;
	int lvl;
	s32int vol;
	u32int tl;
};
struct Phase{
	u32int fq;
	u32int dfq;
	int fbv;
	int opmodoff;
	int out[2];
	int *p;
};
struct Op{
	Envelope;
	Phase;
	int Aon;
	int φon;
	int atkn;
	int decn;
	int reln;
	int mul;
	int scl;
	int ks;
	int kssh;
	u16int wav;
};
struct Chan{
	Op *op;
	u32int scl0;
	u32int dfq0;
	int fn;
	int oct;
	int kcode;
};
static int lta[Lxy];
static uint sint[Sinlen * Nsines];
static int wse, nse, rhythm;
static u32int noise, noiseφ, noiseT;
static u32int egtic, egt, egdt, egfdt;
static u32int lfoA, lfoAt, lfoAdt, lfoφt, lfoφdt;
static s32int lfoφ;
static int lfoAd, lfoφd;
static Chan chs[9];
static Op ops[2*nelem(chs)];
static int φmod, tout;

static u32int fn[1024];	/* 20bit φ increment counter */
/* 27 output levels (triangle waveform), 1 level takes one of 192, 256 or 448
 * samples. each value is repeated on 64 consecutive samples. total length is
 * then 64*210 samples. when am=1 data is used directly, else it is shl 2 before
 * use. */
static u8int lfoAs[Nlfoa] = {
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5,
	5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10,
	11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15,
	15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 19,
	20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23, 24, 24,
	24, 24, 25, 25, 25, 25, 26, 26, 26, 25, 25, 25, 25, 24, 24, 24, 24, 23,
	23, 23, 23, 22, 22, 22, 22, 21, 21, 21, 21, 20, 20, 20, 20, 19, 19, 19,
	19, 18, 18, 18, 18, 17, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 15, 14,
	14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10,
	10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4,
	4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1
};
/* 8 output 1024 samples long levels (triangle waveform) with 16 values (depth 0
 * then depth 1) each */
static s8int lfoφs[8*8*2] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 0,
	1, 0, 0, 0, -1, 0, 0, 0, 2, 1, 0, -1, -2, -1, 0, 1,
	1, 0, 0, 0, -1, 0, 0, 0, 3, 1, 0, -1, -3, -1, 0, 1,
	2, 1, 0, -1, -2, -1, 0, 1, 4, 2, 0, -2, -4, -2, 0, 2,
	2, 1, 0, -1, -2, -1, 0, 1, 5, 2, 0, -2, -5, -2, 0, 2,
	3, 1, 0, -1, -3, -1, 0, 1, 6, 3, 0, -3, -6, -3, 0, 3,
	3, 1, 0, -1, -3, -1, 0, 1, 7, 3, 0, -3, -7, -3, 0, 3
};
static uchar mul[16] = {
	1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30
};
#define O	(0.1875 / 2.0)	/* convert 3 dB/oct → 6 dB/oct */
static u32int lsca[8 * 16] = {	/* 0.1875: bit0 weight of op->vol in dB */
	0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O,
	0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O,
	0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O,
	0.000/O, 0.750/O, 1.125/O, 1.500/O, 1.875/O, 2.250/O, 2.625/O, 3.000/O,
	0.000/O, 0.000/O, 0.000/O, 0.000/O, 0.000/O, 1.125/O, 1.875/O, 2.625/O,
	3.000/O, 3.750/O, 4.125/O, 4.500/O, 4.875/O, 5.250/O, 5.625/O, 6.000/O,
	0.000/O, 0.000/O, 0.000/O, 1.875/O, 3.000/O, 4.125/O, 4.875/O, 5.625/O,
	6.000/O, 6.750/O, 7.125/O, 7.500/O, 7.875/O, 8.250/O, 8.625/O, 9.000/O,
	0.000/O, 0.000/O, 3.000/O, 4.875/O, 6.000/O, 7.125/O, 7.875/O, 8.625/O,
	9.000/O, 9.750/O, 10.125/O, 10.500/O, 10.875/O, 11.250/O, 11.625/O,
	12.000/O,
	0.000/O, 3.000/O, 6.000/O, 7.875/O, 9.000/O, 10.125/O, 10.875/O,
	11.625/O, 12.000/O, 12.750/O, 13.125/O, 13.500/O, 13.875/O, 14.250/O,
	14.625/O, 15.000/O,
	0.000/O, 6.000/O, 9.000/O, 10.875/O, 12.000/O, 13.125/O, 13.875/O,
	14.625/O, 15.000/O, 15.750/O, 16.125/O, 16.500/O, 16.875/O, 17.250/O,
	17.625/O, 18.000/O,
	0.000/O, 9.000/O, 12.000/O, 13.875/O, 15.000/O, 16.125/O, 16.875/O,
	17.625/O, 18.000/O, 18.750/O, 19.125/O, 19.500/O, 19.875/O, 20.250/O,
	20.625/O, 21.000/O
};
#undef O
#define O(n)	(n * (2.0/Estep))	/* n*3 dB */
static u32int sust[16] = {
	O(0), O(1), O(2), O(3), O(4), O(5), O(6), O(7), O(8), O(9), O(10),
	O(11), O(12), O(13), O(14), O(31)
};
#undef O
static uchar estep[15*Erstep] = {
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1,
	0, 1, 1, 1, 1, 1, 1, 1,	/* rate 0-12 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
	1, 2, 2, 2, 1, 2, 2, 2,	/* rate 13 */
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4,
	2, 4, 4, 4, 2, 4, 4, 4, /* rate 14 */
	4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8, /* rate 15 + atk */
	0, 0, 0, 0, 0, 0, 0, 0, /* rate ∞ (atk/dec) */
};
#define O(n)	(n * Erstep)
static uchar erate[Nerates] = {	/* O(13) is directly in code */
	O(14), O(14), O(14), O(14), O(14), O(14), O(14), O(14), O(14), O(14),
	O(14), O(14), O(14), O(14), O(14), O(14), O(0), O(1), O(2), O(3), O(0),
	O(1), O(2), O(3), O(0), O(1), O(2), O(3), O(0), O(1), O(2), O(3), O(0),
	O(1), O(2), O(3), O(0), O(1), O(2), O(3), O(0), O(1), O(2), O(3), O(0),
	O(1), O(2), O(3), O(0), O(1), O(2), O(3), O(0), O(1), O(2), O(3), O(0),
	O(1), O(2), O(3), O(0), O(1), O(2), O(3), O(0), O(1), O(2), O(3), O(4),
	O(5), O(6), O(7), O(8), O(9), O(10), O(11), O(12), O(12), O(12), O(12),
	O(12), O(12), O(12), O(12), O(12), O(12), O(12), O(12), O(12), O(12),
	O(12), O(12), O(12), O(12), O(12), O(12)
};
#undef O
static uchar eratesh[Nerates] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 12, 12, 12, 11, 11,
	11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6,
	5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
};

/* 23-bit shift register noise generator. T=2²³-2 samples, sampling rate equal
 * to that of the chip. */
static void
noiseg(void)
{
	int i;

	noiseφ += noiseT;
	i = noiseφ >> Sfreq;
	noiseφ &= Mfreq;
	while(i-- > 0){
		if(noise & 1)
			noise ^= 0x800302;
		noise >>= 1;
	}
}

static void
advlfo(void)
{
	int a;

	lfoAt += lfoAdt;
	if(lfoAt >= (u32int)Nlfoa << Slfo)
		lfoAt -= (u32int)Nlfoa << Slfo;
	a = lfoAs[lfoAt >> Slfo];
	lfoA = lfoAd ? a : a>>2;
	lfoφt += lfoφdt;
	lfoφ = lfoφt >> Slfo & 7 | lfoφd;
}

static void
adv(void)
{
	int i, f, oc, v;
	Chan *c;
	Op *o;

	egt += egfdt;
	while(egt >= egdt){
		egt -= egdt;
		egtic++;
		o = ops;
		while(o < ops + nelem(ops)){
			switch(o->state){
			case Eatk:
				if(egtic & (1 << o->atksh)-1)
					break;
				v = o->atk + (egtic >> o->atksh & 7);
				/* sign extend it */
				o->vol += (s32int)(~o->vol * estep[v]) >> 3;
				if(o->vol <= Attmin){
					o->vol = Attmin;
					o->state = Edec;
				}
				break;
			case Edec:
				if(egtic & (1 << o->decsh)-1)
					break;
				v = o->dec + (egtic >> o->decsh & 7);
				o->vol += estep[v];
				if(o->vol >= o->sus)
					o->state = Esus;
				break;
			case Esus:
				if(o->son || egtic & (1 << o->relsh)-1)
					break;
				v = o->rel + (egtic >> o->relsh & 7);
				o->vol += estep[v];
				if(o->vol >= Attmax)
					o->vol = Attmax;
				break;
			case Erel:
				if(egtic & (1 << o->relsh)-1)
					break;
				v = o->rel + (egtic >> o->relsh & 7);
				o->vol += estep[v];
				if(o->vol >= Attmax){
					o->vol = Attmax;
					o->state = Eoff;
				}
				break;
			}
			o++;
		}
	}
	o = ops;
	while(o < ops + nelem(ops)){
		c = chs + (o-ops)/2;
		if(o->φon){
			f = c->fn;
			i = lfoφs[lfoφ + (f >> 7 << 4)];
			if(i != 0){
				f += i;
				oc = 7 - c->oct + (f >> 10);
				o->fq += (fn[f & 0x3ff] >> oc) * o->mul;
			}else
				o->fq += o->dfq;
		}else
			o->fq += o->dfq;
		o++;
	}
	noiseg();
}

static int
op(u32int φ, u32int v, int dφ, u16int w)
{
	u32int p;

	p = (v<<4) + sint[w + ((int)((φ & ~Mfreq) + dφ) >> Sfreq & Sinm)];
	return p < Lxy ? lta[p] : 0;
}

static void
chan(Op *o)
{
	u32int v;
	int out;

	φmod = 0;
	out = o->out[0] + o->out[1];
	o->out[0] = o->out[1];
	*o->p += o->out[0];
	o->out[1] = 0;
	v = o->tl;
	if(v < Equiet){
		if(o->fbv == 0)
			out = 0;
		o->out[1] = op(o->fq, v, out << o->fbv, o->wav);
	}
	o++, v = o->tl;
	if(v < Equiet)
		tout += op(o->fq, v, φmod << 16, o->wav);
}

static void
rchan(void)
{
	int out, nr;
	u32int u, v;
	Op *o;

	nr = noise & 1;
	φmod = 0;
	o = chs[6].op;
	out = o->out[0] + o->out[1];
	o->out[0] = o->out[1];
	if(o->opmodoff == 0)
		φmod = o->out[0];
	o->out[1] = 0;

	v = o->tl;
	if(v < Equiet){
		if(o->fbv == 0)
			out = 0;
		o->out[1] = op(o->fq, v, out << o->fbv, o->wav);
	}
	o++, v = o->tl;
	if(v < Equiet)
		tout += op(o->fq, v, φmod << 16, o->wav) * 2;
	o++, v = o->tl;
	if(v < Equiet){
		u = ops[17].fq >> Sfreq;
		if((u ^ u<<2) & 1<<5)
			u = 0x200 | 0xd0 >> 2;
		else{
			u = o->fq >> Sfreq;
			u = ((u ^ u<<5 | u<<4) & 1<<7) ? 0x200 | 0xd0>>2 : 0xd0;
		}
		if(nr)
			u = u & 0x200 ? 0x200 | 0xd0 : 0xd0>>2;
		tout += op(u << Sfreq, v, 0, o->wav) * 2;
	}
	o++, v = o->tl;
	if(v < Equiet){
		u = (ops[14].fq >> Sfreq & 1<<8) ? 1<<9 : 1<<8;
		u = (u ^ nr << 8) << Sfreq;
		tout += op(u, v, 0, o->wav) * 2;
	}
	o++, v = o->tl;
	if(v < Equiet)
		tout += op(o->fq, v, 0, o->wav) * 2;
	o++, v = o->tl;
	if(v < Equiet){
		u = o->fq >> Sfreq;
		if((u ^ u<<2) & 1<<5)
			u = 0x300;
		else{
			u = ops[14].fq >> Sfreq;
			u = ((u ^ u<<5 | u<<4) & 1<<7) ? 0x300 : 0x100;
		}
		tout += op(u << Sfreq, v, 0, o->wav) * 2;
	}
}

static void
setk(Op *o, int k, int on)
{
	if(on){
		if(o->key == 0){
			o->fq = 0;
			o->state = Eatk;
		}
		o->key |= k;
	}else{
		if(o->key == 0)
			return;
	
		o->key &= ~k;
		if(o->key == 0 && o->state > Erel)
			o->state = Erel;
	}
}

static void
upop(Chan *c, Op *o)
{
	int n;

	o->dfq = c->dfq0 * o->mul;
	n = c->kcode >> o->kssh;

	if(o->ks != n){
		o->ks = n;
		if(o->atkn + n < 16+62){
			o->atksh = eratesh[o->atkn + n];
			o->atk = erate[o->atkn + n];
		}else{
			o->atksh = 0;
			o->atk = 13 * Erstep;
		}
		o->decsh = eratesh[o->decn + n];
		o->dec = erate[o->decn + n];
		o->relsh = eratesh[o->reln + n];
		o->rel = erate[o->reln + n];
	}
}

static void
ctl(Chan *c, Op *o, int v)
{
	o->mul = mul[v & Mmfq];
	o->kssh = v & Mkse ? 0 : 2;
	o->son = v & Msse;
	o->φon = v & Mvbe;
	o->Aon = v & Mame ? ~0 : 0;
	upop(c, o);
}

static void
sca(Chan *c, Op *o, int v)
{
	uchar u;

	u = v >> 6;
	o->scl = u ? u % 3 : 31;	/* shift to 0, 3, 1.5, 6 dB/oct */
	o->lvl0 = (v & Mlvl) << Eb-1-7;
	o->lvl = o->lvl0 + (c->scl0 >> o->scl);
}

static void
atkdec(Op *o, int v)
{
	int a, n;

	n = o->ks;
	o->atkn = a = v >> 4 ? 16 + (v >> 4 << 2) : 0;
	a += n;
	if(a < 16+62){
		o->atksh = eratesh[a];
		o->atk = erate[a];
	}else{
		o->atksh = 0;
		o->atk = 13 * Erstep;
	}
	o->decn = a = v & Mdec ? 16 + ((v & Mdec) << 2) : 0;
	a += n;
	o->decsh = eratesh[a];
	o->dec = erate[a];
}

static void
susrel(Op *o, int v)
{
	o->sus = sust[v >> 4];
	o->reln = v & Mrel ? 16 + ((v & Mrel) << 2) : 0;
	o->relsh = eratesh[o->reln + o->ks];
	o->rel = erate[o->reln + o->ks];
}

static void
opm(int v)
{
	lfoAd = v & Mamp;
	lfoφd = v & Mvib ? 8 : 0;
	rhythm = v & Mrms;
	if(v & ~Mrhy)
		v = 0;
	setk(chs[6].op, 2, v & Mbas);
	setk(chs[6].op+1, 2, v & Mbas);
	setk(chs[7].op, 2, v & Mhat);
	setk(chs[7].op+1, 2, v & Msna);
	setk(chs[8].op, 2, v & Mtom);
	setk(chs[8].op+1, 2, v & Mcym);
}

static void
foct(int r, int v)
{
	int n, b, o, f;
	u32int u;
	Chan *c;
	Op *op0, *op1;

	n = r & 0xf;
	if(n > 8)
		return;
	c = chs+n;
	op0 = c->op;
	op1 = op0 + 1;
	o = c->oct;
	if(r & 0x10){	/* Roct */
		f = c->fn & 0xff | (v & Mmsb) << 8;
		o = (v & Moct) >> 2;
		setk(op0, 1, v & Mkon);
		setk(op1, 1, v & Mkon);
	}else
		f = c->fn & 0x300 | v;
	b = f | o << 10;

	if(c->fn != f || c->oct != o){
		c->fn = f;
		c->oct = o;
		c->scl0 = u = lsca[b >> 6];
		c->dfq0 = fn[f] >> 7 - o;
		/* ignore manual full of lies */
		c->kcode = o << 1 | (nse ? f >> 8 : f >> 9) & 1;
		op0->lvl = op0->lvl0 + (u >> op0->scl);
		op1->lvl = op1->lvl0 + (u >> op1->scl);
		upop(c, op0);
		upop(c, op1);
	}
}

static void
feedb(int r, int v)
{
	int n;
	Op *o;

	if(r > 8)
		return;
	n = v & Mmod;
	v = (v & Mfed) >> 1;
	o = chs[r].op;
	o->opmodoff = n;
	o->fbv = v ? v + 7 : 0;
	o->p = n ? &tout : &φmod;
}

static void
tab(void)
{
	int i, x, n, *l, *p;
	uint *s;
	double o, m;

	for(x=0; x<Lx; x++){
		m = (1<<16) / pow(2, (x+1) * (Estep/4.0) / 8.0);
		m = floor(m);
		n = (int)m >> 4;     /* always fits in 12 (16) bits */
		n = (n>>1) + (n&1) << 1;	/* rnr */
		l = lta + x*2;
		l[0] = n;
		l[1] = -n;
		for(i=1, p=l; i<12; i++){
			p += 2*Lx;
			p[0] = n >> i;
			p[1] = -(n >> i);
		}
	}

	for(i=0, s=sint; i<Sinlen; i++){
		m = sin((i*2+1) * PI / Sinlen);
		o = 8 * log((m > 0.0 ? 1.0 : -1.0)/m) / log(2.0);	/* dB */
		o /= Estep / 4.0;
		n = o * 2.0;
		n = (n>>1) + (n&1);	/* rnr */
		*s++ = n*2 + (m >= 0.0 ? 0 : 1);
	}
	for(i=0, s=sint; i<Sinlen; i++, s++){
		/* half-sine, abs-sine, pulse-sine */
		s[Sinlen] = i & 1<<Sinb-1 ? Lxy : *s;
		s[Sinlen*2] = sint[i & Sinm>>1];
		s[Sinlen*3] = i & 1<<Sinb-2 ? Lxy : sint[i & Sinm>>2];
	}
}

uchar *
opl2out(uchar *s, int n)
{
	int v, r;
	uchar *e;
	Op *o;

	r = rhythm & Mrhy;
	e = s + n;
	while(s < e){
		tout = 0;
		advlfo();
		for(o=ops; o<ops+nelem(ops); o++)
			o->tl = o->lvl + (u32int)o->vol + (lfoA & o->Aon);
		for(o=ops; o<ops+nelem(ops); o+=2){
			if(o == ops+6*2 && r){
				rchan();
				break;
			}
			chan(o);
		}
		v = tout;
		if(v > 32767)
			v = 32767;
		else if(v < -32768)
			v = -32768;
		s[0] = s[2] = v;
		s[1] = s[3] = v>>8;
		s += 4;
		adv();
	}
	return s;
}

void
opl2wr(int r, int v)
{
	int n;
	Op *o;
	Chan *c;

	v &= 0xff;
	n = r & 0x1f;
	c = chs + ((n>>3)+(n>>3<<1) + (n+(n+1>>2&1)&3));
	o = c->op + (n + 1 >> 2 & 1);
	n = r & 6 ^ 6 && n < 22;

	switch(r){
	case Rwse: wse = v & Mwse; return;
	case Rcsm: nse = v & Mnse; return;
	case Ropm: opm(v); return;
	}
	switch(r & 0xe0){
	case Rctl: if(n) ctl(c, o, v); break;
	case Rsca: if(n) sca(c, o, v); break;
	case Ratk: if(n) atkdec(o, v); break;
	case Rsus: if(n) susrel(o, v); break;
	case Rnum: foct(r, v); break;
	case Rfed: feedb(r & 0xf, v); break;
	case Rwav: if(wse && n) o->wav = (v & Mwav) * Sinlen; break;
	}
}

void
opl2init(int rate)
{
	int i;
	u32int *fp;
	double f0, n;
	Chan *c;
	Op *o;

	tab();
	f0 = (Clk / 72.0) / rate;
	fp = fn;
	n = 0;
	while(fp < fn+nelem(fn))
		*fp++ = n++ * 64 * f0 * (1 << Sfreqd);
	lfoAdt = (1.0 / 64.0) * (1 << Slfo) * f0;
	lfoφdt = (1.0 / 1024.0) * (1 << Slfo) * f0;
	noiseT = 1.0 * (1 << Sfreq) * f0;
	egfdt = (1 << Seg) * f0;
	egdt = 1 << Seg;
	noise = 1;

	c = chs;
	o = ops;
	while(c < chs+nelem(chs)){
		c++->op = o;
		o->state = Eoff;
		o++->vol = Attmax;
		o->state = Eoff;
		o++->vol = Attmax;
	}
	for(i=Rctl; i<Rwav+22; i++)
		opl2wr(i, 0);
}
