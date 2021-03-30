
#include <stdio.h>
#include <alloc.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include "ddraw.h"
#include <process.h>
#include <time.h>
#include <dir.h>
#pragma hdrstop
#include "dd.h"
#include "common.h"
#include "inter.h"
#include "merc.rh"

/*
 * Borland and Microsoft disagree on the size of the OPENFILENAME structure.
 * The following header is excerpted from "win.h" as included with
 * Jacob Navia's * lcc32 compiler.
 */

typedef struct tagOFN {
	DWORD lStructSize;
	HWND hwndOwner;
	HINSTANCE hInstance;
	LPCTSTR lpstrFilter;
	LPTSTR lpstrCustomFilter;
	DWORD nMaxCustFilter;
	DWORD nFilterIndex;
	LPTSTR lpstrFile;
	DWORD nMaxFile;
	LPTSTR lpstrFileTitle;
	DWORD nMaxFileTitle;
	LPCTSTR lpstrInitialDir;
	LPCTSTR lpstrTitle;
	DWORD Flags;
	WORD nFileOffset;
	WORD nFileExtension;
	LPCTSTR lpstrDefExt;
	DWORD lCustData;
	LPOFNHOOKPROC lpfnHook;
	LPCTSTR lpTemplateName;
} LCC_OPENFILENAME,*LCC_LPOPENFILENAME;

extern char history[20][128];
extern int hist_len[20];
extern char words[2048][40];
extern char passwd[15];

extern void pascal (*ctl3don)(HANDLE,short int);
extern HBRUSH dlg_back;
extern int dlg_col,dlg_fcol;

extern int quit;

extern int do_alpha;
extern int do_shadow;

extern int screen_width, screen_height, screen_tilexoff, screen_tileyoff, screen_viewsize, view_subedges;
extern int screen_overlay_sprite;
extern int xwalk_nx, xwalk_ny, xwalk_ex, xwalk_ey, xwalk_sx, xwalk_sy, xwalk_wx, xwalk_wy;
extern short screen_windowed;
extern short screen_renderdist;

extern HINSTANCE hinst;
extern HWND desk_hwnd;
extern int so_status;

struct key okey;
struct pdata pdata={"","","",0};

//--------------
// option flags
//--------------

extern int domusic,dosound,smode;
extern char host_addr[];
static int opmusic,opsound,opshadow;
int race=0,sex=0;

static char *new_msg1={
	"Do you really want to create a new account?\n\n"
	"Your old account will no longer be accessible, unless you remembered to save it. You did save it, didn't you?\n"};

static char *new_msg2={
	"Have you read and understood the previous message?"};

static char *load_msg={
	"Do you really want to load an account?\n\n"
	"Your old account will no longer be accessible if you didn't save it yet. You did save it, didn't you?\n"
};

int dd_change(int x,int y);

void translate_okey2race(int *race_ptr,int *sex_ptr)
{
	switch(okey.race) 
	{
		case  4: race= 1; sex=1; break;	// templar M
		case  5: race= 1; sex=2; break;	// templar F
		
		case  6: race= 2; sex=1; break;	// mercenary M
		case  7: race= 2; sex=2; break;	// mercenary F
		
		case  8: race= 3; sex=1; break;	// harakim M
		case  9: race= 3; sex=2; break;	// harakim F
		
		//
		
		case 10: race= 7; sex=1; break;	// seyan M
		case 11: race= 7; sex=2; break;	// seyan F
		
		//
		
		case 12: race= 4; sex=1; break;	// arch templar M
		case 13: race= 4; sex=2; break;	// arch templar F
		
		case 14: race= 5; sex=1; break;	// pugilist M
		case 15: race= 5; sex=2; break;	// pugilist F
		
		case 16: race= 6; sex=1; break;	// warrior M
		case 17: race= 6; sex=2; break;	// warrior F
		
		case 18: race= 8; sex=1; break;	// sorcerer M
		case 19: race= 8; sex=2; break;	// sorcerer F
		
		case 20: race= 9; sex=1; break;	// summoner M
		case 21: race= 9; sex=2; break;	// summoner F
		
		case 22: race=10; sex=1; break;	// arch harakim M
		case 23: race=10; sex=2; break;	// arch harakim F
		
		//
		
		case  2: race=11; sex=1; break;	// god M
		case  3: race=11; sex=2; break;	// god F
		
		default: race=0; sex=0; break;
	}

	*race_ptr=race;
	*sex_ptr=sex;
}

