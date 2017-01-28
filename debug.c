#define VIEWTILEX	(vw.dx/16)
#define VIEWTILEY	(vw.dy/16)

s16int	maporgx;
s16int	maporgy;
enum {mapview,tilemapview,actoratview,visview}	viewtype;

s16int DebugKeys (void)
{
	int esc;
	s16int level,i;

	if (Keyboard[sc_O])
	{
		ViewMap();
		return 1;
	}
	return 0;
}

void OverheadRefresh (void)
{
	u16int	x,y,endx,endy,sx,sy;
	u16int	tile;

	endx = maporgx+VIEWTILEX;
	endy = maporgy+VIEWTILEY;

	for (y=maporgy;y<endy;y++)
		for (x=maporgx;x<endx;x++)
		{
			sx = (x-maporgx)*16;
			sy = (y-maporgy)*16;

			switch (viewtype)
			{
#if 0
			case mapview:
				tile = *(mapsegs[0]+farmapylookup[y]+x);
				break;

			case tilemapview:
				tile = tilemap[x][y];
				break;

			case visview:
				tile = spotvis[x][y];
				break;
#endif
			case actoratview:
				tile = (u16int)actorat[x][y];
				break;
			}

			if (tile<MAXWALLTILES)
				LatchDrawTile(sx,sy,tile);
			else
			{
				LatchDrawChar(sx,sy,NUMBERCHARS+((tile&0xf000)>>12));
				LatchDrawChar(sx+8,sy,NUMBERCHARS+((tile&0x0f00)>>8));
				LatchDrawChar(sx,sy+8,NUMBERCHARS+((tile&0x00f0)>>4));
				LatchDrawChar(sx+8,sy+8,NUMBERCHARS+(tile&0x000f));
			}
		}

}

void ViewMap (void)
{
	int		button0held;

	viewtype = actoratview;
//	button0held = false;


	maporgx = player->tilex - VIEWTILEX/2;
	if (maporgx<0)
		maporgx = 0;
	if (maporgx>MAPSIZE-VIEWTILEX)
		maporgx=MAPSIZE-VIEWTILEX;
	maporgy = player->tiley - VIEWTILEY/2;
	if (maporgy<0)
		maporgy = 0;
	if (maporgy>MAPSIZE-VIEWTILEY)
		maporgy=MAPSIZE-VIEWTILEY;

	do
	{
//
// let user pan around
//
		PollControls ();
		if (controlx < 0 && maporgx>0)
			maporgx--;
		if (controlx > 0 && maporgx<mapwidth-VIEWTILEX)
			maporgx++;
		if (controly < 0 && maporgy>0)
			maporgy--;
		if (controly > 0 && maporgy<mapheight-VIEWTILEY)
			maporgy++;
#if 0
		if (c.button0 && !button0held)
		{
			button0held = true;
			viewtype++;
			if (viewtype>visview)
				viewtype = mapview;
		}
		if (!c.button0)
			button0held = false;
#endif

		OverheadRefresh ();

	} while (!Keyboard[sc_Escape]);

	IN_ClearKeysDown ();
}
