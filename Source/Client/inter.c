#include <windows.h>
#include <math.h>
#pragma hdrstop
#include "common.h"
#include "inter.h"

// Zarro 2020 - Define gui rectangles as arrays - easier to find and change them here (sort of)
int gui_inv_up[] 	= { 600,   5, 612,  35 };
int gui_inv_down[]	= { 600,  76, 612, 106 };

int gui_skl_up[]	= { 233, 117, 245, 152 };
int gui_skl_down[]	= { 233, 222, 245, 256 };
int gui_update[]	= { 136, 257, 185, 270 };
int gui_skl_pm[]	= { 160,   6, 184, 255 };
int gui_skl_names[]	= {   6, 118, 135, 255 };

int gui_f_col[]		= {1083,1035 };
int gui_f_row[]		= { 666, 681, 696, 596, 611, 626, 641 };

int gui_trash[]		= { 535, 111 };
int gui_coin[]		= { 433, 467, 501, 111 };

int gui_inv_x[]		= { 260, 599 };
int gui_inv_y[]		= {   5, 106 };

//					   HEAD,NECK,BODY,ARMS,BELT,CHRM,FEET,LHND,RHND,CLOK,LRNG,RRNG,CHRM2
int gui_equ_x[]		= { 738, 700, 738, 704, 738, 777, 738, 806, 670, 772, 776, 700, 801 };
int gui_equ_y[]		= {   5,  18,  39,  56,  73,  17, 107,  56,  56,  56,  94,  94,  17 };

int gui_equ_s[]		= { 670,   5 };
int gui_hud_b[]		= { 260, 181, 196, 211 };

// Back to the regular Borland defines
extern int init_done;
extern int inv_pos,skill_pos,wps_pos,hudmode,mm_magnify;
extern unsigned int look_nr,look_type;
extern unsigned char inv_block[];
extern int tile_x,tile_y,tile_type;

extern int screen_width, screen_height, screen_tilexoff, screen_tileyoff, screen_viewsize, view_subedges;
extern int xwalk_nx, xwalk_ny, xwalk_ex, xwalk_ey, xwalk_sx, xwalk_sy, xwalk_wx, xwalk_wy;
extern short screen_windowed;
extern short screen_renderdist;

extern int xoff,yoff;
extern int stat_raised[];
extern int stat_points_used;
extern int noshop;
extern int do_alpha;

void dd_invalidate_alpha(void);
int st_skill_pts_all(int st_val);

int hightlight=0;
int hightlight_sub=0;
int cursor_type=CT_NONE;
int selected_char=0;
int last_skill=-1;

int xmove=0,xxtimer=0;

void cmd(int cmd,int x,int y);
void cmd3(int cmd,int x,int y,int z);

int mouse_x=0,mouse_y=0;

int trans_button(int x,int y)
{
	int n;
	int tx,ty;

	// Scroll for Inventory
	if (	x>gui_inv_up[RECT_X1] 	&& y>gui_inv_up[RECT_Y1] 
		&&  x<gui_inv_up[RECT_X2] 	&& y<gui_inv_up[RECT_Y2]) return 12;
	if (	x>gui_inv_down[RECT_X1] && y>gui_inv_down[RECT_Y1] 
		&&  x<gui_inv_down[RECT_X2] && y<gui_inv_down[RECT_Y2]) return 13;
	
	// Scroll for Skill List
	if (	x>gui_skl_up[RECT_X1] 	&& y>gui_skl_up[RECT_Y1] 
		&&  x<gui_skl_up[RECT_X2] 	&& y<gui_skl_up[RECT_Y2]) return 14;
	if (	x>gui_skl_down[RECT_X1] && y>gui_skl_down[RECT_Y1] 
		&&  x<gui_skl_down[RECT_X2] && y<gui_skl_down[RECT_Y2]) return 15;
	
	// Fast, Normal, Slow, Health
	tx=x-gui_f_col[0];
	ty=y-gui_f_row[0];
	for (n=0; n<4; n++) 
	{
		if (tx>=0 && tx<=47 && ty>=0 && ty<=14) 
		{
			hightlight=HL_BUTTONBOX;
			hightlight_sub=n;
			cursor_type=CT_NONE;
			return n;
		}
		tx-=48;
	}

	// _, Hide, Names, _
	tx=x-gui_f_col[0];
	ty=y-gui_f_row[1];
	for (n=0; n<4; n++) 
	{
		if (tx>=0 && tx<=47 && ty>=0 && ty<=14) 
		{
			hightlight=HL_BUTTONBOX;
			hightlight_sub=n+4;
			cursor_type=CT_NONE;
			return n+4;
		}
		tx-=48;
	}

	// _, _, _, Exit
	tx=x-gui_f_col[0];
	ty=y-gui_f_row[2];
	for (n=0; n<4; n++) 
	{
		if (tx>=0 && tx<=47 && ty>=0 && ty<=14) 
		{
			hightlight=HL_BUTTONBOX;
			hightlight_sub=n+8;
			cursor_type=CT_NONE;
			return n+8;
		}
		tx-=48;
	}
	
	// First skill shortcut bar row
  	tx=x-gui_f_col[1];
	ty=y-gui_f_row[3];
	for (n=0; n<5; n++) 
	{
		if (tx>=0 && tx<=47 && ty>=0 && ty<=14) 
		{
			hightlight=HL_BUTTONBOX;
			hightlight_sub=n;
			cursor_type=CT_NONE;
			return n+16;
		}
		tx-=48;
	}
	
	// Second skill shortcut bar row
	tx=x-gui_f_col[1];
	ty=y-gui_f_row[4];
	for (n=0; n<5; n++) 
	{
		if (tx>=0 && tx<=47 && ty>=0 && ty<=14) 
		{
			hightlight=HL_BUTTONBOX;
			hightlight_sub=n+4;
			cursor_type=CT_NONE;
			return n+21;
		}
		tx-=48;
	}
	
	// Third skill shortcut bar row
	tx=x-gui_f_col[1];
	ty=y-gui_f_row[5];
	for (n=0; n<5; n++) 
	{
		if (tx>=0 && tx<=47 && ty>=0 && ty<=14) 
		{
			hightlight=HL_BUTTONBOX;
			hightlight_sub=n+8;
			cursor_type=CT_NONE;
			return n+26;
		}
		tx-=48;
	}
	
	// Fourth skill shortcut bar row
	tx=x-gui_f_col[1];
	ty=y-gui_f_row[6];
	for (n=0; n<5; n++) 
	{
		if (tx>=0 && tx<=47 && ty>=0 && ty<=14) 
		{
			hightlight=HL_BUTTONBOX;
			hightlight_sub=n+12;
			cursor_type=CT_NONE;
			return n+31;
		}
		tx-=48;
	}
	
	// New Magnify buttons for mini-map
	if ( x>135 && y>581 && x<152 && y<598 ) return 42;
	if ( x>135 && y>599 && x<152 && y<616 ) return 43;
	
	// New GUI mode buttons
	if ( x>=gui_hud_b[0] && x<=gui_hud_b[0]+66 && y>=gui_hud_b[1] && y<=gui_hud_b[1]+14 ) return 44;
	if ( x>=gui_hud_b[0] && x<=gui_hud_b[0]+66 && y>=gui_hud_b[2] && y<=gui_hud_b[2]+14 ) return 45;
	if ( x>=gui_hud_b[0] && x<=gui_hud_b[0]+66 && y>=gui_hud_b[3] && y<=gui_hud_b[3]+14 ) return 46;
	
	if ( x>=339+2 && x<=339+30 && y>=179 && y<=179+30 ) return 47;

	return -1;
}

void button_command(int nr)
{
	int sk, keys;
	
	keys=0;
	if (GetAsyncKeyState(VK_SHIFT)&0x8000) keys|=1;
	if ((GetAsyncKeyState(VK_CONTROL)&0x8000) || (GetAsyncKeyState(VK_MENU)&0x8000)) keys|=2;

	switch (nr) 
	{
		// Row 1 of F buttons
		case  0: cmd(CL_CMD_MODE,2,0); break;
		case  1: cmd(CL_CMD_MODE,1,0); break;
		case  2: cmd(CL_CMD_MODE,0,0); break;
		case  3: pdata.show_proz=1-pdata.show_proz; break;
		// Row 2 of F buttons
		case  4: pdata.show_stats=1-pdata.show_stats; break; // do_alpha=1-do_alpha; dd_invalidate_alpha(); break;
		case  5: pdata.hide=1-pdata.hide; break;
		case  6: pdata.show_names=1-pdata.show_names; break;
		case  7: pdata.show_bars=1-pdata.show_bars; break;
		// Row 3 of F buttons
		case  8: break;
		case  9: break;
		case 10: break;
		case 11: xmove=xxtimer=0; cmd_exit(); break; // exit

		// Scroll bar for inventory
		case 12: 
			if (keys)
			{
				inv_pos = 0; 
			}
			else
			{
				if (inv_pos> 1)
					inv_pos -= 10; 
			}
			break;
		case 13: 
			if (keys)
			{
				inv_pos = MAXITEMS-30; 
			}
			else
			{
				if (inv_pos<MAXITEMS-30)
					inv_pos += 10; 
			}
			break;

		// Scroll bar for skill list
		case 14: 
			if (keys)
			{
				if (skill_pos>11)	skill_pos -= 10; 
				else 				skill_pos  = 0;
			}
			else
			{
				if (skill_pos> 1)	skill_pos -= 2; 
				else				skill_pos  = 0;
			}
			break;
		case 15: 
			if (keys)
			{
				if (skill_pos<MAXSKILL-20)	skill_pos += 10; 
				else						skill_pos  = MAXSKILL-10;
			}
			else
			{
				if (skill_pos<MAXSKILL-10)	skill_pos += 2; 
				else						skill_pos  = MAXSKILL-10;
			}
			break;
		
		// Spell hotkeys
		case 16: case 17: case 18: case 19: case 20: 
		case 21: case 22: case 23: case 24: case 25: 
		case 26: case 27: case 28: case 29: case 30: 
		case 31: case 32: case 33: case 34: case 35: 
			if ((sk=pdata.xbutton[nr-16].skill_nr)!=-1) 
			{
				if (sk < 60)
				{
					cmd3(CL_CMD_SKILL,sk,selected_char,1);
				}
				else if (sk >= 200)
				{
					cmd3(CL_CMD_INV,5,sk-200,selected_char);
				}
				else if (sk >= 100)
				{
					cmd3(CL_CMD_INV,6,sk-100,selected_char);
				}
            } 
			else 
				xlog(1,"This button is unassigned. Right click on a skill/spell, then right click on the button to assign it.");
			break;
		
		// Magnification buttons for the mini-map
		case 42:
			if (keys) mm_magnify = 4;
			else if (mm_magnify<4) mm_magnify++; 
			break;
		case 43:
			if (keys) mm_magnify = 1;
			else if (mm_magnify>1) mm_magnify--; 
			break;
			
		// New GUI swap buttons
		case 44:
			if (hudmode == 0) hudmode = 3;
			else hudmode = 0;
			break;
		case 45:
			hudmode = 1;
			break;
		case 46:
			hudmode = 2;
			break;
		
		case 47:
			show_shop=0;
			show_wps =0;
			show_book=0;
			show_motd=0;
			show_newp=0;
			show_tuto=0;
			if (keys)
			{
				if (st_skill_pts_all(pl.os_tree)>0 && show_tree!=2)
					show_tree = 2;
				else
					show_tree = 0;
			}
			else
			{
				if (st_skill_pts_all(pl.tree_points)>0 && show_tree!=1)
					show_tree = 1;
				else
					show_tree = 0;
			}
			break;

		default: break;
	}
	if (skill_pos>MAXSKILL-10) skill_pos = MAXSKILL-10;
	if (skill_pos<0) skill_pos = 0;
}

