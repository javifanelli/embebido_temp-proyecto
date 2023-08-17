#define dht_gpio 23 // Pin conectado al DHT11
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
            ESP_LOGI(TAG,"Humidity: %d%% Temperature: %dC\n", humidity/10, temperature/10);
            if (!time_sinc_ok)
                {
                    obtain_time();
                }
            time_t now = time(NULL);
            /* now -= 10800; // Ajusta el tiempo restando 3 horas (10800 segundos) */
            timeinfo = localtime(&now);
            strftime(pant_time, sizeof(pant_time), "%H:%M %d-%m-%Y", timeinfo);
            sprintf(hum_char, "%d", humidity/10);
			sprintf(temp_char, "%d", temperature/10);
            if(level==0)
                pant_main();
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
            if(modo==1 && ((temperature/10)<=(set_point-hist))){
                out_temp=true;
                gpio_set_level(CONTROL, 1);
            }
            if(modo==1 && ((temperature/10)>=(set_point+hist))){
                out_temp=false;
                gpio_set_level(CONTROL, 0);
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