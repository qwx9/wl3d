#include <u.h>
#include <libc.h>
#include <thread.h>
#include "dat.h"
#include "fns.h"

mainstacksize = 16*1024;
char *ext = "wl6";
int debug;
int skipintro;

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
	case 'i': skipintro++; break;
	case 'm': datdir = EARGF(usage()); break;
	case 'o': ext = "sdm"; break;
	case 's': ext = "sod"; break;
	case 'w': /* TODO: warp to ep, level */ break;
	case 'x': /* TODO: set difficulty for warp */ break;
	default:
		usage();
	}ARGEND;
	dat(datdir);

	threadexitsall(nil);
}
