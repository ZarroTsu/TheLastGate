class CanvasHandler {
    constructor(cv) {
        this.cv = cv;
        this.cv.width = cv.offsetWidth;
        this.cv.height = cv.offsetHeight;
        this.ctx = this.cv.getContext('2d');
        this.lineCorrection = false;

        this.drawXOffset = 0;
        this.defaultXoff = 0;
        this.drawYOffset = 0;
        this.defaultYoff = 0;

        /** Keys are the path to the image, values are the images themselves.        
         *  e.g. loadedImages["/assets/tiles/floors/00950.png"] = Image() */
        this.loadedImages = {};
        this.redrawTimeout = 100;
    }

    ctxSetLineCorrection(c) {
        if (!this.lineCorrection && c) this.ctx.translate(0.5, 0.5);
        this.lineCorrection = c;
    }

    resetOffset() {
        this.drawXOffset = this.defaultXoff;
        this.drawYOffset = this.defaultYoff;
    }

    setDefaultOffset(xoff, yoff, reset) {
        this.defaultXoff = xoff;
        this.defaultYoff = yoff;
        if (reset) this.resetOffset();
    }

    /** Load an image for future drawing */
    loadImage(path) {
        if (this.loadedImages.hasOwnProperty(path)) {
            return;
        }
        var newImg = new Image();
        newImg.setAttribute('loaded', '0');
        newImg.onload = () => {
            newImg.setAttribute('loaded', '1');
        };
        newImg.src = path;
        this.loadedImages[path] = newImg;
    }

    /** Load a list of images for future drawing */
    loadImages(pathList) {
        for (let imgP of pathList) {
            this.loadImage(imgP);
        }
    }

    getImage(path) {
        if (!this.loadedImages.hasOwnProperty(path)) {
            return null;
        }
        return this.loadedImages[path];
    }

    clearLoadedImages() {
        this.loadedImages = {};
    }

    clearContext() {
        if (this.cv.offsetWidth != this.cv.width || this.cv.offsetHeight != this.cv.height) {
            this.cv.width = this.cv.offsetWidth;
            this.cv.height = this.cv.offsetHeight;
            if (this.lineCorrection) this.ctx.translate(0.5, 0.5);
        }
        this.ctx.clearRect(0, 0, this.cv.width, this.cv.height);
    }

    /** If enqueue is 1, will queue given image for loading before drawing it (if it hasn't been loaded before) */
    drawImage(imgPath, x, y, enqueue) {
        // Image not loaded
        if (!this.loadedImages.hasOwnProperty(imgPath) && enqueue) {
            // Queue for re-drawing
            this.loadImage(imgPath);
            setTimeout(() => {
                this.drawImage(imgPath, x, y, enqueue);
            }, this.redrawTimeout);
            return;
        }
        var img = this.loadedImages[imgPath];
        if (img.getAttribute('loaded') === '0' && enqueue) {
            setTimeout(() => {
                this.drawImage(imgPath, x, y, enqueue);
            }, this.redrawTimeout);
            return;
        }
        this.ctx.drawImage(img, this.drawXOffset + x, this.drawYOffset + y);
    }

    /** X and Y in tiles */
    drawImageIsometric(imgPath, x, y) {
        // Image not loaded
        if (!this.loadedImages.hasOwnProperty(imgPath) || (this.loadedImages.hasOwnProperty(imgPath) && this.loadedImages[imgPath].width == 0)) {
            this.loadImage(imgPath);
            setTimeout(() => {
                this.drawImageIsometric(imgPath, x, y);
            }, this.redrawTimeout);
            return;
        }
        var img = this.loadedImages[imgPath];
        var drawX = (x + y) * 16 - (img.width - 32);
        var drawY = (y - x) * 8 - (img.height - 32);
        this.drawImage(imgPath, drawX, drawY, false);
    }
}

