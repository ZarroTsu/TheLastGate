=================================================================
        ┌┬┐┌─┐┬ ┬┌─┐┌─┐┌┬┐  ┌┬┐┬ ┬┌─┐┬ ┬  ┌─┐┌─┐┌─┐┌─┐
        │││├─┤└┬┘├┤ └─┐ │    │ ├─┤│ ││ │  ├─┘├─┤└─┐└─┐
        ┴ ┴┴ ┴ ┴ └─┘└─┘ ┴    ┴ ┴ ┴└─┘└─┘  ┴  ┴ ┴└─┘└─┘
-----------------------------------------------------------------
  ________            __               __     ______      __     
 /_  __/ /_  ___     / /   ____ ______/ /_   / ____/___ _/ /____ 
  / / / __ \/ _ \   / /   / __ `/ ___/ __/  / / __/ __ `/ __/ _ \
 / / / / / /  __/  / /___/ /_/ (__  ) /_   / /_/ / /_/ / /_/  __/
/_/ /_/ /_/\___/  /_____/\__,_/____/\__/   \____/\__,_/\__/\___/ 
=================================================================
              FAQ version 0.8.5  -  by ZarroTsu
=================================================================

1. Forward & Rendering
2. What is Mercenaries of Astonia 2? 
3. What is The Last Gate?
4. Change Overview Between MoA2 and TLG
5. Beginner Class Overview
6. Advanced Class Overview
7. Attribute Overview
8. Skill Overview
9. Credits & Links

=================================================================
		- Forward & Rendering -
=================================================================

The renderer for The Last Gate (and its parent, Mercenaries of Astonia 2) is Direct Draw. Direct Draw is an older rendering software that may not work its best on modern graphic cards and has long since become obsolete with newer versions of Direct X. Direct Draw is unfortunately very deeply ingrained in the client for The Last Gate and it would be very difficult and time consuming to rewrite the source code using a more modern or open-source rendering software. We're kind of stuck with it.

To circumvent the issues present in Direct Draw, we have included a more modernized version of the Direct Draw DLL file called "DDrawCompat". DDrawCompat appears to behave as a good work-around for modern graphic card issues with Direct Draw that would otherwise render the game unplayable or very sluggish/choppy. As such it is included with the game in the main folder alongside the client. Please note that this may make the game window's border look a bit strange as it appears to force the game to run in compatibility mode.


	** If for some reason the game refuses to run, please try one of the following solutions: **


1) Delete the included ddraw.dll - This will force your computer to use its default copy of ddraw.dll if it is available.

2) DxWnd - https://sourceforge.net/projects/dxwnd/ - A program used for running most games that use older versions of Direct Draw, 3D, or X. If you use this be sure to specify a window resolution no smaller than 1280x720 or it may throw an error.

3) dgVoodoo - http://dege.freeweb.hu/dgVoodoo/dgVoodoo/ - Another work-around for ddraw. This is typically a last resort as, while it can "run the game", it does so at a sluggish or choppy pace and is typically worse than using DDrawCompat or otherwise.

If none of these options appear to work, please reach out to our Discord server (using the 'Discord' button on the launcher) and someone will be willing to help out resolving errors and getting the game running.


=================================================================
		- What is Mercenaries of Astonia 2? -
=================================================================

Mercenaries of Astonia 2, or short form "MoA2", is a small indie MMO made by a man named Daniel Brockhaus in his college years when he started studying Computer Science. Despite its size and limitations it attracted a fairly large (at the time) audience of devout players, and would eventually lead as the basis to its larger follow-up project "Astonia 3", which you can find a handful of servers for today.

After MoA2 shut down, Daniel sold copies of the source code to people interested in hosting their own servers, and for a while several of these servers would come and go and attract players of its original server to both play and develop the game with additional skills, classes, enemies, maps, items, and so on. Most of these servers have since gone offline and only the rare archive of its minimap or character build guides can be found today.

There are currently two known MoA2 servers online at this time - "Aranock Online" which features a client and server map close to its original version adding new aspirational content to it over time - and "The Last Gate".


=================================================================
		- What is The Last Gate? -
=================================================================

The Last Gate, or short form "TLG", is an experimental server started in early 2020 which has largely overhauled the source code from the original MoA2. TLG intends to provide a mix of nostalgia and brand new content, designing a new world map from the ground up and changing how many of the skills and character classes behave.