void button_help(int nr)
{
	char tmp;
	char itm[8];
	
	switch (nr)
	{
		// Row 1 of F button skills
		case  0: 
			xlog(1,"F1/FAST: Makes you move faster, but uses more endurance. You will also be seen more easily."); 
			break;
		case  1: 
			xlog(1,"F2/NORMAL: Move at normal speed. Takes no endurance when not fighting.");
			break;
		case  2: 
			xlog(1,"F3/SLOW: Makes you move slower. You regain endurance while not fighting. Decreases chances of being seen.");
			break;
		case  3: 
			xlog(1,"F4/HEALTH: Toggle whether the current health of all characters is displayed as a percent.");
			break;
		
		// Row 2 of F buttons
		case  4: 
			xlog(1,"F5/STATS: Toggle seeing stat bases on the skill list, displayed as blue numbers.");
			break;
		case  5: 
			xlog(1,"F6/HIDE: Toggle whether walls and other high objects are displayed.");
			break;
		case  6: 
			xlog(1,"F7/NAMES: Toggle whether the name of all characters is displayed.");
			break;
		case  7: 
			xlog(1,"F8/BARS: Toggle whether the current health of all characters is displayed as bars.");
			break;
		
		// Row 3 of F buttons
		case  8: break;
		case  9: break;
		case 10: break;
		case 11: 
			xlog(1,"F12/EXIT: Leave the game immediately. Will cost you 50%% health and can result in death."); 
			break;
		
		// Scroll bar for inventory
		case 12: xlog(1,"Scroll inventory contents up."); break;
		case 13: xlog(1,"Scroll inventory contents down."); break;
		
		// Scroll bar for skill list
		case 14: xlog(1,"Scroll skill list up."); break;
		case 15: xlog(1,"Scroll skill list down."); break;
		
		// Spell hotkeys
		case 16: case 17: case 18: case 19:	case 20: 
		case 21: case 22: case 23: case 24: case 25: 
		case 26: case 27: case 28: case 29: case 30: 
		case 31: case 32: case 33: case 34: case 35:
			if (last_skill > -1 && (last_skill < 60 || last_skill > 64)) 
			{
				switch (nr)
				{
					case 16: tmp = '1'; break; case 17: tmp = '2'; break; case 18: tmp = '3'; break; case 19: tmp = '4'; break; case 20: tmp = '5'; break; 
					case 21: tmp = 'Q'; break; case 22: tmp = 'W'; break; case 23: tmp = 'E'; break; case 24: tmp = 'R'; break; case 25: tmp = 'T'; break; 
					case 26: tmp = 'A'; break; case 27: tmp = 'S'; break; case 28: tmp = 'D'; break; case 29: tmp = 'F'; break; case 30: tmp = 'G'; break; 
					case 31: tmp = 'Z'; break; case 32: tmp = 'X'; break; case 33: tmp = 'C'; break; case 34: tmp = 'V'; break; case 35: tmp = 'B'; break;
					default: tmp = '?'; break;
				}
				
				// Standard shortcut sets
				if (last_skill < 60)
				{
					if (skilltab[last_skill].nr==pdata.xbutton[nr-16].skill_nr) 
					{
						pdata.xbutton[nr-16].skill_nr=-1;
						strcpy(pdata.xbutton[nr-16].name,"-");
						xlog(1,"CTRL-%c (or ALT-%c), now unassigned.", tmp, tmp);
					} 
					else 
					{
						int pl_flags, pl_flagb;
						pl_flags = pl.worn[WN_FLAGS];
						pl_flagb = pl.worn_p[WN_FLAGS];
						if (	(last_skill==11&&(pl_flagb & (1 << 10))) ||	// Magic Shield -> Magic Shell
								(last_skill==19&&(pl_flags & (1 <<  5))) ||	// Slow -> Greater Slow
								(last_skill==20&&(pl_flags & (1 <<  6))) ||	// Curse -> Greater Curse
								(last_skill==26&&(pl_flags & (1 << 14))) ||	// Heal -> Regen
								(last_skill==41&&(pl_flags & (1 << 10))) )	// Weaken -> Greater Weaken
						{
							pdata.xbutton[nr-16].skill_nr=skilltab[last_skill].nr;
							xlog(1,"CTRL-%c (or ALT-%c), now %s.", tmp, tmp, skilltab[last_skill].alt_a);
							strncpy(pdata.xbutton[nr-16].name,skilltab[last_skill].name,7);
							pdata.xbutton[nr-16].name[7]=0;
						}
						else
						{
							pdata.xbutton[nr-16].skill_nr=skilltab[last_skill].nr;
							xlog(1,"CTRL-%c (or ALT-%c), now %s.", tmp, tmp, skilltab[last_skill].name);
							strncpy(pdata.xbutton[nr-16].name,skilltab[last_skill].name,7);
							pdata.xbutton[nr-16].name[7]=0;
						}
					}
				}
				// Equipment shortcuts
				else if (last_skill >= 200)
				{
					char gearname[20][8] = {
						"*Helmet",	"*Neckla",	"*Armor",	"*Gloves",	"*Belt",
						"*Tarot1",	"*Boots",	"*Offhan",	"*Weapon",	"*Cloak",
						"*L-Ring",	"*R-Ring",	"*Tarot2",	"?",		"?", 
						"?", 		"?",		"?",		"?",		"?"
					};
					
					if (pdata.xbutton[nr-16].skill_nr==last_skill)
					{
						pdata.xbutton[nr-16].skill_nr=-1;
						strcpy(pdata.xbutton[nr-16].name,"-");
						xlog(1,"CTRL-%c (or ALT-%c), now unassigned.", tmp, tmp);
					}
					else
					{
						pdata.xbutton[nr-16].skill_nr=last_skill;
						sprintf(itm,"%s",gearname[last_skill-200]);
						xlog(1,"CTRL-%c (or ALT-%c), now %s.", tmp, tmp, itm);
						strncpy(pdata.xbutton[nr-16].name,itm,7);
						pdata.xbutton[nr-16].name[7]=0;
					}
				}
				// Inventory shortcuts
				else if (last_skill >= 100)
				{
					if (pdata.xbutton[nr-16].skill_nr==last_skill)
					{
						pdata.xbutton[nr-16].skill_nr=-1;
						strcpy(pdata.xbutton[nr-16].name,"-");
						xlog(1,"CTRL-%c (or ALT-%c), now unassigned.", tmp, tmp);
					}
					else
					{
						pdata.xbutton[nr-16].skill_nr=last_skill;
						sprintf(itm,"Item %d",last_skill-100);
						xlog(1,"CTRL-%c (or ALT-%c), now %s.", tmp, tmp, itm);
						strncpy(pdata.xbutton[nr-16].name,itm,7);
						pdata.xbutton[nr-16].name[7]=0;
					}
				}
			}
			else
				xlog(1,"Right click on a skill/spell first to assign it to a button.");
			break;
		
		// Magnification buttons for the mini-map
		case 42:
			xlog(1,"Increase minimap magnification.");
			break;
		case 43:
			xlog(1,"Decrease minimap magnification.");
			break;
		
		// New GUI swap buttons
		case 44:
			xlog(1,"Makes the left panel display skill scores and experience points (default).");
			break;
		case 45:
			xlog(1,"Makes the left panel display Offense-related statistics, such as attack damage and critical hit rate.");
			break;
		case 46:
			xlog(1,"Makes the left panel display Defense-related statistics, such as eHP and regeneration rates.");
			break;

		default: break;
	}
}

void reset_block(void)
{
	int n;

	if (pl.citem) {
		if (pl.citem_p&PL_HEAD) inv_block[WN_HEAD]=0;
		else inv_block[WN_HEAD]=1;
		if (pl.citem_p&PL_NECK) inv_block[WN_NECK]=0;
		else inv_block[WN_NECK]=1;
		if (pl.citem_p&PL_BODY) inv_block[WN_BODY]=0;
		else inv_block[WN_BODY]=1;
		if (pl.citem_p&PL_ARMS) inv_block[WN_ARMS]=0;
		else inv_block[WN_ARMS]=1;
		if (pl.citem_p&PL_BELT) inv_block[WN_BELT]=0;
		else inv_block[WN_BELT]=1;
		if (pl.citem_p&PL_CHARM) inv_block[WN_CHARM]=inv_block[WN_CHARM2]=0;
		else inv_block[WN_CHARM]=inv_block[WN_CHARM2]=1;
		if (pl.citem_p&PL_FEET) inv_block[WN_FEET]=0;
		else inv_block[WN_FEET]=1;
		if (pl.citem_p&PL_WEAPON) inv_block[WN_RHAND]=0;
		else inv_block[WN_RHAND]=1;
		if (pl.citem_p&PL_SHIELD) inv_block[WN_LHAND]=0;
		else inv_block[WN_LHAND]=1;
		if (pl.citem_p&PL_CLOAK) inv_block[WN_CLOAK]=0;
		else inv_block[WN_CLOAK]=1;
		if (pl.citem_p&PL_RING) inv_block[WN_LRING]=inv_block[WN_RRING]=0;
		else inv_block[WN_LRING]=inv_block[WN_RRING]=1;
	} 
	else 
	{
		for (n=0; n<20; n++) inv_block[n]=0;
	}
	if (pl.worn_p[WN_RHAND]&PL_TWOHAND) inv_block[WN_LHAND]=1;
	if (pl.worn_p[WN_LHAND]&&(pl.citem_p&PL_WEAPON)&&(pl.citem_p&PL_TWOHAND)) inv_block[WN_RHAND]=1;
	if (pl.worn_p[WN_LRING]&PL_TWOHAND) inv_block[WN_RRING]=1;
	if (pl.worn_p[WN_RRING]&PL_TWOHAND) inv_block[WN_LRING]=1;
}

