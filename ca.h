extern	s16int			mapon;
extern	void		_seg	*grsegs[NUMCHUNKS];
extern	u8int		far	grneeded[NUMCHUNKS];
extern	u8int		ca_levelbit,ca_levelnum;
extern	char		*titleptr[8];

void CA_Startup (void);
void CA_UpLevel (void);
void CA_DownLevel (void);
void CA_ClearMarks (void);
void CA_ClearAllMarks (void);
void CA_CacheMarks (void);
void CA_CacheScreen (s16int chunk);
