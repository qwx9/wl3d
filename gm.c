#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>
#include "dat.h"
#include "fns.h"

extern Channel *csc;
extern QLock inlck;

Rune keys[Ke] = {
	[K↑] Kup,
	[K↓] Kdown,
	[K←] Kleft,
	[K→] Kright,
	[Krun] Kshift,
	[Kfire] Kctl,
	[Kopen] ' ',
	[Kstrafe] Kalt,
	[Kknife] '1',
	[Kpistol] '2',
	[Kmg] '3',
	[Kgatling] '4',
	[Kmenu] Kesc
};
Game gm;
int msense;
int god, noclip, onestep;

enum{
	Ncrm = 7
};

typedef struct Crm Crm;
struct Crm{
	char s[Ncrm];
	void (*f)(void);
};

static int rndi, rndt[] = {
  0, 8, 109, 220, 222, 241, 149, 107, 75, 248, 254, 140, 16, 66, 74, 21,
  211, 47, 80, 242, 154, 27, 205, 128, 161, 89, 77, 36, 95, 110, 85, 48,
  212, 140, 211, 249, 22, 79, 200, 50, 28, 188, 52, 140, 202, 120, 68, 145,
  62, 70, 184, 190, 91, 197, 152, 224, 149, 104, 25, 178, 252, 182, 202, 182,
  141, 197, 4, 81, 181, 242, 145, 42, 39, 227, 156, 198, 225, 193, 219, 93,
  122, 175, 249, 0, 175, 143, 70, 239, 46, 246, 163, 53, 163, 109, 168, 135,
  2, 235, 25, 92, 20, 145, 138, 77, 69, 166, 78, 176, 173, 212, 166, 113,
  94, 161, 41, 50, 239, 49, 111, 164, 70, 60, 2, 37, 171, 75, 136, 156,
  11, 56, 42, 146, 138, 229, 73, 146, 77, 61, 98, 196, 135, 106, 63, 197,
  195, 86, 96, 203, 113, 101, 170, 247, 181, 113, 80, 250, 108, 7, 255, 237,
  129, 226, 79, 107, 112, 166, 103, 241, 24, 223, 239, 120, 198, 58, 60, 82,
  128, 3, 184, 66, 143, 224, 145, 224, 81, 206, 163, 45, 63, 90, 168, 114,
  59, 33, 159, 95, 28, 139, 123, 98, 125, 196, 15, 70, 194, 253, 54, 14,
  109, 226, 71, 17, 161, 93, 186, 87, 244, 138, 20, 52, 123, 251, 26, 36,
  17, 46, 52, 231, 232, 76, 31, 221, 84, 37, 216, 165, 212, 106, 197, 242,
  98, 43, 39, 175, 254, 145, 190, 84, 118, 222, 187, 136, 120, 163, 236, 249
};
static int demfrm;
static int kon, kold, kΔx, kΔy;
static s16int kΔθ;
static int allrecv, firing, noise;
static char *dem, *deme;
static Crm crms[];
static char crs[Ncrm];
static int dmgtc, bonustc, facetc, funtc;
static Obj *camobj;
static int bosskillx, bosskilly;
static int gotspear, spearx, speary, spearθ;
static int atk[][4] = {
	{0, 2, 0, -1},
	{0, 1, 0, -1},
	{0, 1, 3, -1},
	{0, 1, 4, -1}
};
static int atkfrm, atktc;
static int dofizz;

static void
givea(int n)
{
	if(gm.ammo == 0 && atkfrm == 0){
		gm.w = gm.lastw;
		hudw();
	}
	gm.ammo += n;
	if(gm.ammo > 99)
		gm.ammo = 99;
	huda();
}

static void
givew(int n)
{
	givea(6);
	if(gm.bestw < n)
		gm.w = gm.lastw = gm.bestw = n;
	hudw();
}

static void
givek(int n)
{
	gm.keys |= 1 << n;
	hudk();
}

static void
givel(void)
{
	if(gm.lives < 9)
		gm.lives++;
	hudl();
	sfx(S1up);
}

static void
givep(int n)
{
	gm.pt += n;
	while(gm.pt >= gm.to1up){
		gm.to1up += 40000;
		givel();
	}
	hudp();
}

static void
giveh(int n)
{
	gm.hp += n;
	if(gm.hp > 100)
		gm.hp = 100;
	hudh();
	hudf();
}

static void
slurp(Obj *)
{
	sfx(Sslurp);
}

static void
mechsfx(Obj *o)
{
	if(plrarea[o->areaid])
		sfxatt(Smechwalk, 1, o->x, o->y);
}

static void
tiredsfx(Obj *)
{
	sfx(Sangeltired);
}

static void
victory(Obj *)
{
	gm.end = EDwon;
}

static void
oattack(Obj *o)
{
	switch(o->type){
	case Ogd:
		sfxatt(Sgd, 1, o->x, o->y);
		ostate(o, stt+GSgdchase1);
		o->v *= 3;
		break;
	case Oofc:
		sfxatt(Sofc, 1, o->x, o->y);
		ostate(o, stt+GSofcchase1);
		o->v *= 5;
		break;
	case Omut:
		ostate(o, stt+GSmutchase1);
		o->v *= 3;
		break;
	case Oss:
		sfxatt(Sss, 1, o->x, o->y);
		ostate(o, stt+GSsschase1);
		o->v *= 4;
		break;
	case Odog:
		sfxatt(Sdog, 1, o->x, o->y);
		ostate(o, stt+GSdogchase1);
		o->v *= 2;
		break;
	case Ohans:
		sfx(Shans);
		ostate(o, stt+GShanschase1);
		o->v = 512 * 3;
		break;
	case Oschb:
		sfx(Sschb);
		ostate(o, stt+GSschbchase1);
		o->v = 512 * 3;
		break;
	case Ogretel:
		sfx(Sgretel);
		ostate(o, stt+GSgretelchase1);
		o->v = 512 * 3;
		break;
	case Ootto:
		sfx(Sotto);
		ostate(o, stt+GSottochase1);
		o->v = 512 * 3;
		break;
	case Ofett:
		sfx(Sfett);
		ostate(o, stt+GSfettchase1);
		o->v = 512 * 3;
		break;
	case Ofake:
		sfx(Sfake);
		ostate(o, stt+GSfakechase1);
		o->v = 512 * 3;
		break;
	case Omech:
		sfx(Shitlerdie);
		ostate(o, stt+GSmechchase1);
		o->v = 512 * 3;
		break;
	case Ohitler:
		sfx(Shitlerdie);
		ostate(o, stt+GShitlerchase1);
		o->v *= 5;
		break;
	case Oghost:
		ostate(o, stt+GSgh1chase1);
		o->v *= 2;
		break;
	case Ospectre:
		sfx(Sghost);
		ostate(o, stt+GSspectrechase1);
		o->v = 800;
		break;
	case Oangel:
		sfx(Sangel);
		ostate(o, stt+GSangelchase1);
		o->v = 1536;
		break;
	case Otrans:
		sfx(Strans);
		ostate(o, stt+GStranschase1);
		o->v = 1536;
		break;
	case Ouber:
		ostate(o, stt+GSuberchase1);
		o->v = 3000;
		break;
	case Owilh:
		sfx(Swilh);
		ostate(o, stt+GSwilhchase1);
		o->v = 2048;
		break;
	case Oknight:
		sfx(Sknight);
		ostate(o, stt+GSknightchase1);
		o->v = 2048;
		break;
	}
	if(o->Δr < 0)
		o->Δr = 0;
	o->f |= OFattack | OFflip;
}

static void
mechblow(Obj *o)
{
	Obj *p;

	p = ospawn(o->tl, stt+GShitlerchase1);
	p->v = 2560;
	p->x = o->x;
	p->y = o->y;
	p->Δr = o->Δr;
	p->θ = o->θ;
	p->f = o->f | OFshootable;
	p->type = Ohitler;
	p->hp = gm.difc>GDbaby ? 600 + gm.difc * 100 : 500;
}

static void
yelp(Obj *o)
{
	int n, s[] = {
		Sscream1, Sscream2, Sscream3, Sscream4,
		Sscream5, Sscream7, Sscream8, Sscream9
	};

	n = gm.map;
	if((ver == WL6 && n % 10 == 9 || ver == SOD && (n == 18 || n == 19)) && !rnd())
		switch(o->type){
		case Ogd:
		case Oofc:
		case Oss:
		case Odog:
		case Omut:
			sfxatt(Sscream6, 1, o->x, o->y);
			return;
		}

	switch(o->type){
	case Omut: sfxatt(Smutdie, 1, o->x, o->y); break;
	case Ogd: sfxatt(s[rnd()%(ver==WL1?2:8)], 1, o->x, o->y); break;
	case Oofc: sfxatt(Sofcdie, 1, o->x, o->y); break;
	case Oss: sfxatt(Sssdie, 1, o->x, o->y); break;
	case Odog: sfxatt(Sdogdie, 1, o->x, o->y); break;
	case Ohans: sfx(Shansdie); break;
	case Oschb: sfx(Sschbdie); break;
	case Ogretel: sfx(Sgreteldie); break;
	case Ootto: sfx(Sottodie); break;
	case Ofett: sfx(Sfettdie); break;
	case Ofake: sfx(Shitler); break;
	case Omech: sfx(Smechdie); break;
	case Ohitler: sfx(Seva); break;
	case Otrans: sfx(Stransdie); break;
	case Owilh: sfx(Swilhdie); break;
	case Ouber: sfx(Suberdie); break;
	case Oknight: sfx(Sknightdie); break;
	case Ospectre: sfx(Sghostdie); break;
	case Oangel: sfx(Sangeldie); break;
	}
}

static void
odie(Obj *o)
{
	Tile *tl;

	o->tx = o->x >> Dtlshift;	/* drop item on center */
	o->ty = o->y >> Dtlshift;
	tl = tiles + o->ty*Mapdxy + o->tx;
	o->tl = tl;
	switch(o->type){
	case Ogd:
		givep(100);
		ostate(o, stt+GSgddie1);
		drop(tl, Rclip2);
		break;
	case Oofc:
		givep(400);
		ostate(o, stt+GSofcdie1);
		drop(tl, Rclip2);
		break;
	case Omut:
		givep(700);
		ostate(o, stt+GSmutdie1);
		drop(tl, Rclip2);
		break;
	case Oss:
		givep(500);
		ostate(o, stt+GSssdie1);
		drop(tl, gm.bestw < WPmg ? Rmg : Rclip2);
		break;
	case Odog:
		givep(200);
		ostate(o, stt+GSdogdie1);
		break;
	case Ohans:
		givep(5000);
		ostate(o, stt+GShansdie1);
		drop(tl, Rkey1);
		break;
	case Oschb:
		givep(5000);
		bosskillx = oplr->x;
		bosskilly = oplr->y;
		ostate(o, stt+GSschbdie1);
		yelp(o);
		break;
	case Ogretel:
		givep(5000);
		ostate(o, stt+GSgreteldie1);
		drop(tl, Rkey1);
		break;
	case Ootto:
		givep(5000);
		bosskillx = oplr->x;
		bosskilly = oplr->y;
		ostate(o, stt+GSottodie1);
		break;
	case Ofett:
		givep(5000);
		bosskillx = oplr->x;
		bosskilly = oplr->y;
		ostate(o, stt+GSfettdie1);
		break;
	case Ofake:
		givep(2000);
		ostate(o, stt+GSfakedie1);
		break;
	case Omech:
		givep(5000);
		ostate(o, stt+GSmechdie1);
		break;
	case Ohitler:
		givep(5000);
		bosskillx = oplr->x;
		bosskilly = oplr->y;
		ostate(o, stt+GShitlerdie1);
		yelp(o);
		break;
	case Ospectre:
		givep(200);
		ostate(o, stt+GSspectredie1);
		break;
	case Oangel:
		givep(5000);
		ostate(o, stt+GSangeldie1);
		break;
	case Otrans:
		givep(5000);
		ostate(o, stt+GStransdie1);
		drop(tl, Rkey1);
		break;
	case Ouber:
		givep(5000);
		ostate(o, stt+GSuberdie1);
		drop(tl, Rkey1);
		break;
	case Owilh:
		givep(5000);
		ostate(o, stt+GSwilhdie1);
		drop(tl, Rkey1);
		break;
	case Oknight:
		givep(5000);
		ostate(o, stt+GSknightdie1);
		drop(tl, Rkey1);
		break;
	}
	gm.kills++;
	o->f &= ~OFshootable;
	o->f |= OFnomark;
	tl->o = nil;
}

static void
opain(Obj *o, int n)
{
	noise++;
	if(~o->f & OFattack)
		n *= 2;
	o->hp -= n;
	if(o->hp <= 0){
		odie(o);
		return;
	}
	if(~o->f & OFattack)
		oattack(o);
	switch(o->type){
	case Ogd: ostate(o, stt + (o->hp & 1 ? GSgdpain1 : GSgdpain2)); break;
	case Oofc: ostate(o, stt + (o->hp & 1 ? GSofcpain1 : GSofcpain2)); break;
	case Omut: ostate(o, stt + (o->hp & 1 ? GSmutpain1 : GSmutpain2)); break;
	case Oss: ostate(o, stt + (o->hp & 1 ? GSsspain1 : GSsspain2)); break;
	}
}

