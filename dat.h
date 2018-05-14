typedef struct Col Col;
typedef struct Dat Dat;
typedef struct Sprc Sprc;
typedef struct Spr Spr;
typedef struct Pic Pic;
typedef struct Fnt Fnt;
typedef struct Sfx Sfx;
typedef struct Al Al;
typedef struct View View;
typedef struct State State;
typedef struct Obj Obj;
typedef struct Door Door;
typedef struct Static Static;
typedef struct Tile Tile;
typedef struct Game Game;
typedef struct Score Score;

enum{
	WL6,
	WL1,
	SDM,
	SOD
};
extern int ver;
extern char *ext;
extern int grabon;
extern int demexit;
enum{
	Kfire,
	Kstrafe,
	Krun,
	Kopen,
	Kknife,
	Kpistol,
	Kmg,
	Kgatling,
	K↑,
	K↓,
	K←,
	K→,
	Kmenu,
	Ke
};
extern int msense;
extern int vwsize;
extern int kbon;
extern int kb, mΔx, mΔy, mb;
extern int sfxon, muson, pcmon;
extern int sfxlck;
extern Rune keys[];
extern void (*step)(void);
extern int Δtc;
extern int nosleep;
extern int autorun;
extern int qtc;

enum{
	Vw = 320,
	Vh = 200,
	Vhud = Vh - 40,
	Va = Vw * Vh,
	Vbp = 24/8,
	Vt = Va * Vbp,
	Tb = 70
};
extern uchar *px, pxb[], fzb[];
extern int npx, scale;
extern void (*mclear)(void);
extern void (*stripe)(int);

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

struct Dat{
	uchar *p;
	uchar *e;
};
extern Dat *imfs;
struct Sprc{
	uchar *p;
	int s;
	int e;
};
struct Spr{
	int lx;
	int rx;
	Sprc **cs;
	Sprc **ce;
	uchar *sp;
};
extern Spr *sprs;
extern uchar **exts, **dems, **epis, **wals;
extern int drofs;

struct Pic{
	int x;
	int y;
	uchar *p;
};
extern Pic *pics;
extern uchar *pict;

struct Fnt{
	int h;
	int ofs[256];
	char w[256];
	uchar *p;
};
extern Fnt fnts[], *fnt;

struct Al{
	int pri;
	uchar inst[10];
	int blk;
	Dat;
};
struct Sfx{
	Al;
	Dat *pcm;
};
extern Sfx *sfxs;

enum{
	Shitwall,
	Srockethit,
	Sghost = Srockethit+2,
	Sdrawgun2,
	Sdrawgun1,
	Snoway,
	Srocket = Snoway+2,
	Sdie,
	Sdogdie,
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
	Sdog,
	Sendb1,
	Sendb2,
	S1up,
	Sbonus4,
	Spushwall,
	Snobonus,
	S100,
	Shansdie = S100+2,
	Sss,
	Smutdie,
	Shitlerdie,
	Seva,
	Shans,
	Sssdie,
	Smechdie,
	Sgdfire,
	Shansfire,
	Sssfire,
	Sslurp,
	Sfake,
	Sschbdie,
	Sschb,
	Shitler,
	Sofc,
	Sofcdie,
	Sdogfire,
	Sflame,
	Smechwalk,
	Stransdie,
	Syeah,
	Sscream4,
	Sscream5,
	Sscream6,
	Sscream7,
	Sscream8,
	Sscream9,
	Sottodie,
	Sotto,
	Sfett,
	Sgretel,
	Sgreteldie,
	Sfettdie,
	Smissile,
	Smissilehit,
	Send,

	Sghostdie = Sfake,
	Sammobox = Sschb,
	Sangel = Shitler,
	Sspark = Sflame,
	Strans = Smechwalk,
	Swilh = Syeah,
	Swilhdie = Shansdie,
	Suberdie = Shitlerdie,
	Sknight = Smechdie,
	Sknightdie = Seva,
	Sangeldie = Shans,
	Sknightmissile = Sschbdie,
	Sspear = Sottodie,
	Sangeltired = Sotto,
	Ssend = Sfett,

