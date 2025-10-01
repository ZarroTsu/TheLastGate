/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include "server.h"

// Font Colors:
// 0 = Red
// 1 = Yellow
// 2 = Green
// 3 = Blue
// 4 = Pink
// 5 = Orange
// 6 = Lime
// 7 = Skyblue
// 8 = Violet
// 9 = White

void look_god_details(int cn, int in)
{
	if (!IS_GOD(cn)) return;
	
	do_char_log(cn, 2, "ID %d, Tmp %d, Ori %d, Val %dG%dS.\n", in, it[in].temp, it[in].orig_temp, it[in].value / 100, it[in].value % 100);
	do_char_log(cn, 2, "dri %d, act %d, spr %d/%d\n", it[in].driver, it[in].active, it[in].sprite[I_I], it[in].sprite[I_A]);
	do_char_log(cn, 2, "max_age %d/%d, cur_age %d/%d\n", it[in].max_age[I_I], it[in].max_age[I_A], it[in].current_age[I_I], it[in].current_age[I_A]);
	do_char_log(cn, 2, "max_dam %d, cur_dam %d\n", it[in].max_damage, it[in].current_damage);
}

void look_builder_details(int cn, int in)
{
	if (!IS_BUILDING(cn)) return;
	
	do_char_log(cn, 1, "Tmp %d. Spr %d/%d.\n", it[in].temp, it[in].sprite[I_I], it[in].sprite[I_A]);
	do_char_log(cn, 1, "Age %d of %d (%d of %d).\n", it[in].current_age[I_I], it[in].max_age[I_I], it[in].current_age[I_A], it[in].max_age[I_A]);
	do_char_log(cn, 1, "Dam %d of %d. Act %d of %d.\n", it[in].current_damage, it[in].max_damage, it[in].active, it[in].duration);
	do_char_log(cn, 1, "Driver=%d [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d].\n", it[in].driver, 
		it[in].data[0], it[in].data[1], it[in].data[2], it[in].data[3], it[in].data[4],
		it[in].data[5], it[in].data[6], it[in].data[7], it[in].data[8], it[in].data[9]);
}

void look_appraisal(int cn, int in)
{
	int v = it[in].value/2;
	
	if (!(ch[cn].flags & CF_APPRAISE)) return;
	if (ch[cn].flags & CF_APPR_OFF) return;
	if (!(it_temp[it[in].temp].flags & IF_TAKE)) return;
	if (!v) return;
	
	do_char_log(cn, 6, "You estimate this could sell for %dG %dS.\n", v/100, v%100);
}

void look_damage_state(int cn, int in)
{
	int act = (it[in].active)?1:0;
	
	if (it[in].max_age[act] || it[in].max_damage)
	{
		if (it[in].damage_state==0)      do_char_log(cn, 1, "It's in perfect condition.\n");
		else if (it[in].damage_state==1) do_char_log(cn, 1, "It's showing signs of age.\n");
		else if (it[in].damage_state==2) do_char_log(cn, 1, "It's fairly old.\n");
		else if (it[in].damage_state==3) do_char_log(cn, 1, "It is old.\n");
		else if (it[in].damage_state==4) do_char_log(cn, 0, "It is very old and battered.\n");
	}
}

void look_compare_items(int cn, int in)
{
	int in2 = ch[cn].citem;
	
	if (!IS_SANEITEM(in2) || in == in2) return;
	
	do_char_log(cn, 1, " \n");		                 do_char_log(cn, 1, "You compare it with a %s:\n", it[in2].name);
	if (it[in].weapon[I_I]>it[in2].weapon[I_I])      do_char_log(cn, 1, "A %s is the better weapon.\n", it[in].name);
	else if (it[in].weapon[I_I]<it[in2].weapon[I_I]) do_char_log(cn, 1, "A %s is the better weapon.\n", it[in2].name);
	else                                             do_char_log(cn, 1, "No difference as a weapon.\n");
	if (it[in].armor[I_I]>it[in2].armor[I_I])        do_char_log(cn, 1, "A %s is the better armor.\n", it[in].name);
	else if (it[in].armor[I_I]<it[in2].armor[I_I])   do_char_log(cn, 1, "A %s is the better armor.\n", it[in2].name);
	else                                             do_char_log(cn, 1, "No difference as armor.\n");
	return;
}

void look_can_mod(int cn, int in)
{
	int whet = 0, soul = 0, ench = 0, augm = 0, godw = 0;
	int cata = 0, focu = 0, ease = 0, lega = 0, osiw = 0;
	int coru = 0, isss = (it[in].driver==68)?1:0;
	
	if (!it[in].placement) return;
	
	whet = abs(is_valid_smith_item(in));
	if (whet==2 || (it[in].flags & IF_WHETSTONED)) whet = 0;
	
	if (WAS_SOULSTONED(in)) soul = 0;
	else if (!CAN_SOULSTONE(in)) soul = -1;
	else soul = 1;
	
	if (WAS_ENCHANTED(in)) ench = 0;
	else if (!CAN_ENCHANT(in)) ench = -1;
	else ench = 1;
	
	if (whet>0)
		do_char_log(cn, 5, "Can be Whetstoned.\n");
	
	if (IS_OSIRWEAP(in) && it[in].stack)
		do_char_log(cn, 3, "May be further enhanced by Osiris.\n");
	else if (soul>0)
		do_char_log(cn, 3, "Can be Soulstoned.\n");
	else if (soul<0)
		do_char_log(cn, 0, "Cannot be Soulstoned.\n");
	
	if (ench>0)
		do_char_log(cn, 8, "Can be Enchanted.\n");
	else if (ench<0)
		do_char_log(cn, 0, "Cannot be Enchanted.\n");
}

void look_has_mod(int cn, int in)
{
	int whet = 0, soul = 0, ench = 0, augm = 0, godw = 0;
	int cata = 0, focu = 0, ease = 0, lega = 0, osiw = 0;
	int coru = 0, isss = IS_SOULSTONE(in)?1:0;
	
	if (WAS_SOULSTONED(in))  { if (isss) cata++; else soul++;	}
	if (WAS_ENCHANTED(in))   { if (isss) focu++; else ench++;	}
	if (WAS_WHETSTONED(in))  whet++;
	if (WAS_AUGMENTED(in))   augm++;
	if (WAS_CORRUPTED(in))   coru++;
	if (WAS_MADEEASEUSE(in)) ease++;
	
	if (IS_GODWEAPON(in))                godw++;
	if (IS_OSIRWEAP(in) && it[in].stack) osiw++;
	if (it[in].flags & IF_LEGACY)        lega++;
	
	if (whet) do_char_log(cn, 5, "Has been Whetstoned.\n");
	if (soul) do_char_log(cn, 3, "Has been Soulstoned.\n");
	if (cata) do_char_log(cn, 3, "Has been Catalyzed.\n");
	if (ench) do_char_log(cn, 8, "Has been Enchanted.\n");
	if (focu) do_char_log(cn, 8, "Has been Focused.\n");
	if (augm) do_char_log(cn, 7, "Has been Augmented.\n");
	if (ease) do_char_log(cn, 6, "Has been made easier to use.\n");
	if (coru) do_char_log(cn, 4, "Has been Corrupted.\n");
	
	if (godw && !lega)
		do_char_log(cn, 2, "Level %d. Next level in %d kills.\n", 
			max(1, getitemrank(in, 0)), rank2points(max(1, getitemrank(in, 0)))/250 - it[in].cost);
	
	if (osiw && !lega && it[in].stack)
		do_char_log(cn, 2, "%d more blessing%s. Next in %d kills.\n", 
			it[in].stack, (it[in].stack>1?"s":""), rank2points(max(0, getitemrank(in, it[in].data[1])))/250*it[in].data[1] - it[in].cost);
	
	if (lega)
		do_char_log(cn, 9, "Legacy item.\n");
}