static void
hurt(int n, Obj *from)
{
	if(gm.won)
		return;
	if(gm.difc == GDbaby)
		n >>= 2;
	if(!god)
		gm.hp -= n;
	if(gm.hp <= 0){
		gm.hp = 0;
		gm.end = EDdie;
		camobj = from;
		if(ver >= SDM && from->type == Oneedle)
			gm.mut++;
	}
	dmgtc += n;
	hudh();
	hudf();
	if(ver >= SDM && n > 30 && gm.hp != 0 && !god){
		pic(136, 164, pict[Pouch]);
		facetc = 0;
	}
}

static void
omove(Obj *o, int Δr)
{
	int x, y;

	x = o->x;
	y = o->y;
	switch(o->θ){
	case θN: y -= Δr; break;
	case θNE: x += Δr; y -= Δr; break;
	case θE: x += Δr; break;
	case θSE: x += Δr; y += Δr; break;
	case θS: y += Δr; break;
	case θSW: x -= Δr; y += Δr; break;
	case θW: x -= Δr; break;
	case θNW: x -= Δr; y -= Δr; break;
	default: return;
	}
	if(!plrarea[o->areaid]
	|| abs(x - oplr->x) > Domin || abs(y - oplr->y) > Domin){
		o->x = x;
		o->y = y;
		o->Δr -= Δr;
	}else if(o->type == Oghost || o->type == Ospectre)
		hurt(Δtc * 2, o);
}

static int
spotline(Obj *o)
{
	int ox, oy, otx, oty, px, py, ptx, pty, x, y, dx, dy, δ;
	u16int t;

	ox = o->x >> 8;	/* 1/256 tile precision */
	oy = o->y >> 8;
	otx = ox >> 8;
	oty = oy >> 8;
	px = oplr->x >> 8;
	py = oplr->y >> 8;
	ptx = oplr->tx;
	pty = oplr->ty;

	if(ptx != otx){
		if(ptx > otx){
			δ = 256 - (ox & 0xff);
			dx = 1;
		}else{
			δ = ox & 0xff;
			dx = -1;
		}
		dy = (py - oy << 8) / abs(px - ox);
		if(dy > 0x7fff)
			dy = 0x7fff;
		else if(dy < -0x7fff)
			dy = -0x7fff;

		y = oy + (dy * δ >> 8);
		x = otx + dx;
		ptx += dx;
		do{
			t = tiles[(y >> 8) * Mapdxy + x].tl;
			y += dy;
			x += dx;
			if(t == 0)
				continue;
			if(t < 128 || y - dy / 2 > doors[t & ~0x80].dopen)
				return 0;
		}while(x != ptx);
	}
	if(pty != oty){
		if(pty > oty){
			δ = 256 - (oy & 0xff);
			dy = 1;
		}else{
			δ = oy & 0xff;
			dy = -1;
		}
		dx = (px - ox << 8) / abs(py - oy);
		if(dx > 0x7fff)
			dx = 0x7fff;
		else if(dx < -0x7fff)
			dx = -0x7fff;

		x = ox + (dx * δ >> 8);
		y = oty + dy;
		pty += dy;
		do{
			t = tiles[y * Mapdxy + (x >> 8)].tl;
			x += dx;
			y += dy;
			if(t == 0)
				continue;
			if(t < 128 || x - dx / 2 > doors[t & ~0x80].dopen)
				return 0;
		}while(y != pty);
	}
	return 1;
}

static void
fire(Obj *o)
{
	int dx, dy, Δr, tohit, s;

	if(!plrarea[o->areaid] || !spotline(o))
		return;
	dx = abs(o->tx - oplr->tx);
	dy = abs(o->ty - oplr->ty);
	Δr = dx > dy ? dx : dy;
	if(o->type == Oss || o->type == Ohans)
		Δr = Δr * 2 / 3;
	tohit = (oplr->v >= 6000 ? 160 : 256) - Δr * (o->f & OFvis ? 16 : 8);
	if(rnd() < tohit)
		hurt(rnd() >> (Δr < 2 ? 2 : Δr < 4 ? 3 : 4), o);
	switch(o->type){
	case Oss: s = Sssfire; break;
	case Ootto:
	case Ofett: s = Smissile; break;
	case Omech:
	case Ohitler:
	case Ohans: s = Shansfire; break;
	default: s = Sgdfire;
	}
	sfxatt(s, 1, o->x, o->y);
}

static void
smoke(Obj *o)
{
	Obj *p;

	p = onew();
	p->tx = o->tx;
	p->ty = o->ty;
	p->tl = o->tl;
	p->x = o->x;
	p->y = o->y;
	p->s = o->type == Orocket ? stt+GSrsmoke1 : stt+GSmsmoke1;
	p->type = Oinert;
	p->tc = 6;
	p->on = 1;
	p->f = OFnevermark;
}

static void
launch(Obj *o)
{
	int Δx, Δy;
	double θ;
	Obj *p;

	Δx = oplr->x - o->x;
	Δy = o->y - oplr->y;
	θ = atan2(Δy, Δx);
	if(θ < 0)
		θ = Fpi * 2 + θ;
	p = onew();
	p->tl = o->tl;
	p->tx = o->tx;
	p->ty = o->ty;
	p->x = o->x;
	p->y = o->y;
	p->θ = θ / (Fpi * 2) * 360;
	p->f = OFnomark;
	p->v = 0x2000;
	p->tc = 1;
	p->on = 1;
	switch(o->type){
	case Oschb:
		p->s = stt+GSneedle1;
		p->type = Oneedle;
		sfxatt(Srocket, 1, p->x, p->y);
		break;
	case Ootto:
	case Ofett:
		p->s = stt+GSmissile;
		p->type = Omissile;
		sfxatt(Smissile, 1, p->x, p->y);
		break;
	case Ofake:
		p->s = stt+GSflame1;
		p->type = Oflame;
		p->v = 0x1200;
		p->f = OFnevermark;
		sfxatt(Sflame, 1, p->x, p->y);
		break;
	case Owilh:
		p->s = stt+GSmissile;
		p->type = Omissile;
		sfxatt(Srocket, 1, p->x, p->y);
		break;
	case Oknight:
		fire(o);
		if(o->s == stt+GSknightfire2)
			p->θ = (p->θ + 356) % 360;
		else
			p->θ = (p->θ + 4) % 360;
		p->s = stt+GSrocket;
		p->type = Orocket;
		sfxatt(Sknightmissile, 1, p->x, p->y);
		break;
	case Oangel:
		p->s = stt+GSspark1;
		p->type = Ospark;
		sfxatt(Sspark, 1, p->x, p->y);
		break;
	}
}

static void
uberfire(Obj *o)
{
	int Δx, Δy, Δr;

	fire(o);
	Δx = abs(o->tx - oplr->tx);
	Δy = abs(o->ty - oplr->ty);
	Δr = Δx > Δy ? Δx : Δy;
	if(Δr <= 1)
		hurt(10, o);
}

static void
bite(Obj *o)
{
	sfxatt(Sdogfire, 1, o->x, o->y);
	if(abs(oplr->x - o->x) - Dtlglobal <= Domin
	&& abs(oplr->y - o->y) - Dtlglobal <= Domin
	&& rnd() < 180)
		hurt(rnd() >> 4, o);
}

static void
relaunch(Obj *o)
{
	if(++o->sdt == 3)
		ostate(o, stt+GSangeltired1);
	else if(rnd() & 1)
		ostate(o, stt+GSangelchase1);
}

static void
prelaunch(Obj *o)
{
	o->sdt = 0;
}

static int
spot(Obj *o)
{
	int dx, dy;

	if(!plrarea[o->areaid])
		return 0;
	dx = oplr->x - o->x;
	dy = oplr->y - o->y;
	if(dx > -0x18000 && dx < 0x18000 && dy > -0x18000 && dy < 0x18000)
		return 1;
	switch(o->θ){
	case θN: if(dy > 0) return 0; break;
	case θE: if(dx < 0) return 0; break;
	case θS: if(dy < 0) return 0; break;
	case θW: if(dx > 0) return 0; break;
	}
	return spotline(o);
}

static int
sawplr(Obj *o)
{
	if(o->f & OFattack)
		sysfatal("sawplr: confused attack node");
	if(o->atkdt <= 0){
		if(!plrarea[o->areaid])
			return 0;
		if((o->f & OFambush || !noise) && !spot(o))
			return 0;
		o->f &= ~OFambush;
		switch(o->type){
		case Ogd: o->atkdt = 1 + rnd() / 4; break;
		case Oofc: o->atkdt = 2; break;
		case Omut: o->atkdt = 1 + rnd() / 6; break;
		case Oss: o->atkdt = 1 + rnd() / 6; break;
		case Odog: o->atkdt = 1 + rnd() / 8; break;
		case Ohans:
		case Oschb:
		case Ogretel:
		case Ootto:
		case Ofett:
		case Ofake:
		case Omech:
		case Ohitler:
		case Otrans:
		case Owilh:
		case Ouber:
		case Oknight:
		case Ospectre:
		case Oangel: o->atkdt = 1; break;
		}
		return 0;
	}
	o->atkdt -= Δtc;
	if(o->atkdt <= 0){
		oattack(o);
		return 1;
	}
	return 0;
}

static int
diagok(Tile *tl)
{
	return tl->to == 0 && (tl->o == nil || ~tl->o->f & OFshootable);
}
static int
sideok(Tile *tl, s16int *dn)
{
	if(tl->o != nil && tl->o->f & OFshootable || tl->to > 0 && tl->to < 128)
		return 0;
	if(tl->to & 0x80)
		*dn = tl->to & 0x3f;
	return 1;
}
static int
trywalk(Obj *o, int θ)
{
	s16int dn;

	dn = -1;
	o->θ = θnil;
	switch(θ){
	case θN:
		if((o->type == Odog || o->type == Ofake)
		&& !diagok(o->tl - Mapdxy)
		|| o->type != Oinert && !sideok(o->tl - Mapdxy, &dn))
			return 0;
		o->ty--;
		o->tl -= Mapdxy;
		break;
	case θNE:
		if(o->type != Oinert && (!diagok(o->tl - Mapdxy + 1)
		|| !diagok(o->tl + 1) || !diagok(o->tl - Mapdxy)))
			return 0;
		o->tx++, o->ty--;
		o->tl -= Mapdxy - 1;
		break;
	case θE:
		if((o->type == Odog || o->type == Ofake)
		&& !diagok(o->tl + 1)
		|| o->type != Oinert && !sideok(o->tl + 1, &dn))
			return 0;
		o->tx++;
		o->tl++;
		break;
	case θSE:
		if(o->type != Oinert && (!diagok(o->tl + Mapdxy + 1)
		|| !diagok(o->tl + 1) || !diagok(o->tl + Mapdxy)))
			return 0;
		o->tx++, o->ty++;
		o->tl += Mapdxy + 1;
		break;
	case θS:
		if((o->type == Odog || o->type == Ofake)
		&& !diagok(o->tl + Mapdxy)
		|| o->type != Oinert && !sideok(o->tl + Mapdxy, &dn))
			return 0;
		o->ty++;
		o->tl += Mapdxy;
		break;
	case θSW:
		if(o->type != Oinert && (!diagok(o->tl + Mapdxy - 1)
		|| !diagok(o->tl - 1) || !diagok(o->tl + Mapdxy)))
			return 0;
		o->tx--, o->ty++;
		o->tl += Mapdxy - 1;
		break;
	case θW:
		if((o->type == Odog || o->type == Ofake)
		&& !diagok(o->tl - 1)
		|| o->type != Oinert && !sideok(o->tl - 1, &dn))
			return 0;
		o->tx--;
		o->tl--;
		break;
	case θNW:
		if(o->type != Oinert && (!diagok(o->tl - Mapdxy - 1)
		|| !diagok(o->tl - 1) || !diagok(o->tl - Mapdxy)))
			return 0;
		o->tx--; o->ty--;
		o->tl -= Mapdxy + 1;
		break;
	case θnil:
		return 0;
	default:
		sysfatal("trywalk: bad angle");
	}
	o->θ = θ;
	if(dn != -1){
		dropen(doors + dn);
		o->Δr = -dn - 1;
	}else{
		o->areaid = o->tl->p0 - MTfloor;
		o->Δr = Dtlglobal;
	}
	return 1;
}

static int
walkθ(Obj *o)
{
	int n, θ;

	n = o->tl->p1 - MTarrows;
	θ = n >= 0 && n < 8 ? n * 45 : o->θ;
	o->Δr = Dtlglobal;
	return trywalk(o, θ);
}

static int
jaywalk(Obj *o)
{
	walkθ(o);
	if(--o->sdt == 0){
		ostate(o, stt+GSjump1);
		return 0;
	}
	return 1;
}