	Msdwon = 6,
	Mend = 7,
	Mintro = 7,
	Mmenu = 14,
	Minter = 16,
	Mnazjazz = 18,
	Maward = 20,
	Mroster = 23,
	Mtower = 23,
	Mwon = 24,

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
	Eid = 12,

	SPdemo = 0,
	SPcam,
	SPgd = 50,
	SPgdwalk1 = SPgd + 8,
	SPgdwalk2 = SPgdwalk1 + 8,
	SPgdwalk3 = SPgdwalk2 + 8,
	SPgdwalk4 = SPgdwalk3 + 8,
	SPgdpain1 = SPgdwalk4 + 8,
	SPgddie1,
	SPgddie2,
	SPgddie3,
	SPgdpain2,
	SPgddead,
	SPgdfire1,
	SPgdfire2,
	SPgdfire3,
	SPdogwalk1,
	SPdogwalk2 = SPdogwalk1 + 8,
	SPdogwalk3 = SPdogwalk2 + 8,
	SPdogwalk4 = SPdogwalk3 + 8,
	SPdogdie1 = SPdogwalk4 + 8,
	SPdogdie2,
	SPdogdie3,
	SPdogdead,
	SPdogfire1,
	SPdogfire2,
	SPdogfire3,
	SPss,
	SPsswalk1 = SPss + 8,
	SPsswalk2 = SPsswalk1 + 8,
	SPsswalk3 = SPsswalk2 + 8,
	SPsswalk4 = SPsswalk3 + 8,
	SPsspain1 = SPsswalk4 + 8,
	SPssdie1,
	SPssdie2,
	SPssdie3,
	SPsspain2,
	SPssdead,
	SPssfire1,
	SPssfire2,
	SPssfire3,
	SPmut,
	SPmutwalk1 = SPmut + 8,
	SPmutwalk2 = SPmutwalk1 + 8,
	SPmutwalk3 = SPmutwalk2 + 8,
	SPmutwalk4 = SPmutwalk3 + 8,
	SPmutpain1 = SPmutwalk4 + 8,
	SPmutdie1,
	SPmutdie2,
	SPmutdie3,
	SPmutpain2,
	SPmutdie4,
	SPmutdead,
	SPmutfire1,
	SPmutfire2,
	SPmutfire3,
	SPmutfire4,
	SPofc,
	SPofcwalk1 = SPofc + 8,
	SPofcwalk2 = SPofcwalk1 + 8,
	SPofcwalk3 = SPofcwalk2 + 8,
	SPofcwalk4 = SPofcwalk3 + 8,
	SPofcpain1 = SPofcwalk4 + 8,
	SPofcdie1,
	SPofcdie2,
	SPofcdie3,
	SPofcpain2,
	SPofcdie4,
	SPofcdead,
	SPofcfire1,
	SPofcfire2,
	SPofcfire3,
	SPgh1walk1,
	SPgh1walk2,
	SPgh2walk1,
	SPgh2walk2,
	SPgh3walk1,
	SPgh3walk2,
	SPgh4walk1,
	SPgh4walk2,
	SPhanswalk1,
	SPhanswalk2,
	SPhanswalk3,
	SPhanswalk4,
	SPhansfire1,
	SPhansfire2,
	SPhansfire3,
	SPhansdead,
	SPhansdie1,
	SPhansdie2,
	SPhansdie3,
	SPschbwalk1,
	SPschbwalk2,
	SPschbwalk3,
	SPschbwalk4,
	SPschbfire1,
	SPschbfire2,
	SPschbdie1,
	SPschbdie2,
	SPschbdie3,
	SPschbdead,
	SPneedle1,
	SPneedle2,
	SPneedle3,
	SPneedle4,
	SPfakewalk1,
	SPfakewalk2,
	SPfakewalk3,
	SPfakewalk4,
	SPfakefire,
	SPflame1,
	SPflame2,
	SPfakedie1,
	SPfakedie2,
	SPfakedie3,
	SPfakedie4,
	SPfakedie5,
	SPfakedead,
	SPmechwalk1,
	SPmechwalk2,
	SPmechwalk3,
	SPmechwalk4,
	SPmechfire1,
	SPmechfire2,
	SPmechfire3,
	SPmechdead,
	SPmechdie1,
	SPmechdie2,
	SPmechdie3,
	SPhitlerwalk1,
	SPhitlerwalk2,
	SPhitlerwalk3,
	SPhitlerwalk4,
	SPhitlerfire1,
	SPhitlerfire2,
	SPhitlerfire3,
	SPhitlerdead,
	SPhitlerdie1,
	SPhitlerdie2,
	SPhitlerdie3,
	SPhitlerdie4,
	SPhitlerdie5,
	SPhitlerdie6,
	SPhitlerdie7,
	SPottowalk1,
	SPottowalk2,
	SPottowalk3,
	SPottowalk4,
	SPottofire1,
	SPottofire2,
	SPottodie1,
	SPottodie2,
	SPottodie3,
	SPottodead,
	SPmissile1,
	SPmsmoke1 = SPmissile1 + 8,
	SPmsmoke2,
	SPmsmoke3,
	SPmsmoke4,
	SPmboom1,
	SPmboom2,
	SPmboom3,
	SPgretelwalk1,
	SPgretelwalk2,
	SPgretelwalk3,
	SPgretelwalk4,
	SPgretelfire1,
	SPgretelfire2,
	SPgretelfire3,
	SPgreteldead,
	SPgreteldie1,
	SPgreteldie2,
	SPgreteldie3,
	SPfettwalk1,
	SPfettwalk2,
	SPfettwalk3,
	SPfettwalk4,
	SPfettfire1,
	SPfettfire2,
	SPfettfire3,
	SPfettfire4,
	SPfettdie1,
	SPfettdie2,
	SPfettdie3,
	SPfettdead,
	SPbjwalk1,
	SPbjwalk2,
	SPbjwalk3,
	SPbjwalk4,
	SPbjjump1,
	SPbjjump2,
	SPbjjump3,
	SPbjjump4,
	SPknife,
	SPpistol = SPknife + 5,
	SPmg = SPpistol + 5,
	SPgatling = SPmg + 5,
	SProcket1 = SPofcfire3 + 20,
	SPrsmoke1 = SProcket1 + 8,
	SPrsmoke2,
	SPrsmoke3,
	SPrsmoke4,
	SPrboom1,
	SPrboom2,
	SPrboom3,
	SPspark1,
	SPspark2,
	SPspark3,
	SPspark4,
	SPtranswalk1,
	SPtranswalk2,
	SPtranswalk3,
	SPtranswalk4,
	SPtransfire1,
	SPtransfire2,
	SPtransfire3,
	SPtransdead,
	SPtransdie1,
	SPtransdie2,
	SPtransdie3,
	SPwilhwalk1,
	SPwilhwalk2,
	SPwilhwalk3,
	SPwilhwalk4,
	SPwilhfire1,
	SPwilhfire2,
	SPwilhfire3,
	SPwilhfire4,
	SPwilhdie1,
	SPwilhdie2,
	SPwilhdie3,
	SPwilhdead,
	SPuberwalk1,
	SPuberwalk2,
	SPuberwalk3,
	SPuberwalk4,
	SPuberfire1,
	SPuberfire2,
	SPuberfire3,
	SPuberfire4,
	SPuberdie1,
	SPuberdie2,
	SPuberdie3,
	SPuberdie4,
	SPuberdead,
	SPknightwalk1,
	SPknightwalk2,
	SPknightwalk3,
	SPknightwalk4,
	SPknightfire1,
	SPknightfire2,
	SPknightfire3,
	SPknightfire4,
	SPknightdie1,
	SPknightdie2,
	SPknightdie3,
	SPknightdie4,
	SPknightdie5,
	SPknightdie6,
	SPknightdead,
	SPspectrewalk1,
	SPspectrewalk2,
	SPspectrewalk3,
	SPspectrewalk4,
	SPspectreF1,
	SPspectreF2,
	SPspectreF3,
	SPspectreF4,
	SPangelwalk1,
	SPangelwalk2,
	SPangelwalk3,
	SPangelwalk4,
	SPangelfire1,
	SPangelfire2,
	SPangeltired1,
	SPangeltired2,
	SPangeldie1,
	SPangeldie2,
	SPangeldie3,
	SPangeldie4,
	SPangeldie5,
	SPangeldie6,
	SPangeldie7,
	SPangeldead
};

