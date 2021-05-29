#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "cgi-lib.h"
#include "html-lib.h"
#include "gendefs.h"
#include "data.h"

struct map *map;
struct item *it;
struct item *it_temp;
struct mapedit_queue *maped_queue;

static int load(void)
{
        int handle;
        char cwd[128];

        // Load map
        handle=open("../"DATDIR"/map.dat",O_RDWR);
        if (handle==-1) {
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                fprintf(stderr, "cwd: %s\n", cwd);
            perror("../"DATDIR"/map.dat");
            return -1;
        }
        map=mmap(NULL,MAPSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (map==(void*)-1) {
            fprintf(stderr,"cannot mmap map.dat.\n");
            return -1;
        }
        close(handle);

        // Load map editor queue
        handle=open("../"DATDIR"/mapedQ.dat",O_RDWR);
        if (handle==-1) {
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                fprintf(stderr, "cwd: %s\n", cwd);
            perror("../"DATDIR"/mapedQ.dat");
            return -1;
        }
        maped_queue=mmap(NULL,MAPED_QUEUE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (maped_queue==(void*)-1) {
            fprintf(stderr,"cannot mmap mapedQ.dat.\n");
            return -1;
        }
        close(handle);

        // Load item templates
        handle=open("../"DATDIR"/titem.dat",O_RDWR);
        if (handle==-1) {
                fprintf(stderr,"titem.dat does not exist.\n");
                return -1;
        }

        it_temp=mmap(NULL,TITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (it_temp==(void*)-1) {
                fprintf(stderr,"cannot mmap titem.dat.\n");
                return -1;
        }
        close(handle);

        // Load items
        handle=open("../"DATDIR"/item.dat",O_RDWR);
        if (handle==-1) {
                fprintf(stderr,"item.dat does not exist.\n");
                return -1;
        }

        it=mmap(NULL,ITEMSIZE,PROT_READ|PROT_WRITE,MAP_SHARED,handle,0);
        if (it==(void*)-1) {
                fprintf(stderr,"cannot mmap item.dat.\n");
                return -1;
        }
        close(handle);

        return 0;
}

static void unload(void)
{
    if (munmap(map,MAPSIZE)) perror("munmap(map)");
    if (munmap(maped_queue,MAPED_QUEUE_SIZE)) perror("munmap(maped_queue)");
    if (munmap(it,ITEMSIZE)) perror("munmap(it)");
    if (munmap(it_temp,TITEMSIZE)) perror("munmap(it_temp)");
}

// Load item/wall templates from server
void scriptLoadTemplates()
{
    printf("<script>\n");
    for (int i=2; i<MAXTITEM; i++) {
        if (it_temp[i].used == USE_EMPTY) continue;
        if (it_temp[i].flags&(IF_TAKE)) continue;

        char *it_type = "wall";
        char *it_color = "gray";

        // Considers templates with any of these flags as 'item' type
        if (it_temp[i].flags&(IF_TAKE|IF_LOOK|IF_LOOKSPECIAL|IF_USE|IF_USESPECIAL) || !(it_temp[i].flags&(IF_MOVEBLOCK))) {
            it_type = "item";
            it_color = "yellow";
        }
        printf("item_templates[\"it_temp\" + %d] = new ItemTemp(%d, %d, \"%s\", \"%s\", `%s`);\n", i, i, it_temp[i].sprite[0], it_type, it_color, it_temp[i].name);
        if (it_temp[i].flags&(IF_MOVEBLOCK)) printf("item_templates[\"it_temp\" + %d].flags.moveblock = true;\n", i);
        if (it_temp[i].flags&(IF_SIGHTBLOCK)) printf("item_templates[\"it_temp\" + %d].flags.sightblock = true;\n", i);
    }
    printf("</script>\n");
}

void addQueueInstruction(char type, int x, int y, int it_val)
{
    for (int i=0; i<MAX_MAPED_QUEUE; i++) {
        if (maped_queue[i].used == USE_ACTIVE) {
            // Operating in the same position
            if (maped_queue[i].x == x && maped_queue[i].y == y) {
                // Same operation
                if (maped_queue[i].op_type == type) {
                    // Flag operation
                    if (it_val&0x40000000) {
                        if (maped_queue[i].it_temp == 0) {
                            maped_queue[i].it_temp = it_val;
                            return;
                        } else if (maped_queue[i].it_temp == it_val) {
                            maped_queue[i].it_temp = 0;
                            return;
                        }
                        continue;
                    } else {
                        maped_queue[i].it_temp = it_val;
                        return;
                    }
                }

                // If removing item, change existing placement instruction instead (same position)
                if (type == MAPED_RMVITEM && maped_queue[i].op_type == MAPED_PLACEITEM) {
                    maped_queue[i].it_temp = 0;
                    return;
                }
            }
            continue;
        }

        maped_queue[i].op_type = type;
        maped_queue[i].x = x;
        maped_queue[i].y = y;
        maped_queue[i].it_temp = it_val;
        maped_queue[i].used = USE_ACTIVE;
        break;
    }
}

int main(int argc, char *args[])
{
    LIST *head;
    head = is_form_empty() ? NULL : cgi_input_parse();

    load();

    // Head
    printf("Content-Type: text/html\n\n\n");
    printf("<html><head>\n");
    printf("<meta content=\"text/html;charset=utf-8\" http-equiv=\"Content-Type\">\n");
    printf("<title>v2 Map Editor</title>\n");
    printf("<link rel=\"stylesheet\" href=\"/assets/scripts/mapper.css\"/>\n");
    printf("<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/@simonwep/pickr/dist/themes/nano.min.css\"/>\n");
    printf("<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\"/>\n");
    printf("<script src=\"https://cdn.jsdelivr.net/npm/@simonwep/pickr/dist/pickr.min.js\"></script>\n");
    printf("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script>\n");
    printf("</head>\n");

    // Body
    printf("<body ondragstart=\"return false;\" ondrop=\"return false;\">\n");
    printf("<a id=\"downloadAnchorElem\" style=\"display:none\"></a>\n");
    printf("<div id=\"div-flexmain\">\n");
    printf("<div id=\"div-palette\">\n");
    printf("<div style=\"display: flex; flex-direction: column; height: 100%%;\">\n");
    printf("<div id=\"div-mapload\">\n");
    printf("<button id=\"button-mapload\" onclick=\"loadMapArea()\">Load Area</button>\n");
    printf("X1:<input id=\"inp-map-x1\" style=\"width:40px; margin: 2px;\" type=\"number\" maxlength=\"4\">\n");
    printf("Y1:<input id=\"inp-map-y1\" style=\"width:40px; margin: 2px;\" type=\"number\" maxlength=\"4\"><br>\n");
    printf("X2:<input id=\"inp-map-x2\" style=\"width:40px; margin: 2px;\" type=\"number\" maxlength=\"4\">\n");
    printf("Y2:<input id=\"inp-map-y2\" style=\"width:40px; margin: 2px;\" type=\"number\" maxlength=\"4\">\n");
    printf("</div>\n");
    printf("<div class=\"div-toolbox\">\n");
    printf("<button id=\"but-hidew\" class=\"toolbox-button\" style=\"background-image: url('/assets/ui/hidew.png')\" onclick=\"toggleWalls(); updateUI();\" title=\"Hide walls\"></button>\n");
    printf("<button id=\"but-tempfilter\" class=\"toolbox-button\" style=\"background-image: url('/assets/ui/filter.png');\" onclick=\"toggleDiv('div-filter'); updateUI();\" title=\"Filter item templates\"></button>\n");
    printf("<button id=\"but-grid\" class=\"toolbox-button\" style=\"background-image: url('/assets/ui/togglegrid.png');\" onclick=\"toggleMapGrid(); updateUI();\" title=\"Toggle map grid\"></button>\n");
    printf("<button id=\"but-cpicker\" class=\"toolbox-button\" style=\"background-image: url('/assets/ui/cpicker.png');\" onclick=\"toggleDiv('div-temp-picker'); updateUI();\" title=\"Change template type and color\"></button>\n");
    printf("<button id=\"but-displaymode\" class=\"toolbox-button\" style=\"background-image: url('/assets/ui/toggle_divs.png');\" onclick=\"toggleViewDivs(); updateUI();\" title=\"Toggle between grid-only and preview-only\"></button>\n");
    printf("<button id=\"but-toggleflags\" class=\"toolbox-button\" style=\"background-image: url('/assets/ui/toggle_flags.png');\" onclick=\"toggleTileFlags(); updateUI();\" title=\"Toggle visibility for tile flags\"></button>\n");
    printf("<span id=\"span-seltemp\" style=\"color:wheat;font-size:10pt;margin-left:2px;\">Selected: </span>\n");
    printf("</div>\n");
    printf("<div id=\"div-temp-picker\">\n");
    printf("<div style=\"display: flex;width: 100%%;justify-content: space-evenly;align-items:center;\">\n");
    printf("<input id=\"inp-apply-colpicker\" type=\"checkbox\">\n");
    printf("<button id=\"button-temp-pickr\" class=\"toolbox-button rb-border color-picker\"></button>\n");
    printf("<div style=\"width: 100%%;display: flex;flex-direction: column;text-align: center;flex: 1;\">\n");
    printf("Set template type to:\n");
    printf("<div style=\"width: 100%%;\">\n");
    printf("<input id=\"rad-settype-na\" type=\"radio\" name=\"rad-temp-type\" checked>N/A\n");
    printf("<input id=\"rad-settype-wall\" type=\"radio\" name=\"rad-temp-type\">Wall\n");
    printf("<input id=\"rad-settype-item\" type=\"radio\" name=\"rad-temp-type\">Item\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("<div style=\"margin-top: 8px;\">\n");
    printf("<button style=\"padding: 0;margin: 4px;\" onclick=\"saveTempConfig()\">Save config</button>\n");
    printf("<button style=\"padding: 0;margin: 4px;\" onclick=\"document.getElementById('inp-configfile').click()\">Load config</button>\n");
    printf("<input id=\"inp-configfile\" type=\"file\" accept=\".json\" style=\"display: none\">\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("<div id=\"div-filter\">\n");
    printf("<div>\n");
    printf("Type:\n");
    printf("<input id=\"rad-type-na\" type=\"radio\" name=\"rad-item-types\" onclick=\"applyTempFilter()\" checked>N/A\n");
    printf("<input id=\"rad-type-wall\" type=\"radio\" name=\"rad-item-types\" onclick=\"applyTempFilter()\">Wall\n");
    printf("<input id=\"rad-type-floor\" type=\"radio\" name=\"rad-item-types\" onclick=\"applyTempFilter()\">Floor\n");
    printf("<input id=\"rad-type-item\" type=\"radio\" name=\"rad-item-types\" onclick=\"applyTempFilter()\">Item\n");
    printf("</div>\n");
    printf("<div style=\"margin-top:4px\">\n");
    printf("Item ID:\n");
    printf("<input id=\"inp-minitem\" type=\"number\" style=\"width:32px\" onchange=\"applyTempFilter()\">~\n");
    printf("<input id=\"inp-maxitem\" type=\"number\" style=\"width:32px\" onchange=\"applyTempFilter()\">\n");
    printf("</div>\n");
    printf("<div style=\"margin-top:4px\">\n");
    printf("Item name:\n");
    printf("<input id=\"inp-itemname\" type=\"text\" onchange=\"applyTempFilter()\">\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("<div id=\"div-palette-container\"></div>\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("<div id=\"div-canvastable\">\n");
    printf("<div id=\"div-grid-sup\">\n");
    printf("<div id=\"div-grid-toolbox\" class=\"div-toolbox\" style=\"margin: 4px 0px; height: 100%%; flex-direction: column;\">\n");
    printf("<button id=\"but-brush\" class=\"toolbox-button toolbox-grid-button\" onclick=\"setPaintMode('brush');\" title=\"Brush paint mode\"><i class=\"fa fa-paint-brush\"></i></button>\n");
    printf("<button id=\"but-rect-mode\" class=\"toolbox-button toolbox-grid-button\" onclick=\"setPaintMode('rect');\" title=\"Rectangle paint mode\"><i class=\"fa fa-square-o\"></i></button>\n");
    printf("<button id=\"but-rectfill-mode\" class=\"toolbox-button toolbox-grid-button\" onclick=\"setPaintMode('rectfill');\" title=\"Filled rectangle paint mode\"><i class=\"fa fa-square\"></i></button>\n");
    printf("<p class=\"unselectable\" style=\"margin-top: auto; font-size: 8pt; color: wheat; text-align: center;\">\n");
    printf("T.Size:<br>\n");
    printf("<input id=\"inp-tilesize\" type=\"number\" style=\"width: 16px; box-sizing: content-box;\" maxlength=\"2\" onchange=\"changeTileSize(this.value);\">\n");
    printf("</p>\n");
    printf("</div>\n");
    printf("<div id=\"div-grid\" class=\"cv-base\" style=\"position: relative; height: 100%%;\">\n");
    printf("<canvas id=\"cv-grid\" class=\"cv-grid\" style=\"z-index: 1\" oncontextmenu=\"return false\"></canvas>\n");
    printf("<canvas id=\"cv-grid-selection\" class=\"cv-grid\" style=\"z-index: 2; pointer-events: none;\"></canvas>\n");
    printf("<div id=\"div-grid-tooltip\"></div>\n");
    printf("</div>\n");
    printf("</div>\n");
    printf("<canvas id=\"cv-preview\" class=\"cv-base\" oncontextmenu=\"if (shiftDown) return false;\"></canvas>\n");
    printf("</div>\n");
    printf("</div>\n");

    // Load scripts
    printf("<script src=\"/assets/scripts/cpicker.js\"></script>\n");
    printf("<script src=\"/assets/scripts/undo_redo.js\"></script>\n");
    printf("<script src=\"/assets/scripts/canvas_funcs.js\"></script>\n");
    printf("<script src=\"/assets/scripts/mapper.js\"></script>\n");

    // Templates and floors
    scriptLoadTemplates();
    printf("<script src=\"/assets/scripts/load_floors.js\"></script>\n");
    printf("<script>loadTemplates();</script>\n");

    // UI update
    printf("<script>updateUI();</script>\n");

    int step = 0;
    if (head) {
        char *tmp = find_val(head,"step");
        if (tmp) step = atoi(tmp);
    }

    int x1, y1, x2, y2;
    int x = 0, y = 0;
    char *it_type;
    switch(step) {
        case 1: // Load map area, receives coordinates as "x1", "y1", "x2", "y2"
            x1 = atoi(find_val(head,"x1"));
            y1 = atoi(find_val(head,"y1"));
            x2 = atoi(find_val(head,"x2"))+1;
            y2 = atoi(find_val(head,"y2"))+1;

            if (x1 < 0 || x1 > 1024 || x1 > x2 || y1 < 0 || y1 > 1024 || y1 > y2 ||
                x2 < 0 || x2 > 1024 || y2 < 0 || y2 > 1024) {
                    printf("<script>alert(\"Received invalid input for map area.\");</script>\n");
                    break;
            }
            printf("<script>loadMapCells(%d, %d, %d, %d);\n", x1, y1, x2, y2);

            for (int i=y1; i<y2; i++) {
                for (int j=x1; j<x2; j++) {
                    int map_tileid = i + j * MAPX;
                    if (map_tileid < 0 || map_tileid > MAPX * MAPY) continue;

                    printf("if (tilemap.hasOwnProperty(\"maptile\" + (%d + %d * tilemap_width))) {\n", x, y);
                    printf("var tile = tilemap[\"maptile\" + (%d + %d * tilemap_width)];\n", x, y);

                    // Set floor sprite
                    if (map[map_tileid].sprite) {
                        int fl_id = map[map_tileid].sprite;
                        printf("tile.floor = %d;\n", fl_id);
                    }

                    // Set item
                    int it_id = map[map_tileid].it;
                    if (it_id > 1 && it[it_id].used != USE_EMPTY) {
                        printf("tile.item = %d;\n", it[it_id].temp);
                    } else {
                        // Find item template with the same sprite (since it's not stored in map.it, it shouldn't be an usable/take-able item)
                        for (int in=2; in<MAXTITEM; in++) {
                            if (it_temp[in].used == USE_EMPTY) continue;
                            if (it_temp[in].flags&(IF_TAKE|IF_LOOK|IF_LOOKSPECIAL|IF_USE|IF_USESPECIAL)) continue;

                            if (it_temp[in].sprite[0] == map[map_tileid].fsprite) {
                                printf("tile.item = %d;\n", in);
                                break;
                            }
                        }
                    }

                    // Set flags
                    if (map[map_tileid].flags&(MF_MOVEBLOCK)) printf("tile.flags.moveblock = true;\n");
                    if (map[map_tileid].flags&(MF_SIGHTBLOCK)) printf("tile.flags.sightblock = true;\n");
                    if (map[map_tileid].flags&(MF_INDOORS)) printf("tile.flags.indoors = true;\n");
                    if (map[map_tileid].flags&(MF_UWATER)) printf("tile.flags.underwater = true;\n");
                    if (map[map_tileid].flags&(MF_NOLAG)) printf("tile.flags.nolag = true;\n");
                    if (map[map_tileid].flags&(MF_NOMONST)) printf("tile.flags.nomonster = true;\n");
                    if (map[map_tileid].flags&(MF_BANK)) printf("tile.flags.bank = true;\n");
                    if (map[map_tileid].flags&(MF_TAVERN)) printf("tile.flags.tavern = true;\n");
                    if (map[map_tileid].flags&(MF_NOMAGIC)) printf("tile.flags.nomagic = true;\n");
                    if (map[map_tileid].flags&(MF_DEATHTRAP)) printf("tile.flags.deathtrap = true;\n");
                    if (map[map_tileid].flags&(MF_ARENA)) printf("tile.flags.arena = true;\n");
                    if (map[map_tileid].flags&(MF_NOEXPIRE)) printf("tile.flags.noexpire = true;\n");
                    if (map[map_tileid].flags&(MF_NOFIGHT)) printf("tile.flags.nofight = true;\n");

                    printf("}\n");

                    x++;
                }
                x = 0;
                y++;
            }

            // Load pending tile changes from queue (usually when server is offline)
            for (int i=0; i<MAX_MAPED_QUEUE; i++) {
                if (maped_queue[i].used == USE_EMPTY) continue;

                if (maped_queue[i].x < x1 || maped_queue[i].x > x2) continue;
                if (maped_queue[i].y < y1 || maped_queue[i].y > y2) continue;

                if (maped_queue[i].op_type == MAPED_PLACEITEM || maped_queue[i].op_type == MAPED_SETFLOOR) {
                    printf("var it_temp = ");
                    if (maped_queue[i].it_temp&0x40000000) {
                        if (maped_queue[i].it_temp&MF_MOVEBLOCK) printf("\"flag_moveblock\";\n");
                        else if (maped_queue[i].it_temp&MF_SIGHTBLOCK) printf("\"flag_sightblock\";\n");
                        else if (maped_queue[i].it_temp&MF_INDOORS) printf("\"flag_indoors\";\n");
                        else if (maped_queue[i].it_temp&MF_UWATER) printf("\"flag_underwater\";\n");
                        else if (maped_queue[i].it_temp&MF_NOLAG) printf("\"flag_nolag\";\n");
                        else if (maped_queue[i].it_temp&MF_NOMONST) printf("\"flag_nomonster\";\n");
                        else if (maped_queue[i].it_temp&MF_BANK) printf("\"flag_bank\";\n");
                        else if (maped_queue[i].it_temp&MF_TAVERN) printf("\"flag_tavern\";\n");
                        else if (maped_queue[i].it_temp&MF_NOMAGIC) printf("\"flag_nomagic\";\n");
                        else if (maped_queue[i].it_temp&MF_DEATHTRAP) printf("\"flag_deathtrap\";\n");
                        else if (maped_queue[i].it_temp&MF_ARENA) printf("\"flag_arena\";\n");
                        else if (maped_queue[i].it_temp&MF_NOEXPIRE) printf("\"flag_noexpire\";\n");
                        else if (maped_queue[i].it_temp&MF_NOFIGHT) printf("\"flag_nofight\";\n");
                        else printf("null;\n");

                    } else if (maped_queue[i].op_type == MAPED_SETFLOOR) {
                        printf("\"it_temp\" + %d;\n", 100000 + maped_queue[i].it_temp);
                    } else {
                        printf("\"it_temp\" + %d;\n", maped_queue[i].it_temp);
                    }

                    printf("if (it_temp && item_templates.hasOwnProperty(it_temp)) {\n");
                    printf("var tile_id = \"maptile\" + (%d + %d * tilemap_width);\n", maped_queue[i].y, maped_queue[i].x);
                    printf("if (tilemap.hasOwnProperty(tile_id)) {\n");
                    printf("placeItem(item_templates[it_temp], tile_id, false); } }\n");
                }
            }

            printf("renderPreview(); renderGrid();</script>\n");
        break;

        case 2: // Process tile instruction
            x = atoi(find_val(head, "x"));
            y = atoi(find_val(head, "y"));
            x1 = atoi(find_val(head, "it_val"));
            it_type = find_val(head, "it_type");

            if (x<0 || x>MAPX || y<0 || y>MAPY) break;

            if (strcmp(it_type, "floor") == 0) {
                addQueueInstruction(MAPED_SETFLOOR, x, y, x1);

            } else if (strcmp(it_type, "wall") == 0 || strcmp(it_type, "item") == 0) {
                if (map[x + y * MAPX].it) {
                    addQueueInstruction(MAPED_RMVITEM, x, y, 0);
                }
                addQueueInstruction(MAPED_PLACEITEM, x, y, x1);

            } else if (strcmp(it_type, "remove") == 0) {
                addQueueInstruction(MAPED_RMVITEM, x, y, 0);

            } else if (strcmp(it_type, "flag") == 0) {
                x2 = 0x40000000;
                switch(x1) {
                    case 1: x2|=MF_MOVEBLOCK; break;
                    case 2: x2|=MF_SIGHTBLOCK; break;
                    case 3: x2|=MF_INDOORS; break;
                    case 4: x2|=MF_UWATER; break;
                    case 5: x2|=MF_NOLAG; break;
                    case 6: x2|=MF_NOMONST; break;
                    case 7: x2|=MF_BANK; break;
                    case 8: x2|=MF_TAVERN; break;
                    case 9: x2|=MF_NOMAGIC; break;
                    case 10: x2|=MF_DEATHTRAP; break;
                    case 11: x2|=MF_ARENA; break;
                    case 12: x2|=MF_NOEXPIRE; break;
                    case 13: x2|=MF_NOFIGHT; break;
                }
                addQueueInstruction(MAPED_PLACEITEM, x, y, x2);
            }
        break;
    }
    
    unload();

    printf("</body>\n");
    printf("</html>\n");
    return 0;
}