The Last Gate still uses some vanilla assets such as the look and "feel" of the world map and live characters, but it has also added a huge number of new sprites and other assets made from scratch. The client has also been overhauled to provide additional information (such as "dps" if you like making that sort of number big) and more visual information than the vanilla client normally renders.

At present, TLG does not have its own website, and instead integrates some web functionality such as "who's online" and "top 5 class" lists using a Discord robot. These update once every 10 seconds and are about as close to real-time as they can get without being throttled.


=================================================================
		- Change Overview Between MoA2 and TLG -
=================================================================

Below is a list of most changes between Vanilla Mercenaries of Astonia 2 and The Last Gate. Note that this may not be *all* of them, and is simply as many as I can remember at the time of writing this Read-Me.


	CLIENT:
	
- Supports windowed resolution and latent 1280x720
- Delay between client input and server response has been substantially reduced
- Message-of-the-day has been separated from regular chat log and is presented as its own window upon load
- Built-in tutorial is offered upon creating a new character and helps players through the first quest and how to play the game
- Majority of inventory space is visible at all times
- Scroll wheel implementation for skill list, inventory, chat window
- Zoom functionality for the minimap
- HP bars toggle in the lower-right options
- Skill base values can be set to display to the left of standard values
- Passive skills that are typically "always active" are now present in red font at the bottom of the standard skill list
- Increased the number of available font colors from 4 to 10
- GUI renders a small info panel to the right of the standard skill list, which can be changed by right-clicking on an attribute name
- Adjusted the descriptions of most skills to be clear and concise
- Added support for various new menu systems, detailed below


	SERVER:
	
- Rebuilt the world map from the ground up using many familiar names, and added a huge amount of content to keep players from needing to stop and grind
- Added waypoints as a way to get around the world map quickly (each waypoint must be discovered before it can be travelled to)
- Finished all labyrinth gorges with new ones at 6 and 12, ending at around Colonel rank
- Added new arch-classes for Templars and Harakim upon reaching Lab XIII
- Added a new generic arch-class for all classes, provided they explore the world and figure out how to unlock it
- Removed several redundant, insignificant, or "joke" skills, instead relegating them to set character flags where appropriate
- Added over a dozen new passives, skills and spells
- Adjusted in-game animation speed to increment in much smaller steps and enable speed-affecting skills and items
- Standardized the "Endurance" system to behave around a standard base of 100 endurance
- Added critical hits and relevant critical hit rate and multiplier values
- Added claw weapons for the hand-to-hand skill
- Improved restorative passives to take some effect during combat
- Mana can now be passively recovered without needing to know the Meditate skill
- Mana costs are adjusted in the early game to be cheaper until a given spell grows in power
- Character classes have stronger identities
- Removing "Stun" and replacing it with a "Slow" effect
- All attributes now have a secondary effect other than "modifying certain skill scores"
- Added a new armor series intended for spellcasters, using Willpower and Intuition to equip instead of Agility and Strength
- New equipment items called Tarot Cards that change the behavior of character behavior
- The addition of "quests" beyond the standard skill-teaching ones
- Randomized map/mission content


=================================================================
		- Beginner Class Overview -
=================================================================

	* TEMPLAR *

Templars are the standard melee class of the early game. Expert fighters, they easily surpass other classes in melee combat both in dealing damage and taking it. They offer high Strength, Agility, and melee skill scores, but low Intuition, Willpower and spell skill scores. For players who enjoy less attentive combat, or who want to pull aggro and feel hard to take out.

Templars offer the following skills:

- CLEAVE enemies to deal damage and cause bleeding damage over time
- TAUNT foes to draw their attention toward you
- Naturally augment weapon and armor values with GEAR MASTERY


Templar can advance to one of the following classes after clearing the labyrinth:

- Arch Templars, further improving their defensive capabilities and letting them hit many enemies at once
- Skalds, improving how quickly they, and their allies, can recover between skills
- Seyan'du, the advanced jack-of-all-trades class



	* MERCENARY *

Mercenaries are the jack-of-all-trades class of the early game. They don't necessarily excel in any situation, but they don't pale in it either. They offer the most balanced-to-a-fault spread of attribute, skill and spell scores. For players who prefer a more attentive combat experience, or players who like to play a more stealth-oriented role.

Mercenaries offer the following skills:

- POISON your foes from afar to deal damage over time, increasing in potency with each consecutive cast
- BLIND surrounding enemies to avoid taking damage
- Make use of your STEALTH to approach situations cautiously without being noticed


