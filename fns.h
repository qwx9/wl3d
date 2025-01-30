void*	emalloc(ulong);
void	grab(int);
void	toss(void);
void	flush(void);
void	pack(char*, ...);
void	unpack(char*, ...);
void	wrsav(int);
void	ldsav(int);
char*	demof(char*);
void	wrconf(void);
void	rdconf(void);
u16int*	readmap(int);
void	dat(char*);
void	out(void);
void	put(int, int, int, int, int);
void	pput(int, int, int, int, uchar*);
void	fill(int);
void	clear(void);
void	fizz(void);
void	fizzop(int, int);
void	fadeout(void);
void	fadein(void);
void	fadeop(Col*, int);
void	palfill(Col*);
void	palpic(uchar*);
int	txt(int, int, char*, int);
int	txtnl(int, int, char*, int);
int	txth(char*);
int	txtw(char*);
void	txtcen(int, char*, int);
void	pic(int, int, int);
void	pictxt(int, int, char*);
void	wlmclear(void);
void	sdmclear(void);
void	wlstripe(int);
void	sdstripe(int);
void	outbox(int, int, int, int, int, int);
void	box(int, int, int, int, int, int, int);
void	disking(void);
void	viewbox(void);
void	hudf(void);
void	hudh(void);
void	hudl(void);
void	hudm(void);
void	huda(void);
void	hudk(void);
void	hudw(void);
void	hudp(void);
void	view(void);
void	scalspr(int, int, int);
s32int	ffs(s32int, s32int);
void	render(void);
void	setvw(void);
void	tab(void);
int	quickkey(Rune);
void	gend(void);
void	eatcs(void);
void	qstep(void);
void	init(char*, int, int);
void	drop(Tile*, int);
void	dropen(Door*);
void	druse(Door*);
void	osetglobal(Obj*);
void	ostate(Obj*, State*);
Obj*	onew(void);
Obj*	ospawn(Tile*, State*);
void	uworld(void);
void	mapmus(void);
void	wrmap(void);
void	ldmap(void);
void	initmap(void);
void	sodmap(void);
void	dieturn(void);
void	die(void);
void	camwarp(void);
void	givew(int);
void	givep(int);
void	bonus(Static*);
void	crm114(int);
void	eatcs(void);
int	rnd(void);
void	gstep(void);
void	nextmap(void);
void	game(void);
void	spshunt(void);
void	wrgm(void);
void	ldgm(void);
void	greset(void);
void	ginit(uchar*, int, int);
uchar*	opl2out(uchar*, int);
void	opl2wr(int, int);
void	opl2init(int);
void	stopsfx(void);
int	lastsfx(void);
void	sfxatt(int, int, int, int);
void	locksfx(int);
void	sfx(int);
void	stopmus(void);
void	mus(int);
void	initsnd(void);