static int
trychase(Obj *o)
{
	int Δx, Δy, i, r, d, θ[3], spin, *p;

	Δx = oplr->tx - o->tx;
	Δy = oplr->ty - o->ty;
	r = abs(Δy) > abs(Δx);
	θ[r ? 1 : 0] = Δx > 0 ? θE : Δx < 0 ? θW : θnil;
	θ[r ? 0 : 1] = Δy > 0 ? θS : Δy < 0 ? θN : θnil;
	θ[2] = o->θ;
	spin = θ[2] == θnil ? θnil : (θ[2] + 180) % 360;
	for(p=θ; p<θ+nelem(θ);){
		r = *p++;
		if(r != spin && trywalk(o, r))
			return 1;
	}
	if(rnd() > 128){
		r = 45;
		d = θN;
	}else{
		r = -45;
		d = θW;
	}
	for(i=0; i<3; d+=r, i++)
		if(d != spin && trywalk(o, d))
			return 1;
	if(trywalk(o, spin))
		return 1;
	return 0;
}

static int
tryrun(Obj *o)
{
	int Δx, Δy, i, r, d, θ[2], *p;

	Δx = oplr->tx - o->tx;
	Δy = oplr->ty - o->ty;
	r = abs(Δy) > abs(Δx);
	θ[r ? 1 : 0] = Δx < 0 ? θE : θW;
	θ[r ? 0 : 1] = Δy < 0 ? θS : θN;
	for(p=θ; p<θ+nelem(θ);){
		r = *p++;
		if(trywalk(o, r))
			return 1;
	}
	if(rnd() > 128){
		r = 45;
		d = θN;
	}else{
		r = -45;
		d = θW;
	}
	for(i=0; i<3; d+=r, i++)
		if(trywalk(o, d))
			return 1;
	return 0;
}

static int
trydodge(Obj *o)
{
	int r, Δx, Δy, θ[5], spin, *p;

	spin = θnil;
	if(o->f & OFflip)
		o->f &= ~OFflip;
	else if(o->θ != θnil)
		spin = (o->θ + 180) % 360;
	Δx = oplr->tx - o->tx;
	Δy = oplr->ty - o->ty;
	r = (abs(Δx) > abs(Δy)) ^ (rnd() < 128);
	θ[r ? 2 : 1] = Δx > 0 ? θE : θW;
	θ[r ? 4 : 3] = Δx > 0 ? θW : θE;
	θ[r ? 1 : 2] = Δy > 0 ? θS : θN;
	θ[r ? 3 : 4] = Δy > 0 ? θN : θS;
	θ[0] = θ[1] + (θ[1]<θ[2] ? (θ[2]-θ[1]==90 ? 45 : 315) : (θ[1]-θ[2]==90 ? 315 : 45));
	θ[0] %= 360;
	for(p=θ; p<θ+nelem(θ);){
		r = *p++;
		if(r != spin && trywalk(o, r))
			return 1;
	}
	if(trywalk(o, spin))
		return 1;
	return 0;
}
static void
odisplace(Obj *o, int walk, int run, int dodge)
{
	int n, Δr;
	Door *d;

	if(o->θ == θnil){
		if(walk)
			n = walkθ(o);
		else if(dodge)
			n = trydodge(o);
		else
			n = trychase(o);
		if(!n)
			return;
	}
	Δr = o->v * Δtc;
	while(Δr != 0){
		switch(o->type){
		case Odog:
			if(!walk && abs(oplr->x - o->x) - Δr <= Domin
			&& abs(oplr->y - o->y) - Δr <= Domin){
				ostate(o, stt+GSdogfire1);
				return;
			}
			break;
		case Oblaz:
		case Ofake:
		case Oghost:
		case Ospectre:
			break;
		default:
			if(o->Δr >= 0)
				break;
			d = doors - o->Δr - 1;
			dropen(d);
			if(d->φ != DRopen)
				return;
			o->Δr = Dtlglobal;
			break;
		}
		if(Δr < o->Δr){
			omove(o, Δr);
			break;
		}
		osetglobal(o);
		Δr -= o->Δr;
		if(walk)
			n = walkθ(o);
		else if(run)
			n = tryrun(o);
		else if(dodge)
			n = trydodge(o);
		else if(o->type == Oblaz)
			n = jaywalk(o);
		else
			n = trychase(o);
		if(!n)
			break;
	}
}

static void
uwait(Obj *o)
{
	sawplr(o);
}

static void
uwalk(Obj *o)
{
	if(sawplr(o))
		return;
	odisplace(o, 1, 0, 0);
}

static void
udogchase(Obj *o)
{
	odisplace(o, 0, 0, 1);
}

static void
ufake(Obj *o)
{
	if(spotline(o) && rnd() < Δtc << 1){
		ostate(o, stt+GSfakefire1);
		return;
	}
	odisplace(o, 0, 0, 1);
}

static void
ughost(Obj *o)
{
	odisplace(o, 0, 0, 0);
}

static void
uboss(Obj *o)
{
	int dodge, Δr, Δx, Δy;

	dodge = 0;
	Δx = abs(o->tx - oplr->tx);
	Δy = abs(o->ty - oplr->ty);
	Δr = Δx > Δy ? Δx : Δy;
	if(spotline(o)){
		if(rnd() < Δtc << 3){
			switch(o->type){
			case Oschb: ostate(o, stt+GSschbfire1); break;
			case Ootto: ostate(o, stt+GSottofire1); break;
			case Ofett: ostate(o, stt+GSfettfire1); break;
			case Owilh: ostate(o, stt+GSwilhfire1); break;
			case Oknight: ostate(o, stt+GSknightfire1); break;
			case Oangel: ostate(o, stt+GSangelfire1); break;
			}
			return;
		}
		dodge++;
	}
	odisplace(o, 0, Δr < 4, dodge);
}

static void
uchase(Obj *o)
{
	int dx, dy, dodge, fire, Δr;

	if(gm.won)
		return;
	dodge = 0;
	if(spotline(o)){
		dx = abs(o->tx - oplr->tx);
		dy = abs(o->ty - oplr->ty);
		Δr = dx > dy ? dx : dy;
		if(Δr == 0 || Δr == 1 && o->Δr < 0x4000)
			fire = 256;
		else
			fire = (Δtc << 4) / Δr;
		if(rnd() < fire){
			switch(o->type){
			case Ogd: ostate(o, stt+GSgdfire1); break;
			case Oss: ostate(o, stt+GSssfire1); break;
			case Oofc: ostate(o, stt+GSofcfire1); break;
			case Omut: ostate(o, stt+GSmutfire1); break;
			case Ohans: ostate(o, stt+GShansfire1); break;
			case Ogretel: ostate(o, stt+GSgretelfire1); break;
			case Omech: ostate(o, stt+GSmechfire1); break;
			case Ohitler: ostate(o, stt+GShitlerfire1); break;
			case Otrans: ostate(o, stt+GStransfire1); break;
			case Ouber: ostate(o, stt+GSuberfire1); break;
			}
			return;
		}
		dodge++;
	}
	odisplace(o, 0, 0, dodge);
}

static void
spawnrun(void)
{
	Obj *o;

	o = ospawn(oplr->tl+Mapdxy, stt+GSblaz1);
	o->x = oplr->x;
	o->y = oplr->y;
	o->type = Oblaz;
	o->v = 2048;
	o->θ = θN;
	o->sdt = 6;
}
static void
urun(Obj *o)
{
	odisplace(o, 0, 0, 0);
}
static void
ujump(Obj *o)
{
	omove(o, 680 * Δtc);
}
static void
runyell(Obj *o)
{
	sfxatt(Syeah, 1, o->x, o->y);
}

static int
trymove(Obj *o, int r, int wallsonly, int noobj)
{
	int dw, ds, x1, x2, y1, y2;
	Tile *tl, *te, *tw;

	x1 = o->x - r >> Dtlshift;
	x2 = o->x + r >> Dtlshift;
	y1 = o->y - r >> Dtlshift;
	y2 = o->y + r >> Dtlshift;
	tl = tiles + y1 * Mapdxy + x1;
	te = tiles + y2 * Mapdxy + x2;
	dw = x2 - x1;
	ds = Mapdxy - dw - 1;
	while(tl <= te){
		tw = tl + dw;
		while(tl <= tw){
			if(tl->to != 0
			|| noobj && tl->o != nil && tl->o->f & OFshootable)
				return 0;
			tl++;
		}
		tl += ds;
	}
	if(wallsonly)
		return 1;

	if(x1 > 0)
		x1--, dw++, ds--;
	if(x2 < Mapdxy - 1)
		te++, dw++, ds--;
	if(y1 > 0)
		y1--;
	if(y2 < Mapdxy - 1)
		te += Mapdxy;
	tl = tiles + y1 * Mapdxy + x1;
	while(tl <= te){
		tw = tl + dw;
		while(tl <= tw){
			if(tl->o != nil && tl->o->f & OFshootable
			&& abs(o->x - tl->o->x) <= Domin
			&& abs(o->y - tl->o->y) <= Domin)
				return 0;
			tl++;
		}
		tl += ds;
	}
	return 1;
}

static void
uprj(Obj *o)
{
	int Δx, Δy, v, dmg;

	v = o->v * Δtc;
	Δx = ffs(v, cost[o->θ]);
	Δy = -ffs(v, sint[o->θ]);
	if(Δx > 0x10000)
		Δx = 0x10000;
	if(Δy > 0x10000)
		Δy = 0x10000;
	o->x += Δx;
	o->y += Δy;
	Δx = o->x - oplr->x;
	if(Δx < 0)
		Δx = -Δx;
	Δy = o->y - oplr->y;
	if(Δy < 0)
		Δy = -Δy;
	if(!trymove(o, 0x2000, 1, 0)){
		if(o->type == Omissile){
			o->s = stt+GSmboom1;
			sfxatt(ver<SDM ? Smissilehit : Srockethit, 1, o->x, o->y);
		}else if(o->type == Orocket){
			o->s = stt+GSrboom1;
			sfxatt(Srockethit, 1, o->x, o->y);
		}else
			o->s = nil;
		return;
	}
	if(Δx < 0xc000 && Δy < 0xc000){
		dmg = 0;
		switch(o->type){
		case Oneedle: dmg = (rnd() >> 3) + 20; break;
		case Omissile:
		case Orocket:
		case Ospark: dmg = (rnd() >> 3) + 30; break;
		case Oflame: dmg = rnd() >> 3; break;
		}
		hurt(dmg, o);
		o->s = nil;
		return;
	}
	o->tx = o->x >> Dtlshift;
	o->ty = o->y >> Dtlshift;
	o->tl = tiles + o->ty * Mapdxy + o->tx;
}

static void
wake(Obj *o)
{
	if(abs(o->x - oplr->x) <= Domin && abs(o->y - oplr->y) <= Domin)
		return;
	if(!trymove(o, Dmin, 1, 1))
		return;
	o->f |= OFambush | OFshootable;
	o->f &= ~OFattack;
	o->θ = θnil;
	ostate(o, stt+GSspectrewait1);
}

static void
cam(Obj *o)
{
	if(gm.won){
		gm.end = EDcam2;
		return;
	}
	gm.won++;
	gm.end = EDcam;
	mtc = 0;
	dofizz++;
	camobj = o;
}

static void
clipmove(int Δx, int Δy)
{
	int x, y;

	x = oplr->x;
	y = oplr->y;
	oplr->x = x + Δx;
	oplr->y = y + Δy;
	if(trymove(oplr, Dplr, 0, 0))
		return;
	if(noclip && oplr->x > 2 * Dtlglobal && oplr->y > 2 * Dtlglobal
	&& oplr->x < Mapdxy - 1 << Dtlshift && oplr->y < Mapdxy - 1 << Dtlshift)
		return;
	if(lastsfx() < 0)
		sfx(Shitwall);
	oplr->x = x + Δx;
	oplr->y = y;
	if(trymove(oplr, Dplr, 0, 0))
		return;
	oplr->x = x;
	oplr->y = y + Δy;
	if(trymove(oplr, Dplr, 0, 0))
		return;
	oplr->x = x;
	oplr->y = y;
}

static void
thrust(int θ, int v)
{
	int Δx, Δy;

	if(v != 0)
		funtc = 0;
	oplr->v += v;
	if(v >= Dmin * 2)
		v = Dmin * 2 - 1;
	Δx = ffs(v, cost[θ]);
	Δy = -ffs(v, sint[θ]);
	clipmove(Δx, Δy);
	oplr->tx = oplr->x >> Dtlshift;
	oplr->ty = oplr->y >> Dtlshift;
	oplr->tl = tiles + oplr->ty * Mapdxy + oplr->tx;
	oplr->areaid = oplr->tl->p0 - MTfloor;
	if(oplr->tl->p1 == MTexit){
		if(ver < SDM)
			spawnrun();
		gm.won++;
	}
}

