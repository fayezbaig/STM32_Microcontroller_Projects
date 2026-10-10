/*
 * ble_server.c - BLE GATT server (ESP-IDF, Bluedroid)
 *
 * Environmental Sensing Service (0x181A) with Temperature (0x2A6E)
 * and Pressure (0x2A6D) characteristics, both Read + Notify.
 */
#include "ble_server.h"

#include <stdbool.h>
#include <string.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"

#define TAG                 "ble"
#define GATTS_APP_ID        0x55
#define LOCAL_MTU           100

/* ------------------------------------------------------------------ */
/* Attribute table                                                     */
/* ------------------------------------------------------------------ */
enum {
    IDX_SVC,

    IDX_TEMP_CHAR,
    IDX_TEMP_VAL,
    IDX_TEMP_CCCD,

    IDX_PRESS_CHAR,
    IDX_PRESS_VAL,
    IDX_PRESS_CCCD,

    IDX_NB
};

static const uint16_t UUID_PRIMARY_SVC = ESP_GATT_UUID_PRI_SERVICE;        /* 0x2800 */
static const uint16_t UUID_CHAR_DECL   = ESP_GATT_UUID_CHAR_DECLARE;       /* 0x2803 */
static const uint16_t UUID_CCCD        = ESP_GATT_UUID_CHAR_CLIENT_CONFIG; /* 0x2902 */
static const uint16_t UUID_ESS         = 0x181A;  /* Environmental Sensing */
static const uint16_t UUID_TEMPERATURE = 0x2A6E;
static const uint16_t UUID_PRESSURE    = 0x2A6D;

static const uint8_t PROP_READ_NOTIFY = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

/* Initial values (little-endian, as the BLE spec requires) */
static uint8_t s_temp_init[2]  = {0x00, 0x00};
static uint8_t s_press_init[4] = {0x00, 0x00, 0x00, 0x00};
static uint8_t s_cccd_init[2]  = {0x00, 0x00};

static const esp_gatts_attr_db_t s_gatt_db[IDX_NB] = {
    /* Service declaration */
    [IDX_SVC] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&UUID_PRIMARY_SVC,
        ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(UUID_ESS), (uint8_t *)&UUID_ESS}},

    /* Temperature */
    [IDX_TEMP_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&UUID_CHAR_DECL,
        ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&PROP_READ_NOTIFY}},
    [IDX_TEMP_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&UUID_TEMPERATURE,
        ESP_GATT_PERM_READ, sizeof(s_temp_init), sizeof(s_temp_init), s_temp_init}},
    [IDX_TEMP_CCCD] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&UUID_CCCD,
        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(s_cccd_init), sizeof(s_cccd_init), s_cccd_init}},

    /* Pressure */
    [IDX_PRESS_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&UUID_CHAR_DECL,
        ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&PROP_READ_NOTIFY}},
    [IDX_PRESS_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&UUID_PRESSURE,
        ESP_GATT_PERM_READ, sizeof(s_press_init), sizeof(s_press_init), s_press_init}},
    [IDX_PRESS_CCCD] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&UUID_CCCD,
        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(s_cccd_init), sizeof(s_cccd_init), s_cccd_init}},
};

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */
static uint16_t          s_handles[IDX_NB];
static esp_gatt_if_t     s_gatts_if    = ESP_GATT_IF_NONE;
static volatile bool     s_ready       = false;  /* attribute table created */
static volatile bool     s_connected   = false;
static volatile uint16_t s_conn_id     = 0;
static volatile bool     s_notify_temp = false;
static volatile bool     s_notify_press = false;

/* ------------------------------------------------------------------ */
/* Advertising                                                         */
/* ------------------------------------------------------------------ */
#define ADV_CFG_FLAG       (1 << 0)
#define SCAN_RSP_CFG_FLAG  (1 << 1)
static uint8_t s_adv_config_done;

/* 0x181A expanded to 128-bit (Bluetooth base UUID), little-endian */
static uint8_t s_adv_service_uuid128[16] = {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x1A, 0x18, 0x00, 0x00,
};

