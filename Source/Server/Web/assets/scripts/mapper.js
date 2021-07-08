function padSpriteNum(num) { return ("00000" + num).substr(-5); }

class MapTile {
    constructor(id, x, y, floor, item) {
        this.id = id;
        this.x = x;
        this.y = y;
        this.map_x = 0;
        this.map_y = 0;
        this.floor = floor; // Sprite # of floor - add 100000 to get template ID
        this.item = item;   // ID of template
        
        this.flags = {
            moveblock: false,
            sightblock: false,
            indoors: false,
            underwater: false,
            nolag: false,
            nomonster: false,
            bank: false,
            tavern: false,
            nomagic: false,
            deathtrap: false,
            arena: false,
            noexpire: false,
            nofight: false
        }
    }

    getItemTemp() {
        var it_id = "it_temp" + this.item;
        if (!item_templates.hasOwnProperty(it_id)) return null;
        return item_templates[it_id];
    }

    getFloorSprite() {
        var fl_id = "it_temp" + (this.floor + 100000);
        if (item_templates.hasOwnProperty(fl_id)) return "/assets/sprites/" + padSpriteNum(item_templates[fl_id].item_spr) + ".png";
    }
    getItemSprite() {
        var it_temp = this.getItemTemp();
        if (!it_temp) return null;

        var tmp_item = it_temp.item_spr;
        // Hiding walls toggle
        if (hideWalls) {
            if (it_temp.type == "wall") tmp_item++;
        }
        return "/assets/sprites/" + padSpriteNum(tmp_item) + ".png";
    }
}

class ItemTemp {
    constructor(temp_id, item_spr, type, color, name) {
        this.temp_id = temp_id;
        this.item_spr = item_spr;
        this.type = type;   // 3 types: "floor", "wall" and "item" (candles, portals, etc.)
        this.color = color;
        this.name = name;
        this.dom_elem = null;

        this.flags = {
            moveblock: false,
            sightblock: false
        }
    }

    updateElem() {
        if (!this.dom_elem) return;
        var it_temp_prev = document.getElementById(this.dom_elem.id + "_prev");

        it_temp_prev.style.backgroundColor = this.color;

        var span_txt = "F";
        if (this.type == "wall") span_txt = "W";
        if (this.type == "flag") span_txt = "flag";
        if (this.type == "item") {
            span_txt = "It";
            it_temp_prev.style.borderRadius = "100%";
            it_temp_prev.style.border = "2px orange solid";
        } else {
            it_temp_prev.style.borderRadius = null;
            it_temp_prev.style.border = null;
        }
        document.getElementById(this.dom_elem.id + "_typespan").innerHTML = span_txt;
    }
}

const paletteDiv = document.getElementById("div-palette-container");

var tilemap = {};
var tilemap_width = 0;
var tilemap_height = 0;
var item_templates = {};

// Mouse clicking
var mouseDown = 0;
document.body.onmousedown = function (eventData) {
    if (eventData.button == 0) {
        mouseDown = 1;
    } else if (eventData.button == 2) {
        mouseDown = 2;
    }
}
document.body.onmouseup = function () {
    mouseDown = 0;
}
document.body.onmouseleave = function () {
    mouseDown = 0;
}

// Shift/Ctrl press
var shiftDown = 0;
var ctrlDown = 0;
window.addEventListener("keydown", function (event) {
    switch (event.keyCode) {
        case 16: shiftDown = 1; break;  // Shift
        case 17: ctrlDown = 1; break;   // Ctrl

        case 27:  // Escape
            // Cancel paint mode placement
            paintData.active = false;
            renderGrid();
        break;

        case 89: if (ctrlDown) { event.preventDefault(); redoLastAction(); } break;  // Y (for redo)
        case 90: if (ctrlDown) { event.preventDefault(); undoLastAction(); } break;  // Z (for undo)
    }
});
window.addEventListener("keyup", function (event) {
    switch (event.keyCode) {
        case 16: shiftDown = 0; break;
        case 17: ctrlDown = 0; break;
    }
});

