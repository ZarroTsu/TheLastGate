// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  filemanager.c
//  File manager - handles saving and loading of options and characters
//
// **************************************************************** 
#include "common.h"
#include "filemanager.h"

// F_Load_Options - loads local options data from the data file
void FIL_Load_Options(void)
{
	int file_dat, n, file_error = 0;
	
	// Opens the data file in read mode
	if ((file_dat = open(F_DATA, O_RDONLY|O_BINARY)) != -1) 
	{
		// Cycles through each portion of the data file and loads these values.
		// If the pointer sizes don't match, it triggers a file_error.
		if (read(file_dat, &options, sizeof(options)) != sizeof(options)) file_error = 1;
		
		/*
		if (read(file_dat,history,sizeof(history))!=sizeof(history)) flag=1;
		if (read(file_dat,hist_len,sizeof(hist_len))!=sizeof(hist_len)) flag=1;
		if (read(file_dat,words,sizeof(words))!=sizeof(words)) flag=1;
		if (read(file_dat,&pdata,sizeof(pdata))!=sizeof(pdata)) flag=1;
		if (read(file_dat,&okey,sizeof(okey))!=sizeof(okey)) flag=1;
		*/
		close(file_dat);
	} 
	else 
	{
		file_error = 1;
	}
	
	// A file error has occured for some reason when accessing the data file.
	// This error is handled by initializing all local client variables.
	if (file_error)
	{
		options.fullscreen = DEF_FULLSCREEN;
		options.magnify = DEF_MAGNIFY;
		options.music = DEF_MUSIC;
		options.sound = DEF_SOUND;
		options.alpha = DEF_ALPHA;
		options.shadows = DEF_SHADOWS;
		options.darkmode = DEF_DARKMODE;
		
		/*
		memset(history,0,sizeof(history));
		memset(hist_len,0,sizeof(hist_len));
		memset(words,0,sizeof(words));
		memset(&pdata,0,sizeof(pdata));
		
		pdata.show_names=1;
		pdata.hide=1;
		pdata.show_stats=0;
		pdata.show_bars=0;
		
		for (n=0; n<16; n++) 
		{
			pdata.xbutton[n].skill_nr=-1;
			strcpy(pdata.xbutton[n].name,"-");
		}
		
		memset(&okey,0,sizeof(okey));
		strcpy(okey.name, "New Account");
		*/
	}
}

// F_Save_Options - saves local options data to the data file
void FIL_Save_Options(void)
{
	int file_dat;
	
	// Opens the data file in write mode, and writes each local client struct to file
	if ((file_dat = open(F_DATA, O_WRONLY|O_BINARY|O_CREAT|O_TRUNC, F_RWX)) != -1) 
	{
		write(file_dat, &options, sizeof(options));
		/*
		write(file_dat,history,sizeof(history));
		write(file_dat,hist_len,sizeof(hist_len));
		write(file_dat,words,sizeof(words));
		write(file_dat,&pdata,sizeof(pdata));
		write(file_dat,&okey,sizeof(okey));
		*/
		close(file_dat);
	}
}

void FIL_Load_Character(char *file_name)
{
	int file_dat, n, file_error = 0;
	char buf[256];
	
	// >> Confirmation Dialog? <<
	
	// Opens the data file in read mode
	// If it cannot open the file this will throw an error and exit
	if ((file_dat = open(file_name, O_RDONLY|O_BINARY)) == -1) 
	{
		sprintf(buf, "Could not open file \"%s\".", file_name);
		// >> Error Dialog? <<
		return;
	}
	
	/*
	if (lseek(file_dat,0,SEEK_END)>(long) (sizeof(struct pdata)+sizeof(struct key))) flag=1;
	lseek(file_dat,0,SEEK_SET);

	read(file_dat,&okey,sizeof(struct key));
	if (read(file_dat,&pdata,sizeof(struct pdata))!=sizeof(struct pdata) || flag) {
		pdata.hide=1;
		pdata.show_names=1;
		pdata.show_proz=1;
		pdata.show_stats=0;
		pdata.show_bars=0;
		pdata.cname[0]=0;
		pdata.ref[0]=0;
		pdata.desc[0]=0;
		pdata.changed=0;

		for (n=0; n<16; n++) {
			pdata.xbutton[n].skill_nr=-1;
			strcpy(pdata.xbutton[n].name,"-");
		}
	}

	close(file_dat);

	pdata.changed=1;
	*/
}

void FIL_Save_Character(char *file_name)
{
	int file_dat;
	char buf[256];
	
	// Opens the data file in write mode
	// If it cannot open the file this will throw an error and exit
	if ((file_dat = open(file_name, O_WRONLY|O_BINARY|O_CREAT|O_TRUNC, F_RWX)) == -1) 
	{
		sprintf(buf,"Could not open file \"%s\".", file_name);
		// >> Error Dialog? <<
		return;
	}
	
	//write(file_dat, &okey, sizeof(struct Key));
	//write(file_dat, &pdata, sizeof(struct pdata));

	close(file_dat);

	sprintf(buf, "Saved as \"%s\".", file_name);
	// >> Confirmation Dialog? <<
}

/* END OF FILE */