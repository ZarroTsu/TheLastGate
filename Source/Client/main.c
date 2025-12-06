/*
 * MAIN.C - Client Entry Point and Main Loop
 *
 * TODO: MODERN GCC/MINGW COMPATIBILITY
 * ====================================
 * This file requires updates for modern compiler compatibility:
 *
 * HEADERS:
 * - <alloc.h> -> <malloc.h> or <stdlib.h>
 * - <io.h> -> <unistd.h> or standard C file I/O
 * - <windowsx.h> -> May not be needed
 * - "ddraw.h" -> Replace with SDL2 headers
 * - #pragma hdrstop -> Remove (Borland C++ specific)
 *
 * WINDOWS API:
 * - HWND, HINSTANCE -> SDL_Window* in SDL2
 * - HCURSOR -> SDL_Cursor* in SDL2
 * - Window message loop -> SDL event loop
 * - WinMain() -> main() with SDL_main wrapper
 *
 * See individual TODO comments below for specific locations.
 */

// TODO: Modern GCC/MinGW - Replace deprecated headers
#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <io.h>     // TODO: Replace with <unistd.h> or standard C file I/O
#include <stdlib.h>
#include <windows.h>  // TODO: Replace with SDL2 headers for cross-platform
#include <windowsx.h> // TODO: May not be needed with SDL2
#include "main.h"
#include "ddraw.h"    // TODO: Remove - DirectDraw is deprecated
#include <process.h>  // TODO: Use pthread or SDL_thread for cross-platform threading
#include <signal.h>
#pragma hdrstop  // TODO: Remove - Borland C++ specific
#include <SDL2/SDL_mouse.h>

#include "common.h"
#include "input.h"
#include "inter.h"
#include "render.h"
#include "sound.h"
//#include "minilzo.h"

AppState app_state = {
	.tricky_flag = 0,
	.gamma = 5000,
	.path = "",
	.windowed = 1,
	.window_size = {1280, 720},
	.volume_level = 10, // 100%
	.cost_helper = false,
	.escape_closes_menus_first = true,
	.give_more = false,
	.use_queue = false,
};

extern void cmd3(int cmd,int x,int y,int z);

extern int pskip,pidle;
extern int noshop;
extern int selected_char;

// Scroll Wheel - using position from other files
extern int inv_pos,skill_pos,wps_pos,hudmode;
extern int gui_inv_x[],gui_inv_y[],gui_skl_names[];
extern unsigned int dept_page;

// Screen data, can be shared with other files via extern
int screen_width, screen_height, screen_tilexoff, screen_tileyoff, screen_viewsize, view_subedges;
//int screen_overlay_sprite;
int xwalk_nx, xwalk_ny, xwalk_ex, xwalk_ey, xwalk_sx, xwalk_sy, xwalk_wx, xwalk_wy;
short screen_renderdist;
int screen_target_fps = 48;  // Configurable FPS target (default 120)

void dd_invalidate_cache(void);
void conv_init(void);
int init_pnglib(void);

extern int cursor_type;
// TODO: Modern GCC/MinGW - HCURSOR is Windows-specific
// SDL2: Use SDL_Cursor* created with SDL_CreateCursor() or SDL_CreateSystemCursor()
HCURSOR cursor[10];  // TODO: Replace with SDL_Cursor* array
SDL_Cursor* cursors[10];

void cmd(int cmd,int x,int y);

int quit=0;
char host_addr[84]={MHOST};
char host_proxy[84]={MPROXY};
int host_port=5555;

// TODO: Modern GCC/MinGW - Windows-specific window handles
// SDL2: Use SDL_Window* instead of HWND
// HINSTANCE not needed in SDL2
HWND desk_hwnd;      // TODO: Replace with SDL_Window*
HINSTANCE hinst;     // TODO: Remove - not needed in SDL2

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
int do_alpha=0;
int do_shadow=1;
int do_darkmode=0;

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
// TODO: Modern GCC/MinGW - CTL3D (3D control library) is obsolete Windows 95/98 era
// TODO: Remove entire CTL3D section, modern UI frameworks don't need this
// HANDLE is Windows-specific, HBRUSH is GDI-specific
void pascal (*ctl3don)(HANDLE,short int)=NULL;
// TODO: Modern GCC/MinGW - HBRUSH is GDI-specific (Graphics Device Interface)
// SDL2: Not applicable, use SDL_SetRenderDrawColor for colors
HBRUSH dlg_back;
int dlg_col,dlg_fcol;

