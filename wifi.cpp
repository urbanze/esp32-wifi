#include "wifi.h"

int8_t WF::wf_sts = 0;

int8_t WF::sta_sts = 0;
int16_t WF::sta_dscrsn = 0;
char WF::sta_ip[] = {0};

int8_t WF::ap_sts = 0;


esp_err_t WF::event_handler(void *ctx, system_event_t *event)
{
    const char tag[] = "WiFi";
    //ESP_LOGI(tag, "[Event]: %d", event->event_id);

    wf_sts = event->event_id;
    

    //STA
    if (event->event_id >= SYSTEM_EVENT_STA_START && event->event_id <= SYSTEM_EVENT_STA_WPS_ER_PIN)
    {
        sta_sts = event->event_id;
        sta_dscrsn = 0;

        if (event->event_id == SYSTEM_EVENT_STA_START)
        {
            esp_err_t err = esp_wifi_connect();
            if (err != ESP_OK)
            {
                ESP_LOGE(tag, "1: 0x%x", err);
            }
        }
        else if (event->event_id == SYSTEM_EVENT_STA_CONNECTED)
        {
            ESP_LOGI(tag, "[STA]: Connected");
        }
        else if (event->event_id == SYSTEM_EVENT_STA_DISCONNECTED)
        {
            system_event_sta_disconnected_t *disconnected = &event->event_info.disconnected;
            sta_dscrsn = disconnected->reason;
            memset(sta_ip, 0, sizeof(sta_ip));

            ESP_LOGW(tag, "STA_DISCONNECTED_REASON: %d", disconnected->reason);
        }
        else if (event->event_id == SYSTEM_EVENT_STA_GOT_IP)
        {
            snprintf(sta_ip, 16, "%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        }
        else if (event->event_id == SYSTEM_EVENT_STA_LOST_IP)
        {
            memset(sta_ip, 0, sizeof(sta_ip));
        }
    }

    //AP
    else if (event->event_id >= SYSTEM_EVENT_AP_START && event->event_id <= SYSTEM_EVENT_AP_PROBEREQRECVED)
    {
        ap_sts = event->event_id;
    }


    

    return ESP_OK;
}

/**
 * @brief Init driver/necessary itens to WiFi work.
 * 
 * @return [0]: Fail.
 * @return [1]: Sucess.
  */
int8_t WF::init()
{
    if (started) {return 1;}
    

    esp_err_t err;
    

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(tag, "Fail to init default NVS partition, erasing...");

        err = nvs_flash_erase();
        if (err != ESP_OK)
        {
            ESP_LOGE(tag, "Fail to erase NVS [0x%x]", err);
            return 0;
        }

        err = nvs_flash_init();
        if (err != ESP_OK)
        {
            ESP_LOGE(tag, "Fail to reinit NVS [0x%x]", err);
            return 0;
        }
    }
    else if (err != ESP_OK)
    {
        ESP_LOGE(tag, "NVS error");
        return 0;
    }
    

    tcpip_adapter_init();
    err = esp_event_loop_init(event_handler, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "Fail to create event loop [0x%x]", err);
        return 0;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "Fail to init WiFi [0x%x]", err);
        return 0;
    }

    esp_wifi_set_mode(WIFI_MODE_NULL);
    
    started = 1;
    return 1;
}


//============Generic functions============

/**
 * @brief Get last WiFi event.
 * 
 * This status refers to all events [typedef enum system_event_id_t], check it!
 */
int8_t WF::status()
{
    return wf_sts;
}

/**
 * @brief Get actual WiFi mode.
 * 
 * @return [0]: WIFI_MODE_NULL.
 * @return [1]: WIFI_MODE_STA.
 * @return [2]: WIFI_MODE_AP.
 * @return [3]: WIFI_MODE_APSTA.
 */
int8_t WF::mode()
{
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    return mode;
}


//============STA functions================

/**
 * @brief Enable use of STATIC IP
 * 
 * @param [*ip]: IP. (Eg: "192.168.0.110")
 * @param [*gateway]: Network gateway. (Eg: "192.168.0.100")
 * @param [*mask]: Network mask. (Eg: "255.255.255.0")
 */
void WF::sta_static_ip(const char *ip, const char *gateway, const char *mask)
{
    if (!init()) {return;}

    esp_err_t err;
    err = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
    if (err != ESP_OK)
        {ESP_LOGW(tag, "DHCP stop fail");}

    tcpip_adapter_ip_info_t cfg;
    cfg.ip.addr = ipaddr_addr(ip);
    cfg.gw.addr = ipaddr_addr(gateway);
    cfg.netmask.addr = ipaddr_addr(mask);

    err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &cfg);
    if (err != ESP_OK)
        {ESP_LOGE(tag, "Static ip fail");}
}

/**
 * @brief Connect ESP32 in specific WiFi Station.
 * 
 * @param [*ssid]: SSID (name) of station.
 * @param [*pass]: Password of stations.
 * @param [wait]: Block this function up to 20sec until STA got IP Address.
 */
