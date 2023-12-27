#ifndef ESPBASIC_INC_H
#define ESPBASIC_INC_H

#ifndef SERIAL_PORT_SPEED
#define SERIAL_PORT_SPEED  115200
#endif

#define CONFIG_FILE "config.json"


#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <StreamString.h>
#include <utils.h>
#include <stringTemplates.h>

#ifdef ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
//#include <ESPAsyncWebServer.h>
#include <ESPAsyncWebSrv.h>
#include "FS.h"
#include <LittleFS.h>
#include "ESP32SSDP.h"



//headers
void endExec(DynamicJsonDocument &json, AsyncWebServerRequest *request);
void subExecScanWifi(AsyncWebServerRequest *request, void *outros);
void setupCore();
void loopCore( void * parameters );
void WiFiSTAMonitor();


char SSDP_MODEL[33] = "";
char SSDP_NAME[33] = "";
char HOST_NAME[33] = "";
char shared[512];
    
const char *reset_txt[] PROGMEM = {"POWER_ON", "HARD_WATCH_DOG", "SOFT_EXCEPTION", "SOFT_WATCH_DOG", "SOFT_RESTART", "WAKE_DEEP_SLEEP", "EXTERNAL_RESET"};
const char *wifistsdesc[] PROGMEM = {"IDLE_STATUS", "NO_SSID_AVAIL", "SCAN_COMPLETED", "CONNECTED", "CONNECT_FAILED", "CONNECTION_LOST", "WRONG_PASSWORD", "DISCONNECTED"};
bool _doReset = false, _useDNS = false;
int _staStatus=-1, _wifimon_ct=-2000;
TaskHandle_t loopCoreHanlde;
uint32_t otherCounter = 0;

DNSServer DNS;
AsyncWebServer server(80);

