#define	MaxHighName	57
#define	MaxScores	7
typedef	struct
		{
			char	name[MaxHighName + 1];
			s32int	score;
			u16int	completed,episode;
		} HighScore;

#define	MaxString	128	// Maximum input string size

extern	HighScore	Scores[];