/* there is an angle hack because at 70Hz the roundoff becomes significant */
static void
kmove(void)
{
	int θ;
	s16int u;

	oplr->v = 0;
	if(kon & 1<<Kstrafe){
		if(kΔx > 0)
			thrust((oplr->θ + 270) % 360, kΔx * 150);
		else if(kΔx < 0)
			thrust((oplr->θ + 90) % 360, -kΔx * 150);
	}else{
		kΔθ += kΔx;
		u = kΔθ / 20;
		kΔθ -= u * 20;
		θ = oplr->θ - u;
		if(θ >= 360)
			θ -= 360;
		if(θ < 0)
			θ += 360;
		oplr->θ = θ;
	}
	if(kΔy < 0)
		thrust(oplr->θ, -kΔy * 150);
	else if(kΔy > 0)
		thrust((oplr->θ + 180) % 360, kΔy * 100);
}

static void
push(Tile *tl, int θ)
{
	Tile *c;

	if(pusher.φ != 0 || tl->tl == 0)
		return;
	c = nil;
	switch(θ){
	case θN: c = tl - Mapdxy; break;
	case θE: c = tl + 1; break;
	case θS: c = tl + Mapdxy; break;
	case θW: c = tl - 1; break;
	}
	if(c->o != nil || c->to != 0){
		sfx(Snoway);
		return;
	}
	c->to = tl->tl;
	c->tl = tl->tl;
	gm.secret++;
	pusher.tl = tl;
	pusher.isvert = θ;
	pusher.φ = 1;
	pusher.dopen = 0;
	tl->tl |= 0xc0;
	tl->p1 = 0;
	sfx(Spushwall);
}

static void
kopen(void)
{
	Tile *tl;
	int upok, dn, θ;

	tl = oplr->tl;
	upok = 0;
	if(oplr->θ < 360/8 || oplr->θ > 7*360/8){
		tl++;
		θ = θE;
		upok++;
	}else if(oplr->θ < 3*360/8){
		tl -= Mapdxy;
		θ = θN;
	}else if (oplr->θ < 5*360/8){
		tl--;
		θ = θW;
		upok++;
	}else{
		tl += Mapdxy;
		θ = θS;
	}
	dn = tl->tl;
	if(tl->p1 == MTpush){
		push(tl, θ);
		return;
	}
	if(~kold & 1<<Kopen){
		if(dn == MTgoup && upok){
			kold |= 1<<Kopen;
			tl->tl++;	/* flip switch */
			gm.end = oplr->tl->p0 == MTsetec ? EDsetec : EDup;
			sfx(Slvlend);
			sfxlck++;
			return;
		/* bug: 1<<6 may be set around pushwalls and cause memory
		 * corruption if this check goes through by writing past the
		 * doors array */
		}else if((dn & 0xc0) == 0x80){
			kold |= 1<<Kopen;
			druse(doors + (dn & ~0x80));
			return;
		}
	}
	sfx(Snope);
}

static void
kfire(void)
{
	kold |= 1<<Kfire;
	firing++;
	atkfrm = 0;
	atktc = 6;
	gm.wfrm = 1;
}

static void
winspin(void)
{
	int y;

	if(oplr->θ > 270){
		oplr->θ -= Δtc * 3;
		if(oplr->θ < 270)
			oplr->θ = 270;
	}else if(oplr->θ < 270){
		oplr->θ += Δtc * 3;
		if(oplr->θ > 270)
			oplr->θ = 270;
	}
	y = (oplr->ty - 5 << Dtlshift) - 0x3000;
	if(oplr->y > y){
		oplr->y -= Δtc * 4096;
		if(oplr->y < y)
			oplr->y = y;
	}
}

static void
face(void)
{
	/* bug: demos must be played back with the same sound settings as they
	 * were recorded, namely adlib sfx (wl6) and pcm (sod) or desync when
	 * lastsfx is called */
	if(lastsfx() == Sgetgatling)
		return;
	facetc += Δtc;
	if(facetc > rnd())
	{
		gm.facefrm = rnd() >> 6;
		if(gm.facefrm == 3)
			gm.facefrm = 1;
		facetc = 0;
		hudf();
	}
}

static void
idleface(void)
{
	funtc += Δtc;
	if(funtc > 30 * Tb){
		funtc = 0;
		pic(17*8, 164, pict[Pwait] + (rnd() & 1));
		facetc = 0;
	}
}

static void
wep(void)
{
	int i;

	if(gm.ammo == 0)
		return;
	for(i=0; i<=gm.bestw; i++)
		if(kon & 1 << Kknife + i){
			gm.w = gm.lastw = i;
			hudw();
			return;
		}
}

static void
knife(void)
{
	int Δx;
	Obj *o, *hit;

	sfx(Sknife);
	Δx = 0x7fffffff;
	hit = nil;
	for(o=oplr->n; o!=objs; o=o->n)
		if((o->f & (OFshootable | OFvis)) == (OFshootable | OFvis)
		&& abs(o->vwdx - vw.mid) < vw.Δhit && o->vwx < Δx){
			Δx = o->vwx;
			hit = o;
		}
	if(hit == nil || Δx > 0x18000)
		return;
	opain(hit, rnd() >> 4);
}

static void
gun(void)
{
	int n, dx, dy, Δx;
	Obj *o, *hit, *last;

	switch(gm.w){
	case WPpistol: sfx(Spistol); break;
	case WPmg: sfx(Smg); break;
	case WPgatling: sfx(Sgatling); break;
	}
	noise++;

	Δx = 0x7fffffff;
	hit = nil;
	for(;;){
		last = hit;
		for(o=oplr->n; o!=objs; o=o->n)
			if((o->f & (OFshootable | OFvis)) == (OFshootable | OFvis)
			&& abs(o->vwdx - vw.mid) < vw.Δhit && o->vwx < Δx){
				Δx = o->vwx;
				hit = o;
			}
		if(hit == last)
			return;
		if(spotline(hit))
			break;
	}
	dx = abs(hit->tx - oplr->tx);
	dy = abs(hit->ty - oplr->ty);
	Δx = dx > dy ? dx : dy;
	if(Δx < 2)
		n = rnd() / 4;
	else if(Δx < 4)
		n = rnd() / 6;
	else{
		if(rnd() / 12 < Δx)
			return;
		n = rnd() / 6;
	}
	opain(hit, n);
}

static void
uplr(Obj *)
{
	int n, shot;

	if(firing)
		face();
	if(gm.won){
		winspin();
		return;
	}
	shot = 0;
	if(!firing){
		face();
		wep();
		if(kon & 1<<Kopen)
			kopen();
		if(kon & 1<<Kfire & ~kold){
			kfire();
			shot++;
		}
	}
	kmove();
	if(gm.won || !firing || shot)
		return;

	atktc -= Δtc;
	while(atktc <= 0){
		n = atk[gm.w][atkfrm];
		switch(n){
		case -1:
			firing = 0;
			if(gm.ammo == 0){
				gm.w = WPknife;
				hudw();
			}else if(gm.w != gm.lastw){
				gm.w = gm.lastw;
				hudw();
			};
			atkfrm = 0;
			gm.wfrm = 0;
			return;
		case 4:
			if(gm.ammo == 0)
				break;
			if(kon & 1<<Kfire)
				atkfrm -= 2;
			/* wet floor */
		case 1:
			if(gm.ammo == 0){
				atkfrm++;
				break;
			}
			gun();
			gm.ammo--;
			huda();
			break;
		case 2:
			knife();
			break;
		case 3:
			if(gm.ammo != 0 && (kon & 1<<Kfire))
				atkfrm -= 2;
			break;
		}
		atktc += 6;
		atkfrm++;
		gm.wfrm = atkfrm + 1;
	}
}

static void
crmwapr(void)
{
	allrecv++;
}
static void
crmamo(void)
{
	gm.ammo = 99;
	huda();
}
static void
crmkey(void)
{
	gm.keys = 15;
	hudk();
}
static void
crmwep(void)
{
	if(gm.bestw < WPgatling)
		givew(gm.bestw + 1);
}
static void
crmmli(void)
{
	gm.hp = 100;
	gm.ammo = 99;
	gm.keys = 15;
	givew(WPgatling);
	gm.pt = 0;
	gm.lvltc += 42000;
	hudw();
	hudh();
	hudk();
	huda();
	hudp();
}
static void
crmmap(void)
{
}
static void
crmgod(void)
{
	if(ver < SDM)
		return;
	god ^= 1;
	if(god)
		sfx(Sendb2);
	else
		sfx(Snobonus);
}
static void
crmclp(void)
{
	if(ver < SDM)
		return;
	noclip ^= 1;
}
static void
crmslo(void)
{
	onestep ^= 1;
}
static void
crmskp(void)
{
}
static void
crmwrp(void)
{
}

static Crm crms[] = {
	{"fgd135", crmwapr},
	{"opepak", crmamo},
	{"opeopn", crmkey},
	{"opephz", crmwep},
	{"opemli", crmmli},
	{"opepda", crmmap},
	{"opedqd", crmgod},
	{"opeclp", crmclp},
	{"opeslo", crmslo},
	{"opeskp", crmskp},
	{"opewrp", crmwrp}
};

static char *
crm114(void)
{
	int n;
	Crm *p, *e;

	n = strlen(crs);
	p = crms;
	e = crms + 1;
	if(allrecv){
		p++;
		e = crms + nelem(crms);
	}
	while(p < e){
		if(strncmp(crs, p->s, n) != 0){
			memset(crs, 0, sizeof crs);
			return crs;
		}else if(n = Ncrm-1){
			p->f();
			memset(crs, 0, sizeof crs);
			return crs;
		}
		p++;
	}
	return crs+n;
}
static int
quickkey(Rune r)
{
	switch(r){
	case Kesc:
	case KF|1:
	case KF|2:
	case KF|3:
	case KF|4:
	case KF|5:
	case KF|6:
	case KF|7:
	case KF|8:
	case KF|9:
		;
	}
	return 0;
}
static void
eatcs(void)
{
	int i, d;
	char *p, rc[UTFmax];
	Rune r;

	if(gm.demo && nbrecv(csc, nil) > 0){
		gm.end = EDkey;
		return;
	}
	if(gm.record)
		return;
	i = 0;
	p = crs + strlen(crs);
	while(nbrecv(csc, &r) > 0 && i++ < 6){
		if(quickkey(r))
			continue;
		d = runetochar(rc, &r);
		if(p + d < crs + sizeof crs){
			memcpy(p, rc, d);
			p = crm114();
		}
	}
}
static void
gamein(void)
{
	int mx, my, scale;

	qlock(&inlck);
	mx = mΔx;
	my = mΔy;
	kon = kb | mΔb & 5 | (mΔb & 2) << 2;
	mΔx = mΔy = 0;
	qunlock(&inlck);

	kΔx = kΔy = 0;
	scale = Δtc * (kon & 1<<Krun ? 70 : 35);
	if(kon & 1<<K↑)
		kΔy -= scale;
	if(kon & 1<<K↓)
		kΔy += scale;
	if(kon & 1<<K←)
		kΔx -= scale;
	if(kon & 1<<K→)
		kΔx += scale;
	kΔx += mx * 10 / (13 - msense);
	kΔy += my * 20 / (13 - msense);

	scale = 100 * Δtc;
	if(kΔx > scale)
		kΔx = scale;
	else if(kΔx < -scale)
		kΔx = -scale;
	if(kΔy > scale)
		kΔy = scale;
	else if(kΔy < -scale)
		kΔy = -scale;
}
static void
demoin(void)
{
	kon = *dem++;
	kΔx = *dem++ * Δtc;
	kΔy = *dem++ * Δtc;
	if(dem >= deme)
		gm.end = EDdem;
}
static void
input(void)
{
	kold = kon;
	if(gm.demo && dem < deme)
		demoin();
	else
		gamein();
	if(gm.record){
		if(dem+3 >= deme)
			sysfatal("demo overflow");
		*dem++ = kon & 0xff;
		*dem++ = kΔx / Δtc & 0xff;
		*dem++ = kΔy / Δtc & 0xff;
	}
	if(firing){
		if(kon & 1<<Kopen & ~kold)
			kon &= ~(1<<Kopen);
		if(kon & 1<<Kfire & ~kold)
			kon &= ~(1<<Kfire);	
	}
}

