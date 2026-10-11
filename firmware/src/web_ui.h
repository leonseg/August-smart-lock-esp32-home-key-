/**
 * web_ui.h
 *
 * Small local control page served by the lock once it is on WiFi (STA mode).
 *   GET  /            control page
 *   GET  /api/status  JSON state
 *   POST /api/lock | /api/unlock
 *   POST /api/led     rgb=r,g,b | brightness=0-255 | off=1
 *
 * NOTE: there is no authentication — anyone on the same network can operate
 * the lock. Intended for bench use; disable (WEB_UI_ENABLED=0) or put it
 * behind a token before installing the lock on a real door.
 */

#ifndef WEB_UI_H
#define WEB_UI_H

#include "config.h"

#ifndef WEB_UI_ENABLED
#define WEB_UI_ENABLED 1
#endif

void web_ui_loop();   // Call from loop(); starts the server once WiFi is up

#endif // WEB_UI_H
