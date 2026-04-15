/**
 * wifi_mgr.h
 *
 * WiFi connection manager with AP fallback for first-time setup.
 *
 * Behaviour:
 *  - On boot, load saved credentials from NVS.
 *  - If creds present: try to connect (timeout WIFI_CONNECT_TIMEOUT_MS).
 *  - If no creds OR connect fails: start an open AP "AugustHomeKey-XXXX"
 *    and serve a captive-portal-style config page at http://192.168.4.1
 *  - Once configured, save creds and reboot into STA mode.
 */

#ifndef WIFI_MGR_H
#define WIFI_MGR_H

#include "config.h"

typedef enum {
    WIFI_STATE_BOOT,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_AP_CONFIG,
    WIFI_STATE_RETRY
} wifi_state_t;

void wifi_mgr_init();
void wifi_mgr_loop();

wifi_state_t wifi_mgr_state();
String       wifi_mgr_ssid();
int          wifi_mgr_rssi();
String       wifi_mgr_ip();

/**
 * Persist new credentials (called by the config web handler).
 * Returns true on success.
 */
bool wifi_mgr_save_credentials(const String& ssid, const String& pass);

/**
 * Erase saved credentials and reboot into AP config mode.
 */
void wifi_mgr_factory_reset();

#endif // WIFI_MGR_H