const gridCanvas = new CanvasHandler(document.getElementById('cv-grid'));
gridCanvas.ctxSetLineCorrection(true);
const gridCanvasTooltip = document.getElementById('div-grid-tooltip');

const gridSelCanvas = new CanvasHandler(document.getElementById('cv-grid-selection'));
gridSelCanvas.ctxSetLineCorrection(true);

const prevCanvas = new CanvasHandler(document.getElementById('cv-preview'));
prevCanvas.setDefaultOffset(200, 200, true);

// Click and hold functionalities
gridCanvas.cv.addEventListener("mousedown", (ev) => { cvMouseClick(ev, gridCanvas); }, false);
gridCanvas.cv.addEventListener("mousemove", (ev) => { cvMouseMove(ev, gridCanvas); }, false);
gridCanvas.cv.addEventListener("mouseup", () => { cvGridEndAction(); });
gridCanvas.cv.addEventListener("mouseleave", () => { cvGridEndAction(); });

prevCanvas.cv.addEventListener("mousedown", (ev) => { cvMouseClick(ev, prevCanvas); }, false);
prevCanvas.cv.addEventListener("mousemove", (ev) => { cvMouseMove(ev, prevCanvas); }, false);

var gridTileSize = 32;
function changeTileSize(size) {
    if (size < 1) return;
    if (size > 256) size = 256;
    gridTileSize = size;
    updateUI();
    renderGrid();
}

var draggingPrev = false;
var clickX = 0, clickY = 0;
var cam_startx = 0, cam_starty = 0;

function cvMouseToTilePos(event, cvHandler) {
    var cvRect = cvHandler.cv.getBoundingClientRect()
    var selectionX = Math.floor((event.clientX - cvRect.left - cvHandler.drawXOffset) / gridTileSize);
    var selectionY = Math.floor((event.clientY - cvRect.top - cvHandler.drawYOffset) / gridTileSize);
    return [selectionX, selectionY];
}

var cvGridActionList = [];

var paintMode = "brush";
var paintData = {
    active: false
};
function setPaintMode(mode) {
    paintMode = mode;
    updateUI();

    paintData.active = false;
    renderGrid();
}

setInterval(() => { if (draggingPrev) renderPreview(); }, 100);

var lastTile = "";
function cvMouseMove(event, cvHandler) {
    if (shiftDown) {
        if (mouseDown) {
            cvHandler.drawXOffset = cam_startx - (clickX - event.clientX);
            cvHandler.drawYOffset = cam_starty - (clickY - event.clientY);
            cvHandler.cv.style.cursor = "grabbing";
        } else {
            cvHandler.cv.style.cursor = "grab";
        }
    } else {
        cvHandler.cv.style.cursor = "default";
    }
    switch(cvHandler) {
        case gridCanvas:
            // Selection box
            gridSelCanvas.clearContext();
            var render = false;
            var selPos = cvMouseToTilePos(event, cvHandler);

            if (selPos[0] >= 0 && selPos[0] < tilemap_width && selPos[1] >= 0 && selPos[1] < tilemap_height) {
                var tile_x = cvHandler.drawXOffset + selPos[0] * gridTileSize;
                var tile_y = cvHandler.drawYOffset + selPos[1] * gridTileSize;
                var tile_id = "maptile" + (selPos[0] + selPos[1] * tilemap_width);

                gridSelCanvas.ctx.strokeStyle = "rgb(0, 153, 255)";
                gridSelCanvas.ctx.strokeRect(tile_x, tile_y, gridTileSize, gridTileSize);

                if (paintMode == "brush") {
                    if (mouseDown && !shiftDown) {
                        if (tile_id != lastTile) {
                            lastTile = tile_id;
                            mapCellClick(tile_id, mouseDown, true);
                        }
                    }
                } else if (paintData.active) {
                    if (paintMode == "rect" || paintMode == "rectfill") {
                        paintData["x2"] = selPos[0];
                        paintData["y2"] = selPos[1];
                        render = true;
                    }
                }

                // Update grid tooltip
                var ttip = "";
                if (tilemap.hasOwnProperty(tile_id)) {
                    var m = tilemap[tile_id].map_x + tilemap[tile_id].map_y * 1024;
                    ttip += `X: ${tilemap[tile_id].map_x} / Y: ${tilemap[tile_id].map_y} / M: ${m} `;
                    if (tilemap[tile_id].floor) ttip += `floor: ${tilemap[tile_id].floor} `;
                    if (tilemap[tile_id].item) ttip += `item: ${tilemap[tile_id].item} `;
                }
                gridCanvasTooltip.innerHTML = ttip;
            }

            if ((mouseDown && shiftDown) || render) {
                renderGrid();
            }
        break;

        case prevCanvas:
            if (shiftDown && mouseDown) draggingPrev = true;
            else draggingPrev = false;
        break;
    }
}

