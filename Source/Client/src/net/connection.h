/*
 * CONNECTION.H - Async Connection State Machine
 *
 * Replaces the threaded blocking connection system with a non-blocking
 * async state machine that can be polled frame-by-frame without freezing
 * the main thread.
 *
 * This allows the launcher to show connection progress and cancel
 * connection attempts without thread synchronization complexity.
 */

#pragma once

/* Connection states for async state machine */
typedef enum {
	CONNECTION_STATE_IDLE,                    /* No connection attempt */
	CONNECTION_STATE_CONNECTING,
	CONNECTION_STATE_SENDING_PASSWORD,        /* Sending password packet (if passwd[0] != 0) */
	CONNECTION_STATE_SENDING_LOGIN,           /* Sending login/newlogin packet */
	CONNECTION_STATE_WAITING_CHALLENGE,       /* Waiting for SV_CHALLENGE from server */
	CONNECTION_STATE_WAITING_LOGIN_RESPONSE,  /* Waiting for SV_LOGIN_OK/SV_NEWPLAYER */
	CONNECTION_STATE_CONNECTED,               /* Successfully connected */
	CONNECTION_STATE_ERROR                    /* Connection failed */
} ConnectionState;

/* Connection status structure returned by connection_update() */
typedef struct {
	ConnectionState state;
	char status_message[256];    /* Human-readable status for UI display */
	char error_message[256];     /* Error message if state == CONN_STATE_ERROR */
	int retry_count;             /* Number of retries attempted */
} ConnectionStatus;

/* Connecting Thread */
typedef enum {
	CONNECTING_IDLE,
	CONNECTING_RESOLVE_HOST,
	CONNECTING_RESOLVE_PROXY,
	CONNECTING_CONNECT_HOST,
	CONNECTING_CONNECT_PROXY,
	CONNECTING_DONE,
	CONNECTING_ERROR
} ConnectingStatus;

/* Connection info */
extern char host_addr[84];
extern char host_proxy[84];
extern int host_port;

/* Initialize connection system (call once at startup) */
void connection_init(void);

/* Start connection attempt with given race/sex for new characters
 * Uses existing globals: okey, passwd, pdata for credentials
 * Race is encoded as: (user_race - 1) * 4 + 4 + sex * 2
 */
void connection_start(int race_id, int sex_id);

/* Update connection state (call every frame)
 * Returns status with current state and messages
 * Non-blocking: returns immediately regardless of socket state
 */
ConnectionStatus connection_update(void);

/* Cancel connection attempt and return to IDLE state */
void connection_cancel(void);

/* Check if connected (returns 1 if CONNECTED, 0 otherwise) */
int connection_is_connected(void);

/* Get current status message for UI (convenience function) */
const char* connection_get_status(void);
