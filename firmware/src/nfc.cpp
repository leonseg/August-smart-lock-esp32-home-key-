/**
 * nfc.cpp
 *
 * PN532 over SPI — passive ISO14443A (MIFARE) UID reading.
 *
 * The Adafruit_PN532 library polls in blocking mode; we keep the timeout
 * short (50ms) so the main loop stays responsive.
 */

#include "nfc.h"
#include <Adafruit_PN532.h>
#include <Preferences.h>
#include <SPI.h>

// SPI mode constructor (SCK, MISO, MOSI, SS)
static Adafruit_PN532 s_pn532(PIN_NFC_SCK, PIN_NFC_MISO, PIN_NFC_MOSI, PIN_NFC_SS);

static bool          s_ok = false;
static nfc_auth_cb_t s_cb = nullptr;
static uint32_t      s_last_poll = 0;

// Authorized list cached in RAM, persisted to NVS as a packed blob.
static uint8_t  s_uids[NFC_MAX_AUTHORIZED_UIDS][NFC_UID_MAX_LEN];
static uint8_t  s_uid_lens[NFC_MAX_AUTHORIZED_UIDS];
static uint8_t  s_uid_count = 0;

static Preferences s_prefs;

static void load_uids() {
    s_prefs.begin(NVS_NAMESPACE, true);
    size_t blob_size = s_prefs.getBytesLength(NVS_KEY_NFC_UIDS);
    s_uid_count = 0;
    if (blob_size > 0 && blob_size <= sizeof(s_uids) + sizeof(s_uid_lens)) {
        uint8_t buf[sizeof(s_uids) + sizeof(s_uid_lens)];
        s_prefs.getBytes(NVS_KEY_NFC_UIDS, buf, blob_size);
        // Format: [count][len0][uid0...][len1][uid1...]...
        size_t off = 0;
        if (blob_size >= 1) {
            s_uid_count = buf[off++];
            if (s_uid_count > NFC_MAX_AUTHORIZED_UIDS) s_uid_count = 0;
            for (uint8_t i = 0; i < s_uid_count && off < blob_size; i++) {
                s_uid_lens[i] = buf[off++];
                if (s_uid_lens[i] > NFC_UID_MAX_LEN) { s_uid_count = i; break; }
                memcpy(s_uids[i], buf + off, s_uid_lens[i]);
                off += s_uid_lens[i];
            }
        }
    }
    s_prefs.end();
    LOG_TAG("NFC", "loaded %u authorized UIDs", s_uid_count);
}

static void save_uids() {
    uint8_t buf[sizeof(s_uids) + sizeof(s_uid_lens) + 1];
    size_t off = 0;
    buf[off++] = s_uid_count;
    for (uint8_t i = 0; i < s_uid_count; i++) {
        buf[off++] = s_uid_lens[i];
        memcpy(buf + off, s_uids[i], s_uid_lens[i]);
        off += s_uid_lens[i];
    }
    s_prefs.begin(NVS_NAMESPACE, false);
    s_prefs.putBytes(NVS_KEY_NFC_UIDS, buf, off);
    s_prefs.end();
}

bool nfc_init() {
    s_pn532.begin();
    uint32_t version = s_pn532.getFirmwareVersion();
    if (!version) {
        LOG_TAG("NFC", "PN532 not found — check SPI wiring & DIP switches");
        s_ok = false;
        return false;
    }
    LOG_TAG("NFC", "PN532 fw=0x%08X", version);
    s_pn532.SAMConfig();
    load_uids();
    s_ok = true;
    return true;
}

void nfc_set_auth_callback(nfc_auth_cb_t cb) { s_cb = cb; }

void nfc_poll() {
    if (!s_ok) return;
    uint32_t now = millis();
    if (now - s_last_poll < NFC_POLL_INTERVAL_MS) return;
    s_last_poll = now;

    uint8_t uid[NFC_UID_MAX_LEN] = {0};
    uint8_t uid_len = 0;
    bool found = s_pn532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uid_len, 50);
    if (!found || uid_len == 0) return;

    bool authorized = nfc_is_authorized(uid, uid_len);
    LOG_TAG("NFC", "tap uid=%s authorized=%d", nfc_uid_to_string(uid, uid_len).c_str(), authorized);
    if (s_cb) s_cb(authorized, uid, uid_len);
}

bool nfc_is_authorized(const uint8_t* uid, uint8_t len) {
    for (uint8_t i = 0; i < s_uid_count; i++) {
        if (s_uid_lens[i] == len && memcmp(s_uids[i], uid, len) == 0) return true;
    }
    return false;
}

bool nfc_add_authorized_uid(const uint8_t* uid, uint8_t len) {
    if (len == 0 || len > NFC_UID_MAX_LEN) return false;
    if (nfc_is_authorized(uid, len)) return true;
    if (s_uid_count >= NFC_MAX_AUTHORIZED_UIDS) return false;
    memcpy(s_uids[s_uid_count], uid, len);
    s_uid_lens[s_uid_count] = len;
    s_uid_count++;
    save_uids();
    return true;
}

bool nfc_remove_authorized_uid(const uint8_t* uid, uint8_t len) {
    for (uint8_t i = 0; i < s_uid_count; i++) {
        if (s_uid_lens[i] == len && memcmp(s_uids[i], uid, len) == 0) {
            for (uint8_t j = i; j < s_uid_count - 1; j++) {
                memcpy(s_uids[j], s_uids[j + 1], NFC_UID_MAX_LEN);
                s_uid_lens[j] = s_uid_lens[j + 1];
            }
            s_uid_count--;
            save_uids();
            return true;
        }
    }
    return false;
}

uint8_t nfc_authorized_count() { return s_uid_count; }

void nfc_clear_authorized() {
    s_uid_count = 0;
    save_uids();
}

String nfc_uid_to_string(const uint8_t* uid, uint8_t len) {
    String out;
    out.reserve(len * 3);
    for (uint8_t i = 0; i < len; i++) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%02X%s", uid[i], (i < len - 1) ? ":" : "");
        out += buf;
    }
    return out;
}