enum{
	Dtlshift = 16,
	Dtlglobal = 1 << Dtlshift,
	Dmin = 0x5800,
	Dplr = Dmin,
	Domin = 0x10000
};
#define	Fpi	3.14159265358979323846	/* bcpp 1.3 */
extern s32int sint[], *cost;

struct View{
	int dx;
	int dy;
	int ofs;
	int x;
	int y;
	int tx;
	int ty;
	int θ;
	int cos;
	int sin;
	int mid;
	int Δhit;
};
extern View vw;

enum{
	Oplr = 1,
	Oblaz,
	Ogd,
	Oss,
	Oofc,
	Omut,
	Odog,
	Ohans,
	Oschb,
	Ogretel,
	Ootto,
	Ofett,
	Ofake,
	Omech,
	Ohitler,
	Oghost,
	Otrans,
	Owilh,
	Ouber,
	Oknight,
	Ospectre,
	Oangel,
	Orocket,
	Omissile,
	Oflame,
	Oneedle,
	Ospark,
	Oinert,

	OFshootable = 1<<0,
	OFbonus = 1<<1,
	OFnevermark = 1<<2,
	OFvis = 1<<3,
	OFattack = 1<<4,
	OFflip = 1<<5,
	OFambush = 1<<6,
	OFnomark = 1<<7,

