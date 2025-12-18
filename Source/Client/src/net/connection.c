/*
 * CONNECTION.C - Async Connection State Machine Implementation
 *
 * Non-blocking connection system that replaces the threaded blocking
 * approach in socket.c. This allows frame-by-frame progress updates
 * without freezing the UI or requiring thread synchronization.
 *
 * Extracted from socket.c:so_connect() and socket.c:so_login()
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL_net.h>
#include <zlib.h>

#include "connection.h"
#include "security/security.h"
#include "../log/log.h"
#include "../../common.h"
#include "../../inter.h"
#include "socket.h"
#include "../../options.h"

/* Forward declarations for helper functions */
static void load_unique(void);
static unsigned long xcrypt(unsigned long val);
static int handle_login_packet(unsigned char *buf);

/* Connection info */
char host_addr[84]={MHOST};
char host_proxy[84]={MPROXY};
int host_port=5555;

/* Connection state */
static ConnectionState current_state = CONNECTION_STATE_IDLE;
static ConnectionStatus status;
static int race_param = 0;
static int sex_param = 0;

/* Intermediate connection data */
static IPaddress ip;
static IPaddress proxy_ip;
static unsigned char recv_buf[16];
static int capcnt = 0;         /* Login queue attempt counter */
static char mod[256] = {0};    /* Server mod string (8 packets) */

/* Secret for xcrypt (from socket.c) */
static char secret[256] = "\
Ifhjf64hH8sa,-#39ddj843tvxcv0434dvsdc40G#34Trefc349534Y5#34trecerr943\
5#erZt#eA534#5erFtw#Trwec,9345mwrxm gerte-534lMIZDN(/dn8sfn8&DBDB/D&s\
8efnsd897)DDzD'D'D''Dofs,t0943-rg-gdfg-gdf.t,e95.34u.5retfrh.wretv.56\
u35ti#ertger454z,utizerwt,gerh54zju6iko68ik7k8k9k0k1k2k3k4k5k5k7k";

/* Initialize connection system */
void connection_init(void) {
	current_state = CONNECTION_STATE_IDLE;
	memset(&status, 0, sizeof(status));
	strcpy(status.status_message, "Ready");
	status.state = CONNECTION_STATE_IDLE;
	race_param = 0;
	sex_param = 0;
	so_status = 0;
}

/* Start connection attempt */
void connection_start(int race_id, int sex_id) {
	race_param = race_id;
	sex_param = sex_id;
	current_state = CONNECTION_STATE_RESOLVING;
	status.state = CONNECTION_STATE_RESOLVING;
	status.retry_count = 0;
	strcpy(status.status_message, "Resolving hostname...");
	status.error_message[0] = '\0';
	capcnt = 0;
	so_status = 1;  /* Mark as attempting connection */
}