void look_identify(int cn, int in)
{
	if (it[in].flags & IF_IDENTIFIED)
	{
		item_info(cn, in, 1);
		look_can_mod(cn, in);
	}
	
	look_has_mod(cn, in);
	look_damage_state(cn, in);
	look_appraisal(cn, in);
}

void look_item_details(int cn, int in)
{
	if (it[in].driver == 113 && !IS_GOD(cn)) return; // ignore for contracts
	
	look_builder_details(cn, in);
	look_god_details(cn, in);
	look_compare_items(cn, in);
	
	if (it[in].driver != 68 && (it[in].flags & IF_LOOKSPECIAL)) return;
	
	look_identify(cn, in);
}

void look_extra(int cn, int in)
{
	int temp, n, m=0, mm=0;
	
	if (!(temp = it[in].temp) && it[in].orig_temp) temp = it[in].orig_temp;
	
	switch (temp)
	{
	// -------- TOWER ITEMS --------
	case IT_TW_CROWN:   do_char_log(cn, FN_TOWER, DESC_TW_CROWN);   break; // Crown of the First King
	case IT_TW_CLOAK:   do_char_log(cn, FN_TOWER, DESC_TW_CLOAK);   break; // Cloak of Shadows
	case IT_TW_DREAD:   do_char_log(cn, FN_TOWER, DESC_TW_DREAD);   break; // The Dreadplate
	case IT_TW_DOUSER:  do_char_log(cn, FN_TOWER, DESC_TW_DOUSER);  break; // Douser Gauntlets
	case IT_TW_MARCH:   do_char_log(cn, FN_TOWER, DESC_TW_MARCH);   break; // Commander's March
	case IT_TW_BBELT:   do_char_log(cn, FN_TOWER, DESC_TW_BBELT);   break; // The Black Belt
	case IT_TW_OUTSIDE: do_char_log(cn, FN_TOWER, DESC_TW_OUTSIDE); break; // Outsider's Eye
	case IT_TW_HEAVENS: do_char_log(cn, FN_TOWER, DESC_TW_HEAVENS); break; // Fists of the Heavens
	
	// -------- ABYSS ITEMS --------
	case IT_TW_ACEDIA:   do_char_log(cn, FN_ABYSS, DESC_TW_ACEDIA);   break; // Sin Dagger
	case IT_TW_IRA:      do_char_log(cn, FN_ABYSS, DESC_TW_IRA);      break; // Sin Staff
	case IT_TW_INVIDIA:  do_char_log(cn, FN_ABYSS, DESC_TW_INVIDIA);  break; // Sin Spear
	case IT_TW_GULA:     do_char_log(cn, FN_ABYSS, DESC_TW_GULA);     break; // Sin Sword
	case IT_TW_LUXURIA:  do_char_log(cn, FN_ABYSS, DESC_TW_LUXURIA);  break; // Sin Axe
	case IT_TW_AVARITIA: do_char_log(cn, FN_ABYSS, DESC_TW_AVARITIA); break; // Sin Twohander
	case IT_TW_SUPERBIA: do_char_log(cn, FN_ABYSS, DESC_TW_SUPERBIA); break; // Sin Greataxe
	case IT_TW_SINBIND:  do_char_log(cn, FN_ABYSS, DESC_TW_SINBIND);  temp = it[in].data[2]; break; // Sinbinder ring
	
	// -------- SIGNET RINGS --------
	case IT_SIGN_SKUA: do_char_log(cn, FN_SIGNT, DESC_SIGN_SKUA); do_char_log(cn, FN_O, DESC_O); break;
	case IT_SIGN_SHOU: do_char_log(cn, FN_SIGNT, DESC_SIGN_SHOU); do_char_log(cn, FN_O, DESC_O); break;
	case IT_SIGN_SLAY: do_char_log(cn, FN_SIGNT, DESC_SIGN_SLAY); do_char_log(cn, FN_O, DESC_O); break;
	case IT_SIGN_STOR: do_char_log(cn, FN_SIGNT, DESC_SIGN_STOR); do_char_log(cn, FN_O, DESC_O); break;
	case IT_SIGN_SICK: do_char_log(cn, FN_SIGNT, DESC_SIGN_SICK); do_char_log(cn, FN_O, DESC_O); break;
	case IT_SIGN_SHAD: do_char_log(cn, FN_SIGNT, DESC_SIGN_SHAD); do_char_log(cn, FN_O, DESC_O); break;
	case IT_SIGN_SPAR: do_char_log(cn, FN_SIGNT, DESC_SIGN_SPAR); do_char_log(cn, FN_O, DESC_O); break;
	case IT_SIGN_SYMM: do_char_log(cn, FN_SIGNT, DESC_SIGN_SYMM); do_char_log(cn, FN_O, DESC_O); break;
	case IT_SIGN_SONG: do_char_log(cn, FN_SIGNT, DESC_SIGN_SONG); do_char_log(cn, FN_O, DESC_O); break;
	case IT_SIGN_SCRE: do_char_log(cn, FN_SIGNT, DESC_SIGN_SCRE); do_char_log(cn, FN_O, DESC_O); break;
	
	case IT_SIGNET_TE: case IT_SIGNET_MR: case IT_SIGNET_HA: case IT_SIGNET_AT:
	case IT_SIGNET_SK: case IT_SIGNET_WA: case IT_SIGNET_SO: case IT_SIGNET_SU:	
	case IT_SIGNET_AH: case IT_SIGNET_SE: case IT_SIGNET_BR: case IT_SIGNET_LY:
	case IT_ICELOTUS: do_char_log(cn, FN_O, DESC_O); break;
	
	// -------- SPECIAL GLOVES --------
	case IT_GL_SERPENT: do_char_log(cn, FN_EX, DESC_GL_SERPENT); break;
	case IT_GL_BURNING: do_char_log(cn, FN_EX, DESC_GL_BURNING); break;
	case IT_GL_SHADOW:  do_char_log(cn, FN_EX, DESC_GL_SHADOW);  break;
	case IT_GL_CHILLED: do_char_log(cn, FN_EX, DESC_GL_CHILLED); break;
	case IT_GL_CURSED:  do_char_log(cn, FN_EX, DESC_GL_CURSED);  break;
	case IT_GL_TITANS:  do_char_log(cn, FN_EX, DESC_GL_TITANS);  break;
	case IT_GL_BLVIPER: do_char_log(cn, FN_EX, DESC_GL_BLVIPER); break;
		
	// -------- SPECIAL ITEMS --------
	case IT_BT_NATURES: do_char_log(cn, FN_EX, DESC_BT_NATURES); break;
	case IT_RINGWARMTH: do_char_log(cn, FN_EX, DESC_RINGWARMTH); do_char_log(cn, FN_O, DESC_O); break;
	case IT_LIZCROWN:   do_char_log(cn, FN_EX, DESC_LIZCROWN);   break;
		
	// -------- SPECIAL AMULETS --------
	case IT_ANKHAMULET: do_char_log(cn, FN_EX, DESC_ANKHAMULET); break;
	case IT_AMBERANKH:  do_char_log(cn, FN_EX, DESC_AMBERANKH);  break;
	case IT_TURQUANKH:  do_char_log(cn, FN_EX, DESC_TURQUANKH);  break;
	case IT_GARNEANKH:  do_char_log(cn, FN_EX, DESC_GARNEANKH);  break;
	case IT_TRUEANKH:   do_char_log(cn, FN_EX, DESC_TRUEANKH);   break;
	case IT_AM_BLOODS:  do_char_log(cn, FN_EX, DESC_AM_BLOODS);  break;
	case IT_AM_VERDANT: do_char_log(cn, FN_EX, DESC_AM_VERDANT); break;
	case IT_AM_SEABREZ: do_char_log(cn, FN_EX, DESC_AM_SEABREZ); break;
	case IT_AM_TRUESUN: do_char_log(cn, FN_EX, DESC_AM_TRUESUN); break;
	case IT_AM_FALMOON: do_char_log(cn, FN_EX, DESC_AM_FALMOON); break;
	case IT_AM_ECLIPSE: do_char_log(cn, FN_EX, DESC_AM_ECLIPSE); break;
	case IT_GAMBLERFAL: do_char_log(cn, FN_EX, DESC_GAMBLERFAL); break;
		
	// -------- SPECIAL BELTS --------
	case IT_BL_SOLAR:   case IT_BL_LUNAR: 
	case IT_BL_ECLIPSE: do_char_log(cn, FN_EX, DESC_BL_ECLIPSE); break;
	case IT_WHITEBELT:  do_char_log(cn, FN_EX, DESC_WHITEBELT);  break; // The White Belt
	
	// -------- CASINO ITEMS --------
	case IT_MISERRING: do_char_log(cn, FN_EX, DESC_MISERRING); break; // Miser Ring
	case IT_FORTERING: do_char_log(cn, FN_EX, DESC_FORTERING); break; // Forte Ring

	// -------- DRINK ITEMS --------
	case BUF_IT_PIGS: do_char_log(cn, FN_EX, DESC_IT_PIGS); break;
	case BUF_IT_AALE: do_char_log(cn, FN_EX, DESC_IT_AALE); break;
	case BUF_IT_DRAG: do_char_log(cn, FN_EX, DESC_IT_DRAG); break;
	case BUF_IT_MANA: do_char_log(cn, FN_EX, DESC_IT_MANA); break;
	
	// -------- SPECIAL BOOKS --------
	case IT_BOOK_ALCH: case IT_IMBK_ALCH: do_char_log(cn, FN_EX, DESC_BOOK_ALCH); break;
	case IT_BOOK_HOLY: case IT_IMBK_HOLY: do_char_log(cn, FN_EX, DESC_BOOK_HOLY); break;
	case IT_BOOK_TRAV: case IT_IMBK_TRAV: do_char_log(cn, FN_EX, DESC_BOOK_TRAV); break;
	case IT_BOOK_ADVA: case IT_IMBK_ADVA: do_char_log(cn, FN_EX, DESC_BOOK_ADVA); break;
	case IT_BOOK_MALT: case IT_IMBK_MALT: do_char_log(cn, FN_EX, DESC_BOOK_MALT); break;
	case IT_BOOK_DAMO: case IT_IMBK_DAMO: do_char_log(cn, FN_EX, DESC_BOOK_DAMO); break;
	case IT_BOOK_SHIV: case IT_IMBK_SHIV: do_char_log(cn, FN_EX, DESC_BOOK_SHIV); break;
	case IT_BOOK_BISH: case IT_IMBK_BISH: do_char_log(cn, FN_EX, DESC_BOOK_BISH); break;
	case IT_BOOK_GREA: case IT_IMBK_GREA: do_char_log(cn, FN_EX, DESC_BOOK_GREA); break;
	case IT_BOOK_PROD: case IT_IMBK_PROD: do_char_log(cn, FN_EX, DESC_BOOK_PROD); break;
	case IT_BOOK_VENO: do_char_log(cn, FN_EX, DESC_BOOK_VENO); break;
	case IT_BOOK_NECR: do_char_log(cn, FN_EX, DESC_BOOK_NECR); break;
	case IT_BOOK_BURN: do_char_log(cn, FN_EX, DESC_BOOK_BURN); break;
	case IT_BOOK_DEVI: do_char_log(cn, FN_EX, DESC_BOOK_DEVI); break;
	case IT_BOOK_VERD: do_char_log(cn, FN_EX, DESC_BOOK_VERD); break;
	case IT_BOOK_GRAN: do_char_log(cn, FN_EX, DESC_BOOK_GRAN); break;
	
	// -------- SPECIAL ITEMS --------
	case IT_WB_STONEDAGG:  do_char_log(cn, FN_EX, DESC_STONEDAGG);  break;
	case IT_WP_LIFESPRIG:  
	case IT_WB_LIFESPRIG:  do_char_log(cn, FN_EX, DESC_LIFESPRIG);  break;
	case IT_WB_SPIDERFANG: do_char_log(cn, FN_EX, DESC_SPIDERFANG); break;
	case IT_WP_MAGEMASH:   
	case IT_WB_MAGEMASH:   do_char_log(cn, FN_EX, DESC_MAGEMASH);   break;
	case IT_WP_BLOODLET:   
	case IT_WB_BLOODLET:   do_char_log(cn, FN_EX, DESC_BLOODLET);   break;
	case IT_WP_GEMCUTTER:  do_char_log(cn, FN_EX, DESC_GEMCUTTER);  break;
	case IT_WP_QUICKSILV:  do_char_log(cn, FN_EX, DESC_QUICKSILV);  break;
	case IT_WB_JANESOBLIT: do_char_log(cn, FN_EX, DESC_JANESOBLIT); break;
	case IT_WB_RATTANBO:   do_char_log(cn, FN_EX, DESC_RATTANBO);   break;
	case IT_WP_PUTRIDIRE:  
	case IT_WB_PUTRIDIRE:  do_char_log(cn, FN_EX, DESC_PUTRIDIRE);  break;
	case IT_WP_STARLIGHT:  do_char_log(cn, FN_EX, DESC_STARLIGHT);  break;
	case IT_WP_BLOODYSCY:  do_char_log(cn, FN_EX, DESC_BLOODYSCY);  break;
	case IT_WB_GOLDGLAIVE: do_char_log(cn, FN_EX, DESC_GOLDGLAIVE); break;
	case IT_WP_KELPTRID:   
	case IT_WB_KELPTRID:   do_char_log(cn, FN_EX, DESC_KELPTRID);   break;
	case IT_WP_FLAGBEAR:   do_char_log(cn, FN_EX, DESC_FLAGBEAR);   break;
	case IT_WB_FROSTGLASS: do_char_log(cn, FN_EX, DESC_FROSTGLASS); break;
	case IT_WP_PHALANX:    
	case IT_WB_PHALANX:    do_char_log(cn, FN_EX, DESC_PHALANX);    break;
	case IT_WP_RISINGPHO:  
	case IT_WB_RISINGPHO:  do_char_log(cn, FN_EX, DESC_RISINGPHO);  break;
	case IT_WP_THEWALL:    do_char_log(cn, FN_EX, DESC_THEWALL);    break;
	case IT_WB_BARBSWORD:  do_char_log(cn, FN_EX, DESC_BARBSWORD);  break;
	case IT_WP_LAMEDARG:   if (it[in].data[0] < REQ_LAME)
	                       do_char_log(cn, FN_EX, DESC_LAMEDARG);
	                       else
	                       do_char_log(cn, FN_WO, DESC_LAMEFULL);   break;
	case IT_WP_WHITEODA:   
	case IT_WB_WHITEODA:   do_char_log(cn, FN_EX, DESC_WHITEODA);   break;
	case IT_WP_EXCALIBUR:  do_char_log(cn, FN_EX, DESC_EXCALIBUR);  break;
	case IT_WB_BEINESTOC:  do_char_log(cn, FN_EX, DESC_BEINESTOC);  break;
	case IT_WP_BLACKTAC:   
	case IT_WB_BLACKTAC:   do_char_log(cn, FN_EX, DESC_BLACKTAC);   break;
	case IT_WP_EVERGREEN:  do_char_log(cn, FN_EX, DESC_EVERGREEN);  break;
	case IT_WB_LIONSPAWS:  do_char_log(cn, FN_EX, DESC_LIONSPAWS);  break;
	case IT_WP_CRIMRIP:    
	case IT_WB_CRIMRIP:    do_char_log(cn, FN_EX, DESC_CRIMRIP);    break;
	case IT_WP_SOVERIGNS:  do_char_log(cn, FN_EX, DESC_SOVERIGNS);  break;
	case IT_WP_CRESSUN:    
	case IT_WB_CRESSUN:    do_char_log(cn, FN_EX, DESC_CRESSUN);    break;
	case IT_WP_MJOLNIR:    do_char_log(cn, FN_EX, DESC_MJOLNIR);    break;
	case IT_WB_LAVA2HND:   do_char_log(cn, FN_EX, DESC_LAVA2HND);   break;
	case IT_WP_GILDSHINE:  
	case IT_WB_GILDSHINE:  do_char_log(cn, FN_EX, DESC_GILDSHINE);  break;
	case IT_WB_BURN2HND:   do_char_log(cn, FN_EX, DESC_BURN2HND);   break;
	case IT_WB_ICE2HND:    do_char_log(cn, FN_EX, DESC_ICE2HND);    break;
	case IT_WP_COLDSTEEL:  do_char_log(cn, FN_EX, DESC_COLDSTEEL);  break;
	case IT_WP_CROSSBLAD:  do_char_log(cn, FN_EX, DESC_CROSSBLAD);  break;
	case IT_WP_BRONCHIT:   
	case IT_WB_BRONCHIT:   do_char_log(cn, FN_EX, DESC_BRONCHIT);   break;
	case IT_WP_VOLCANF:    do_char_log(cn, FN_EX, DESC_VOLCANF);    break;
	case IT_WB_VIKINGMALT: do_char_log(cn, FN_EX, DESC_VIKINGMALT); break;
	case IT_WP_GUNGNIR:    do_char_log(cn, FN_EX, DESC_GUNGNIR);    break;
	
	case IT_BONEARMOR:     do_char_log(cn, FN_EX, DESC_BONEARMOR);  break; // Bone Armor
	
	case IT_OS_BRV: case IT_OS_WIL: case IT_OS_INT: case IT_OS_AGL: case IT_OS_STR:
		if (temp==IT_OS_BRV) do_char_log(cn, 8, "Grants an implicit +1 to Braveness.\n");
		if (temp==IT_OS_WIL) do_char_log(cn, 8, "Grants an implicit +1 to Willpower.\n");
		if (temp==IT_OS_INT) do_char_log(cn, 8, "Grants an implicit +1 to Intuition.\n");
		if (temp==IT_OS_AGL) do_char_log(cn, 8, "Grants an implicit +1 to Agility.\n");
		if (temp==IT_OS_STR) do_char_log(cn, 8, "Grants an implicit +1 to Strength.\n");
		do_char_log(cn, 1, " \n");
		for (n=0;n<5;n++) m += ch[cn].attrib[n][1];
		do_char_log(cn, 6, "You have used %d out of 10 greater attribute scrolls.\n", m);
		break;
		
	case IT_OS_SK:
		do_char_log(cn, 8, "Grants an implicit +1 to %s.\n", skilltab[it[in].data[1]].name);
		do_char_log(cn, 1, " \n");
		for (n=0;n<50;n++) { m += ch[cn].skill[n][1]; mm = mm + (ch[cn].skill[n][0]?1:0); }
		do_char_log(cn, 6, "You have used %d out of %d greater skill scrolls.\n", m, mm);
		break;
	
	default: break;
	}
	
	switch (temp)
	{
	// -------- TAROT CARDS --------
	case IT_CH_FOOL:     if (!IS_SOULSTONED(in) || !IS_ENCHANTED(in))
	                     do_char_log(cn, FN_TAROT, DESC_FOOL);     break;
	case IT_CH_MAGI:     do_char_log(cn, FN_TAROT, DESC_MAGI);     break;
	case IT_CH_PREIST:   do_char_log(cn, FN_TAROT, DESC_PREIST);   break;
	case IT_CH_EMPRESS:  do_char_log(cn, FN_TAROT, DESC_EMPRESS);  break;
	case IT_CH_EMPEROR:  do_char_log(cn, FN_TAROT, DESC_EMPEROR);  break;
	case IT_CH_HEIROPH:  do_char_log(cn, FN_TAROT, DESC_HEIROPH);  break;
	case IT_CH_LOVERS:   do_char_log(cn, FN_TAROT, DESC_LOVERS);   break;
	case IT_CH_CHARIOT:  do_char_log(cn, FN_TAROT, DESC_CHARIOT);  break;
	case IT_CH_STRENGTH: do_char_log(cn, FN_TAROT, DESC_STRENGTH); break;
	case IT_CH_HERMIT:   do_char_log(cn, FN_TAROT, DESC_HERMIT);   break;
	case IT_CH_WHEEL:    do_char_log(cn, FN_TAROT, DESC_WHEEL);    break;
	case IT_CH_JUSTICE:  do_char_log(cn, FN_TAROT, DESC_JUSTICE);  break;
	case IT_CH_HANGED:   do_char_log(cn, FN_TAROT, DESC_HANGED);   break;
	case IT_CH_DEATH:    do_char_log(cn, FN_TAROT, DESC_DEATH);    break;
	case IT_CH_TEMPER:   do_char_log(cn, FN_TAROT, DESC_TEMPER);   break;
	case IT_CH_DEVIL:    do_char_log(cn, FN_TAROT, DESC_DEVIL);    break;
	case IT_CH_TOWER:    do_char_log(cn, FN_TAROT, DESC_TOWER);    break;
	case IT_CH_STAR:     do_char_log(cn, FN_TAROT, DESC_STAR);     break;
	case IT_CH_MOON:     do_char_log(cn, FN_TAROT, DESC_MOON);     break;
	case IT_CH_SUN:      do_char_log(cn, FN_TAROT, DESC_SUN);      break;
	case IT_CH_JUDGE:    do_char_log(cn, FN_TAROT, DESC_JUDGE);    break;
	case IT_CH_WORLD:    do_char_log(cn, FN_TAROT, DESC_WORLD);    break;
	
	case IT_CH_FOOL_R:   do_char_log(cn, FN_TAROT, DESC_FOOL_R);   break;
	case IT_CH_MAGI_R:   do_char_log(cn, FN_TAROT, DESC_MAGI_R);   break;
	case IT_CH_PREIST_R: do_char_log(cn, FN_TAROT, DESC_PREIST_R); break;
	case IT_CH_EMPRES_R: do_char_log(cn, FN_TAROT, DESC_EMPRES_R); break;
	case IT_CH_EMPERO_R: do_char_log(cn, FN_TAROT, DESC_EMPERO_R); break;
	case IT_CH_HEIROP_R: do_char_log(cn, FN_TAROT, DESC_HEIROP_R); break;
	case IT_CH_LOVERS_R: do_char_log(cn, FN_TAROT, DESC_LOVERS_R); break;
	case IT_CH_CHARIO_R: do_char_log(cn, FN_TAROT, DESC_CHARIO_R); break;
	case IT_CH_STRENG_R: do_char_log(cn, FN_TAROT, DESC_STRENG_R); break;
	case IT_CH_HERMIT_R: do_char_log(cn, FN_TAROT, DESC_HERMIT_R); break;
	case IT_CH_WHEEL_R:  do_char_log(cn, FN_TAROT, DESC_WHEEL_R);  break;
	case IT_CH_JUSTIC_R: do_char_log(cn, FN_TAROT, DESC_JUSTIC_R); break;
	case IT_CH_HANGED_R: do_char_log(cn, FN_TAROT, DESC_HANGED_R); break;
	case IT_CH_DEATH_R:  do_char_log(cn, FN_TAROT, DESC_DEATH_R);  break;
	case IT_CH_TEMPER_R: do_char_log(cn, FN_TAROT, DESC_TEMPER_R); break;
	case IT_CH_DEVIL_R:  do_char_log(cn, FN_TAROT, DESC_DEVIL_R);  break;
	case IT_CH_TOWER_R:  do_char_log(cn, FN_TAROT, DESC_TOWER_R);  break;
	case IT_CH_STAR_R:   do_char_log(cn, FN_TAROT, DESC_STAR_R);   break;
	case IT_CH_MOON_R:   do_char_log(cn, FN_TAROT, DESC_MOON_R);   break;
	case IT_CH_SUN_R:    do_char_log(cn, FN_TAROT, DESC_SUN_R);    break;
	case IT_CH_JUDGE_R:  do_char_log(cn, FN_TAROT, DESC_JUDGE_R);  break;
	case IT_CH_WORLD_R:  do_char_log(cn, FN_TAROT, DESC_WORLD_R);  break;
		
	default: break;
	}
	
	if (it[in].flags & IF_ENCHANTED) switch (it[in].enchantment)
	{
	// -------- REGULAR ENCHANTMENTS --------
	case  1: do_char_log(cn, 8, "8%% more total Armor Value.\n"); break;
	case  2: do_char_log(cn, 8, "Healing items are 25%% more effective.\n"); break;
	case  3: do_char_log(cn, 8, "Cannot trigger floor traps.\n"); break;
	case  4: do_char_log(cn, 8, "20%% less effect of debuffs on you.\n"); break;
	case  5: do_char_log(cn, 8, "%d%% more total Braveness.\n", 3 * (IS_TWOHAND(in)?2:1)); break;
	case  6: do_char_log(cn, 8, "Stealth is 25%% stronger while moving.\n"); break;
	case  7: do_char_log(cn, 8, "20%% more effect of Weakens you inflict.\n"); break;
	case  8: do_char_log(cn, 8, "Weakens inflicted on you are 80%% weaker.\n"); break;
	case  9: do_char_log(cn, 8, "Reduces Healing Sickness duration on you by 25%%.\n"); break;
	case 10: do_char_log(cn, 8, "Cannot step on death traps.\n"); break;
	case 11: do_char_log(cn, 8, "10%% of total Armor Value is granted as additional Resistance.\n"); break;
	case 12: do_char_log(cn, 8, "%d%% more total Willpower.\n", 3 * (IS_TWOHAND(in)?2:1)); break;
	case 13: do_char_log(cn, 8, "20%% more effect of Slows you inflict.\n"); break;
	case 14: do_char_log(cn, 8, "Slows inflicted on you are 80%% weaker.\n"); break;
	case 15: do_char_log(cn, 8, "Cannot lose focus.\n"); break;
	case 16: do_char_log(cn, 8, "20%% of damage taken from hits is dealt to your Endurance instead.\n"); break;
	case 17: do_char_log(cn, 8, "Grants infravision.\n"); break;
	case 18: do_char_log(cn, 8, "20%% more maximum Endurance.\n"); break;
	case 19: do_char_log(cn, 8, "%d%% more total Intuition.\n", 3 * (IS_TWOHAND(in)?2:1)); break;
	case 20: do_char_log(cn, 8, "20%% more effect of Curses you inflict.\n"); break;
	case 21: do_char_log(cn, 8, "Curses inflicted on you are 80%% weaker.\n"); break;
	case 22: do_char_log(cn, 8, "20%% less cost of skills and spells.\n"); break;
	case 23: do_char_log(cn, 8, "20%% of damage taken from hits is dealt to your Mana instead.\n"); break;
	case 24: do_char_log(cn, 8, "10%% of total Armor Value is granted as additional Immunity.\n"); break;
	case 25: do_char_log(cn, 8, "%d%% more total Agility.\n", 3 * (IS_TWOHAND(in)?2:1)); break;
	case 26: do_char_log(cn, 8, "Movement speed cannot fall below 1.50.\n"); break;
	case 27: do_char_log(cn, 8, "20%% more effect of Poisons you inflict.\n"); break;
	case 28: do_char_log(cn, 8, "Stealth is 25%% stronger while idle.\n"); break;
	case 29: do_char_log(cn, 8, "%d%% more total Strength.\n", 3 * (IS_TWOHAND(in)?2:1)); break;
	case 30: do_char_log(cn, 8, "20%% more effect of Bleeds you inflict.\n"); break;
	case 31: do_char_log(cn, 8, "20%% more effect of Blinds you inflict.\n"); break;
	case 32: do_char_log(cn, 8, "Grants unreduced regeneration rates while walking.\n"); break;
	case 33: do_char_log(cn, 8, "Blinds inflicted on you are 80%% weaker.\n"); break;
	case 34: do_char_log(cn, 8, "Item is indestructible.\n"); break;
	case 35: do_char_log(cn, 8, "8%% chance to half damage taken.\n"); break;
	case 36: do_char_log(cn, 8, "Half of your Meditate score is also applied to your Hitpoints.\n"); break;
	case 37: do_char_log(cn, 8, "5%% additional chance to avoid being hit.\n"); break;
	case 38: do_char_log(cn, 8, "Restore %d Mana upon hitting an enemy.\n", (IS_TWOHAND(in)?2:1)); break;
	case 39: do_char_log(cn, 8, "Restore 2 Mana upon being hit by an enemy.\n"); break;
	case 40: do_char_log(cn, 8, "20%% more total Movement Speed.\n"); break;
	case 41: do_char_log(cn, 8, "%d%% additional chance to hit.\n", 5 * (IS_TWOHAND(in)?2:1)); break;
	case 42: do_char_log(cn, 8, "Restore %d Endurance upon hitting an enemy.\n", (IS_TWOHAND(in)?2:1)); break;
	case 43: do_char_log(cn, 8, "Restore 2 Endurance upon being hit by an enemy.\n"); break;
	case 44: do_char_log(cn, 8, "Movement Speed is doubled while underwater.\n"); break;
	case 45: do_char_log(cn, 8, "Half of your Rest score is also applied to your Mana.\n"); break;
	case 46: do_char_log(cn, 8, "You can always escape from combat.\n"); break;
	case 47: do_char_log(cn, 8, "Reduces extra damage taken from enemy critical hits by 50%%.\n"); break;
	case 48: do_char_log(cn, 8, "30%% more total Thorns score.\n"); break;
	case 49: do_char_log(cn, 8, "Restore %d hitpoints upon hitting an enemy.\n", (IS_TWOHAND(in)?2:1)); break;
	case 50: do_char_log(cn, 8, "2%% more damage dealt.\n"); break;
	case 51: do_char_log(cn, 8, "2%% less damage taken.\n"); break;
	case 52: do_char_log(cn, 8, "Perception is 25%% stronger.\n"); break;
	case 53: do_char_log(cn, 8, "15%% reduced damage taken from damage over time.\n"); break;
	case 54: do_char_log(cn, 8, "%d%% more total attributes.\n", 2 * (IS_TWOHAND(in)?2:1)); break;
	case 55: do_char_log(cn, 8, "Total Glow score is reduced to 0.\n"); break;
	case 56: do_char_log(cn, 8, "20%% more effect of heals and regens you apply.\n"); break;
	
	// -------- SKUA ENCHANTMENTS --------
	case 57: do_char_log(cn, 7, "25%% of Armor Value granted by Magic Shield is granted as additional Weapon Value.\n"); break;
	case 58: do_char_log(cn, 7, "4%% of total Glow score is granted as additional spell modifier.\n"); break;
	case 59: do_char_log(cn, 7, "Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Slow, reducing enemy action speed.\n"); break;
	case 60: do_char_log(cn, 7, "Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Scorch, causing enemies to take additional damage.\n"); break;
	
	// -------- KWAI ENCHANTMENTS --------
	case 61: do_char_log(cn, 5, "3%% of total hit score is granted as extra parry score.\n"); break;
	case 62: do_char_log(cn, 5, "3%% of total parry score is granted as extra hit score.\n"); break;
	case 63: do_char_log(cn, 5, "Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Curse, reducing enemy attributes.\n"); break;
	case 64: do_char_log(cn, 5, "Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Weaken, reducing enemy weapon value.\n"); break;
	
	// -------- GORN ENCHANTMENTS --------
	case 65: do_char_log(cn, 6, "20%% of damage dealt with hits is also dealt to enemy mana.\n"); break;
	case 66: do_char_log(cn, 6, "Damage dealt with damage over time is restored as hitpoints at a rate equal to experience gain.\n"); break;
	case 67: do_char_log(cn, 6, "Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Poison, dealing damage over time.\n"); break;
	case 68: do_char_log(cn, 6, "Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Douse, reducing enemy spell modifier.\n"); break;
	
	// -------- PURP ENCHANTMENTS --------
	case 69: do_char_log(cn, 8, "4%% of damage dealt with hits is restored as hitpoints.\n"); break;
	case 70: do_char_log(cn, 8, "10%% of current endurance value is granted as additional damage on hit.\n"); break;
	case 71: do_char_log(cn, 8, "Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Blind, reducing enemy perception, hit and parry.\n"); break;
	case 72: do_char_log(cn, 8, "Has a 25%% chance on skill hit and a 5%% chance on melee hit to inflict Frostburn, draining enemy endurance and mana.\n"); break;
	
	// -------- OFFH ENCHANTMENTS --------
	case 73: do_char_log(cn, 9, "Restore 2 Hitpoints upon being hit by an enemy.\n"); break;
	case 74: do_char_log(cn, 9, "10%% of weapon value is granted as additional rapid mana regeneration.\n"); break;
	case 75: do_char_log(cn, 9, "Your lowest attribute score is increased by 20%%.\n"); break;
	case 76: do_char_log(cn, 9, "Reduces extra damage taken from enemy critical hits by 50%%.\n"); break;
	
	default:break;
	}
	
	if (IS_CORRUPTOR(in))
	{
		if (it[in].data[0])
		{
			do_char_log(cn, 1, "%s:\n", sk_corrupt[it[in].data[0]-1].name);
			do_char_log(cn, 4, "%s%s\n", sk_corrupt[it[in].data[0]-1].dsc1, sk_corrupt[it[in].data[0]-1].dsc2);
		}
		do_char_log(cn, 1, "Maybe you could use it on something?\n");
		return;
	}
	
	if ((it[in].flags & IF_CORRUPTED) && it[in].corruption)
	{
		do_char_log(cn, 4, "%s%s\n", sk_corrupt[it[in].corruption-1].dsc1, sk_corrupt[it[in].corruption-1].dsc2);
	}
	
	look_identify(cn, in);
}

