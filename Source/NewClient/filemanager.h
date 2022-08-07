// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//
//  filemanager.h
//  Header information specific to filemanager.c
//
// **************************************************************** 
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

// Data file name - Stores saved client settings
#define F_DATA				"TLG.dat"

// Read-write-execute file flags - used by open() when writing to file
#define F_RWX				0666

extern struct Opts options;

static char *f_new_msg1 = {
	"Do you really want to create a new account?\n\n"
	"Your old account will no longer be accessible, unless you remembered to save it. You did save it, didn't you?\n"};

static char *f_new_msg2 = {
	"Have you read and understood the previous message?"};

static char *f_load_msg = {
	"Do you really want to load an account?\n\n"
	"Your old account will no longer be accessible if you didn't save it yet. You did save it, didn't you?\n"
};

#endif // FILEMANAGER_H
/* END OF FILE */