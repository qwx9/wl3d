typedef short s16int;
typedef int s32int;

enum{
	WL6,
	WL1,
	SDM,
	SOD
};
extern int ver;
extern char *ext;
extern int grabon;
enum{
	K↑,
	K↓,
	K←,
	K→,
	Krun,
	Kfire,
	Kopen,
	Kstrafe,
	Kmenu,
	Ke
};
extern int cson, kbon, mson;
extern Rune keys[];
extern void (*step)(void);

enum{
	Vw = 320,
	Vh = 200,
	Va = Vw * Vh,
	Vbp = 24/8,
	Vt = Va * Vbp,
	Tb = 70
};
extern uchar *px;
extern int npx, scale;

typedef struct Col Col;
enum{
	C0,
	Cred,
	Cwht = Cred+6,
	Cfad = Cwht+3,
	Csod,
	Cend
};
extern u32int *pal, pals[][256];
struct Col{
	int r;
	int g;
	int b;
};

typedef struct Dat Dat;
typedef struct Pic Pic;
typedef struct Fnt Fnt;
typedef struct Sfx Sfx;

struct Dat{
	u16int sz;
	uchar *p;
};
extern Dat *wals, *sprs, *spre;
extern Dat *imfs, *imfe;
extern Dat *maps, *mape, *map;
extern Dat *exts, *exte;
extern Dat *dems, *deme, *epis;

struct Pic{
	int x;
	int y;
	Dat;
};
extern Pic *pics, *pice;
extern uchar *pict;

struct Fnt{
	int h;
	int ofs[256];
	char w[256];
	Dat;
};
extern Fnt fnts[], *fnt;

struct Sfx{
	Dat pc;
	Dat al;
	Dat *pcm;
};
extern Sfx *sfxs, *sfxe;

enum{
	Shitwall,
	Srockethit,
	Sghost = Srockethit+2,
	Sdrawgun2,
	Sdrawgun1,
	Snoway,
	Sthrow = Snoway+2,
	Sdeath,
	Sdogdeath,
	Sgatling,
	Sgetkey,
	Sopendoor = Sgetkey+6,
	Sclosedoor,
	Snope,
	Sgd,
	Sscream2,
	Sknife,
	Spistol,
	Sscream3,
	Smg,
	Shitdoor = Smg+2,
	Sscream1,
	Sgetmg,
	Sgetammo,
	Sshoot,
	Shealth1,
	Shealth2,
	Sbonus1,
	Sbonus2,
	Sbonus3,
	Sgetgatling,
	Sesc,
	Slvlend,
	Sdogbark,
	Sendb1,
	Sendb2,
	S1up,
	Sbonus4,
	Spushwall,
	Snobonus,
	S100,
	Shansdeath = S100+2,
	Sss,
	Smutdeath,
	Shitlerdeath,
	Seva,
	Shans,
	Sssdeath,
	Smechadeath,
	Sgdfire,
	Shansfire,
	Sssfire,
	Sslurp,
	Sfake,
	Sschbdeath,
	Sschb,
	Shitler,
	Soffc,
	Soffcdeath,
	Sdogfire,
	Sflame,
	Smechwalk,
	Stransdeath,
	Syeah,
	Sscream4,
	Sscream5,
	Sscream6,
	Sscream7,
	Sscream8,
	Sscream9,
	Sottodeath,
	Sotto,
	Sfett,
	Sgretel,
	Sgreteldeath,
	Sfettdeath,
	Smissile,
	Smissilehit,
	Send,

	Sghostdeath = Sfake,
	Sammobox = Sschb,
	Sangel = Shitler,
	Sangelfire = Sflame,
	Strans = Smechwalk,
	Swilh = Syeah,
	Swilhdeath = Shansdeath,
	Suberdeath = Shitlerdeath,
	Sknight = Smechadeath,
	Sknightdeath = Seva,
	Sangeldeath = Shans,
	Sknightmissile = Sschbdeath,
	Spear = Sottodeath,
	Sangeltired = Sotto,
	Ssend = Sfett,

	Mintro = 7,
	Mmenu = 14,
	Mnazjazz = 18,
	Maward = 20,
	Mroster = 23,
	Mtower = 23,

	Pbackdrop = 0,
	Pmouselback,
	Pcur1,
	Punsel,
	Psel,
	Pcustom,
	Pmouse,
	Pjs,
	Pkb,
	Pctl,
	Popt,
	Psfx,
	Ppcm,
	Pmus,
	Pdiffc,
	Pbaby,
	Pread1,
	Pload,
	Psave,
	Pscores,
	Pspear,
	Pcollapse,
	Pguy,
	Pcolon,
	P0,
	Ppercent,
	Pa,
	Pexcl,
	Papo,
	Pguy2,
	Pwin,
	Ptitle1,
	Ptitle2,
	Pend1,
	Pstat,
	Ppg13,
	Pcreds,
	Pid1,
	Pid2,
	Pknife,
	Pnokey,
	Pgkey,
	Pskey,
	Pblank,
	Pn0,
	Pface1,
	Pface8,
	Pgat,
	Pgod,
	Pwait,
	Pouch,
	Ppause,
	Ppsyched,
	Pend,

	Pep1 = Pmouse,
	Ptc = Pjs,
	Plvl = Pkb,
	Pname = Pdiffc,
	Phigh = Pspear,
	Pmut = Pcollapse,

	Eorder = 0,
	Eerror,
	Etitpal,
	Eend1,
	Eid = 12
};