void look_door(int cn, int in)
{
	int power;
	
	if (it[in].driver != 2) return;
	
	// If the door needs a key, it is locked.
	// If you know how to pick locks, you can summarize the door
	// This is done in increments of 20.
	// Ideally each available lockpick (if any) will also increment by 20.
	if (it[in].data[0] && (ch[cn].flags & CF_LOCKPICK))
	{
		// The difficulty of the door to pick
		power = it[in].data[2];
		
		if (!power)           do_char_log(cn, 0, "You're not sure this door can be picked...\n");
		else if (power >=100) do_char_log(cn, 1, "The lock seems extremely difficult to pick.\n");
		else if (power >= 80) do_char_log(cn, 1, "The lock seems very difficult to pick.\n");
		else if (power >= 60) do_char_log(cn, 1, "The lock seems difficult to pick.\n");
		else if (power >= 40) do_char_log(cn, 1, "The lock seems tricky to pick.\n");
		else if (power >= 20) do_char_log(cn, 1, "The lock seems simple to pick.\n");
		else                  do_char_log(cn, 1, "The lock seems very simple to pick.\n");
	}
	else if (ch[cn].flags & CF_LOCKPICK)
		do_char_log(cn, 1, "This door appears to be unlocked.\n");
}

void look_pole(int cn, int in)
{
	int percent;
	
	if (it[in].driver != 57) return;
		
	percent = min(100, 100 * (ch[cn].points_tot / 10) / (it[in].data[4] + 1));
	
	if (percent<50)      do_char_log(cn, 2, "You sense that it's far too early in your career to touch this pole.\n");
	else if (percent<70) do_char_log(cn, 2, "You sense that it's too early in your career to touch this pole.\n");
	else if (percent<90) do_char_log(cn, 2, "You sense that it's a bit early in your career to touch this pole.\n");
}