int mouse_inventory(int x,int y,int mode)
{
	int nr,keys;
	int tx,ty;
	int n;
	static int firstrclick = 0;

	keys=0;
	if (GetAsyncKeyState(VK_SHIFT)&0x8000) keys|=1;
	if (GetAsyncKeyState(VK_CONTROL)&0x8000) keys|=2;
	if (GetAsyncKeyState(VK_MENU)&0x8000) keys|=4;
	
	// money - 10,000.00
	if (x>gui_coin[0] && x<gui_coin[0]+34 && y>gui_coin[3] && y<gui_coin[3]+34) 
	{
		if (mode==MS_LB_UP) cmd3(CL_CMD_INV,2,1000000,selected_char);
		if (mode==MS_RB_UP) xlog(1,"10,000 gold coins.");
		return 1;
	}
	
	// money -  1,000.00
	if (x>gui_coin[1] && x<gui_coin[1]+34 && y>gui_coin[3] && y<gui_coin[3]+34) 
	{
		if (mode==MS_LB_UP) cmd3(CL_CMD_INV,2,100000,selected_char);
		if (mode==MS_RB_UP) xlog(1,"1,000 gold coins.");
		return 1;
	}
	
	// money -    100.00
	if (x>gui_coin[2] && x<gui_coin[2]+34 && y>gui_coin[3] && y<gui_coin[3]+34) 
	{
		if (mode==MS_LB_UP) cmd3(CL_CMD_INV,2,10000,selected_char);
		if (mode==MS_RB_UP) xlog(1,"100 gold coins.");
		return 1;
	}
	
	// trashbin
	if (x>gui_trash[0] && x<gui_trash[0]+34 && y>gui_trash[1] && y<gui_trash[1]+34) 
	{
		if (mode==MS_LB_UP) cmd3(CL_CMD_INV,9,0,selected_char);
		if (mode==MS_RB_UP) xlog(1,"Dispose of items under your cursor here.");
		return 1;
	}
	
	// backpack
	if (x>gui_inv_x[0] && x<gui_inv_x[1] && y>gui_inv_y[0] && y<gui_inv_y[1]) 
	{
		tx=(x-gui_inv_x[0])/34;
		ty=(y-gui_inv_y[0])/34;

		nr=tx+ty*10;
		if (keys>=2)
		{
			if (mode==MS_LB_UP)
			{
				if (show_shop && show_shop != 110 && show_shop != 111)
				{	// Sell item from inventory
					cmd3(CL_CMD_QSHOP,shop.nr,nr+inv_pos,dept_page);
				}
				else
				{	// Push or pull item stacks
					cmd3(CL_CMD_INV,3,nr+inv_pos,selected_char); 
				}
			}
			else if (mode==MS_RB_UP)
			{
				if (last_skill >= 60 && last_skill <= 64)
				{
					xlog(6,"Details panel now showing default.");
				}
				// Lock item where it is ;  TODO: fix /sort server-side before uncommenting
				//cmd3(CL_CMD_INV,4,nr+inv_pos,selected_char);
				//pl.item_l[nr+inv_pos] = 1-pl.item_l[nr+inv_pos];
				if (last_skill == 100+nr+inv_pos)
				{
					last_skill = -1;
					xlog(6,"Inventory slot %d no longer selected for shortcut.",nr+inv_pos);
				}
				else
				{
					last_skill = 100+nr+inv_pos;
					if (firstrclick)
					{
						xlog(6,"Inventory slot %d selected for shortcut.",nr+inv_pos);
					}
					else
					{
						firstrclick++;
						xlog(6,"Inventory slot %d selected for shortcut. Right-click on one of the shortcut keys in the bottom right to set a shortcut.",nr+inv_pos);
					}
				}
			}
			if (pl.item[nr+inv_pos]) 
			{
				if (pl.citem) cursor_type=CT_NONE;
				else cursor_type=CT_TAKE;
			} else 
			{
				if (pl.citem) cursor_type=CT_DROP;
				else cursor_type=CT_NONE;
			}
		}
		else if (keys==1) 
		{
			if (mode==MS_LB_UP) cmd3(CL_CMD_INV,0,nr+inv_pos,selected_char);
			else if (mode==MS_RB_UP) cmd3(CL_CMD_INV_LOOK,nr+inv_pos,0,selected_char);
			if (pl.item[nr+inv_pos]) 
			{
				if (pl.citem) cursor_type=CT_SWAP;
				else cursor_type=CT_TAKE;
			} else 
			{
				if (pl.citem) cursor_type=CT_DROP;
				else cursor_type=CT_NONE;
			}
		}
		else if (keys==0) 
		{
			if (mode==MS_LB_UP) cmd3(CL_CMD_INV,6,nr+inv_pos,selected_char);
			else if (mode==MS_RB_UP) cmd3(CL_CMD_INV_LOOK,nr+inv_pos,0,selected_char);
			if (pl.item[nr+inv_pos]) cursor_type=CT_USE;
			else cursor_type=CT_NONE;
		} 
		else 
			cursor_type=CT_NONE;
		hightlight=HL_BACKPACK;
		hightlight_sub=nr+inv_pos;
		return 1;
	}
	
	// worn
	for (n = 0; n < 13; n++)
	{
		if (x>gui_equ_x[ n ] && x<gui_equ_x[ n ]+33 && y>gui_equ_y[ n ] && y<gui_equ_y[ n ]+33) 
		{
			if (keys>=2)
			{
				if (mode==MS_RB_UP)
				{
					char gearname[20][8] = {
						"*Helmet",	"*Neckla",	"*Armor",	"*Gloves",	"*Belt",
						"*Tarot1",	"*Boots",	"*Offhan",	"*Weapon",	"*Cloak",
						"*L-Ring",	"*R-Ring",	"*Tarot2",	"?",		"?", 
						"?", 		"?",		"?",		"?",		"?"
					};
					
					if (last_skill >= 60 && last_skill <= 64)
					{
						xlog(6,"Details panel now showing default.");
					}
					if (last_skill == 200+n)
					{
						last_skill = -1;
						xlog(6,"%s equipment slot no longer selected for shortcut.", gearname[n]);
					}
					else
					{
						last_skill = 200+n;
						if (firstrclick)
						{
							xlog(6,"%s equipment slot selected for shortcut.", gearname[n]);
						}
						else
						{
							firstrclick++;
							xlog(6,"%s equipment slot selected for shortcut. Right-click on one of the shortcut keys in the bottom right to set a shortcut.", gearname[n]);
						}
					}
				}
			}
			else if (keys==1) 
			{
				if 			(mode==MS_LB_UP) cmd3(CL_CMD_INV,1, n ,selected_char);
				else if 	(mode==MS_RB_UP) cmd3(CL_CMD_INV,7, n ,selected_char);
				if (pl.worn[ n ]) {	if (pl.citem) cursor_type=CT_SWAP; else cursor_type=CT_TAKE; } 
				else { 					if (pl.citem) cursor_type=CT_DROP; else cursor_type=CT_NONE; }
			} 
			else if (keys==0) 
			{
				if 			(mode==MS_LB_UP) cmd3(CL_CMD_INV,5, n ,selected_char);
				else if 	(mode==MS_RB_UP) cmd3(CL_CMD_INV,7, n ,selected_char);
				if (pl.worn[ n ]) cursor_type=CT_USE; else cursor_type=CT_NONE;
			} 
			else { cursor_type=CT_NONE; }
			hightlight=HL_EQUIPMENT;
			hightlight_sub= n ;
			return 1;
		}
	}
	
	// swap button
	if (x>gui_equ_s[0] && x<gui_equ_s[0]+17 && y>gui_equ_s[1] && y<gui_equ_s[1]+17) 
	{
		if (mode==MS_LB_UP) cmd3(CL_CMD_INV,9,1,selected_char);
		if (mode==MS_RB_UP) xlog(1,"Swap equipment with a stored alternate set.");
		return 1;
	}
	
	return 0;
}

int mouse_buttonbox(int x,int y,int state)
{
	int nr;

	nr=trans_button(x,y);
	if (nr==-1) return 0;

	if (state==MS_LB_UP) button_command(nr);
	if (state==MS_RB_UP) button_help(nr);

	return 1;
}

int _mouse_statbox(int x,int y,int state)
{
	int n,m;

	/* ***
	if (screen_windowed == 1) {
        y-=3;
    }
	else
	{
		y-=1;
	}
	*/

	// Update Button
	if (	x>gui_update[RECT_X1] 	&& y>gui_update[RECT_Y1] 
		&&  x<gui_update[RECT_X2] 	&& y<gui_update[RECT_Y2] && hudmode!=1 && hudmode!=2)
	{
		hightlight=HL_STATBOX;
		hightlight_sub=0;
		if (state==MS_RB_UP) xlog(1,"Make the changes permanent");
		if (state!=MS_LB_UP) return 1;

		stat_points_used=0;

		for (n=0; n<108; n++) 
		{
			if (stat_raised[n]) 
			{
				if (n>7) 
				{
					m=skilltab[n-8].nr+8;
				} 
				else 
					m=n;
				cmd(CL_CMD_STAT,m,stat_raised[n]);
			}
			stat_raised[n]=0;
		}
		return 1;
	}

	if (x<gui_skl_pm[RECT_X1]) return 0;
	if (x>gui_skl_pm[RECT_X2]) return 0;
	if (y<gui_skl_pm[RECT_Y1]) return 0;
	if (y>gui_skl_pm[RECT_Y2]) return 0;

	n=(y-2)/14;

	hightlight=HL_STATBOX;
	hightlight_sub=n;

	if (x<172) { // raise
		if (state==MS_RB_UP) {
			if (n<5 && hudmode==0) xlog(1,"Raise %s.",at_name[n]);
			else if (n==5 && hudmode==0) xlog(1,"Raise Hitpoints.");
			else if (n==6 && hudmode==0) xlog(1,"Raise Mana."); 	// xlog(1,"Raise Endurance.");
			else if (n==7) return 1; 				// xlog(1,"Raise Mana.");
			else if (n>=8 && hudmode!=1 && hudmode!=2) xlog(1,"Raise %s.",skilltab[n-8+skill_pos].name);
			return 1;
		}
		if (state!=MS_LB_UP) return 1;

		if (n<5 && hudmode==0) {
			if (attrib_needed(n,pl.attrib[n][0]+stat_raised[n])>pl.points-stat_points_used) return 1;
			stat_points_used+=attrib_needed(n,pl.attrib[n][0]+stat_raised[n]);
			stat_raised[n]++;
			return 1;
		} else if (n==5 && hudmode==0) {
			if (hp_needed(pl.hp[0]+stat_raised[n])>pl.points-stat_points_used) return 1;
			stat_points_used+=hp_needed(pl.hp[0]+stat_raised[n]);
			stat_raised[n]++;
			return 1;
		} 
		else if (n==6 && hudmode==0) 
		{
			if (mana_needed(pl.mana[0]+stat_raised[n+1])>pl.points-stat_points_used) return 1;
			stat_points_used+=mana_needed(pl.mana[0]+stat_raised[n+1]);
			stat_raised[n+1]++;
			return 1;
		} 
		else if (n==7) return 1;
		else if (n>=8 && hudmode!=1 && hudmode!=2) {
			m=skilltab[n-8+skill_pos].nr;
			if (skill_needed(m,pl.skill[m][0]+stat_raised[n+skill_pos])>pl.points-stat_points_used) return 1;
			stat_points_used+=skill_needed(m,pl.skill[m][0]+stat_raised[n+skill_pos]);
			stat_raised[n+skill_pos]++;
			return 1;
		}
	} 
	else 
	{ // lower
		if (state==MS_RB_UP) {
			if (n<5 && hudmode==0) xlog(1,"Lower %s.",at_name[n]);
			else if (n==5 && hudmode==0) xlog(1,"Lower Hitpoints.");
			else if (n==6 && hudmode==0) xlog(1,"Lower Mana."); 	// xlog(1,"Lower Endurance.");
			else if (n==7) return 1; 				// xlog(1,"Lower Mana.");
			else if (n>=8 && hudmode!=1 && hudmode!=2) xlog(1,"Lower %s.",skilltab[n-8+skill_pos].name);
			return 1;
		}
		if (state!=MS_LB_UP) return 1;

		if (n<5 && hudmode==0) {
			if (!stat_raised[n]) return 1;
			stat_raised[n]--;
			stat_points_used-=attrib_needed(n,pl.attrib[n][0]+stat_raised[n]);
			return 1;
		} else if (n==5 && hudmode==0) {
			if (!stat_raised[n]) return 1;
			stat_raised[n]--;
			stat_points_used-=hp_needed(pl.hp[0]+stat_raised[n]);
			return 1;
		} 
		else if (n==6 && hudmode==0) 
		{
			if (!stat_raised[n+1]) return 1;
			stat_raised[n+1]--;
			stat_points_used-=mana_needed(pl.mana[0]+stat_raised[n+1]);
			return 1;
		} 
		else if (n==7) return 1;
		else if (n>=8 && hudmode!=1 && hudmode!=2) {
			if (!stat_raised[n+skill_pos]) return 1;
			m=skilltab[n-8+skill_pos].nr;
			stat_raised[n+skill_pos]--;
			stat_points_used-=skill_needed(m,pl.skill[m][0]+stat_raised[n+skill_pos]);
			return 1;
		}
	}
}

