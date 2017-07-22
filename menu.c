void CP_ReadThis(void)
{
	StartCPMusic(0);
	HelpScreens();
	StartCPMusic(MENUSONG);
}

s16int CP_CheckQuick(u16int scancode)
{
	switch(scancode)
	{
		//
		// END GAME
		//
		case sc_F7:
			CA_CacheGrChunk(STARTFONT+1);

			WindowH=160;
			if (Confirm(ENDGAMESTR))
			{
				gm.φ = ex_died;
				pickquick = gamestate.lives = 0;
			}

			DrawAllPlayBorder();
			WindowH=200;
			fontnumber=0;
			MainMenu[savegame].active = 0;
			return 1;

		//
		// QUICKSAVE
		//
		case sc_F8:
			if (SaveGamesAvail[LSItems.curpos] && pickquick)
			{
				CA_CacheGrChunk(STARTFONT+1);
				fontnumber = 1;
				Message("Saving...");
				CP_SaveGame(1);
				fontnumber=0;
			}
			else
			{
				VW_FadeOut ();

				StartCPMusic(MENUSONG);
				pickquick=CP_SaveGame(0);

				SETFONTCOLOR(0,15);
				IN_ClearKeysDown();
				view ();

				if (!startgame && !gm.load)
				{
					VW_FadeIn ();
					mapmus ();
				}

				if (gm.load)
					gm.φ = ex_abort;
				lasttimecount = TimeCount;

				if (MousePresent)
					Mouse(MDelta);	// Clear accumulated mouse movement

				PM_CheckMainMem ();
			}
			return 1;

		//
		// QUICKLOAD
		//
		case sc_F9:
			if (SaveGamesAvail[LSItems.curpos] && pickquick)
			{
				char string[100]="Load Game called\n\"";


				CA_CacheGrChunk(STARTFONT+1);
				fontnumber = 1;

				strcat(string,SaveGameNames[LSItems.curpos]);
				strcat(string,"\"?");

				if (Confirm(string))
					CP_LoadGame(1);

				DrawAllPlayBorder();
				fontnumber=0;
			}
			else
			{
				VW_FadeOut ();

				StartCPMusic(MENUSONG);
				pickquick=CP_LoadGame(0);

				SETFONTCOLOR(0,15);
				IN_ClearKeysDown();
				view ();

				if (!startgame && !gm.load)
				{
					VW_FadeIn ();
					mapmus ();
				}

				if (gm.load)
					gm.φ = ex_abort;

				lasttimecount = TimeCount;

				if (MousePresent)
					Mouse(MDelta);	// Clear accumulated mouse movement
				PM_CheckMainMem ();
			}
			return 1;
		}

	return 0;
}
