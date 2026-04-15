/**
 * mqtt.cpp
 *
 * PubSubClient wrapper + Home Assistant MQTT auto-discovery.
 */

#include "mqtt.h"
#include "lock_controller.h"
#include "leds.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>

static WiFiClient    s_wifi_client;
static PubSubClient  s_mqtt(s_wifi_client);
static uint32_t      s_last_connect_try = 0;
static uint32_t      s_last_publish = 0;
static String        s_device_id;

static String        s_host;
static uint16_t      s_port = MQTT_DEFAULT_PORT;
static String        s_user;
static String        s_pass;

static String device_id() {
    if (s_device_id.length()) return s_device_id;
    uint64_t mac = ESP.getEfuseMac();
    char buf[13];
    snprintf(buf, sizeof(buf), "%012llx", mac);
    s_device_id = String("august_homekey_") + buf;
    return s_device_id;
}

static const char* state_payload(lock_state_t s) {
    switch (s) {
        case LOCK_STATE_LOCKED:     return MQTT_PAYLOAD_LOCKED;
        case LOCK_STATE_UNLOCKED:   return MQTT_PAYLOAD_UNLOCKED;
        case LOCK_STATE_LOCKING:    return "LOCKING";
        case LOCK_STATE_UNLOCKING:  return "UNLOCKING";
        case LOCK_STATE_JAMMED:     return "JAMMED";
        default:                    return "UNKNOWN";
    }
}

static void on_message(char* topic, byte* payload, unsigned int len) {
    String t(topic);
    String p; p.reserve(len);
    for (unsigned i = 0; i < len; i++) p += (char)payload[i];
    p.trim(); p.toUpperCase();
    LOG_TAG("MQTT", "rx %s = %s", topic, p.c_str());

    if (t == MQTT_TOPIC_CMD) {
        if (p == MQTT_PAYLOAD_LOCK)        lock_controller_request_lock();
        else if (p == MQTT_PAYLOAD_UNLOCK) lock_controller_request_unlock();
    } else if (t == MQTT_TOPIC_LED_SET) {
        leds_set_mode(p == "ON" ? LED_MODE_CUSTOM : LED_MODE_OFF);
        mqtt_publish_led_state();
    } else if (t == MQTT_TOPIC_LED_BRIGHTNESS_SET) {
        int b = p.toInt();
        if (b >= 0 && b <= 255) leds_set_brightness((uint8_t)b);
        mqtt_publish_led_state();
    } else if (t == MQTT_TOPIC_LED_RGB_SET) {
        int r, g, b;
        if (sscanf(p.c_str(), "%d,%d,%d", &r, &g, &b) == 3) {
            leds_set_custom_rgb((uint8_t)r, (uint8_t)g, (uint8_t)b);
        }
        mqtt_publish_led_state();
    }
}

static void subscribe_all() {
    s_mqtt.subscribe(MQTT_TOPIC_CMD);
    s_mqtt.subscribe(MQTT_TOPIC_LED_SET);
    s_mqtt.subscribe(MQTT_TOPIC_LED_BRIGHTNESS_SET);
    s_mqtt.subscribe(MQTT_TOPIC_LED_RGB_SET);
}

static bool try_connect() {
    if (s_host.isEmpty()) return false;
    s_mqtt.setServer(s_host.c_str(), s_port);
    s_mqtt.setCallback(on_message);
    s_mqtt.setKeepAlive(MQTT_KEEPALIVE_SEC);

    String cid = device_id();
    bool ok;
    if (s_user.length())
        ok = s_mqtt.connect(cid.c_str(), s_user.c_str(), s_pass.c_str(),
                            MQTT_TOPIC_AVAILABILITY, 1, true, MQTT_PAYLOAD_OFFLINE);
    else
        ok = s_mqtt.connect(cid.c_str(),
                            MQTT_TOPIC_AVAILABILITY, 1, true, MQTT_PAYLOAD_OFFLINE);

    if (ok) {
        LOG_TAG("MQTT", "connected to %s:%u", s_host.c_str(), s_port);
        subscribe_all();
        mqtt_publish_availability(true);
        mqtt_publish_discovery();
        mqtt_publish_state(lock_controller_state());
    } else {
        LOG_TAG("MQTT", "connect failed rc=%d", s_mqtt.state());
    }
    return ok;
}

void mqtt_init() {
    Preferences p;
    p.begin(NVS_NAMESPACE, true);
    s_host = p.getString(NVS_KEY_MQTT_HOST, "");
    s_port = p.getUInt(NVS_KEY_MQTT_PORT, MQTT_DEFAULT_PORT);
    s_user = p.getString(NVS_KEY_MQTT_USER, "");
    s_pass = p.getString(NVS_KEY_MQTT_PASS, "");
    p.end();

    s_mqtt.setBufferSize(1024);  // enough for HA discovery payloads
    LOG_TAG("MQTT", "config host=%s:%u user=%s", s_host.c_str(), s_port,
            s_user.isEmpty() ? "(none)" : s_user.c_str());
}

void mqtt_loop() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (s_host.isEmpty()) return;

    if (!s_mqtt.connected()) {
        uint32_t now = millis();
        if (now - s_last_connect_try < MQTT_RECONNECT_INTERVAL_MS) return;
        s_last_connect_try = now;
        try_connect();
        return;
    }

    s_mqtt.loop();

    uint32_t now = millis();
    if (now - s_last_publish > MQTT_PUBLISH_INTERVAL_MS) {
        s_last_publish = now;
        mqtt_publish_availability(true);
        mqtt_publish_rssi(WiFi.RSSI());
    }
}