/* Advertising packet: flags + name + appearance (fits in 31 bytes) */
static esp_ble_adv_data_t s_adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,
    .include_txpower     = false,
    .min_interval        = 0x0006,   /* preferred conn interval 7.5 ms  */
    .max_interval        = 0x0010,   /*                         20 ms   */
    .appearance          = 0x0300,   /* Generic Thermometer */
    .manufacturer_len    = 0,
    .p_manufacturer_data = NULL,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = 0,
    .p_service_uuid      = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

/* Scan response: service UUID so apps can filter for it */
static esp_ble_adv_data_t s_scan_rsp_data = {
    .set_scan_rsp        = true,
    .include_name        = false,
    .include_txpower     = true,
    .appearance          = 0,
    .manufacturer_len    = 0,
    .p_manufacturer_data = NULL,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = sizeof(s_adv_service_uuid128),
    .p_service_uuid      = s_adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t s_adv_params = {
    .adv_int_min       = 0x20,       /* 20 ms  (units of 0.625 ms) */
    .adv_int_max       = 0x40,       /* 40 ms                      */
    .adv_type          = ADV_TYPE_IND,
    .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .channel_map       = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static void start_advertising(void)
{
    esp_err_t err = esp_ble_gap_start_advertising(&s_adv_params);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "start advertising failed: %s", esp_err_to_name(err));
    }
}

/* ------------------------------------------------------------------ */
/* GAP events                                                          */
/* ------------------------------------------------------------------ */
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        s_adv_config_done &= (uint8_t)~ADV_CFG_FLAG;
        if (s_adv_config_done == 0) start_advertising();
        break;

    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        s_adv_config_done &= (uint8_t)~SCAN_RSP_CFG_FLAG;
        if (s_adv_config_done == 0) start_advertising();
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(TAG, "advertising as \"%s\"", BLE_DEVICE_NAME);
        } else {
            ESP_LOGE(TAG, "advertising start failed (%d)", param->adv_start_cmpl.status);
        }
        break;

    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(TAG, "conn params: interval=%d latency=%d timeout=%d",
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;

    default:
        break;
    }
}

