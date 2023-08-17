// Certificados usados para la conexión segura con el broker
extern const uint8_t client_cert_pem_start[] asm("_binary_client_pem_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_pem_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_pem_end");

#include "sntp_time.h"

#define BROKER_URI "mqtts://192.168.0.70" // IP del broker (Raspberry Pi)
#define TAG "sensor"
char formatted_time[20]; // Fecha y hora en char para mandar por MQTT
static const char *ID ="1";
static char *buffer_mqtt;
static char TOPIC_OUT[50]="/home/temperatura/data"; // Topic de MQTT de datos de salida
static char TOPIC_IN[50]="/home/temperatura/settings"; // Topic de MQTT de datos de entrada
static esp_mqtt_client_handle_t client;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    /* esp_mqtt_client_handle_t  */client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        mqtt_state = true;
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, TOPIC_OUT, 0);
        msg_id = esp_mqtt_client_subscribe(client, TOPIC_IN, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        ssd1306_clear_screen(&devd, false);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqtt_state = false;
        break;
    
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        buffer_mqtt = event->data;
        break;
    
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = BROKER_URI,
        .client_cert_pem = (const char *)client_cert_pem_start,
        .client_key_pem = (const char *)client_key_pem_start,
        .cert_pem = (const char *)server_cert_pem_start,
    };
    
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void mqtt_send_info(void)
{
    char out_char[10];
    char m_char[10];
    memset(out_char, 0, sizeof(out_char));
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "ID", ID);
    cJSON_AddStringToObject(root, "tipo", tipo_disp);
    strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", timeinfo);
    cJSON_AddStringToObject(root, "time", formatted_time);
    cJSON_AddStringToObject(root, "valor", temp_char);
    cJSON_AddStringToObject(root, "set_point", sp_char);
    if(modo==0)
        cJSON_AddStringToObject(root, "modo", "Manual");
    if(modo==1)
        cJSON_AddStringToObject(root, "modo", "Automático");
    if (out_temp == false)
        cJSON_AddStringToObject(root, "salida", "0");
    if (out_temp == true)
        cJSON_AddStringToObject(root, "salida", "100");
    
    char *json_string = cJSON_PrintUnformatted(root);
    esp_mqtt_client_publish(client, TOPIC_OUT, json_string, 0, 0, 0);
    free(json_string);
    cJSON_Delete(root);
}