//
#define VERSION 0x000D02
//

#define MAXSPRITE 2000+(128*1024)

//#define MAPX			TILEX
//#define MAPY			TILEY

#define MAPX_MAX		2048
#define MAPY_MAX		2048

#define MS_MOVE		    0
#define MS_LB_DOWN	    1
#define MS_RB_DOWN	    2
#define MS_LB_UP		3
#define MS_RB_UP		4

#define HPBAR_WIDTH		24

#define TICK			(1000/TICKS)

#define HIGH_VAL		(1<<30)

#define QSIZE			8

#define CLICKVOL		100

// Rendering distance (Must match server-side)
#define RENDERDIST			54 			// "TILEX" and "TILEY" in client.h for server-side

// Resolution-specific data
// 800x600
#define VIEWSIZE_800		34			// View distance in tiles (must be lower than RENDERDIST)
#define VIEW_SUBEDGES_800	4			// Amount of tiles to make un-clickable from edges, in southern and eastern corners (use if your overlay doesn't display those)
#define XPOS_800			0			// X origin for tile drawing
#define YPOS_800			440			// Y origin for tile drawing
#define XWALK_NX_800		164			// Auto-walk button coordinates, format XWALK_<dir><X/Y>_<res>
#define XWALK_NY_800		315
#define XWALK_EX_800		635
#define XWALK_EY_800		315
#define XWALK_SX_800		569
#define XWALK_SY_800		518
#define XWALK_WX_800		230
#define XWALK_WY_800		518

// 1280x720
#define VIEWSIZE_1280		48
#define VIEW_SUBEDGES_1280	0
#define XPOS_1280			224+14
#define YPOS_1280			460
#define XWALK_NX_1280		289
#define XWALK_NY_1280		272
#define XWALK_EX_1280		990
#define XWALK_EY_1280		272
#define XWALK_SX_1280		990
#define XWALK_SY_1280		622
#define XWALK_WX_1280		289
#define XWALK_WY_1280		622

// 1600x900
#define VIEWSIZE_1600		54
#define VIEW_SUBEDGES_1600	0
#define XPOS_1600			440
#define YPOS_1600			520
#define XWALK_NX_1600		415
#define XWALK_NY_1600		325
#define XWALK_EX_1600		1260
#define XWALK_EY_1600		325
#define XWALK_SX_1600		1259
#define XWALK_SY_1600		747
#define XWALK_WX_1600		415
#define XWALK_WY_1600		747

// Rectangle references so coords can just be small arrays
#define RECT_X1				0
#define RECT_Y1				1
#define RECT_X2				2
#define RECT_Y2				3

// GUI overlay sprite number, based on resolution 		// #define GUI_OVERLAY_1280		101
#define GUI_OVERLAY_800		100
#define GUI_OVERLAY_1280	  1
#define GUI_OVERLAY_1600	102

// Offsets from screen width/screen height for positioning GUI elements
#define GUI_QSPELLS_XOFF	198  // Spells quickbar
#define GUI_QSPELLS_YOFF	98

#define GUI_XTRADATA_XOFF	153  // WV/AV/Exp table below chat
#define GUI_XTRADATA_YOFF	243

#define GUI_CHAT_XOFF		299  // Chatbox

#define GUI_MINIMAP_YOFF	130  // Minimap

#define FN_RED			0
#define FN_YEL			1
#define FN_GRE			2
#define FN_BLU			3

struct xbutton
{
	char name[8];
	int skill_nr;
};

struct pdata
{
	char cname[80];
	char ref[80];
	char desc[160];

	char changed;

	int hide;
	int show_names;
	int show_proz;
	int show_bars;
	int show_stats;
	struct xbutton xbutton[20];
};

extern int last_skill;

extern struct pdata pdata;

extern struct cplayer pl;
extern struct cmap *map;

void mouse(int x,int y,int state);

void cmd1s(int cmd,int val);
void add_look(unsigned short nr,char *name,unsigned short id);
int attrib_needed(int n,int v);
int hp_needed(int v);
int mana_needed(int v);
int skill_needed(int n,int v);
void xlog(char font,char *format,...);
void mxlog(char font,char *format,...);
int play_sound(char *file,int vol,int pan);
void reset_block(void);

void setres_800(void);
void setres_1600(void);
void setres_default(void);

void save_options(void);
void load_options(void);
void options(void);

void xsend(unsigned char *buf);
void engine_tick(void);
void so_perf_report(int ticksize,int skip,int idle);
int game_loop(void);
int tick_do(void);
void init_engine(void);

void button_command(int nr);
void cmd_exit(void);
void cmds(int cmd,int x,int y);

struct key
{
	unsigned int usnr;
	unsigned int pass1,pass2;
	char name[40];
	int race;
};

struct look
{
	unsigned char autoflag;
	unsigned short worn[20];
	unsigned short sprite;
	unsigned int points;
	char name[40];
	unsigned int hp;
	unsigned int end;
	unsigned int mana;
	unsigned int a_hp;
	unsigned int a_end;
	unsigned int a_mana;
	unsigned short nr;
	unsigned short id;
	unsigned char extended;
	unsigned short item[62];
	unsigned char item_p[62];
	unsigned int price[62];
	unsigned int pl_price;
	unsigned short  depot[8][64];	// New! Depot list
	unsigned char depot_s[8][64];	//      Depot stacks
	unsigned char depot_f[8][64];	//      Depot flags
	unsigned char depot_c[8][64];	//      Depot catalyst #
};

extern struct look look;
extern struct look shop;
extern unsigned int show_shop, show_wps, show_tree, show_book, show_motd, show_newp, show_tuto, tuto_page, tuto_max, dept_page;

#define HL_BUTTONBOX	1
#define HL_STATBOX		2
#define HL_BACKPACK		3
#define HL_EQUIPMENT	4
#define HL_SPELLBOX		5
#define HL_CITEM		6
#define HL_MONEY		7
#define HL_MAP			8
#define HL_SHOP			9
#define HL_STATBOX2		10
#define HL_WAYPOINT		11
#define HL_SKTREE		12

extern int hightlight;
extern int hightlight_sub;

#define CT_NONE			1
#define CT_TAKE			2
#define CT_DROP			3
#define CT_USE			4
#define CT_GIVE			5
#define CT_WALK			6
#define CT_HIT			7
#define CT_SWAP			8
#define CT_SEL			9

struct skilltab
{
	int nr;
	char sortkey;
	
	char name[40];
	char desc[200];
	
	char alt_a[200]; // If something changes the name
	char alt_b[200]; // If something changes the desc
	
	int attrib[3];
};

extern struct skilltab *skilltab;
extern char *at_name[];

struct wpslist
{
	int nr;
	
	char name[30];
	char desc[30];
};

struct sk_tree
{
	unsigned short icon;
	char name[30];
	char dsc1[50];
	char dsc2[50];
};

struct sk_icon
{
	int x;
	int y;
};

extern struct wpslist wpslist[MAXWPS];
extern struct sk_tree sk_tree[2][12];
extern struct sk_icon sk_icon[12];

void dd_puttext(int x,int y,int font,char *text);
void dd_gputc(int xpos,int ypos,int font,int c);
void dd_gputtext(int xpos,int ypos,int font,char *text,int xoff,int yoff);
void dd_putc(int xpos,int ypos,char font,int c);
void dd_xputtext(int x,int y,int font,char *format,...);
void say(char *input);
void cmd1(int cmd,int x);
void init_xalloc(void);
void tlog(char *text,char font);
void motdlog(char *text,char font);
void do_msg(void);
