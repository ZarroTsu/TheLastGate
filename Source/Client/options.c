/*
 * OPTIONS.C - Game Options and Settings Dialog
 *
 */

#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include "common.h"
#include "inter.h"
#include "main.h"
#include "options.h"

#include "config/config.h"
#include "config/keybindings.h"

extern char history[20][128];
extern int hist_len[20];
extern char words[2048][40];
extern char passwd[15];

extern int quit;

extern int do_alpha;
extern int do_shadow;
extern int do_darkmode;

extern int screen_width, screen_height, screen_tilexoff, screen_tileyoff, screen_viewsize, view_subedges;
//extern int screen_overlay_sprite;
extern int xwalk_nx, xwalk_ny, xwalk_ex, xwalk_ey, xwalk_sx, xwalk_sy, xwalk_wx, xwalk_wy;
extern short screen_renderdist;

extern int so_status;

struct key okey;
struct pdata pdata={"","","",0};

//--------------
// option flags
//--------------

int dd_change(int x,int y);

void setres_800()
{
	screen_width=800;
	screen_height=600;
	screen_tilexoff=XPOS_800;
	screen_tileyoff=YPOS_800;
	screen_viewsize=VIEWSIZE_800;
	view_subedges=VIEW_SUBEDGES_800;
	//screen_overlay_sprite=GUI_OVERLAY_800;

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
	//screen_overlay_sprite=GUI_OVERLAY_1280;

	xwalk_nx=XWALK_NX_1280;
	xwalk_ny=XWALK_NY_1280;
	xwalk_ex=XWALK_EX_1280;
	xwalk_ey=XWALK_EY_1280;
	xwalk_sx=XWALK_SX_1280;
	xwalk_sy=XWALK_SY_1280;
	xwalk_wx=XWALK_WX_1280;
	xwalk_wy=XWALK_WY_1280;
	
	if (g_config.video.windowed == 0)
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
	//screen_overlay_sprite=GUI_OVERLAY_1600;

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
		if (read(handle,&g_config.audio.music_enabled,sizeof(int))!=sizeof(int)) flag=1; // sizeof(int) for backwards compatibility
		if (read(handle,&g_config.audio.sound_enabled,sizeof(int))!=sizeof(int)) flag=1; // sizeof(int) for backwards compatibility
		if (read(handle,&pdata,sizeof(pdata))!=sizeof(pdata)) flag=1;
		if (read(handle,&okey,sizeof(okey))!=sizeof(okey)) flag=1;
		if (read(handle,&g_config.video.gamma,sizeof(g_config.video.gamma))!=sizeof(g_config.video.gamma)) g_config.video.gamma=5000;
		if (read(handle,&do_shadow,sizeof(do_shadow))!=sizeof(do_shadow)) do_shadow=1;
		if (read(handle,&g_config.video.windowed,sizeof(g_config.video.windowed))!=sizeof(g_config.video.windowed)) g_config.video.windowed=1;
		if (read(handle,&do_darkmode,sizeof(do_darkmode))!=sizeof(do_darkmode)) do_darkmode=0;
		close(handle);
	} else flag=1;
	
	do_alpha=0;

	if (flag) {
		/* flag=1; */
		memset(history,0,sizeof(history));
		memset(hist_len,0,sizeof(hist_len));
		memset(words,0,sizeof(words));
		g_config.audio.music_enabled=0; g_config.audio.sound_enabled=1; do_alpha=0; do_shadow=1; g_config.video.windowed=1; do_darkmode=0; g_config.video.gamma=5000;
		memset(&pdata,0,sizeof(pdata));
		pdata.show_names=1;
		pdata.hide=1;
		pdata.show_stats=0;
		pdata.show_bars=0;
		for (n=0; n<20; n++) {
			pdata.xbutton[n].skill_nr=-1;
			strcpy(pdata.xbutton[n].name,"-");
		}
		memset(&okey,0,sizeof(okey));
		strcpy(okey.name,"New Account");
	}
	
	if (g_config.video.gamma>6000 || g_config.video.gamma<5000) g_config.video.gamma=5000;

	/* Load TLGExtended.dat separately */
	load_extended_options();
}