void setupCore() {
  Serial.begin(SERIAL_PORT_SPEED);
  while (!Serial) {
    ;
  }
  Serial.write(27);   // hack para reset de tela do terminal
  Serial.print("[2J"); // hack para reset de tela do terminal
  Serial.write(27);  // hack para reset de tela do terminal
  Serial.print("[H");  // hack para reset de tela do terminal
  delay(500);
  
  Serial.println("");
  Serial.println("PROTOCOM - www.protocom.tech.br");
  Serial.print("build ");
  Serial.print(__DATE__);
  Serial.print("-");
  Serial.println(__TIME__);
  Serial.println("");
  WiFi.disconnect();
  WiFi.softAPdisconnect();
  WiFi.mode(WIFI_OFF);

  if (!LittleFS.begin()) {
    return;
  }
  logger(OK, "Sistema de arquivos iniciado...");

  String mac = WiFi.softAPmacAddress().c_str();
  mac.replace(":", "");
  mac.toLowerCase();
  mac = mac.substring(4);

  DynamicJsonDocument config(768);
  if (!parseJSON(CONFIG_FILE, config)) {
    logger(INFO, "Criando config padrão.");
    config["wifi_mode"] = "3";
    config["sta_hostname"] = "PROTOCOM-" + mac;
    config["sta_ssid"] = "familia_rodrigues";
    config["sta_pass"] = "258456wifi";
    config["sta_dhcp"] = "1";
    config["sta_channel"] = "0";
    config["sta_ip"] = "0.0.0.0";
    config["sta_gateway_ip"] = "0.0.0.0";
    config["sta_subnet_ip"] = "0.0.0.0";
    config["sta_dns1_ip"] = "0.0.0.0";
    config["sta_dns2_ip"] = "0.0.0.0";
    config["ap_ssid"] = "PROTOCOM-" + mac;
    config["ap_pass"] = "";
    config["ap_ip"] = "192.168.25.1";
    File jsonFile = LittleFS.open(CONFIG_FILE, "w");
    serializeJsonPretty(config, jsonFile);
    jsonFile.close();
  } else {
	logger(INFO, "Utilizando config existente.");
  }
  JsonObject root = config.as<JsonObject>();
  for (JsonPair p : root) {
	  String key=p.key().c_str(), val=p.value().as<const char *>();
	  logger(INFO, key + ": " + val);
}

  char pout[512];
  memset(pout, 0, 512);


  int wifimode = config["wifi_mode"].as<unsigned int>();
  WiFi.mode(WiFiMode_t(wifimode));
  delay(500);
  if (wifimode == 1 or wifimode == 3) {
    logger(INFO, "AP Init.");
    IPAddress ip, gateway, subnet;
    ip.fromString(config["ap_ip"].as<const char *>());
    gateway = ip;
    subnet.fromString("255.255.255.0");
    WiFi.softAPConfig(ip, gateway, subnet);

    bool ap_ret;
    if (strlen(config["ap_pass"].as<const char *>()) < 8 ) {
      ap_ret = WiFi.softAP(config["ap_ssid"].as<const char *>());
    } else {
      ap_ret = WiFi.softAP(config["ap_ssid"].as<const char *>(), config["ap_pass"].as<const char *>());
    }
    //Serial.println(strlen(pass));

        
      memset(pout, 0, 512);
      sprintf(pout, "AP IP:%s GATEWAY:%s SUBNET:%s ATUAL_IP:%s AP_NAME:%s AP_PASS:%s",
              ip.toString().c_str(),
              gateway.toString().c_str(),
              subnet.toString().c_str(),
              WiFi.softAPIP().toString().c_str(), config["ap_ssid"].as<const char *>(),
              config["ap_pass"].as<const char *>());
      logger(INFO, pout);
       delay(100);

		DNS.stop();
		DNS.setErrorReplyCode(DNSReplyCode::NoError);
		_useDNS = DNS.start(53, "*", WiFi.softAPIP());
		logger((_useDNS) ? OK:ERRO, "AP Iniciando DNS.");
		logger(OK, "AP Configuração concluída");

  }


  if (wifimode == 2 or wifimode == 3)
  {
    logger(INFO, "STA Init.");
    memset(HOST_NAME, 0, 33);
    strncpy(HOST_NAME, config["sta_hostname"].as<const char *>(), 32);
    if (strlen(SSDP_NAME) == 0) {
      strcpy(SSDP_NAME, HOST_NAME);
    }
    if (strlen(SSDP_MODEL) == 0) {
      strcpy(SSDP_MODEL, HOST_NAME);
    }


    WiFi.setAutoReconnect(true);
    WiFi.hostname(HOST_NAME);
    /*
    if (!WiFi.setPhyMode(WIFI_PHY_MODE_11N)) {
      logger(INFO, "STA in B/G mode.");
    } else {
      logger(INFO, "STA in N mode.");
    }
*/
    bool sta_dhcp = config["sta_autocfg_bln"].as<bool>();
    if (!sta_dhcp)
    {
      IPAddress staip, gateip, subip, dns1ip, dns2ip;
      staip.fromString(config["sta_ip"].as<const char *>());
      gateip.fromString(config["sta_gateway_ip"].as<const char *>());
      subip.fromString(config["sta_subnet_ip"].as<const char *>());
      dns1ip.fromString(config["sta_dns1_ip"].as<const char *>());
      dns2ip.fromString(config["sta_dns2_ip"].as<const char *>());
      WiFi.config(staip, gateip, subip, dns1ip, dns2ip);

    }
    char stassid[33] = {};
    char stapass[64] = {};
    strncpy(stassid, config["sta_ssid"].as<const char *>(), 32);
    strncpy(stapass, config["sta_pass"].as<const char *>(), 63);
    int stachnl = config["sta_channel"].as<const int>();

    WiFi.begin(stassid, stapass, stachnl);
  //  os_timer_setfn(&wifimon, WiFiSTAMonitor, NULL);
  //  os_timer_arm(&wifimon, 3000, true);
    logger(INFO, "STA WifiMonitor started.");

  }

  /* SERVER CONFIG */
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");

  server.rewrite("/", "/index.html");
  server.rewrite("/generate_204", "/index.html");
  server.rewrite("/fwlink", "/index.html");
  server.serveStatic("/", LittleFS, "/").setDefaultFile("/index.html");
  server.on("/ssdp", HTTP_GET, [](AsyncWebServerRequest * request) {
    logger(INFO, "SSDP call");
    StreamString output;
    if (output.reserve(1024)) {
      //uint32_t ip = WiFi.localIP();
      uint32_t chipId = ESP.getEfuseMac();
      output.printf(ssdpTemplate,
                    WiFi.localIP().toString().c_str(),
                    HOST_NAME,
                    chipId,
                    SSDP_NAME,
                    SSDP_MODEL,
                    (uint8_t) ((chipId >> 16) & 0xff),
                    (uint8_t) ((chipId >>  8) & 0xff),
                    (uint8_t)   chipId        & 0xff
                   );
      request->send(200, "text/xml", (String)output);
    } else {
      request->send(500);
    }
  });

/*
  server.on(
    "/upload", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    uploadData *obj = (uploadData *)request->_tempObject;
    DynamicJsonDocument json(256);
    json["command"] = "upload";
    json["message"] = obj->message;
    json["type"] = (obj->isFirmware) ? "FIRMWARE" : "ARQUIVO";
    json["nome"] = obj->filename;
    endExec(json, request);
  },
  handleUpload);
*/
  server.onNotFound([](AsyncWebServerRequest * request)
  {
    if (request->method() == HTTP_OPTIONS)
    {
      logger(INFO,"Response OPTIONS for:" + request->url());
	  request->send(200);
    }else{
	  logger(INFO,"Response NOT FOUND for:" + request->url());
      request->send(404, "text/plain", "PROTOCOM 404 ->" + request->url());
    }

  });

    server.on("/setconfig", HTTP_ANY, [](AsyncWebServerRequest * request) {

    DynamicJsonDocument json(768);
    
    int args = request->args();
    for (int i = 0; i < args; i++) {
      json[request->argName(i).c_str()] = request->arg(i).c_str();
    }

    File jsonFile = LittleFS.open(CONFIG_FILE, "w");
    if (jsonFile) {
      serializeJsonPretty(json, jsonFile);
      jsonFile.close();
      json["message"] = "OK - Salvo.";
    } else {
      json["message"] = "ERRO - Erro salvando config.";
    }
    json["command"]="setconfig";
    endExec(json, request);
  });




  server.on("/files", HTTP_ANY, [](AsyncWebServerRequest * request) {
    //subExecListfiles(request, NULL);
  });
  server.on("/scanwifi", HTTP_ANY, [](AsyncWebServerRequest * request) {
    subExecScanWifi(request, NULL);
  });
  server.on("/delete", HTTP_ANY, [](AsyncWebServerRequest * request) {
    //subExecDelete(request, NULL);
  });

  server.on("/reboot", HTTP_ANY, [](AsyncWebServerRequest * request) {
	
	//subExecReboot(request, NULL);
    
  });
  server.on("/format", HTTP_ANY, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "Hello World! format");
  });
  
 server.on("/description.xml", HTTP_GET, [&](AsyncWebServerRequest *request) {
            request->send(200, "text/xml", SSDP.schema(false));
        });