State stt[] = {
	[GSplr] {uplr, nil, 0, 0, nil, 0},
	[GSplrcam] {nil, nil, 0, 0, nil, 0},
	[GSblaz1] {urun, nil, 12, SPbjwalk1, stt+GSblaz2, 0},
	[GSblaz2] {nil, nil, 3, SPbjwalk1, stt+GSblaz3, 0},
	[GSblaz3] {urun, nil, 8, SPbjwalk2, stt+GSblaz4, 0},
	[GSblaz4] {urun, nil, 12, SPbjwalk3, stt+GSblaz5, 0},
	[GSblaz5] {nil, nil, 3, SPbjwalk3, stt+GSblaz6, 0},
	[GSblaz6] {urun, nil, 8, SPbjwalk4, stt+GSblaz1, 0},
	[GSjump1] {ujump, nil, 14, SPbjjump1, stt+GSjump2, 0},
	[GSjump2] {ujump, runyell, 14, SPbjjump2, stt+GSjump3, 0},
	[GSjump3] {ujump, nil, 14, SPbjjump3, stt+GSjump4, 0},
	[GSjump4] {nil, victory, 300, SPbjjump4, stt+GSjump4, 0},
	[GSgd] {uwait, nil, 0, SPgd, stt+GSgd, 1},
	[GSgdwalk1] {uwalk, nil, 20, SPgdwalk1, stt+GSgdwalk2, 1},
	[GSgdwalk2] {nil, nil, 5, SPgdwalk1, stt+GSgdwalk3, 1},
	[GSgdwalk3] {uwalk, nil, 15, SPgdwalk2, stt+GSgdwalk4, 1},
	[GSgdwalk4] {uwalk, nil, 20, SPgdwalk3, stt+GSgdwalk5, 1},
	[GSgdwalk5] {nil, nil, 5, SPgdwalk3, stt+GSgdwalk6, 1},
	[GSgdwalk6] {uwalk, nil, 15, SPgdwalk4, stt+GSgdwalk1, 1},
	[GSgdpain1] {nil, nil, 10, SPgdpain1, stt+GSgdchase1, 2},
	[GSgdpain2] {nil, nil, 10, SPgdpain2, stt+GSgdchase1, 2},
	[GSgdchase1] {uchase, nil, 10, SPgdwalk1, stt+GSgdchase2, 1},
	[GSgdchase2] {nil, nil, 3, SPgdwalk1, stt+GSgdchase3, 1},
	[GSgdchase3] {uchase, nil, 8, SPgdwalk2, stt+GSgdchase4, 1},
	[GSgdchase4] {uchase, nil, 10, SPgdwalk3, stt+GSgdchase5, 1},
	[GSgdchase5] {nil, nil, 3, SPgdwalk3, stt+GSgdchase6, 1},
	[GSgdchase6] {uchase, nil, 8, SPgdwalk4, stt+GSgdchase1, 1},
	[GSgdfire1] {nil, nil, 20, SPgdfire1, stt+GSgdfire2, 0},
	[GSgdfire2] {nil, fire, 20, SPgdfire2, stt+GSgdfire3, 0},
	[GSgdfire3] {nil, nil, 20, SPgdfire3, stt+GSgdchase1, 0},
	[GSgddie1] {nil, yelp, 15, SPgddie1, stt+GSgddie2, 0},
	[GSgddie2] {nil, nil, 15, SPgddie2, stt+GSgddie3, 0},
	[GSgddie3] {nil, nil, 15, SPgddie3, stt+GSgddie4, 0},
	[GSgddie4] {nil, nil, 0, SPgddead, stt+GSgddie4, 0},
	[GSss] {uwait, nil, 0, SPss, stt+GSss, 1},
	[GSsswalk1] {uwalk, nil, 20, SPsswalk1, stt+GSsswalk2, 1},
	[GSsswalk2] {nil, nil, 5, SPsswalk1, stt+GSsswalk3, 1},
	[GSsswalk3] {uwalk, nil, 15, SPsswalk2, stt+GSsswalk4, 1},
	[GSsswalk4] {uwalk, nil, 20, SPsswalk3, stt+GSsswalk5, 1},
	[GSsswalk5] {nil, nil, 5, SPsswalk3, stt+GSsswalk6, 1},
	[GSsswalk6] {uwalk, nil, 15, SPsswalk4, stt+GSsswalk1, 1},
	[GSsspain1] {nil, nil, 10, SPsspain1, stt+GSsschase1, 2},
	[GSsspain2] {nil, nil, 10, SPsspain2, stt+GSsschase1, 2},
	[GSsschase1] {uchase, nil, 10, SPsswalk1, stt+GSsschase2, 1},
	[GSsschase2] {nil, nil, 3, SPsswalk1, stt+GSsschase3, 1},
	[GSsschase3] {uchase, nil, 8, SPsswalk2, stt+GSsschase4, 1},
	[GSsschase4] {uchase, nil, 10, SPsswalk3, stt+GSsschase5, 1},
	[GSsschase5] {nil, nil, 3, SPsswalk3, stt+GSsschase6, 1},
	[GSsschase6] {uchase, nil, 8, SPsswalk4, stt+GSsschase1, 1},
	[GSssfire1] {nil, nil, 20, SPssfire1, stt+GSssfire2, 0},
	[GSssfire2] {nil, fire, 20, SPssfire2, stt+GSssfire3, 0},
	[GSssfire3] {nil, nil, 10, SPssfire3, stt+GSssfire4, 0},
	[GSssfire4] {nil, fire, 10, SPssfire2, stt+GSssfire5, 0},
	[GSssfire5] {nil, nil, 10, SPssfire3, stt+GSssfire6, 0},
	[GSssfire6] {nil, fire, 10, SPssfire2, stt+GSssfire7, 0},
	[GSssfire7] {nil, nil, 10, SPssfire3, stt+GSssfire8, 0},
	[GSssfire8] {nil, fire, 10, SPssfire2, stt+GSssfire9, 0},
	[GSssfire9] {nil, nil, 10, SPssfire3, stt+GSsschase1, 0},
	[GSssdie1] {nil, yelp, 15, SPssdie1, stt+GSssdie2, 0},
	[GSssdie2] {nil, nil, 15, SPssdie2, stt+GSssdie3, 0},
	[GSssdie3] {nil, nil, 15, SPssdie3, stt+GSssdie4, 0},
	[GSssdie4] {nil, nil, 0, SPssdead, stt+GSssdie4, 0},
	[GSofc] {uwait, nil, 0, SPofc, stt+GSofc, 1},
	[GSofcwalk1] {uwalk, nil, 20, SPofcwalk1, stt+GSofcwalk2, 1},
	[GSofcwalk2] {nil, nil, 5, SPofcwalk1, stt+GSofcwalk3, 1},
	[GSofcwalk3] {uwalk, nil, 15, SPofcwalk2, stt+GSofcwalk4, 1},
	[GSofcwalk4] {uwalk, nil, 20, SPofcwalk3, stt+GSofcwalk5, 1},
	[GSofcwalk5] {nil, nil, 5, SPofcwalk3, stt+GSofcwalk6, 1},
	[GSofcwalk6] {uwalk, nil, 15, SPofcwalk4, stt+GSofcwalk1, 1},
	[GSofcpain1] {nil, nil, 10, SPofcpain1, stt+GSofcchase1, 2},
	[GSofcpain2] {nil, nil, 10, SPofcpain2, stt+GSofcchase1, 2},
	[GSofcchase1] {uchase, nil, 10, SPofcwalk1, stt+GSofcchase2, 1},
	[GSofcchase2] {nil, nil, 3, SPofcwalk1, stt+GSofcchase3, 1},
	[GSofcchase3] {uchase, nil, 8, SPofcwalk2, stt+GSofcchase4, 1},
	[GSofcchase4] {uchase, nil, 10, SPofcwalk3, stt+GSofcchase5, 1},
	[GSofcchase5] {nil, nil, 3, SPofcwalk3, stt+GSofcchase6, 1},
	[GSofcchase6] {uchase, nil, 8, SPofcwalk4, stt+GSofcchase1, 1},
	[GSofcfire1] {nil, nil, 6, SPofcfire1, stt+GSofcfire2, 0},
	[GSofcfire2] {nil, fire, 20, SPofcfire2, stt+GSofcfire3, 0},
	[GSofcfire3] {nil, nil, 10, SPofcfire3, stt+GSofcchase1, 0},
	[GSofcdie1] {nil, yelp, 11, SPofcdie1, stt+GSofcdie2, 0},
	[GSofcdie2] {nil, nil, 11, SPofcdie2, stt+GSofcdie3, 0},
	[GSofcdie3] {nil, nil, 11, SPofcdie3, stt+GSofcdie4, 0},
	[GSofcdie4] {nil, nil, 11, SPofcdie4, stt+GSofcdie5, 0},
	[GSofcdie5] {nil, nil, 0, SPofcdead, stt+GSofcdie5, 0},
	[GSmut] {uwait, nil, 0, SPmut, stt+GSmut, 1},
	[GSmutwalk1] {uwalk, nil, 20, SPmutwalk1, stt+GSmutwalk2, 1},
	[GSmutwalk2] {nil, nil, 5, SPmutwalk1, stt+GSmutwalk3, 1},
	[GSmutwalk3] {uwalk, nil, 15, SPmutwalk2, stt+GSmutwalk4, 1},
	[GSmutwalk4] {uwalk, nil, 20, SPmutwalk3, stt+GSmutwalk5, 1},
	[GSmutwalk5] {nil, nil, 5, SPmutwalk3, stt+GSmutwalk6, 1},
	[GSmutwalk6] {uwalk, nil, 15, SPmutwalk4, stt+GSmutwalk1, 1},
	[GSmutpain1] {nil, nil, 10, SPmutpain1, stt+GSmutchase1, 2},
	[GSmutpain2] {nil, nil, 10, SPmutpain2, stt+GSmutchase1, 2},
	[GSmutchase1] {uchase, nil, 10, SPmutwalk1, stt+GSmutchase2, 1},
	[GSmutchase2] {nil, nil, 3, SPmutwalk1, stt+GSmutchase3, 1},
	[GSmutchase3] {uchase, nil, 8, SPmutwalk2, stt+GSmutchase4, 1},
	[GSmutchase4] {uchase, nil, 10, SPmutwalk3, stt+GSmutchase5, 1},
	[GSmutchase5] {nil, nil, 3, SPmutwalk3, stt+GSmutchase6, 1},
	[GSmutchase6] {uchase, nil, 8, SPmutwalk4, stt+GSmutchase1, 1},
	[GSmutfire1] {nil, fire, 6, SPmutfire1, stt+GSmutfire2, 0},
	[GSmutfire2] {nil, nil, 20, SPmutfire2, stt+GSmutfire3, 0},
	[GSmutfire3] {nil, fire, 10, SPmutfire3, stt+GSmutfire4, 0},
	[GSmutfire4] {nil, nil, 20, SPmutfire4, stt+GSmutchase1, 0},
	[GSmutdie1] {nil, yelp, 7, SPmutdie1, stt+GSmutdie2, 0},
	[GSmutdie2] {nil, nil, 7, SPmutdie2, stt+GSmutdie3, 0},
	[GSmutdie3] {nil, nil, 7, SPmutdie3, stt+GSmutdie4, 0},
	[GSmutdie4] {nil, nil, 7, SPmutdie4, stt+GSmutdie5, 0},
	[GSmutdie5] {nil, nil, 0, SPmutdead, stt+GSmutdie5, 0},
	[GSdogwalk1] {uwalk, nil, 20, SPdogwalk1, stt+GSdogwalk2, 1},
	[GSdogwalk2] {nil, nil, 5, SPdogwalk1, stt+GSdogwalk3, 1},
	[GSdogwalk3] {uwalk, nil, 15, SPdogwalk2, stt+GSdogwalk4, 1},
	[GSdogwalk4] {uwalk, nil, 20, SPdogwalk3, stt+GSdogwalk5, 1},
	[GSdogwalk5] {nil, nil, 5, SPdogwalk3, stt+GSdogwalk6, 1},
	[GSdogwalk6] {uwalk, nil, 15, SPdogwalk4, stt+GSdogwalk1, 1},
	[GSdogchase1] {udogchase, nil, 10, SPdogwalk1, stt+GSdogchase2, 1},
	[GSdogchase2] {nil, nil, 3, SPdogwalk1, stt+GSdogchase3, 1},
	[GSdogchase3] {udogchase, nil, 8, SPdogwalk2, stt+GSdogchase4, 1},
	[GSdogchase4] {udogchase, nil, 10, SPdogwalk3, stt+GSdogchase5, 1},
	[GSdogchase5] {nil, nil, 3, SPdogwalk3, stt+GSdogchase6, 1},
	[GSdogchase6] {udogchase, nil, 8, SPdogwalk4, stt+GSdogchase1, 1},
	[GSdogfire1] {nil, nil, 10, SPdogfire1, stt+GSdogfire2, 0},
	[GSdogfire2] {nil, bite, 10, SPdogfire2, stt+GSdogfire3, 0},
	[GSdogfire3] {nil, nil, 10, SPdogfire3, stt+GSdogfire4, 0},
	[GSdogfire4] {nil, nil, 10, SPdogfire1, stt+GSdogfire5, 0},
	[GSdogfire5] {nil, nil, 10, SPdogwalk1, stt+GSdogchase1, 0},
	[GSdogdie1] {nil, yelp, 15, SPdogdie1, stt+GSdogdie2, 0},
	[GSdogdie2] {nil, nil, 15, SPdogdie2, stt+GSdogdie3, 0},
	[GSdogdie3] {nil, nil, 15, SPdogdie3, stt+GSdogdie4, 0},
	[GSdogdie4] {nil, nil, 15, SPdogdead, stt+GSdogdie4, 0},
	[GShans] {uwait, nil, 0, SPhanswalk1, stt+GShans, 0},
	[GShanschase1] {uchase, nil, 10, SPhanswalk1, stt+GShanschase2, 0},
	[GShanschase2] {nil, nil, 3, SPhanswalk1, stt+GShanschase3, 0},
	[GShanschase3] {uchase, nil, 8, SPhanswalk2, stt+GShanschase4, 0},
	[GShanschase4] {uchase, nil, 10, SPhanswalk3, stt+GShanschase5, 0},
	[GShanschase5] {nil, nil, 3, SPhanswalk3, stt+GShanschase6, 0},
	[GShanschase6] {uchase, nil, 8, SPhanswalk4, stt+GShanschase1, 0},
	[GShansfire1] {nil, nil, 30, SPhansfire1, stt+GShansfire2, 0},
	[GShansfire2] {nil, fire, 10, SPhansfire2, stt+GShansfire3, 0},
	[GShansfire3] {nil, fire, 10, SPhansfire3, stt+GShansfire4, 0},
	[GShansfire4] {nil, fire, 10, SPhansfire2, stt+GShansfire5, 0},
	[GShansfire5] {nil, fire, 10, SPhansfire3, stt+GShansfire6, 0},
	[GShansfire6] {nil, fire, 10, SPhansfire2, stt+GShansfire7, 0},
	[GShansfire7] {nil, fire, 10, SPhansfire3, stt+GShansfire8, 0},
	[GShansfire8] {nil, nil, 10, SPhansfire1, stt+GShanschase1, 0},
	[GShansdie1] {nil, yelp, 15, SPhansdie1, stt+GShansdie2, 0},
	[GShansdie2] {nil, nil, 15, SPhansdie2, stt+GShansdie3, 0},
	[GShansdie3] {nil, nil, 15, SPhansdie3, stt+GShansdie4, 0},
	[GShansdie4] {nil, nil, 0, SPhansdead, stt+GShansdie4, 0},
	[GSschb] {uwait, nil, 0, SPschbwalk1, stt+GSschb, 0},
	[GSschbchase1] {uboss, nil, 10, SPschbwalk1, stt+GSschbchase2, 0},
	[GSschbchase2] {nil, nil, 3, SPschbwalk1, stt+GSschbchase3, 0},
	[GSschbchase3] {uboss, nil, 8, SPschbwalk2, stt+GSschbchase4, 0},
	[GSschbchase4] {uboss, nil, 10, SPschbwalk3, stt+GSschbchase5, 0},
	[GSschbchase5] {nil, nil, 3, SPschbwalk3, stt+GSschbchase6, 0},
	[GSschbchase6] {uboss, nil, 8, SPschbwalk4, stt+GSschbchase1, 0},
	[GSschbfire1] {nil, nil, 30, SPschbfire1, stt+GSschbfire2, 0},
	[GSschbfire2] {nil, launch, 10, SPschbfire2, stt+GSschbchase1, 0},
	[GSschbcam] {nil, nil, 1, SPschbwalk1, stt+GSschbdie1, 0},
	[GSschbdie1] {nil, yelp, 10, SPschbwalk1, stt+GSschbdie2, 0},
	[GSschbdie2] {nil, nil, 10, SPschbwalk1, stt+GSschbdie3, 0},
	[GSschbdie3] {nil, nil, 10, SPschbdie1, stt+GSschbdie4, 0},
	[GSschbdie4] {nil, nil, 10, SPschbdie2, stt+GSschbdie5, 0},
	[GSschbdie5] {nil, nil, 10, SPschbdie3, stt+GSschbdie6, 0},
	[GSschbdie6] {nil, cam, 20, SPschbdead, stt+GSschbdie6, 0},
	[GSgretel] {uwait, nil, 0, SPgretelwalk1, stt+GSgretel, 0},
	[GSgretelchase1] {uchase, nil, 10, SPgretelwalk1, stt+GSgretelchase2, 0},
	[GSgretelchase2] {nil, nil, 3, SPgretelwalk1, stt+GSgretelchase3, 0},
	[GSgretelchase3] {uchase, nil, 8, SPgretelwalk2, stt+GSgretelchase4, 0},
	[GSgretelchase4] {uchase, nil, 10, SPgretelwalk3, stt+GSgretelchase5, 0},
	[GSgretelchase5] {nil, nil, 3, SPgretelwalk3, stt+GSgretelchase6, 0},
	[GSgretelchase6] {uchase, nil, 8, SPgretelwalk4, stt+GSgretelchase1, 0},
	[GSgretelfire1] {nil, nil, 30, SPgretelfire1, stt+GSgretelfire2, 0},
	[GSgretelfire2] {nil, fire, 10, SPgretelfire2, stt+GSgretelfire3, 0},
	[GSgretelfire3] {nil, fire, 10, SPgretelfire3, stt+GSgretelfire4, 0},
	[GSgretelfire4] {nil, fire, 10, SPgretelfire2, stt+GSgretelfire5, 0},
	[GSgretelfire5] {nil, fire, 10, SPgretelfire3, stt+GSgretelfire6, 0},
	[GSgretelfire6] {nil, fire, 10, SPgretelfire2, stt+GSgretelfire7, 0},
	[GSgretelfire7] {nil, fire, 10, SPgretelfire3, stt+GSgretelfire8, 0},
	[GSgretelfire8] {nil, nil, 10, SPgretelfire1, stt+GSgretelchase1, 0},
	[GSgreteldie1] {nil, yelp, 15, SPgreteldie1, stt+GSgreteldie2, 0},
	[GSgreteldie2] {nil, nil, 15, SPgreteldie2, stt+GSgreteldie3, 0},
	[GSgreteldie3] {nil, nil, 15, SPgreteldie3, stt+GSgreteldie4, 0},
	[GSgreteldie4] {nil, nil, 0, SPgreteldead, stt+GSgreteldie4, 0},
	[GSotto] {uwait, nil, 0, SPottowalk1, stt+GSotto, 0},
	[GSottochase1] {uboss, nil, 10, SPottowalk1, stt+GSottochase2, 0},
	[GSottochase2] {nil, nil, 3, SPottowalk1, stt+GSottochase3, 0},
	[GSottochase3] {uboss, nil, 8, SPottowalk2, stt+GSottochase4, 0},
	[GSottochase4] {uboss, nil, 10, SPottowalk3, stt+GSottochase5, 0},
	[GSottochase5] {nil, nil, 3, SPottowalk3, stt+GSottochase6, 0},
	[GSottochase6] {uboss, nil, 8, SPottowalk4, stt+GSottochase1, 0},
	[GSottofire1] {nil, nil, 30, SPottofire1, stt+GSottofire2, 0},
	[GSottofire2] {nil, launch, 10, SPottofire2, stt+GSottochase1, 0},
	[GSottocam] {nil, nil, 1, SPottowalk1, stt+GSottodie1, 0},
	[GSottodie1] {nil, yelp, 1, SPottowalk1, stt+GSottodie2, 0},
	[GSottodie2] {nil, nil, 10, SPottowalk1, stt+GSottodie3, 0},
	[GSottodie3] {nil, nil, 10, SPottodie1, stt+GSottodie4, 0},
	[GSottodie4] {nil, nil, 10, SPottodie2, stt+GSottodie5, 0},
	[GSottodie5] {nil, nil, 10, SPottodie3, stt+GSottodie6, 0},
	[GSottodie6] {nil, cam, 20, SPottodead, stt+GSottodie6, 0},
	[GSfett] {uwait, nil, 0, SPfettwalk1, stt+GSfett, 0},
	[GSfettchase1] {uboss, nil, 10, SPfettwalk1, stt+GSfettchase2, 0},
	[GSfettchase2] {nil, nil, 3, SPfettwalk1, stt+GSfettchase3, 0},
	[GSfettchase3] {uboss, nil, 8, SPfettwalk2, stt+GSfettchase4, 0},
	[GSfettchase4] {uboss, nil, 10, SPfettwalk3, stt+GSfettchase5, 0},
	[GSfettchase5] {nil, nil, 3, SPfettwalk3, stt+GSfettchase6, 0},
	[GSfettchase6] {uboss, nil, 8, SPfettwalk4, stt+GSfettchase1, 0},
	[GSfettfire1] {nil, nil, 30, SPfettfire1, stt+GSfettfire2, 0},
	[GSfettfire2] {nil, launch, 10, SPfettfire2, stt+GSfettfire3, 0},
	[GSfettfire3] {nil, fire, 10, SPfettfire3, stt+GSfettfire4, 0},
	[GSfettfire4] {nil, fire, 10, SPfettfire4, stt+GSfettfire5, 0},
	[GSfettfire5] {nil, fire, 10, SPfettfire3, stt+GSfettfire6, 0},
	[GSfettfire6] {nil, fire, 10, SPfettfire4, stt+GSfettchase1, 0},
	[GSfettcam] {nil, nil, 1, SPfettwalk1, stt+GSfettdie1, 0},
	[GSfettdie1] {nil, yelp, 1, SPfettwalk1, stt+GSfettdie2, 0},
	[GSfettdie2] {nil, nil, 10, SPfettwalk1, stt+GSfettdie3, 0},
	[GSfettdie3] {nil, nil, 10, SPfettdie1, stt+GSfettdie4, 0},
	[GSfettdie4] {nil, nil, 10, SPfettdie2, stt+GSfettdie5, 0},
	[GSfettdie5] {nil, nil, 10, SPfettdie3, stt+GSfettdie6, 0},
	[GSfettdie6] {nil, cam, 20, SPfettdead, stt+GSfettdie6, 0},
	[GSfake] {uwait, nil, 0, SPfakewalk1, stt+GSfake, 0},
	[GSfakechase1] {ufake, nil, 10, SPfakewalk1, stt+GSfakechase2, 0},
	[GSfakechase2] {nil, nil, 3, SPfakewalk1, stt+GSfakechase3, 0},
	[GSfakechase3] {ufake, nil, 8, SPfakewalk2, stt+GSfakechase4, 0},
	[GSfakechase4] {ufake, nil, 10, SPfakewalk3, stt+GSfakechase5, 0},
	[GSfakechase5] {nil, nil, 3, SPfakewalk3, stt+GSfakechase6, 0},
	[GSfakechase6] {ufake, nil, 8, SPfakewalk4, stt+GSfakechase1, 0},
	[GSfakefire1] {nil, launch, 8, SPfakefire, stt+GSfakefire2, 0},
	[GSfakefire2] {nil, launch, 8, SPfakefire, stt+GSfakefire3, 0},
	[GSfakefire3] {nil, launch, 8, SPfakefire, stt+GSfakefire4, 0},
	[GSfakefire4] {nil, launch, 8, SPfakefire, stt+GSfakefire5, 0},
	[GSfakefire5] {nil, launch, 8, SPfakefire, stt+GSfakefire6, 0},
	[GSfakefire6] {nil, launch, 8, SPfakefire, stt+GSfakefire7, 0},
	[GSfakefire7] {nil, launch, 8, SPfakefire, stt+GSfakefire8, 0},
	[GSfakefire8] {nil, launch, 8, SPfakefire, stt+GSfakefire9, 0},
	[GSfakefire9] {nil, nil, 8, SPfakefire, stt+GSfakechase1, 0},
	[GSfakedie1] {nil, yelp, 10, SPfakedie1, stt+GSfakedie2, 0},
	[GSfakedie2] {nil, nil, 10, SPfakedie2, stt+GSfakedie3, 0},
	[GSfakedie3] {nil, nil, 10, SPfakedie3, stt+GSfakedie4, 0},
	[GSfakedie4] {nil, nil, 10, SPfakedie4, stt+GSfakedie5, 0},
	[GSfakedie5] {nil, nil, 10, SPfakedie5, stt+GSfakedie6, 0},
	[GSfakedie6] {nil, nil, 0, SPfakedead, stt+GSfakedie6, 0},
	[GSmech] {uwait, nil, 0, SPmechwalk1, stt+GSmech, 0},
	[GSmechchase1] {uchase, mechsfx, 10, SPmechwalk1, stt+GSmechchase2, 0},
	[GSmechchase2] {nil, nil, 6, SPmechwalk1, stt+GSmechchase3, 0},
	[GSmechchase3] {uchase, nil, 8, SPmechwalk2, stt+GSmechchase4, 0},
	[GSmechchase4] {uchase, mechsfx, 10, SPmechwalk3, stt+GSmechchase5, 0},
	[GSmechchase5] {nil, nil, 6, SPmechwalk3, stt+GSmechchase6, 0},
	[GSmechchase6] {uchase, nil, 8, SPmechwalk4, stt+GSmechchase1, 0},
	[GSmechfire1] {nil, nil, 30, SPmechfire1, stt+GSmechfire2, 0},
	[GSmechfire2] {nil, fire, 10, SPmechfire2, stt+GSmechfire3, 0},
	[GSmechfire3] {nil, fire, 10, SPmechfire3, stt+GSmechfire4, 0},
	[GSmechfire4] {nil, fire, 10, SPmechfire2, stt+GSmechfire5, 0},
	[GSmechfire5] {nil, fire, 10, SPmechfire3, stt+GSmechfire6, 0},
	[GSmechfire6] {nil, fire, 10, SPmechfire2, stt+GSmechchase1, 0},
	[GSmechdie1] {nil, yelp, 10, SPmechdie1, stt+GSmechdie2, 0},
	[GSmechdie2] {nil, nil, 10, SPmechdie2, stt+GSmechdie3, 0},
	[GSmechdie3] {nil, mechblow, 10, SPmechdie3, stt+GSmechdie4, 0},
	[GSmechdie4] {nil, nil, 0, SPmechdead, stt+GSmechdie4, 0},
	[GShitlerchase1] {uchase, nil, 6, SPhitlerwalk1, stt+GShitlerchase2, 0},
	[GShitlerchase2] {nil, nil, 4, SPhitlerwalk1, stt+GShitlerchase3, 0},
	[GShitlerchase3] {uchase, nil, 2, SPhitlerwalk2, stt+GShitlerchase4, 0},
	[GShitlerchase4] {uchase, nil, 6, SPhitlerwalk3, stt+GShitlerchase5, 0},
	[GShitlerchase5] {nil, nil, 4, SPhitlerwalk3, stt+GShitlerchase6, 0},
	[GShitlerchase6] {uchase, nil, 2, SPhitlerwalk4, stt+GShitlerchase1, 0},
	[GShitlerfire1] {nil, nil, 30, SPhitlerfire1, stt+GShitlerfire2, 0},
	[GShitlerfire2] {nil, fire, 10, SPhitlerfire2, stt+GShitlerfire3, 0},
	[GShitlerfire3] {nil, fire, 10, SPhitlerfire3, stt+GShitlerfire4, 0},
	[GShitlerfire4] {nil, fire, 10, SPhitlerfire2, stt+GShitlerfire5, 0},
	[GShitlerfire5] {nil, fire, 10, SPhitlerfire3, stt+GShitlerfire6, 0},
	[GShitlerfire6] {nil, fire, 10, SPhitlerfire2, stt+GShitlerchase1, 0},
	[GShitlercam] {nil, nil, 10, SPhitlerwalk1, stt+GShitlerdie1, 0},
	[GShitlerdie1] {nil, yelp, 1, SPhitlerwalk1, stt+GShitlerdie2, 0},
	[GShitlerdie2] {nil, nil, 10, SPhitlerwalk1, stt+GShitlerdie3, 0},
	[GShitlerdie3] {nil, slurp, 10, SPhitlerdie1, stt+GShitlerdie4, 0},
	[GShitlerdie4] {nil, nil, 10, SPhitlerdie2, stt+GShitlerdie5, 0},
	[GShitlerdie5] {nil, nil, 10, SPhitlerdie3, stt+GShitlerdie6, 0},
	[GShitlerdie6] {nil, nil, 10, SPhitlerdie4, stt+GShitlerdie7, 0},
	[GShitlerdie7] {nil, nil, 10, SPhitlerdie5, stt+GShitlerdie8, 0},
	[GShitlerdie8] {nil, nil, 10, SPhitlerdie6, stt+GShitlerdie9, 0},
	[GShitlerdie9] {nil, nil, 10, SPhitlerdie7, stt+GShitlerdie10, 0},
	[GShitlerdie10] {nil, cam, 20, SPhitlerdead, stt+GShitlerdie10, 0},
	[GSgh1chase1] {ughost, nil, 10, SPgh1walk1, stt+GSgh1chase2, 0},
	[GSgh2chase1] {ughost, nil, 10, SPgh3walk1, stt+GSgh2chase2, 0},
	[GSgh3chase1] {ughost, nil, 10, SPgh2walk1, stt+GSgh3chase2, 0},
	[GSgh4chase1] {ughost, nil, 10, SPgh2walk1, stt+GSgh4chase2, 0},
	[GSgh1chase2] {ughost, nil, 10, SPgh1walk2, stt+GSgh1chase1, 0},
	[GSgh2chase2] {ughost, nil, 10, SPgh3walk2, stt+GSgh2chase1, 0},
	[GSgh3chase2] {ughost, nil, 10, SPgh2walk2, stt+GSgh3chase1, 0},
	[GSgh4chase2] {ughost, nil, 10, SPgh2walk2, stt+GSgh4chase1, 0},
	[GStrans] {uwait, nil, 0, SPtranswalk1, stt+GStrans, 0},
	[GStranschase1] {uchase, nil, 10, SPtranswalk1, stt+GStranschase2, 0},
	[GStranschase2] {nil, nil, 3, SPtranswalk1, stt+GStranschase3, 0},
	[GStranschase3] {uchase, nil, 8, SPtranswalk2, stt+GStranschase4, 0},
	[GStranschase4] {uchase, nil, 10, SPtranswalk3, stt+GStranschase5, 0},
	[GStranschase5] {nil, nil, 3, SPtranswalk3, stt+GStranschase6, 0},
	[GStranschase6] {uchase, nil, 8, SPtranswalk4, stt+GStranschase1, 0},
	[GStransfire1] {nil, nil, 30, SPtransfire1, stt+GStransfire2, 0},
	[GStransfire2] {nil, fire, 10, SPtransfire2, stt+GStransfire3, 0},
	[GStransfire3] {nil, fire, 10, SPtransfire3, stt+GStransfire4, 0},
	[GStransfire4] {nil, fire, 10, SPtransfire2, stt+GStransfire5, 0},
	[GStransfire5] {nil, fire, 10, SPtransfire3, stt+GStransfire6, 0},
	[GStransfire6] {nil, fire, 10, SPtransfire2, stt+GStransfire7, 0},
	[GStransfire7] {nil, fire, 10, SPtransfire3, stt+GStransfire8, 0},
	[GStransfire8] {nil, nil, 10, SPtransfire1, stt+GStranschase1, 0},
	[GStransdie1] {nil, yelp, 1, SPtranswalk1, stt+GStransdie2, 0},
	[GStransdie2] {nil, nil, 1, SPtranswalk1, stt+GStransdie3, 0},
	[GStransdie3] {nil, nil, 15, SPtransdie1, stt+GStransdie4, 0},
	[GStransdie4] {nil, nil, 15, SPtransdie2, stt+GStransdie5, 0},
	[GStransdie5] {nil, nil, 15, SPtransdie3, stt+GStransdie6, 0},
	[GStransdie6] {nil, nil, 0, SPtransdead, stt+GStransdie6, 0},
	[GSwilh] {uwait, nil, 0, SPwilhwalk1, stt+GSwilh, 0},
	[GSwilhchase1] {uboss, nil, 10, SPwilhwalk1, stt+GSwilhchase2, 0},
	[GSwilhchase2] {nil, nil, 3, SPwilhwalk1, stt+GSwilhchase3, 0},
	[GSwilhchase3] {uboss, nil, 8, SPwilhwalk2, stt+GSwilhchase4, 0},
	[GSwilhchase4] {uboss, nil, 10, SPwilhwalk3, stt+GSwilhchase5, 0},
	[GSwilhchase5] {nil, nil, 3, SPwilhwalk3, stt+GSwilhchase6, 0},
	[GSwilhchase6] {uboss, nil, 8, SPwilhwalk4, stt+GSwilhchase1, 0},
	[GSwilhfire1] {nil, nil, 30, SPwilhfire1, stt+GSwilhfire2, 0},
	[GSwilhfire2] {nil, launch, 10, SPwilhfire2, stt+GSwilhfire3, 0},
	[GSwilhfire3] {nil, fire, 10, SPwilhfire3, stt+GSwilhfire4, 0},
	[GSwilhfire4] {nil, fire, 10, SPwilhfire4, stt+GSwilhfire5, 0},
	[GSwilhfire5] {nil, fire, 10, SPwilhfire3, stt+GSwilhfire6, 0},
	[GSwilhfire6] {nil, fire, 10, SPwilhfire4, stt+GSwilhchase1, 0},
	[GSwilhdie1] {nil, yelp, 1, SPwilhwalk1, stt+GSwilhdie2, 0},
	[GSwilhdie2] {nil, nil, 10, SPwilhwalk1, stt+GSwilhdie3, 0},
	[GSwilhdie3] {nil, nil, 10, SPwilhdie1, stt+GSwilhdie4, 0},
	[GSwilhdie4] {nil, nil, 10, SPwilhdie2, stt+GSwilhdie5, 0},
	[GSwilhdie5] {nil, nil, 10, SPwilhdie3, stt+GSwilhdie6, 0},
	[GSwilhdie6] {nil, nil, 20, SPwilhdead, stt+GSwilhdie6, 0},
	[GSuber] {uwait, nil, 0, SPuberwalk1, stt+GSuber, 0},
	[GSuberchase1] {uchase, nil, 10, SPuberwalk1, stt+GSuberchase2, 0},
	[GSuberchase2] {nil, nil, 3, SPuberwalk1, stt+GSuberchase3, 0},
	[GSuberchase3] {uchase, nil, 8, SPuberwalk2, stt+GSuberchase4, 0},
	[GSuberchase4] {uchase, nil, 10, SPuberwalk3, stt+GSuberchase5, 0},
	[GSuberchase5] {nil, nil, 3, SPuberwalk3, stt+GSuberchase6, 0},
	[GSuberchase6] {uchase, nil, 8, SPuberwalk4, stt+GSuberchase1, 0},
	[GSuberfire1] {nil, nil, 30, SPuberfire1, stt+GSuberfire2, 0},
	[GSuberfire2] {nil, uberfire, 12, SPuberfire2, stt+GSuberfire3, 0},
	[GSuberfire3] {nil, uberfire, 12, SPuberfire3, stt+GSuberfire4, 0},
	[GSuberfire4] {nil, uberfire, 12, SPuberfire4, stt+GSuberfire5, 0},
	[GSuberfire5] {nil, uberfire, 12, SPuberfire3, stt+GSuberfire6, 0},
	[GSuberfire6] {nil, uberfire, 12, SPuberfire2, stt+GSuberfire7, 0},
	[GSuberfire7] {nil, nil, 12, SPuberfire1, stt+GSuberchase1, 0},
	[GSuberdie1] {nil, yelp, 1, SPuberwalk1, stt+GSuberdie2, 0},
	[GSuberdie2] {nil, nil, 1, SPuberwalk1, stt+GSuberdie3, 0},
	[GSuberdie3] {nil, nil, 15, SPuberdie1, stt+GSuberdie4, 0},
	[GSuberdie4] {nil, nil, 15, SPuberdie2, stt+GSuberdie5, 0},
	[GSuberdie5] {nil, nil, 15, SPuberdie3, stt+GSuberdie6, 0},
	[GSuberdie6] {nil, nil, 15, SPuberdie4, stt+GSuberdie7, 0},
	[GSuberdie7] {nil, nil, 0, SPuberdead, stt+GSuberdie7, 0},
	[GSknight] {uwait, nil, 0, SPknightwalk1, stt+GSknight, 0},
	[GSknightchase1] {uboss, nil, 10, SPknightwalk1, stt+GSknightchase2, 0},
	[GSknightchase2] {nil, nil, 3, SPknightwalk1, stt+GSknightchase3, 0},
	[GSknightchase3] {uboss, nil, 8, SPknightwalk2, stt+GSknightchase4, 0},
	[GSknightchase4] {uboss, nil, 10, SPknightwalk3, stt+GSknightchase5, 0},
	[GSknightchase5] {nil, nil, 3, SPknightwalk3, stt+GSknightchase6, 0},
	[GSknightchase6] {uboss, nil, 8, SPknightwalk4, stt+GSknightchase1, 0},
	[GSknightfire1] {nil, nil, 30, SPknightfire1, stt+GSknightfire2, 0},
	[GSknightfire2] {nil, launch, 10, SPknightfire2, stt+GSknightfire3, 0},
	[GSknightfire3] {nil, fire, 10, SPknightfire4, stt+GSknightfire4, 0},
	[GSknightfire4] {nil, launch, 10, SPknightfire3, stt+GSknightfire5, 0},
	[GSknightfire5] {nil, fire, 10, SPknightfire4, stt+GSknightchase1, 0},
	[GSknightdie1] {nil, yelp, 1, SPknightwalk1, stt+GSknightdie2, 0},
	[GSknightdie2] {nil, nil, 10, SPknightwalk1, stt+GSknightdie3, 0},
	[GSknightdie3] {nil, nil, 10, SPknightdie1, stt+GSknightdie4, 0},
	[GSknightdie4] {nil, nil, 10, SPknightdie2, stt+GSknightdie5, 0},
	[GSknightdie5] {nil, nil, 10, SPknightdie3, stt+GSknightdie6, 0},
	[GSknightdie6] {nil, nil, 10, SPknightdie4, stt+GSknightdie7, 0},
	[GSknightdie7] {nil, nil, 10, SPknightdie5, stt+GSknightdie8, 0},
	[GSknightdie8] {nil, nil, 10, SPknightdie6, stt+GSknightdie9, 0},
	[GSknightdie9] {nil, nil, 0, SPknightdead, stt+GSknightdie9, 0},
	[GSspectrewait1] {uwait, nil, 10, SPspectrewalk1, stt+GSspectrewait2, 0},
	[GSspectrewait2] {uwait, nil, 10, SPspectrewalk2, stt+GSspectrewait3, 0},
	[GSspectrewait3] {uwait, nil, 10, SPspectrewalk3, stt+GSspectrewait4, 0},
	[GSspectrewait4] {uwait, nil, 10, SPspectrewalk4, stt+GSspectrewait1, 0},
	[GSspectrewake] {nil, wake, 10, SPspectreF4, stt+GSspectrewake, 0},
	[GSspectrechase1] {ughost, nil, 10, SPspectrewalk1, stt+GSspectrechase2, 0},
	[GSspectrechase2] {ughost, nil, 10, SPspectrewalk2, stt+GSspectrechase3, 0},
	[GSspectrechase3] {ughost, nil, 10, SPspectrewalk3, stt+GSspectrechase4, 0},
	[GSspectrechase4] {ughost, nil, 10, SPspectrewalk4, stt+GSspectrechase1, 0},
	[GSspectredie1] {nil, nil, 10, SPspectreF1, stt+GSspectredie2, 0},
	[GSspectredie2] {nil, nil, 10, SPspectreF2, stt+GSspectredie3, 0},
	[GSspectredie3] {nil, nil, 10, SPspectreF3, stt+GSspectredie4, 0},
	[GSspectredie4] {nil, nil, 300, SPspectreF4, stt+GSspectrewake, 0},
	[GSangel] {uwait, nil, 0, SPangelwalk1, stt+GSangel, 0},
	[GSangelchase1] {uboss, nil, 10, SPangelwalk1, stt+GSangelchase2, 0},
	[GSangelchase2] {nil, nil, 3, SPangelwalk1, stt+GSangelchase3, 0},
	[GSangelchase3] {uboss, nil, 8, SPangelwalk2, stt+GSangelchase4, 0},
	[GSangelchase4] {uboss, nil, 10, SPangelwalk3, stt+GSangelchase5, 0},
	[GSangelchase5] {nil, nil, 3, SPangelwalk3, stt+GSangelchase6, 0},
	[GSangelchase6] {uboss, nil, 8, SPangelwalk4, stt+GSangelchase1, 0},
	[GSangelfire1] {nil, prelaunch, 10, SPangelfire1, stt+GSangelfire2, 0},
	[GSangelfire2] {nil, launch, 20, SPangelfire2, stt+GSangelfire3, 0},
	[GSangelfire3] {nil, relaunch, 10, SPangelfire1, stt+GSangelfire2, 0},
	[GSangeldie1] {nil, yelp, 1, SPangelwalk1, stt+GSangeldie2, 0},
	[GSangeldie2] {nil, nil, 1, SPangelwalk1, stt+GSangeldie3, 0},
	[GSangeldie3] {nil, slurp, 10, SPangeldie1, stt+GSangeldie4, 0},
	[GSangeldie4] {nil, nil, 10, SPangeldie2, stt+GSangeldie5, 0},
	[GSangeldie5] {nil, nil, 10, SPangeldie3, stt+GSangeldie6, 0},
	[GSangeldie6] {nil, nil, 10, SPangeldie4, stt+GSangeldie7, 0},
	[GSangeldie7] {nil, nil, 10, SPangeldie5, stt+GSangeldie8, 0},
	[GSangeldie8] {nil, nil, 10, SPangeldie6, stt+GSangeldie9, 0},
	[GSangeldie9] {nil, nil, 10, SPangeldie7, stt+GSangeldie10, 0},
	[GSangeldie10] {nil, victory, 130, SPangeldead, stt+GSangeldie10, 0},
	[GSangeltired1] {nil, tiredsfx, 40, SPangeltired1, stt+GSangeltired2, 0},
	[GSangeltired2] {nil, nil, 40, SPangeltired2, stt+GSangeltired3, 0},
	[GSangeltired3] {nil, tiredsfx, 40, SPangeltired1, stt+GSangeltired4, 0},
	[GSangeltired4] {nil, nil, 40, SPangeltired2, stt+GSangeltired5, 0},
	[GSangeltired5] {nil, tiredsfx, 40, SPangeltired1, stt+GSangeltired6, 0},
	[GSangeltired6] {nil, nil, 40, SPangeltired2, stt+GSangeltired7, 0},
	[GSangeltired7] {nil, tiredsfx, 40, SPangeltired1, stt+GSangelchase1, 0},
	[GSmissile] {uprj, smoke, 3, SPmissile1, stt+GSmissile, 1},
	[GSmsmoke1] {nil, nil, 3, SPmsmoke1, stt+GSmsmoke2, 0},
	[GSmsmoke2] {nil, nil, 3, SPmsmoke2, stt+GSmsmoke3, 0},
	[GSmsmoke3] {nil, nil, 3, SPmsmoke3, stt+GSmsmoke4, 0},
	[GSmsmoke4] {nil, nil, 3, SPmsmoke4, nil, 0},
	[GSmboom1] {nil, nil, 6, SPmboom1, stt+GSmboom2, 0},
	[GSmboom2] {nil, nil, 6, SPmboom2, stt+GSmboom3, 0},
	[GSmboom3] {nil, nil, 6, SPmboom3, nil, 0},
	[GSrocket] {uprj, smoke, 3, SProcket1, stt+GSrocket, 1},
	[GSrsmoke1] {nil, nil, 3, SPrsmoke1, stt+GSrsmoke2, 0},
	[GSrsmoke2] {nil, nil, 3, SPrsmoke2, stt+GSrsmoke3, 0},
	[GSrsmoke3] {nil, nil, 3, SPrsmoke3, stt+GSrsmoke4, 0},
	[GSrsmoke4] {nil, nil, 3, SPrsmoke4, nil, 0},
	[GSrboom1] {nil, nil, 6, SPrboom1, stt+GSrboom2, 0},
	[GSrboom2] {nil, nil, 6, SPrboom2, stt+GSrboom3, 0},
	[GSrboom3] {nil, nil, 6, SPrboom3, nil, 0},
	[GSflame1] {nil, uprj, 6, SPflame1, stt+GSflame2, 0},
	[GSflame2] {nil, uprj, 6, SPflame2, stt+GSflame1, 0},
	[GSneedle1] {uprj, nil, 6, SPneedle1, stt+GSneedle2, 0},
	[GSneedle2] {uprj, nil, 6, SPneedle2, stt+GSneedle3, 0},
	[GSneedle3] {uprj, nil, 6, SPneedle3, stt+GSneedle4, 0},
	[GSneedle4] {uprj, nil, 6, SPneedle4, stt+GSneedle1, 0},
	[GSspark1] {uprj, nil, 6, SPspark1, stt+GSspark2, 0},
	[GSspark2] {uprj, nil, 6, SPspark2, stt+GSspark3, 0},
	[GSspark3] {uprj, nil, 6, SPspark3, stt+GSspark4, 0},
	[GSspark4] {uprj, nil, 6, SPspark4, stt+GSspark1, 0}
};

