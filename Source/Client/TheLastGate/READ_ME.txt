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
              FAQ version 0.5.0  -  by ZarroTsu
=================================================================

- What is The Last Gate? -

The Last Gate is an experimental, "WIP" server for Mercenaries of Astonia 2. It is a playground for trying out new ideas and concepts that the traditional MoA2 server, such as Aranock, may not immediately or intentionally try, either due to fear of backlash from fans of vanilla MoA2, or due to the ideas being far too experimental or against the grain to realistically try without proof of concept.

As a general overview of the changes in The Last Gate, I will try to list as many as I can think of off the top of my head below. This FAQ will cover many of these subjects, however if it is not listed here you can ask about it either on the server (provided I am online), or on the discord (provided I don't suddenly and mysteriously cease to be).

Server changes (from vanilla) are as follows, in no particular order:

- The server map has been redesigned from scratch. You will see familiar names, familiar locations, but it will all have been designed from scratch.
- The client has been overhauled to support windowed mode, as well as latent 1280x720 resolution.
- With the larger client, additional information has been added and HUD elements have been rearranged.
- Delay from client to server has been severely reduced. No more second-long delays between clicking and moving.
- The "speedmod" table has been extended to allow for faster high-end movement speed
- Endurance and its effects have been overhauled and balanced around a standard base of 100 Endurance.
- There are now claw weapons for the hand-to-hand skill.
- The regenerative skills, Regenerate, Rest, and Meditate, have been overhauled to allow regenerating during combat and other actions, at varying rates. The rate out-of-combat is much higher than in-combat.
- Mana Cost of spells now grows into its standard cost between a power of 1 and 100. At 1 base power, mana costs will be 5, and at 100 or more base power the mana costs will become normal.
- Spells cast on other players are limited to depend on the target player's total Willpower and Intuition scores. As a result, the scenario of "A Templar spelled by a Harakim" is hard nerfed, but still fairly strong.
- New skills have been added to the game, such as Weapon Mastery, Armor Mastery, Cleave, Weaken, etc.
- The effectiveness of the Bless and Curse spells have been halved.
- The effectiveness of the Heal spell will be reduced after repeated uses, through an effect called 'Healing Sickness' which wears off over time.
- The 'Dispel' spell no longer affects enemy buffs. Instead, it has a chance to remove multiple debuffs in a single cast.
- The 'Stun' spell has been removed and replaced by a 'Slow' spell, to utilize the speedmod changes.
- The 'Lockpick' skill has been removed and replaced with a character flag. You can still pick locks, but there's no longer a dedicated skill for it.
- Attributes such as Willpower, Intuition and Agility, now each have secondary effects on your character.
- There are multiple towns. Aston still exists, but players will find their new game starts in the town of Lynbore.
- The effective range of the 'Bartering' skill has been reduced. In effect this means you can buy things for cheaper earlier.
- New arch races have been added to the game, branching as alternatives through Templars and Harakims.
- There are new tiers of weapons and armors, including armors dedicated to Harakim and other spellcasters.
- Special magic armor, such as "godly gold armor of the bear" etc, has been expanded to include several other skills and combinations.
- The "Random Dungeon" has been reworked and is split into three tiers of difficulty.
- "Tarot Cards" have been added in place of one of the armor slots. These cards offer a variety of unique benefits and augmentations which can change the way you play the game.
- Various other tweaks and changes

=================================================================

- Help! My game won't run!! -

Mercenaries of Astonia 2, and The Last Gate, are old games with janky graphic-card specific issues and whatnot. Sometimes it's troublesome to get it working correctly. I have provided two solutions that may work separate or together from one another.

1) Delete ddraw.dll

Oddly, deleting ddraw.dll can resolve problems on certain operating systems. Perhaps the ddraw included in base MoA is defunct? Either way, try cut/pasting it to a different folder and see if it helps.


2) DxWnd

DxWnd is a program that can run older games using older draw methods. You can get DxWnd from here:

https://sourceforge.net/projects/dxwnd/

When adding The Last Gate to the list of games on its list, make sure if you specify a resolution that it is no smaller than 1280x720, or it may throw errors when the game tries to launch.


3) dgVoodoo

DgVoodoo is an alternative version to the usual DirectDraw and Direct3D .dll files, which can help get the game running on systems whose internal graphics card doesn't support the game. Simply copy all files from the folder 'fixdll' into the main directory alongside the TheLastGate.exe, overwriting the existing ddraw.dll (you may want to back up the existing ddraw.dll just in case). This solution is only recommended if you cannot get the game working by any other method, as it is known to cause the game to visually appear choppy, which may affect the quality of play.