/* ------------------------------------------------------------------ */
/* GATT server events                                                  */
/* ------------------------------------------------------------------ */
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
    case ESP_GATTS_REG_EVT:
        if (param->reg.status != ESP_GATT_OK) {
            ESP_LOGE(TAG, "app register failed (%d)", param->reg.status);
            return;
        }
        s_gatts_if = gatts_if;

        esp_ble_gap_set_device_name(BLE_DEVICE_NAME);

        s_adv_config_done = ADV_CFG_FLAG | SCAN_RSP_CFG_FLAG;
        esp_ble_gap_config_adv_data(&s_adv_data);
        esp_ble_gap_config_adv_data(&s_scan_rsp_data);

        esp_ble_gatts_create_attr_tab(s_gatt_db, gatts_if, IDX_NB, 0);
        break;

    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
        if (param->add_attr_tab.status != ESP_GATT_OK ||
            param->add_attr_tab.num_handle != IDX_NB) {
            ESP_LOGE(TAG, "attribute table creation failed (status %d, handles %d)",
                     param->add_attr_tab.status, param->add_attr_tab.num_handle);
            return;
        }
        memcpy(s_handles, param->add_attr_tab.handles, sizeof(s_handles));
        esp_ble_gatts_start_service(s_handles[IDX_SVC]);
        s_ready = true;
        ESP_LOGI(TAG, "Environmental Sensing service started");
        break;

    case ESP_GATTS_CONNECT_EVT: {
        s_conn_id   = param->connect.conn_id;
        s_connected = true;
        ESP_LOGI(TAG, "phone connected (conn_id %d)", param->connect.conn_id);

        /* Ask for a relaxed connection interval - we only send once a second */
        esp_ble_conn_update_params_t cp = {0};
        memcpy(cp.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        cp.min_int = 0x18;   /* 30 ms  (units of 1.25 ms) */
        cp.max_int = 0x28;   /* 50 ms                     */
        cp.latency = 0;
        cp.timeout = 400;    /* 4 s    (units of 10 ms)   */
        esp_ble_gap_update_conn_params(&cp);
        break;
    }

    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "phone disconnected (reason 0x%02x), advertising again",
                 param->disconnect.reason);
        s_connected    = false;
        s_notify_temp  = false;
        s_notify_press = false;
        start_advertising();
        break;

    case ESP_GATTS_WRITE_EVT:
        /* CCCD write = phone (un)subscribing. AUTO_RSP already answered it. */
        if (!param->write.is_prep && param->write.len == 2) {
            const uint16_t v = (uint16_t)(param->write.value[0] | (param->write.value[1] << 8));
            const bool on = (v & 0x0001) != 0;
            if (param->write.handle == s_handles[IDX_TEMP_CCCD]) {
                s_notify_temp = on;
                ESP_LOGI(TAG, "temperature notifications %s", on ? "ON" : "OFF");
            } else if (param->write.handle == s_handles[IDX_PRESS_CCCD]) {
                s_notify_press = on;
                ESP_LOGI(TAG, "pressure notifications %s", on ? "ON" : "OFF");
            }
        }
        break;

    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(TAG, "MTU = %d", param->mtu.mtu);
        break;

    default:
        break;
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
esp_err_t ble_server_init(void)
{
    esp_err_t err;

    /* BLE only: give the Classic BT memory back to the heap */
    err = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (err != ESP_OK) ESP_LOGW(TAG, "classic BT mem release: %s", esp_err_to_name(err));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK)            goto fail;
    if ((err = esp_bt_controller_enable(ESP_BT_MODE_BLE)) != ESP_OK)  goto fail;
    if ((err = esp_bluedroid_init()) != ESP_OK)                       goto fail;
    if ((err = esp_bluedroid_enable()) != ESP_OK)                     goto fail;
    if ((err = esp_ble_gatts_register_callback(gatts_event_handler)) != ESP_OK) goto fail;
    if ((err = esp_ble_gap_register_callback(gap_event_handler)) != ESP_OK)     goto fail;
    if ((err = esp_ble_gatts_app_register(GATTS_APP_ID)) != ESP_OK)   goto fail;

    err = esp_ble_gatt_set_local_mtu(LOCAL_MTU);
    if (err != ESP_OK) ESP_LOGW(TAG, "set local MTU: %s", esp_err_to_name(err));

    return ESP_OK;

fail:
    ESP_LOGE(TAG, "BLE init failed: %s", esp_err_to_name(err));
    return err;
}

void ble_server_update(int32_t temp_centi, uint32_t press_pa)
{
    if (!s_ready) return;

    /* Temperature: sint16, resolution 0.01 degC, little-endian */
    const int16_t t = (int16_t)temp_centi;
    uint8_t t_le[2] = { (uint8_t)(t & 0xFF), (uint8_t)((t >> 8) & 0xFF) };

    /* Pressure: uint32, resolution 0.1 Pa, little-endian */
    const uint32_t p = press_pa * 10U;
    uint8_t p_le[4] = { (uint8_t)p, (uint8_t)(p >> 8), (uint8_t)(p >> 16), (uint8_t)(p >> 24) };

    /* Update the stored values -> a phone that just READs gets them */
    esp_ble_gatts_set_attr_value(s_handles[IDX_TEMP_VAL],  sizeof t_le, t_le);
    esp_ble_gatts_set_attr_value(s_handles[IDX_PRESS_VAL], sizeof p_le, p_le);

    /* Push to a subscribed phone */
    if (s_connected) {
        if (s_notify_temp) {
            esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id, s_handles[IDX_TEMP_VAL],
                                        sizeof t_le, t_le, false);
        }
        if (s_notify_press) {
            esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id, s_handles[IDX_PRESS_VAL],
                                        sizeof p_le, p_le, false);
        }
    }
}
