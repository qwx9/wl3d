#define	MaxPlayers	4
#define	NumCodes	128

// 	Stuff for the mouse
#define	MReset		0
#define	MButtons	3
#define	MDelta		11

#define	MouseInt	0x33
#define	Mouse(x)	_AX = x,geninterrupt(MouseInt)

typedef	enum		{
						demo_Off,demo_Record,demo_Playback,demo_PlayDone
					} Demo;
typedef	enum		{
						ctrl_Keyboard,
							ctrl_Keyboard1 = ctrl_Keyboard,ctrl_Keyboard2,
						ctrl_Mouse
					} ControlType;
typedef	enum		{
						motion_Left = -1,motion_Up = -1,
						motion_None = 0,
						motion_Right = 1,motion_Down = 1
					} Motion;
typedef	enum		{
						dir_North,dir_NorthEast,
						dir_East,dir_SouthEast,
						dir_South,dir_SouthWest,
						dir_West,dir_NorthWest,
						dir_None
					} Direction;
typedef	struct		{
						int		button0,button1,button2,button3;
						s16int			x,y;
						Motion		xaxis,yaxis;
						Direction	dir;
					} CursorInfo;
typedef	CursorInfo	ControlInfo;
typedef	struct		{
						u8int	button0,button1,
									upleft,		up,		upright,
									left,				right,
									downleft,	down,	downright;
					} KeyboardDef;
// Global variables
extern	int		Keyboard[], MousePresent;
extern	int		Paused;
extern	char		LastASCII;
extern	u8int	LastScan;
extern	KeyboardDef	KbdDefs;
extern	ControlType	Controls[MaxPlayers];

extern	Demo		DemoMode;
extern	u8int _seg	*DemoBuffer;
extern	u16int		DemoOffset,DemoSize;

// Function prototypes
#define	IN_KeyDown(code)	(Keyboard[(code)])
#define	IN_ClearKey(code)	{Keyboard[code] = false;\
							if (code == LastScan) LastScan = sc_None;}
