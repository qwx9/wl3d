void Victory (void)
{
	won();
	IN_Ack();
	VW_FadeOut ();
#ifndef SPEAR
	EndText();
#else
	EndSpear();
#endif
}

void	CheckHighScore (s32int score,u16int other)
{
	u16int		i,j;
	s16int			n;
	HighScore	myscore;

	strcpy(myscore.name,"");
	myscore.score = score;
	myscore.episode = gamestate.episode;
	myscore.completed = other;

	for (i = 0,n = -1;i < MaxScores;i++)
	{
		if
		(
			(myscore.score > Scores[i].score)
		||	(
				(myscore.score == Scores[i].score)
			&& 	(myscore.completed > Scores[i].completed)
			)
		)
		{
			for (j = MaxScores;--j > i;)
				Scores[j] = Scores[j - 1];
			Scores[i] = myscore;
			n = i;
			break;
		}
	}

#ifdef SPEAR
	StartCPMusic (20);
#else
	StartCPMusic (23);
#endif
	DrawHighScores ();

	VW_FadeIn ();

	if (n != -1)
	{
	//
	// got a high score
	//
		PrintY = 76 + (16 * n);
#ifndef SPEAR
		PrintX = 4*8;
		backcolor = BORDCOLOR;
		fontcolor = 15;
		US_LineInput(PrintX,PrintY,Scores[n].name,nil,true,MaxHighName,100);
#else
		PrintX = 16;
		fontnumber = 1;
		VWB_Bar (PrintX-2,PrintY-2,145,15,0x9c);
		VW_UpdateScreen ();
		backcolor = 0x9c;
		fontcolor = 15;
		US_LineInput(PrintX,PrintY,Scores[n].name,nil,true,MaxHighName,130);
#endif
	}
	else
	{
		IN_ClearKeysDown ();
		IN_UserInput(500);
	}
}
