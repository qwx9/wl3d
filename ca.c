// ID_CA.C

// this has been customized for WOLF

/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/

#include "ID_HEADS.H"
#pragma hdrstop

#pragma warn -pro
#pragma warn -use

#define THREEBYTEGRSTARTS

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

typedef struct
{
  u16int bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;


typedef struct
{
	u16int	RLEWtag;
	s32int		headeroffsets[100];
	u8int		tileinfo[];
} mapfiletype;


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

u8int 		_seg	*tinf;
s16int			mapon;

u16int	_seg	*mapsegs[MAPPLANES];
maptype		_seg	*mapheaderseg[NUMMAPS];
u8int		_seg	*audiosegs[NUMSNDCHUNKS];
void		_seg	*grsegs[NUMCHUNKS];

u8int		far	grneeded[NUMCHUNKS];
u8int		ca_levelbit,ca_levelnum;

s16int			profilehandle,debughandle;

char		audioname[13]="AUDIO.";

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

extern	s32int	far	CGAhead;
extern	s32int	far	EGAhead;
extern	u8int	CGAdict;
extern	u8int	EGAdict;
extern	u8int	far	maphead;
extern	u8int	mapdict;
extern	u8int	far	audiohead;
extern	u8int	audiodict;


char extension[5],	// Need a string, not constant to change cache files
     gheadname[10]=GREXT"HEAD.",
     gfilename[10]=GREXT"GRAPH.",
     gdictname[10]=GREXT"DICT.",
     mheadname[10]="MAPHEAD.",
     mfilename[10]="MAPTEMP.",
     aheadname[10]="AUDIOHED.",
     afilename[10]="AUDIOT.";

void CA_CannotOpen(char *string);

s32int		_seg *grstarts;	// array of offsets in egagraph, -1 for sparse
s32int		_seg *audiostarts;	// array of offsets in audio / audiot

#ifdef GRHEADERLINKED
huffnode	*grhuffman;
#else
huffnode	grhuffman[255];
#endif

#ifdef AUDIOHEADERLINKED
huffnode	*audiohuffman;
#else
huffnode	audiohuffman[255];
#endif


s16int			grhandle;		// handle to EGAGRAPH
s16int			maphandle;		// handle to MAPTEMP / GAMEMAPS
s16int			audiohandle;	// handle to AUDIOT / AUDIO

s32int		chunkcomplen,chunkexplen;

SDMode		oldsoundmode;



void	CAL_CarmackExpand (u16int far *source, u16int far *dest,
		u16int length);


#ifdef THREEBYTEGRSTARTS
#define FILEPOSSIZE	3
//#define	GRFILEPOS(c) (*(s32int far *)(((u8int far *)grstarts)+(c)*3)&0xffffff)
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
#else
#define FILEPOSSIZE	4
#define	GRFILEPOS(c) (grstarts[c])
#endif

/*
=============================================================================

					   LOW LEVEL ROUTINES

=============================================================================
*/

/*
============================
=
= CA_OpenDebug / CA_CloseDebug
=
= Opens a binary file with the handle "debughandle"
=
============================
*/

void CA_OpenDebug (void)
{
	unlink ("DEBUG.TXT");
	debughandle = open("DEBUG.TXT", O_CREAT | O_WRONLY | O_TEXT);
}

void CA_CloseDebug (void)
{
	close (debughandle);
}



/*
============================
=
= CAL_GetGrChunkLength
=
= Gets the length of an explicit length chunk (not tiles)
= The file pointer is positioned so the compressed data can be read in next.
=
============================
*/

void CAL_GetGrChunkLength (s16int chunk)
{
	lseek(grhandle,GRFILEPOS(chunk),SEEK_SET);
	read(grhandle,&chunkexplen,sizeof(chunkexplen));
	chunkcomplen = GRFILEPOS(chunk+1)-GRFILEPOS(chunk)-4;
}


/*
==========================
=
= CA_FarRead
=
= Read from a file to a far pointer
=
==========================
*/

int CA_FarRead (s16int handle, u8int far *dest, s32int length)
{
	if (length>0xffffl)
		Quit ("CA_FarRead doesn't support 64K reads yet!");

asm		push	ds
asm		mov	bx,[handle]
asm		mov	cx,[WORD PTR length]
asm		mov	dx,[WORD PTR dest]
asm		mov	ds,[WORD PTR dest+2]
asm		mov	ah,0x3f				// READ w/handle
asm		int	21h
asm		pop	ds
asm		jnc	good
	errno = _AX;
	return	false;
good:
asm		cmp	ax,[WORD PTR length]
asm		je	done
	errno = EINVFMT;			// user manager knows this is bad read
	return	false;
done:
	return	true;
}


/*
==========================
=
= CA_SegWrite
=
= Write from a file to a far pointer
=
==========================
*/

int CA_FarWrite (s16int handle, u8int far *source, s32int length)
{
	if (length>0xffffl)
		Quit ("CA_FarWrite doesn't support 64K reads yet!");

asm		push	ds
asm		mov	bx,[handle]
asm		mov	cx,[WORD PTR length]
asm		mov	dx,[WORD PTR source]
asm		mov	ds,[WORD PTR source+2]
asm		mov	ah,0x40			// WRITE w/handle
asm		int	21h
asm		pop	ds
asm		jnc	good
	errno = _AX;
	return	false;
good:
asm		cmp	ax,[WORD PTR length]
asm		je	done
	errno = ENOMEM;				// user manager knows this is bad write
	return	false;

done:
	return	true;
}


/*
==========================
=
= CA_ReadFile
=
= Reads a file into an allready allocated buffer
=
==========================
*/

int CA_ReadFile (char *filename, uchar **ptr)
{
	s16int handle;
	s32int size;

	if ((handle = open(filename,O_RDONLY | O_BINARY, S_IREAD)) == -1)
		return false;

	size = filelength (handle);
	if (!CA_FarRead (handle,*ptr,size))
	{
		close (handle);
		return false;
	}
	close (handle);
	return true;
}


/*
==========================
=
= CA_WriteFile
=
= Writes a file from a memory buffer
=
==========================
*/

int CA_WriteFile (char *filename, void far *ptr, s32int length)
{
	s16int handle;
	s32int size;

	handle = open(filename,O_CREAT | O_BINARY | O_WRONLY,
				S_IREAD | S_IWRITE | S_IFREG);

	if (handle == -1)
		return false;

	if (!CA_FarWrite (handle,ptr,length))
	{
		close (handle);
		return false;
	}
	close (handle);
	return true;
}



/*
==========================
=
= CA_LoadFile
=
= Allocate space for and load a file
=
==========================
*/

int CA_LoadFile (char *filename, uchar **ptr)
{
	s16int handle;
	s32int size;

	if ((handle = open(filename,O_RDONLY | O_BINARY, S_IREAD)) == -1)
		return false;

	size = filelength (handle);
	MM_GetPtr (ptr,size);
	if (!CA_FarRead (handle,*ptr,size))
	{
		close (handle);
		return false;
	}
	close (handle);
	return true;
}

/*
============================================================================

		COMPRESSION routines, see JHUFF.C for more

============================================================================
*/



/*
===============
=
= CAL_OptimizeNodes
=
= Goes through a huffman table and changes the 256-511 node numbers to the
= actular address of the node.  Must be called before CAL_HuffExpand
=
===============
*/

void CAL_OptimizeNodes (huffnode *table)
{
  huffnode *node;
  s16int i;

  node = table;

  for (i=0;i<255;i++)
  {
	if (node->bit0 >= 256)
	  node->bit0 = (u16int)(table+(node->bit0-256));
	if (node->bit1 >= 256)
	  node->bit1 = (u16int)(table+(node->bit1-256));
	node++;
  }
}



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
//  u16int bit,byte,node,code;
  u16int sourceseg,sourceoff,destseg,destoff,endoff;
  huffnode *headptr;
  u8int		mapmask;
//  huffnode *nodeon;

  headptr = hufftable+254;	// head node is allways node 254

  source++;	// normalize
  source--;
  dest++;
  dest--;

  if (screenhack)
  {
	mapmask = 1;
asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK + 256
asm	out	dx,ax
	length >>= 2;
  }

  sourceseg = FP_SEG(source);
  sourceoff = FP_OFF(source);
  destseg = FP_SEG(dest);
  destoff = FP_OFF(dest);
  endoff = destoff+length;

//
// ds:si source
// es:di dest
// ss:bx node pointer
//

	if (length <0xfff0)
	{

//--------------------------
// expand less than 64k of data
//--------------------------

asm mov	bx,[headptr]

asm	mov	si,[sourceoff]
asm	mov	di,[destoff]
asm	mov	es,[destseg]
asm	mov	ds,[sourceseg]
asm	mov	ax,[endoff]

asm	mov	ch,[si]				// load first byte
asm	inc	si
asm	mov	cl,1

expandshort:
asm	test	ch,cl			// bit set?
asm	jnz	bit1short
asm	mov	dx,[ss:bx]			// take bit0 path from node
asm	shl	cl,1				// advance to next bit position
asm	jc	newbyteshort
asm	jnc	sourceupshort

bit1short:
asm	mov	dx,[ss:bx+2]		// take bit1 path
asm	shl	cl,1				// advance to next bit position
asm	jnc	sourceupshort

newbyteshort:
asm	mov	ch,[si]				// load next byte
asm	inc	si
asm	mov	cl,1				// back to first bit

sourceupshort:
asm	or	dh,dh				// if dx<256 its a byte, else move node
asm	jz	storebyteshort
asm	mov	bx,dx				// next node = (huffnode *)code
asm	jmp	expandshort

storebyteshort:
asm	mov	[es:di],dl
asm	inc	di					// write a decopmpressed byte out
asm	mov	bx,[headptr]		// back to the head node for next bit

asm	cmp	di,ax				// done?
asm	jne	expandshort

//
// perform screenhack if needed
//
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
	else
	{

//--------------------------
// expand more than 64k of data
//--------------------------

  length--;

asm mov	bx,[headptr]
asm	mov	cl,1

asm	mov	si,[sourceoff]
asm	mov	di,[destoff]
asm	mov	es,[destseg]
asm	mov	ds,[sourceseg]

asm	lodsb			// load first byte

expand:
asm	test	al,cl		// bit set?
asm	jnz	bit1
asm	mov	dx,[ss:bx]	// take bit0 path from node
asm	jmp	gotcode
bit1:
asm	mov	dx,[ss:bx+2]	// take bit1 path

gotcode:
asm	shl	cl,1		// advance to next bit position
asm	jnc	sourceup
asm	lodsb
asm	cmp	si,0x10		// normalize ds:si
asm  	jb	sinorm
asm	mov	cx,ds
asm	inc	cx
asm	mov	ds,cx
asm	xor	si,si
sinorm:
asm	mov	cl,1		// back to first bit

sourceup:
asm	or	dh,dh		// if dx<256 its a byte, else move node
asm	jz	storebyte
asm	mov	bx,dx		// next node = (huffnode *)code
asm	jmp	expand

storebyte:
asm	mov	[es:di],dl
asm	inc	di		// write a decopmpressed byte out
asm	mov	bx,[headptr]	// back to the head node for next bit

asm	cmp	di,0x10		// normalize es:di
asm  	jb	dinorm
asm	mov	dx,es
asm	inc	dx
asm	mov	es,dx
asm	xor	di,di
dinorm:

asm	sub	[WORD PTR ss:length],1
asm	jnc	expand
asm  	dec	[WORD PTR ss:length+2]
asm	jns	expand		// when length = ffff ffff, done

	}

asm	mov	ax,ss
asm	mov	ds,ax

}


/*
======================
=
= CAL_CarmackExpand
=
= Length is the length of the EXPANDED data
=
======================
*/

#define NEARTAG	0xa7
#define FARTAG	0xa8

void CAL_CarmackExpand (u16int far *source, u16int far *dest, u16int length)
{
	u16int	ch,chhigh,count,offset;
	u16int	far *copyptr, far *inptr, far *outptr;

	length/=2;

	inptr = source;
	outptr = dest;

	while (length)
	{
		ch = *inptr++;
		chhigh = ch>>8;
		if (chhigh == NEARTAG)
		{
			count = ch&0xff;
			if (!count)
			{				// have to insert a word containing the tag byte
				ch |= *((uchar far *)inptr)++;
				*outptr++ = ch;
				length--;
			}
			else
			{
				offset = *((uchar far *)inptr)++;
				copyptr = outptr - offset;
				length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		}
		else if (chhigh == FARTAG)
		{
			count = ch&0xff;
			if (!count)
			{				// have to insert a word containing the tag byte
				ch |= *((uchar far *)inptr)++;
				*outptr++ = ch;
				length --;
			}
			else
			{
				offset = *inptr++;
				copyptr = dest + offset;
				length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		}
		else
		{
			*outptr++ = ch;
			length --;
		}
	}
}



/*
======================
=
= CA_RLEWcompress
=
======================
*/

s32int CA_RLEWCompress (u16int huge *source, s32int length, u16int huge *dest,
  u16int rlewtag)
{
  s32int complength;
  u16int value,count,i;
  u16int huge *start,huge *end;

  start = dest;

  end = source + (length+1)/2;

//
// compress it
//
  do
  {
	count = 1;
	value = *source++;
	while (*source == value && source<end)
	{
	  count++;
	  source++;
	}
	if (count>3 || value == rlewtag)
	{
    //
    // send a tag / count / value string
    //
      *dest++ = rlewtag;
      *dest++ = count;
      *dest++ = value;
    }
    else
    {
    //
    // send word without compressing
    //
      for (i=1;i<=count;i++)
	*dest++ = value;
	}

  } while (source<end);

  complength = 2*(dest-start);
  return complength;
}


/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/

void CA_RLEWexpand (u16int huge *source, u16int huge *dest,s32int length,
  u16int rlewtag)
{
//  u16int value,count,i;
  u16int huge *end;
  u16int sourceseg,sourceoff,destseg,destoff,endseg,endoff;


//
// expand it
//
#if 0
  do
  {
	value = *source++;
	if (value != rlewtag)
	//
	// uncompressed
	//
	  *dest++=value;
	else
	{
	//
	// compressed string
	//
	  count = *source++;
	  value = *source++;
	  for (i=1;i<=count;i++)
	*dest++ = value;
	}
  } while (dest<end);
#endif

  end = dest + (length)/2;
  sourceseg = FP_SEG(source);
  sourceoff = FP_OFF(source);
  destseg = FP_SEG(dest);
  destoff = FP_OFF(dest);
  endseg = FP_SEG(end);
  endoff = FP_OFF(end);


//
// ax = source value
// bx = tag value
// cx = repeat counts
// dx = scratch
//
// NOTE: A repeat count that produces 0xfff0 bytes can blow this!
//

asm	mov	bx,rlewtag
asm	mov	si,sourceoff
asm	mov	di,destoff
asm	mov	es,destseg
asm	mov	ds,sourceseg

expand:
asm	lodsw
asm	cmp	ax,bx
asm	je	repeat
asm	stosw
asm	jmp	next

repeat:
asm	lodsw
asm	mov	cx,ax		// repeat count
asm	lodsw			// repeat value
asm	rep stosw

next:

asm	cmp	si,0x10		// normalize ds:si
asm  	jb	sinorm
asm	mov	ax,si
asm	shr	ax,1
asm	shr	ax,1
asm	shr	ax,1
asm	shr	ax,1
asm	mov	dx,ds
asm	add	dx,ax
asm	mov	ds,dx
asm	and	si,0xf
sinorm:
asm	cmp	di,0x10		// normalize es:di
asm  	jb	dinorm
asm	mov	ax,di
asm	shr	ax,1
asm	shr	ax,1
asm	shr	ax,1
asm	shr	ax,1
asm	mov	dx,es
asm	add	dx,ax
asm	mov	es,dx
asm	and	di,0xf
dinorm:

asm	cmp     di,ss:endoff
asm	jne	expand
asm	mov	ax,es
asm	cmp	ax,ss:endseg
asm	jb	expand

asm	mov	ax,ss
asm	mov	ds,ax

}



/*
=============================================================================

					 CACHE MANAGER ROUTINES

=============================================================================
*/


/*
======================
=
= CAL_SetupGrFile
=
======================
*/

void CAL_SetupGrFile (void)
{
	char fname[13];
	s16int handle;
	uchar *compseg;

#ifdef GRHEADERLINKED

	grhuffman = (huffnode *)&EGAdict;
	grstarts = (s32int _seg *)FP_SEG(&EGAhead);

	CAL_OptimizeNodes (grhuffman);

#else

//
// load ???dict.ext (huffman dictionary for graphics files)
//

	strcpy(fname,gdictname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	read(handle, &grhuffman, sizeof(grhuffman));
	close(handle);
	CAL_OptimizeNodes (grhuffman);
//
// load the data offsets from ???head.ext
//
	MM_GetPtr (&(uchar *)grstarts,(NUMCHUNKS+1)*FILEPOSSIZE);

	strcpy(fname,gheadname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	CA_FarRead(handle, (uchar *)grstarts, (NUMCHUNKS+1)*FILEPOSSIZE);

	close(handle);


#endif

//
// Open the graphics file, leaving it open until the game is finished
//
	strcpy(fname,gfilename);
	strcat(fname,extension);

	grhandle = open(fname, O_RDONLY | O_BINARY);
	if (grhandle == -1)
		CA_CannotOpen(fname);


//
// load the pic and sprite headers into the arrays in the data segment
//
	MM_GetPtr(&(uchar *)pictable,NUMPICS*sizeof(pictabletype));
	CAL_GetGrChunkLength(STRUCTPIC);		// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (u8int huge *)pictable,NUMPICS*sizeof(pictabletype),grhuffman,false);
	MM_FreePtr(&compseg);
}

//==========================================================================


/*
======================
=
= CAL_SetupMapFile
=
======================
*/

void CAL_SetupMapFile (void)
{
	s16int	i;
	s16int handle;
	s32int length,pos;
	char fname[13];

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef MAPHEADERLINKED
	strcpy(fname,mheadname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	length = filelength(handle);
	MM_GetPtr (&(uchar *)tinf,length);
	CA_FarRead(handle, tinf, length);
	close(handle);
#else

	tinf = (u8int _seg *)FP_SEG(&maphead);

#endif

//
// open the data file
//
#ifdef CARMACIZED
	strcpy(fname,"GAMEMAPS.");
	strcat(fname,extension);

	if ((maphandle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
#else
	strcpy(fname,mfilename);
	strcat(fname,extension);

	if ((maphandle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
#endif

//
// load all map header
//
	for (i=0;i<NUMMAPS;i++)
	{
		pos = ((mapfiletype	_seg *)tinf)->headeroffsets[i];
		if (pos<0)						// $FFFFFFFF start is a sparse map
			continue;

		MM_GetPtr(&(uchar *)mapheaderseg[i],sizeof(maptype));
		MM_SetLock(&(uchar *)mapheaderseg[i],true);
		lseek(maphandle,pos,SEEK_SET);
		CA_FarRead (maphandle,(uchar *)mapheaderseg[i],sizeof(maptype));
	}

//
// allocate space for 3 64*64 planes
//
	for (i=0;i<MAPPLANES;i++)
	{
		MM_GetPtr (&(uchar *)mapsegs[i],64*64*2);
		MM_SetLock (&(uchar *)mapsegs[i],true);
	}
}


//==========================================================================


/*
======================
=
= CAL_SetupAudioFile
=
======================
*/

void CAL_SetupAudioFile (void)
{
	s16int handle;
	s32int length;
	char fname[13];

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef AUDIOHEADERLINKED
	strcpy(fname,aheadname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	length = filelength(handle);
	MM_GetPtr (&(uchar *)audiostarts,length);
	CA_FarRead(handle, (u8int far *)audiostarts, length);
	close(handle);
#else
	audiohuffman = (huffnode *)&audiodict;
	CAL_OptimizeNodes (audiohuffman);
	audiostarts = (s32int _seg *)FP_SEG(&audiohead);
#endif

//
// open the data file
//
#ifndef AUDIOHEADERLINKED
	strcpy(fname,afilename);
	strcat(fname,extension);

	if ((audiohandle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
#else
	if ((audiohandle = open("AUDIO."EXTENSION,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		Quit ("Can't open AUDIO."EXTENSION"!");
#endif
}

//==========================================================================


/*
======================
=
= CA_Startup
=
= Open all files and load in headers
=
======================
*/

void CA_Startup (void)
{
#ifdef PROFILE
	unlink ("PROFILE.TXT");
	profilehandle = open("PROFILE.TXT", O_CREAT | O_WRONLY | O_TEXT);
#endif

	CAL_SetupMapFile ();
	CAL_SetupGrFile ();
	CAL_SetupAudioFile ();

	mapon = -1;
	ca_levelbit = 1;
	ca_levelnum = 0;

}

//==========================================================================


/*
======================
=
= CA_Shutdown
=
= Closes all files
=
======================
*/

void CA_Shutdown (void)
{
#ifdef PROFILE
	close (profilehandle);
#endif

	close (maphandle);
	close (grhandle);
	close (audiohandle);
}

//===========================================================================

/*
======================
=
= CA_CacheAudioChunk
=
======================
*/

void CA_CacheAudioChunk (s16int chunk)
{
	s32int	pos,compressed;
#ifdef AUDIOHEADERLINKED
	s32int	expanded;
	uchar *bigbufferseg;
	u8int	far *source;
#endif

	if (audiosegs[chunk])
	{
		MM_SetPurge (&(uchar *)audiosegs[chunk],0);
		return;							// allready in memory
	}

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = audiostarts[chunk];
	compressed = audiostarts[chunk+1]-pos;

	lseek(audiohandle,pos,SEEK_SET);

#ifndef AUDIOHEADERLINKED

	MM_GetPtr (&(uchar *)audiosegs[chunk],compressed);
	if (mmerror)
		return;

	CA_FarRead(audiohandle,audiosegs[chunk],compressed);

#else

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(audiohandle,bufferseg,compressed);
		source = bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		if (mmerror)
			return;
		MM_SetLock (&bigbufferseg,true);
		CA_FarRead(audiohandle,bigbufferseg,compressed);
		source = bigbufferseg;
	}

	expanded = *(s32int far *)source;
	source += 4;			// skip over length
	MM_GetPtr (&(uchar *)audiosegs[chunk],expanded);
	if (mmerror)
		goto done;
	CAL_HuffExpand (source,audiosegs[chunk],expanded,audiohuffman,false);

done:
	if (compressed>BUFFERSIZE)
		MM_FreePtr(&bigbufferseg);
#endif
}

//===========================================================================

/*
======================
=
= CA_LoadAllSounds
=
= Purges all sounds, then loads all new ones (mode switch)
=
======================
*/

void CA_LoadAllSounds (void)
{
	u16int	start,i;

	switch (oldsoundmode)
	{
	case sdm_Off:
		goto cachein;
	case sdm_PC:
		start = STARTPCSOUNDS;
		break;
	case sdm_AdLib:
		start = STARTADLIBSOUNDS;
		break;
	}

	for (i=0;i<NUMSOUNDS;i++,start++)
		if (audiosegs[start])
			MM_SetPurge (&(uchar *)audiosegs[start],3);		// make purgable

cachein:

	switch (SoundMode)
	{
	case sdm_Off:
		return;
	case sdm_PC:
		start = STARTPCSOUNDS;
		break;
	case sdm_AdLib:
		start = STARTADLIBSOUNDS;
		break;
	}

	for (i=0;i<NUMSOUNDS;i++,start++)
		CA_CacheAudioChunk (start);

	oldsoundmode = SoundMode;
}

//===========================================================================


/*
======================
=
= CAL_ExpandGrChunk
=
= Does whatever is needed with a pointer to a compressed chunk
=
======================
*/

void CAL_ExpandGrChunk (s16int chunk, u8int far *source)
{
	s32int	expanded;


	if (chunk >= STARTTILE8 && chunk < STARTEXTERNS)
	{
	//
	// expanded sizes of tile8/16/32 are implicit
	//

#define BLOCK		64
#define MASKBLOCK	128

		if (chunk<STARTTILE8M)			// tile 8s are all in one chunk!
			expanded = BLOCK*NUMTILE8;
		else if (chunk<STARTTILE16)
			expanded = MASKBLOCK*NUMTILE8M;
		else if (chunk<STARTTILE16M)	// all other tiles are one/chunk
			expanded = BLOCK*4;
		else if (chunk<STARTTILE32)
			expanded = MASKBLOCK*4;
		else if (chunk<STARTTILE32M)
			expanded = BLOCK*16;
		else
			expanded = MASKBLOCK*16;
	}
	else
	{
	//
	// everything else has an explicit size u32int
	//
		expanded = *(s32int far *)source;
		source += 4;			// skip over length
	}

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
	MM_GetPtr (&grsegs[chunk],expanded);
	if (mmerror)
		return;
	CAL_HuffExpand (source,grsegs[chunk],expanded,grhuffman,false);
}


/*
======================
=
= CA_CacheGrChunk
=
= Makes sure a given chunk is in memory, loadiing it if needed
=
======================
*/

void CA_CacheGrChunk (s16int chunk)
{
	s32int	pos,compressed;
	uchar *bigbufferseg;
	u8int	far *source;
	s16int		next;

	grneeded[chunk] |= ca_levelbit;		// make sure it doesn't get removed
	if (grsegs[chunk])
	{
		MM_SetPurge (&grsegs[chunk],0);
		return;							// allready in memory
	}

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = GRFILEPOS(chunk);
	if (pos<0)							// $FFFFFFFF start is a sparse tile
	  return;

	next = chunk +1;
	while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
		next++;

	compressed = GRFILEPOS(next)-pos;

	lseek(grhandle,pos,SEEK_SET);

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(grhandle,bufferseg,compressed);
		source = bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		MM_SetLock (&bigbufferseg,true);
		CA_FarRead(grhandle,bigbufferseg,compressed);
		source = bigbufferseg;
	}

	CAL_ExpandGrChunk (chunk,source);

	if (compressed>BUFFERSIZE)
		MM_FreePtr(&bigbufferseg);
}



//==========================================================================

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

//==========================================================================

/*
======================
=
= CA_CacheMap
=
= WOLF: This is specialized for a 64*64 map size
=
======================
*/

void CA_CacheMap (s16int mapnum)
{
	s32int	pos,compressed;
	s16int		plane;
	uchar **dest,bigbufferseg;
	u16int	size;
	u16int	far	*source;
#ifdef CARMACIZED
	uchar *buffer2seg;
	s32int	expanded;
#endif

	mapon = mapnum;

//
// load the planes into the allready allocated buffers
//
	size = 64*64*2;

	for (plane = 0; plane<MAPPLANES; plane++)
	{
		pos = mapheaderseg[mapnum]->planestart[plane];
		compressed = mapheaderseg[mapnum]->planelength[plane];

		dest = &(uchar *)mapsegs[plane];

		lseek(maphandle,pos,SEEK_SET);
		if (compressed<=BUFFERSIZE)
			source = bufferseg;
		else
		{
			MM_GetPtr(&bigbufferseg,compressed);
			MM_SetLock (&bigbufferseg,true);
			source = bigbufferseg;
		}

		CA_FarRead(maphandle,(u8int far *)source,compressed);
#ifdef CARMACIZED
		//
		// unhuffman, then unRLEW
		// The huffman'd chunk has a two byte expanded length first
		// The resulting RLEW chunk also does, even though it's not really
		// needed
		//
		expanded = *source;
		source++;
		MM_GetPtr (&buffer2seg,expanded);
		CAL_CarmackExpand (source, (u16int far *)buffer2seg,expanded);
		CA_RLEWexpand (((u16int far *)buffer2seg)+1,*dest,size,
		((mapfiletype _seg *)tinf)->RLEWtag);
		MM_FreePtr (&buffer2seg);

#else
		//
		// unRLEW, skipping expanded length
		//
		CA_RLEWexpand (source+1, *dest,size,
		((mapfiletype _seg *)tinf)->RLEWtag);
#endif

		if (compressed>BUFFERSIZE)
			MM_FreePtr(&bigbufferseg);
	}
}

//===========================================================================

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

//===========================================================================

/*
======================
=
= CA_ClearMarks
=
= Clears out all the marks at the current level
=
======================
*/

void CA_ClearMarks (void)
{
	s16int i;

	for (i=0;i<NUMCHUNKS;i++)
		grneeded[i]&=~ca_levelbit;
}


//===========================================================================

/*
======================
=
= CA_ClearAllMarks
=
= Clears out all the marks on all the levels
=
======================
*/

void CA_ClearAllMarks (void)
{
	_fmemset (grneeded,0,sizeof(grneeded));
	ca_levelbit = 1;
	ca_levelnum = 0;
}


//===========================================================================


/*
======================
=
= CA_FreeGraphics
=
======================
*/


void CA_SetGrPurge (void)
{
	s16int i;

//
// free graphics
//
	CA_ClearMarks ();

	for (i=0;i<NUMCHUNKS;i++)
		if (grsegs[i])
			MM_SetPurge (&(uchar *)grsegs[i],3);
}



/*
======================
=
= CA_SetAllPurge
=
= Make everything possible purgable
=
======================
*/

void CA_SetAllPurge (void)
{
	s16int i;


//
// free sounds
//
	for (i=0;i<NUMSNDCHUNKS;i++)
		if (audiosegs[i])
			MM_SetPurge (&(uchar *)audiosegs[i],3);

//
// free graphics
//
	CA_SetGrPurge ();
}


//===========================================================================

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

void CA_CannotOpen(char *string)
{
 char str[30];

 strcpy(str,"Can't open ");
 strcat(str,string);
 strcat(str,"!\n");
 Quit (str);
}