void load_extended_options(void)
{
	int handle, i;
	unsigned int version = 0, count = 0;

	// Set all defaults, keybinds are handled in init
	g_config.ui.escape_closes_menu_first = 1;
	g_config.ui.cost_helper = 0;
	g_config.ui.enter_to_talk = 0;
	g_config.audio.sound_volume = 10;
	g_config.gameplay.give_more = 0;
	g_config.gameplay.use_queue = 0;

	handle = open("TLGExtended.dat", O_RDONLY|O_BINARY);
	if (handle != -1) {
		/* Read and validate version */
		if (read(handle, &version, sizeof(version)) != sizeof(version)) {
			/* Wrong version or corrupted - use defaults, delete old file */
			close(handle);
			unlink("TLGExtended.dat");
		}
		if (version >= 1) {
			/* Read g_config.video fields */
			if (read(handle, &g_config.ui.escape_closes_menu_first, sizeof(g_config.ui.escape_closes_menu_first)) != sizeof(g_config.ui.escape_closes_menu_first)) return;
			if (read(handle, &g_config.ui.cost_helper, sizeof(g_config.ui.cost_helper)) != sizeof(g_config.ui.cost_helper)) return;
			if (read(handle, &g_config.audio.sound_volume, sizeof(g_config.audio.sound_volume)) != sizeof(g_config.audio.sound_volume)) return;
			if (read(handle, &g_config.gameplay.give_more, sizeof(g_config.gameplay.give_more)) != sizeof(g_config.gameplay.give_more)) return;
			if (read(handle, &g_config.gameplay.use_queue, sizeof(g_config.gameplay.use_queue)) != sizeof(g_config.gameplay.use_queue)) return;

			/* Read keybinding section */
			if (version < 5) { // new versions wont have this
				if (read(handle, &count, sizeof(count)) == sizeof(count)) {
					/* Bounds check */
					if (count > NUM_SPELL_HOTKEYS) count = NUM_SPELL_HOTKEYS;

					for (i = 0; i < (int)count; i++) {
						SDL_Keycode key;
						KeybindModifier mod;

						if (read(handle, &key, sizeof(SDL_Keycode)) != sizeof(SDL_Keycode)) break;
						if (read(handle, &mod, sizeof(KeybindModifier)) != sizeof(KeybindModifier)) break;

						/* Validate before applying */
						if (key == SDLK_UNKNOWN || mod > KEYBIND_MOD_ALT) continue;
					}
				}
			}
		}

		if (version >= 2 && version < 5) {
			if (read(handle, &count, sizeof(count)) == sizeof(count)) {
				if (count > NUM_GENERAL_HOTKEYS) count = NUM_GENERAL_HOTKEYS;

				for (i = 0; i < (int)count; i++) {
					SDL_Keycode key;
					KeybindModifier mod;

					if (read(handle, &key, sizeof(SDL_Keycode)) != sizeof(SDL_Keycode)) break;
					if (read(handle, &mod, sizeof(KeybindModifier)) != sizeof(KeybindModifier)) break;

					if (key == SDLK_UNKNOWN || mod > KEYBIND_MOD_ALT) continue;
				}
			}
		}

		if (version >= 3) {
			if (read(handle, &g_config.gameplay.stubborn_use, sizeof(g_config.gameplay.stubborn_use)) != sizeof(g_config.gameplay.stubborn_use)) return;
			if (read(handle, &g_config.gameplay.stubborn_drop, sizeof(g_config.gameplay.stubborn_drop)) != sizeof(g_config.gameplay.stubborn_drop)) return;
			if (read(handle, &g_config.gameplay.stubborn_give, sizeof(g_config.gameplay.stubborn_give)) != sizeof(g_config.gameplay.stubborn_give)) return;
			if (read(handle, &g_config.gameplay.stubborn_move, sizeof(g_config.gameplay.stubborn_move)) != sizeof(g_config.gameplay.stubborn_move)) return;
			if (read(handle, &g_config.gameplay.stubborn_pickup, sizeof(g_config.gameplay.stubborn_pickup)) != sizeof(g_config.gameplay.stubborn_pickup)) return;
		}

		if (version >= 4) {
			if (read(handle, &g_config.ui.enter_to_talk, sizeof(g_config.ui.enter_to_talk)) != sizeof(g_config.ui.enter_to_talk)) return;
		}

		if (version >= 5) {
			if (read(handle, &count, sizeof(count)) == sizeof(count)) {
				char id[64];
				for (i = 0; i < (int)count; i++) {
					uint32_t id_length;
					SDL_Keycode key;
					KeybindModifier mod;
					if (read(handle, &id_length, sizeof(id_length)) != sizeof(id_length)) break;
					if (id_length >= sizeof(id)) break;
					if (read(handle, &id, id_length) != ((ssize_t)id_length)) break;
					id[id_length] = '\0';
					if (read(handle, &key, sizeof(SDL_Keycode)) != sizeof(SDL_Keycode)) break;
					if (read(handle, &mod, sizeof(KeybindModifier)) != sizeof(KeybindModifier)) break;
					BindingDescriptor *binding = binding_find_by_id(id);
					if (binding) {
						binding->keybinding.key = key;
						binding->keybinding.modifier = mod;
					}

				}
			}
		}

		close(handle);
		return;
	}
}