/* Update connection state machine (call every frame) */
ConnectionStatus connection_update(void) {
	status.state = current_state;

	switch (current_state) {
		case CONNECTION_STATE_IDLE:
			strcpy(status.status_message, "Ready");
			break;

		case CONNECTION_STATE_RESOLVING:
			/* Resolve primary host */
			if (SDLNet_ResolveHost(&ip, host_addr, host_port) < 0) {
				log_warning("Unable to resolve host address: %s", SDLNet_GetError());
				ip.host = INADDR_NONE;
			}

			/* Resolve proxy host */
			if (SDLNet_ResolveHost(&proxy_ip, host_proxy, host_port) < 0) {
				log_warning("Unable to resolve proxy host address: %s", SDLNet_GetError());
				proxy_ip.host = INADDR_NONE;
			}

			/* Check if at least one resolved */
			if (ip.host == INADDR_NONE && proxy_ip.host == INADDR_NONE) {
				log_error("Unable to resolve any host");
				current_state = CONNECTION_STATE_ERROR;
				strcpy(status.error_message, "Could not resolve hostname");
				so_status = 0;
				break;
			}

			/* Success, move to connecting */
			current_state = CONNECTION_STATE_CONNECTING;
			strcpy(status.status_message, "Connecting to server...");
			break;

		case CONNECTION_STATE_CONNECTING:
			/* Try primary host first, then proxy */
			if (ip.host != INADDR_NONE) {
				sock = SDLNet_TCP_Open(&ip);
			} else {
				sock = SDLNet_TCP_Open(&proxy_ip);
			}

			if (!sock) {
				log_error("Unable to connect to host: %s", SDLNet_GetError());
				current_state = CONNECTION_STATE_ERROR;
				strcpy(status.error_message, "Connection refused");
				so_status = 0;
				break;
			}

			/* Create socket set for non-blocking operations */
			socket_set = SDLNet_AllocSocketSet(1);
			if (!socket_set) {
				log_error("Unable to create socket set: %s", SDLNet_GetError());
				current_state = CONNECTION_STATE_ERROR;
				strcpy(status.error_message, "Socket initialization failed");
				so_status = 0;
				break;
			}

			if (SDLNet_TCP_AddSocket(socket_set, sock) < 0) {
				log_error("Unable to add socket to set: %s", SDLNet_GetError());
				current_state = CONNECTION_STATE_ERROR;
				strcpy(status.error_message, "Socket initialization failed");
				so_status = 0;
				break;
			}

			/* Connected, check if we need to send password */
			if (passwd[0]) {
				current_state = CONNECTION_STATE_SENDING_PASSWORD;
				strcpy(status.status_message, "Sending password...");
			} else {
				current_state = CONNECTION_STATE_SENDING_LOGIN;
				strcpy(status.status_message, "Sending login...");
			}
			break;

		case CONNECTION_STATE_SENDING_PASSWORD: {
			unsigned char obuf[16];
			obuf[0] = CL_PASSWD;
			memcpy(obuf + 1, passwd, 15);
			if (SDLNet_TCP_Send(sock, (char*)obuf, 16) < 16) {
				log_error("Failed to send password");
				current_state = CONNECTION_STATE_ERROR;
				strcpy(status.error_message, "Server closed connection");
				so_status = 0;
				break;
			}
			current_state = CONNECTION_STATE_SENDING_LOGIN;
			strcpy(status.status_message, "Sending login info...");
			break;
		}

		case CONNECTION_STATE_SENDING_LOGIN: {
			unsigned char obuf[16];
			/* Prepare login or newlogin packet */
			if (okey.usnr) {
				obuf[0] = CL_LOGIN;
				*(unsigned long*)(obuf + 1) = okey.usnr;
				*(unsigned long*)(obuf + 5) = okey.pass1;
				*(unsigned long*)(obuf + 9) = okey.pass2;
			} else {
				obuf[0] = CL_NEWLOGIN;
			}

			if (SDLNet_TCP_Send(sock, (char*)obuf, 16) < 16) {
				log_error("Failed to send login");
				current_state = CONNECTION_STATE_ERROR;
				strcpy(status.error_message, "Server closed connection");
				so_status = 0;
				break;
			}

			current_state = CONNECTION_STATE_WAITING_CHALLENGE;
			strcpy(status.status_message, "Waiting for server response...");
			break;
		}

		case CONNECTION_STATE_WAITING_CHALLENGE:
		case CONNECTION_STATE_WAITING_LOGIN_RESPONSE: {
			/* Non-blocking check for data */
			int ready = SDLNet_CheckSockets(socket_set, 0);
			if (ready > 0 && SDLNet_SocketReady(sock)) {
				/* Data available, receive packet */
				if (SDLNet_TCP_Recv(sock, recv_buf, 16) < 16) {
					log_error("Server closed connection");
					current_state = CONNECTION_STATE_ERROR;
					strcpy(status.error_message, "Server closed connection");
					so_status = 0;
					break;
				}

				/* Handle the packet */
				int result = handle_login_packet(recv_buf);
				if (result == 1) {
					/* Login successful */
					current_state = CONNECTION_STATE_CONNECTED;
					strcpy(status.status_message, "Connected!");

					/* Initialize zlib decompressor (from socket.c:366) */
					zs.zalloc = Z_NULL;
					zs.zfree = Z_NULL;
					zs.opaque = Z_NULL;
					if (inflateInit(&zs)) {
						log_error("Compressor initialization failed");
						current_state = CONNECTION_STATE_ERROR;
						strcpy(status.error_message, "Compressor failure");
						so_status = 0;
					}
				} else if (result == -1) {
					/* Login failed */
					current_state = CONNECTION_STATE_ERROR;
					so_status = 0;
					SDLNet_TCP_Close(sock);
					sock = NULL;
				}
				/* result == 0 means keep waiting (challenge, cap, mod packets) */
			}
			/* If not ready, just return status - will poll again next frame */
			break;
		}

		case CONNECTION_STATE_CONNECTED:
			strcpy(status.status_message, "Connected");
			break;

		case CONNECTION_STATE_ERROR:
			/* Error message already set */
			break;
	}

	return status;
}

