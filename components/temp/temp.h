#define dht_gpio 23 // Pin conectado al DHT11
#define refresh 5 // tiempo en segundos para refrescar medición en display
#define min_mqtt 5 // Tiempo en minutos para enviar el mensaje MQTT

static const dht_sensor_type_t sensor_type = DHT_TYPE_AM2301;
int16_t humidity=0; // valor de humedad leído
int16_t temperature=0; // valor de temperatura leído
int cont_temp=0; // variable auxiliar para no mostrar la primera medición de temperatura
int cont_mqtt = (min_mqtt*60/refresh)-4;
bool time_sinc_ok = false;

void get_temp(void *pvParameter)
{
    while(1) {
        set_times();
        if (dht_read_data(sensor_type, dht_gpio, &humidity, &temperature) == ESP_OK) {
            ESP_LOGI(TAG,"Humidity: %d%% Temperature: %dC\n", humidity/10, temperature/10);
            if (!time_sinc_ok)
                obtain_time();
            time_t now = time(NULL);
            now-=3*3600;
            timeinfo = localtime(&now);
            strftime(pant_time, sizeof(pant_time), "%H:%M %d-%m-%Y", timeinfo);
            now = time(NULL);
            timeinfo = localtime(&now);
            strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", &timeinfo);
            sprintf(hum_char, "%d", humidity/10);
			sprintf(temp_char, "%d", temperature/10);
            if(level==0)
                pant_main();
            esp_wifi_sta_get_ap_info(&ap_info);
            net_con = (ap_info.authmode != WIFI_AUTH_OPEN);
            if(cont_mqtt==60)
                {
                if (net_con==false)
                    esp_wifi_connect();
                cont_mqtt=0;
                if(mqtt_state)    
                    mqtt_send_info();
                }
            cont_mqtt++;
            if(modo==1){
                if(time_func && ((temperature/10)<=(set_point-hist))){
                    out_temp=true;
                    gpio_set_level(CONTROL, 1);
                }
                if(time_func && ((temperature/10)>=(set_point+hist))){
                    out_temp=false;
                    gpio_set_level(CONTROL, 0);
                }
                if(!time_func){
                    out_temp=false;
                    gpio_set_level(CONTROL, 0);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(1000*refresh));
        } else {
            if (cont_temp > 5){
                ESP_LOGE(TAG,"Could not read data from sensor\n");
			    pant_no_sensor();
                }
            cont_temp++;
            xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
        }
    }
   vTaskDelete(NULL);
}