int mouse_statbox(int x,int y,int state)
{
	int n,m,keys,ret=0;

	keys=0;
	if (GetAsyncKeyState(VK_SHIFT)&0x8000) keys|=1;
	if ((GetAsyncKeyState(VK_CONTROL)&0x8000) || (GetAsyncKeyState(VK_MENU)&0x8000)) keys|=2;

	if (state==MS_LB_UP) 
	{
		if (keys&2) m=90;
		else if (keys&1) m=10;
		else m=1;
	} 
	else 
		m=1;
	
	if (hudmode==0 || hudmode==3)
	{
		for (n=0; n<m; n++) ret=_mouse_statbox(x,y,state);
	}

	return ret;
}

extern int pl_dmgbn, pl_reflc, pl_aoebn, pl_flags, pl_flagb, pl_flagc, pl_dmgrd;

void meta_stat_descs(int n)
{
	if (n<7)					// Topmost standard stats
	{
		switch (n)
		{
			case  0: xlog(1,"Cooldown Duration is the multiplier that cooldown from skills is applied with, the lower the better. Affected by Cooldown Rate."); break;
			case  1: xlog(1,"Spell Aptitude is how powerful a spell you can receive from any source. Determined by Willpower, Intuition, and base class Spell Modifier."); break;
			case  2: if (pl_flagc&(1<<10)) xlog(1,"Skill Modifier is a multiplier which effects the strength of skills you use.");
					 else xlog(1,"Spell Modifier is a multiplier which effects the strength of spells you cast. Determined by your character class."); break;
			case  3: xlog(1,"Base Action speed is the base speed at which ALL actions are performed. Determined by Agility and Strength."); break;
			case  4: xlog(1,"Movement Speed is the speed at which your character runs around Astonia."); break;
			case  5: xlog(1,"Your Hit Score is the value used to determine the rate of hitting enemies in melee combat. Granted by your weapon skill and other sources."); break;
			case  6: xlog(1,"Your Parry Score is the value used to determine the rate of avoiding damage from enemies. Granted by your weapon skill and other sources."); break;
			default: break;
		}
	}
	else if (hudmode==1)		// Offense Stats
	{
		n-=7;
		switch (n+skill_pos)
		{
			case  1: if (pl_dmgbn!=10000)
					 xlog(1,"Damage Multiplier is the final multiplier for all damage you deal."); break;
			case  2: xlog(1,"Melee DPS is the average of your damage per hit, times your attack speed. Does not account for bonus damage from your Hit Score."); break;
			case  3: xlog(1,"Melee Hit Dmg ranges from 1/4 of your Weapon Value, to 1/4 of (your Weapon Value, plus half Strength, plus 14) times your Crit Chance & Crit Multi."); break;
			case  4: xlog(1,"Critical Multiplier is the damage multiplier upon dealing a successful critical hit."); break;
			case  5: xlog(1,"Critical Chance is the chance, out of 100.00, that you will inflict a melee critical hit. Determined by your equipped weapon, and increased by Braveness and other sources of Crit Chance."); break;
			case  6: xlog(1,"Melee Ceiling Damage is the highest possible damage a melee hit may deal. This is affected by increases to Top Damage and your critical hit scores."); break;
			case  7: xlog(1,"Melee Floor Damage is the lowest possible damage a melee hit may deal. Determined by 1/4 of your Weapon Value."); break;
			case  8: xlog(1,"Attack speed is the speed at which melee attacks are performed. This is increased by Agility and other sources of Attack Speed."); break;
			case  9: xlog(1,"Cast Speed is the speed at which casting and action animations occur per second. This is increased by Willpower and other sources of Cast Speed."); break;
			case 10: if (pl_reflc>0)
					 xlog(1,"Thorns is damage dealt to attackers when you are successfully hit (even if you take no damage). Does not damage attackers if they fail to hit you."); break;
			case 11: if (pl.skill[34][0])
					 xlog(1,"Mana Cost Multiplier is the multiplier of mana for spells, determined by your Concentrate skill."); break;
			case 12: if (pl_aoebn)
					 xlog(1,"Total AoE Bonus is a flat increase to area-of-effect skills."); break;
			//
			case 17: if (pl.skill[40][0])
					 xlog(1,"Damage dealt by your Cleave skill, before reduction from target Parry Score and Armor Value. Surrounding targets take 3/4 of this value."); break;
			case 18: if (pl.skill[40][0] && !(pl_flags&(1<<8)))
					 xlog(1,"Effective damage over time dealt by Bleeding caused by Cleave, before reduction from target Parry Score, Armor Value and Immunity."); break;
			case 19: if (pl.skill[40][0])
					 xlog(1,"Skill exhaustion duration expected upon using your Cleave skill."); break;
			case 20: if (pl.skill[49][0])
					 xlog(1,"Damage dealt by your Leap skill if your target is at or near maximum hitpoints."); break;
			case 21: if (pl.skill[49][0])
					 xlog(1,"Damage dealt by your Leap skill, before reduction from target Parry Score and Armor Value. Surrounding targets take 3/4 of this value."); break;
			case 22: if (pl.skill[49][0])
					 xlog(1,"Skill exhaustion duration expected upon using your Leap skill."); break;
			case 23: if (pl.skill[22][0] && !IS_SHIFTED)
					 xlog(1,"Effective increase to top damage granted while under the effect of your Rage skill."); break;
			case 24: if (pl.skill[22][0] && !IS_SHIFTED)
					 xlog(1,"Effective multiplier to damage over time granted while under the effect of your Rage skill."); break;
			case 25: if (pl.skill[24][0]) 
					 xlog(1,"Damage dealt by your Blast spell, before reduction from target Immunity and Armor Value. Surrounding targets take 3/4 of this value."); break;
			case 26: if (pl.skill[24][0])
					 xlog(1,"Skill exhaustion duration expected upon using your Blast spell."); break;
			case 27: if (pl.skill[15][0]) 
					 xlog(1,"Effective penetration of target Immunity and Resistance when casting debuffs."); break;
			case 28: if (pl.skill[42][0]) {
					 if (pl_flagb&(1<<14)) xlog(1,"Effective damage over time dealt by your Poison spell, before reduction from target Immunity.");
					 else xlog(1,"Effective damage over time dealt by each individual stack of your Venom spell, before reduction from target Immunity. This can be stacked up to three times."); } break;
			case 29: if (pl.skill[42][0])
					 xlog(1,"Skill exhaustion duration expected upon using your %s spell.", (pl_flagb&(1<<14))?"Venom":"Poison"); break;
			case 30: if (pl.skill[43][0]) { 
					 if (pl_flagb&(1<<6)) { xlog(1,"Healing caused by the Pulse spell to surrounding allies when pulsing."); }
					 else                 { xlog(1,"Damage dealt by the Pulse spell to surrounding enemies when pulsing, before reduction from target Immunity and Armor Value."); } } break;
			case 31: if (pl.skill[43][0])
					 xlog(1,"Number of pulses expected during the duration of your Pulse spell, determined by the rate of pulses from Cooldown Rate."); break;
			case 32: if (pl.skill[43][0])
					 xlog(1,"Skill exhaustion duration expected upon using your Pulse spell."); break;
			case 33: if (pl.skill[ 7][0])
					 xlog(1,"Damage granted by your Zephyr spell, before reduction from target Parry Score and Armor Value. This occurs one second after a successful hit."); break;
			case 34: if (pl_flagc&(1<<13))
					 xlog(1,"Effective damage over time dealt to nearby enemies while affected by Immolate, before reduction from enemy Immunity."); break;
			case 36: if (pl.skill[27][0])
					 xlog(1,"Effective power of your Ghost Companion, granted by your Ghost Companion spell. A higher number grants a stronger companion."); break;
			case 37: if (pl.skill[27][0])
					 xlog(1,"Skill exhaustion duration expected upon using your Ghost Companion spell."); break;
			case 38: if (pl.skill[46][0])
					 xlog(1,"Effective power of your Shadow Copy, granted by your Shadow Copy spell. A higher number grants a stronger companion."); break;
			case 39: if (pl.skill[46][0])
					 xlog(1,"Effective duration of your Shadow Copy, granted by your Shadow Copy spell."); break;
			case 40: if (pl.skill[46][0])
					 xlog(1,"Skill exhaustion duration expected upon using your Shadow Copy spell."); break;
			//
			default: break;
		}
	}
	else						// Defense Stats
	{
		n-=7;
		switch (n+skill_pos)
		{
			case  1: if (pl_dmgrd!=10000)
					 xlog(1,"Damage Reduction is the final multiplier for all damage you take."); break;
			case  2: if (pl_dmgrd!=10000||(pl_flagc&(1<<9|1<<11|1<<12|1<<14))||(pl_flags&(1<<9)))
					 xlog(1,"Your effective total hitpoints, taken by dividing your maximum hitpoints by your damage reduction multiplier, as well as any effects which nullify damage or transfer it to endurance or mana."); break;
			case  3: xlog(1,"Rate at which health is regenerated per second. This is improved by the Regenerate skill, and can be further adjusted by various items."); break;
			case  4: xlog(1,"Rate at which endurance is regenerated per second. This is improved by the Rest skill, and can be further adjusted by various items."); break;
			case  5: xlog(1,"Rate at which mana is regenerated per second. This is improved by the Meditate skill, and can be further adjusted by various items."); break;
			case  6: xlog(1,"Estimated Immunity score. This displays your 'true' Immunity value after adjustments that do not display on the skill list."); break;
			case  7: xlog(1,"Estimated Resistance score. This displays your 'true' Resistance value after adjustments that do not display on the skill list.");  break;
			case  8: xlog(1,"Attack speed is the speed at which melee attacks are performed. This is increased by Agility and other sources of Attack Speed."); break;
			case  9: xlog(1,"Cast Speed is the speed at which casting and action animations occur per second. This is increased by Willpower and other sources of Cast Speed."); break;
			case 10: if (pl_reflc>0)
					 xlog(1,"Thorns is damage dealt to attackers when you are successfully hit (even if you take no damage). Does not damage attackers if they fail to hit you."); break;
			case 11: if (pl.skill[34][0])
					 xlog(1,"Mana Cost Multiplier is the multiplier of mana for spells, determined by your Concentrate skill."); break;
			case 12: if (pl_aoebn)
					 xlog(1,"Total AoE Bonus is a flat increase to area-of-effect skills."); break;
			case 13: xlog(1,"Aptitude Bonus granted to target allies when casting friendly spells. This is granted by Willpower."); break;
			case 14: xlog(1,"Rate at which health is lost while underwater. This can be reduced by the Metabolism skill, and can be further reduced by other items."); break;
			//
			case 17: if (pl.skill[21][0])
					 xlog(1,"Estimated increase to attributes granted by your Bless spell."); break;
			case 18: if (pl.skill[18][0])
					 xlog(1,"Effective increase to Weapon Value granted by your Enhance spell."); break;
			case 19: if (pl.skill[17][0])
					 xlog(1,"Effective increase to Armor Value granted by your Protect spell."); break;
			case 20: if (pl.skill[11][0])
					 xlog(1,"Effective increase to %s granted by your Magic %s spell. Decreases as you %s.", (pl_flagb&(1<<10))?"Resistance and Immunity Scores,":"Armor Value", (pl_flagb&(1<<10))?"Shell":"Shield", (pl_flagb&(1<<10))?"take or avoid debuffs":"take damage"); break;
			case 21: if (pl.skill[11][0])
					 xlog(1,"Estimated duration of your Magic %s, not including reductions from %s.", (pl_flagb&(1<<10))?"Shell":"Shield", (pl_flagb&(1<<10))?"taking or avoid debuffs":"taking damage"); break;
			case 22: if (pl.skill[47][0])
					 xlog(1,"Estimated increase to Speed granted by your Haste spell."); break;
			case 23: if (pl.skill[22][0] && IS_SHIFTED)
					 xlog(1,"Effective reduction to incoming top damage granted while under the effect of your Calm skill."); break;
			case 24: if (pl.skill[22][0] && IS_SHIFTED)
					 xlog(1,"Effective multiplier to incoming damage over time granted while under the effect of your Calm skill."); break;
			case 25: if (pl.skill[26][0])
					 xlog(1,"Effective %s expected when casting your %s spell.", (pl_flags&(1<<14))?"healing over time":"flat healing", (pl_flags&(1<<14))?"Regen":"Heal"); break;
			case 26: if (pl.skill[37][0])  {
					 if (pl_flagb&(5<<11)) { xlog(1,"Effective reduction of target Spell Modifier when using your Blind (Douse) skill, before reduction from target Immunity."); }
					 else                  { xlog(1,"Effective reduction of target Hit and Parry Scores when using your Blind skill, before reduction from target Immunity."); } } break;
			case 27: if (pl.skill[37][0])
					 xlog(1,"Skill exhaustion duration expected upon using your %s skill.", (pl_flagb&(1<<11))?"Douse":"Blind"); break;
			case 28: if (pl.skill[35][0])  {
					 if (pl_flagb&(1<<12)) { xlog(1,"Effective bonus to hit and parry score granted to allies when using your Rally skill. Half of this value is granted to yourself as well."); }
					 else                  { xlog(1,"Effective reduction of target attributes when using your Warcry skill, before reduction from target Immunity."); } } break;
			case 29: if (pl.skill[35][0])
					 xlog(1,"Skill exhaustion duration expected upon using your %s skill.", (pl_flagb&(1<<12))?"Rally":"Warcry"); break;
			case 30: if (pl.skill[41][0]) {
					 if (pl_flags&(1<<10)) xlog(1,"Effective reduction of target Armor Value when using your Crush skill, before reduction from target Immunity.");
					 else xlog(1,"Effective reduction of target Weapon Value when using your Weaken skill, before reduction from target Immunity."); } break;
			case 31: if (pl.skill[41][0])
					 xlog(1,"Skill exhaustion duration expected upon using your %s skill.", (pl_flags&(1<<10))?"Crush":"Weaken"); break;
			case 32: if (pl.skill[20][0])
					 xlog(1,"Effective reduction of target attributes when casting your Curse spell, before reduction from target Immunity."); break;
			case 33: if (pl.skill[20][0])
					 xlog(1,"Skill exhaustion duration expected upon using your Curse spell."); break;
			case 34: if (pl.skill[19][0])
					 xlog(1,"Effective reduction of target action speed when casting your Slow spell, before reduction from target Immunity."); break;
			case 35: if (pl.skill[19][0])
					 xlog(1,"Skill exhaustion duration expected upon using your Slow spell."); break;
			case 36: if (pl.skill[27][0])
					 xlog(1,"Effective power of your Ghost Companion, granted by your Ghost Companion spell. A higher number grants a stronger companion."); break;
			case 37: if (pl.skill[27][0])
					 xlog(1,"Skill exhaustion duration expected upon using your Ghost Companion spell."); break;
			case 38: if (pl.skill[46][0])
					 xlog(1,"Effective power of your Shadow Copy, granted by your Shadow Copy spell. A higher number grants a stronger companion."); break;
			case 39: if (pl.skill[46][0])
					 xlog(1,"Effective duration of your Shadow Copy, granted by your Shadow Copy spell."); break;
			case 40: if (pl.skill[46][0])
					 xlog(1,"Skill exhaustion duration expected upon using your Shadow Copy spell."); break;
			//
			default: break;
		}
	}
}

