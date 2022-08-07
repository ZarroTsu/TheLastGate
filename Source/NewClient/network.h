// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//
//  network.h
//  Header information specific to network.c
//
// **************************************************************** 
#ifndef NETWORK_H
#define NETWORK_H

// client message types:
#define CL_EMPTY			0
#define CL_NEWLOGIN			1
#define CL_LOGIN			2
#define CL_CHALLENGE		3
#define CL_PERF_REPORT		4
#define CL_CMD_MOVE			5
#define CL_CMD_PICKUP		6
#define CL_CMD_ATTACK		7
#define CL_CMD_MODE			8
#define CL_CMD_INV			9
#define CL_CMD_STAT			10
#define CL_CMD_DROP			11
#define CL_CMD_GIVE			12
#define CL_CMD_LOOK			13
#define CL_CMD_INPUT1		14
#define CL_CMD_INPUT2		15
#define CL_CMD_INV_LOOK		16
#define CL_CMD_LOOK_ITEM	17
#define CL_CMD_USE			18
#define CL_CMD_SETUSER		19
#define CL_CMD_TURN			20
#define CL_CMD_AUTOLOOK		21
#define CL_CMD_INPUT3		22
#define CL_CMD_INPUT4		23
#define CL_CMD_RESET		24
#define CL_CMD_SHOP			25
#define CL_CMD_SKILL		26
#define CL_CMD_INPUT5		27
#define CL_CMD_INPUT6		28
#define CL_CMD_INPUT7		29
#define CL_CMD_INPUT8		30
#define CL_CMD_EXIT			31
#define CL_CMD_UNIQUE		32
#define CL_PASSWD			33
#define CL_RENDERDATA		34
#define CL_CMD_WPS			35
#define CL_CMD_MOTD			36
#define CL_CMD_BSSHOP		37
#define CL_CMD_QSHOP		38
#define CL_CMD_CTICK		255

// server message types:
#define SV_EMPTY					0
#define SV_CHALLENGE				1
#define SV_NEWPLAYER				2
#define SV_SETCHAR_NAME1			3
#define SV_SETCHAR_NAME2			4
#define SV_SETCHAR_NAME3			5
#define SV_SETCHAR_MODE				6
#define SV_SETCHAR_ATTRIB			7
#define SV_SETCHAR_SKILL			8

#define SV_SETCHAR_HP				12
#define SV_SETCHAR_ENDUR			13
#define SV_SETCHAR_MANA				14

#define SV_SETCHAR_AHP				20
#define SV_SETCHAR_PTS				21
#define SV_SETCHAR_GOLD				22
#define SV_SETCHAR_ITEM				23
#define SV_SETCHAR_WORN				24
#define SV_SETCHAR_OBJ				25

#define SV_TICK						27

#define SV_LOOK1					29
#define SV_SCROLL_RIGHT				30
#define SV_SCROLL_LEFT				31
#define SV_SCROLL_UP				32
#define SV_SCROLL_DOWN				33
#define SV_LOGIN_OK					34
#define SV_SCROLL_RIGHTUP       	35
#define SV_SCROLL_RIGHTDOWN     	36
#define SV_SCROLL_LEFTUP        	37
#define SV_SCROLL_LEFTDOWN		  	38
#define SV_LOOK2					39
#define SV_LOOK3					40
#define SV_LOOK4					41
#define SV_SETTARGET				42
#define SV_SETMAP2					43
#define SV_SETORIGIN				44
#define SV_SETMAP3					45
#define SV_SETCHAR_SPELL			46
#define SV_PLAYSOUND				47
#define SV_EXIT						48
#define SV_MSG						49
#define SV_LOOK5					50
#define SV_LOOK6					51

#define SV_LOAD                 	56
#define SV_CAP						57
#define SV_MOD1						58
#define SV_MOD2						59
#define SV_MOD3						60
#define SV_MOD4						61
#define SV_MOD5						62
#define SV_MOD6						63
#define SV_MOD7						64
#define SV_MOD8						65
#define SV_SETMAP4					66
#define SV_SETMAP5					67
#define SV_SETMAP6					68
#define SV_SETCHAR_AEND				69
#define SV_SETCHAR_AMANA			70
#define SV_SETCHAR_DIR				71
#define SV_UNIQUE					72
#define SV_IGNORE					73
#define SV_WAYPOINTS				74
#define SV_SHOWMOTD					75
#define SV_SETCHAR_WPS				76
#define SV_CLEARBOX					77
#define SV_SETCHAR_TOK				78

#define SV_MOTD						82
#define SV_MOTD0					82
#define SV_MOTD1					83
#define SV_MOTD2					84
#define SV_MOTD3					85

#define SV_LOG						90
#define SV_LOG0						90
#define SV_LOG1						91
#define SV_LOG2						92
#define SV_LOG3						93
#define SV_LOG4						94
#define SV_LOG5						95
#define SV_LOG6						96
#define SV_LOG7						97
#define SV_LOG8						98
#define SV_LOG9						99

#define SV_SETMAP					128

static char secret[256] = {"\
Ifhjf64hH8sa,-#39ddj843tvxcv0434dvsdc40G#34Trefc349534Y5#34trecerr943\
5#erZt#eA534#5erFtw#Trwec,9345mwrxm gerte-534lMIZDN(/dn8sfn8&DBDB/D&s\
8efnsd897)DDzD'D'D''Dofs,t0943-rg-gdfg-gdf.t,e95.34u.5retfrh.wretv.56\
9v4#asf.59m(D)/ND/DDLD;gd+dsa,fw9r,x  OD(98snfsfa"};
static int sock = -1;

char passwd[15] = {0};
char *logout_reason[] = {
	"unknown",																				//  0
	"Client failed challenge.",																//  1
	"Client was idle too long.",															//  2
	"No room to drop character.",															//  3
	"Invalid parameters.",																	//  4
	"Character already active or no player character.",										//  5
	"Invalid password.",																	//  6
	"Client too slow.",																		//  7
	"Receive failure.",																		//  8
	"Server is being shutdown.",															//  9
	"You entered a Tavern.",																// 10
	"Client version too old. Update needed.",												// 11
	"Aborting on user request.",															// 12
	"this should never show up",															// 13
	"You have been banned for an hour. Enhance your social behavior before you come back."	// 14
};

#endif // NETWORK_H
/* END OF FILE */