Mercenaries can advance to one of the following classes after clearing the labyrinth:

- Warriors, turning their focus towards their melee abilities to hit things fast and hard
- Sorcerers, turning their focus towards their magic abilities to curse many foes at once
- Seyan'du, the advanced jack-of-all-trades class



	* HARAKIM *

Harakim are the standard magic class of the early game. Expert casters, they fight from afar using spells and summoning companions to fight in their stead. They offer high Intuition, Willpower and spell skill scores, but low Strength, Agility and melee skill scores. For players who enjoy more complicated character building, or simply want to watch the world burn.

Harakim offer the following skills:

- BLAST your foes from afar to deal significant damage
- Summon GHOST COMPANIONs to your command to defend you
- Reduce the cost of spells with CONCENTRATE


Harakim can advance to one of the following classes after clearing the labyrinth:

- Summoners, earning more powerful and protective ghost companions
- Arch-Harakim, focusing their attention on decimating the battlefield with fierce explosions
- Seyan'du, the advanced jack-of-all-trades class


=================================================================
		- Advanced Class Overview -
=================================================================

	* Arch-Templar *

Arch-Templar are the well-rounded tanks of the late game. They are capable of being resilient tanks or ferocious fighters, and offer the highest Strength score in the game. Arch-Templar can implicitly hit a broader range of targets with their Surround Hit skill.

Arch-Templars offer the following skills:

- Improve your defenses with SAFEGUARD
- WARCRY to stun and inflict fear onto your foes



	* Skald *

Skalds are the swift-acting bards of the late-game. They are capable of leaping between enemies in quick succession, and offer the highest Agility score in the game.

Skalds offer the following skills:

- Improve ally cooldowns and debilitate enemies with ARIA
- Quickly LEAP long distances to enter the fray in an instant



	* Warrior *

Warriors are the agile speedsters of the late-game. They are capable of hitting and dodging foes with extraordinary speed, and offer high Strength and Agility scores. Warriors can implicitly hit targets much more often with their Surround Hit skill.

Warriors offer the following skills:

- Equip a second sword with DUAL WIELD to improve your offenses
- Hit enemies additional times with ZEPHYR



	* Sorcerer *

Sorcerers are the prime debuff supports of the late-game. They can easily debuff huge groups of enemies with curses and poisons, and offer high Intuition and Willpower scores.

Sorcerers offer the following skills:

- Use PROXIMITY to grant area-of-effect to your Poison, Curse and Slow spells
- Inflict LETHARGY on yourself to penetrate enemy spell resistances



	* Summoner *

Summoners are the... summoners of the late-game. They can summon powerful companions, copy enemy shadows to aid them in combat, and offer the highest Willpower score in the game.

Summoners offer the following skills:

- Gain COMPANION MASTERY to teach ghost companions how to use Heal, Taunt and Blind
- Pull temporary companions from your targets with SHADOW COPY



	* Arch-Harakim *

Arch-Harakim are the ranged DPS of the late-game. They make things explode, explode things while waiting to make things explode, and offer the highest Intuition score in the game.

Arch-Harakim offer the following skills:

- Use PROXIMITY to grant area-of-effect to your Blast spell
- Arm yourself with PULSE to cause even further damage to enemies around you



	* Braver *

Bravers are the extreme glass cannons of the late-game. They have extremely high critical hit rates but can be quite fragile without support. They offer the highest Braveness score in the game.

Bravers are reset to zero experience upon promotion, and restart their journey from the beginning of the game.

Bravers borrow the following skills from the classes above:

- SAFEGUARD
- ARIA
- DUAL WIELD
- HASTE
- PROXIMITY for your Aria skill

Bravers offer the following skills:

- Improve the rate and strength of critical hits with apt PRECISION
- Enter a furious RAGE to bolster your weapon value



	* Seyan'du *

Seyan'du are the epitome of "Jack of all trades, master of none". They have a wide variety of skills, spells and passives and can be built in a broad number of ways, but may be difficult to get started with and require a lot of investment. They offer the highest cumulative attribute total, but nothing stand-outish.

Seyan'du are reset to zero experience upon promotion, and restart their journey from the beginning of the game. They are awarded a unique weapon that can be upgraded by visiting various shrines around the world, hidden behind striped red doors. Their weapon is capable of utilizing whichever passive weapon skill is highest, enabling them to plan and scale any set of attributes for their strongest dedicated effect.

