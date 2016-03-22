#include <u.h>
#include <libc.h>
#include <thread.h>
#include "dat.h"
#include "fns.h"

Dat *imfs, *imfe;
Sfx *sfxs, *sfxe;

void
stopmus(void)
{
}

void
playmus(int n)
{
	stopmus();
	USED(n);
}
