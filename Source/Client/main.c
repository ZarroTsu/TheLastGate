#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "main.h"
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_net.h>

#include "common.h"
#include "game/game_input.h"
#include "inter.h"
#include "net/connection.h"
#include "graphics/render.h"
#include "audio/sound.h"
#include "config/config.h"
#include "config/keybindings.h"
#include "graphics/scaling.h"
#include "graphics/sdl.h"
#include "input/input.h"
#include "launcher/launcher.h"
#include "log/log.h"
#include "security/security.h"
#include "ui/imgui/imgui_wrapper.h"
#include "util/perf.h"

const SdlClientVersion CLIENT_VERSION = {3,2};

// Screen data, can be shared with other files via extern
int screen_width, screen_height, screen_tilexoff, screen_tileyoff, screen_viewsize, view_subedges;
//int screen_overlay_sprite;
int xwalk_nx, xwalk_ny, xwalk_ex, xwalk_ey, xwalk_sx, xwalk_sy, xwalk_wx, xwalk_wy;
short screen_renderdist;
int screen_target_fps = 48;  // Configurable FPS target (default 120)

SDL_Cursor* cursors[10];

void cmd(int cmd,int x,int y);

int quit=0;
GameState g_game_state = GAME_STATE_LAUNCHER;

void engine(void);

#define MWORD 2048

char input[128];
int in_len=0;
int cur_pos=0;
int hist_nr=0;
int view_pos=0;
int tabmode=0;
int tabstart=0;
int chat_mode_active=0;
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

int parse_cmd(char *s)
{
	int n;

	while (isspace(*s))	s++;
	while (*s) {
		if (*s=='-') {
			s++;
			if (tolower(*s)=='d') {
				s++;
				while (isspace(*s)) s++;
				n=0; while (n<150 && *s && !isspace(*s)) g_config.runtime.base_path[n++]=*s++;
				if (g_config.runtime.base_path[n]!='/') g_config.runtime.base_path[n++]='/';
				g_config.runtime.base_path[n]=0;
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

int main(int argc, char *argv[]) {
	log_init();
	config_init_paths();
	security_init();
	SDLNet_Init();
	load_options();
	screen_renderdist=RENDERDIST;
	setres_default();
	sound_init();
	init(g_config.video.windowed);
	perf_init();
	launcher_init();
	init_engine();
	// if (!security_try_lock()) return 0;

	g_game_state = GAME_STATE_LAUNCHER;
	while (!quit && g_game_state == GAME_STATE_LAUNCHER) {
		handle_input();

		glClear(GL_COLOR_BUFFER_BIT);
		sdl_start_scaling();
		sdl_batch_flush();
		for (int i = 0; i < input_event_count; i++) {
			imgui_process_event(&input_events[i]);
		}
		input_event_count = 0;

		imgui_new_frame(1.0f, 1.0f);
		launcher_render();
		imgui_render();
		sdl_stop_scaling();
		SDL_GL_SwapWindow(renderer.window);
		SDL_Delay(1);
	}

	launcher_shutdown();

	keybindings_init();
	game_input_init();

	engine();

	deinit();
	save_options();

	sound_shutdown();
	security_release_lock();
	config_cleanup_paths();
	SDLNet_Quit();
	return 0;
}