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
int debug, nointro;

static Rectangle fbr;
static Image *fb;

void *
emalloc(ulong n)
{
	void *p;

	p = mallocz(n, 1);
	if(p == nil)
		sysfatal("emalloc: %r");
	return p;
}

/* use only for shortening buffers, no zeroing done */
void *
erealloc(void *p, ulong n)
{
	p = realloc(p, n);
	if(p == nil)
		sysfatal("erealloc: %r");
	return p;
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

	freeimage(fb);
	free(px);
	npx = Vt * scale;
	px = emalloc(npx);
	fb = allocimage(display, Rect(0,0,Vw*scale,scale==1 ? Vh : 1), RGB24, 1, 0);
	if(fb == nil)
		sysfatal("resetfb: %r");

	draw(screen, screen->r, display->black, nil, ZP);
}

static void
usage(void)
{
	fprint(2, "usage: %s [-23Ddios] [-m dir] [-w map] [-x difficulty]\n", argv0);
	threadexits("usage");
}

void
threadmain(int argc, char **argv)
{
	char *datdir = "/sys/games/lib/wl3d/";

	ARGBEGIN{
	case '2': ext = "sd2"; break;
	case '3': ext = "sd3"; break;
	case 'D': debug++; break;
	case 'd': ext = "wl1"; break;
	case 'i': nointro++; break;
	case 'm': datdir = EARGF(usage()); break;
	case 'o': ext = "sdm"; break;
	case 's': ext = "sod"; break;
	case 'w': /* TODO: warp to ep, level */ break;
	case 'x': /* TODO: set difficulty for warp */ break;
	default:
		usage();
	}ARGEND;
	dat(datdir);

	if(initdraw(nil, nil, "wl3d") < 0)
		sysfatal("initdraw: %r");
	resetfb();

	init();
	demos();

	threadexitsall(nil);
}