int mx=0,my=0;
// TODO: Modern GCC/MinGW - POINT is Windows-specific struct (x, y coordinates)
// SDL2: Use SDL_Point struct or just separate int x, y variables
POINT pt;

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

int parse_cmd(char *s)
{
	int n;

	while (isspace(*s))	s++;
	while (*s) {
		if (*s=='-') {
			s++;
			if (tolower(*s)=='t') {
				s++;
				app_state.tricky_flag=1;
			} else if (tolower(*s)=='d') {
				s++;
				while (isspace(*s)) s++;
				n=0; while (n<150 && *s && !isspace(*s)) app_state.path[n++]=*s++;
				if (app_state.path[n]!='\\') app_state.path[n++]='\\';
				app_state.path[n]=0;
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

// TODO: Modern GCC/MinGW - log_system_data uses Windows-specific system info APIs
// Cross-platform: Use standard C/POSIX functions or SDL_GetPlatform() for basic info
void log_system_data(void)
{
	char buf[256];
	unsigned int langid,lcid,size=80;
	char systemdir[256],windir[256],cdir[256],user[256],computer[256];

	// TODO: Modern GCC/MinGW - GetSystemDefaultLangID/GetSystemDefaultLCID are Windows-specific
	// Cross-platform: Use setlocale(LC_ALL, NULL) to get locale, or SDL doesn't provide this
	langid=GetSystemDefaultLangID();
	lcid=GetSystemDefaultLCID();

	// TODO: Modern GCC/MinGW - GetSystemDirectory/GetWindowsDirectory are Windows-specific
	// Cross-platform: Not directly applicable, use getcwd() for current directory
	GetSystemDirectory(systemdir,80);
        GetWindowsDirectory(windir,80);
	// TODO: Modern GCC/MinGW - GetCurrentDirectory is Windows-specific
	// Cross-platform: Use getcwd() from <unistd.h>
	GetCurrentDirectory(80,cdir);
	// TODO: Modern GCC/MinGW - GetUserName/GetComputerName are Windows-specific
	// Cross-platform: Use getenv("USER") or getenv("USERNAME") for user, gethostname() for computer
	GetUserName((void*)user,&size); size=80;
	GetComputerName((void*)computer,&size);

	sprintf(buf,"|langid=%u, lcid=%u",langid,lcid); say(buf);
	sprintf(buf,"|systemdir=\"%s\"",systemdir); say(buf);
	sprintf(buf,"|windowsdir=\"%s\"",windir); say(buf);
	sprintf(buf,"|currentdir=\"%s\"",cdir); say(buf);
	sprintf(buf,"|username=\"%s\"",user); say(buf);
	sprintf(buf,"|computername=\"%s\"",computer); say(buf);
}

#pragma argsused  // TODO: Remove - Borland C++ specific pragma
// TODO: Modern GCC/MinGW - WinMain is Windows-specific entry point
// SDL2: Replace with standard main() using SDL_main wrapper
// Example:
//   #define SDL_MAIN_HANDLED  // or use SDL_main.h
//   int main(int argc, char* argv[]) {
//     SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
//     // ... rest of initialization
//   }
// TODO: Modern GCC/MinGW - PASCAL is obsolete keyword
// MinGW: Use WINAPI instead (defined as __stdcall in windows.h)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd;  // TODO: SDL2: Replace with SDL_Window*
	char buf[2048];
	int tmp;
	HANDLE lib;  // TODO: Modern GCC/MinGW - HANDLE is Windows-specific
	void pascal (*regxx)(HANDLE);  // TODO: Remove - CTL3D32.DLL is obsolete
	void pascal (*regxy)(HANDLE);  // TODO: Remove - CTL3D32.DLL is obsolete
	HANDLE mutex;  // TODO: Modern GCC/MinGW - Replace with SDL_mutex or file locking
	char *mutmoa;
	int tmpi,tmpj = 0;

        /* create_pnglib();
	exit(1); */

	// AllocConsole();
	// freopen("CONOUT$", "w", stdout);
	// freopen("CONOUT$", "w", stderr);
	// freopen("CONIN$",  "r", stdin);
	parse_cmd(lpCmdLine);
	freopen("client.log", "w", stdout);
	setvbuf(stdout, NULL, _IONBF, 0); // unbuffered (optional but recommended)

	// TODO: Modern GCC/MinGW - CreateMutex is Windows-specific for single instance check
	// Cross-platform: Use file locking (flock/lockf) or SDL_CreateMutex for thread safety
	// Example: int lock_fd = open("game.lock", O_CREAT | O_RDWR, 0666); flock(lock_fd, LOCK_EX | LOCK_NB);
	mutex=CreateMutex(NULL,0,mutmoa="MOATLG");
	// TODO: Modern GCC/MinGW - GetLastError is Windows-specific
	// Cross-platform: Use errno for POSIX functions
	if (mutex==NULL || GetLastError()==ERROR_ALREADY_EXISTS && strcmp(host_addr,"127.0.0.1")) {
		// TODO: Modern GCC/MinGW - MessageBox is Windows-specific
		// SDL2: Use SDL_ShowSimpleMessageBox()
		// Example: SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "message", NULL);
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

	// TODO: Modern GCC/MinGW - LoadLibrary/GetProcAddress are Windows-specific dynamic loading
	// TODO: CTL3D32.DLL is obsolete (Win95/98 era 3D controls), remove this entire section
	// Cross-platform: Use dlopen/dlsym on POSIX systems if dynamic loading is needed
	lib=LoadLibrary("CTL3D32.DLL");
	if (lib) {
		// TODO: Modern GCC/MinGW - GetCurrentProcess is Windows-specific
		regxx=(void pascal *)GetProcAddress(lib,"Ctl3dRegister");
		if (regxx) regxx(GetCurrentProcess());
		ctl3don=(void pascal *)GetProcAddress(lib,"Ctl3dSubclassDlg");
		regxy=(void pascal *)GetProcAddress(lib,"Ctl3dUnregister");
	} else {
		regxy=NULL;
		ctl3don=NULL;
	}

	// TODO: Modern GCC/MinGW - GetSysColor is Windows-specific
	// SDL2: Not directly applicable, use custom color scheme or theme
	dlg_col=GetSysColor(COLOR_BTNFACE);
	// TODO: Modern GCC/MinGW - CreateSolidBrush is GDI-specific
	// SDL2: Not needed, use SDL_SetRenderDrawColor for solid colors
	dlg_back=CreateSolidBrush(dlg_col);
	dlg_fcol=GetSysColor(COLOR_WINDOWTEXT);

	screen_renderdist=RENDERDIST;

	setres_default();

	// TODO: MinGW - Set hinst early so options() can use it before InitWindow()
	hinst = hInstance;

	load_options();
	options();  // Show options dialog BEFORE creating the game window
	init_engine();

	if (quit) exit(0);

	init_sound();
	tmp=init(app_state.windowed);
	/*
	if (tmp!=0) { // A hacky fix for fullscreen support
		screen_height=800;
		if (screen_windowed == 1) {
			tmp=dd_init_windowed(hwnd,screen_width,screen_height);
		} else {
			tmp=dd_init(hwnd,screen_width,screen_height);
		}
	}
	*/

	if (tmp!=0) {

		sprintf(buf,"|DDERROR=%d",-tmp);
		say(buf);
		SDL_Delay(1000);

		// TODO: Revisit this, it needs way better logging...
		sprintf(buf,
				"SDL init failed with code %d.\n"
				"Client Version %d.%02d.%02d\n"
				"R=%04X, G=%04X, B=%04X\n"
				-tmp,VERSION>>16,(VERSION>>8)&255,VERSION&255,RED,GREEN,BLUE);
		// TODO: Modern GCC/MinGW - MessageBox is Windows-specific
		// SDL2: Use SDL_ShowSimpleMessageBox()
		MessageBox(hwnd,buf,"DirectX init failed.",MB_ICONSTOP|MB_OK);
		exit(1);
	}
    sprintf(buf,"|R=%04X, G=%04X, B=%04X, RGBM=%d",RED,GREEN,BLUE);
	say(buf);

	init_input();

	log_system_data();

	engine();

	deinit();

	if (regxy) regxy(GetCurrentProcess());

	save_options();

	cleanup_sound();

	return 0;
}