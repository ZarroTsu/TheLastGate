#include <stdio.h>
#include <alloc.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include "ddraw.h"
#include <process.h>
#include <signal.h>
#pragma hdrstop
#include "dd.h"
#include "common.h"
#include "inter.h"
//#include "minilzo.h"

extern int RED,GREEN,BLUE,RGBM,MAXXOVER;
extern char *DDERR;
extern int dd_cache_hit,dd_cache_miss,MAXCACHE,invisible,cachex,cachey,MAXXOVER;
extern int pskip,pidle;
extern int maxmem,usedmem,maxvid,usedvid;
extern int noshop;

// Screen data, can be shared with other files via extern
int screen_width, screen_height, screen_tilexoff, screen_tileyoff, screen_viewsize, view_subedges;
int screen_overlay_sprite;
int xwalk_nx, xwalk_ny, xwalk_ex, xwalk_ey, xwalk_sx, xwalk_sy, xwalk_wx, xwalk_wy;
short screen_windowed;
short screen_renderdist;

void dd_invalidate_cache(void);
void conv_init(void);
int init_pnglib(void);

extern int cursor_type;
HCURSOR cursor[10];

void cmd(int cmd,int x,int y);

int quit=0;
char host_addr[84]={MHOST};
int host_port=5555;

extern char path[];
extern int tricky_flag;

HWND desk_hwnd;
HINSTANCE hinst;

int init_sound(HWND hwnd);
void engine(void);

#define MWORD 2048

char input[128];
int in_len=0;
int cur_pos=0;
int hist_nr=0;
int view_pos=0;
int tabmode=0;
int tabstart=0;
int logstart=0;
int logtimer=0;
int do_alpha=2;
int do_shadow=1;

void dd_invalidate_alpha(void);

char history[20][128];
int hist_len[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char words[MWORD][40];

#define xisalpha(a) (((a)=='#') || (isalpha(a)))

void complete_word(void)
{
	int n=0,z,pos;
	char buf[40];

	if (cur_pos<1) return;

	for (z=cur_pos-1; z>=0; z--) if (!xisalpha(input[z])) {
			z++; break;
		}
	if (z<0) z=0;
	while (z<cur_pos && n<39) buf[n++]=input[z++];
	buf[n]=0;

	if (n<1) return;

	for (z=tabstart; z<MWORD; z++) {
		if (!strncmp(buf,words[z],n) && strlen(words[z])>(unsigned)n) {
			pos=cur_pos;
			while (pos<115 && words[z][n]) input[pos++]=words[z][n++];
			if (pos<115) input[pos++]=' ';
			in_len=pos;
			tabmode=1;
			tabstart=z+1;
			return;
		}
	}
	tabmode=0;
	tabstart=0;
	in_len=cur_pos;
}

void add_word(char *buf)
{
	int n;

	for (n=0; n<MWORD-1; n++)
		if (!strcmp(words[n],buf)) break;

	memmove(words[1],words[0],n*40);
	memcpy(words[0],buf,40);
}

void add_words(void)
{
	char buf[40];
	int z1=0,z2;

	while (input[z1]) {
		z2=0;
		while (xisalpha(input[z1]) && z2<39) {
			buf[z2++]=input[z1++];
		}
		buf[z2]=0;
		add_word(buf);
		while (input[z1] && !xisalpha(input[z1])) z1++;
	}
}

// CTL3D
void pascal (*ctl3don)(HANDLE,short int)=NULL;
HBRUSH dlg_back;
int dlg_col,dlg_fcol;

extern int blockcnt,blocktot,blockgc;
int mx=0,my=0;

void say(char *input)
{
	int n;
	char buf[16];
	buf[0]=CL_CMD_INPUT1;
	for (n=0; n<15; n++)
		buf[n+1]=input[n];
	xsend(buf);

	buf[0]=CL_CMD_INPUT2;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+15];
	xsend(buf);

	buf[0]=CL_CMD_INPUT3;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+30];
	xsend(buf);

	buf[0]=CL_CMD_INPUT4;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+45];
	xsend(buf);

	buf[0]=CL_CMD_INPUT5;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+60];
	xsend(buf);

	buf[0]=CL_CMD_INPUT6;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+75];
	xsend(buf);

	buf[0]=CL_CMD_INPUT7;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+90];
	xsend(buf);

	buf[0]=CL_CMD_INPUT8;
	for (n=0; n<15; n++)
		buf[n+1]=input[n+105];
	xsend(buf);
}

int do_ticker=1;
extern int gamma;
extern int usedvidmem;