void save_options(void)
{
	int handle;

	/* Save TLG.dat */
	handle=open("TLG.dat",O_WRONLY|O_BINARY|O_CREAT|O_TRUNC,0666);
	if (handle!=-1) {
		write(handle,history,sizeof(history));
		write(handle,hist_len,sizeof(hist_len));
		write(handle,words,sizeof(words));
		write(handle,&g_config.audio.music_enabled,sizeof(int)); // sizeof(int) for backwards compatibility
		write(handle,&g_config.audio.sound_enabled,sizeof(int)); // sizeof(int) for backwards compatibility
		write(handle,&pdata,sizeof(pdata));
		write(handle,&okey,sizeof(okey));
		write(handle,&g_config.video.gamma,sizeof(g_config.video.gamma));
		write(handle,&do_shadow,sizeof(do_shadow));
		write(handle,&g_config.video.windowed,sizeof(g_config.video.windowed));
		write(handle,&do_darkmode,sizeof(do_darkmode));
		close(handle);
	}

	/* Save TLGExtended.dat separately */
	save_extended_options();
}

void save_extended_options(void)
{
	int handle, i;
	unsigned int version, count;

	handle = open("TLGExtended.dat", O_WRONLY|O_BINARY|O_CREAT|O_TRUNC, 0666);
	if (handle != -1) {
		/* Write version header first */
		version = SAVE_VERSION;
		write(handle, &version, sizeof(version));

		/* Write existing g_config.video fields */
		write(handle, &g_config.ui.escape_closes_menu_first, sizeof(g_config.ui.escape_closes_menu_first));
		write(handle, &g_config.ui.cost_helper, sizeof(g_config.ui.cost_helper));
		write(handle, &g_config.audio.sound_volume, sizeof(g_config.audio.sound_volume));
		write(handle, &g_config.gameplay.give_more, sizeof(g_config.gameplay.give_more));
		write(handle, &g_config.gameplay.use_queue, sizeof(g_config.gameplay.use_queue));  /* FIXED */

		write(handle, &g_config.gameplay.stubborn_use, sizeof(g_config.gameplay.stubborn_use));
		write(handle, &g_config.gameplay.stubborn_drop, sizeof(g_config.gameplay.stubborn_drop));
		write(handle, &g_config.gameplay.stubborn_give, sizeof(g_config.gameplay.stubborn_give));
		write(handle, &g_config.gameplay.stubborn_move, sizeof(g_config.gameplay.stubborn_move));
		write(handle, &g_config.gameplay.stubborn_pickup, sizeof(g_config.gameplay.stubborn_pickup));

		write(handle, &g_config.ui.enter_to_talk, sizeof(g_config.ui.enter_to_talk));

		write(handle, &g_config.keybind.num_bindings, sizeof(g_config.keybind.num_bindings));

		for (i = 0; i < g_config.keybind.num_bindings; i++) {
			BindingDescriptor binding = g_config.keybind.bindings[i];

			uint32_t id_length = (uint32_t) strlen(binding.id);
			write(handle, &id_length, sizeof(id_length));
			write(handle, binding.id, id_length);
			write(handle, &binding.keybinding.key, sizeof(binding.keybinding.key));
			write(handle, &binding.keybinding.modifier, sizeof(binding.keybinding.modifier));
		}

		close(handle);
	}
}