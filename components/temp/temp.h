#define dht_gpio 10 // Pin conectado al DHT11
#define refresh 5 // tiempo en segundos para refrescar medición en display

static const dht_sensor_type_t sensor_type = DHT_TYPE_AM2301;
int16_t humidity=0; // valor de humedad leído
int16_t temperature=0; // valor de temperatura leído
int cont_temp=0; // variable auxiliar para no mostrar la primera medición de temperatura
const int min_mqtt=5; // tiempo en minutos para enviar el mensaje MQTT
int cont_mqtt = min_mqtt * 60 / refresh;
bool time_sinc_ok = false;

void get_temp(void *pvParameter)
{
    while(1) {
        if (dht_read_data(sensor_type, dht_gpio, &humidity, &temperature) == ESP_OK) {
            ESP_LOGI(TAG,"Humidity: %d%% Temperature: %dC\n", humidity / 10, temperature / 10);
            if (!time_sinc_ok)
                {
                vTaskDelay(100 * 1);
                obtain_time();
                }
            time_t now = time(NULL);
            timeinfo = localtime(&now);
            strftime(pant_time, sizeof(pant_time), "%H:%M %d-%m-%Y", timeinfo);
            sprintf(hum_char, "%d", humidity/10);
			sprintf(temp_char, "%d", temperature/10);
            esp_wifi_sta_get_ap_info(&ap_info);
            net_con = (ap_info.authmode != WIFI_AUTH_OPEN);
            if(cont_mqtt==60)
                {
                if (net_con==false)
                {esp_wifi_connect();}
                cont_mqtt=0;
                mqtt_send_info();
                }
            cont_mqtt++;
            pant_main();
            vTaskDelay(1000*refresh / portTICK_PERIOD_MS);
        } else {
            if (cont_temp > 5){
                ESP_LOGE(TAG,"Could not read data from sensor\n");
			    pant_no_sensor();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                }
            cont_temp++;
            vTaskDelay(100 * 1);
        }
		
   }
   vTaskDelete(NULL);
}