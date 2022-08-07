// **************************************************************** 
//
//  The Last Gate - SDL2 Client by Zachary "ZarroTsu" Baillie
//  Based on the Mercenaries of Astonia engine by Daniel Brockhaus
//  
//  math.h
//  Header information specific to math.c
//
// **************************************************************** 
#ifndef MATH_H
#define MATH_H

#define max(a, b)		((a)>(b) ? (a) : (b))
#define min(a, b)		((a)<(b) ? (a) : (b))

#define HIGH_VAL		(1<<30)

// -------- Damage Multipliers -------- //

#define DAM_MULT_HIT		 250
#define DAM_MULT_BLAST		 625
#define DAM_MULT_HOLYW		 750
#define DAM_MULT_THORNS		1000
#define DAM_MULT_CLEAVE		 500
#define DAM_MULT_PULSE		 100
#define DAM_MULT_ZEPHYR		  25
#define DAM_MULT_LEAP		 250
#define DAM_MULT_POISON		1750

#define SPEED_CAP			 300

#endif // MATH_H
/* END OF FILE */