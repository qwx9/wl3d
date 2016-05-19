#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>
#include "dat.h"
#include "fns.h"

mainstacksize = 16*1024;
char *ext = "wl6";
int ver;
int grabon;
int cson, kbon, mson;
void (*step)(void);
Channel *csc, *kbc, *msc;

enum{
	Te9 = 1000000000,
	Te6 = 1000000,
	Td = Te9/Tb
};
static Point p0;
static Rectangle fbr, grabr;
static Image *fb;
static Channel *reszc;

static void
mproc(void *)
{
	int n, fd, nerr;
	char buf[1+5*12], *px, *py, *pb;
	Point o, p;
	Mouse m;

	fd = open("/dev/mouse", ORDWR);
	if(fd < 0)
		sysfatal("mproc: %r");
	nerr = 0;
	px = buf+1;
	py = px + 12;
	pb = py + 12;
	o = p0;
	for(;;){
		n = read(fd, buf, sizeof buf);
		if(n != 1+4*12){
			if(n < 0 || ++nerr > 10)
				break;
			fprint(2, "mproc: bad count %d not 49: %r\n", n);
			continue;
		}
		nerr = 0;
		switch(*buf){
		case 'r':
			send(reszc, nil);
			/* wet floor */
		case 'm':
			if(!mson)
				break;
			p.x = strtol(px, nil, 10);
			p.y = strtol(py, nil, 10);
			m.xy.x = p.x - o.x;
			m.xy.y = o.y - p.y;
			m.buttons = *pb;
			nbsend(msc, &m);
			if(!ptinrect(p, grabr)){
				fprint(fd, "m%d %d", p0.x, p0.y);
				p = p0;
			}
			o = p;
		}
	}
}

static void
kproc(void *)
{
	int n, k, fd;
	char c, buf[256], *s;
	Rune r, *a;

	fd = open("/dev/kbd", OREAD);
	if(fd < 0)
		sysfatal("kproc: %r");
	memset(buf, 0, sizeof buf);
	for(;;){
		if(buf[0] != 0){
			n = strlen(buf)+1;
			memmove(buf, buf+n, sizeof(buf)-n);
		}
		if(buf[0] == 0){
			n = read(fd, buf, sizeof(buf)-1);
			if(n <= 0)
				break;
			buf[n-1] = 0;
			buf[n] = 0;
		}
		c = *buf;
		if(c == 'c' && cson){
			chartorune(&r, buf+1);
			send(csc, &r);
		}
		if(c != 'k' || c != 'K' || !kbon)
			continue;
		s = buf+1;
		k = 0;
		while(*s != 0){
			s += chartorune(&r, s);
			for(a=keys; a<keys+Ke; a++)
				if(r == *a){
					k |= 1<<a-keys;
					break;
				}
		}
		send(kbc, &k);
	}
}

static void
resetfb(void)
{
	Point p, d;

	scale = Dx(screen->r) / Vw;
	if(scale <= 0)
		scale = 1;
	else if(scale > 10)
		scale = 10;
	p = divpt(addpt(screen->r.min, screen->r.max), 2);
	d = Pt(Vw/2 * scale, Vh/2 * scale);
	fbr = Rpt(subpt(p, d), addpt(p, d));
	d = Pt(Vh/4, Vh/4);
	grabr = Rpt(subpt(p, d), addpt(p, d));
	p0 = p;

	freeimage(fb);
	free(px);
	npx = Vt * scale;
	px = emalloc(npx);
	fb = allocimage(display, Rect(0,0,Vw*scale,scale==1 ? Vh : 1), RGB24, 1, 0);
	if(fb == nil)
		sysfatal("resetfb: %r");

	draw(screen, screen->r, display->black, nil, ZP);
	out();
}

static void
croak(void *, char *s)
{
	if(strncmp(s, "sys:", 4) == 0)
		mson = 0;
	noted(NDFLT);
}

static void
usage(void)
{
	fprint(2, "usage: %s [-23dos] [-m dir] [-w map] [-x difficulty]\n", argv0);
	threadexits("usage");
}

void *
emalloc(ulong n)
{
	void *p;

	p = mallocz(n, 1);
	if(p == nil)
		sysfatal("emalloc: %r");
	return p;
}

void *
erealloc(void *p, ulong n)
{
	p = realloc(p, n);
	if(p == nil)
		sysfatal("erealloc: %r");
	return p;
}

void
grab(int on)
{
	static char nocurs[2*4+2*2*16];
	static int fd = -1;

	if(mson == on)
		return;
	if(mson = on && grabon){
		fd = open("/dev/cursor", ORDWR|OCEXEC);
		if(fd < 0){
			fprint(2, "grab: %r\n");
			return;
		}
		write(fd, nocurs, sizeof nocurs);
	}else if(fd >= 0){
		close(fd);
		fd = -1;
	}
}

void
toss(void)
{
	while(nbrecv(csc, nil) != 0);
	while(nbrecv(msc, nil) != 0);
	while(nbrecv(kbc, nil) != 0);
}

void
flush(void)
{
	Rectangle r;
	uchar *p;

	if(scale == 1){
		loadimage(fb, fb->r, px, npx);
		draw(screen, fbr, fb, nil, ZP);
	}else{
		p = px;
		r = fbr;
		while(r.min.y < fbr.max.y){
			r.max.y = r.min.y + scale;
			p += loadimage(fb, fb->r, p, npx/Vh);
			draw(screen, r, fb, nil, ZP);
			r.min.y = r.max.y;
		}
	}
	flushimage(display, 1);
}

void
threadmain(int argc, char **argv)
{
	vlong t0, t, dt;
	char *datdir = "/sys/games/lib/wl3d/";

	step = mstep;
	ARGBEGIN{
	case '2': ext = "sd2"; ver = SOD; break;
	case '3': ext = "sd3"; ver = SOD; break;
	case 'd': ext = "wl1"; ver = WL1; break;
	case 'm': datdir = EARGF(usage()); break;
	case 'o': ext = "sdm"; ver = SDM; break;
	case 's': ext = "sod"; ver = SOD; break;
	case 'w': /* TODO: warp to ep, level */ break;
	case 'x': /* TODO: set difficulty for warp */ break;
	default:
		usage();
	}ARGEND;
	dat(datdir);

	notify(croak);
	initsnd();
	if(initdraw(nil, nil, "wl3d") < 0)
		sysfatal("initdraw: %r");
	resetfb();
	kbc = chancreate(sizeof(int), 20);
	csc = chancreate(sizeof(Rune), 20);
	reszc = chancreate(sizeof(int), 2);
	msc = chancreate(sizeof(Mouse), 0);
	if(kbc == nil || csc == nil | reszc == nil || msc == nil)
		sysfatal("chancreate: %r");
	if(proccreate(kproc, nil, 8192) < 0 || proccreate(mproc, nil, 8192) < 0)
		sysfatal("proccreate: %r");

	init();
	t0 = nsec();
	for(;;){
		if(nbrecv(reszc, nil) != 0){
			if(getwindow(display, Refnone) < 0)
				sysfatal("resize failed: %r");
			resetfb();
		}
		step();
		sndstep();
		t0 += Td;
		t = nsec();
		dt = (t0 - t) / Te6;
		if(dt > 0)
			sleep(dt);
		else
			t0 = t;
	}
}