If you have trouble with either of these solutions, please don't hesitate to ask for help on our Discord server, available through the 'Discord' button from the game's initial Options menu.

=================================================================

- Old Classes - changes -

TEMPLAR
 - Templar has been overhauled and no longer relies on spells like other classes.
 - Instead of Bless, Protect, and Enhance, templar learns Combat, Armor, and Weapon Mastery respectively.
 - Now starts with Meditate. This is useful for casting the few spells Templars still have access to, such as Light, Heal, and Identify.
 - Now starts with Axe, Cleave and Armor Mastery.
 - Now learns Shield and Weaken.
 - No longer knows or learns Sword.
 - Templars begin the game equipped with a Bronze Axe, but can buy a Bronze Twohander from the weapon shop if desired.

MERCENARY
 - Mercenary is slightly more powerful than vanilla, but still behaves largely the same.
 - Now starts with Combat Mastery.
 - Now learns Shield, Slow and Immunity.
 - The maximum value Meditate can reach is now 60, to match other stats. Note that Meditate has been nerfed, so this is roughly in line with vanilla.

HARAKIM
 - Harakim is mostly the same as vanilla.
 - No longer starts with Dispel. Instead, Dispel is learned during the opening questline.
 - Now learns Staff and Slow.

ARCH-TEMPLAR
 - It is no longer required to reach 90/90 to arch to Arch-Templar. Instead, you need only 75 Agility and 90 Strength.
 - Now learns Dual Wield and Surround Area.
 - Maximum Strength and Intuition of Arch-Templar has been increased by 15.
 - Maximum Agility of Arch-Templar has been reduced by 15.

WARRIOR
 - Now learns Cleave, Dual Wield, Surround Rate and Haste.
 - Magic skills now improve to 75 base after arching.

SORCERER
 - Now learns Poison, Hex Proximity and Haste.
 - Melee skills now improve to 75 base after arching.

ARCH-HARAKIM
 - It is no longer required to reach 90/90 to arch to Arch-Harakim. Instead, you need only 75 Willpower and 90 Intuition.
 - Now learns Poison and Damage Proximity.
 - Maximum Intuition and Agility of Arch-Harakim has been increased by 15.
 - Maximum Willpower of Arch-Harakim has been reduced by 15.

SEYAN'DU
 - Spell Mod has been reduced.
 - Learns all skills that are known by pre-arch classes, including Axe, Staff, Shield, Combat Mastery, Armor Mastery, Weapon Mastery, Cleave and Weaken.
 - Now learns Focus.
 - There are additional Shrines of Kwai that can be found.
 - To compensate for the new available shrines, the power granted by these shrines has been reduced from 4 to 3. However, after finding all available shrines, the final power of the sword will be greater than vanilla.

=================================================================

- New Classes - additions -

BRAWLER
 - A new Arch class available to Templars.
 - Requires 90 Agility and 75 Strength.
 - Learns Slow, Precision and Shadow Copy.

SUMMONER
 - A new Arch class available to Harakims.
 - Requires 90 Willpower and 75 Intuition.
 - Learns Weaken, Companion Mastery and Shadow Copy.

=================================================================

- Old Skills - changes -

ENDURANCE
 - Endurance no longer increases with each level-up.
 - Endurance can no longer be raised by spending EXP.
 - Endurance has been removed from the skill table in the upper left entirely.
 - Instead, Endurance starts at 100 base for all characters.
 - Certain pieces of gear, typically belts, can be equipped to increase your maximum endurance value.
 - Active melee skills, such as Warcry and the new Cleave and Weaken skills, cost Endurance to use.
 - Various other adjustments have been taken to adapt to this change in Endurance's functionality.

BLESS
 - No longer grants +1 to all attributes per 5 power
 - Instead grants a cascading +1 per 2 power, or +1 to all attributes per 10 power.
 - The cascade is Braveness, then Willpower, then Intuition, then Agility, then Strength, and repeat.
 - A Bless power of 16 would, for example, provide +2/+2/+2/+1/+1 to B/W/I/A/S
 
CURSE
 - No longer grants -1 to all attributes per 5 power
 - Instead grants a cascading -1 per 2 power, or -1 to all attributes per 10 power.
 - The cascade is Braveness, then Willpower, then Intuition, then Agility, then Strength, and repeat.
 - A Curse power of 16 would, for example, provide -2/-2/-2/-1/-1 to B/W/I/A/S