static void
upal(void)
{
	int step;

	if(dmgtc > 0){
		step = dmgtc / 10;
		if(step > Cwht - Cred - 1)
			step = Cwht - Cred - 1;
		dmgtc -= Δtc;
		if(dmgtc < 0)
			dmgtc = 0;
		if(step > 0)
			pal = pals[Cred + step];
	}else if(bonustc > 0){
		step = bonustc / 6;
		if(step > Cfad - Cwht - 1)
			step = Cfad - Cwht - 1;
		bonustc -= Δtc;
		if(bonustc < 0)
			bonustc = 0;
		if(step > 0)
			pal = pals[Cwht + step];
	}else
		pal = pals[C0];
}

void
camwarp(void)
{
	int Δx, Δy, Δr;
	double θ;

	oplr->x = bosskillx;
	oplr->y = bosskilly;
	Δx = camobj->x - oplr->x;
	Δy = oplr->y - camobj->y;
	θ = atan2(Δy, Δx);
	if(θ < 0)
		θ = Fpi * 2 + θ;
	oplr->θ = θ / (Fpi * 2) * 360;
	Δr = 0x14000;
	do{
		oplr->x = camobj->x - ffs(Δr, cost[oplr->θ]);
		oplr->y = camobj->y + ffs(Δr, sint[oplr->θ]);
		Δr += 0x1000;
	}while(!trymove(oplr, Dplr, 1, 0));
	oplr->tx = oplr->x >> Dtlshift;
	oplr->ty = oplr->y >> Dtlshift;
	oplr->tl = tiles + oplr->ty * Mapdxy + oplr->tx;
	ostate(oplr, stt+GSplrcam);
	switch(camobj->type){
	case Oschb: ostate(camobj, stt+GSschbcam); break;
	case Ootto: ostate(camobj, stt+GSottocam); break;
	case Ofett: ostate(camobj, stt+GSfettcam); break;
	case Ohitler: ostate(camobj, stt+GShitlercam); break;
	}
}