Seyan'du do not learn any unique skills or spells. However, they are capable of learning *one* of seven unique skills from the classes above:

- WARCRY
- LEAP
- ZEPHYR
- LETHARGY
- SHADOW COPY
- PULSE
- RAGE


=================================================================
		- Attribute Overview -
=================================================================

	BRAVENESS

The general-use attribute, Braveness improves a majority of skill modifiers by a small amount. Implicitly, Braveness will improve your character's critical hit rate.


	WILLPOWER

Willpower is a defense oriented spell attribute and strongly improves most positive buffs. Implicitly, Willpower grants faster casting speed, improves the power of spells cast on allies, and contributes to the strength of spells that can affect you naturally.


	INTUITION

Intuition is an offense oriented spell attribute and strongly improves most negative debuffs and damaging spells. Implicitly, Intuition grants faster cooldown speed of skills and spells, and contributes to the strength of spells that can affect you naturally.


	AGILITY

Agility is a speed oriented melee attribute and strongly improves hand-to-hand and twohanded weapon skills. Implicitly, Agility increases your character's attack speed, and contributes to your character's overall action speed.


	STRENGTH

Strength is a power oriented melee attribute and strongly improves axe weapon skills. Implicitly, Strength increases your character's top-end attack power, the speed at which you can mine walls, and contributes to your character's overall action speed.


=================================================================
		- Skill Overview -
=================================================================

	* Passive restorative skills *

		REGENERATE

Dictates and improves the rate in which you recover hitpoints passively over time. Has a stronger effect while standing still. Increases in effectiveness during New Moons and Full Moons.


		REST

Dictates and improves the rate in which you recover endurance passively over time. Has a stronger effect while standing still. Increases in effectiveness during New Moons and Full Moons.


		MEDITATE

Dictates and improves the rate in which you recover mana passively over time. Has a stronger effect while standing still. Increases in effectiveness during New Moons and Full Moons.



	* Passive weapon skills *

		HAND TO HAND

Passive ability to hit and parry during melee combat while either unarmed or using a claw weapon. Known by all character classes.


		AXE

Passive ability to hit and parry during melee combat while using an axe weapon. If a greataxe is equipped, the higher between this and the Two-Handed skill is used instead. Known by Templar classes and learned by Warriors upon promotion.


		DAGGER

Passive ability to hit and parry during melee combat while using a dagger weapon. If a spear is equipped, the higher between this and the Staff skill is used instead. Known by Harakim and Mercenary classes.


		DUAL WIELD

Passive ability to hit while using a dual-sword in the off-hand, at a rate of one-in-six. Learned by Warriors and Bravers upon promotion.


		SHIELD

Passive ability to parry during melee combat while using a shield in the off-hand, at a rate of one-in-six. Learned by Templar and Mercenary classes.


		STAFF

Passive ability to hit and parry during melee combat while using a staff weapon. If a spear is equipped, the higher between this and the Dagger skill is used instead. Learned by Harakim classes.


		SWORD

Passive ability to hit and parry during melee combat while using a one-handed sword weapon. Known by Templar and Mercenary classes.


		TWO-HANDED

Passive ability to hit and parry during melee combat while using a two-handed sword weapon. If a greataxe is equipped, the higher between this and the Axe skill is used instead. Known by Templar classes.



	* Active endurance skills *

		BLIND

Active skill which attempts to blind all surrounding enemies in a small circle. Blind reduces enemy hit and parry rates, as well as their Perception score. Known by Mercenary classes.


		CLEAVE

Active skill which deals damage to the enemy you are fighting, and surrounding enemies, and inflicts Bleeding upon dealing damage. Bleeding deals damage equal to the amount dealt by Cleave over a 15 second duration. Known by Templar classes and learned by Warriors upon promotion.


		LEAP

Active skill which deals damage to the enemy you are fighting, and attempts to jump to the enemy you are targeting with ALT. Damages all surrounding targets at both the start and end locations. Additionally, hitpoints can be sacrificed to bypass the cooldown of this skill proportional to the remaining cooldown duration. Learned by Skalds after completing a difficult quest.


		RAGE

Toggle skill which when active causes you to lose endurance over time, but gain an increase in weapon value. Automatically turns off if you run out of endurance. Learned by Bravers after completing a difficult quest.


		REPAIR