void WF::sta_connect(const char *ssid, const char *pass, int8_t wait=1)
{
    esp_err_t err;


    if (!init()) {return;}

    wifi_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    strncpy((char*)cfg.sta.ssid, ssid, 32);
    strncpy((char*)cfg.sta.password, pass, 64);
    cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    cfg.sta.bssid_set = 0;
    cfg.sta.channel = 0;
    cfg.sta.listen_interval = 0;
    cfg.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    cfg.sta.threshold.rssi = -127;
    cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
    
    

    wifi_mode_t mode;
    err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[STA]: Get mode fail [0x%x]", err);
        return;
    }

    if (mode == WIFI_MODE_AP)
        {err = esp_wifi_set_mode(WIFI_MODE_APSTA);}
    else 
        {err = esp_wifi_set_mode(WIFI_MODE_STA);}

    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[STA]: Set mode fail [0x%x]", err);
        return;
    }

    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[STA]: Set configs fail [0x%x]", err);
        return;
    }

    

    err = esp_wifi_start();
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[STA]: Start WiFi fail [0x%x]", err);
        return;
    }

    tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, "ESP32");
    ESP_LOGI(tag, "[STA]: Started at [%s] [%s]", ssid, pass);

    if (wait)
    {
        for (int16_t i = 0; i < 1000; i++)
        {
            vTaskDelay(pdMS_TO_TICKS(20));

            if (sta_status() == SYSTEM_EVENT_STA_GOT_IP || sta_dscrsn != 0)
                {break;}
        }

        if (sta_status() != SYSTEM_EVENT_STA_GOT_IP)
        {
            ESP_LOGW(tag, "DHCP fail");
        }
    }
}

/**
 * @brief Disconnect ESP32 from station.
 */
void WF::sta_disconnect()
{
    esp_err_t err;
    
    err = esp_wifi_disconnect();
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[STA]: Disconnect fail [0x%x]", err);
        return;
    }
}

/**
 * @brief Reconnect in station set before.
 */
void WF::sta_reconnect()
{
    if (!started) {return;}

    sta_sts = 2;

    esp_err_t err = esp_wifi_connect();
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[STA]: Reconnect fail [0x%x]", err);
    }
}

/**
 * @brief Get station IP Address.
 * 
 * @return [char*]: IP Address in string format. Eg: "192.168.1.1".
 */
char *WF::sta_get_ip()
{
    return sta_ip;
}

/**
 * @brief Get last WiFi STA event.
 * 
 * This status refers only to STATION events [typedef enum system_event_id_t], check it!
 */
int8_t WF::sta_status()
{
    return sta_sts;
}

int16_t WF::sta_disconnect_reason()
{
    return sta_dscrsn;
}



//============AP functions=================

/**
 * @brief Create Acess Point (AP).
 * 
 * @param [*ssid]: AP SSID.
 * @param [*pass]: AP password.
 * @param [channel]: AP channel.
 * @param [max]: Max simult. clients.
 * @param [hidden]: Hidde AP SSID broadcast.
 */
void WF::ap_start(const char *ssid, const char *pass, int8_t channel=1, int8_t max=4, int8_t hidden=0)
{
    esp_err_t err;

    if (!init()) {return;}

    wifi_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    cfg.ap.beacon_interval = 100;
    cfg.ap.channel = channel;
    cfg.ap.max_connection = max;
    strncpy((char*)cfg.ap.ssid, ssid, 32);
    strncpy((char*)cfg.ap.password, pass, 64);
    cfg.ap.ssid_hidden = hidden;
    cfg.ap.ssid_len = strlen(ssid);
    if (strlen(pass) < 8) {cfg.ap.authmode = WIFI_AUTH_OPEN;}


    wifi_mode_t mode;
    err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[AP]: Get mode fail [0x%x]", err);
        return;
    }

    if (mode == WIFI_MODE_STA)
        {err = esp_wifi_set_mode(WIFI_MODE_APSTA);}
    else 
        {err = esp_wifi_set_mode(WIFI_MODE_AP);}

    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[AP]: Set mode fail [0x%x]", err);
        return;
    }

    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &cfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[AP]: Set config fail [0x%x]", err);
        return;
    }

    
    err = esp_wifi_start();
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[AP]: Start fail [0x%x]", err);
        return;
    }

    ESP_LOGI(tag, "[AP]: started at [%s] [%s]", ssid, pass);
}

/**
 * @brief Close ESP32 AP.
 */
void WF::ap_stop()
{
    esp_err_t err;

    wifi_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &cfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[AP]: Set config fail [0x%x]", err);
        return;
    }

    wifi_mode_t mode;
    err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[AP]: Get mode fail [0x%x]", err);
        return;
    }

    if (mode == WIFI_MODE_APSTA)
        {err = esp_wifi_set_mode(WIFI_MODE_STA);}
    else 
        {err = esp_wifi_set_mode(WIFI_MODE_NULL);}

    if (err != ESP_OK)
    {
        ESP_LOGE(tag, "[AP]: Set mode fail [0x%x]", err);
        return;
    }
}

/**
 * @brief Get last WiFi AP event.
 * 
 * This status refers only to AP events [typedef enum system_event_id_t], check it!
 */
int8_t WF::ap_status()
{
    return ap_sts;
}