void setres_800()
{
	screen_width=800;
	screen_height=600;
	screen_tilexoff=XPOS_800;
	screen_tileyoff=YPOS_800;
	screen_viewsize=VIEWSIZE_800;
	view_subedges=VIEW_SUBEDGES_800;
	screen_overlay_sprite=GUI_OVERLAY_800;

	xwalk_nx=XWALK_NX_800;
	xwalk_ny=XWALK_NY_800;
	xwalk_ex=XWALK_EX_800;
	xwalk_ey=XWALK_EY_800;
	xwalk_sx=XWALK_SX_800;
	xwalk_sy=XWALK_SY_800;
	xwalk_wx=XWALK_WX_800;
	xwalk_wy=XWALK_WY_800;
}

void setres_1280()
{
	screen_width=1280;
	screen_height=720;
	screen_tilexoff=XPOS_1280;
	screen_tileyoff=YPOS_1280;
	screen_viewsize=VIEWSIZE_1280;
	view_subedges=VIEW_SUBEDGES_1280;
	screen_overlay_sprite=GUI_OVERLAY_1280;

	xwalk_nx=XWALK_NX_1280;
	xwalk_ny=XWALK_NY_1280;
	xwalk_ex=XWALK_EX_1280;
	xwalk_ey=XWALK_EY_1280;
	xwalk_sx=XWALK_SX_1280;
	xwalk_sy=XWALK_SY_1280;
	xwalk_wx=XWALK_WX_1280;
	xwalk_wy=XWALK_WY_1280;
	
	if (screen_windowed == 0)
	{
		screen_tilexoff+=2;
		screen_tileyoff-=5;
	}
}

void setres_1600()
{
	screen_width=1600;
	screen_height=900;
	screen_tilexoff=XPOS_1600;
	screen_tileyoff=YPOS_1600;
	screen_viewsize=VIEWSIZE_1600;
	view_subedges=VIEW_SUBEDGES_1600;
	screen_overlay_sprite=GUI_OVERLAY_1600;

	xwalk_nx=XWALK_NX_1600;
	xwalk_ny=XWALK_NY_1600;
	xwalk_ex=XWALK_EX_1600;
	xwalk_ey=XWALK_EY_1600;
	xwalk_sx=XWALK_SX_1600;
	xwalk_sy=XWALK_SY_1600;
	xwalk_wx=XWALK_WX_1600;
	xwalk_wy=XWALK_WY_1600;
}

void setres_default()
{
	//setres_800();
	setres_1280();
}

void load_options(void)
{
	int n,handle,flag=0;

	handle=open("TLG.dat",O_RDONLY|O_BINARY);
	if (handle!=-1) {
		if (read(handle,history,sizeof(history))!=sizeof(history)) flag=1;
		if (read(handle,hist_len,sizeof(hist_len))!=sizeof(hist_len)) flag=1;
		if (read(handle,words,sizeof(words))!=sizeof(words)) flag=1;
		if (read(handle,&domusic,sizeof(domusic))!=sizeof(domusic)) flag=1;
		if (read(handle,&dosound,sizeof(dosound))!=sizeof(dosound)) flag=1;
		if (read(handle,&pdata,sizeof(pdata))!=sizeof(pdata)) flag=1;
		if (read(handle,&okey,sizeof(okey))!=sizeof(okey)) flag=1;
		if (read(handle,&do_alpha,sizeof(do_alpha))!=sizeof(do_alpha)) do_alpha=2;
		if (read(handle,&do_shadow,sizeof(do_shadow))!=sizeof(do_shadow)) do_shadow=1;
		if (read(handle,&screen_windowed,sizeof(screen_windowed))!=sizeof(screen_windowed)) screen_windowed=1;
		close(handle);
	} else flag=1;

	if (flag) {
		/* flag=1; */
		memset(history,0,sizeof(history));
		memset(hist_len,0,sizeof(hist_len));
		memset(words,0,sizeof(words));
		domusic=0; dosound=1; do_alpha=2; do_shadow=1; screen_windowed=1;
		memset(&pdata,0,sizeof(pdata));
		pdata.show_names=1;
		pdata.hide=1;
		pdata.show_stats=0;
		pdata.show_bars=0;
		for (n=0; n<16; n++) {
			pdata.xbutton[n].skill_nr=-1;
			strcpy(pdata.xbutton[n].name,"-");
		}
		memset(&okey,0,sizeof(okey));
		strcpy(okey.name,"New Account");
	}
}