/* Cancel connection attempt */
void connection_cancel(void) {
	if (sock) {
		SDLNet_TCP_Close(sock);
		sock = NULL;
	}
	if (socket_set) {
		SDLNet_FreeSocketSet(socket_set);
		socket_set = NULL;
	}
	current_state = CONNECTION_STATE_IDLE;
	status.state = CONNECTION_STATE_IDLE;
	strcpy(status.status_message, "Cancelled");
	so_status = 0;
}

/* Check if connected */
int connection_is_connected(void) {
	return (current_state == CONNECTION_STATE_CONNECTED) ? 1 : 0;
}

/* Get current status message */
const char* connection_get_status(void) {
	return status.status_message;
}

/* ========================================================================
 * HELPER FUNCTIONS (from socket.c)
 * ======================================================================== */

/* Handle login packet (extracted from so_login in socket.c:152-257) */
static int handle_login_packet(unsigned char *buf) {
	if (buf[0] == SV_CHALLENGE) {
		/* Answer challenge */
		strcpy(status.status_message, "Login challenge...");

		unsigned long tmp = *(unsigned long*)(buf + 1);
		tmp = xcrypt(tmp);

		unsigned char obuf[16];
		obuf[0] = CL_CHALLENGE;
		*(unsigned long*)(obuf + 1) = tmp;
		*(unsigned long*)(obuf + 5) = NETWORKING_VERSION;
		*(unsigned long*)(obuf + 9) = race_param;
		SDLNet_TCP_Send(sock, (char*)obuf, 16);

		load_unique();

		obuf[0] = CL_CMD_UNIQUE;
		*(unsigned long*)(obuf + 1) = unique1;
		*(unsigned long*)(obuf + 5) = unique2;
		SDLNet_TCP_Send(sock, (char*)obuf, 16);

		capcnt = 0;
		current_state = CONNECTION_STATE_WAITING_LOGIN_RESPONSE;
		return 0;
	}

	if (buf[0] == SV_NEWPLAYER) {
		/* New player login success */
		strcpy(status.status_message, "Login as new player OK");
		okey.usnr = *(unsigned long*)(buf + 1);
		okey.pass1 = *(unsigned long*)(buf + 5);
		okey.pass2 = *(unsigned long*)(buf + 9);
		ser_ver = *(unsigned char*)(buf + 13);
		ser_ver += (int)((*(unsigned char*)(buf + 14))) << 8;
		ser_ver += (int)((*(unsigned char*)(buf + 15))) << 16;

		/* Initialize default skill buttons for new player */
		for (int n = 0; n < 20; n++) {
			pdata.xbutton[n].skill_nr = -1;
			strcpy(pdata.xbutton[n].name, "-");
		}
		pdata.xbutton[0].skill_nr = 50;
		strcpy(pdata.xbutton[0].name, "Light");
		pdata.xbutton[19].skill_nr = 51;
		strcpy(pdata.xbutton[19].name, "Recall");

		if (race_param == 4 || race_param == 5) { /* Templar */
			pdata.xbutton[7].skill_nr = 40;
			strcpy(pdata.xbutton[7].name, "Cleave");
		} else if (race_param == 8 || race_param == 9) { /* Harakim */
			pdata.xbutton[7].skill_nr = 24;
			strcpy(pdata.xbutton[7].name, "Blast");
			pdata.xbutton[11].skill_nr = 27;
			strcpy(pdata.xbutton[11].name, "Ghost C");
			pdata.xbutton[4].skill_nr = 11;
			strcpy(pdata.xbutton[4].name, "Magic S");
			pdata.xbutton[3].skill_nr = 17;
			strcpy(pdata.xbutton[3].name, "Protect");
		} else { /* Mercenary */
			pdata.xbutton[11].skill_nr = 37;
			strcpy(pdata.xbutton[11].name, "Blind");
			pdata.xbutton[3].skill_nr = 17;
			strcpy(pdata.xbutton[3].name, "Protect");
		}

		/* Save account info (from options.c) */
		/* Note: save_options() should be called by launcher after connection */
		return 1;
	}

	if (buf[0] == SV_LOGIN_OK) {
		/* Existing player login success */
		ser_ver = *(unsigned long*)(buf + 1);
		strcpy(status.status_message, "Login OK");
		return 1;
	}

	if (buf[0] == SV_EXIT) {
		/* Server kicked us out */
		unsigned int tmp = *(unsigned int*)(buf + 1);
		char xbuf[128];
		if (tmp < 1 || tmp > 14) {
			sprintf(xbuf, "Server demands exit: unknown reason");
		} else {
			/* logout_reason array from common.h */
			extern char *logout_reason[];
			sprintf(xbuf, "Server demands exit: %s", logout_reason[tmp]);
		}
		strcpy(status.error_message, xbuf);
		return -1;
	}

	if (buf[0] == SV_CAP) {
		/* Login queue (server full) */
		unsigned int tmp = *(unsigned int*)(buf + 1);
		unsigned int prio = *(unsigned int*)(buf + 5);
		capcnt++;
		sprintf(status.status_message,
		        "Player limit reached. Queue position: %d, Priority: %d, Try: %d",
		        tmp, prio, capcnt);
		return 0;
	}

	/* Server mod string (8 packets) */
	if (buf[0] == SV_MOD1) { memcpy(mod, buf + 1, 15); return 0; }
	if (buf[0] == SV_MOD2) { memcpy(mod + 15, buf + 1, 15); return 0; }
	if (buf[0] == SV_MOD3) { memcpy(mod + 30, buf + 1, 15); return 0; }
	if (buf[0] == SV_MOD4) { memcpy(mod + 45, buf + 1, 15); return 0; }
	if (buf[0] == SV_MOD5) { memcpy(mod + 60, buf + 1, 15); return 0; }
	if (buf[0] == SV_MOD6) { memcpy(mod + 75, buf + 1, 15); return 0; }
	if (buf[0] == SV_MOD7) { memcpy(mod + 90, buf + 1, 15); return 0; }
	if (buf[0] == SV_MOD8) {
		memcpy(mod + 105, buf + 1, 15);
		/* Mod string complete, could display in launcher if desired */
		return 0;
	}

	return 0;
}

/* Load unique ID from file (from socket.c) */
static void load_unique(void) {
	FILE *fp = fopen("unique.dat", "rb");
	if (!fp) {
		unique1 = (rand() << 16) + rand();
		unique2 = (rand() << 16) + rand();
		return;
	}

	fread(&unique1, 1, 4, fp);
	fread(&unique2, 1, 4, fp);
	fclose(fp);
}

/* Encryption function for challenge response (from socket.c) */
static unsigned long xcrypt(unsigned long val) {
	unsigned char *cval = (unsigned char*)&val;
	unsigned long ret;
	unsigned char *cret = (unsigned char*)&ret;

	for (int n = 0; n < 4; n++) {
		int m = 0;
		m += secret[cval[n]];
		m += secret[cval[n] + 64];
		m += secret[cval[n] + 128];
		m += secret[cval[n] + 192];
		cret[n] = m;
	}

	return ret;
}
