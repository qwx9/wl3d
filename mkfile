</$objtype/mkfile

BIN=/$objtype/bin/games
TARG=wl3d

OFILES=\
	drw.$O\
	fs.$O\
	gm.$O\
	hub.$O\
	map.$O\
	opl2.$O\
	rend.$O\
	snd.$O\
	wl3d.$O\

HFILES= dat.h fns.h

</sys/src/cmd/mkone

sysinstall:V:
	mkdir -p /sys/games/lib/wl3d
	cp intro.wl6 intro.sod /sys/games/lib/wl3d/
	cp man/1/wl3d /sys/man/1/
	cp man/6/wl3d /sys/man/6/