int mouse_statbox2(int x,int y,int state)
{
	int n, m;
    extern struct skilltab _skilltab[];
	int pl_flags, pl_flagb;
	char tmp[200];
	static int firstclick=0;
	
	/*
	if (screen_windowed == 1) {
        y-=1;
    }
	else
	{
		y+=1;
	}
	*/
	
	// Attributes
	if (state==MS_RB_UP) 
	{
		int xt=5,yt=5,xb=136,yb=18,shf=14;
		
		// 5,  5		136, 18
		// 5, 19		136, 32
		// 5, 33		136, 46
		
		n=0; if ( x>xt && y>yt+shf*n && x<xb && y<yb+shf*n ) // Braveness
		{
			if (hudmode==0)
				xlog(1,"%s improves most skills and spells. It also improves your critical hit rate.",at_name[n]);
			else
				meta_stat_descs(n);
			return 1;
		}
		n=1; if ( x>xt && y>yt+shf*n && x<xb && y<yb+shf*n ) // Willpower
		{
			if (hudmode==0)
				xlog(1,"%s improves most support spells. It also improves the speed of casting spells and using skills, and helps overwhelm spell suppression.",at_name[n]);
			else
				meta_stat_descs(n);
			return 1;
		}
		n=2; if ( x>xt && y>yt+shf*n && x<xb && y<yb+shf*n ) // Intuition
		{
			if (hudmode==0)
				xlog(1,"%s improves most offensive spells. It also reduces the duration of skill exhaustion.",at_name[n]);
			else
				meta_stat_descs(n);
			return 1;
		}
		n=3; if ( x>xt && y>yt+shf*n && x<xb && y<yb+shf*n ) // Agility
		{
			if (hudmode==0)
				xlog(1,"%s improves most combat skills. It also improves your movement speed and your attack speed.",at_name[n]);
			else
				meta_stat_descs(n);
			return 1;
		}
		n=4; if ( x>xt && y>yt+shf*n && x<xb && y<yb+shf*n ) // Strength
		{
			if (hudmode==0)
				xlog(1,"%s improves most combat skills. It also improves your movement speed and the damage dealt by your attacks.",at_name[n]);
			else
				meta_stat_descs(n);
			return 1;
		}
		n=5; if ( x>xt && y>yt+shf*n && x<xb && y<yb+shf*n ) // Hitpoints
		{
			if (hudmode==0)
				xlog(1,"Your Hitpoints.");
			else
				meta_stat_descs(n);
			return 1;
		}
		n=6; if ( x>xt && y>yt+shf*n && x<xb && y<yb+shf*n ) // Mana
		{
			if (hudmode==0)
				xlog(1,"Your Mana.");
			else
				meta_stat_descs(n);
			return 1;
		}
	}
	
	if (x<gui_skl_names[RECT_X1]) return 0;
	if (x>gui_skl_names[RECT_X2]) return 0;
	if (y<gui_skl_names[RECT_Y1]) return 0;
	if (y>gui_skl_names[RECT_Y2]) return 0;

	n=(y-gui_skl_names[RECT_Y1])/14;

	hightlight=HL_STATBOX2;
	hightlight_sub=n;
	
	// Player Flags from special items
	pl_flags = pl.worn[WN_FLAGS];
	pl_flagb = pl.worn_p[WN_FLAGS];
	
	// Skills
	if (state==MS_RB_UP) 
	{
		if (hudmode==0 || hudmode==3)
		{
			m = skilltab[n+skill_pos].nr;
			if (pl.skill[m][0] || m==50 || m==51 || (m==52 && KNOW_IDENTIFY) || ((m==53 || m==54) && IS_LYCANTH))
			{
				if (	(m==11&&(pl_flagb & (1 << 10))) ||	// Magic Shield -> Magic Shell
						(m==19&&(pl_flags & (1 <<  5))) ||	// Slow -> Greater Slow
						(m==20&&(pl_flags & (1 <<  6))) ||	// Curse -> Greater Curse
						(m==24&&(pl_flags & (1 <<  7))) ||	// Blast -> +Scorch
						(m==26&&(pl_flags & (1 << 14))) ||	// Heal -> Regen
						(m==37&&(pl_flagb & (1 << 11))) ||	// Blind -> Douse
						(m==40&&(pl_flags & (1 <<  8))) ||	// Cleave -> +Aggravate
						(m==41&&(pl_flags & (1 << 10))) ||  // Weaken -> Greater Weaken
						(m==16&&(pl_flagb & (1 <<  5))) ||  // Shield -> Shield Bash
						(m==43&&(pl_flagb & (1 <<  6))) ||  // Pulse -> Healing Pulses
						(m==49&&(pl_flagb & (1 <<  7))) ||  // Leap
						(m==35&&(pl_flagb & (1 << 12))) ||  // Warcry -> Rally
						(m==42&&(pl_flagb & (1 << 14))) ||  // Poison -> Venom
						(m==14&&(pl_flagb & (1 <<  3))) ||  // Finesse invert
						(m==22&&IS_SHIFTED)
					)
				{
					strcpy(tmp, skilltab[n+skill_pos].alt_a);
					xlog(1,skilltab[n+skill_pos].alt_b);
				}
				else if (m==44)	// Proximity has special descriptions
				{
					strcpy(tmp, skilltab[n+skill_pos].name);
					if (IS_BRAVER)
						xlog(1,skilltab[n+skill_pos].desc); // Braver
					else if (IS_SORCERER)
						xlog(1,skilltab[n+skill_pos].alt_a); // Sorcerer
					else if (IS_ARCHHARAKIM)
						xlog(1,skilltab[n+skill_pos].alt_b); // Arch-Harakim
					else
						xlog(1,"Passively improves the area-of-effect of various skills and spells.");
				}
				else
				{
					strcpy(tmp, skilltab[n+skill_pos].name);
					xlog(1,skilltab[n+skill_pos].desc);
				}
				
				if (last_skill == n+skill_pos)
				{
					last_skill = -1;
					xlog(6,"%s no longer selected for shortcut.",tmp);
				}
				else
				{
					last_skill = n+skill_pos;
					if (!firstclick)
					{
						xlog(6,"%s selected for shortcut. Right-click on one of the shortcut keys in the bottom right to set a shortcut.",tmp);
						firstclick=1;
					}
					else
					{
						xlog(6,"%s selected for shortcut.",tmp);
					}
					
				}
			}
		}
		else if (hudmode==1)
		{
			meta_stat_descs(n+7);
		}
		else if (hudmode==2)
		{
			meta_stat_descs(n+7);
		}
	} 
	else if (state==MS_LB_UP && (hudmode==0 || hudmode==3))
	{
		cmd3(CL_CMD_SKILL,skilltab[n+skill_pos].nr,selected_char,skilltab[n+skill_pos].attrib[0]);
	}
	return 1;
}