bool mqtt_is_connected() { return s_mqtt.connected(); }

void mqtt_publish_state(lock_state_t s) {
    if (!s_mqtt.connected()) return;
    s_mqtt.publish(MQTT_TOPIC_STATE, state_payload(s), true);
}

void mqtt_publish_availability(bool online) {
    if (!s_mqtt.connected()) return;
    s_mqtt.publish(MQTT_TOPIC_AVAILABILITY,
                   online ? MQTT_PAYLOAD_ONLINE : MQTT_PAYLOAD_OFFLINE, true);
}

void mqtt_publish_battery(uint8_t pct) {
    if (!s_mqtt.connected()) return;
    char buf[8];
    snprintf(buf, sizeof(buf), "%u", pct);
    s_mqtt.publish(MQTT_TOPIC_BATTERY, buf, true);
}

void mqtt_publish_rssi(int rssi) {
    if (!s_mqtt.connected()) return;
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", rssi);
    s_mqtt.publish(MQTT_TOPIC_RSSI, buf, true);
}

void mqtt_publish_led_state() {
    if (!s_mqtt.connected()) return;
    s_mqtt.publish(MQTT_TOPIC_LED_STATE, leds_mode() == LED_MODE_OFF ? "OFF" : "ON", true);
    char buf[8];
    snprintf(buf, sizeof(buf), "%u", leds_brightness());
    s_mqtt.publish(MQTT_TOPIC_LED_BRIGHTNESS, buf, true);
}

/**
 * Home Assistant MQTT discovery — publishes a lock entity + LED light entity.
 * Topic: homeassistant/<component>/<object_id>/config
 */
void mqtt_publish_discovery() {
    if (!s_mqtt.connected()) return;
    String id = device_id();

    // Device block shared by all entities
    StaticJsonDocument<512> dev;
    JsonObject device = dev.to<JsonObject>();
    JsonArray ids = device.createNestedArray("ids");
    ids.add(id);
    device["name"] = "August HomeKey Lock";
    device["mf"]   = FW_AUTHOR;
    device["mdl"]  = "August ASL-02 + ESP32-C6";
    device["sw"]   = FW_VERSION;

    // --- Lock entity ---
    {
        StaticJsonDocument<768> doc;
        doc["name"]             = "August HomeKey";
        doc["uniq_id"]          = id + "_lock";
        doc["stat_t"]           = MQTT_TOPIC_STATE;
        doc["cmd_t"]            = MQTT_TOPIC_CMD;
        doc["avty_t"]           = MQTT_TOPIC_AVAILABILITY;
        doc["pl_lock"]          = MQTT_PAYLOAD_LOCK;
        doc["pl_unlk"]          = MQTT_PAYLOAD_UNLOCK;
        doc["stat_locked"]      = MQTT_PAYLOAD_LOCKED;
        doc["stat_unlocked"]    = MQTT_PAYLOAD_UNLOCKED;
        doc["stat_locking"]     = "LOCKING";
        doc["stat_unlocking"]   = "UNLOCKING";
        doc["stat_jammed"]      = "JAMMED";
        doc["device"]           = device;
        String topic = "homeassistant/lock/" + id + "/config";
        String payload; serializeJson(doc, payload);
        s_mqtt.publish(topic.c_str(), payload.c_str(), true);
    }

    // --- RSSI sensor ---
    {
        StaticJsonDocument<512> doc;
        doc["name"]         = "August HomeKey RSSI";
        doc["uniq_id"]      = id + "_rssi";
        doc["stat_t"]       = MQTT_TOPIC_RSSI;
        doc["unit_of_meas"] = "dBm";
        doc["dev_cla"]      = "signal_strength";
        doc["ent_cat"]      = "diagnostic";
        doc["device"]       = device;
        String topic = "homeassistant/sensor/" + id + "_rssi/config";
        String payload; serializeJson(doc, payload);
        s_mqtt.publish(topic.c_str(), payload.c_str(), true);
    }

    // --- LED light entity ---
    {
        StaticJsonDocument<768> doc;
        doc["name"]       = "August HomeKey LEDs";
        doc["uniq_id"]    = id + "_led";
        doc["stat_t"]     = MQTT_TOPIC_LED_STATE;
        doc["cmd_t"]      = MQTT_TOPIC_LED_SET;
        doc["bri_stat_t"] = MQTT_TOPIC_LED_BRIGHTNESS;
        doc["bri_cmd_t"]  = MQTT_TOPIC_LED_BRIGHTNESS_SET;
        doc["rgb_stat_t"] = MQTT_TOPIC_LED_RGB;
        doc["rgb_cmd_t"]  = MQTT_TOPIC_LED_RGB_SET;
        doc["avty_t"]     = MQTT_TOPIC_AVAILABILITY;
        doc["device"]     = device;
        String topic = "homeassistant/light/" + id + "_led/config";
        String payload; serializeJson(doc, payload);
        s_mqtt.publish(topic.c_str(), payload.c_str(), true);
    }

    LOG_TAG("MQTT", "discovery published");
}