Active skill which attempts to repair the item held under your cursor when used. Requires a skill score of at least the 'Power' of a given item in order to repair it. Some items cannot be repaired. Learned by all classes.


		TAUNT

Active skill which draws an enemy's full attention to the user, and grants the user a small buff called Guard. Guard grants a small bonus to defense for its duration. If the target cannot reach you they will still attempt to exit combat and wander toward the user. Learned by Templar classes.


		WARCRY

Active skill which inflicts Stun and Fear on all surrounding enemies, increasing in range as it increases in power. Stun locks a target in place for a brief duration, and Fear reduces all attributes of an afflicted target for a longer duration. Learned by Arch-Templar after completing a difficult quest.


		WEAKEN

Active skill which inflicts Weakness on the enemy you are fighting. Weakness reduces enemy weapon value. Learned by Templar classes.



	* Active mana spells *

		BLAST

Active spell which damages the enemy you are targeting with ALT, as well as all surrounding combat enemies. If no enemy is targeted with ALT, it grants priority to target the enemy you are facing first, and the enemy behind you last. Known by Harakim classes and learned by Sorcerers upon promotion.


		BLESS

Active spell which buffs yourself or your target. Bless grants the player a bonus to all attribute scores. Learned by all classes.


		CURSE

Active spell which debuffs the enemy you are targeting with ALT, as well as all surrounding combat enemies. If no enemy is targeted with ALT, it grants priority to target the enemy you are facing first, and the enemy behind you last. Curse reduces all enemy attribute scores. Learned by Harakim and Mercenary classes. 


		DISPEL

Active spell which attempts to remove a debuff from yourself or your target, and grants full immunity to that debuff for a short duration. If Dispel is strong enough, it can remove and protect against up to three debuffs at a time. If Dispel is cast during an active immunity, that immunity is removed before new ones are added. Learned by Harakim classes and Sorcerers upon promotion.


		ENHANCE

Active spell which buffs yourself or your target. Enhance grants the player a bonus to weapon value. Learned by Harakim and Mercenary classes.


		GHOST COMPANION

Active spell which summons a ghost companion. Ghost companions automatically engage with the enemy you are targeting with ALT when summoned. Your ghost companion can be buffed by Bless, Enhance, Protect, and other spells in the same way a player could be buffed. Ghost companions can also be commanded to act in certain ways or change their general behavior - saying "COMMAND" to one will give you a list of which commands are available. You may also use the /gctome chat command to allow ghost companions to follow you through portals and ladders, and can check on the status of their active buffs and debuffs by using /gcb. Known by Harakim classes.


		HASTE

Active spell which buffs yourself with Haste, granting you an increase in action speed. Learned by Warriors and Sorcerers upon promotion.


		HEAL

Active spell which heals yourself or your target, and inflicts a debuff called Healing Sickness for one minute. Healing Sickness can stack up to three times, each stack reducing the next cast of Heal by 25% power. All stacks are removed when Healing Sickness expires. Learned by all classes.


		IDENTIFY

Active spell which attempts to identify a target player, enemy or held item. Learned by all classes.


		LETHARGY

Toggle spell which when active causes you to lose mana over time, but grants you the ability to pierce enemy resistance and immunity scores. Automatically turns off if you run out of mana. Learned by Sorcerers after completing a difficult quest.


		MAGIC SHIELD

Active spell which buffs yourself with Magic Shield, granting an increase in armor value. Magic Shield decays over time, and further decays when hit by damaging enemy skills or melee strikes. Automatically overwrites an active Magic Shield if a fresh cast would grant more armor value than the current copy. Known by Harakim classes and learned by Templar classes.


		POISON

Active spell which debuffs the enemy you are targeting with ALT, as well as all surrounding combat enemies. If no enemy is targeted with ALT, it grants priority to target the enemy you are facing first, and the enemy behind you last. Poison deals damage over time and can stack up to three times on a single target. Learned by Mercenary classes.


		PROTECT

Active spell which buffs yourself or your target. Protect grants the player a bonus to armor value. Learned by Harakim and Mercenary classes.


		PULSE

Active spell which buffs yourself with Pulse, dealing damage to enemies around you once every few seconds. The rate of damage dealt by Pulse is improved with cooldown rate. Learned by Arch-Harakim after completing a difficult quest.


		SHADOW COPY

Active spell which summons a Shadow Copy. Shadow copies automatically engage with the enemy you are targeting with ALT when summoned. If an enemy is targeted with ALT, it will copy that target's shadow, otherwise it copies the caster's shadow. Shadow Copy's behavior is dictated by the sprite it imitates when summoned. Increases in duration as skill score increases. Learned by Summoners after completing a difficult quest.


		SLOW