void cmd(int cmd,int x,int y)
{
	unsigned char buf[16];

	play_sound("sfx\\click.wav",CLICKVOL,0);

	buf[0]=(char)cmd;
	*(unsigned short*)(buf+1)=(short)x;
	*(unsigned long*)(buf+3)=(long)y;
	xsend(buf);
}

void cmds(int cmd,int x,int y)
{
	unsigned char buf[16];

	buf[0]=(char)cmd;
	*(unsigned short*)(buf+1)=(short)x;
	*(unsigned long*)(buf+3)=(long)y;
	xsend(buf);
}

void cmd3(int cmd,int x,int y,int z)
{
	unsigned char buf[16];

	play_sound("sfx\\click.wav",CLICKVOL,0);

	buf[0]=(char)cmd;
	*(unsigned long*)(buf+1)=x;
	*(unsigned long*)(buf+5)=y;
	*(unsigned long*)(buf+9)=z;
	xsend(buf);
}

void cmd1(int cmd,int x)
{
	unsigned char buf[16];

	play_sound("sfx\\click.wav",CLICKVOL,0);

	buf[0]=(char)cmd;
	*(unsigned int*)(buf+1)=x;
	xsend(buf);
}

void cmd1s(int cmd,int x)
{
	unsigned char buf[16];

	buf[0]=(char)cmd;
	*(unsigned int*)(buf+1)=x;
	xsend(buf);
}

void mouse_mapbox(int x,int y,int state)
{
	int mx,my,m,keys,dist_diff;
	int xr,yr,tst;

	// X and Y without offset below
	xr=x;
	yr=y;

	x+=176-16;
	y+=8;
	
	/*
	if (screen_windowed == 1) {
		y+=4;
	}
	*/

	dist_diff=(screen_renderdist-screen_viewsize)/2;

	mx=2*y+x-(screen_tileyoff*2)-screen_tilexoff+((screen_renderdist-34)/2*32);
	my=x-2*y+(screen_tileyoff*2)-screen_tilexoff+((screen_renderdist-34)/2*32);

        if (mx<(dist_diff+3)*32+12 || mx>(screen_renderdist-dist_diff-view_subedges-3)*32+20 || my<(dist_diff+view_subedges+3)*32+12 || my>(screen_renderdist-dist_diff-3)*32+20) {
			// Clicking auto-walk buttons
			if (state==MS_LB_UP) 
			{
				if 		((int)sqrt(pow(xr-xwalk_nx,2)+pow(yr-xwalk_ny,2)) < 12)	{ xmove=1; xxtimer=0; } // North
				else if ((int)sqrt(pow(xr-xwalk_wx,2)+pow(yr-xwalk_wy,2)) < 12)	{ xmove=2; xxtimer=0; } // West
				else if ((int)sqrt(pow(xr-xwalk_sx,2)+pow(yr-xwalk_sy,2)) < 12)	{ xmove=3; xxtimer=0; } // South
				else if ((int)sqrt(pow(xr-xwalk_ex,2)+pow(yr-xwalk_ey,2)) < 12)	{ xmove=4; xxtimer=0; } // East
			}
			return;
		}

	mx=mx/32;
	my=my/32;

        tile_x=-1; tile_y=-1; tile_type=-1;
	if (mx<dist_diff+3 || mx>screen_renderdist-dist_diff-view_subedges-3) return;
	if (my<dist_diff+view_subedges+3 || my>screen_renderdist-dist_diff-3) return;

	m=mx+my*screen_renderdist;

//	xlog("light=%d",map[m].light);
//  xlog("ch_sprite=%d, nr=%d, id=%d",map[m].ch_sprite,map[m].ch_nr,map[m].ch_id);

	if (pl.citem==46) { // build
		if (state==MS_RB_UP) { xmove=xxtimer=0; cmd(CL_CMD_DROP,map[m].x,map[m].y); }
		if (state==MS_LB_UP) { xmove=xxtimer=0; cmd(CL_CMD_PICKUP,map[m].x,map[m].y); }
		tile_type=0; tile_x=mx; tile_y=my;
		hightlight=HL_MAP;
		return;
	}

	keys=0;
	if (GetAsyncKeyState(VK_SHIFT)&0x8000) keys|=1;
	if (GetAsyncKeyState(VK_CONTROL)&0x8000) keys|=2;
	if (GetAsyncKeyState(VK_MENU)&0x8000) keys|=4;

	if (keys==0) {
		tile_x=mx; tile_y=my;
		tile_type=0;

		if (state==MS_RB_UP) { xmove=xxtimer=0; cmd(CL_CMD_TURN,map[m].x,map[m].y); }
		if (state==MS_LB_UP) { xmove=xxtimer=0; cmd(CL_CMD_MOVE,map[m].x,map[m].y); }
		hightlight=HL_MAP;
		cursor_type=CT_WALK;
		return;
	}

	if (keys==1) {
		if (pl.citem) { hightlight=HL_MAP; cursor_type=CT_DROP; }
		else if (map[m].flags&ISITEM) ;
		else if (map[m+1-screen_renderdist].flags&ISITEM) { mx++; my--; }
		else if (map[m+2-2*screen_renderdist].flags&ISITEM) { mx+=2; my-=2; }
		else if (map[m+1].flags&ISITEM) { mx++; }
		else if (map[m+screen_renderdist].flags&ISITEM) { my++; }
		else if (map[m-1].flags&ISITEM) { mx--; }
		else if (map[m-screen_renderdist].flags&ISITEM) { my--; }
		else if (map[m+1+screen_renderdist].flags&ISITEM) { mx++; my++; }
		else if (map[m-1+screen_renderdist].flags&ISITEM) { mx--; my++; }
		else if (map[m-1-screen_renderdist].flags&ISITEM) { mx--; my--; }
		else if (map[m+2].flags&ISITEM) { mx+=2; }
		else if (map[m+2*screen_renderdist].flags&ISITEM) { my+=2; }
		else if (map[m-2].flags&ISITEM) { mx-=2; }
		else if (map[m-2*screen_renderdist].flags&ISITEM) { my-=2; }
		else if (map[m+1+2*screen_renderdist].flags&ISITEM) { mx++; my+=2; }
		else if (map[m-1+2*screen_renderdist].flags&ISITEM) { mx--; my+=2; }
		else if (map[m+1-2*screen_renderdist].flags&ISITEM) { mx++; my-=2; }
		else if (map[m-1-2*screen_renderdist].flags&ISITEM) { mx--; my-=2; }
		else if (map[m+2+1*screen_renderdist].flags&ISITEM) { mx+=2; my++; }
		else if (map[m-2+1*screen_renderdist].flags&ISITEM) { mx-=2; my++; }
		else if (map[m+2-1*screen_renderdist].flags&ISITEM) { mx+=2; my--; }
		else if (map[m-2-1*screen_renderdist].flags&ISITEM) { mx-=2; my--; }
		else if (map[m+2+2*screen_renderdist].flags&ISITEM) { mx+=2; my+=2; }
		else if (map[m-2+2*screen_renderdist].flags&ISITEM) { mx-=2; my+=2; }
		else if (map[m-2-2*screen_renderdist].flags&ISITEM) { mx-=2; my-=2; }

		m=mx+my*screen_renderdist;
		tile_x=mx; tile_y=my;

		if (pl.citem && map[m].flags&ISITEM) { if (map[m].flags&ISUSABLE) cursor_type=CT_USE; else cursor_type=CT_NONE; }
		else if (map[m].flags&ISITEM) { hightlight=HL_MAP; if (map[m].flags&ISUSABLE) cursor_type=CT_USE; else cursor_type=CT_TAKE; }

		if (pl.citem && !(map[m].flags&ISITEM)) {
			if (state==MS_LB_UP) { xmove=xxtimer=0; cmd(CL_CMD_DROP,map[m].x,map[m].y); }
			tile_type=0;
		}
		if ((map[m].flags&ISITEM)) {
			if (state==MS_LB_UP) {
				if (map[m].flags&ISUSABLE) { xmove=xxtimer=0; cmd(CL_CMD_USE,map[m].x,map[m].y); noshop=0; }
				else { xmove=xxtimer=0; cmd(CL_CMD_PICKUP,map[m].x,map[m].y); }
			}
			if (state==MS_RB_UP) { xmove=xxtimer=0; cmd(CL_CMD_LOOK_ITEM,map[m].x,map[m].y); }
			tile_type=1;
		}
		return;
	}

	if (keys==2) {
		if (map[m].flags&ISCHAR) hightlight=HL_MAP;
		else if (map[m+1-screen_renderdist].flags&ISCHAR) { mx++; my--; hightlight=HL_MAP; }
		else if (map[m+2-2*screen_renderdist].flags&ISCHAR) { mx+=2; my-=2; hightlight=HL_MAP; }
		else if (map[m+1].flags&ISCHAR) { mx++; hightlight=HL_MAP; }
		else if (map[m+screen_renderdist].flags&ISCHAR) { my++; hightlight=HL_MAP; }
		else if (map[m-1].flags&ISCHAR) { mx--; hightlight=HL_MAP; }
		else if (map[m-screen_renderdist].flags&ISCHAR) { my--; hightlight=HL_MAP; }
		else if (map[m+1+screen_renderdist].flags&ISCHAR) { mx++; my++; hightlight=HL_MAP; }
		else if (map[m-1+screen_renderdist].flags&ISCHAR) { mx--; my++; hightlight=HL_MAP; }
		else if (map[m-1-screen_renderdist].flags&ISCHAR) { mx--; my--; hightlight=HL_MAP; }
		else if (map[m+2].flags&ISCHAR) { mx+=2; hightlight=HL_MAP; }
		else if (map[m+2*screen_renderdist].flags&ISCHAR) { my+=2; hightlight=HL_MAP; }
		else if (map[m-2].flags&ISCHAR) { mx-=2; hightlight=HL_MAP; }
		else if (map[m-2*screen_renderdist].flags&ISCHAR) { my-=2; hightlight=HL_MAP; }
		else if (map[m+1+2*screen_renderdist].flags&ISCHAR) { mx++; my+=2; hightlight=HL_MAP; }
		else if (map[m-1+2*screen_renderdist].flags&ISCHAR) { mx--; my+=2; hightlight=HL_MAP; }
		else if (map[m+1-2*screen_renderdist].flags&ISCHAR) { mx++; my-=2; hightlight=HL_MAP; }
		else if (map[m-1-2*screen_renderdist].flags&ISCHAR) { mx--; my-=2; hightlight=HL_MAP; }
		else if (map[m+2+1*screen_renderdist].flags&ISCHAR) { mx+=2; my++; hightlight=HL_MAP; }
		else if (map[m-2+1*screen_renderdist].flags&ISCHAR) { mx-=2; my++; hightlight=HL_MAP; }
		else if (map[m+2-1*screen_renderdist].flags&ISCHAR) { mx+=2; my--; hightlight=HL_MAP; }
		else if (map[m-2-1*screen_renderdist].flags&ISCHAR) { mx-=2; my--; hightlight=HL_MAP; }
		else if (map[m+2+2*screen_renderdist].flags&ISCHAR) { mx+=2; my+=2; hightlight=HL_MAP; }
		else if (map[m-2+2*screen_renderdist].flags&ISCHAR) { mx-=2; my+=2; hightlight=HL_MAP; }
		else if (map[m-2-2*screen_renderdist].flags&ISCHAR) { mx-=2; my-=2; hightlight=HL_MAP; }

		m=mx+my*screen_renderdist;
		tile_x=mx; tile_y=my;

      if (map[m].flags&ISCHAR) {
			if (pl.citem) cursor_type=CT_GIVE;
			else cursor_type=CT_HIT;
		}

		if (map[m].flags&ISCHAR) {
			if (pl.citem && state==MS_LB_UP) { xmove=xxtimer=0; cmd1(CL_CMD_GIVE,map[m].ch_nr); }
			else if (state==MS_LB_UP) { xmove=xxtimer=0; cmd1(CL_CMD_ATTACK,map[m].ch_nr); }
			else if (state==MS_RB_UP) { xmove=xxtimer=0; cmd1(CL_CMD_LOOK,map[m].ch_nr); noshop=0; }
			tile_type=2;
		}
	}

	if (keys==4) {
		if (map[m].flags&ISCHAR) hightlight=HL_MAP;
		else if (map[m+1-screen_renderdist].flags&ISCHAR) { mx++; my--; hightlight=HL_MAP; }
		else if (map[m+2-2*screen_renderdist].flags&ISCHAR) { mx+=2; my-=2; hightlight=HL_MAP; }
		else if (map[m+1].flags&ISCHAR) { mx++; hightlight=HL_MAP; }
		else if (map[m+screen_renderdist].flags&ISCHAR) { my++; hightlight=HL_MAP; }
		else if (map[m-1].flags&ISCHAR) { mx--; hightlight=HL_MAP; }
		else if (map[m-screen_renderdist].flags&ISCHAR) { my--; hightlight=HL_MAP; }
		else if (map[m+1+screen_renderdist].flags&ISCHAR) { mx++; my++; hightlight=HL_MAP; }
		else if (map[m-1+screen_renderdist].flags&ISCHAR) { mx--; my++; hightlight=HL_MAP; }
		else if (map[m-1-screen_renderdist].flags&ISCHAR) { mx--; my--; hightlight=HL_MAP; }
		else if (map[m+2].flags&ISCHAR) { mx+=2; hightlight=HL_MAP; }
		else if (map[m+2*screen_renderdist].flags&ISCHAR) { my+=2; hightlight=HL_MAP; }
		else if (map[m-2].flags&ISCHAR) { mx-=2; hightlight=HL_MAP; }
		else if (map[m-2*screen_renderdist].flags&ISCHAR) { my-=2; hightlight=HL_MAP; }
		else if (map[m+1+2*screen_renderdist].flags&ISCHAR) { mx++; my+=2; hightlight=HL_MAP; }
		else if (map[m-1+2*screen_renderdist].flags&ISCHAR) { mx--; my+=2; hightlight=HL_MAP; }
		else if (map[m+1-2*screen_renderdist].flags&ISCHAR) { mx++; my-=2; hightlight=HL_MAP; }
		else if (map[m-1-2*screen_renderdist].flags&ISCHAR) { mx--; my-=2; hightlight=HL_MAP; }
		else if (map[m+2+1*screen_renderdist].flags&ISCHAR) { mx+=2; my++; hightlight=HL_MAP; }
		else if (map[m-2+1*screen_renderdist].flags&ISCHAR) { mx-=2; my++; hightlight=HL_MAP; }
		else if (map[m+2-1*screen_renderdist].flags&ISCHAR) { mx+=2; my--; hightlight=HL_MAP; }
		else if (map[m-2-1*screen_renderdist].flags&ISCHAR) { mx-=2; my--; hightlight=HL_MAP; }
		else if (map[m+2+2*screen_renderdist].flags&ISCHAR) { mx+=2; my+=2; hightlight=HL_MAP; }
		else if (map[m-2+2*screen_renderdist].flags&ISCHAR) { mx-=2; my+=2; hightlight=HL_MAP; }
		else if (map[m-2-2*screen_renderdist].flags&ISCHAR) { mx-=2; my-=2; hightlight=HL_MAP; }

		m=mx+my*screen_renderdist;
		tile_x=mx; tile_y=my;

		if (map[m].flags&ISCHAR) {
			cursor_type=CT_SEL;
			if (state==MS_LB_UP) { xmove=xxtimer=0; if (selected_char==map[m].ch_nr) selected_char=0; else selected_char=map[m].ch_nr; }
			else if (state==MS_RB_UP) { xmove=xxtimer=0; cmd1(CL_CMD_LOOK,map[m].ch_nr); noshop=0; }
			tile_type=2;
		} else if (state==MS_LB_UP) selected_char=0;
	}
}

