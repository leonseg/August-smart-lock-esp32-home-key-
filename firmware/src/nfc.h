/**
 * nfc.h
 *
 * PN532 SPI interface for tap-to-unlock.
 *
 * This implementation supports basic UID-based authentication — any card
 * whose UID is in the authorized list unlocks the door.
 *
 * For full Apple HomeKey support (iPhone/Apple Watch Express Mode), flash
 * the rednblkx/HomeKey-ESP32 firmware instead — it implements the Apple
 * reverse-engineered ECC protocol required for true HomeKey auth.
 * The hook `nfc_on_homekey_auth()` is the integration point.
 */

#ifndef NFC_H
#define NFC_H

#include "config.h"

typedef void (*nfc_auth_cb_t)(bool authorized, const uint8_t* uid, uint8_t uid_len);

/**
 * Initialize the PN532 over SPI.
 * Returns true on success.
 */
bool nfc_init();

/**
 * Poll for a card. Call from loop().
 * Invokes the auth callback on every successful read (authorized or not).
 */
void nfc_poll();

void nfc_set_auth_callback(nfc_auth_cb_t cb);

/**
 * Authorized UID list management. UIDs persist in NVS.
 */
bool  nfc_add_authorized_uid(const uint8_t* uid, uint8_t len);
bool  nfc_remove_authorized_uid(const uint8_t* uid, uint8_t len);
uint8_t nfc_authorized_count();
void  nfc_clear_authorized();
bool  nfc_is_authorized(const uint8_t* uid, uint8_t len);

/**
 * Hex-string helper.
 */
String nfc_uid_to_string(const uint8_t* uid, uint8_t len);

#endif // NFC_H
