#include <u.h>
#include <libc.h>
#include <thread.h>
#include "dat.h"
#include "fns.h"

Dat *imfs, *imfe;
Sfx *sfxs, *sfxe;

static int mi;

void
sfx(int n)
{
	Sfx *s;

	s = sfxs+n;
	USED(s);
}

void
stopmus(void)
{
}

void
mus(int n)
{
	if(n == mi)
		return;
	stopmus();
}