	Rnil = 0,
	Rblock,
	Rgibs,
	Ralpo,
	Rstim,
	Rkey1,
	Rkey2,
	Rkey3,
	Rkey4,
	Rcross,
	Rchalice,
	Rbible,
	Rcrown,
	Rclip1,
	Rclip2,
	Rmg,
	Rchaingun,
	Rfood,
	R1up,
	Rammobox,
	Rspear,

	GSplr = 0,
	GSplrcam,
	GSblaz1,
	GSblaz2,
	GSblaz3,
	GSblaz4,
	GSblaz5,
	GSblaz6,
	GSjump1,
	GSjump2,
	GSjump3,
	GSjump4,
	GSgd,
	GSgdwalk1,
	GSgdwalk2,
	GSgdwalk3,
	GSgdwalk4,
	GSgdwalk5,
	GSgdwalk6,
	GSgdpain1,
	GSgdpain2,
	GSgdchase1,
	GSgdchase2,
	GSgdchase3,
	GSgdchase4,
	GSgdchase5,
	GSgdchase6,
	GSgdfire1,
	GSgdfire2,
	GSgdfire3,
	GSgddie1,
	GSgddie2,
	GSgddie3,
	GSgddie4,
	GSss,
	GSsswalk1,
	GSsswalk2,
	GSsswalk3,
	GSsswalk4,
	GSsswalk5,
	GSsswalk6,
	GSsspain1,
	GSsspain2,
	GSsschase1,
	GSsschase2,
	GSsschase3,
	GSsschase4,
	GSsschase5,
	GSsschase6,
	GSssfire1,
	GSssfire2,
	GSssfire3,
	GSssfire4,
	GSssfire5,
	GSssfire6,
	GSssfire7,
	GSssfire8,
	GSssfire9,
	GSssdie1,
	GSssdie2,
	GSssdie3,
	GSssdie4,
	GSofc,
	GSofcwalk1,
	GSofcwalk2,
	GSofcwalk3,
	GSofcwalk4,
	GSofcwalk5,
	GSofcwalk6,
	GSofcpain1,
	GSofcpain2,
	GSofcchase1,
	GSofcchase2,
	GSofcchase3,
	GSofcchase4,
	GSofcchase5,
	GSofcchase6,
	GSofcfire1,
	GSofcfire2,
	GSofcfire3,
	GSofcdie1,
	GSofcdie2,
	GSofcdie3,
	GSofcdie4,
	GSofcdie5,
	GSmut,
	GSmutwalk1,
	GSmutwalk2,
	GSmutwalk3,
	GSmutwalk4,
	GSmutwalk5,
	GSmutwalk6,
	GSmutpain1,
	GSmutpain2,
	GSmutchase1,
	GSmutchase2,
	GSmutchase3,
	GSmutchase4,
	GSmutchase5,
	GSmutchase6,
	GSmutfire1,
	GSmutfire2,
	GSmutfire3,
	GSmutfire4,
	GSmutdie1,
	GSmutdie2,
	GSmutdie3,
	GSmutdie4,
	GSmutdie5,
	GSdogwalk1,
	GSdogwalk2,
	GSdogwalk3,
	GSdogwalk4,
	GSdogwalk5,
	GSdogwalk6,
	GSdogchase1,
	GSdogchase2,
	GSdogchase3,
	GSdogchase4,
	GSdogchase5,
	GSdogchase6,
	GSdogfire1,
	GSdogfire2,
	GSdogfire3,
	GSdogfire4,
	GSdogfire5,
	GSdogdie1,
	GSdogdie2,
	GSdogdie3,
	GSdogdie4,
	GShans,
	GShanschase1,
	GShanschase2,
	GShanschase3,
	GShanschase4,
	GShanschase5,
	GShanschase6,
	GShansfire1,
	GShansfire2,
	GShansfire3,
	GShansfire4,
	GShansfire5,
	GShansfire6,
	GShansfire7,
	GShansfire8,
	GShansdie1,
	GShansdie2,
	GShansdie3,
	GShansdie4,
	GSschb,
	GSschbchase1,
	GSschbchase2,
	GSschbchase3,
	GSschbchase4,
	GSschbchase5,
	GSschbchase6,
	GSschbfire1,
	GSschbfire2,
	GSschbcam,
	GSschbdie1,
	GSschbdie2,
	GSschbdie3,
	GSschbdie4,
	GSschbdie5,
	GSschbdie6,
	GSgretel,
	GSgretelchase1,
	GSgretelchase2,
	GSgretelchase3,
	GSgretelchase4,
	GSgretelchase5,
	GSgretelchase6,
	GSgretelfire1,
	GSgretelfire2,
	GSgretelfire3,
	GSgretelfire4,
	GSgretelfire5,
	GSgretelfire6,
	GSgretelfire7,
	GSgretelfire8,
	GSgreteldie1,
	GSgreteldie2,
	GSgreteldie3,
	GSgreteldie4,
	GSotto,
	GSottochase1,
	GSottochase2,
	GSottochase3,
	GSottochase4,
	GSottochase5,
	GSottochase6,
	GSottofire1,
	GSottofire2,
	GSottocam,
	GSottodie1,
	GSottodie2,
	GSottodie3,
	GSottodie4,
	GSottodie5,
	GSottodie6,
	GSfett,
	GSfettchase1,
	GSfettchase2,
	GSfettchase3,
	GSfettchase4,
	GSfettchase5,
	GSfettchase6,
	GSfettfire1,
	GSfettfire2,
	GSfettfire3,
	GSfettfire4,
	GSfettfire5,
	GSfettfire6,
	GSfettcam,
	GSfettdie1,
	GSfettdie2,
	GSfettdie3,
	GSfettdie4,
	GSfettdie5,
	GSfettdie6,
	GSfake,
	GSfakechase1,
	GSfakechase2,
	GSfakechase3,
	GSfakechase4,
	GSfakechase5,
	GSfakechase6,
	GSfakefire1,
	GSfakefire2,
	GSfakefire3,
	GSfakefire4,
	GSfakefire5,
	GSfakefire6,
	GSfakefire7,
	GSfakefire8,
	GSfakefire9,
	GSfakedie1,
	GSfakedie2,
	GSfakedie3,
	GSfakedie4,
	GSfakedie5,
	GSfakedie6,
	GSmech,
	GSmechchase1,
	GSmechchase2,
	GSmechchase3,
	GSmechchase4,
	GSmechchase5,
	GSmechchase6,
	GSmechfire1,
	GSmechfire2,
	GSmechfire3,
	GSmechfire4,
	GSmechfire5,
	GSmechfire6,
	GSmechdie1,
	GSmechdie2,
	GSmechdie3,
	GSmechdie4,
	GShitlerchase1,
	GShitlerchase2,
	GShitlerchase3,
	GShitlerchase4,
	GShitlerchase5,
	GShitlerchase6,
	GShitlerfire1,
	GShitlerfire2,
	GShitlerfire3,
	GShitlerfire4,
	GShitlerfire5,
	GShitlerfire6,
	GShitlercam,
	GShitlerdie1,
	GShitlerdie2,
	GShitlerdie3,
	GShitlerdie4,
	GShitlerdie5,
	GShitlerdie6,
	GShitlerdie7,
	GShitlerdie8,
	GShitlerdie9,
	GShitlerdie10,
	GSgh1chase1,
	GSgh2chase1,
	GSgh3chase1,
	GSgh4chase1,
	GSgh1chase2,
	GSgh2chase2,
	GSgh3chase2,
	GSgh4chase2,
	GStrans,
	GStranschase1,
	GStranschase2,
	GStranschase3,
	GStranschase4,
	GStranschase5,
	GStranschase6,
	GStransfire1,
	GStransfire2,
	GStransfire3,
	GStransfire4,
	GStransfire5,
	GStransfire6,
	GStransfire7,
	GStransfire8,
	GStransdie1,
	GStransdie2,
	GStransdie3,
	GStransdie4,
	GStransdie5,
	GStransdie6,
	GSwilh,
	GSwilhchase1,
	GSwilhchase2,
	GSwilhchase3,
	GSwilhchase4,
	GSwilhchase5,
	GSwilhchase6,
	GSwilhfire1,
	GSwilhfire2,
	GSwilhfire3,
	GSwilhfire4,
	GSwilhfire5,
	GSwilhfire6,
	GSwilhdie1,
	GSwilhdie2,
	GSwilhdie3,
	GSwilhdie4,
	GSwilhdie5,
	GSwilhdie6,
	GSuber,
	GSuberchase1,
	GSuberchase2,
	GSuberchase3,
	GSuberchase4,
	GSuberchase5,
	GSuberchase6,
	GSuberfire1,
	GSuberfire2,
	GSuberfire3,
	GSuberfire4,
	GSuberfire5,
	GSuberfire6,
	GSuberfire7,
	GSuberdie1,
	GSuberdie2,
	GSuberdie3,
	GSuberdie4,
	GSuberdie5,
	GSuberdie6,
	GSuberdie7,
	GSknight,
	GSknightchase1,
	GSknightchase2,
	GSknightchase3,
	GSknightchase4,
	GSknightchase5,
	GSknightchase6,
	GSknightfire1,
	GSknightfire2,
	GSknightfire3,
	GSknightfire4,
	GSknightfire5,
	GSknightdie1,
	GSknightdie2,
	GSknightdie3,
	GSknightdie4,
	GSknightdie5,
	GSknightdie6,
	GSknightdie7,
	GSknightdie8,
	GSknightdie9,
	GSspectrewait1,
	GSspectrewait2,
	GSspectrewait3,
	GSspectrewait4,
	GSspectrewake,
	GSspectrechase1,
	GSspectrechase2,
	GSspectrechase3,
	GSspectrechase4,
	GSspectredie1,
	GSspectredie2,
	GSspectredie3,
	GSspectredie4,
	GSangel,
	GSangelchase1,
	GSangelchase2,
	GSangelchase3,
	GSangelchase4,
	GSangelchase5,
	GSangelchase6,
	GSangeldie1,
	GSangeldie2,
	GSangeldie3,
	GSangeldie4,
	GSangeldie5,
	GSangeldie6,
	GSangeldie7,
	GSangeldie8,
	GSangeldie9,
	GSangeldie10,
	GSangelfire1,
	GSangelfire2,
	GSangelfire3,
	GSangeltired1,
	GSangeltired2,
	GSangeltired3,
	GSangeltired4,
	GSangeltired5,
	GSangeltired6,
	GSangeltired7,
	GSmissile,
	GSmsmoke1,
	GSmsmoke2,
	GSmsmoke3,
	GSmsmoke4,
	GSmboom1,
	GSmboom2,
	GSmboom3,
	GSrocket,
	GSrsmoke1,
	GSrsmoke2,
	GSrsmoke3,
	GSrsmoke4,
	GSrboom1,
	GSrboom2,
	GSrboom3,
	GSflame1,
	GSflame2,
 	GSneedle1,
	GSneedle2,
	GSneedle3,
	GSneedle4,
	GSspark1,
	GSspark2,
	GSspark3,
	GSspark4,
	GSe,