#ifndef GUI_SHOP_X
	#define GUI_SHOP_X		((1280/2)-(320/2))
	#define GUI_SHOP_Y		((736/2)-(320/2)+72)
#endif

int mouse_shop(int x,int y,int mode)
{
	int nr,tx,ty,keys;

	if (!show_shop)     return 0;
	if (show_shop>=112) return 0;
	
	keys=0;
	
	if (show_shop==110 || show_shop==111) // Blacksmith
	{
		if (GetAsyncKeyState(VK_SHIFT)&0x8000)   keys|=1;
		if (GetAsyncKeyState(VK_CONTROL)&0x8000) keys|=2;
		if (GetAsyncKeyState(VK_MENU)&0x8000)    keys|=4;
	
		// [X]
		if (x>(GUI_SHOP_X+279) && x<(GUI_SHOP_X+296) && y>(GUI_SHOP_Y+80) && y<(GUI_SHOP_Y+94)) 
		{	if (mode==MS_LB_UP) { show_shop=0; noshop=QSIZE*3; } return 1;	}
		
		nr = -1;
		
		if (     x>(GUI_SHOP_X+ 44) && x<(GUI_SHOP_X+ 79) && y>(GUI_SHOP_Y+123) && y<(GUI_SHOP_Y+158))	nr = 0;	// Left item
		if (pl.sitem[1]==17357)	{
			if ( x>(GUI_SHOP_X+123) && x<(GUI_SHOP_X+158) && y>(GUI_SHOP_Y+ 97) && y<(GUI_SHOP_Y+132))	nr = 1;	// Top-Middle item
			if ( x>(GUI_SHOP_X+123) && x<(GUI_SHOP_X+158) && y>(GUI_SHOP_Y+149) && y<(GUI_SHOP_Y+184))	nr = 2;	// Bottom-Middle item
		}
		else if (x>(GUI_SHOP_X+123) && x<(GUI_SHOP_X+158) && y>(GUI_SHOP_Y+123) && y<(GUI_SHOP_Y+158))	nr = 1;	// Middle item
		if (     x>(GUI_SHOP_X+202) && x<(GUI_SHOP_X+237) && y>(GUI_SHOP_Y+123) && y<(GUI_SHOP_Y+158))	nr = 3;	// Right item
		
		if (     x>(GUI_SHOP_X+ 35) && x<(GUI_SHOP_X+ 82) && y>(GUI_SHOP_Y+211) && y<(GUI_SHOP_Y+225))	nr = 4;	// Left button
		if (     x>(GUI_SHOP_X+117) && x<(GUI_SHOP_X+164) && y>(GUI_SHOP_Y+211) && y<(GUI_SHOP_Y+225))	nr = 5;	// Middle button
		if (     x>(GUI_SHOP_X+199) && x<(GUI_SHOP_X+246) && y>(GUI_SHOP_Y+211) && y<(GUI_SHOP_Y+225))	nr = 6;	// Right button
		
		if (nr > -1 && nr < 4)		// Item clicks
		{
			if (mode==MS_LB_UP)
			{
				if (keys == 2 || keys == 4)
					cmd(CL_CMD_SMITH,shop.nr,nr+16);
				else if (keys == 1)
					cmd(CL_CMD_SMITH,shop.nr,nr);
			}
			if (mode==MS_RB_UP)
				cmd(CL_CMD_SMITH,shop.nr,nr+8);
			hightlight=HL_SHOP;
			hightlight_sub=nr;
			return 1;
		}
		if (nr > 3 && nr < 7)		// Button clicks
		{
			if (pl.sitem[1]!=17356 && (nr == 4 || nr == 6)) return 0;
			if (mode==MS_LB_UP)
				cmd(CL_CMD_SMITH,shop.nr,nr);
			if (mode==MS_RB_UP)
				cmd(CL_CMD_SMITH,shop.nr,nr+8);
			return 1;
		}
		// prevent clicking the world behind the menu
		if (x>(GUI_SHOP_X) && x<(GUI_SHOP_X+281) && y>(GUI_SHOP_Y+80) && y<(GUI_SHOP_Y+236))
			return 1;
		
		return 0;
	}
	if ((GetAsyncKeyState(VK_CONTROL)&0x8000)||(GetAsyncKeyState(VK_MENU)&0x8000)) keys=1;
	
	// [X]
	if (x>(GUI_SHOP_X+279) && x<(GUI_SHOP_X+296) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+14)) 
	{	if (mode==MS_LB_UP) { show_shop=0; noshop=QSIZE*3; } return 1;	}
	
	// Shop Contents
	if (x>(GUI_SHOP_X) && x<(GUI_SHOP_X+280) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+280))
	{
		tx=(x-(GUI_SHOP_X))/35;
		ty=(y-(GUI_SHOP_Y))/35;

		nr=min(62,tx+ty*8);
		if (mode==MS_LB_UP) 
		{
			if (keys)
				cmd(CL_CMD_SHOP,shop.nr,nr+124);
			else
				cmd(CL_CMD_SHOP,shop.nr,nr);
		}
		if (mode==MS_RB_UP) cmd(CL_CMD_SHOP,shop.nr,nr+62);

		if (shop.item[nr])	{ cursor_type=CT_TAKE; }
		else if (pl.citem)	{ cursor_type=CT_DROP; }
		hightlight=HL_SHOP;
		hightlight_sub=nr;
		return 1;
	}
	
	// prevent clicking the world behind the menu
	if (x>(GUI_SHOP_X) && x<(GUI_SHOP_X+281) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+316))
		return 1;
	
	return 0;
}

