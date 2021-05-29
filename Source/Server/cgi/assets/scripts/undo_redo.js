class EditorAction {
    constructor(type, tile_id, data) {
        this.type = type;
        this.tile_id = tile_id;
        this.data = data;
    }
}

var actionList = [];
var redoList = [];

var actionLimit = 50;

function addEditorAction(type, tile_id, data) {
    actionList.unshift(new EditorAction(type, tile_id, data));
    if (actionList.length > actionLimit) actionList.pop();

    if (redoList.length > 0) redoList = [];
}

function undoAction(action, queueRedo) {
    switch(action.type) {
        case "place":
            if (action.data.it_temp.type == "flag") {
                placeItem(action.data.it_temp, action.tile_id, true);

            } else if (action.data.it_temp.type == "floor") {
                if (action.data.flr_old && item_templates.hasOwnProperty("it_temp" + (100000 + action.data.flr_old))) {
                    placeItem(item_templates["it_temp" + (100000 + action.data.flr_old)], action.tile_id, true);
                }
            } else {
                if (action.data.it_old && item_templates.hasOwnProperty("it_temp" + action.data.it_old)) {
                    placeItem(item_templates["it_temp" + action.data.it_old], action.tile_id, true);
                } else {
                    removeItem(action.tile_id);
                }
            }
        break;

        case "remove":
            if (action.data.it_old && item_templates.hasOwnProperty("it_temp" + action.data.it_old)) {
                placeItem(item_templates["it_temp" + action.data.it_old], action.tile_id, true);
            }
            for (var flag of action.data.flags) {
                if (!tilemap[action.tile_id].flags[flag]) placeItem(item_templates["flag_" + flag], action.tile_id, true);
            }
        break;

        case "multiple_actions":
            for (var sub_act of action.data) undoAction(sub_act, false);
        break;
    }
    if (queueRedo) redoList.unshift(action);
}

function undoLastAction() {
    if (actionList.length == 0) return;
    undoAction(actionList.shift(), true);
    renderGrid();
    renderPreview();
}

function redoAction(action, queueUndo) {
    switch(action.type) {
        case "place":
            placeItem(action.data.it_temp, action.tile_id, true);
        break;

        case "remove":
            removeItem(action.tile_id);
        break;

        case "multiple_actions":
            for (var sub_act of action.data) redoAction(sub_act, false);
        break;
    }
    if (queueUndo) actionList.unshift(action);
}

function redoLastAction() {
    if (redoList.length == 0) return;
    redoAction(redoList.shift(), true);
    renderGrid();
    renderPreview();
}