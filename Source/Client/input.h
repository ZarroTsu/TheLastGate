#ifndef THELASTGATE_INPUT_H
#define THELASTGATE_INPUT_H

extern int quit;

void init_input(void);
void handle_input(void);

typedef enum {
    NONE,
    INVENTORY,
    SKILL_LIST,
    WAYPOINT_PAGE,
    DEPOT_PAGE,
    CHAT_HISTORY
} ScrollableRegion;

#endif //THELASTGATE_INPUT_H