Active spell which debuffs the enemy you are targeting with ALT, as well as all surrounding combat enemies. If no enemy is targeted with ALT, it grants priority to target the enemy you are facing first, and the enemy behind you last. Slow reduces a target's action speed, but decays over time as its duration runs out. Learned by Harakim and Mercenary classes.


		ZEPHYR

Active spell which buffs yourself with Zephyr, inflicting a short debuff each time you hit an enemy. The debuff inflicted lasts one second, after which it damages the target. This debuff can be stacked up to three times independently, each stack independently expiring and dealing its damage one second after being inflicted. Learned by Warriors after completing a difficult quest.



	* Miscellaneous Passive skills *

		AURA

Passive skill which grants the Aria buff to nearby allies, and an Aria debuff to nearby enemies. Aria improves ally cooldown rates and reduces enemy cooldown rates. Learned by Skalds and Bravers upon promotion.


		BARTERING

Passive skill used when shopping. Reduces the cost of items sold by merchants, and increases the value of items when sold to merchants. Certain rare or invaluable items may not be affected by Bartering when sold to merchants, and will always sell for the same amount. Learned by all classes.


		COMPANION MASTERY

Passive skill which grants Ghost Companions the ability to cast Blind, Heal and Taunt. Gradually improves how powerful your Ghost Companions can become. Learned by Summoners upon promotion.


		CONCENTRATE

Passive skill which reduces the mana cost of all spells. Learned by Harakim classes and Sorcerers upon promotion.


		GEAR MASTERY

Passive skill which improves weapon and armor values up to an amount dictated by equipped gear. Known by Templar classes.


		IMMUNITY

Passive skill which reduces the potency of negative spells inflicted on you by enemies. Learned by Templar and Mercenary classes.


		PERCEPTION

Passive skill which improves the ability to see in the dark and see stealthy enemies from greater distances. Additionally notifies you if an enemy has dropped a rare or magic item once Sense Magic is learned. Known by all classes.


		PRECISION

Passive skill which improves the rate of inflicting critical hits, and subtly improves the damage of those critical hits. Learned by Bravers after completing a difficult quest.


		PROXIMITY

Passive skill which improves the area of effect of various skills and spells, depending on the class it is known by. Learned by Arch-Harakim, Sorcerers and Bravers upon promotion.


		RESISTANCE

Passive skill which attempts to nullify negative spells from being inflicted on you. Enemies automatically stop attempting to cast debuffs on you as this increases. Learned by all classes.


		SAFEGUARD

Passive skill which reduces the amount of damage taken by all sources by a percent amount. Learned by Arch-Templar and Bravers upon promotion.


		STEALTH

Passive skill which attempts to avoid enemy detection. Has improved effectiveness in dark areas and while moving on SLOW mode. Has very little effectiveness if moving on FAST mode. Known by Mercenary classes.


		SURROUND HIT

Passive skill which enables hitting all targets around you during melee combat at a rate of about one-in-three attacks. Must approach or exceed enemy weapon skill in order to activate. This also deals extra damage to the enemy you are presently fighting when it activates. When used by a Warrior, the rate of activation is improved to two-in-three attacks. Learned by Templar classes and by Warriors upon promotion.


		SWIMMING

Passive skill which reduces damage taken over time when standing on "underwater" tiles, dictated by a light blue hue.


=================================================================
		- Credits & Links -
=================================================================

Repository for The Last Gate's source code:
https://github.com/ZarroTsu/TheLastGate


The Last Gate is based on the Mercenaries of Astonia engine by Daniel Brockhaus.
https://brockhaus.org/merc2.html


Openmerc is a standardized version of the MoA2 source code by Dylanyaga which provides additional instructions and a generalized setup file that can get everything you need installed and running without needing a lot of in-depth Linux knowledge
https://github.com/dylanyaga/openMerc


DDrawCompat is a DLL wrapper aimed at fixing compatibility and performance issues with games based on DirectDraw and Direct3D 1-7.
https://github.com/narzoul/DDrawCompat


Cactus sprites by Hansjörg Malthaner.
http://opengameart.org/users/varkalandar


Aranock Online is a more "standard" version of MoA2 and offers an experience closer to vanilla.
http://www.aranockonline.com/