	θE = 0,
	θNE = 45,
	θN = 90,
	θNW = 135,
	θW = 180,
	θSW = 225,
	θS = 270,
	θSE = 315,
	θnil = -1,

	DRshut = 0,
	DRopening,
	DRopen,
	DRclosing,

	DRunlk = 0,
	DRlock1,
	DRlock2,
	DRlock3,
	DRlock4,
	DRup
};
struct State{
	void (*up)(Obj *);
	void (*act)(Obj *);
	int dt;
	int sprn;
	State *n;
	int rot;
	Spr *spr;
};
extern State stt[];
struct Obj{
	int on;
	int type;
	int f;
	State *s;
	int tc;
	int hp;
	int v;
	int θ;
	int Δr;
	Tile *tl;
	int tx;
	int ty;
	int x;
	int y;
	int vwx;
	int vwdx;
	int vwdy;
	int areaid;
	int atkdt;
	int sdt;
	Obj *n;
	Obj *p;
};
extern Obj *objs, *ofree, *oplr;
struct Door{
	Tile *tl;
	int isvert;	/* also push direction */
	int lock;
	int φ;
	int tc;
	u16int dopen;
};
extern Door doors[], *doore, pusher;
struct Static{
	Tile *tl;
	Spr *spr;
	int f;
	int item;
};
extern Static stcs[], *stce;