// Cria uma tarefa que será executada na função Task1code(), com  prioridade 1 e execução no núcleo 0
//  xTaskCreatePinnedToCore(
//                    loopCore,   /* Função da tarefa */
//                    "loopCore",  /* nome da tarefa */
//                    10000,       /* Tamanho (bytes) */
//                    NULL,        /* parâmetro da tarefa */
//                    1,           /* prioridade da tarefa */
//                    &loopCoreHanlde,      /* observa a tarefa criada */
//                    0);          /* tarefa alocada ao núcleo 0 */  

}


void endExec(DynamicJsonDocument &json, AsyncWebServerRequest *request)
{
  if (!json["command"]) {
    json["command"] = "Não definido";
  }
  json["client"] = "Serial";
  json["source"] = "CLIENT_SERIAL";
  if (request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    json["source"] = "CLIENT_AP";
    if (ON_STA_FILTER(request)) {
      json["source"] = "CLIENT_STA";
    }
    json["client"] = request->client()->remoteIP().toString();
    serializeJsonPretty(json, *response);
    request->send(response);
  }


  bool hasdata = false;
  Serial.println("");
  Serial.println("-----------------------------------------------------------");
  Serial.printf("COMMAND: %s FROM: %s\r\n", json["command"].as<const char *>(), json["source"].as<const char *>());
  Serial.println("-----------------------------------------------------------");
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


void subExecScanWifi(AsyncWebServerRequest *request, void *outros)
{
    /*
  WiFi.scanNetworksAsync([request](int n) {
    DynamicJsonDocument json((n * 120) + 200);
    json["message"] = "OK - Busca wifi terminada.";
    json["command"] = "scanwifi";
    JsonArray list = json.createNestedArray("values");
    for (int i = 0; i < n; i++) {
      JsonArray flobj = list.createNestedArray();
      flobj.add(WiFi.SSID(i));
      lobj.add(getQuality(WiFi.RSSI(i)));
      flobj.add(WiFi.encryptionType(i));
      flobj.add(WiFi.channel(i));
      flobj.add(WiFi.RSSI(i));
      
    }
    endExec(json, request);
  });
  */
}



void loopCore( void * parameters ){
    for(;;){ // Loop infinito
      //   WiFiSTAMonitor();
     vTaskDelay(50);     
  }
}

void WiFiSTAMonitor()
{
    _wifimon_ct++;
    //Serial.println(_wifimon_ct);
   //   vTaskDelay(1000); 
    if(_wifimon_ct>=60){
    _wifimon_ct=0;
  int wifistatus = WiFi.status();
  if (_staStatus != wifistatus)
  {
   // char buf[250];
    //sprintf(shared, "STA Status wifi alterado para de %s para %s ", wifistsdesc[_staStatus], wifistsdesc[wifistatus]);
    logger(INFO, "Teste");
    _staStatus = wifistatus;
    if (wifistatus == WL_CONNECTED)
    {

     sprintf(shared, "STA WiFi conectado IP:%s SUBNET:%s GATEWAY:%s DNS(s):%s ,%s", WiFi.localIP().toString().c_str(), WiFi.subnetMask().toString().c_str(), WiFi.gatewayIP().toString().c_str(), WiFi.dnsIP().toString().c_str(), WiFi.dnsIP(1).toString().c_str());
      logger(INFO, shared);
  /*
      SSDP.setSchemaURL("ssdp");
      SSDP.setHTTPPort(80);
      SSDP.setManufacturer("PROTOCOM");
      SSDP.setManufacturerURL("https://protocom.tec.br");
      SSDP.setName("TESTE_NAME");
      SSDP.setURL("http://" + WiFi.localIP().toString() + "/index.html");
      SSDP.setModelName("TESTE_MODEL");
      SSDP.setModelNumber(String(ESP.getChipModel()));
      SSDP.setModelURL("https://protocom.tec.br/models/" + String("TESTE_MODEL"));
      SSDP.setSerialNumber(String("50524f544f434f4d-VALUE") );//+ String(ESP.getChipModel()));
      SSDP.setDeviceType("upnp:rootdevice");

      if (SSDP.begin()){
        Serial.println("Ssdp iniciado");
      }else{
        delay(500);
              if (SSDP.begin()){
        Serial.println("Ssdp retry ok");
      }else{
        Serial.println("Ssdp NAOOOOO iniciado");
      }
      
*/
  server.begin();
        //set schema xml url, nees to match http handler
        //"ssdp/schema.xml" if not set
        SSDP.setSchemaURL("description.xml");
        
        //set port
        //80 if not set
        SSDP.setHTTPPort(80);
        //set device name
        //Null string if not set
        SSDP.setName("Philips hue clone");
        //set Serial Number
        //Null string if not set
        SSDP.setSerialNumber("001788102201");
        //set device url
        //Null string if not set
        SSDP.setURL("index.html");
        //set model name
        //Null string if not set
        SSDP.setModelName("Philips hue bridge 2012");
        //set model description
        //Null string if not set
        SSDP.setModelDescription("This device can be controled by WiFi");
        //set model number
        //Null string if not set
        SSDP.setModelNumber("929000226503");
        //set model url
        //Null string if not set
        SSDP.setModelURL("http://www.meethue.com");
        //set model manufacturer name
        //Null string if not set
        SSDP.setManufacturer("Royal Philips Electronics");
        //set model manufacturer url
        //Null string if not set
        SSDP.setManufacturerURL("http://www.philips.com");
        //set device type
        //"urn:schemas-upnp-org:device:Basic:1" if not set
        SSDP.setDeviceType("rootdevice"); //to appear as root device, other examples: MediaRenderer, MediaServer ...
        //set server name
        //"Arduino/1.0" if not set
        SSDP.setServerName("SSDPServer/1.0");
        //set UUID, you can use https://www.uuidgenerator.net/
        //use 38323636-4558-4dda-9188-cda0e6 + 4 last bytes of mac address if not set
        //use SSDP.setUUID("daa26fa3-d2d4-4072-bc7a-a1b88ab4234a", false); for full UUID
        SSDP.setUUID("daa26fa3-d2d4-4072-bc7a");
        //Set icons list, NB: optional, this is ignored under windows
        SSDP.setIcons(  "<icon>"
                        "<mimetype>image/png</mimetype>"
                        "<height>48</height>"
                        "<width>48</width>"
                        "<depth>24</depth>"
                        "<url>icon48.png</url>"
                        "</icon>");
        //Set service list, NB: optional for simple device
        SSDP.setServices(  "<service>"
                           "<serviceType>urn:schemas-upnp-org:service:SwitchPower:1</serviceType>"
                           "<serviceId>urn:upnp-org:serviceId:SwitchPower:1</serviceId>"
                           "<SCPDURL>/SwitchPower1.xml</SCPDURL>"
                           "<controlURL>/SwitchPower/Control</controlURL>"
                           "<eventSubURL>/SwitchPower/Event</eventSubURL>"
                           "</service>");

        Serial.printf("Starting SSDP...\n");
        if(SSDP.begin()){
          Serial.println("SSDP started");
        }else{
          Serial.println("SSDP start ERROR");
        }

      }

    

    }
  }
}


#endif