function loadMapArea() {
    var x1 = document.getElementById("inp-map-x1").value;
    var y1 = document.getElementById("inp-map-y1").value;
    var x2 = document.getElementById("inp-map-x2").value;
    var y2 = document.getElementById("inp-map-y2").value;

    if (!x1.match(/^[0-9]+$/)) { alert("X1 contains non-number characters."); return; }
    if (!y1.match(/^[0-9]+$/)) { alert("Y1 contains non-number characters."); return; }
    if (!x2.match(/^[0-9]+$/)) { alert("X2 contains non-number characters."); return; }
    if (!y2.match(/^[0-9]+$/)) { alert("Y2 contains non-number characters."); return; }

    x1 = parseInt(x1);
    x2 = parseInt(x2);
    y1 = parseInt(y1);
    y2 = parseInt(y2);

    if (x1 < 0 || x1 > 1024 || x1 >= x2) { alert("X1 must be between 0 and 1024, and must be lower than X2."); return; }
    if (y1 < 0 || y1 > 1024 || y1 >= y2) { alert("Y1 must be between 0 and 1024, and must be lower than Y2."); return; }
    if (x2 < 0 || x2 > 1024) { alert("X2 must be between 0 and 1024."); return; }
    if (y2 < 0 || y2 > 1024) { alert("Y2 must be between 0 and 1024."); return; }

    // Load x1,y1 -> x2,y2 from server map into the tilemap, then redirect...
    window.location.replace("/cgi-imp/mapper.cgi?step=1&x1=" + x1 + "&y1=" + y1 + "&x2=" + x2 + "&y2=" + y2);
}

function loadMapCells(x1, y1, x2, y2) {
    tilemap_width = x2 - x1;
    tilemap_height = y2 - y1;
    tilemap = {};

    var x = 0, y = 0;
    for (var i = y1; i < y2; i++) {
        for (var j = x1; j < x2; j++) {
            var id = x + y * tilemap_width;
            var tile_id = "maptile" + id;
            tilemap[tile_id] = new MapTile(id, x, y, 0, 0);
            tilemap[tile_id].map_x = x1 + y;
            tilemap[tile_id].map_y = y1 + x;

            x++;
        }
        x = 0;
        y++;
    }
    renderGrid();
}

function loadTemplates() {
    paletteDiv.innerHTML = "";
    for (var temp in item_templates) {
        var item = item_templates[temp];

        if (item.type != "flag") {
            var sprite_url = "/assets/sprites/" + padSpriteNum(item.item_spr) + ".png";

            // Load image into preview renderer
            prevCanvas.loadImage(sprite_url);
            if (item.type == "wall") {
                // Load hidden wall image
                var hidden_spr = item.item_spr + 1;
                prevCanvas.loadImage("/assets/sprites/" + padSpriteNum(hidden_spr) + ".png");
            }
        }

        var temp_cell = document.createElement("div");
        temp_cell.id = temp;
        temp_cell.className = "temp-cell";

        var tmp_flagspan = null;
        if (item.type != "flag") {
            temp_cell.style.backgroundImage = "url('" + sprite_url + "')";
        } else {
            tmp_flagspan = document.createElement('span');
            tmp_flagspan.id = temp + "_flagspan";
            tmp_flagspan.className = "temp-flagspan unselectable";
            tmp_flagspan.innerHTML = temp.split("_")[1];
        }
        temp_cell.onmousedown = function (obj) { itemTempClick(obj.srcElement.id); }
        if (item.type != "floor" && item.type != "flag") {
            temp_cell.ondblclick = function (obj) { itemTempDblClick(obj); }
        }
        if (item.name) temp_cell.title = item.name;

        var tmp_typespan = document.createElement("span");
        tmp_typespan.id = temp + "_typespan";
        tmp_typespan.className = "temp-typespan unselectable";
        switch (item.type) {
            case "floor": tmp_typespan.innerHTML = "F"; break;
            case "wall": tmp_typespan.innerHTML = "W"; break;
            case "item": tmp_typespan.innerHTML = "It"; break;
            case "flag": tmp_typespan.innerHTML = "flag"; break;
        }
        temp_cell.appendChild(tmp_typespan);

        var tmp_colorprev = document.createElement("div");
        tmp_colorprev.id = temp + "_prev";
        tmp_colorprev.className = "temp-cell-color";
        tmp_colorprev.style.backgroundColor = item.color;
        if (item.type == "item") {
            tmp_colorprev.style.borderRadius = "100%";
            tmp_colorprev.style.border = "2px orange solid";
        }
        temp_cell.appendChild(tmp_colorprev);

        if (tmp_flagspan) temp_cell.appendChild(tmp_flagspan);

        paletteDiv.appendChild(temp_cell);
        item.dom_elem = temp_cell;
    }
}

