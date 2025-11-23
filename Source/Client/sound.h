#ifndef THELASTGATE_SOUND_H
#define THELASTGATE_SOUND_H

extern int domusic;
extern int dosound;

int init_sound(void);
int play_sound(char *file, int vol, int p);
void cleanup_sound(void);

#endif //THELASTGATE_SOUND_H