void
bonus(Static *s)
{
	switch(s->item){
	case Rstim:
		if(gm.hp == 100)
			return;
		sfx(Shealth2);
		giveh(25);
		break;
	case Rkey1:
	case Rkey2:
	case Rkey3:
	case Rkey4:
		sfx(Sgetkey);
		givek(s->item - Rkey1);
		break;
	case Rcross:
		sfx(Sbonus1);
		givep(100);
		gm.treasure++;
		break;
	case Rchalice:
		sfx(Sbonus2);
		givep(500);
		gm.treasure++;
		break;
	case Rbible:
		sfx(Sbonus3);
		givep(1000);
		gm.treasure++;
		break;
	case Rcrown:
		sfx(Sbonus4);
		givep(5000);
		gm.treasure++;
		break;
	case Rclip1:
		if(gm.ammo == 99)
			return;
		sfx(Sgetammo);
		givea(8);
		break;
	case Rclip2:
		if(gm.ammo == 99)
			return;
		sfx(Sgetammo);
		givea(4);
		break;
	case Rammobox:
		if(gm.ammo == 99)
			return;
		sfx(Sammobox);
		givea(25);
		break;
	case Rmg:
		sfx(Sgetmg);
		givew(WPmg);
		break;
	case Rchaingun:
		sfx(Sgetgatling);
		givew(WPgatling);
		pic(136, 164, pict[Pgat]);
		facetc = 0;
		break;
	case R1up:
		sfx(S1up);
		giveh(99);
		givea(25);
		givel();
		gm.treasure++;
		break;
	case Rfood:
		if(gm.hp == 100)
			return;
		sfx(Shealth1);
		giveh(10);
		break;
	case Ralpo:
		if(gm.hp == 100)
			return;
		sfx(Shealth1);
		giveh(4);
		break;
	case Rgibs:
		if(gm.hp > 10)
			return;
		sfx(Sslurp);
		giveh(1);
		break;
	case Rspear:
		gotspear++;
		spearx = oplr->x;
		speary = oplr->y;
		spearθ = oplr->θ;
		gm.end = EDup;
	}
	bonustc = 6 * (Cfad-Cwht);
	s->tl = nil;
}