function cvMouseClick(event, cv) {
    var clickType = 1;
    if (event.button == 2) clickType = 2;

    if (shiftDown) event.preventDefault();
    // Shift + right click to reset camera
    if (shiftDown && clickType == 2) {
        cv.resetOffset();
    }
    clickX = event.clientX;
    clickY = event.clientY;
    cam_startx = cv.drawXOffset;
    cam_starty = cv.drawYOffset;

    switch(cv) {
        case gridCanvas:
            if (!shiftDown) {
                var selPos = cvMouseToTilePos(event, cv);
                if (selPos[0] >= 0 && selPos[0] < tilemap_width && selPos[1] >= 0 && selPos[1] < tilemap_height) {
                    var tile_id = "maptile" + (selPos[0] + selPos[1] * tilemap_width);
                    lastTile = tile_id;

                    if (paintMode == "brush" || ctrlDown) {
                        mapCellClick(tile_id, clickType, true);
                    } else {
                        if (!paintData.active) {
                            paintData["x1"] = selPos[0];
                            paintData["y1"] = selPos[1];
                            paintData.active = true;
                        } else {
                            // Paint mode second click - apply changes
                            switch(paintMode) {
                                case "rect":
                                case "rectfill":
                                    var x1 = Math.min(paintData.x1, paintData.x2);
                                    var x2 = Math.max(paintData.x1, paintData.x2);
                                    var y1 = Math.min(paintData.y1, paintData.y2);
                                    var y2 = Math.max(paintData.y1, paintData.y2);

                                    for (var i = y1; i <= y2; i++) {
                                        for (var j = x1; j <= x2; j++) {
                                            if (paintMode == "rect" && j != x1 && j != x2 && i != y1 && i != y2) continue;

                                            var tmp_tile_id = "maptile" + (j + i * tilemap_width);
                                            mapCellClick(tmp_tile_id, clickType, false);
                                        }
                                    }
                                    renderPreview();
                                break;
                            }

                            paintData.active = false;
                            renderGrid();
                        }
                    }
                }
            } else {
                renderGrid();
                gridSelCanvas.clearContext();
            }
        break;

        case prevCanvas:
            if (shiftDown) renderPreview();
        break;
    }
};

function cvGridEndAction() {
    if (cvGridActionList.length == 0) return;

    if (cvGridActionList.length == 1) {
        var act = cvGridActionList[0];
        addEditorAction(act.type, act.tile_id, act.data);
    } else {
        var actList = [];
        for (var act of cvGridActionList) {
            actList.push(act);
        }
        addEditorAction("multiple_actions", "", actList);
    }
    cvGridActionList = [];
}

