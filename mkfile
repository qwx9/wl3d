</$objtype/mkfile

TARG=\
	opl2\
	wl3d\

OFILES=
WOFILES=\
	fs.$O\
	gm.$O\
	map.$O\
	mn.$O\
	opl2.$O\
	rend.$O\
	snd.$O\
	wl3d.$O\

HFILES= dat.h fns.h

</sys/src/cmd/mkmany
BIN=$home/bin/$objtype

dirinstall:V:
	mkdir -p /sys/games/lib/wl3d
	cp intro.wl6 intro.sod /sys/games/lib/wl3d

$O.wl3d: $WOFILES
	$LD -o $target $prereq

$O.opl2: opl2.$O opl2m.$O
	$LD -o $target $prereq
