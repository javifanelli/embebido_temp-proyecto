// Variables globales usadas
#ifndef VARIABLES_H
#define VARIABLES_H
int	CONTRAST = 15; // Contraste del display
char hum_char[10]; // valor de humedad en char para mostrar
char temp_char[10]; // valor de temperatura en char para mostrar
uint8_t mac[6]; // MAC del dispositivo ESP32
static char mac_str[18]; // MAC del dispositivo ESP32 en char
static char mac_short[13]; // MAC del dispositivo ESP32 en char sin los :
static char ip[16]; // IP del dispositivo ESP32
static char gw[16]; // IP del gateway de red
char formatted_time[20]; // Fecha y hora en char para mandar por MQTT
char pant_time[20]; // Fecha y hora en char para mostrar en display
static int8_t rssi = 0; // Potencia de la señal
char RSSI_CHAR[10]; // Potencia de la señal en char
bool net_con = false; // Variable global para almacenar el estado de la conexión WiFi
bool mqtt_state = false; // Variable para detectar el estado de conexión al servidor por MQTT
bool out_temp = false; // Variable para detectar el estado de la salida del triac
int out_luz = 0; // Variable para detectar el estado de la salida del dimmer
bool btn_enc = false; // Variable para detectar el botón pulsado
bool inc_enc = false; // Variable para detectar el giro horario del encoder
bool dec_enc = false; // Variable para detectar el giro antihorario del encoder
int level = 0; // Variable para detectar el nivel del menú
char tipo_disp[50]="Temperatura";
TickType_t xLastWakeTime;
#endif