extern int alphapix,fullpix;

LRESULT FAR PASCAL _export MainWndProc(HWND hWnd, UINT message,WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	int keys;
	extern int ser_ver,xmove;

	keys=0;
	if (GetAsyncKeyState(VK_SHIFT)&0x8000) keys|=1;
	if ((GetAsyncKeyState(VK_CONTROL)&0x8000) || (GetAsyncKeyState(VK_MENU)&0x8000)) keys|=2;

	switch (message) {
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			switch ((int)wParam) 
			{
				// **************** Ctrl cases for number keys **************** //
				// -------- CTRL + 1 
				case  49: if (keys>=2) button_command(16); break;
				// -------- CTRL + 2
				case  50: if (keys>=2) button_command(17); break;
				// -------- CTRL + 3
				case  51: if (keys>=2) button_command(18); break;
				// -------- CTRL + 4
				case  52: if (keys>=2) button_command(19); break;
				
				// -------- ESC
				case  27: 
					cmd(CL_CMD_RESET,0,0); 
					show_shop=0; 
					show_wps=0; 
					noshop=QSIZE*4; 
					xmove=0; 
					break;
				// **************** F-Key Row 1 **************** //
				// -------- F1
				case 112: 
					cmd(CL_CMD_MODE,2,0); 
					break;
				// -------- F2
				case 113: 
					cmd(CL_CMD_MODE,1,0); 
					break;
				// -------- F3
				case 114: 
					cmd(CL_CMD_MODE,0,0); 
					break;
				// -------- F4
				case 115: 
					pdata.show_proz=1-pdata.show_proz; 
					break;
				// **************** F-Key Row 2 **************** //
				// -------- F5
				case 116: 
					pdata.show_stats=1-pdata.show_stats; 
					//else do_alpha++; 
					//if (do_alpha==3) do_alpha=0; 
					//dd_invalidate_alpha(); 
					break;
				// -------- F6
				case 117: 
					pdata.hide=1-pdata.hide; 
					break;
				// -------- F7
				case 118: 
					pdata.show_names=1-pdata.show_names; 
					break;
				// -------- F8
				case 119: 
					pdata.show_bars=1-pdata.show_bars; 
					break;
				// **************** F-Key Row 3 **************** //
				// -------- F9
				case 120: 
					dd_savescreen(); 
					break;
				// -------- F10
				case 121: 
					button_command(25);
					gamma+=250;
					if (gamma>6000)	gamma=5000;
					xlog(2,"Set gamma correction to %1.2f",gamma/5000.0);
					dd_invalidate_cache();
					break;
				// -------- F11
				case 122: 
					xlog(2," ");
					xlog(2,"Client Version %d.%02d.%02d",VERSION>>16,(VERSION>>8)&255,VERSION&255);
					xlog(2,"Server Version %d.%02d.%02d",ser_ver>>16,(ser_ver>>8)&255,ser_ver&255);
					xlog(2,"MAXX=%d, MAXY=%d, MAXXO=%d",MAXX,MAXY,MAXXOVER);
					xlog(2,"R=%04X, G=%04X, B=%04X",RED,GREEN,BLUE);
					xlog(2,"RGBM=%d",RGBM);
					xlog(2,"MAXCACHE=%d",MAXCACHE);
					xlog(2,"Hit=%d, Miss=%d, Invis=%d",dd_cache_hit,dd_cache_miss,invisible);
					xlog(2,"Ratio=%.2f%%",100.0/(dd_cache_hit+dd_cache_miss)*dd_cache_hit);
					xlog(2,"Skip=%d%% Idle=%d%%",pskip,pidle);
					xlog(2,"MaxMem=%dK, UsedMem=%dK",maxmem>>10,usedmem>>10);
					xlog(2,"MemBlocks=%d (T=%d,GC=%d)",blockcnt,blocktot,blockgc);
					xlog(2,"MaxVid=%dK, UsedVid=%dK",(maxvid*32*32*2)>>10,(usedvid*32*32*2)>>10);
					xlog(2,"cachex=%d, cachey=%d, MAXXOVER=%d",cachex,cachey,MAXXOVER);
					xlog(2,"usedvidmemflag=%d",usedvidmem);
					xlog(2,"alphapix=%d, fullpix=%d, ratio=%.2f",alphapix,fullpix,100.0/(alphapix+fullpix+1)*alphapix);

//                                do_ticker=1-do_ticker;
					break;
				// -------- F12
				case 123: 
					cmd_exit();
					break;
				
				// -------- Insert
				case 45:
					cmd3(CL_CMD_INV,9,1,selected_char);
					break;

				// **************** Text Editor **************** //
				// -------- Tab
				case   9: 
					complete_word();
					break;
				// -------- Backspace
				case   8: 
					if (cur_pos && in_len) {
						if (tabmode) {
							in_len=cur_pos; tabmode=0; tabstart=0;
						}
						if (cur_pos>in_len)	cur_pos=in_len;
						memmove(input+cur_pos-1,input+cur_pos,120-cur_pos);
						in_len--;
						cur_pos--;
					}
					break;
				// -------- Del
				case  46: 
					if (in_len) {
						if (tabmode) {
							in_len=cur_pos; tabmode=0; tabstart=0;
						} else {
							memmove(input+cur_pos,input+cur_pos+1,120-cur_pos);
							in_len--;
						}
					}
					break;
				case  33:
					if (logstart<22*8) {
						logstart+=11; logtimer=TICKS*30/TICKMULTI;
					}
					break;
				case  34:
					if (logstart>0) {
						logstart-=11; logtimer=TICKS*30/TICKMULTI;
					}
					break;
				// -------- Home
				case  36:
					cur_pos=0; 
					tabmode=0; 
					tabstart=0; 
					break;
				// -------- End
				case  35:
					cur_pos=in_len; 
					tabmode=0; 
					tabstart=0; 
					break;
				case  37:
					if (cur_pos) cur_pos--;
					tabmode=0; 
					tabstart=0;
					break;
				case  39:
					if (cur_pos<115) cur_pos++;
					tabmode=0; 
					tabstart=0;
					break;
				case  38:
					if (hist_nr<19) 
					{
						memcpy(history[hist_nr],input,128);
						hist_len[hist_nr]=in_len;
						hist_nr++;

						memcpy(input,history[hist_nr],128);
						in_len=cur_pos=hist_len[hist_nr];

						tabmode=0; tabstart=0;
					}
					break;
				case  40:
					if (hist_nr>0) 
					{
						memcpy(history[hist_nr],input,128);
						hist_len[hist_nr]=in_len;

						hist_nr--;

						memcpy(input,history[hist_nr],128);
						in_len=cur_pos=hist_len[hist_nr];

						tabmode=0; tabstart=0;
					}
					break;
/*
					default:
						xlog(3,"key=%d",(int)wParam);
						break;
*/

			}
			break;

		case WM_CHAR:
			switch ((int)wParam) 
			{
				// **************** Ctrl cases for letter keys **************** //
				// -------- CTRL + Q
				case  17: if (keys>=2) button_command(20);break;
				// -------- CTRL + W
				case  23: if (keys>=2) button_command(21);break;
				// -------- CTRL + E
				case   5: if (keys>=2) button_command(22);break;
				// -------- CTRL + R
				case  18: if (keys>=2) button_command(23);break;
				
				// -------- CTRL + A
				case   1: if (keys>=2) button_command(24);break;
				// -------- CTRL + S
				case  19: if (keys>=2) button_command(25);break;
				// -------- CTRL + D
				case   4: if (keys>=2) button_command(26);break;
				// -------- CTRL + F
				case   6: if (keys>=2) button_command(27);break;
				
				// -------- CTRL + Z
				case  26: if (keys>=2) button_command(28);break;
				// -------- CTRL + X
				case  24: if (keys>=2) button_command(29);break;
				// -------- CTRL + C
				case   3: if (keys>=2) button_command(30);break;
				// -------- CTRL + V
				case  22: if (keys>=2) button_command(31);break;
				
				// -------- Return Key
				case 13: 
					if (in_len==0) break;

					if (tabmode) 
					{
						tabmode=0; tabstart=0;
						in_len--;
					}

					memmove(history[2],history[1],18*128);
					memmove(&hist_len[2],&hist_len[1],sizeof(int)*18);

					memcpy(history[1],input,128);
					hist_len[1]=in_len;

					input[in_len]=0;
					in_len=0;
					cur_pos=0;
					view_pos=0;
					hist_nr=0;

					add_words();

					say(input);

					break;

				default:
					// Keys between 32 and 127 are valid typing keys
					if ((int)wParam>31 && (int)wParam<128 && in_len<115 && keys<2) 
					{
						if (tabmode) 
						{
							if (!isalnum((char)wParam))	in_len--;
							cur_pos=in_len;
							tabmode=0;
							tabstart=0;
						}
						if (cur_pos>in_len)	cur_pos=in_len;
						memmove(input+cur_pos+1,input+cur_pos,120-cur_pos);
						input[cur_pos]=(char)wParam;
						in_len++;
						cur_pos++;
					}
					break;
			}
			break;

		case WM_PAINT:
			BeginPaint(hWnd,&ps);
			EndPaint(hWnd,&ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_MOUSEMOVE:
			mouse(LOWORD(lParam),HIWORD(lParam),MS_MOVE);
			mx=LOWORD(lParam);
			my=HIWORD(lParam);
			break;

		case WM_LBUTTONDOWN:
			mouse(LOWORD(lParam),HIWORD(lParam),MS_LB_DOWN);
			break;

		case WM_LBUTTONUP:
			mouse(LOWORD(lParam),HIWORD(lParam),MS_LB_UP);
			break;

		case WM_RBUTTONDOWN:
			mouse(LOWORD(lParam),HIWORD(lParam),MS_RB_DOWN);
			break;

		case WM_RBUTTONUP:
			mouse(LOWORD(lParam),HIWORD(lParam),MS_RB_UP);
			break;

		default:
			return(DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}


HWND InitWindow(HINSTANCE hInstance,int nCmdShow)
{
	WNDCLASS wc;
	HWND hWnd;
	DWORD dwWidth = GetSystemMetrics(SM_CXSCREEN);
	DWORD dwHeight = GetSystemMetrics(SM_CYSCREEN);
	char buf[256];
	int n;
	int xx, yy;
    HMENU hmenu;
    LONG style;

	xx = yy = 10;

	hinst=hInstance;

	for (n=1; n<10; n++)
		cursor[n]=LoadCursor(hInstance,MAKEINTRESOURCE(n));

	wc.style=CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc=(long (FAR PASCAL*)())MainWndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(1));
	wc.hCursor=NULL;
	wc.hbrBackground=NULL;
	wc.lpszMenuName=NULL;
	wc.lpszClassName="DDCWin";

	RegisterClass(&wc);

	sprintf(buf,MNAME" v%d.%02d.%02d",
			VERSION>>16,(VERSION>>8)&255,VERSION&255);
	
	if (screen_windowed == 0) 
	{
		desk_hwnd=hWnd=CreateWindowEx(WS_EX_TOPMOST, "DDCWin", buf, WS_VISIBLE|WS_BORDER|WS_POPUP|WS_SYSMENU, 
			xx, yy, 0, 0, NULL, NULL, hInstance, NULL);
	} 
	else 
	{
		xx = dwWidth/2 - (screen_width+2)/2;
		yy = dwHeight/2 - (screen_height+21)/2;
		desk_hwnd=hWnd=CreateWindowEx(0, "DDCWin", buf, WS_VISIBLE|WS_BORDER|WS_SYSMENU|WS_MINIMIZEBOX, 
			xx, yy, screen_width+2, screen_height+21, NULL, NULL, hInstance, NULL);
			
		hmenu = GetSystemMenu(hWnd,FALSE);
		DeleteMenu(hmenu,SC_CLOSE,MF_BYCOMMAND);
		style = GetWindowLong(hWnd,GWL_STYLE);
		style ^= WS_SYSMENU;
		SetWindowLong(hWnd,GWL_STYLE,style);
		
		hmenu = GetSystemMenu(desk_hwnd,FALSE);
		DeleteMenu(hmenu,SC_CLOSE,MF_BYCOMMAND);
		style = GetWindowLong(desk_hwnd,GWL_STYLE);
		style ^= WS_SYSMENU;
		SetWindowLong(desk_hwnd,GWL_STYLE,style);
	}

    SetFocus(hWnd);
	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

int parse_cmd(char *s)
{
	int n;

	while (isspace(*s))	s++;
	while (*s) {
		if (*s=='-') {
			s++;
			if (tolower(*s)=='t') {
				s++;
				tricky_flag=1;
			} else if (tolower(*s)=='d') {
				s++;
				while (isspace(*s)) s++;
				n=0; while (n<150 && *s && !isspace(*s)) path[n++]=*s++;
				if (path[n]!='\\') path[n++]='\\';
				path[n]=0;
			} else if (tolower(*s)=='p') {
				s++;
				while (isspace(*s)) s++;
				host_port=atoi(s);
				while (*s && !isspace(*s)) s++;
			} else return -1;
		} else return -2;
		while (isspace(*s))	s++;
	}
	return 1;
}

void log_system_data(void)
{
	char buf[256];
	unsigned int langid,lcid,size=80;
	char systemdir[256],windir[256],cdir[256],user[256],computer[256];

	langid=GetSystemDefaultLangID();
	lcid=GetSystemDefaultLCID();

	GetSystemDirectory(systemdir,80);
        GetWindowsDirectory(windir,80);
	GetCurrentDirectory(80,cdir);
	GetUserName((void*)user,&size); size=80;
	GetComputerName((void*)computer,&size);

	sprintf(buf,"|langid=%u, lcid=%u",langid,lcid); say(buf);
	sprintf(buf,"|systemdir=\"%s\"",systemdir); say(buf);
	sprintf(buf,"|windowsdir=\"%s\"",windir); say(buf);
	sprintf(buf,"|currentdir=\"%s\"",cdir); say(buf);
	sprintf(buf,"|username=\"%s\"",user); say(buf);
	sprintf(buf,"|computername=\"%s\"",computer); say(buf);
}

#pragma argsused
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd;
	char buf[2048];
	int tmp;
	HANDLE lib;
	void pascal (*regxx)(HANDLE);
	void pascal (*regxy)(HANDLE);
	HANDLE mutex;
	char *mutmoa;
	int tmpi,tmpj = 0;
	
        /* create_pnglib();
	exit(1); */

	parse_cmd(lpCmdLine);

	mutex=CreateMutex(NULL,0,mutmoa="MOATLG");
	if (mutex==NULL || GetLastError()==ERROR_ALREADY_EXISTS && strcmp(host_addr,"127.0.0.1")) {
		MessageBox(0,"Another instance of "MNAME" is already running.","Error",MB_OK|MB_ICONSTOP);
		return 0;
	}
	// Amateur anti-cheat
	for (tmpi = 0; tmpi<6; tmpi++) tmpj+=mutmoa[tmpi];
	if (tmpj!=452)
	{
		MessageBox(0,"There is a problem with "MNAME". You aren't trying to cheat, are you?","Error",MB_OK|MB_ICONSTOP);
		return 0;
	}

	lib=LoadLibrary("CTL3D32.DLL");
	if (lib) {
		regxx=(void pascal *)GetProcAddress(lib,"Ctl3dRegister");
		if (regxx) regxx(GetCurrentProcess());
		ctl3don=(void pascal *)GetProcAddress(lib,"Ctl3dSubclassDlg");
		regxy=(void pascal *)GetProcAddress(lib,"Ctl3dUnregister");
	} else {
		regxy=NULL;
		ctl3don=NULL;
	}

	dlg_col=GetSysColor(COLOR_BTNFACE);
	dlg_back=CreateSolidBrush(dlg_col);
	dlg_fcol=GetSysColor(COLOR_WINDOWTEXT);

	screen_renderdist=RENDERDIST;

	// Default windowed
	screen_windowed=1;

	setres_default();

	load_options();
	options();
	hwnd=InitWindow(hInstance,nCmdShow);

	init_engine();
	
	if (quit) exit(0);

	init_sound(hwnd);

	if (screen_windowed == 1) {
		tmp=dd_init_windowed(hwnd,screen_width,screen_height);
	} else {
		tmp=dd_init(hwnd,screen_width,screen_height);
	}
	if (tmp!=0) {

		sprintf(buf,"|DDERROR=%d",-tmp);
		say(buf);
		Sleep(1000);

		sprintf(buf,
				"DirectX init failed with code %d.\n"
				"DDError=%s\n"
				"Client Version %d.%02d.%02d\n"
				"MAXX=%d, MAXY=%d\n"
				"R=%04X, G=%04X, B=%04X\n"
				"RGBM=%d\n"
				"MAXCACHE=%d\n",
				-tmp,DDERR,VERSION>>16,(VERSION>>8)&255,VERSION&255,MAXX,MAXY,RED,GREEN,BLUE,RGBM,MAXCACHE);
		MessageBox(hwnd,buf,"DirectX init failed.",MB_ICONSTOP|MB_OK);
		exit(1);
	}
    sprintf(buf,"|R=%04X, G=%04X, B=%04X, RGBM=%d",RED,GREEN,BLUE,RGBM);
	say(buf);

    init_xalloc();
	conv_init();
	init_pnglib();
	dd_init_sprites();	

	if (RGBM==-1) {
		sprintf(buf,"|unknown card: R=%04X G=%04X B=%04X",RED,GREEN,BLUE);
		say(buf);
	}

	log_system_data();

	engine();

	if (screen_windowed == 1) {
		dd_deinit_windowed();
	} else {
		dd_deinit();
	}

	if (regxy) regxy(GetCurrentProcess());

	save_options();

	return 0;
}