function mapCellClick(tile_id, clickType, render) {
    if (!tilemap.hasOwnProperty(tile_id)) return;

    if (clickType == 1) {
        if (ctrlDown) {
            // Pick tile
            if (tilemap[tile_id].item) itemTempClick("it_temp" + tilemap[tile_id].item);
            else if (tilemap[tile_id].floor) itemTempClick("it_temp" + (100000 + tilemap[tile_id].floor));
            return;
        } else if (selected_item) {
            var old_item = tilemap[tile_id].item;
            var old_floor = tilemap[tile_id].floor;

            if (placeItem(selected_item, tile_id, true)) {
                var act = new EditorAction("place", tile_id, { it_temp: selected_item, it_old: old_item, flr_old: old_floor });
                cvGridActionList.push(act);
            }
        }
    } else if (clickType == 2) {
        var act = new EditorAction("remove", tile_id, { it_old: tilemap[tile_id].item, flags: [] });

        if (tilemap[tile_id].flags.moveblock) act.data.flags.push("moveblock");
        if (tilemap[tile_id].flags.sightblock) act.data.flags.push("sightblock");
        
        if (removeItem(tile_id)) cvGridActionList.push(act);
    }

    if (render) {
        renderGrid();
        renderPreview();
    }
}

var drawFlags = true;
function toggleTileFlags() {
    drawFlags = !drawFlags;
    renderGrid();
    updateUI();
}

function renderGrid() {
    gridCanvas.clearContext();

    var x1 = Math.max(0, Math.floor((-gridCanvas.drawXOffset) / gridTileSize));
    var x2 = Math.max(0, Math.min(tilemap_width, Math.ceil((gridCanvas.cv.width - gridCanvas.drawXOffset) / gridTileSize)));
    var y1 = Math.max(0, Math.floor((-gridCanvas.drawYOffset) / gridTileSize));
    var y2 = Math.max(0, Math.min(tilemap_height, Math.ceil((gridCanvas.cv.height - gridCanvas.drawYOffset) / gridTileSize)));

    for (var i = y1; i < y2; i++) {
        for (var j = x1; j < x2; j++) {
            var tile_id = j + i * tilemap_width;
            var tile = tilemap["maptile" + tile_id];

            var draw_x = gridCanvas.drawXOffset + j * gridTileSize;
            var draw_y = gridCanvas.drawYOffset + i * gridTileSize;

            // Grid border
            if (grid_enabled && gridTileSize > 2) {
                gridCanvas.ctx.strokeStyle = "gray";
                gridCanvas.ctx.lineWidth = 1;
                gridCanvas.ctx.strokeRect(draw_x, draw_y, gridTileSize, gridTileSize);
            }

            var it_temp = null;
            // Fill based on type
            if (tile.item && item_templates.hasOwnProperty("it_temp" + tile.item)) {
                it_temp = item_templates["it_temp" + tile.item];
            }

            if (tile.floor && item_templates.hasOwnProperty("it_temp" + (100000 + tile.floor))) {
                var flr_temp = item_templates["it_temp" + (100000 + tile.floor)];
                gridCanvas.ctx.fillStyle = flr_temp.color;
                if (grid_enabled && gridTileSize > 2) {
                    gridCanvas.ctx.fillRect(draw_x + 1, draw_y + 1, gridTileSize - 2, gridTileSize - 2);
                } else {
                    gridCanvas.ctx.fillRect(draw_x, draw_y, gridTileSize, gridTileSize);
                }
            }

            if (it_temp) {
                if (it_temp.type == "wall") {
                    // Wall
                    gridCanvas.ctx.fillStyle = it_temp.color;
                    if (grid_enabled && gridTileSize > 2) {
                        gridCanvas.ctx.fillRect(draw_x + 1, draw_y + 1, gridTileSize - 2, gridTileSize - 2);
                    } else {
                        gridCanvas.ctx.fillRect(draw_x, draw_y, gridTileSize, gridTileSize);
                    }
                } else if (it_temp.type == "item") {
                    // Item circle
                    gridCanvas.ctx.beginPath();
                    gridCanvas.ctx.arc(draw_x + gridTileSize / 2, draw_y + gridTileSize / 2, gridTileSize / 4, 0, 360);
                    gridCanvas.ctx.fillStyle = it_temp.color;
                    gridCanvas.ctx.fill();
                    gridCanvas.ctx.strokeStyle = "orange";
                    gridCanvas.ctx.lineWidth = 2;
                    gridCanvas.ctx.stroke();
                }
            }

            // Draw flags
            if (drawFlags) {
                // Outline flag if it matches selected item template
                var matchFlag = "";
                if (selected_item && selected_item.type == "flag") {
                    for (var flag in selected_item.flags) {
                        if (selected_item.flags[flag]) matchFlag = flag;
                    }
                }

                var flagSize = Math.ceil(gridTileSize / 6);
                var fpos = 0;
                for (var flag in tile.flags) {
                    if (tile.flags[flag]) {
                        if (item_templates.hasOwnProperty("flag_" + flag)) {
                            gridCanvas.ctx.fillStyle = item_templates["flag_" + flag].color;
                            gridCanvas.ctx.strokeStyle = item_templates["flag_" + flag].color;
                        }
                        var fpos_x = (fpos % 5) * flagSize;
                        var fpos_y = gridTileSize - (1 + Math.floor(fpos / 5)) * flagSize;
                        gridCanvas.ctx.fillRect(draw_x + fpos_x, draw_y + fpos_y, flagSize, flagSize);

                        if (flag == matchFlag) {
                            gridCanvas.ctx.lineWidth = 2;
                            gridCanvas.ctx.strokeRect(draw_x + 1, draw_y + 1, gridTileSize - 2, gridTileSize - 2);
                        }
                    }
                    fpos++;
                }
            }

            // Rectangle paint mode border
            if (paintData.active) {
                switch(paintMode) {
                    case "rect":
                    case "rectfill":
                        var rect_x1 = Math.min(paintData.x1, paintData.x2);
                        var rect_x2 = Math.max(paintData.x1, paintData.x2);
                        var rect_y1 = Math.min(paintData.y1, paintData.y2);
                        var rect_y2 = Math.max(paintData.y1, paintData.y2);
                        if (j >= rect_x1 && j <= rect_x2 && i >= rect_y1 && i <= rect_y2) {
                            if (paintMode == "rectfill" || j == rect_x1 || j == rect_x2 || i == rect_y1 || i == rect_y2) {
                                gridCanvas.ctx.strokeStyle = "blue";
                                gridCanvas.ctx.lineWidth = 1;
                                gridCanvas.ctx.strokeRect(draw_x + 1, draw_y + 1, gridTileSize - 2, gridTileSize - 2);
                            }
                        }
                    break;
                }
            }
        }
    }
}