void look_chest(int cn, int in)
{
	int one_hou = TICKS*60*60;
	int one_min = TICKS*60;
	
	if (it[in].driver!=42 && it[in].driver!=59 && it[in].driver!=103 && it[in].driver!=121) return;
	if (!it[in].active) return;

	// Remaining duration is higher than 30 minutes
	if (it[in].active > one_min*30)
	{
		do_char_log(cn, 6, "This chest will refill in %d hours and %d minutes.\n", it[in].active/one_hou, (it[in].active/one_min)%60);
	}
	else
	{
		do_char_log(cn, 6, "This chest will refill in %d minutes and %d seconds.\n", it[in].active/one_min, (it[in].active/TICKS)%60);
	}
}

void look_collection(int cn, int in)
{
	int n;
	
	for (n = 0; n<9; n++)
	{
		if (it[in].data[n]) do_char_log(cn, 1, "The slot for a %s is free.\n", it_temp[it[in].data[n]].name);
	}
}

void look_spell_scroll(int cn, int in)
{
	int n = it[in].data[2];
	
	do_char_log(cn, 1, "There are %d charge%s left.\n", n, (n==1 ? "s" : ""));
}

void look_contract(int cn, int in, int desc)
{
	int cc, rank, tier, mission, fl, n, font;
	int flags[NUM_MAP_POS+NUM_MAP_NEG] = {0};
	int r1=1, r2=2, r3=3;
	
	if (!it[in].data[1]) it[in].data[1] = RANDOM(MSN_COUNT)+1;
	
	cc      = it[in].data[0];
	mission = it[in].data[1];
	tier    = it[in].data[2];
	
	if (T_OS_TREE(cc, 9)) r1 = r2 = r3 = 3;
	
	if ((fl = it[in].data[3]) && fl > 0 && fl < NUM_MAP_POS+1) flags[fl-1] = r1;
	if ((fl = it[in].data[5]) && fl > 0 && fl < NUM_MAP_POS+1) flags[fl-1] = r2;
	if ((fl = it[in].data[7]) && fl > 0 && fl < NUM_MAP_POS+1) flags[fl-1] = r3;
	if ((fl = it[in].data[4]) && fl > 0 && fl < NUM_MAP_NEG+1) flags[fl-1+NUM_MAP_POS] = r1;
	if ((fl = it[in].data[6]) && fl > 0 && fl < NUM_MAP_NEG+1) flags[fl-1+NUM_MAP_POS] = r2;
	if ((fl = it[in].data[8]) && fl > 0 && fl < NUM_MAP_NEG+1) flags[fl-1+NUM_MAP_POS] = r3;
	
	font = get_tier_font(tier);
	
	if (desc)
	{
		do_char_log(cn, 1, "A contract. It reads:\n");
		do_char_log(cn, 1, " \n");
	}
	switch (mission)
	{
		case  1: do_char_log(cn, font, MSN_CN MSN_01 "\n"); break;
		case  2: do_char_log(cn, font, MSN_CN MSN_02 "\n"); break;
		case  3: do_char_log(cn, font, MSN_CN MSN_03 "\n"); break;
		case  4: do_char_log(cn, font, MSN_CN MSN_04 "\n"); break;
		case  5: do_char_log(cn, font, MSN_CN MSN_05 "\n"); break;
		case  6: do_char_log(cn, font, MSN_CN MSN_06 "\n"); break;
		case  7: do_char_log(cn, font, MSN_CN MSN_07 "\n"); break;
		case  8: do_char_log(cn, font, MSN_CN MSN_08 "\n"); break;
		case  9: do_char_log(cn, font, MSN_CN MSN_09 "\n"); break;
		default: do_char_log(cn, font, MSN_CN MSN_00 "\n"); break;
	}
	do_char_log(cn, 1, " \n");
	if (desc || it[in].data[3])	show_map_flags(cn, flags, tier);
	if (desc) do_char_log(cn, font, "%s\n", it[in].description);
	look_god_details(cn, in);
}

