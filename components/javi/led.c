#include "esp_wifi.h"
#include "esp_log.h"

#define LED_R GPIO_NUM_15
#define LED_G GPIO_NUM_4
#define CONTROL GPIO_NUM_13
#define TAG "Led"

TickType_t xLastWakeTime;
bool btn_enc;
extern bool inc_enc;
extern bool dec_enc;

void config_led (void);
void read_enc (void *pvParameter);

void config_led (void)
{
    gpio_pad_select_gpio(LED_R);
    gpio_set_direction(LED_R, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(LED_G);
    gpio_set_direction(LED_G, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(CONTROL);
    gpio_set_direction(CONTROL, GPIO_MODE_OUTPUT);
}

void read_enc (void *pvParameter)
{
    while(1){
		if (level==0 && btn_enc){
			btn_enc=false;
			level=1;
			menu1();
		}
		if (modo==1 && level==0 && inc_enc){	
			inc_enc=false;
			set_point+=1;
				if(set_point>28)
					set_point=28;
			sprintf(sp_char, "%d", set_point);
			pant_main();
		}	
    	if(modo==1 && level==0 && dec_enc){
			dec_enc=false;
			set_point-=1;
			if(set_point<18)
				set_point=18;
			sprintf(sp_char, "%d", set_point);
			pant_main();
			}
		if(modo==0 && level==0 && dec_enc){
			dec_enc=false;
			out_temp=false;
			gpio_set_level(CONTROL, 0);
			pant_main();
		}
		if(modo==0 && level==0 && inc_enc){	
			inc_enc=false;
			out_temp=true;
			gpio_set_level(CONTROL, 1);
			pant_main();
		}
		xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
	}
	vTaskDelete(NULL);
}