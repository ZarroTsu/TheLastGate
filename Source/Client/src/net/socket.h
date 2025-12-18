#pragma once
#include <SDL2/SDL_net.h>

extern int ser_ver;
extern char passwd[15];
extern struct z_stream_s zs;

/* Socket globals */
extern TCPsocket sock;
extern SDLNet_SocketSet socket_set;
extern int so_status;