DISPEL
 - No longer affects enemy buffs.
 - Now has a chance of removing multiple debuffs at a time
 - The "effective power" of Dispel has been improved so that you do not need a Dispel mod higher than the enemy's spell mod to remove a debuff.
 - When Dispel removes a debuff, the remaining "effective power" after removing that debuff is applied to try to remove the next debuff.
 - This loops through all debuffs. It is possible to remove every debuff in the game simultaneously if they're weak enough and Dispel's power is high enough.

HEAL
 - Now grants a debuff called 'Healing Sickness' which can stack up to three times.
 - For each stack of Healing Sickness on a target, the potency of Heal is reduced by 25%, up to a reduction of 75%
 - This is target-specific, and not caster specific. Many casters using Heal at once on the same target will stack many layers of Healing Sickness at once.

GHOST COMPANION
 - The effectiveness of Ghost Companion has been adjusted. 
 - It should have more staying power at later ranks and grows slightly faster than before.

BARTERING
 - Reduced the high end and low end multipliers so that vendors are no longer ridiculously expensive and volatile.

SURROUND HIT
 - The effective hit range of Surround Hit has been extended. 
 - Instead of needing -20 of an enemy's weapon skill to begin hitting a target, you now only need -40.
 - The chance to hit is 1/40 per point until it matches the weapon skill, from 1/20.
 - Effectively, at -20 you have 50% chance to hit, instead of 5% chance to hit.
 - You can now gain bonus damage if your surround hit mod exceeds the enemy's weapon skill, up to 12.5% extra base damage at +20.

REGENERATE
 - The effectiveness of Regenerate has been tweaked.
 - You can now regenerate hitpoints slowly while walking and out of combat, without needing to use an Ankh amulet.

REST
 - The effectiveness of Rest has been tweaked.
 - You can now regenerate endurance slowly while walking and out of combat, without needing to use an Ankh amulet.
 - Melee classes, such as Templars and Warriors, benefit a little bit more from Rest than other classes.

MEDITATE
 - The effectiveness of Meditate has been tweaked.
 - You can now regenerate mana slowly while walking and out of combat, without needing to use an Ankh amulet.
 - The overall effectiveness of Meditate has been nerfed in most cases. However,
 - Magic classes, such as Harakim and Sorcerers, aren't hit as hard by this nerf. They are roughly similar to vanilla.

CONCENTRATE
 - The effectiveness of Concentrate has been reduced.
 - It now provides 1% reduced mana cost per 5 points, down from 1% per 3 points.

WARCRY
 - Stun duration and Curse power are now based off the power of the Warcry
 - Area of effect is also based off the power of Warcry
 - Warcry's Endurance cost has been reduced to compensate for maximum Endurance scores being lower.


=================================================================

- New Skills - additions -

SHIELD
 - Can be learned in Lynbore near the end of the 'tutorial' sequence.
 - Allows the use of shields in the off-hand slot.
 - Most shields require Willpower and Intuition, and award Armor Value.
 - The Shield skill itself will grant a passive bonus to your parry rate in combat.
 - Not as effective as raising weapon skill, this acts as a supplement.

COMBAT MASTERY
 - Can be learned by Templars instead of Bless, in Lynbore.
 - Grants a passive bonus to both hitting and parrying with any weapon.
 - Not as effective as raising weapon skill, this acts as a supplement.

WEAPON MASTERY
 - Learned by Templars instead of Enhance, in Lynbore.
 - Grants a passive bonus to Weapon Value, on a linear scale similar to Enhance.
 - Grants a base of +2 WV for Templar classes, +1 WV for other classes.
 - Grants +1 WV for every 4 points on Templar classes.
 - The bonus granted to WV cannot exceed that of your total Weapon Value from equipment, on Templar classes.
 - Grants +1 WV for every 8 points on non-Templar classes.
 - The bonus granted to WV cannot exceed that of half your total Weapon Value from equipment, on non-Templar classes.

ARMOR MASTERY
 - Starter skill for both Templar and Mercenaries.
 - Grants a passive bonus to Armor Value, on a linear scale similar to Enhance.
 - Grants a base of +2 AV for Templar classes, +1 AV for other classes.
 - Grants +1 AV for every 4 points on Templar classes.
 - The bonus granted to WV cannot exceed that of your total Armor Value from equipment, on Templar classes.
 - Grants +1 AV for every 8 points on non-Templar classes.
 - The bonus granted to WV cannot exceed that of half your total Armor Value from equipment, on non-Templar classes.

