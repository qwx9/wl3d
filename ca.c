s16int			mapon;

void		_seg	*grsegs[NUMCHUNKS];

u8int		far	grneeded[NUMCHUNKS];
u8int		ca_levelbit,ca_levelnum;

s32int		_seg *grstarts;	// array of offsets in egagraph, -1 for sparse
s16int			grhandle;		// handle to EGAGRAPH
s32int		chunkcomplen,chunkexplen;

s32int GRFILEPOS(s16int c)
{
	s32int value;
	s16int	offset;

	offset = c*3;

	value = *(s32int far *)(((u8int far *)grstarts)+offset);

	value &= 0x00ffffffl;

	if (value == 0xffffffl)
		value = -1;

	return value;
};

/*
======================
=
= CAL_HuffExpand
=
= Length is the length of the EXPANDED data
= If screenhack, the data is decompressed in four planes directly
= to the screen
=
======================
*/

void CAL_HuffExpand (u8int huge *source, u8int huge *dest,
  s32int length,huffnode *hufftable, int screenhack)
{
  u16int sourceseg,sourceoff,destseg,destoff,endoff;
  huffnode *headptr;
  u8int		mapmask;

  headptr = hufftable+254;	// head node is allways node 254

  if (screenhack)
  {
	mapmask = 1;
asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK + 256
asm	out	dx,ax
	length >>= 2;
  }

	// setup
expandshort:
	{
		// loop
	}

asm	test	[screenhack],1
asm	jz	notscreen
asm	shl	[mapmask],1
asm	mov	ah,[mapmask]
asm	cmp	ah,16
asm	je	notscreen			// all four planes done
asm	mov	dx,SC_INDEX
asm	mov	al,SC_MAPMASK
asm	out	dx,ax
asm	mov	di,[destoff]
asm	mov	ax,[endoff]
asm	jmp	expandshort

notscreen:;
}

void CA_Startup (void)
{
	mapon = -1;
	ca_levelbit = 1;
	ca_levelnum = 0;
}

/*
======================
=
= CA_CacheScreen
=
= Decompresses a chunk from disk straight onto the screen
=
======================
*/

void CA_CacheScreen (s16int chunk)
{
	s32int	pos,compressed,expanded;
	uchar *bigbufferseg;
	u8int	far *source;
	s16int		next;

//
// load the chunk into a buffer
//
	pos = GRFILEPOS(chunk);
	next = chunk +1;
	while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
		next++;
	compressed = GRFILEPOS(next)-pos;

	lseek(grhandle,pos,SEEK_SET);

	MM_GetPtr(&bigbufferseg,compressed);
	MM_SetLock (&bigbufferseg,true);
	CA_FarRead(grhandle,bigbufferseg,compressed);
	source = bigbufferseg;

	expanded = *(s32int far *)source;
	source += 4;			// skip over length

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
	CAL_HuffExpand (source,MK_FP(SCREENSEG,bufferofs),expanded,grhuffman,true);
	VW_MarkUpdateBlock (0,0,319,199);
	MM_FreePtr(&bigbufferseg);
}

/*
======================
=
= CA_UpLevel
=
= Goes up a bit level in the needed lists and clears it out.
= Everything is made purgable
=
======================
*/

void CA_UpLevel (void)
{
	s16int	i;

	if (ca_levelnum==7)
		Quit ("CA_UpLevel: Up past level 7!");

	for (i=0;i<NUMCHUNKS;i++)
		if (grsegs[i])
			MM_SetPurge (&(uchar *)grsegs[i],3);
	ca_levelbit<<=1;
	ca_levelnum++;
}

//===========================================================================

/*
======================
=
= CA_DownLevel
=
= Goes down a bit level in the needed lists and recaches
= everything from the lower level
=
======================
*/

void CA_DownLevel (void)
{
	if (!ca_levelnum)
		Quit ("CA_DownLevel: Down past level 0!");
	ca_levelbit>>=1;
	ca_levelnum--;
	CA_CacheMarks();
}

/*
======================
=
= CA_CacheMarks
=
======================
*/
#define MAXEMPTYREAD	1024

void CA_CacheMarks (void)
{
	s16int 	i,next,numcache;
	s32int	pos,endpos,nextpos,nextendpos,compressed;
	s32int	bufferstart,bufferend;	// file position of general buffer
	u8int	far *source;
	uchar *bigbufferseg;

	numcache = 0;
//
// go through and make everything not needed purgable
//
	for (i=0;i<NUMCHUNKS;i++)
		if (grneeded[i]&ca_levelbit)
		{
			if (grsegs[i])					// its allready in memory, make
				MM_SetPurge(&grsegs[i],0);	// sure it stays there!
			else
				numcache++;
		}
		else
		{
			if (grsegs[i])					// not needed, so make it purgeable
				MM_SetPurge(&grsegs[i],3);
		}

	if (!numcache)			// nothing to cache!
		return;


//
// go through and load in anything still needed
//
	bufferstart = bufferend = 0;		// nothing good in buffer now

	for (i=0;i<NUMCHUNKS;i++)
		if ( (grneeded[i]&ca_levelbit) && !grsegs[i])
		{
			pos = GRFILEPOS(i);
			if (pos<0)
				continue;

			next = i +1;
			while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
				next++;

			compressed = GRFILEPOS(next)-pos;
			endpos = pos+compressed;

			if (compressed<=BUFFERSIZE)
			{
				if (bufferstart<=pos
				&& bufferend>= endpos)
				{
				// data is allready in buffer
					source = (u8int _seg *)bufferseg+(pos-bufferstart);
				}
				else
				{
				// load buffer with a new block from disk
				// try to get as many of the needed blocks in as possible
					while ( next < NUMCHUNKS )
					{
						while (next < NUMCHUNKS &&
						!(grneeded[next]&ca_levelbit && !grsegs[next]))
							next++;
						if (next == NUMCHUNKS)
							continue;

						nextpos = GRFILEPOS(next);
						while (GRFILEPOS(++next) == -1)	// skip past any sparse tiles
							;
						nextendpos = GRFILEPOS(next);
						if (nextpos - endpos <= MAXEMPTYREAD
						&& nextendpos-pos <= BUFFERSIZE)
							endpos = nextendpos;
						else
							next = NUMCHUNKS;			// read pos to posend
					}

					lseek(grhandle,pos,SEEK_SET);
					CA_FarRead(grhandle,bufferseg,endpos-pos);
					bufferstart = pos;
					bufferend = endpos;
					source = bufferseg;
				}
			}
			else
			{
			// big chunk, allocate temporary buffer
				MM_GetPtr(&bigbufferseg,compressed);
				if (mmerror)
					return;
				MM_SetLock (&bigbufferseg,true);
				lseek(grhandle,pos,SEEK_SET);
				CA_FarRead(grhandle,bigbufferseg,compressed);
				source = bigbufferseg;
			}

			CAL_ExpandGrChunk (i,source);
			if (mmerror)
				return;

			if (compressed>BUFFERSIZE)
				MM_FreePtr(&bigbufferseg);

		}
}
