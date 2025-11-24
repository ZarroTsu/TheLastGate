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

#include "dd.h"
#include "common.h"
#include "input.h"
#include "inter.h"
#include "render.h"
#include "sound.h"
//#include "minilzo.h"

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
short screen_windowed;
short screen_renderdist;

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

extern char path[];

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

// TODO: Modern GCC/MinGW - MainWndProc is Windows-specific message handler
// SDL2: Replace entire function with SDL event loop in main()
// Example:
//   SDL_Event event;
//   while (SDL_PollEvent(&event)) {
//     switch (event.type) {
//       case SDL_KEYDOWN: // replaces WM_KEYDOWN
//       case SDL_MOUSEBUTTONDOWN: // replaces WM_LBUTTONDOWN
//       case SDL_MOUSEWHEEL: // replaces WM_MOUSEWHEEL
//       ...
//     }
//   }
// TODO: Modern GCC/MinGW - Windows-specific types and calling conventions:
// SDL2: Would replace entire function with SDL event loop
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message,WPARAM wParam, LPARAM lParam)
{
	// TODO: Modern GCC/MinGW - PAINTSTRUCT is Windows-specific for WM_PAINT handling
	PAINTSTRUCT ps;
	int keys;
	extern int ser_ver,xmove,xxtimer;
	static int delta=0; // scrollwheel direction/speed

	keys=0;
	// TODO: Modern GCC/MinGW - GetAsyncKeyState is Windows-specific
	// SDL2: Use SDL_GetKeyboardState() or check event.key.keysym.mod for modifiers
	// Example: if (SDL_GetModState() & KMOD_SHIFT)
	if (GetAsyncKeyState(VK_SHIFT)&0x8000) keys|=1;
	if ((GetAsyncKeyState(VK_CONTROL)&0x8000) || (GetAsyncKeyState(VK_MENU)&0x8000)) keys|=2;

	// TODO: Modern GCC/MinGW - Windows message handling
	// SDL2: Replace switch(message) with switch(event.type)
	switch (message) {
		case WM_MENUCHAR:  // TODO: SDL2: Not directly applicable, handle in menu system
            return MNC_CLOSE<<16;
		case WM_SYSKEYDOWN:  // TODO: SDL2: Use SDL_KEYDOWN with KMOD_ALT check
		case WM_KEYDOWN:  // TODO: SDL2: Use SDL_KEYDOWN, check event.key.keysym.sym
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
				// -------- CTRL + 5
				case  53: if (keys>=2) button_command(20); break;

				// **************** Ctrl cases for letter keys **************** //
				// -------- CTRL + Q
				case  81: if (keys>=2) button_command(21);break;
				// -------- CTRL + W
				case  87: if (keys>=2) button_command(22);break;
				// -------- CTRL + E
				case  69: if (keys>=2) button_command(23);break;
				// -------- CTRL + R
				case  82: if (keys>=2) button_command(24);break;
				// -------- CTRL + T
				case  84: if (keys>=2) button_command(25);break;

				// -------- CTRL + A
				case  65: if (keys>=2) button_command(26);break;
				// -------- CTRL + S
				case  83: if (keys>=2) button_command(27);break;
				// -------- CTRL + D
				case  68: if (keys>=2) button_command(28);break;
				// -------- CTRL + F
				case  70: if (keys>=2) button_command(29);break;
				// -------- CTRL + G
				case  71: if (keys>=2) button_command(30);break;

				// -------- CTRL + Z
				case  90: if (keys>=2) button_command(31);break;
				// -------- CTRL + X
				case  88: if (keys>=2) button_command(32);break;
				// -------- CTRL + C
				case  67: if (keys>=2) button_command(33);break;
				// -------- CTRL + V
				case  86: if (keys>=2) button_command(34);break;
				// -------- CTRL + B
				case  66: if (keys>=2) button_command(35);break;

				// -------- ESC (SDL: SDLK_ESCAPE)
				case  27:
					cmd(CL_CMD_RESET,0,0);
					show_shop=0;
					show_wps=0;
					show_tree=0;
					show_book=0;
					show_motd=0;
					show_newp=0;
					show_tuto=0;
					noshop=QSIZE*3;
					xmove=xxtimer=0;
					break;
				// **************** F-Key Row 1 **************** //
				// -------- F1 (SDL: SDLK_F1)
				case 112:
					cmd(CL_CMD_MODE,2,0);
					break;
				// -------- F2 (SDL: SDLK_F2)
				case 113:
					cmd(CL_CMD_MODE,1,0);
					break;
				// -------- F3 (SDL: SDLK_F3)
				case 114:
					cmd(CL_CMD_MODE,0,0);
					break;
				// -------- F4 (SDL: SDLK_F4)
				case 115:
					pdata.show_proz=1-pdata.show_proz;
					break;
				// **************** F-Key Row 2 **************** //
				// -------- F5 (SDL: SDLK_F5)
				case 116:
					pdata.show_stats=1-pdata.show_stats;
					//else do_alpha++;
					//if (do_alpha==3) do_alpha=0;
					//dd_invalidate_alpha();
					break;
				// -------- F6 (SDL: SDLK_F6)
				case 117:
					pdata.hide=1-pdata.hide;
					break;
				// -------- F7 (SDL: SDLK_F7)
				case 118:
					pdata.show_names=1-pdata.show_names;
					break;
				// -------- F8 (SDL: SDLK_F8)
				case 119:
					pdata.show_bars=1-pdata.show_bars;
					break;
				// **************** F-Key Row 3 **************** //
				// -------- F9 (SDL: SDLK_F9)
				case 120:
					dd_savescreen();
					break;
				// -------- F10 (SDL: SDLK_F10)
				case 121:
					button_command(25);
					gamma+=250;
					if (gamma>6000)	gamma=5000;
					xlog(2,"Set gamma correction to %1.2f",gamma/5000.0);
					dd_invalidate_cache();
					break;
				// -------- F11 (SDL: SDLK_F11)
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
				// -------- F12 (SDL: SDLK_F12)
				case 123:
					cmd_exit();
					break;

				// -------- Insert (SDL: SDLK_INSERT)
				case 45:
					cmd3(CL_CMD_INV,9,1,selected_char);
					break;

				// **************** Text Editor **************** //
				// -------- Tab (SDL: SDLK_TAB)
				case   9:
					complete_word();
					break;
				// -------- Backspace (SDL: SDLK_BACKSPACE)
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
				// -------- Del (SDL: SDLK_DELETE)
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
				// -------- Page Up (SDL: SDLK_PAGEUP)
				case  33:
					if (logstart<22*8)
					{
						logstart+=11;
						if (logstart > 22*8) logstart = 22*8;
						logtimer=TICKS*30/TICKMULTI;
					}
					break;
				// -------- Page Down (SDL: SDLK_PAGEDOWN)
				case  34:
					if (logstart>0)
					{
						logstart-=11;
						if (logstart < 0) logstart = 0;
						logtimer=TICKS*30/TICKMULTI;
					}
					break;
				// -------- Home (SDL: SDLK_HOME)
				case  36:
					cur_pos=0;
					tabmode=0;
					tabstart=0;
					break;
				// -------- End (SDL: SDLK_END)
				case  35:
					cur_pos=in_len;
					tabmode=0;
					tabstart=0;
					break;
				// -------- Left Arrow (SDL: SDLK_LEFT)
				case  37:
					if (cur_pos) cur_pos--;
					tabmode=0;
					tabstart=0;
					break;
				// -------- Right Arrow (SDL: SDLK_RIGHT)
				case  39:
					if (cur_pos<115) cur_pos++;
					tabmode=0;
					tabstart=0;
					break;
				// -------- Up Arrow (SDL: SDLK_UP)
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
				// -------- Down Arrow (SDL: SDLK_DOWN)
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
			}
			break;

		case WM_CHAR:  // TODO: SDL2: Use SDL_TEXTINPUT for character input (Unicode support)
			switch ((int)wParam)
			{
				/*
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
				*/

				// -------- Return Key (SDL: SDLK_RETURN)
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

		case WM_PAINT:  // TODO: SDL2: Not needed, rendering happens in main loop
			// TODO: Modern GCC/MinGW - BeginPaint/EndPaint are Windows-specific
			BeginPaint(hWnd,&ps);
			EndPaint(hWnd,&ps);
			break;

		case WM_DESTROY:  // TODO: SDL2: Handle SDL_QUIT event instead
			// TODO: Modern GCC/MinGW - PostQuitMessage is Windows-specific
			PostQuitMessage(0);
			break;

		case WM_MOUSEMOVE:  // TODO: SDL2: Use SDL_MOUSEMOTION event
			// TODO: Modern GCC/MinGW - LOWORD/HIWORD are Windows-specific macros
			// SDL2: Use event.motion.x and event.motion.y
			mouse(LOWORD(lParam),HIWORD(lParam),MS_MOVE);
			mx=LOWORD(lParam);
			my=HIWORD(lParam);
			break;

		case WM_LBUTTONDOWN:  // TODO: SDL2: Use SDL_MOUSEBUTTONDOWN with event.button.button == SDL_BUTTON_LEFT
			mouse(LOWORD(lParam),HIWORD(lParam),MS_LB_DOWN);
			break;

		case WM_LBUTTONUP:  // TODO: SDL2: Use SDL_MOUSEBUTTONUP with event.button.button == SDL_BUTTON_LEFT
			mouse(LOWORD(lParam),HIWORD(lParam),MS_LB_UP);
			break;

		case WM_RBUTTONDOWN:  // TODO: SDL2: Use SDL_MOUSEBUTTONDOWN with event.button.button == SDL_BUTTON_RIGHT
			mouse(LOWORD(lParam),HIWORD(lParam),MS_RB_DOWN);
			break;

		case WM_RBUTTONUP:  // TODO: SDL2: Use SDL_MOUSEBUTTONUP with event.button.button == SDL_BUTTON_RIGHT
			mouse(LOWORD(lParam),HIWORD(lParam),MS_RB_UP);
			break;

		//  Adding support for Scroll Wheel
		case WM_MOUSEWHEEL:  // TODO: SDL2: Use SDL_MOUSEWHEEL event with event.wheel.y
            if (!(short)(HIWORD(wParam))) return 0;

			// Fix to grab X/Y for window mode / multi screen
			// TODO: Modern GCC/MinGW - GET_X_LPARAM/GET_Y_LPARAM are Windows-specific
			// SDL2: event.wheel.mouseX and event.wheel.mouseY provide mouse position
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			// TODO: Modern GCC/MinGW - ScreenToClient is Windows-specific
			// SDL2: Not needed, coordinates are already window-relative
			ScreenToClient(hWnd, &pt);

			mx = pt.x;
			my = pt.y;

			delta+=(short)(HIWORD(wParam));

			// Scrolling Down
			while (delta<=-120) {

				//  INVENTORY
				if (mx>gui_inv_x[0] && mx<gui_inv_x[1] && my>gui_inv_y[0] && my<gui_inv_y[1])
				{
					if (inv_pos<MAXITEMS-30)
						inv_pos+=10;
				}

				//  SKILL LIST
				if (mx>gui_skl_names[RECT_X1] && mx<gui_skl_names[RECT_X2]+110 && my>gui_skl_names[RECT_Y1] && my<gui_skl_names[RECT_Y2])
				{
					if (skill_pos<MAXSKILL-10)
						skill_pos++;
				}

				//  WAYPOINT PAGE - this was defined in multiple places so one more cant hurt!
				if (show_wps) {
					if (mx>(((1280/2)-(320/2))) && mx<(((1280/2)-(320/2))+280-13) && my>(((736/2)-(320/2)+72)+1) && my<(((736/2)-(320/2)+72)+1+280))
					{
						if (wps_pos<(MAXWPS- 8))
							wps_pos++;
					}
				}

				// DEPOT PAGE
				if (show_shop==112) {
					if (mx>(((1280/2)-(320/2))) && mx<(((1280/2)-(320/2))+280-13) && my>(((736/2)-(320/2)+72)+1) && my<(((736/2)-(320/2)+72)+1+280))
					{
						if (dept_page<7)
							dept_page++;
					}
				}

				//	CHAT HISTORY - no good position placement so hardcoding
				if (mx>973 && mx<1275 && my>6 && my<230)
				{
					if (logstart>0)
					{
						logstart-=3; logtimer=TICKS*30/TICKMULTI;
					}
				}

				delta+=120;
			}

			// SCROLLING UP
			while (delta>=120) {

				//  INVENTORY
				if (mx>gui_inv_x[0] && mx<gui_inv_x[1] && my>gui_inv_y[0] && my<gui_inv_y[1])
				{
					if (inv_pos> 1)
						inv_pos-=10;
				}

				//  SKILL LIST
				if (mx>gui_skl_names[RECT_X1] && mx<gui_skl_names[RECT_X2]+110 && my>gui_skl_names[RECT_Y1] && my<gui_skl_names[RECT_Y2])
				{
					if (skill_pos>0)
						skill_pos--;
				}

				//  WAYPOINT PAGE
				if (show_wps) {
					if (mx>(((1280/2)-(320/2))) && mx<(((1280/2)-(320/2))+280-13) && my>(((736/2)-(320/2)+72)+1) && my<(((736/2)-(320/2)+72)+1+280))
					{
						if (wps_pos>0 )
							wps_pos--;
					}
				}

				// DEPOT PAGE
				if (show_shop==112) {
					if (mx>(((1280/2)-(320/2))) && mx<(((1280/2)-(320/2))+280-13) && my>(((736/2)-(320/2)+72)+1) && my<(((736/2)-(320/2)+72)+1+280))
					{
						if (dept_page>0)
							dept_page--;
					}
				}

				//  CHAT HISTORY
				if (mx>973 && mx<1275 && my>6 && my<230)
				{
					if (logstart<22*8)
					{
						logstart+=3;
						logtimer=TICKS*30/TICKMULTI;
					}
				}

				delta-=120;
			}
			break;

		default:
			// TODO: Modern GCC/MinGW - DefWindowProc is Windows-specific
			// SDL2: No equivalent needed, unhandled events are simply ignored
			return(DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}

// TODO: Modern GCC/MinGW - InitWindow is Windows-specific window creation
// SDL2: Replace entire function with SDL_CreateWindow()
// Example:
//   SDL_Window* window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
//     SDL_WINDOWPOS_CENTERED, width, height, flags);
//   SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
HWND InitWindow(HINSTANCE hInstance,int nCmdShow)
{
	WNDCLASS wc;  // TODO: SDL2: Not needed, SDL_CreateWindow handles this
	HWND hWnd;
	// TODO: Modern GCC/MinGW - GetSystemMetrics is Windows-specific
	// SDL2: Use SDL_GetDisplayBounds() to get screen dimensions
	DWORD dwWidth = GetSystemMetrics(SM_CXSCREEN);
	DWORD dwHeight = GetSystemMetrics(SM_CYSCREEN);
	char buf[256];
	int n;
	int xx, yy;
    HMENU hmenu;  // TODO: SDL2: Not applicable
    LONG style;  // TODO: SDL2: Use SDL window flags instead

	xx = yy = 10;

	hinst=hInstance;

	// TODO: Modern GCC/MinGW - LoadCursor is Windows-specific
	// SDL2: Use SDL_CreateSystemCursor() or SDL_CreateCursor()
	// Example: SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	for (n=1; n<10; n++) {
		cursor[n]=LoadCursor(hInstance,MAKEINTRESOURCE(n));
	}

	// TODO: Modern GCC/MinGW - WNDCLASS is Windows-specific window class registration
	// SDL2: Not needed, SDL handles window class internally
	wc.style=CS_HREDRAW|CS_VREDRAW;
	// TODO: Modern GCC/MinGW - FAR PASCAL are obsolete keywords
	// MinGW: Use WNDPROC type cast instead
	wc.lpfnWndProc=(WNDPROC)MainWndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	// TODO: Modern GCC/MinGW - LoadIcon is Windows-specific
	// SDL2: Use SDL_SetWindowIcon() with SDL_Surface* loaded from image
	wc.hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(1));
	wc.hCursor=NULL;
	wc.hbrBackground=NULL;
	wc.lpszMenuName=NULL;
	wc.lpszClassName="DDCWin";

	// TODO: Modern GCC/MinGW - RegisterClass is Windows-specific
	// SDL2: Not needed
	RegisterClass(&wc);

	sprintf(buf,MNAME" v%d.%02d.%02d",
			VERSION>>16,(VERSION>>8)&255,VERSION&255);

	// TODO: Modern GCC/MinGW - CreateWindowEx is Windows-specific
	// SDL2: Use SDL_CreateWindow() with appropriate flags
	if (screen_windowed == 0)
	{
		// TODO: SDL2: Use SDL_WINDOW_FULLSCREEN or SDL_WINDOW_FULLSCREEN_DESKTOP
		desk_hwnd=hWnd=CreateWindowEx(WS_EX_TOPMOST, "DDCWin", buf, WS_VISIBLE|WS_BORDER|WS_POPUP|WS_SYSMENU,
			xx, yy, 0, 0, NULL, NULL, hInstance, NULL);
	}
	else
	{
		// TODO: SDL2: Use SDL_WINDOW_SHOWN flag for windowed mode
		xx = dwWidth/2 - (screen_width+2)/2;
		yy = dwHeight/2 - (screen_height+21)/2;
		desk_hwnd=hWnd=CreateWindowEx(0, "DDCWin", buf, WS_VISIBLE|WS_BORDER|WS_SYSMENU|WS_MINIMIZEBOX,
			xx, yy, screen_width+2, screen_height+21, NULL, NULL, hInstance, NULL);

		// TODO: Modern GCC/MinGW - GetSystemMenu/DeleteMenu are Windows-specific
		// SDL2: To disable close button, handle SDL_QUIT event and ignore it
		hmenu = GetSystemMenu(hWnd,FALSE);
		DeleteMenu(hmenu,SC_CLOSE,MF_BYCOMMAND);
		// TODO: Modern GCC/MinGW - GetWindowLong/SetWindowLong are Windows-specific
		// SDL2: Not directly applicable, use SDL window flags
		style = GetWindowLong(hWnd,GWL_STYLE);
		style ^= WS_SYSMENU;
		SetWindowLong(hWnd,GWL_STYLE,style);

		hmenu = GetSystemMenu(desk_hwnd,FALSE);
		DeleteMenu(hmenu,SC_CLOSE,MF_BYCOMMAND);
		style = GetWindowLong(desk_hwnd,GWL_STYLE);
		style ^= WS_SYSMENU;
		SetWindowLong(desk_hwnd,GWL_STYLE,style);
	}

    // TODO: Modern GCC/MinGW - SetFocus is Windows-specific
	// SDL2: Use SDL_RaiseWindow() to bring window to front
    SetFocus(hWnd);
	// TODO: Modern GCC/MinGW - ShowWindow is Windows-specific
	// SDL2: Use SDL_ShowWindow() or SDL_WINDOW_SHOWN flag in SDL_CreateWindow()
	ShowWindow(hWnd,nCmdShow);
	// TODO: Modern GCC/MinGW - UpdateWindow is Windows-specific
	// SDL2: Not needed, SDL_RenderPresent() handles updates
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

	// This is required for now, when DD isn't loaded we dont set colors...
	RED=0xF800;
	GREEN=0x07E0;
	BLUE=0x001F;


	parse_cmd(lpCmdLine);

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

	// Default windowed
	screen_windowed=1;

	setres_default();

	// TODO: MinGW - Set hinst early so options() can use it before InitWindow()
	hinst = hInstance;

	load_options();
	options();  // Show options dialog BEFORE creating the game window
	init_engine();

	if (quit) exit(0);

	init_sound();

	tmp=init(screen_windowed);
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

		sprintf(buf,
				"DirectX init failed with code %d.\n"
				"DDError=%s\n"
				"Client Version %d.%02d.%02d\n"
				"MAXX=%d, MAXY=%d\n"
				"R=%04X, G=%04X, B=%04X\n"
				"RGBM=%d\n"
				"MAXCACHE=%d\n",
				-tmp,DDERR,VERSION>>16,(VERSION>>8)&255,VERSION&255,MAXX,MAXY,RED,GREEN,BLUE,RGBM,MAXCACHE);
		// TODO: Modern GCC/MinGW - MessageBox is Windows-specific
		// SDL2: Use SDL_ShowSimpleMessageBox()
		MessageBox(hwnd,buf,"DirectX init failed.",MB_ICONSTOP|MB_OK);
		exit(1);
	}
    sprintf(buf,"|R=%04X, G=%04X, B=%04X, RGBM=%d",RED,GREEN,BLUE,RGBM);
	say(buf);

    init_xalloc();
	conv_init();
	init_pnglib();
	init_input();

	if (RGBM==-1) {
		sprintf(buf,"|unknown card: R=%04X G=%04X B=%04X",RED,GREEN,BLUE);
		say(buf);
	}

	log_system_data();

	engine();

	deinit();

	if (regxy) regxy(GetCurrentProcess());

	save_options();

	cleanup_sound();

	return 0;
}