int
rnd(void)
{
	rndi = rndi+1 & 0xff;
	return rndt[rndi];
}

void
gstep(void)
{
	if(gm.demo || gm.record){
		if(demfrm-- != 0)
			return;
		demfrm = 3;
		Δtc = 4;
	}
	input();
	noise = 0;
	uworld();
	upal();
	render();
	mtc += Δtc;
	gm.lvltc += Δtc;
	if(dofizz){
		dofizz = 0;
		if(!gm.end)
			gm.end = EDfizz;
		gend();
		return;
	}
	if(ver >= SDM)
		idleface();
	out();
	eatcs();
	if(gm.end)
		gend();
}

void
demo(void)
{
	initmap();
	mapmus();
	pal = pals[C0];
	dofizz++;
	step = gstep;
}

void
initg(int r, uchar *p)
{
	memset(&gm, 0, sizeof gm);
	if(p != nil){
		gm.demo++;
		gm.difc = GDhard;
		gm.map = p[0];
		deme = (char*)(p + (p[2]<<8 | p[1]));
		dem = (char*)p + 4;
		if((deme-dem) % 3 != 0)
			sysfatal("initd: invalid demo lump\n");
		demfrm = 0;
	}
	gm.hp = 100;
	gm.ammo = 8;
	gm.lives = 3;
	gm.w = gm.lastw = gm.bestw = WPpistol;
	gm.to1up = GPextra;
	rndi = r ? time(nil) & 0xff : 0;
	dmgtc = bonustc = facetc = funtc = 0;
	firing = 0;
	kon = kold = 0;
	kΔθ = 0;
	allrecv = 0;
	sfxlck = 0;
	gotspear = 0;
	if(ver == SOD && gm.map == 20)
		givek(0);
}
