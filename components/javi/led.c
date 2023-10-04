#define LED_R GPIO_NUM_15
#define LED_G GPIO_NUM_4
#define LED_B GPIO_NUM_2
#define CONTROL GPIO_NUM_13
#define TAG "Led"

TickType_t xLastWakeTime;
bool btn_enc;
extern bool inc_enc;
extern bool dec_enc;

void config_led (void);
void read_enc (void *pvParameter);
void blink_1 (void);
void blink_2(void);
void blink_3(void);

void config_led (void)
{
    gpio_pad_select_gpio(LED_R);
    gpio_set_direction(LED_R, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(LED_G);
    gpio_set_direction(LED_G, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(CONTROL);
    gpio_set_direction(CONTROL, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(LED_B);
    gpio_set_direction(LED_B, GPIO_MODE_OUTPUT);
}

void read_enc (void *pvParameter)
{
    while(level==0){
		if (btn_enc){
			btn_enc=false;
			level=1;
			menu1();
		}
		if(modo==0){
			if (inc_enc){	
				inc_enc=false;
				out_temp=true;
				pant_main();
			}	
    		if(dec_enc){
				dec_enc=false;
				out_temp=false;
				pant_main();
			}
			if((temperature/10)>=30)
				out_temp=false;
			if(out_temp)
				gpio_set_level(CONTROL, 1);
			if(!out_temp)
				gpio_set_level(CONTROL, 0);
		}
		sprintf(sp_char, "%d", set_point);
		vTaskDelay(pdMS_TO_TICKS(50));
	}
	vTaskDelete(NULL);
}

void blink_1(void){
	int i=0;
	bool led=false;
	while(i<2){
		if(!led){
			gpio_set_level(LED_B, 1);
			led=true;
		}
		else{
			gpio_set_level(LED_B, 0);
			led=false;
		}
		vTaskDelay(pdMS_TO_TICKS(400));
		i+=1;
	}
}

void blink_2(void){
	int i=0;
	bool led=false;
	while(i<4){
		if(!led){
			gpio_set_level(LED_B, 1);
			led=true;
		}
		else{
			gpio_set_level(LED_B, 0);
			led=false;
		}
		vTaskDelay(pdMS_TO_TICKS(200));
		i+=1;
	}
}

void blink_3(void){
	int i=0;
	bool led=false;
	while(i<8){
		if(!led){
			gpio_set_level(LED_B, 1);
			led=true;
		}
		else{
			gpio_set_level(LED_B, 0);
			led=false;
		}
		vTaskDelay(pdMS_TO_TICKS(100));
		i+=1;
	}
}