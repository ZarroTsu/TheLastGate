#pragma once
#include <stdbool.h>

typedef struct {
    int cmd;
    int x;
    int y;
} QueuedCommand;

typedef struct {
    QueuedCommand commands[5];
    int head;
    int tail;
    int count;
} QueuedCommands;

void add_cmd_to_queue(int cmd, int x, int y);
void pop_cmd_from_queue();
int has_cmd_been_queued(int cmd, int x, int y);
void clear_cmd_queue();
int cmd_queue_size();
void cmd_queue_on_target(int x, int y);