void save_options(void)
{
	int handle;

	handle=open("TLG.dat",O_WRONLY|O_BINARY|O_CREAT|O_TRUNC,0666);
	if (handle!=-1) {
		write(handle,history,sizeof(history));
		write(handle,hist_len,sizeof(hist_len));
		write(handle,words,sizeof(words));
		write(handle,&domusic,sizeof(domusic));
		write(handle,&dosound,sizeof(dosound));
		write(handle,&pdata,sizeof(pdata));
		write(handle,&okey,sizeof(okey));
		write(handle,&do_alpha,sizeof(do_alpha));
		write(handle,&do_shadow,sizeof(do_shadow));
		write(handle,&screen_windowed,sizeof(screen_windowed));
		close(handle);
	}
}

void load_char(HWND hwnd,char *name)
{
	int handle,n,flag=0;
	char buf[256];

	if (MessageBox(hwnd,load_msg,"Are you sure?",MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL)!=IDYES)
		return;

	handle=open(name,O_RDONLY|O_BINARY);
	if (handle==-1) {
		sprintf(buf,"Could not open file \"%s\".",name);
		MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
		return;
	}

	if (lseek(handle,0,SEEK_END)>(long) (sizeof(struct pdata)+sizeof(struct key))) flag=1;
	lseek(handle,0,SEEK_SET);

	read(handle,&okey,sizeof(struct key));
	if (read(handle,&pdata,sizeof(struct pdata))!=sizeof(struct pdata) || flag) {
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

	close(handle);

	pdata.changed=1;
}

void save_char(HWND hwnd,char *name)
{
	int handle;
	char buf[256];

	handle=open(name,O_WRONLY|O_BINARY|O_CREAT|O_TRUNC,0600);
	if (handle==-1) {
		sprintf(buf,"Could not open file \"%s\".",name);
		MessageBox(hwnd,buf,"Error",MB_OK|MB_ICONSTOP);
		return;
	}

	write(handle,&okey,sizeof(struct key));
	write(handle,&pdata,sizeof(struct pdata));

	close(handle);

	sprintf(buf,"Saved as \"%s\".",name);
	MessageBox(hwnd,buf,"Done",MB_OK);
}

int isNT(void)
{
	OSVERSIONINFO o;
	o.dwOSVersionInfoSize=sizeof(o);

	GetVersionEx(&o);

	if (o.dwPlatformId==VER_PLATFORM_WIN32_NT) return 1;
	else return 0;
}

struct mtp {
	DWORD        lStructSize;
	HWND         hwndOwner;
	HINSTANCE    hInstance;
	LPCSTR       lpstrFilter;
	LPSTR        lpstrCustomFilter;
	DWORD        nMaxCustFilter;
	DWORD        nFilterIndex;
	LPSTR        lpstrFile;
	DWORD        nMaxFile;
	LPSTR        lpstrFileTitle;
	DWORD        nMaxFileTitle;
	LPCSTR       lpstrInitialDir;
	LPCSTR       lpstrTitle;
	DWORD        Flags;
	WORD         nFileOffset;
	WORD         nFileExtension;
	LPCSTR       lpstrDefExt;
	LPARAM       lCustData;
	LPOFNHOOKPROC lpfnHook;
	LPCSTR       lpTemplateName;
};

int load_dialog(HWND hwnd,char *name)
{
	LCC_OPENFILENAME ofn;
        char filter[]={MNAME" Character Save\0*.moa\0\0"};
	char buf[256]={"\0"};
	int disk,err;
	char dir[256],dir2[300];

	disk=getdisk();
	getcurdir(0,dir);

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hwnd;
	ofn.hInstance=hinst;
	ofn.lpstrFilter=filter;
	ofn.lpstrCustomFilter=NULL;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=buf;
	ofn.nMaxFile=256;
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrTitle="Load Character";
	ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	ofn.nFileOffset=0;
	ofn.nFileExtension=0;
	ofn.lpstrDefExt="moa";
	ofn.lCustData=NULL;
	ofn.lpfnHook=NULL;

	if (!GetOpenFileName((OPENFILENAME *) &ofn)) {
		setdisk(disk);
		chdir(dir);
		return 0;
	}

	strcpy(name,buf);

	setdisk(disk);
	sprintf(dir2,"\\%s",dir);
	chdir(dir2);

	return 1;
}

int save_dialog(HWND hwnd,char *name)
{
	LCC_OPENFILENAME ofn;
        char filter[]={MNAME" Character Save\0*.moa\0\0"};
	char buf[256];
	int disk;
	char dir[256],dir2[300];

	disk=getdisk();
	getcurdir(0,dir);

	strcpy(buf,okey.name);
	strcat(buf,".moa");

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hwnd;
	ofn.hInstance=hinst;
	ofn.lpstrFilter=filter;
	ofn.lpstrCustomFilter=NULL;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=buf;
	ofn.nMaxFile=256;
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrTitle="Save Character as";
	ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
	ofn.nFileOffset=0;
	ofn.nFileExtension=0;
	ofn.lpstrDefExt="moa";
	ofn.lCustData=NULL;
	ofn.lpfnHook=NULL;

	if (!GetSaveFileName((OPENFILENAME *) &ofn)) {
		setdisk(disk);
		chdir(dir);
		return 0;
	}

	strcpy(name,buf);

	setdisk(disk);
	sprintf(dir2,"\\%s",dir);
	chdir(dir2);

	return 1;
}

void update_race(HWND hwnd)
{
	//char buf[2];
	
	if (race < 4)
	{
		SetWindowPos(GetDlgItem(hwnd,IDC_ARCH),NULL,0,0,0,0,0);

		if (race==1)  CheckRadioButton(hwnd,IDC_TEMP,IDC_ARCH,IDC_TEMP);
		if (race==2)  CheckRadioButton(hwnd,IDC_TEMP,IDC_ARCH,IDC_MERC);
		if (race==3)  CheckRadioButton(hwnd,IDC_TEMP,IDC_ARCH,IDC_HARA);
		
		//sprintf(buf,"%d",race);
		//SetDlgItemText(hwnd,IDC_ARCH,buf);
		SetDlgItemText(hwnd,IDC_ARCH,"Null");
	}
	else
	{
		//  AUTORADIOBUTTON "", IDC_ARCH, 176, 143, 60, 12, WS_DISABLED
		SetWindowPos(GetDlgItem(hwnd,IDC_ARCH),NULL,176+88,143+92,60,12,0);
		
		CheckRadioButton(hwnd,IDC_TEMP,IDC_ARCH,IDC_ARCH);
	
		if (race==4)  SetDlgItemText(hwnd,IDC_ARCH,"Arch-Templar");
		if (race==5)  SetDlgItemText(hwnd,IDC_ARCH,"Brawler");
		if (race==6)  SetDlgItemText(hwnd,IDC_ARCH,"Warrior");
		if (race==7)  SetDlgItemText(hwnd,IDC_ARCH,"Seyan'du");
		if (race==8)  SetDlgItemText(hwnd,IDC_ARCH,"Sorcerer");
		if (race==9)  SetDlgItemText(hwnd,IDC_ARCH,"Summoner");
		if (race==10) SetDlgItemText(hwnd,IDC_ARCH,"Arch-Harakim");
		if (race==11) SetDlgItemText(hwnd,IDC_ARCH,"God");
	}

	if (sex==1)   CheckRadioButton(hwnd,IDC_MALE,IDC_FEMALE,IDC_MALE);
	if (sex==2)   CheckRadioButton(hwnd,IDC_MALE,IDC_FEMALE,IDC_FEMALE);
}

void update_alpha(HWND hwnd)
{
	if (do_alpha==2) CheckRadioButton(hwnd,IDC_FULLALPHA,IDC_NOALPHA,IDC_FULLALPHA);
	if (do_alpha==1) CheckRadioButton(hwnd,IDC_FULLALPHA,IDC_NOALPHA,IDC_PARTIALALPHA);
	if (do_alpha==0) CheckRadioButton(hwnd,IDC_FULLALPHA,IDC_NOALPHA,IDC_NOALPHA);
}

void update_screenmode(HWND hwnd)
{
	if (screen_windowed == 0) CheckRadioButton(hwnd,IDC_WINDOWED,IDC_FULLSCREEN,IDC_FULLSCREEN);
	if (screen_windowed == 1) CheckRadioButton(hwnd,IDC_WINDOWED,IDC_FULLSCREEN,IDC_WINDOWED);

	//if (screen_width == 800) CheckRadioButton(hwnd,IDC_RES800,IDC_RES1600,IDC_RES800);
	//if (screen_width == 1280) CheckRadioButton(hwnd,IDC_RES800,IDC_RES1600,IDC_RES1280);
	//if (screen_width == 1600) CheckRadioButton(hwnd,IDC_RES800,IDC_RES1600,IDC_RES1600);
}

void update_buttons(HWND hwnd)
{
	update_alpha(hwnd);
	update_race(hwnd);
	update_screenmode(hwnd);
}

#pragma argsused
APIENTRY OptionsProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	int n;
	char buf[256];
	static int done=0;
	CREATESTRUCT *cs;

	switch (message) {
		case WM_CTLCOLOR:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORSCROLLBAR:
			SetTextColor((HDC) wParam,dlg_fcol);
			SetBkColor((HDC) wParam,dlg_col);
			return(int) dlg_back;

		case WM_CLOSE:			
			//if (IsDlgButtonChecked(hwnd,IDC_DOMUSIC)) domusic=1;
			//else domusic=0;
			if (IsDlgButtonChecked(hwnd,IDC_DOSOUND)) dosound=1;
			else dosound=0;

			if (IsDlgButtonChecked(hwnd,IDC_DOSHADOW)) do_shadow=1;
			else do_shadow=0;

			GetDlgItemText(hwnd,IDC_CNAME,pdata.cname,79);
			GetDlgItemText(hwnd,IDC_DESC,pdata.desc,149);

			save_options();
			EndDialog(hwnd,0);
			quit=1;
			return 1;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case    IDOK:
#ifdef DOCONVERT
					convert(hwnd);
					create_pnglib(hwnd);
					exit(1);
#endif

					if (so_status) {
						MessageBeep(MB_ICONEXCLAMATION);
						break;
					}
					//if (IsDlgButtonChecked(hwnd,IDC_DOMUSIC)) domusic=1;
					//else domusic=0;
					if (IsDlgButtonChecked(hwnd,IDC_DOSOUND)) dosound=1;
					else dosound=0;
					if (IsDlgButtonChecked(hwnd,IDC_DOSHADOW)) do_shadow=1;
					else do_shadow=0;

					GetDlgItemText(hwnd,IDC_CNAME,pdata.cname,79);
					GetDlgItemText(hwnd,IDC_DESC,pdata.desc,158);
					GetDlgItemText(hwnd,IDC_PASS,passwd,15);

					randomize();

					if (sex==0) sex=random(2)+1;
					if (race==0) race=random(3)+1;

					if (sex==1) 
					{
						if 			(race== 1)	race=  4;	// M Templar
						else if 	(race== 2)	race=  6;	// M Mercenary
						else if 	(race== 3)	race=  8;	// M Harakim
						/*
						else if 	(race== 4)	race= 12;	// M Arch-Templar
						else if 	(race== 5)	race= 14;	// M Pugilist
						else if 	(race== 6)	race= 16;	// M Warrior
						else if 	(race== 8)	race= 18;	// M Sorcerer
						else if 	(race== 9)	race= 20;	// M Summoner
						else if 	(race==10)	race= 22;	// M Arch-Harakim
						// */
						//else if	(race==11)  race=  2;   // M God
						else 					race=  6;	// M Mercenary
					} 
					else 
					{
						if 			(race== 1) 	race=  5;	// F Templar
						else if 	(race== 2) 	race=  7;	// F Mercenary
						else if 	(race== 3) 	race=  9;	// F Harakim
						/*
						else if 	(race== 4)	race= 13;	// F Arch-Templar
						else if 	(race== 5)	race= 15;	// F Pugilist
						else if 	(race== 6)	race= 17;	// F Warrior
						else if 	(race== 8)	race= 19;	// F Sorcerer
						else if 	(race== 9)	race= 21;	// F Summoner
						else if 	(race==10)	race= 23;	// F Arch-Harakim
						// */
						//else if	(race==11)  race=  3;   // F God
						else 					race=  7;	// F Mercenary
					}

					save_options();
					{
						void so_connect(void*);
						_beginthread(so_connect,16384,(void*)hwnd);
					}

					return 1;

				case    IDCANCEL:
					//if (IsDlgButtonChecked(hwnd,IDC_DOMUSIC)) domusic=1;
					//else domusic=0;
					if (IsDlgButtonChecked(hwnd,IDC_DOSOUND)) dosound=1;
					else dosound=0;
					if (IsDlgButtonChecked(hwnd,IDC_DOSHADOW)) do_shadow=1;
					else do_shadow=0;

					GetDlgItemText(hwnd,IDC_CNAME,pdata.cname,79);
					GetDlgItemText(hwnd,IDC_DESC,pdata.desc,149);

					save_options();
					EndDialog(hwnd,0);
					quit=1;
					return 1;
				//case    IDC_HELP:
				//	ShowWindow(hwnd,SW_MINIMIZE);
				//	ShellExecute(hwnd,"open",MHELP,NULL,NULL,SW_SHOWNORMAL);
				//	break;
				case    IDC_NEWS:
					ShowWindow(hwnd,SW_MINIMIZE);
					ShellExecute(hwnd,"open",MNEWS,NULL,NULL,SW_SHOWNORMAL);
					break;
				case    IDC_NEW:
					if (MessageBox(hwnd,new_msg1,"Are you sure?",MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL)!=IDYES)
						return 1;
					if (MessageBox(hwnd,new_msg2,"Are you really sure?",MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL)!=IDYES)
						return 1;
					okey.usnr=0;
					okey.race=0;
					okey.pass1=0;
					okey.pass2=0;
					pdata.changed=1;
					strcpy(okey.name,"New Account");
					SetDlgItemText(hwnd,IDC_RNAME,okey.name);
					SetDlgItemText(hwnd,IDC_CNAME,"");
					SetDlgItemText(hwnd,IDC_DESC,"");
					SetDlgItemText(hwnd,IDC_PASS,"");
					SetDlgItemText(hwnd,IDC_STATUS,"STATUS: OK");
					//
					race=0;
					sex=0;
					update_buttons(hwnd);
					//
					return 1;
				case IDC_SAVE:
					if (save_dialog(hwnd,buf)) save_char(hwnd,buf);
					return 1;
				case IDC_LOAD:
					if (load_dialog(hwnd,buf)) load_char(hwnd,buf);
					SetDlgItemText(hwnd,IDC_RNAME,okey.name);

					SetDlgItemText(hwnd,IDC_CNAME,pdata.cname);
					SetDlgItemText(hwnd,IDC_DESC,pdata.desc);

					translate_okey2race(&race,&sex);

					update_buttons(hwnd);
					return 1;
				/*case IDC_DOMUSIC:
					opmusic=1-opmusic; CheckDlgButton(hwnd,IDC_DOMUSIC,opmusic);
					return 1;*/
				case IDC_DOSOUND:
					opsound=1-opsound; CheckDlgButton(hwnd,IDC_DOSOUND,opsound);
					return 1;
				case IDC_DOSHADOW:
					opshadow=1-opshadow; CheckDlgButton(hwnd,IDC_DOSHADOW,opshadow);
					return 1;

				case IDC_CNAME:
					if (done && HIWORD(wParam)==EN_CHANGE) {
						n=SendDlgItemMessage(hwnd,LOWORD(wParam),EM_LINELENGTH,0,0);
						if (n>38) {
							GetDlgItemText(hwnd,LOWORD(wParam),buf,79);
							buf[38]=0;
							SetDlgItemText(hwnd,LOWORD(wParam),buf);
							MessageBeep(-1);
						}
						pdata.changed=1;
					}
					return 1;

				case IDC_DESC:
					if (done && HIWORD(wParam)==EN_CHANGE) {
						n=SendDlgItemMessage(hwnd,LOWORD(wParam),EM_LINELENGTH,0,0);
						if (n>150) {
							GetDlgItemText(hwnd,LOWORD(wParam),buf,149);
							buf[150]=0;
							SetDlgItemText(hwnd,LOWORD(wParam),buf);
							MessageBeep(-1);
						}
						pdata.changed=1;
					}
					return 1;

				case IDC_MALE:
					if (okey.usnr) MessageBox(hwnd,"Changing the gender has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else sex=1;
					update_buttons(hwnd);
					return 1;

				case IDC_FEMALE:
					if (okey.usnr) MessageBox(hwnd,"Changing the gender has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else sex=2;
					update_buttons(hwnd);
					return 1;
				
				// **** Race Checks **** //
				
				case IDC_TEMP:
					if (okey.usnr) 
						MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else 
					{
						race=1; 
						SetDlgItemText(hwnd,IDC_STATUS,"STATUS: OK\n\n\nTemplar:\nAn adept at melee combat, these fighters can become heavy hitters or unbreakable tanks. They learn only a few utility spells.");
					}
					update_buttons(hwnd); 
					return 1;
				case IDC_MERC:
					if (okey.usnr) 
						MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else 
					{
						race=2; 
						SetDlgItemText(hwnd,IDC_STATUS,"STATUS: OK\n\n\nMercenary:\nJack of all trades, master of none. These fighters can do a bit of everything, and can become capable in any situation.");
					}
					update_buttons(hwnd); 
					return 1;
				case IDC_HARA:
					if (okey.usnr) 
						MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else 
					{
						race=3; 
						SetDlgItemText(hwnd,IDC_STATUS,"STATUS: OK\n\n\nHarakim:\nSpellcaster extraordinare, these fighters can summon capable companions and hit foes with a powerful long-range blast.");
					}
					update_buttons(hwnd); 
					return 1;
					
				case IDC_ARCH:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=2; 
					update_buttons(hwnd); 
					return 1;
				
				case IDC_ARCHTEMPLAR:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=4; update_buttons(hwnd); return 1;
				case IDC_PUGILIST:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=5; update_buttons(hwnd); return 1;
					
				case IDC_WARRIOR:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=6; update_buttons(hwnd); return 1;
				case IDC_SEY:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=7; update_buttons(hwnd); return 1;
				case IDC_SORCERER:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=8; update_buttons(hwnd); return 1;
					
				case IDC_SUMMONER:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=9; update_buttons(hwnd); return 1;
				case IDC_ARCHHARAKIM:
					if (okey.usnr) MessageBox(hwnd,"Changing the race has no effect for existing accounts.","Sorry",MB_OK|MB_ICONSTOP);
					else race=10; update_buttons(hwnd); return 1;
				
				// **** //// ///// **** //

				case IDC_FULLALPHA:
					do_alpha=2;
					update_buttons(hwnd);
					return 1;
					
				case IDC_PARTIALALPHA:
					do_alpha=1;
					update_buttons(hwnd);
					return 1;

				case IDC_NOALPHA:
					do_alpha=0;
					update_buttons(hwnd);
					return 1;
				
				case IDC_WINDOWED:
					screen_windowed=1;
					setres_1280();
					update_buttons(hwnd);
					return 1;
				
				case IDC_FULLSCREEN:
					screen_windowed=0;
					setres_1280();
					update_buttons(hwnd);
					return 1;
				
				/*
				case IDC_RES800:
					setres_800();
					update_buttons(hwnd);
					return 1;
				
				case IDC_RES1280:
					setres_1280();
					update_buttons(hwnd);
					return 1;
				
				case IDC_RES1600:
					setres_1600();
					update_buttons(hwnd);
					return 1;
				*/

				default:
					return 1;
			}
		case WM_INITDIALOG:
			if (ctl3don) ctl3don(hwnd,0xfffe);

			//CheckDlgButton(hwnd,IDC_DOMUSIC,opmusic);
			CheckDlgButton(hwnd,IDC_DOSOUND,opsound);
			CheckDlgButton(hwnd,IDC_DOSHADOW,opshadow);

			SetFocus(GetDlgItem(hwnd,IDOK));

			SetDlgItemText(hwnd,IDC_RNAME,okey.name);

			SetDlgItemText(hwnd,IDC_CNAME,pdata.cname);
			SetDlgItemText(hwnd,IDC_DESC,pdata.desc);			

			translate_okey2race(&race,&sex);

            update_buttons(hwnd);

			sprintf(buf,MNAME" v%d.%02d.%02d  --  Game Options",VERSION>>16,(VERSION>>8)&255,VERSION&255);
			SetWindowText(hwnd,buf);
			done=1;
			return 1;

		default:
			return 0;
	}
}

void options(void)
{
	opmusic=domusic; opsound=dosound; opshadow=do_shadow;

	if (DialogBox(hinst,MAKEINTRESOURCE(OPTIONS),desk_hwnd,OptionsProc)==-1) {
		MessageBeep(MB_ICONEXCLAMATION);
	}
	save_options();
}
