#ifndef ESPBASIC_HEADER_H
#define ESPBASIC_HEADER_H
#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include "ESP32SSDP.h"
#include <DNSServer.h>
#include <Update.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#endif
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <LittleFS.h>

#include <ArduinoJson.h>
#include <StreamString.h>


#ifndef SERIAL_PORT_SPEED
#define SERIAL_PORT_SPEED  115200
#endif

#ifndef CUSTOM_FUNC_QT
#define CUSTOM_FUNC_QT 25
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
bool DO_ESP_RESET=false;

typedef void (*functionHolderPtr) ();
functionHolderPtr functionHolder[CUSTOM_FUNC_QT];
void addLoopFunction(functionHolderPtr obj, const char msg[64]);
void loopFunction();

int rmct=600;
void logger(const char type[10], String msg);
bool parseJSON(String fileORjson, DynamicJsonDocument &json);
void endExec(DynamicJsonDocument &json, AsyncWebServerRequest *request);
void getHeapStatus();
bool subExecListfiles(AsyncWebServerRequest *request, void *outros);


void logger(const char type[10], String msg) {
  Serial.printf("[%s] %s\r\n", type, msg.c_str());
}

bool parseJSON(String fileORjson, DynamicJsonDocument &json)
{
  DeserializationError error;
  bool _setError = false;
  char sysout[256];
  if (fileORjson.endsWith(".json"))
  {
    strcpy(sysout, "JSON processando arquivo: ");
    strcat(sysout, fileORjson.c_str());
    File jsonFile = LittleFS.open(fileORjson, "r");
    if (!jsonFile)
    {
      strcat(sysout, " - Falha abrindo arquivo.");
      _setError = true;
    }
    else
    {
      error = deserializeJson(json, jsonFile);
      jsonFile.close();
    }
  }
  else
  {
    strcpy(sysout, "JSON processando texto");
    error = deserializeJson(json, fileORjson);
  }
  if (!_setError)
  {
    switch (error.code())
    {
      case DeserializationError::Ok:
        strcat(sysout, " - OK.");
        break;
      case DeserializationError::InvalidInput:
        strcat(sysout, " - Conteúdo JSON inválido.");
        _setError = true;
        break;
      case DeserializationError::NoMemory:
        strcat(sysout, " - Memória insuficiente para JSON.");
        _setError = true;
        break;
      default:
        strcat(sysout, " - Ocorreu um erro: ");
        strcat(sysout, error.c_str());
        _setError = true;
        break;
    }
  }
  if (_setError) {
    logger(ERRO, sysout);
    deserializeJson(json, "{}"); //return non null object
  } else {
    logger(OK, sysout);
  }

  return !_setError;
}

void endExec(DynamicJsonDocument &json, AsyncWebServerRequest *request)
{
  Serial.println("enter");
  if (!json["command"]) {
    json["command"] = "Não definido";
  }
  json["client"] = "Serial";
  json["source"] = "CLIENT_SERIAL";
  if (request) {
    Serial.println("request");
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    Serial.println("End_request");
    json["source"] = "CLIENT_AP";
    Serial.println("End_source");
    /**
    if (ON_STA_FILTER(request)) {
      Serial.println("request filter");
     json["source"] = "CLIENT_STA";
    }
    **/
   Serial.println(request->client()->remoteIP().toString());
   json["client"] = request->client()->remoteIP().toString();
   Serial.println("Serialize ");
    serializeJsonPretty(json, *response);
    Serial.println("Send ");
    request->send(response);
  }
Serial.println("Passou daqui");

  bool hasdata = false;
  Serial.println("");
  //Serial.println("-----------------------------------------------------------");
 // Serial.printf("COMMAND: %s FROM: %s\r\n", json["command"].as<const char *>(), json["source"].as<const char *>());
 // Serial.println("-----------------------------------------------------------");
  JsonArray array = json["values"].as<JsonArray>();
  for (JsonVariant v : array) {
    hasdata = true;
    serializeJson(v, Serial);
    Serial.println("");
  }

  if (hasdata) {
    Serial.println("-----------------------------------------------------------");
  }
  Serial.println(json["message"].as<String>());
  return;
}

void getHeapStatus(){
  if (rmct < 60){
    rmct++;
    return;
  }
  rmct=0;
  Serial.println("Free   \tmaxblk\tminimum\tsize");
  Serial.print(ESP.getFreeHeap());
  Serial.print("\t");
  Serial.print(ESP.getMaxAllocHeap());
  Serial.print("\t");
  Serial.print(ESP.getMinFreeHeap());
  Serial.print("\t");
  Serial.println(ESP.getHeapSize());
}




#ifdef ESP32
TaskHandle_t basicLoopTask;
void basicLoopSched( void * pvParameters );
void basicLoop();

void basicLoopSched( void * pvParameters ){
 for(;;){
    loopFunction();
     vTaskDelay(100/portTICK_PERIOD_MS);
 }
}

void basicLoop(){

  addLoopFunction([](){
if(DO_ESP_RESET){
    delay(500);
   #ifdef ESP32
      ESP.restart();
     #elif defined(ESP8266)
       ESP.reset();
  #endif
}
},"ESP reset agendado");

  xTaskCreatePinnedToCore(
                    basicLoopSched,   /* Função da tarefa */
                    "LoopSched",  /* nome da tarefa */
                    10000,       /* Tamanho (bytes) */
                    NULL,        /* parâmetro da tarefa */
                    1,           /* prioridade da tarefa */
                    &basicLoopTask,      /* observa a tarefa criada */
                    0);          /* tarefa alocada ao núcleo 0 */  
}

#elif defined(ESP8266)
void wifimon();
void wifimon(){

}

#endif

void addLoopFunction(functionHolderPtr obj, const char msg[64]){
for (int i = 0; i < CUSTOM_FUNC_QT ; i++) {
    if (functionHolder[i] == NULL) {
      functionHolder[i] = obj;
      logger(INFO, msg);
      return;
    }
  }

}

void loopFunction(){
    for (int i = 0; i < CUSTOM_FUNC_QT; i++){
		if (functionHolder[i] == NULL)
		{
			break;
		}
		functionHolder[i]();
	}
}


bool subExecListfiles(AsyncWebServerRequest *request, void *outros){
  DynamicJsonDocument json(512);
  if (LittleFS.begin()){
    json["message"] = "OK - Lista de arquivos.";
    
    File dir = LittleFS.open("/");
    //Dir dir = LittleFS.openDir("");
    int i = 0;
    //char str[10];
    File file = dir.openNextFile();
    JsonArray list = json.createNestedArray("values");
    while (file){
      JsonArray flobj = list.createNestedArray();
      //Serial.println(file.name());
      char name[64]={0};
      strcpy(name,file.name());
      flobj.add(name);
      flobj.add(file.size());
      i++;
      file = dir.openNextFile();
    }

    if (i == 0) {
      json["message"] = "OK - Sem arquivos.";
    }
  }else  {
    json["message"] = "ERRO - Falha listando arquivos";
  }
  json["command"]="files";
  endExec(json, request);
  return true;

}

#endif