enum{
	Mapdxy = 64,
	Mapa = Mapdxy * Mapdxy,

	MTgoup = 21,
	MTarrows = 90,
	MTpush = 98,
	MTexit = 99,
	MTambush = 106,
	MTsetec = 107,
	MTfloor = MTsetec,

	Nobj = 150,
	Ndoor = 64,
	Nstc = 400,
	Narea = 37
};
struct Tile{
	u16int p0;
	u16int p1;
	uchar tl;
	Obj *o;
	uchar to;
	uchar vis;
};
extern Tile tiles[Mapa];
extern uchar plrarea[], conarea[];
extern int wspr[];

enum{
	GPextra = 40000,

	GDbaby = 0,
	GDeasy,
	GDmed,
	GDhard,

	WPknife = 0,
	WPpistol,
	WPmg,
	WPgatling,
	WPnone,

	GMup = 0,
	GMsetec,
	GMret,

	EDfizz = 1,
	EDcam,
	EDcam2,
	EDdem,
	EDkey,
	EDdie,
	EDup,
	EDsetec,
	EDwon,
	EDspear,
	EDmsg
};
struct Game{
	int hp;
	int w;
	int lastw;
	int bestw;
	int ammo;
	int lives;
	int keys;
	int pt;
	int oldpt;
	int to1up;
	int wfrm;
	int facefrm;
	int map;
	int difc;
	int tc;
	int eptm;
	int kp;
	int ktot;
	int epk;
	int sp;
	int stot;
	int eps;
	int tp;
	int ttot;
	int ept;
	int won;
	int mut;
	int end;
	int com;
	int demo;
	int record;
};
extern Game gm;
extern int allrecv, god, noclip, slomo;
extern int loaded;

struct Score{
	char name[58];
	int n;
	int lvl;
	int ep;
};
extern Score scs[];
extern char savs[][32];
