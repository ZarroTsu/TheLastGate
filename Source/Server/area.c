/*************************************************************************

   This file is part of 'Mercenaries of Astonia v2'
   Copyright (c) 1997-2001 Daniel Brockhaus (joker@astonia.com)
   All rights reserved.

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "server.h"

struct area
{
	int x1, y1;
	int x2, y2;
	char *name;
	int flag;       // 0=in, 1=the, 2=on, 3=at
};

struct area area[] = {
//	Order matters - the LAST value that fits will be used. So, broad areas first, specific areas second.
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 741,  205,  851,  237, "Lynbore",                        0},
	{ 696,  238,  851,  343, "Lynbore",                        0},
	{ 730,  344,  851,  382, "Lynbore",                        0},
	{ 783,  383,  851,  394, "Lynbore",                        0},
	{ 813,  395,  851,  428, "Lynbore",                        0},
	{ 749,  229,  823,  233, "Shore Crescent ",                2},
	{ 819,  234,  823,  293, "Shore Crescent ",                2},
	{ 749,  234,  753,  382, "First Street",                   2},
	{ 783,  234,  787,  353, "Second Street",                  2},
	{ 699,  294,  831,  298, "Silver Avenue",                  2},
	{ 827,  299,  831,  374, "Castle Way",                     2},
	{ 754,  354,  826,  358, "Rose Lane",                      2},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 756,  236,  778,  244, "Bluebird Tavern",                1},
	{ 756,  245,  776,  251, "Bluebird Tavern",                1},
	{ 756,  252,  780,  260, "Bluebird Tavern",                1},
	{ 756,  261,  772,  264, "Bluebird Tavern",                1},
	{ 756,  268,  766,  274, "Inga's House",                   0},
	{ 756,  275,  764,  278, "Inga's House",                   0},
	{ 756,  282,  766,  291, "Sirjan's House",                 0},
	{ 791,  302,  798,  311, "Jefferson's House",              0},
	{ 799,  304,  802,  311, "Jefferson's House",              0},
	{ 790,  336,  797,  350, "Steven's House",                 0},
	{ 835,  353,  842,  362, "Ingrid's House",                 0},
	{ 843,  353,  846,  360, "Ingrid's House",                 0},
	{ 814,  319,  824,  329, "Leopold's House",                0},
	{ 816,  302,  824,  314, "Gunther's House",                0},
	{ 805,  301,  812,  313, "Manfred's House",                0},
	{ 772,  269,  780,  279, "Serena's House",                 0},
	{ 774,  280,  780,  283, "Serena's House",                 0},
	{ 791,  236,  798,  248, "Gordon's House",                 0},
	{ 807,  236,  815,  248, "Edna's House",                   0},
	{ 808,  280,  815,  283, "Nasir's House",                  0},
	{ 806,  284,  815,  291, "Nasir's House",                  0},
	{ 735,  333,  745,  342, "Garna's House",                  0},
	{ 732,  347,  745,  353, "Lynbore Guardhouse",             1},
	{ 732,  354,  743,  358, "Lynbore Guardhouse",             1},
	{ 732,  359,  745,  383, "Lynbore Guardhouse",             1},
	{ 791,  282,  802,  290, "Lynbore Bank",                   1},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 710,  246,  746,  266, "Thieves House I",                1},
	{ 677,   21,  713,   41, "Thieves House Cellar",           1},
	{ 710,  246,  722,  266, "Thieves House II",               1},
	{ 702,  269,  746,  291, "Lynbore Park",                   0},
	{ 709,  301,  746,  329, "Cursed Tomb",                    1},
	{ 700,  310,  708,  329, "Cursed Tomb",                    1},
	{ 709,  330,  728,  338, "Cursed Tomb",                    1},
	{ 756,  308,  780,  351, "Lynbore Library",                1},
	{ 791,  317,  809,  330, "Joe's House",                    0},
	{ 783,  370,  807,  394, "Templar Outlaws",                3},
	{ 800,  333,  824,  350, "Skeleton Lord",                  3},
	{ 813,  375,  845,  428, "Haunted Castle",                 1},
	{ 835,  301,  851,  349, "Dungeon of Doors",               1},
	{ 791,  252,  815,  276, "Hedge Maze",                     1},
	{ 827,  270,  851,  290, "Magic Maze",                     1},
	{ 831,  264,  851,  269, "Magic Maze",                     1},
	{ 839,  291,  851,  294, "Magic Maze",                     1},
	{ 827,  270,  851,  290, "Magic Maze",                     1},
	{ 842,  295,  848,  300, "Magic Maze",                     1},
	{ 658,  238,  695,  343, "Lynbore Mines",                  1},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 661,  505,  667,  513, "Weeping Bend                  ", 1},
	{ 668,  504,  709,  512, "Weeping Bend                  ", 1},
	{ 730,  384,  782,  506, "Weeping Woods",                  1},
	{ 652,  462,  729,  506, "Weeping Woods",                  1},
	{ 783,  395,  812,  480, "Weeping Woods",                  1},
	{ 782,  395,  812,  410, "Spider's Den",                   1},
	{ 774,  411,  812,  437, "Spider's Den",                   1},
	{ 774,  438,  812,  464, "Abandoned Manor",                1},
	{ 782,  465,  812,  480, "Abandoned Manor",                1},
	{ 752,  478,  766,  492, "Stone Circle",                   1},
	{ 707,  477,  719,  489, "Argha's Ambush",                 3},
	{ 718,  498,  728,  505, "Argha's Hideout",                0},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 495,  417,  658,  460, "Aston",                          0},
	{ 493,  447,  494,  453, "Aston",                          0},
	{ 495,  461,  650,  586, "Aston",                          0},
	{ 477,  524,  494,  558, "Aston",                          0},
	{ 491,  559,  494,  586, "Aston",                          0},
	{ 526,  587,  553,  597, "Aston",                          0},
	{ 584,  411,  596,  416, "Aston",                          0},
	{ 538,  417,  542,  586, "Temple Street",                  2},
	{ 635,  428,  639,  480, "South End",                      2},
	{ 504,  539,  600,  543, "Rose Street",                    2},
	{ 596,  486,  600,  575, "Marble Lane",                    2},
	{ 498,  481,  650,  485, "Merchant's Way",                 2},
	{ 501,  423,  639,  427, "Bulwark Avenue",                 2},
	{ 643,  460,  650,  474, "Gene's House",                   0},
	{ 603,  527,  613,  537, "April's House",                  0},
	{ 584,  547,  592,  559, "Richie's House",                 0},
	{ 505,  548,  517,  556, "Alphonse's House",               0},
	{ 621,  467,  631,  477, "Cherri's House",                 0},
	{ 578,  466,  585,  469, "Rocky's House",                  0},
	{ 578,  470,  587,  477, "Rocky's House",                  0},
	{ 525,  459,  528,  467, "Roxie's House",                  0},
	{ 529,  459,  535,  470, "Roxie's House",                  0},
	{ 611,  466,  618,  469, "Elric's House",                  0},
	{ 609,  470,  618,  477, "Elric's House",                  0},
	{ 546,  431,  554,  443, "Lucci's House",                  0},
	{ 643,  444,  650,  451, "Robin's House",                  0},
	{ 643,  452,  654,  456, "Robin's House",                  0},
	{ 604,  546,  613,  557, "Marline's House",                0},
	{ 525,  547,  535,  557, "Rufus's House",                  0},
	{ 546,  547,  554,  559, "Gomez's House",                  0},
	{ 546,  467,  555,  477, "Donna's House",                  0},
	{ 525,  441,  533,  447, "Oscar's House",                  0},
	{ 525,  448,  535,  452, "Oscar's House",                  0},
	{ 584,  563,  592,  575, "Castor's House",                 0},
	{ 603,  561,  613,  570, "Grover's House",                 0},
	{ 650,  426,  654,  440, "Monica's House",                 0},
	{ 643,  434,  649,  440, "Monica's House",                 0},
	{ 517,  432,  521,  438, "Shera's House",                  0},
	{ 522,  430,  530,  438, "Shera's House",                  0},
	{ 527,  566,  534,  575, "Aster's House",                  0},
	{ 523,  568,  526,  577, "Aster's House",                  0},
	{ 496,  431,  507,  439, "Blanche's House",                0},
	{ 584,  411,  596,  419, "Serena's House",                 0},
	{ 515,  470,  526,  478, "North Aston Bank",               1},
	{ 604,  489,  615,  497, "South Aston Bank",               1},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 499,  492,  519,  503, "Temple of Skua",                 1},
	{ 499,  504,  531,  520, "Temple of Skua",                 1},
	{ 505,  521,  519,  535, "Temple of Skua",                 1},
	{ 549,  446,  585,  464, "Temple of The Purple One",       1},
	{ 559,  465,  575,  474, "Temple of The Purple One",       1},
	{ 551,  494,  587,  530, "Merchant's Court",               1},
	{ 503,  444,  521,  466, "The Arena",                      3},
	{ 589,  449,  611,  460, "Damor's Magic Shop",             0},
	{ 589,  461,  607,  466, "Damor's Magic Shop",             0},
	{ 591,  467,  605,  477, "Damor's Magic Shop",             0},
	{ 628,  489,  634,  511, "South Aston Guardhouse",         1},
	{ 635,  491,  639,  511, "South Aston Guardhouse",         1},
	{ 640,  489,  646,  511, "South Aston Guardhouse",         1},
	{ 616,  443,  631,  462, "Blue Ogre Tavern",               1},
	{  93,   58,  112,   81, "Blue Ogre Bedrooms",             1},
	{ 608,  502,  623,  522, "House of Cards",                 1},
	{ 559,  547,  579,  581, "Butler's Mansion",               1},
	{ 496,  563,  514,  581, "Abandoned Bell House",           1},
	{ 477,  524,  501,  558, "Aston Graveyard",                1},
	{ 614,  524,  650,  586, "Aston Park",                     0},
	{ 485,  436,  494,  444, "Aston Mines, Level I",           1},
	{ 442,  445,  492,  455, "Aston Mines, Level I",           1},
	{ 442,  456,  495,  509, "Aston Mines, Level I",           1},
	{ 470,  510,  495,  523, "Aston Mines, Level I",           1},
	{ 442,  436,  484,  444, "Fire Pents, Approach",           1},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 781,   21,  835,   32, "Aston Mines, Level II",          1},
	{ 781,   33,  831,   37, "Aston Mines, Level II",          1},
	{ 781,   38,  835,  108, "Aston Mines, Level II",          1},
	{ 726,   21,  780,  108, "Aston Mines, Level III",         1},
	{ 832,   33,  835,   37, "Underground I",                  1},
	{ 836,   21, 1004,  115, "Underground I",                  1},
	{ 851,  116,  931,  123, "Underground I",                  1},
	{ 887,  124,  919,  139, "Underground I",                  1},
	{ 951,  116,  991,  123, "Underground I",                  1},
	{ 963,  124,  975,  131, "Underground I",                  1},
	{ 936,  116,  946,  123, "Arachnid Den",                   1},
	{ 920,  124,  962,  142, "Arachnid Den",                   1},
	{ 963,  136,  975,  142, "Arachnid Den",                   1},
	{ 976,  124, 1000,  142, "Arachnid Den",                   1},
	{ 996,  116, 1000,  123, "Arachnid Den",                   1},
	{ 838,  125,  856,  142, "Bell House Basement",            1},
	{ 880,  146, 1004,  270, "Underground II",                 1},
	{ 884,  275, 1004,  359, "Underground III",                1},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 495,  205,  538,  416, "Strange Forest",                 1},
	{ 539,  373,  613,  416, "Strange Forest",                 1},
	{ 614,  205,  657,  416, "Strange Forest",                 1},
	{ 538,  281,  562,  300, "Stronghold Barracks, North",     1},
	{ 563,  281,  589,  300, "Stronghold Approach",            1},
	{ 590,  281,  614,  300, "Stronghold Barracks, South",     1},
	{ 539,  301,  563,  372, "Stronghold, First Line",         1},
	{ 564,  301,  588,  372, "Stronghold, Second Line",        1},
	{ 589,  301,  613,  372, "Stronghold, Third Line",         1},
	{ 538,  206,  614,  280, "Black Stronghold",               1},
	{ 599,  218,  613,  232, "Shiva's Chamber",                0},
	{ 558,  114,  609,  195, "Black Stronghold Basement",      1},
	{ 610,  178,  626,  192, "Black Stronghold Basement",      1},
	{ 569,  115,  583,  129, "Shiva's Chamber",                0},
	{ 652,  382,  656,  387, "Webbed Bush",                    1},
	{ 633,  388,  656,  392, "Webbed Bush",                    1},
	{ 626,  393,  656,  415, "Webbed Bush",                    1},
	{ 502,  323,  531,  372, "Serpent Pond",                   3},
	{ 502,  323,  531,  372, "Serpent River",                  3},
	{ 614,  319,  649,  365, "Lavender Lake",                  3},
	{ 442,  363,  494,  435, "Astonia Penitentiary",           1},
	{ 620,  266,  634,  278, "The Mad Hermit's House",         3},
	{ 669,   53,  714,  123, "Lavender Lakebed",               1},
	{ 674,  135,  713,  145, "Strange Grotto",                 1},
	{ 442,  232,  495,  362, "Gargoyle Nest",                  1},
	{ 658,  205,  740,  237, "Southern Swamp",                 1},
	{ 217,  205,  494,  231, "Northern Mountains",             2},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 783,  482,  852,  505, "Forgotten Canyon",               1},
	{ 651,  506,  661,  553, "Forgotten Canyon",               1},
	{ 662,  514,  714,  553, "Forgotten Canyon",               1},
	{ 662,  506,  852,  553, "Forgotten Canyon",               1},
	{ 651,  554,  748,  569, "Forgotten Canyon",               1},
	{ 782,  554,  852,  586, "Forgotten Canyon",               1},
	{ 651,  570,  661,  598, "Forgotten Tunnel",               1},
	{ 668,  570,  748,  594, "Scorpion Burrow",                1},
	{ 696,  595,  744,  619, "Scorpion Hive",                  1},
	{ 782,  587,  852,  608, "Jagged Pass",                    1},
	{ 621,   21,  664,   41, "Jagged Pass, Cellar",            1},
	{ 782,  609,  852,  710, "Winding Valley",                 1},
	{ 790,  681,  800,  691, "Regal's House",                  0},
	{ 846,  426,  852,  435, "Abandoned Archives",             1},
	{ 819,  436,  852,  481, "Abandoned Archives",             1},
	{ 621,   54,  663,  146, "Forbidden Tomes",                1},
	{ 662,  595,  694,  598, "Basalt Desert",                  1},
	{ 651,  599,  694,  755, "Basalt Desert",                  1},
	{ 695,  620,  748,  755, "Basalt Desert",                  1},
	{ 749,  587,  781,  755, "Basalt Desert",                  1},
	{ 745,  595,  748,  619, "Basalt Desert",                  1},
	{ 663,  606,  683,  626, "Circle of Fire",                 1},
	{ 689,  726,  700,  737, "The Oasis",                      3},
	{ 709,  642,  766,  702, "Empty Outset",                   1},
	{ 976,  372, 1004,  394, "Outset Den",                     1},
	{ 639,  720,  665,  748, "The Basalt Ziggurat",            0},
	{ 666,  730,  672,  738, "The Basalt Ziggurat",            0},
	{ 936,  549, 1005,  618, "The Basalt Ziggurat",            0},
	{ 749,  554,  781,  586, "The Tower",                      0},
	{ 438,   72,  470,  104, "The Tower II",                   0},
	{ 438,   34,  470,   66, "The Tower III",                  0},
	{ 400,   34,  432,   66, "The Tower IV",                   0},
	{ 400,   72,  432,  104, "The Tower V",                    0},
	{ 400,  110,  432,  142, "The Tower VI",                   0},
	{ 400,  148,  432,  180, "The Tower VII",                  0},
	{ 362,  148,  394,  180, "The Tower VIII",                 0},
	{ 362,  110,  394,  142, "The Tower IX",                   0},
	{ 324,  110,  356,  142, "The Tower X",                    0},
	{ 324,  148,  356,  180, "The Tower XI",                   0},
	{ 286,  148,  318,  180, "The Tower XII",                  0},
	{ 286,  110,  318,  142, "The Tower XIII",                 0},
	{ 248,  110,  280,  142, "The Tower XIV",                  0},
	{ 248,  148,  280,  180, "The Tower XV",                   0},
	{ 210,  148,  242,  180, "The Tower XVI",                  0},
	{ 210,  110,  242,  142, "The Tower XVII",                 0},
	{ 172,  110,  204,  142, "The Tower XVIII",                0},
	{ 172,  148,  204,  180, "The Tower XIX",                  0},
	{ 172,  186,  204,  234, "The Tower XX",                   0},
	{ 172,  240,  204,  288, "The Tower Rewards",              3},
	{ 438,  110,  470,  142, "The Abyss I",                    0},
	{ 438,  148,  470,  180, "The Abyss II",                   0},
	{ 476,  148,  508,  180, "The Abyss III",                  0},
	{ 476,  110,  508,  142, "The Abyss IV",                   0},
	{ 476,   72,  508,  104, "The Abyss V",                    0},
	{ 476,   34,  508,   66, "The Abyss VI",                   0},
	{ 514,   34,  546,   66, "The Abyss VII",                  0},
	{ 514,   72,  546,  104, "The Abyss VIII",                 0},
	{ 514,  110,  546,  142, "The Abyss IX",                   0},
	{ 523,  148,  537,  180, "The Abyss X",                    0},
	{ 142,  264,  158,  280, "The Abyss Rewards",              3},
	{ 514,  148,  522,  180, "The Abyss Rewards",              3},
	{ 538,  148,  546,  180, "The Abyss Rewards",              3},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 554,  587,  650,  597, "Astonian Farms",                 1},
	{ 506,  598,  650,  639, "Astonian Farms",                 1},
	{ 506,  640,  650,  650, "Autumn Meadow",                  1},
	{ 475,  598,  505,  650, "Autumn Meadow",                  1},
	{ 443,  651,  650,  717, "Autumn Meadow",                  1},
	{ 442,  718,  637,  755, "Autumn Meadow",                  1},
	{ 841,  156,  869,  190, "Old Well",                       1},
	{ 442,  598,  474,  650, "Crumbling Cathedral",            1},
	{ 781,  156,  831,  190, "Cathedral Basement",             1},
	{ 557,   20,  609,  102, "Strider Hatchery",               1},
	{ 578,  687,  637,  755, "Thug's Camp",                    1},
	{ 638,  687,  649,  718, "Thug's Camp",                    1},
	{ 621,  159,  638,  165, "Buried Brush",                   1},
	{ 639,  159,  713,  192, "Buried Brush",                   1},
	{ 389,  598,  442,  684, "Ice Gargoyle Nest",              1},
	{ 389,  685,  441,  694, "Lizard Temple, Approach",        1},
	{ 425,  695,  441,  745, "Lizard Temple, Approach",        1},
	{ 389,  746,  441,  755, "Lizard Temple, Approach",        1},
	{ 366,  695,  424,  745, "Lizard Temple",                  1},
	{ 366,  674,  388,  694, "The Beryl Jungle",               0},
	{ 366,  746,  388,  755, "The Beryl Jungle",               0},
	{ 310,  674,  365,  755, "The Beryl Jungle",               0},
	{ 302,  680,  309,  755, "The Beryl Jungle",               0},
	{ 188,  685,  301,  755, "The Beryl Jungle",               0},
	{ 178,  691,  187,  755, "The Beryl Jungle",               0},
	{ 165,  702,  177,  755, "The Beryl Jungle",               0},
	{ 188,  685,  247,  755, "Settlement",                     1},
	{ 178,  691,  187,  755, "Settlement",                     1},
	{ 165,  702,  177,  755, "Settlement",                     1},
	{ 314,  598,  388,  673, "Platinum Mines",                 1},
	{ 968,  460,  976,  470, "Platinum Mines II",              1},
	{ 977,  460, 1004,  534, "Platinum Mines II",              1},
	{ 296,  587,  324,  597, "Jungle Pents, Approach",         1},
	{  97,  489,  134,  625, "The Emerald Cavern",             0},
	{ 135,  489,  144,  513, "The Emerald Cavern",             0},
	{ 145,  489,  155,  503, "The Emerald Cavern",             0},
	{ 135,  551,  177,  701, "The Emerald Cavern",             0},
	{ 186,  534,  200,  542, "The Emerald Cavern",             0},
	{ 178,  543,  200,  550, "The Emerald Cavern",             0},
	{ 178,  551,  200,  684, "The Emerald Cavern",             0},
	{ 178,  685,  187,  690, "The Emerald Cavern",             0},
	{ 201,  599,  301,  684, "The Emerald Cavern",             0},
	{ 302,  599,  309,  679, "The Emerald Cavern",             0},
	{ 151,  419,  200,  488, "The Emerald Valley",             3},
	{ 156,  489,  200,  503, "The Emerald Valley",             3},
	{ 145,  504,  200,  513, "The Emerald Valley",             3},
	{ 135,  514,  177,  550, "The Emerald Valley",             3},
	{ 178,  514,  185,  542, "The Emerald Valley",             3},
	{ 186,  514,  200,  533, "The Emerald Valley",             3},
	{ 151,  407,  181,  418, "The Emerald Approach",           3},
	{ 120,  304,  200,  406, "The Emerald Palace",             3},
	{ 182,  407,  200,  418, "The Emerald Palace",             3},
	{ 884,  377,  964,  491, "The Emerald Palace",             3},
	{ 141,  430,  150,  445, "The Hideout",                    3},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 165,  756,  205,  796, "The Beryl Jungle",               0},
	{ 165,  797,  205,  920, "Ashen Channel",                  1},
	{ 206,  890,  255,  920, "Heated Passage",                 1},
	{ 173,  921,  255, 1003, "The Volcano",                    3},
	{ 199,  947,  229,  977, "Summit",                         2},
	{ 120,  826,  164,  920, "Onyx Gargoyle Nest",             1},
	{ 281,  844,  297,  848, "The Burning Plains",             2},
	{ 217,  851,  227,  853, "The Burning Plains",             2},
	{ 228,  849,  248,  853, "The Burning Plains",             2},
	{ 264,  849,  297,  853, "The Burning Plains",             2},
	{ 217,  854,  297,  889, "The Burning Plains",             2},
	{ 269,  890,  297,  930, "The Burning Plains",             2},
	{ 217,  767,  280,  848, "The Obsidian Fortress",          0},
	{ 281,  767,  297,  843, "The Obsidian Fortress",          0},
	{ 217,  849,  227,  850, "The Obsidian Fortress",          0},
	{ 249,  849,  263,  853, "The Obsidian Fortress",          0},
	{ 279,  931,  287,  939, "The Archon's Gate",              3},
	{ 269,  975,  297, 1003, "The Archon's Trial",             3},
	{ 269,  953,  297,  961, "The Archon's Reward",            3},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 644,  767,  650,  843, "Neiseer",                        0},
	{ 651,  767,  851,  877, "Neiseer",                        0},
	{ 738,  756,  781,  766, "Neiseer",                        0},
	{ 782,  720,  851,  766, "Neiseer",                        0},
	{ 737,  878,  765,  899, "Neiseer",                        0},
	{ 685,  848,  698,  864, "Temple of Kwai",                 1},
	{ 699,  832,  715,  868, "Temple of Kwai",                 1},
	{ 716,  848,  729,  864, "Temple of Kwai",                 1},
	{ 773,  780,  786,  796, "Temple of Gorn",                 1},
	{ 787,  775,  803,  809, "Temple of Gorn",                 1},
	{ 788,  810,  802,  812, "Temple of Gorn",                 1},
	{ 804,  780,  817,  796, "Temple of Gorn",                 1},
	{ 682,  828,  690,  839, "North Bank",                     1},
	{ 812,  805,  820,  816, "South Bank",                     1},
	{ 777,  842,  791,  852, "Ealdwulf's Shop",                0},
	{ 757,  853,  800,  861, "Merchant's Alley",               1},
	{ 792,  842,  800,  852, "Merchant's Alley",               1},
	{ 792,  842,  800,  852, "Merchant's Alley",               1},
	{ 787,  726,  813,  755, "Merlin's Magic Shoppe",          0},
	{ 787,  756,  807,  766, "Merlin's Magic Shoppe",          0},
	{ 787,  767,  795,  768, "Merlin's Magic Shoppe",          0},
	{ 814,  733,  823,  751, "Merlin's Magic Shoppe",          0},
	{ 727,  776,  745,  803, "Dueling Dragons Tavern",         1},
	{ 743,  900,  759,  912, "Colosseum Entrance",             1},
	{ 700,  890,  736,  926, "Colosseum",                      1},
	{ 700,  940,  736,  976, "Colosseum",                      1},
	{ 733,  915,  769,  951, "Colosseum",                      1},
	{ 766,  890,  802,  926, "Colosseum",                      1},
	{ 766,  940,  802,  976, "Colosseum",                      1},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 673,  819,  679,  825, "North Crossing",                 2},
	{ 704,  819,  710,  825, "Kwai's Crossing",                2},
	{ 748,  819,  754,  825, "Center Crossing",                2},
	{ 826,  819,  832,  825, "South Crossing",                 2},
	{ 651,  820,  672,  824, "Titan Street",                   2},
	{ 680,  820,  703,  824, "Titan Street",                   2},
	{ 711,  820,  747,  824, "Titan Street",                   2},
	{ 755,  820,  825,  824, "Titan Street",                   2},
	{ 833,  820,  851,  824, "Titan Street",                   2},
	{ 674,  826,  678,  874, "Last Avenue",                    2},
	{ 705,  771,  709,  818, "Turtle Road",                    2},
	{ 749,  769,  753,  818, "Victory Road",                   2},
	{ 749,  826,  753,  895, "Victory Road",                   2},
	{ 827,  723,  831,  818, "Warlock Way",                    2},
	{ 827,  826,  831,  877, "Warlock Way",                    2},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 968,  407, 1004,  447, "Merlin's Laboratory",            0},
	{ 553,  767,  643,  817, "Ravaged Prairie",                1},
	{ 565,  818,  643,  826, "Ravaged Prairie",                1},
	{ 553,  827,  643,  877, "Ravaged Prairie",                1},
	{ 644,  844,  650,  877, "Ravaged Prairie",                1},
	{ 536,  818,  564,  826, "The Scar of Astonia",            3},
	{ 864,  552,  920,  612, "Smuggler's Hovel",               1},
	{ 547,  878,  587, 1003, "Hollow Trench",                  1},
	{ 490,  891,  546, 1003, "Cold Cavern",                    1},
	{ 588,  889,  618, 1003, "Lost Thicket",                   1},
	{ 619,  889,  629, 1003, "Hidden Village",                 1},
	{ 630,  889,  676,  972, "Hidden Village",                 1},
	{ 677,  889,  686, 1003, "Hidden Village",                 1},
	{ 630,  973,  676, 1003, "Seppuku House",                  1},
	{ 852,  720,  924,  836, "Raging Rivers",                  1},
	{ 871,  837,  924,  840, "Raging Rivers",                  1},
	{ 890,  841,  924,  847, "Raging Rivers",                  1},
	{ 895,  848,  924,  855, "Raging Rivers",                  1},
	{ 852,  837,  870,  840, "The Widow's Nest",               0},
	{ 852,  841,  889,  847, "The Widow's Nest",               0},
	{ 852,  848,  894,  855, "The Widow's Nest",               0},
	{ 852,  856,  924,  877, "The Widow's Nest",               0},
	{ 864,  630,  924,  719, "Wellspring Chasm",               1},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 658,  344,  730,  461, "Novice Pentagram Quest",         1},
	{ 307,  445,  441,  586, "Earth Pentagram Quest",          1},
	{ 308,  498,  316,  514, "Grolm Laboratory",               1},
	{ 262,  303,  306,  373, "Fire Pentagram Quest",           1},
	{ 307,  303,  441,  444, "Fire Pentagram Quest",           1},
	{ 262,  374,  306,  444, "Jungle Pentagram Quest",         1},
	{ 217,  445,  306,  586, "Jungle Pentagram Quest",         1},
	{ 217,  232,  261,  444, "Ice Pentagram Quest",            1},
	{ 262,  232,  351,  302, "Sea Pentagram Quest",            1},
	{ 352,  232,  441,  302, "Onyx Pentagram Quest",           1},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 456,  538,  476,  544, "Mausoleum Entrance",             1},
	{ 442,  545,  462,  551, "Mausoleum Entrance",             1},
	{ 456,  552,  462,  572, "Mausoleum Entrance",             1},
	{ 463,  566,  476,  572, "Mausoleum Entrance",             1},
	{ 456,  510,  469,  537, "Mausoleum I",                    1},
	{ 470,  524,  476,  537, "Mausoleum I",                    1},
	{ 442,  510,  455,  544, "Mausoleum II",                   1},
	{ 442,  552,  455,  586, "Mausoleum III",                  1},
	{ 456,  573,  490,  586, "Mausoleum IV",                   1},
	{ 463,  545,  476,  565, "Mausoleum V",                    1},
	{ 477,  559,  490,  572, "Mausoleum V",                    1},
	{ 782,  130,  795,  136, "Mausoleum Basement",             1},
	{ 796,  123,  802,  136, "Mausoleum Basement",             1},
	{ 782,  116,  802,  122, "Mausoleum Basement",             1},
	{ 768,  109,  788,  115, "Mausoleum Basement",             1},
	{ 761,  109,  767,  122, "Mausoleum Basement",             1},
	{ 754,  123,  767,  129, "Mausoleum Basement",             1},
	{ 782,  123,  795,  129, "Mausoleum VI",                   1},
	{ 775,  123,  781,  136, "Mausoleum VI",                   1},
	{ 775,  137,  802,  143, "Mausoleum VI",                   1},
	{ 803,  130,  809,  143, "Mausoleum VI",                   1},
	{ 803,  123,  830,  129, "Mausoleum VII",                  1},
	{ 810,  130,  830,  143, "Mausoleum VII",                  1},
	{ 789,  109,  802,  115, "Mausoleum VIII",                 1},
	{ 803,  109,  830,  122, "Mausoleum VIII",                 1},
	{ 768,  116,  781,  122, "Mausoleum IX",                   1},
	{ 768,  123,  774,  136, "Mausoleum IX",                   1},
	{ 754,  130,  767,  136, "Mausoleum IX",                   1},
	{ 747,  137,  774,  143, "Mausoleum IX",                   1},
	{ 726,  109,  760,  122, "Mausoleum X",                    1},
	{ 726,  123,  753,  136, "Mausoleum XI",                   1},
	{ 726,  137,  746,  143, "Mausoleum XI",                   1},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{ 132,  112,  152,  132, "Arena",     					   0},
	{  21,   21,   78,   71, "Lab I, Grolm Gorge",             0},
	{  21,   94,  111,  146, "Lab II, Lizard Gorge",           0},
	{  21,  169,  111,  223, "Lab III, Undead Gorge",          0},
	{  21,  246,  115,  302, "Lab IV, Caster Gorge",           0},
	{  21,  325,  101,  383, "Lab V, Knight Gorge",            0},
	{  21,  406,  133,  466, "Lab VI, Desert Gorge",           0},
	{  21,  489,   71,  549, "Lab VII, Light/Dark Gorge",      0},
	{  21,  572,   83,  634, "Lab VIII, Underwater Gorge",     0},
	{  21,  657,  125,  706, "Lab IX, Riddle Gorge",           0},
	{  21,  729,  147,  809, "Lab X, Forest Gorge",            0},
	{  21,  832,  103,  914, "Lab XI, Seasons Gorge",          0},
	{  21,  937,  156, 1003, "Lab XII, Ascent Gorge",          0},
	{ 132,  153,  152,  167, "Lab XIII, Final Gorge",          0},
	{ 132,  168,  152,  181, "Lab XIII, Final Preparation",    0},
	{ 136,  182,  148,  192, "Lab XIII, Final Test",           0},
	{ 136,  193,  148,  246, "Lab XIII, Final Reward",         0},
	{  93, 1025,  157, 1113, "Lab XIV, Miner's Gorge",         0},
	{  93, 1134,  183, 1224, "Lab XV, Vantablack Gorge",       0},
	{  93, 1245,  185, 1337, "Lab XVI, Pirate's Gorge",        0},
	{  93, 1358,  168, 1452, "Lab XVII, Gargoyle's Gorge",     0},
	{ 884,  504,  964,  534, "Hall of Lost Souls",             1},
	{ 738,  972,  764, 1003, "Temple of Osiris",               1},
//   _x1_, _y1_, _x2_, _y2_, "Area Name",                      0 = in  /  1 = the  /  2 = on  /  3 = at
	{  91,   21,  115,   45, "Random Dungeon Entrance",        1},
	{ 131,   21,  201,   91, "Random Dungeon I",               1},
	{ 216,   21,  286,   91, "Random Dungeon II",              1},
	{ 301,   21,  371,   91, "Random Dungeon III",             1},
	{  20, 1025,   73, 1590, "Contract Zone ",                 1}
};

extern struct waypoint waypoint[MAXWPS] = {
//    _x_, _y_, "123456789012345678901234567890", 	"..."
	{ 778, 265, "Lynbore, Tavern", 					"outside the Bluebird Tavern in Lynbore" },
	{ 739, 393, "Lynbore, East", 					"the beginning of the Weeping Woods, east of Lynbore" },
	{ 657, 471, "Aston, South", 					"the end of the Weeping Woods, south of Aston" },
	{ 548, 491, "Aston, Crossroads", 				"the crossroads in the middle of Aston" },
	{ 550, 399, "Aston, West", 						"the entrance to the Strange Forest, west of Aston" },
	{ 550, 606, "Aston, East", 						"the entrance to the Autumn Meadow, east of Aston" },
	{ 796, 495, "Forgotten Canyon", 				"the entrance to the Forgotten Canyon" },
	{ 359, 711, "Lizard Temple, North", 			"the entrance to the Beryl Jungle, north of the Lizard Temple" },
	{ 758, 997, "Temple of Osiris", 				"the Temple of Osiris" },
	{ 742, 745, "Neiseer, West", 					"the entrance to the Basalt Desert, west of Neiseer" },
	{ 761, 832, "Neiseer", 							"the middle of Neiseer" },
	{ 635, 813, "Neiseer, North", 					"the entrance to the Ravaged Prairie, north of Neiseer" },
	{ 785, 106, "Mausoleum, Basement", 				"the basement of the Mausoleum" },
	{ 661, 347, "Pentagram Quest, Novice", 			"the beginning of the Novice Pentagram Quest" },
	{ 438, 448, "Pentagram Quest, Earth", 			"the beginning of the Earth Pentagram Quest" },
	{ 310, 441, "Pentagram Quest, Fire", 			"the beginning of the Fire Pentagram Quest" },
	{ 303, 377, "Pentagram Quest, Jungle", 			"the beginning of the Jungle Pentagram Quest" },
	{ 220, 441, "Pentagram Quest, Ice", 			"the beginning of the Ice Pentagram Quest" },
	{ 265, 235, "Pentagram Quest, Underwater", 		"the beginning of the Underwater Pentagram Quest" },
	{ 355, 235, "Pentagram Quest, Onyx", 			"the beginning of the Onyx Pentagram Quest" },
	{ 431, 729, "Lizard Temple, South", 			"the entrance to the Lizard Temple, north of the Autumn Meadow" },
	{ 177, 756, "Lizard Settlement, East", 		    "the north-east end of the Lizard Settlement" },
	{ 156, 544, "The Emerald Cavern, East", 		"the western side of the Emerald Cavern" },
	{ 771, 595, "The Tower", 						"the entrance to The Tower" },										//   8388608
	{ 512, 512, "Neiseer, East", 					"the entrance to the Violet Bog, east of Neiseer" },				//  16777216
	{ 867, 813, "Neiseer, South", 					"the entrance to the Raging Rivers, south of Neiseer" },			//  33554432
	{ 292, 921, "Burning Plains", 					"the Burning Plains" }												//  67108864
};

// return a description of the area character cn is in
char *get_area(int cn, int verbose)
{
	int n, flag = 0;
	static char buf[512];

	buf[0] = 0;

	if (cn<1 || cn>=MAXCHARS)
	{
		return( buf);
	}

	for (n = 0; n<sizeof(area) / sizeof(area[0]); n++)
	{
		if (ch[cn].x>=area[n].x1 && ch[cn].y>=area[n].y1 &&
		    ch[cn].x<=area[n].x2 && ch[cn].y<=area[n].y2)
		{
			if (verbose)
			{
				if (!flag)
				{
					strcat(buf, "In ");
					flag = 1;
				}
				else
				{
					strcat(buf, ", in ");
				}
				if (area[n].flag==1)
				{
					strcat(buf, "the ");
				}
				else if (area[n].flag==2)
				{
					strcat(buf, "on ");
				}
				else if (area[n].flag==3)
				{
					strcat(buf, "at ");
				}
				strcat(buf, area[n].name);
			}
			else
			{
				if (!flag)
				{
					flag = 1;
				}
				else
				{
					strcat(buf, ", ");
				}
				strcat(buf, area[n].name);
			}
			if (area[n].y1==1025 && area[n].y2==1590)
			{
				char con[8];
				sprintf(con, "%d", CONT_NUM(cn));
				strcat(buf, con);
			}
		}
	}

	return(buf);
}

//return the area description for x,y
char *get_area_m(int x, int y, int verbose)
{
	int n, flag = 0;
	static char buf[512];

	buf[0] = 0;

	for (n = 0; n<sizeof(area) / sizeof(area[0]); n++)
	{
		if (x>=area[n].x1 && y>=area[n].y1 &&
		    x<=area[n].x2 && y<=area[n].y2)
		{
			if (verbose)
			{
				if (!flag)
				{
					strcat(buf, "in ");
					flag = 1;
				}
				else
				{
					strcat(buf, ", in ");
				}
				if (area[n].flag==1)
				{
					strcat(buf, "the ");
				}
				else if (area[n].flag==2)
				{
					strcat(buf, "on ");
				}
				else if (area[n].flag==3)
				{
					strcat(buf, "at ");
				}
				strcat(buf, area[n].name);
			}
			else
			{
				if (!flag)
				{
					flag = 1;
				}
				else
				{
					strcat(buf, ", ");
				}
				strcat(buf, area[n].name);
			}
		}
	}

	return(buf);
}