var hideWalls = false;
function toggleWalls() {
    hideWalls = !hideWalls;
    renderPreview();
}

function renderPreview() {
    prevCanvas.clearContext();

    var cam_x = prevCanvas.cv.width / 2 - prevCanvas.drawXOffset;
    var cam_y = prevCanvas.cv.height / 2 - prevCanvas.drawYOffset;
    var x1 = Math.max(0, Math.round((cam_x - prevCanvas.cv.width - cam_y*2) / 32));
    var x2 = Math.max(0, Math.min(tilemap_width - 1, Math.round((cam_x + prevCanvas.cv.width - cam_y*2) / 32)));
    var y1 = Math.max(0, Math.round((cam_y - prevCanvas.cv.height + cam_x/2) / 16));
    var y2 = Math.max(0, Math.min(tilemap_height, Math.round((cam_y + prevCanvas.cv.height + cam_x/2) / 16)));
    
    for (var i = y1; i < y2; i++) {
        for (var j = x2; j >= x1; j--) {
            var tile_id = j + i * tilemap_width;
            var tile = tilemap["maptile" + tile_id];
            
            // Floor
            if (tile.floor) {
                prevCanvas.drawImageIsometric(tile.getFloorSprite(), j, i);
            }
            // Item/wall
            if (tile.item) {
                var it_spr = tile.getItemSprite();
                if (it_spr) prevCanvas.drawImageIsometric(it_spr, j, i);
            }
        }
    }
}