function itemTempDblClick(obj) {
    if (!item_templates.hasOwnProperty(obj.srcElement.id)) return;

    var it_temp = item_templates[obj.srcElement.id];
    if (it_temp.type == "floor" || it_temp.type == "flag") return;

    window.open("/cgi-imp/acct.cgi?step=23&in=" + it_temp.temp_id);
}

var selected_item = null;
var selected_item_elem = null;
function itemTempClick(temp_id) {
    if (!item_templates.hasOwnProperty(temp_id)) return;

    var it_temp = item_templates[temp_id];
    if (shiftDown) {
        if (document.getElementById("inp-apply-colpicker").checked) {
            it_temp.color = pickr_button.style.backgroundColor;
        }

        if (it_temp.type != "floor" && it_temp.type != "flag") {
            if (document.getElementById("rad-settype-wall").checked) it_temp.type = "wall";
            else if (document.getElementById("rad-settype-item").checked) it_temp.type = "item";
        }
        it_temp.updateElem();
        renderGrid();
        return;
    }

    if (selected_item_elem != null) {
        selected_item_elem.style.border = null;
    }
    selected_item = item_templates[temp_id];
    selected_item_elem = document.getElementById(temp_id);
    if (selected_item_elem) {
        selected_item_elem.style.border = "1px blue solid";
    }

    var tmp_spantxt = "Selected: " + selected_item.temp_id;
    if (selected_item.type == "flag") tmp_spantxt = "Selected: " + temp_id;
    document.getElementById("span-seltemp").innerHTML = tmp_spantxt;
    
    renderGrid();
}

function applyTempFilter() {
    var type = "";
    var min = 0;
    var max = 0;

    // Get type filter
    if (document.getElementById("rad-type-wall").checked) type = "wall";
    else if (document.getElementById("rad-type-floor").checked) type = "floor";
    else if (document.getElementById("rad-type-item").checked) type = "item";
    
    // Get min/max temp id filter
    var tmp_val = document.getElementById("inp-minitem").value;
    if (tmp_val.match(/^[0-9]+$/)) min = tmp_val;

    tmp_val = document.getElementById("inp-maxitem").value;
    if (tmp_val.match(/^[0-9]+$/)) max = tmp_val;

    // Get name filter
    var name = RegExp(document.getElementById("inp-itemname").value, "i");

    var temp_elems = document.getElementsByClassName("temp-cell");
    for (var temp of temp_elems) {
        if (!item_templates.hasOwnProperty(temp.id)) continue;

        var hide = 0;
        var it_temp = item_templates[temp.id];
        if (type != "" && it_temp.type != type) hide = 1;
        if (max > 0 && it_temp.temp_id > max) hide = 1;
        if (min > 0 && it_temp.temp_id < min) hide = 1;
        if (!name.test(it_temp.name)) hide = 1;

        if (hide) temp.style.display = "none";
        else temp.style.display = "";
    }
}

var sendChanges = true;

function sendOperation(it_type, tile_id, it_val) {
    if (!sendChanges) return;
    if (!tilemap.hasOwnProperty(tile_id)) return;

    $.ajax({
        url: "/cgi-imp/mapper.cgi",
        type: "POST",
        data: { step: 2, x: tilemap[tile_id].map_x, y: tilemap[tile_id].map_y, it_type: it_type, it_val: it_val },
        dataType: "html"
    });
}