CLEAVE
 - Starter skill for Templars. Can be learned by Warriors.
 - Will always target the enemy you are fighting, and any surrounding enemies.
 - Deals damage from a combination of the Cleave Skill, your Weapon Value, and your Strength score.
 - Costs a variable amount of Endurance to use, determined by skill power.
 - Has a base skill cooldown of 5 seconds.

WEAKEN
 - Can be learned by Templars, in Lynbore.
 - Will always target the enemy you are fighting, and any surrounding enemies.
 - Reduces a target's Weapon Value and Armor Value.
 - Rate of reduction is 2 + power/8. Power is reduced by target Immunity.
 - Costs 10 Endurance to use.
 - Has a base skill cooldown of 3 seconds.

SLOW
 - Can be learned by Harakim and Mercenaries, in Lynbore.
 - Reduces the target's movement speed.
 - Duration of Slow is between 15 and 75 seconds, depending on spell power.
 - The higher the power of Slow, the greater the effect.
 - The effects of Slow gradually reduce over time, until returning to normal when it runs out.
 - Has a base Mana cost of 20.
 - Has a base skill cooldown of 4 seconds.


PRECISION
 - Arch skill learned by Brawlers.
 - Grants an additional chance to deal critical hits.
 - Granted crit chance is multiplicative! Weapons with a high critical hit base will benefit more from this skill.

DUAL WIELD
 - Arch skill learned Arch Templars and Warriors.
 - Allows the use of Dual Swords in the off-hand slot.
 - Most dual swords require Agility and Strength at numbers scaling from 60 onward.
 - The Dual Wield skill itself will grant a passive bonus to your hit rate in combat.
 - Not as effective as raising weapon skill, this acts as a supplement.

POISON
 - Arch skill learned by Arch-Harakim and Sorcerers.
 - Inflicts the 'Poison' debuff on targets, causing hitpoints to decrease over time.
 - Poison's effectiveness is doubled for Sorcerers.
 - Poisoned enemies will grant a steady stream of EXP to the caster who poisoned them.

DAMAGE PROXIMITY
 - Arch skill learned by Arch-Harakim
 - Grants a passive area-of-effect to the Blast and Poison spells.
 - You can still target a distant enemy with ALT as normal, this only affects the area-of-effect of surrounding targets.

HEX PROXIMITY
 - Arch skill learned by Sorcerers.
 - Grants a passive area-of-effect to the Curse, Slow, and Poison spells.
 - You can still target a distant enemy with ALT as normal, this only affects the area-of-effect of surrounding targets.

COMPANION MASTERY
 - Arch skill learned by Summoners.
 - Grants your Ghost Companion access to Surround Hit and the Heal spell.
 - Improves the effectiveness of your Ghost Companion skill.
 - Improves the 'ceiling' for skills known by your Ghost Companion.

SHADOW COPY
 - Arch skill learned by Brawlers and Summoners.
 - Summons a companion, similar to Ghost Companion, on a short timer.
 - Timer improves as the power of Shadow Copy improves.
 - Shadow Copy creates a copy of your character's stats and abilities at the time of casting it.
 - Shadow Copies used by Brawlers will be strong melee fighters, while copies used by Summoners will be powerful casters.

HASTE
 - Arch skill learned by Warriors and Sorcerers.
 - Grants a buff to the caster, awarding additional action speed.
 - Effectiveness of the additional action speed tappers off as the number gets higher.
 - Cannot be cast on other players.
 - Has a duration of 10 minutes.
 - Has a base mana cost of 75.
 - Has a base cooldown of 3 seconds.

SURROUND AREA
 - Arch skill learned by Arch Templars.
 - Grants a bonus to the area-of-effect of Surround Hit.
 - Effectiveness of Surround Hit's damage gradually reduces as the number of hit targets increases.

SURROUND RATE
 - Arch skill learned by Warriors.
 - Grants an additional chance to hit during Surround Hit, improving the odds from 1:3 to 2:3.
 - Reduces the power of Surround Hit to less than half of normal by default.
 - Investing in Surround Rate will improve Surround Hit's damage, up to slightly below normal per hit.

FOCUS
 - Arch skill learned by Seyan'du.
 - You can no longer randomly lose focus when casting spells.
 - You can still fail to inflict debuffs on enemies that resist you.
 - Grants a bonus to your Spell Mod, at a rate of 0.001 per point.
