#include "use_queue.h"

#include <stdbool.h>

#include "engine.h"
#include "inter.h"
#include "stubborn_actions.h"

static QueuedCommands queued_commands = {0};

void add_cmd_to_queue(const int cmd, const int x, const int y) {
    // Do not queue an action we are already doing.
    if (pl.misc_action == 4 && pl.misc_target1 == x && pl.misc_target2 == y) return;
    if (queued_commands.count >= 5) {
        xlog(0, "Your queue is full.");
        return;
    }

    if (has_cmd_been_queued(cmd, x, y)) return;

    play_sound("sfx\\click.wav",CLICKVOL, 0);

    queued_commands.commands[queued_commands.tail] = (QueuedCommand){
        .cmd = cmd,
        .x = x,
        .y = y,
    };

    queued_commands.tail = (queued_commands.tail + 1) % 5;
    queued_commands.count++;
    if (pl.misc_action == 0 && queued_commands.count == 1) pop_cmd_from_queue();
}

void pop_cmd_from_queue() {
    if (queued_commands.count == 0) return;
    const QueuedCommand command = queued_commands.commands[queued_commands.head];
    mod_stubborn_actions_set_use_cmd(command.x, command.y);
    cmdq(command.cmd, command.x, command.y);
}

int has_cmd_been_queued(const int cmd, const int x, const int y) {
    int position = 0;
    int index = queued_commands.head;
    int remaining = queued_commands.count;

    while (remaining > 0) {
        const QueuedCommand value = queued_commands.commands[index];
        if (value.cmd == cmd && value.x == x && value.y == y) return position + 1;
        index = (index + 1) % 5;
        remaining--;
        position++;
    }
    return 0;
}

void clear_cmd_queue() {
    queued_commands.head = 0;
    queued_commands.tail = 0;
    queued_commands.count = 0;
}

int cmd_queue_size() {
    return queued_commands.count;
}

void cmd_queue_on_target(int x, int y) {
    if (queued_commands.count == 0) return;
    const QueuedCommand current_command = queued_commands.commands[queued_commands.head];
    if (current_command.x != x || current_command.y != y) return;

    queued_commands.head = (queued_commands.head + 1) % 5;
    queued_commands.count--;
}