function placeItem(it_temp, tile_id, send_op) {
    if (!tilemap.hasOwnProperty(tile_id)) return 0;

    var it_temp_val = 0;
    if (it_temp.type == "floor") {
        if (tilemap[tile_id].floor == it_temp.item_spr) return 0;

        tilemap[tile_id].floor = it_temp.item_spr;
        it_temp_val = selected_item.item_spr;

    } else if (it_temp.type == "flag") {
        if (!drawFlags) toggleTileFlags();

        for (var flag in it_temp.flags) {
            if (it_temp.flags[flag] == true) {
                tilemap[tile_id].flags[flag] = !tilemap[tile_id].flags[flag];

                if (flag == "moveblock") it_temp_val = 1;
                else if (flag == "sightblock") it_temp_val = 2;
                else if (flag == "indoors") it_temp_val = 3;
                else if (flag == "underwater") it_temp_val = 4;
                else if (flag == "nolag") it_temp_val = 5;
                else if (flag == "nomonster") it_temp_val = 6;
                else if (flag == "bank") it_temp_val = 7;
                else if (flag == "tavern") it_temp_val = 8;
                else if (flag == "nomagic") it_temp_val = 9;
                else if (flag == "deathtrap") it_temp_val = 10;
                else if (flag == "arena") it_temp_val = 11;
                else if (flag == "noexpire") it_temp_val = 12;
                else if (flag == "nofight") it_temp_val = 13;
            }
        }
    } else {
        if (tilemap[tile_id].item == it_temp.temp_id) return 0;

        tilemap[tile_id].item = it_temp.temp_id;
        if (it_temp.flags.moveblock) tilemap[tile_id].flags.moveblock = true;
        if (it_temp.flags.sightblock) tilemap[tile_id].flags.sightblock = true;
        it_temp_val = it_temp.temp_id;
    }

    if (send_op) sendOperation(it_temp.type, tile_id, it_temp_val);
    return 1;
}

function removeItem(tile_id) {
    if (!tilemap.hasOwnProperty(tile_id)) return 0;

    if (!tilemap[tile_id].flags.moveblock && !tilemap[tile_id].flags.sightblock && !tilemap[tile_id].item) return 0;

    tilemap[tile_id].flags.moveblock = false;
    tilemap[tile_id].flags.sightblock = false;
    tilemap[tile_id].item = 0;

    sendOperation("remove", tile_id, 0);
    return 1;
}

function saveTempConfig() {
    var dl_elem = document.getElementById("downloadAnchorElem");
    var dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(item_templates, 0, 4));
    dl_elem.setAttribute("href", "data:" + dataStr);
    dl_elem.setAttribute("download", "template_config.json");
    dl_elem.click();
}

document.getElementById("inp-configfile").onchange = function (e) {
    if (e.srcElement.value.substr(-5) != ".json") {
        alert("Invalid file type (expecting .json)");
        return;
    }
    
    var reader = new FileReader();
    reader.onload = function (ev_onload) {
        var jsonData = JSON.parse(ev_onload.target.result);

        for (var temp in jsonData) {
            var loaded_temp = jsonData[temp];
            if (item_templates.hasOwnProperty(temp)) {
                item_templates[temp].type = loaded_temp.type;
                item_templates[temp].color = loaded_temp.color;
                item_templates[temp].updateElem();
            }
        }
        renderGrid();
    };
    reader.readAsText(e.target.files[0]);
}

function toggleDiv(tgt_div) {
    var tempFilterDiv = document.getElementById(tgt_div);
    if (tempFilterDiv.style.display == "none") {
        tempFilterDiv.style.display = null;
    } else {
        tempFilterDiv.style.display = "none";
    }
}

var viewdivs_mode = 0;
function toggleViewDivs() {
    viewdivs_mode = (viewdivs_mode + 1) % 3;

    var divGrid = document.getElementById("div-grid-sup");
    switch (viewdivs_mode) {
        case 0:
            divGrid.style.display = null;
            prevCanvas.cv.style.display = null;
            break;
        case 1:
            prevCanvas.cv.style.display = "none";
            divGrid.style.display = null;
            break;
        case 2:
            divGrid.style.display = "none";
            prevCanvas.cv.style.display = null;
            break;
    }
    renderGrid();
    renderPreview();
}

var grid_enabled = true;
function toggleMapGrid() {
    grid_enabled = !grid_enabled;
    renderGrid();
}