int mouse_depot(int x,int y,int mode)
{
	int n,m,nr,tx,ty;

	if (show_shop!=112) return 0;
	
	// [X]
	if (x>(GUI_SHOP_X+279) && x<(GUI_SHOP_X+296) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+14)) 
	{	if (mode==MS_LB_UP) { show_shop=0; noshop=QSIZE*3; } return 1;	}
	
	// Depot Contents
	if (x>(GUI_SHOP_X+4) && x<(GUI_SHOP_X+276) && y>(GUI_SHOP_Y+4) && y<(GUI_SHOP_Y+276))
	{
		tx=(x-(GUI_SHOP_X+4))/34;
		ty=(y-(GUI_SHOP_Y+4))/34;

		nr=min(512,dept_page*64+tx+ty*8);
		if (mode==MS_LB_UP) cmd(CL_CMD_SHOP,shop.nr,nr);
		if (mode==MS_RB_UP) cmd(CL_CMD_SHOP,shop.nr,nr+8*64);

		if (shop.depot[nr/64][nr%64])	{ cursor_type=CT_TAKE; }
		else if (pl.citem)	{ cursor_type=CT_DROP; }
		hightlight=HL_SHOP;
		hightlight_sub=nr;
		return 1;
	}
	
	// Depot Page Buttons
	for (n=0;n<4;n++) for (m=0;m<2;m++) if (x>(GUI_SHOP_X+7+68*n) && x<(GUI_SHOP_X+70+68*n) && y>(GUI_SHOP_Y+281+17*m) && y<(GUI_SHOP_Y+293+17*m))
	{
		if (mode==MS_LB_UP)
		{
			dept_page = n+m*4;
			play_sound("sfx\\click.wav",CLICKVOL,0);
		}
		return 1;
	}
	
	// prevent clicking the world behind the menu
	if (x>(GUI_SHOP_X) && x<(GUI_SHOP_X+281) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+316))
	{
		return 1;
	}
	
	return 0;
}

int mouse_wps(int x,int y,int mode)
{
	int nr, ty, keys;
	
	if (!show_wps) return 0;
	
	keys=0;
	if ((GetAsyncKeyState(VK_SHIFT)&0x8000)||(GetAsyncKeyState(VK_CONTROL)&0x8000)||(GetAsyncKeyState(VK_MENU)&0x8000)) keys=1;
	
	// Close Window
	if (x>(GUI_SHOP_X+279) && x<(GUI_SHOP_X+296) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+14)) 
	{
		if (mode==MS_LB_UP) 
		{ 
			show_wps=0;
		}
		return 1;
	}
	
	// Selecting a Waypoint
	if (x>(GUI_SHOP_X) && x<(GUI_SHOP_X+280-13) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+280)) 
	{
		ty=(y-(GUI_SHOP_Y))/35;

		nr=wpslist[ty+wps_pos].nr;
		if (mode==MS_LB_UP) 
		{
			cmd1(CL_CMD_WPS,nr);
			if (pl.waypoints&(1<<nr))
			{
				show_wps=0;
				noshop=QSIZE*3; 
			}
		}
		if (mode==MS_RB_UP) cmd1(CL_CMD_WPS,nr+32);
		
		hightlight=HL_WAYPOINT;
		hightlight_sub=ty;
		return 1;
	}
	
	// Scroll up
	if (x>(GUI_SHOP_X+280-13) && x<(GUI_SHOP_X+280) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+35))
	{
		if (mode==MS_LB_UP) 
		{
			if (keys) 			wps_pos -= 8; 
			else				wps_pos -= 2; 
			if (wps_pos < 0) 	wps_pos  = 0;
		}
		return 1;
	}
	
	// Scroll down
	if (x>(GUI_SHOP_X+280-13) && x<(GUI_SHOP_X+280) && y>(GUI_SHOP_Y+280-35) && y<(GUI_SHOP_Y+280))
	{
		if (mode==MS_LB_UP) 
		{
			if (keys)					wps_pos += 8;
			else						wps_pos += 2;
			if (wps_pos > (MAXWPS-8))	wps_pos  = (MAXWPS-8);
		}
		return 1;
	}
	
	// prevent clicking the world behind the menu
	if (x>(GUI_SHOP_X) && x<(GUI_SHOP_X+281) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+316))
	{
		return 1;
	}
	
	return 0;
}

int mouse_tree(int x, int y, int mode)
{
	int nr, n, keys;
	
	if (!show_tree) return 0;
	
	keys=0;
	if ((GetAsyncKeyState(VK_SHIFT)&0x8000)||(GetAsyncKeyState(VK_CONTROL)&0x8000)||(GetAsyncKeyState(VK_MENU)&0x8000)) keys=1;
	
	// Close Window
	if (x>(GUI_SHOP_X+279) && x<(GUI_SHOP_X+296) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+14)) 
	{
		if (mode==MS_LB_UP) 
		{ 
			show_tree=0;
		}
		return 1;
	}
	
	// Figure out which skill icon we may be moused over
	for (nr=0;nr<12;nr++)
	{
		if ((int)sqrt(pow(x-(GUI_SHOP_X+sk_icon[nr].x),2)+pow(y-(GUI_SHOP_Y+sk_icon[nr].y),2)) < 14) break;
	}
	
	// Selecting a skill icon
	if (nr<12) 
	{
		if (show_tree==2)
		{
			if (mode==MS_LB_UP) 
			{
				if (keys) cmd1(CL_CMD_TREE,nr+60);
				else cmd1(CL_CMD_TREE,nr+36);
			}
			if (mode==MS_RB_UP) cmd1(CL_CMD_TREE,nr+48);
		}
		else
		{
			if (mode==MS_LB_UP) 
			{
				if (keys) cmd1(CL_CMD_TREE,nr+24);
				else cmd1(CL_CMD_TREE,nr);
			}
			if (mode==MS_RB_UP) cmd1(CL_CMD_TREE,nr+12);
		}
		
		hightlight=HL_SKTREE;
		hightlight_sub=nr;
		return 1;
	}
	
	// prevent clicking the world behind the menu
	if (x>(GUI_SHOP_X) && x<(GUI_SHOP_X+281) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+316))
	{
		return 1;
	}
	
	return 0;
}

int mouse_book(int x,int y,int mode)
{
	int nr, tx, ty;
	
	if (!show_book) return 0;
	
	// Close Window
	if (x>(GUI_SHOP_X+279) && x<(GUI_SHOP_X+296) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+14)) 
	{
		if (mode==MS_LB_UP) 
		{ 
			if (show_newp) cmd1(CL_CMD_MOTD,16); // Tell server we've skipped the tutorial
			if (show_tuto) cmd1(CL_CMD_MOTD,show_tuto); // Tell server we did this tutorial
			show_book=0;
			noshop=QSIZE*3;
		}
		return 1;
	}
	
	// [Bottom left button]
	if (x>(GUI_SHOP_X+11) && x<(GUI_SHOP_X+58) && y>(GUI_SHOP_Y+291) && y<(GUI_SHOP_Y+305))
	{
		if (mode==MS_LB_UP) 
		{ 
			tuto_page--; 
			if (tuto_page<=1) tuto_page=1;
			else play_sound("sfx\\click.wav",CLICKVOL,0);
		}
		return 1;
	}
	
	// [Bottom right button]
	if (x>(GUI_SHOP_X+223) && x<(GUI_SHOP_X+270) && y>(GUI_SHOP_Y+291) && y<(GUI_SHOP_Y+305))
	{
		if (mode==MS_LB_UP) 
		{
			tuto_page++; 
			if (tuto_page>=tuto_max) tuto_page=tuto_max;
			else play_sound("sfx\\click.wav",CLICKVOL,0);
		}
		return 1;
	}
	
	// prevent clicking the world behind the menu
	if (x>(GUI_SHOP_X) && x<(GUI_SHOP_X+281) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+316))
	{
		return 1;
	}
	
	return 0;
}

int mouse_motd(int x,int y,int mode)
{
	int nr, tx, ty;
	
	if (!show_motd && !show_newp && !show_tuto) return 0;
	
	// Close Window
	if (x>(GUI_SHOP_X+279) && x<(GUI_SHOP_X+296) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+14)) 
	{
		if (mode==MS_LB_UP) 
		{ 
			if (show_newp) cmd1(CL_CMD_MOTD,16); // Tell server we've skipped the tutorial
			if (show_tuto) cmd1(CL_CMD_MOTD,show_tuto); // Tell server we did this tutorial
			show_motd=0;
			show_newp=0;
			show_tuto=0;
			noshop=QSIZE*3;
		}
		return 1;
	}
	
	// Normal MOTD 'OK' button (closes window)
	if (show_motd && x>(GUI_SHOP_X+117) && x<(GUI_SHOP_X+164) && y>(GUI_SHOP_Y+291) && y<(GUI_SHOP_Y+305))
	{
		if (mode==MS_LB_UP) 
		{ 
			show_motd=0;
			noshop=QSIZE*3;
		}
		return 1;
	}
	
	// [Bottom left button]
	if (x>(GUI_SHOP_X+11) && x<(GUI_SHOP_X+58) && y>(GUI_SHOP_Y+291) && y<(GUI_SHOP_Y+305))
	{
		if (mode==MS_LB_UP) 
		{ 
			// New Player MOTD 'Tutorial' button
			if (show_newp)
			{
				show_newp=0;
				show_tuto=1; tuto_page=1; tuto_max=3;
				play_sound("sfx\\click.wav",CLICKVOL,0);
			}
			// Tutorial window PREV
			else if (show_tuto)
			{
				tuto_page--; 
				if (tuto_page<=1) tuto_page=1;
				else play_sound("sfx\\click.wav",CLICKVOL,0);
				
			}
		}
		return 1;
	}
	
	// [Bottom right button]
	if (x>(GUI_SHOP_X+223) && x<(GUI_SHOP_X+270) && y>(GUI_SHOP_Y+291) && y<(GUI_SHOP_Y+305))
	{
		if (mode==MS_LB_UP) 
		{
			// New Player MOTD 'OK' button (closes window)
			if (show_newp)
			{
				cmd1(CL_CMD_MOTD,16); // Tell server we've skipped the tutorial
				show_newp=0;
				noshop=QSIZE*3;
			}
			// Tutorial window NEXT
			else if (show_tuto)
			{
				tuto_page++; 
				if (tuto_page>=tuto_max) tuto_page=tuto_max;
				else play_sound("sfx\\click.wav",CLICKVOL,0);
			}
		}
		return 1;
	}
	
	// prevent clicking the world behind the menu
	if (x>(GUI_SHOP_X) && x<(GUI_SHOP_X+281) && y>(GUI_SHOP_Y) && y<(GUI_SHOP_Y+316))
	{
		return 1;
	}
	
	return 0;
}

void mouse(int x,int y,int state)
{
	if (!init_done) return;

	hightlight=0;
	cursor_type=CT_NONE;
	
	
	/*
	if (screen_windowed == 1) {
		// Adjust position when windowed
		//y += 4;
		x += 2;
		y += 2;
	}
	else
	{
		//y -= 2;
	}
	*/

	mouse_x=x; mouse_y=y;
	if (mouse_inventory(x,y,state)) ;
	else if (mouse_tree(x,y,state)) ;
	else if (mouse_shop(x,y,state)) ;
	else if (mouse_depot(x,y,state)) ;
	else if (mouse_wps(x,y,state)) ;
	else if (mouse_motd(x,y,state)) ;
	else if (mouse_buttonbox(x,y,state)) ;
	else if (mouse_statbox(x,y,state)) ;
	else if (mouse_statbox2(x,y,state)) ;
	else mouse_mapbox(x,y,state);
}