void look_talisman(int cn, int in)
{
	do_char_log(cn, 6, "%s:\n", it[in].name);
	
	// No enchantment
	if (!it[in].data[0])
	{
		switch (it[in].data[1])
		{
		case  1: do_char_log(cn, 1, "A talisman socketed with a huge sapphire. A second socket remains vacant.\n"); break;
		case  2: do_char_log(cn, 1, "A talisman socketed with a huge ruby. A second socket remains vacant.\n"); break;
		case  3: do_char_log(cn, 1, "A talisman socketed with a huge amethyst. A second socket remains vacant.\n"); break;
		case  4: do_char_log(cn, 1, "A talisman socketed with a huge topaz. A second socket remains vacant.\n"); break;
		case  5: do_char_log(cn, 1, "A talisman socketed with a huge emerald. A second socket remains vacant.\n"); break;
		case  6: do_char_log(cn, 1, "A talisman socketed with a huge diamond. A second socket remains vacant.\n"); break;
		case  7: do_char_log(cn, 1, "A talisman socketed with a huge spinel. A second socket remains vacant.\n"); break;
		case  8: do_char_log(cn, 1, "A talisman socketed with a huge citrine. A second socket remains vacant.\n"); break;
		case  9: do_char_log(cn, 1, "A talisman socketed with a huge opal. A second socket remains vacant.\n"); break;
		case 10: do_char_log(cn, 1, "A talisman socketed with a huge aquamarine. A second socket remains vacant.\n"); break;
		case 11: do_char_log(cn, 1, "A talisman socketed with a huge beryl. A second socket remains vacant.\n"); break;
		case 12: do_char_log(cn, 1, "A talisman socketed with a huge zircon. A second socket remains vacant.\n"); break;
		default: do_char_log(cn, 1, "%s\n", it[in].description); break;
		}
	}
	else
	{
		do_char_log(cn, 1, "A Talisman socketed with two gemstones. It can be used on a piece of gear to grant the following effect:\n");
		switch (it[in].data[0])
		{
		case  1: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Braveness", 4); break;
		case  2: do_char_log(cn, 8, "[Chest Only] 8%% more total Armor Value.\n"); break; //  1
		case  3: do_char_log(cn, 8, "[Helmet Only] Healing items are 50%% more effective.\n"); break; //  2
		case  4: do_char_log(cn, 8, "[Boots Only] Cannot trigger floor traps.\n"); break; //  3
		case  5: do_char_log(cn, 8, "[Cloak Only] 20%% less effect of debuffs on you.\n"); break; //  4
		case  6: do_char_log(cn, 8, "3%% more total Braveness.\n"); break; //  5
		case  7: do_char_log(cn, 8, "[Armor Pieces Only] %-12.12s  %+4d\n", "AoE Bonus", 1); break;
		case  8: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Weakens you inflict.\n"); break; //  7
		case  9: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Move Speed", 6); break;
		case 10: do_char_log(cn, 8, "[Cloak Only] Weakens inflicted on you are 80%% weaker.\n"); break; //  8
		case 11: do_char_log(cn, 1, "%-12.12s  %+4d\n", "HP & Mana", 35); break;
		case 12: do_char_log(cn, 1, "%-12.12s  %+4d\n", "WV & AV", 2); break;
		case 13: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Willpower", 4); break;
		case 14: do_char_log(cn, 8, "[Helmet Only] Reduces Healing Sickness duration on you by 33%%.\n"); break; //  9
		case 15: do_char_log(cn, 8, "[Boots Only] Cannot step on death traps.\n"); break; // 10
		case 16: do_char_log(cn, 8, "[Chest Only] 10%% of total Armor Value is granted as additional Resistance.\n"); break; // 11
		case 17: do_char_log(cn, 8, "3%% more total Willpower.\n"); break; // 12
		case 18: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Mana", 50); break;
		case 19: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Slows you inflict.\n"); break; // 13
		case 20: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Cast Speed", 3); break;
		case 21: do_char_log(cn, 8, "[Cloak Only] Slows inflicted on you are 80%% weaker.\n"); break; // 14
		case 22: do_char_log(cn, 8, "[Jewellery Only] Cannot lose focus.\n"); break; // 15
		case 23: do_char_log(cn, 8, "[Chest Only] 20%% of damage taken from hits is dealt to your Endurance instead.\n"); break; // 16
		case 24: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Intuition", 4); break;
		case 25: do_char_log(cn, 8, "[Helmet Only] Grants infravision.\n"); break; // 17
		case 26: do_char_log(cn, 8, "[Armor Pieces Only] 20%% more maximum Endurance.\n"); break; // 18
		case 27: do_char_log(cn, 8, "3%% more total Intuition.\n"); break; // 19
		case 28: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Spell Mod", 1); break;
		case 29: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Curses you inflict.\n"); break; // 20
		case 30: do_char_log(cn, 8, "[Boots Only] Stealth is 25%% stronger while moving.\n"); break; //  6
		case 31: do_char_log(cn, 8, "[Cloak Only] Curses inflicted on you are 80%% weaker.\n"); break; // 21
		case 32: do_char_log(cn, 8, "[Helmet Only] 20%% less cost of skills and spells.\n"); break; // 22
		case 33: do_char_log(cn, 8, "[Chest Only] 20%% of damage taken from hits is dealt to your Mana instead.\n"); break; // 23
		case 34: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Agility", 4); break;
		case 35: do_char_log(cn, 8, "[Chest Only] 10%% of total Armor Value is granted as additional Immunity.\n"); break; // 24
		case 36: do_char_log(cn, 8, "3%% more total Agility.\n"); break; // 25
		case 37: do_char_log(cn, 8, "[Boots Only] Movement speed cannot fall below 1.50.\n"); break; // 26
		case 38: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Poisons you inflict.\n"); break; // 27
		case 39: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Atk Speed", 3); break;
		case 40: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Weapon Value", 3); break;
		case 41: do_char_log(cn, 8, "[Cloak Only] Stealth is 25%% stronger while idle.\n"); break; // 28
		case 42: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Parry Bonus", 3); break;
		case 43: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Strength", 4); break;
		case 44: do_char_log(cn, 8, "3%% more total Strength.\n"); break; // 29
		case 45: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Hitpoints", 50); break;
		case 46: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Bleeds you inflict.\n"); break; // 30
		case 47: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Endurance", 25); break;
		case 48: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Hit Bonus", 3); break;
		case 49: do_char_log(cn, 8, "[Weapons Only] %-12.12s  %+4d\n", "Base Crit", 1); break;
		case 50: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Armor Value", 3); break;
		case 51: do_char_log(cn, 1, "%-12.12s  %+4d\n", "All Attribs", 2); break;
		case 52: do_char_log(cn, 8, "[Armor Pieces Only] %-12.12s  %+4d\n", "Glow", 20); break;
		case 53: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of Blinds you inflict.\n"); break; // 31
		case 54: do_char_log(cn, 8, "[Jewellery Only] Grants unreduced regeneration rates while walking.\n"); break; // 32
		case 55: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Hit & Parry", 2); break;
		case 56: do_char_log(cn, 8, "[Cloak Only] Blinds inflicted on you are 80%% weaker.\n"); break; // 33
		case 57: do_char_log(cn, 8, "Item is indestructible.\n"); break; // 34
		case 58: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Spell Apt", 6); break;
		case 59: do_char_log(cn, 8, "8%% chance to half damage taken.\n"); break; // 35
		case 60: do_char_log(cn, 8, "[Helmet Only] Half of your Meditate score is also applied to your Hitpoints.\n"); break; // 36
		case 61: do_char_log(cn, 8, "[Cloak Only] 5%% additional chance to avoid being hit.\n"); break; // 37
		case 62: do_char_log(cn, 8, "[Weapons Only] Restore 1 Mana upon hitting an enemy.\n"); break; // 38
		case 63: do_char_log(cn, 8, "[Armor Pieces Only] Restore 2 Mana upon being hit by an enemy.\n"); break; // 39
		case 64: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Crit Multi", 10); break;
		case 65: do_char_log(cn, 8, "[Boots Only] 20%% more total Movement Speed.\n"); break; // 40
		case 66: do_char_log(cn, 8, "[Weapons Only] 5%% additional chance to hit.\n"); break; // 41
		case 67: do_char_log(cn, 8, "[Weapons Only] Restore 1 Endurance upon hitting an enemy.\n"); break; // 42
		case 68: do_char_log(cn, 8, "[Armor Pieces Only] Restore 2 Endurance upon being hit by an enemy.\n"); break; // 43
		case 69: do_char_log(cn, 1, "%-12.12s  %+4d\n", "All Speed", 2); break;
		case 70: do_char_log(cn, 8, "[Boots Only] Movement Speed is doubled while underwater.\n"); break; // 44
		case 71: do_char_log(cn, 8, "[Helmet Only] Half of your Rest score is also applied to your Mana.\n"); break; // 45
		case 72: do_char_log(cn, 8, "[Boots Only] You can always escape from combat.\n"); break; // 46
		case 73: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Top Damage", 12); break;
		case 74: do_char_log(cn, 8, "[Chest Only] Reduces extra damage taken from enemy critical hits by 50%%.\n"); break; // 47
		case 75: do_char_log(cn, 8, "[Chest Only] 30%% more total Thorns score.\n"); break; // 48
		case 76: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Crit Bonus", 16); break;
		case 77: do_char_log(cn, 8, "[Weapons Only] Restore 1 hitpoints upon hitting an enemy.\n"); break; // 49
		case 78: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Thorns", 2); break;
		case 79: do_char_log(cn, 8, "[Jewellery Only] 2%% more damage dealt.\n"); break; // 50
		case 80: do_char_log(cn, 8, "[Jewellery Only] 2%% less damage taken.\n"); break; // 51
		case 81: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Parry", 4);
				 do_char_log(cn, 1, "%-12.12s  %+4d\n", "Hit", -2); break;
		case 82: do_char_log(cn, 8, "[Helmet Only] Perception is 33%% stronger.\n"); break; // 52
		case 83: do_char_log(cn, 8, "[Jewellery Only] 15%% reduced damage taken from damage over time.\n"); break; // 53
		case 84: do_char_log(cn, 8, "2%% more total attributes.\n"); break; // 54
		case 85: do_char_log(cn, 8, "[Jewellery Only] Total Glow score is reduced to 0.\n"); break; // 55
		case 86: do_char_log(cn, 8, "[Gloves Only] 20%% more effect of heals and regens you apply.\n"); break; // 56
		case 87: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Hit", 4);
				 do_char_log(cn, 1, "%-12.12s  %+4d\n", "Parry", -2); break;
		case 88: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Weapon Value", 2);
				 do_char_log(cn, 1, "%-12.12s  %+4d\n", "Top Damage", 8); break;
		case 89: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Crit Bonus", 8);
				 do_char_log(cn, 1, "%-12.12s  %+4d\n", "Crit Multi", 5); break;
		case 90: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Armor Value", 2);
				 do_char_log(cn, 1, "%-12.12s  %+4d\n", "Thorns", 1); break;
		case 91: do_char_log(cn, 1, "%-12.12s  %+4d\n", "Cooldown", 2); break;
		default: break;
		}
	}
	look_god_details(cn, in);
}