function updateUI() {
    // Hide walls button
    var tmp_ui = document.getElementById('but-hidew');
    if (hideWalls) tmp_ui.style.border = "1px blue solid";
    else tmp_ui.style.border = null;

    // Template filter button
    tmp_ui = document.getElementById('but-tempfilter');
    if (document.getElementById('div-filter').style.display == "none") {
        tmp_ui.style.backgroundImage = "url('/assets/ui/filter.png')";
    } else {
        tmp_ui.style.backgroundImage = "url('/assets/ui/filter_off.png')";
    }

    // Grid toggle button
    tmp_ui = document.getElementById('but-grid');
    if (grid_enabled) tmp_ui.style.backgroundImage = "url('/assets/ui/togglegrid.png";
    else tmp_ui.style.backgroundImage = "url('/assets/ui/togglegrid_off.png";

    // Color/type picker button
    tmp_ui = document.getElementById('but-cpicker');
    if (document.getElementById('div-temp-picker').style.display == "none") {
        tmp_ui.style.backgroundImage = "url('/assets/ui/cpicker.png')";
    } else {
        tmp_ui.style.backgroundImage = "url('/assets/ui/cpicker_off.png')";
    }

    // Grid/preview display mode button
    tmp_ui = document.getElementById('but-displaymode');
    if (viewdivs_mode == 0) tmp_ui.style.backgroundImage = "url('/assets/ui/toggle_divs.png')";
    else tmp_ui.style.backgroundImage = `url('/assets/ui/toggle_divs_${viewdivs_mode}.png')`;

    // Toggle flag visibility button
    tmp_ui = document.getElementById('but-toggleflags');
    if (!drawFlags) tmp_ui.style.backgroundImage = "url('/assets/ui/toggle_flags.png')";
    else tmp_ui.style.backgroundImage = "url('/assets/ui/toggle_flags_off.png')";

    // Grid - brush paint mode button
    tmp_ui = document.getElementById('but-brush');
    if (paintMode == "brush") tmp_ui.style.border = "1px blue solid";
    else tmp_ui.style.border = null;

    // Grid - rectangle paint mode button
    tmp_ui = document.getElementById('but-rect-mode');
    if (paintMode == "rect") tmp_ui.style.border = "1px blue solid";
    else tmp_ui.style.border = null;

    // Grid - filled rectangle paint mode button
    tmp_ui = document.getElementById('but-rectfill-mode');
    if (paintMode == "rectfill") tmp_ui.style.border = "1px blue solid";
    else tmp_ui.style.border = null;

    // Grid - tile size input
    tmp_ui = document.getElementById('inp-tilesize');
    tmp_ui.value = gridTileSize;
}

toggleDiv("div-filter");
toggleDiv("div-temp-picker");

// Flag item templates
item_templates["flag_moveblock"] = new ItemTemp(0, 0, "flag", "#278177");
item_templates["flag_moveblock"].flags.moveblock = true;

item_templates["flag_sightblock"] = new ItemTemp(0, 0, "flag", "pink");
item_templates["flag_sightblock"].flags.sightblock = true;

item_templates["flag_indoors"] = new ItemTemp(0, 0, "flag", "#9601ff");
item_templates["flag_indoors"].flags.indoors = true;

item_templates["flag_underwater"] = new ItemTemp(0, 0, "flag", "azure");
item_templates["flag_underwater"].flags.underwater = true;

item_templates["flag_nolag"] = new ItemTemp(0, 0, "flag", "orange");
item_templates["flag_nolag"].flags.nolag = true;

item_templates["flag_nomonster"] = new ItemTemp(0, 0, "flag", "blue");
item_templates["flag_nomonster"].flags.nomonster = true;

item_templates["flag_bank"] = new ItemTemp(0, 0, "flag", "red");
item_templates["flag_bank"].flags.bank = true;

item_templates["flag_tavern"] = new ItemTemp(0, 0, "flag", "green");
item_templates["flag_tavern"].flags.tavern = true;

item_templates["flag_nomagic"] = new ItemTemp(0, 0, "flag", "yellow");
item_templates["flag_nomagic"].flags.nomagic = true;

item_templates["flag_deathtrap"] = new ItemTemp(0, 0, "flag", "#cc43ff");
item_templates["flag_deathtrap"].flags.deathtrap = true;

item_templates["flag_arena"] = new ItemTemp(0, 0, "flag", "#e2ff41");
item_templates["flag_arena"].flags.arena = true;

item_templates["flag_noexpire"] = new ItemTemp(0, 0, "flag", "#8b00ff");
item_templates["flag_noexpire"].flags.noexpire = true;

item_templates["flag_nofight"] = new ItemTemp(0, 0, "flag", "#00aaff");
item_templates["flag_nofight"].flags.nofight = true;