# openMerc server
* Get the Repo
* Run `chmod +x setup.sh`
* Run `./setup.sh`
* Change directories to `/home/merc/`
* Run the server with: `./server console`, `CTRL + C` will shutdown
* After further changes are made to the source, just run `make` and it will compile and deploy

## Web UI
* Open a browser and navigate to `localhost/cgi-imp/acct.cgi`
* Edit items, NPCs and templates
* **REMEMBER** This is editing the **LIVE** Server data, so be cautious, make backups.

## Website
* The regular website should be hosted at `localhost`

# Old README Info:
Please read the LICENSE

There is a bug somewhere if the server finds lost items (garbage collection) or
lost links!

Take great care not to call `act_xxx()` twice from a driver!

`MAXCHARS` may not exceed `0x7fff` since `0x8000` is used as a flag for
`do_look_depot()` in `svr_do` and `svr_tick`

usage of data for players:

* `0:`      away from keyboard
* `1-9:`    group with X,Y...
* `10:`     following X
* `11:`     no fightback
* `12:`     follow suspension timeout
* `13:`     money in bank
* `14:`     number of deaths
* `15:`     killed by X
* `16:`      date
* `17:`      area
* `18:`     current experience for pentagramas
* `19:`     lag timer
* `20:`     highest gorge solved
* `21:`     seyan'du sword: bits of shrines already used
* `22:`     current monster in arena
* `23:`     overall kill counter
* `24:`     --Zarro-- Extended poles 2
* `25:`     --Zarro-- Underdark entry M storage
* `26:`     Black Stronghold Kill Counter (to assure wave reward is legit)
* `27:`     --Zarro-- Gambling: Player hand value & wager
* `28:`     --Zarro-- Gambling: Dealer hand value & active game
* `29:`     other players killed outside of arena
* `30-39:`  soft ignore list
* `40:`     shopkeepers etc. killed
* `41:`     --Zarro-- Contract ID, mission, goal, progess
* `42:`     group
* `43:`     --Zarro-- Contract item ID for /who
* `44:`     number of times X was saved by the gods
* `45:`     level X got hp/end/mana bonus for
* `46-49:`  Flags of explorer points we visited (1,2,4,8,16)
* `50-59:`  hard ignore list
* `60-63:`  FKs   1/*, 2/*, 3/*, 4/*
* `64:`     current ghost companion
* `65:`     player #ALLOWed to search corpse // used for purple priest timer to free up dr67
* `66:`     Corpse's owner
* `67:`     --Zarro-- FKs 8/*
* `68:`     Date of last player attack (PCD_ATTACKTIME)
* `69:`     Last player attacked       (PCD_ATTACKVICT)
* `70:`		--Zarro-- FKs 5/*
* `71:`     number of says/tells/looks/lights
* `72:`     --Zarro-- Quests 1
* '73:'		--Zarro-- FKs 7/*
* '74:'     --Zarro-- for spell delays when hit by warcry
* '75:'		--Zarro-- for spell delays when hit by blast, cleave
* '76:'     --Zarro-- for waypoint storage
* '77:'     --Zarro-- for pentagram quest kill streak
* '78:'     --Zarro-- store highest pent for kill streak reward
* '79:'     --Zarro-- store latest version to prevent MotD pop-up
* `80-89:`	seen logging in from these class C nets
* `90:`     number in database
* `91:`		--Zarro-- Extended poles
* `92:`     weird unusable sleep timer slot
* `93:`		--Zarro-- FKs 6/*
* `94:`		--Zarro-- Quests 2
* `95:`		--Zarro-- current shadow copy
* `96:`     queued spells
* `97:`     computation time used last action
* `98:`     computation time used
* `99:`     used by populate

## Text:
* (`0-2` are clobbered by login)
* `0:`      afk message
* `3:`      mark message