void look_colosseum_shrine(int cn, int in)
{
	// A shrine made of pure gold. It radiates magic and power.
	switch (it[in].data[0])
	{
		case  1:
			do_char_log(cn, 1, "%s When used, a random piece of equipped gear will gain +2 to a random attribute.\n", it[in].description);
			break;
		case  2:
			do_char_log(cn, 1, "%s When used, a donated piece of gear will gain +2 to a random attribute.\n", it[in].description);
			break;
		case  3:
			do_char_log(cn, 1, "%s When used, a random piece of equipped gear will gain +2 to your highest attribute.\n", it[in].description);
			break;
		case  4:
			do_char_log(cn, 1, "%s When used, a donated piece of gear will gain +2 to your highest attribute.\n", it[in].description);
			break;
		default:
			do_char_log(cn, 1, "%s It radiates magic and power.\n", it[in].description);
			break;
	}
}

void look_driver(int cn, int in, int showname)
{
	if (it[in].driver != 113 && it[in].driver != 119 && it[in].driver != 122) // Skip for Contracts & Talismans & Colosseum Shrines
	{
		if (showname)
			do_char_log(cn, 6, "%s:\n", it[in].name);
		do_char_log(cn, 1, "%s\n", it[in].description);
		
		look_item_details(cn, in);
		look_chest(cn, in);
		look_door(cn, in);
		look_pole(cn, in);
		
		if (it[in].temp == IT_TOMBSTONE && it[in].data[0])
			do_ransack_corpse(cn, it[in].data[0], " * Carries a%s.\n");
	}
	
	if (!(it[in].flags & IF_LOOKSPECIAL)) return;
	
	switch(it[in].driver)
	{
		case   0: case  32: case  40: case  52: case  60:
		case  92: case  93: case 110: case 114: case 115:
		case 116: case 133: look_extra(cn, in);  break;
		
		case  17: look_collection(cn, in);       break;
		case  48: look_spell_scroll(cn, in);     break;
		case 113: look_contract(cn, in, 1);      break;
		case 119: look_talisman(cn, in);         break;
		case 122: look_colosseum_shrine(cn, in); break;
		
		default: break;
	}
}
