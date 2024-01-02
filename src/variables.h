#ifndef ESPBASIC_VARIABLES_H
#define ESPBASIC_VARIABLES_H
#include <Arduino.h>
#ifndef SERIAL_PORT_SPEED
#define SERIAL_PORT_SPEED  115200
#endif


#define OK  " OK "
#define ERRO  "ERRO"
#define INFO  "INFO"

#define CONFIG_FILE "/wifi.json"

const char *reset_txt[] PROGMEM = {"POWER_ON", "HARD_WATCH_DOG", "SOFT_EXCEPTION", "SOFT_WATCH_DOG", "SOFT_RESTART", "WAKE_DEEP_SLEEP", "EXTERNAL_RESET"};
const char *wifistsdesc[] PROGMEM = {"IDLE_STATUS", "NO_SSID_AVAIL", "SCAN_COMPLETED", "CONNECTED", "CONNECT_FAILED", "CONNECTION_LOST", "WRONG_PASSWORD", "DISCONNECTED","WIFI_INIT"};

char SSDP_MODEL[65] = {0};
char SSDP_NAME[65] = {0};
char HOST_NAME[33] = {0};
char AP_PASS[33] = {0};
char UUIDCHAR[38]="50524f54-4f43-4f4d-9000-cda0e6";


#endif