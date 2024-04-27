#ifndef ESPBASIC_INC_H
#define ESPBASIC_INC_H
/*
  uuid:50524f54-4f43-4f4d-9000-cda0e6%02x%02x%02x
  PROTOCOM HEX 50524f544f434f4d
*/

#include <common.h>
#include <utils/json.h>

void startBasic();
void initDevice();
void WiFiMonitor();
void startAP();
void stopAP();
String wifiStatus(int status);
String getMacAddr(int sz=12);
void scanWIFI(AsyncWebServerRequest *_request);
void scanWIFI_ESP32(void *pvParameters);
int getQuality(int i);

void basicServer();
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void serviceAdd(const char* uri, ArRequestHandlerFunction onRequest);
void serviceAdd(const char *uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest);
void serviceAdd(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload);
void serviceAdd(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload, ArBodyHandlerFunction onBody);




DNSServer DNS;
int _staStatus=8, _wifimon_ct=100;
bool _startAP=true, _useDNS=false, _check_scan=false;
AsyncWebServerRequest *scan_request;

struct uploadData
{
  String message = "";
  String client_addr = "";
  String filename = "";
  bool isFirmware = false;
  bool hasError = false;
  size_t total_sz = 0;
};
int _update_step=0;

String serviceList="";

AsyncWebServer server(80);


void startBasic(){
  Serial.begin(SERIAL_PORT_SPEED);
  while (!Serial) {
    ;
  }
  Serial.write(27);   // hack para reset de tela do terminal
  Serial.print("[2J"); // hack para reset de tela do terminal
  Serial.write(27);  // hack para reset de tela do terminal
  Serial.print("[H");  // hack para reset de tela do terminal
  delay(100);
  
  Serial.println("");
  Serial.println("PROTOCOM - www.protocom.tech.br");
  Serial.print("build ");
  Serial.print(__DATE__);
  Serial.print("-");
  Serial.println(__TIME__);
  Serial.println("");
  logger(OK, "Sistema iniciando.");
  logger(LittleFS.begin()?OK:ERRO, "Sistema de arquivos iniciado...");

initDevice();
basicServer();
internalLoop();
}

void initDevice(){
  logger("INIT", "Iniciando...");
  strcat(UUIDCHAR,getMacAddr(6).c_str());
  JsonHelper config;
  config.openJSON(CONFIG_FILE);
  String def_sid_host = "protocom-" + getMacAddr(6);
 
 // ############# WIFI SETUP ###############
WiFi.setAutoReconnect(false);
WiFi.mode(WIFI_STA); //default mode
if (WiFi.isConnected()){
  String msg="Desconectando do wifi:" + WiFi.SSID();
  WiFi.disconnect();
  logger("INIT", msg);
}
//  WiFi.softAPdisconnect();

    strncpy(HOST_NAME, config.getValue("wifi_hostname",def_sid_host).c_str(), 32);
    strncpy(SSDP_NAME, config.getValue("ssdp_name",def_sid_host).c_str(), 64);
    strncpy(SSDP_MODEL, config.getValue("ssdp_model","default").c_str(), 64);
    strncpy(AP_PASS, config.getValue("ap_pass",getMacAddr(8)).c_str(), 32);
    
    WiFi.hostname(HOST_NAME);
 
     logger("INIT", "WiFi STA config...");
     if (config.getValue("sta_dhcp","1") == "0"){

      IPAddress staip, gateip, subip, dns1ip, dns2ip;
      staip.fromString(config.getValue("sta_ip","0.0.0.0"));
      gateip.fromString(config.getValue("sta_gateway_ip","0.0.0.0"));
      subip.fromString(config.getValue("sta_subnet_ip","0.0.0.0"));
      dns1ip.fromString(config.getValue("sta_dns1_ip","0.0.0.0"));
      dns2ip.fromString(config.getValue("sta_dns2_ip","0.0.0.0"));
      WiFi.config(staip, gateip, subip, dns1ip, dns2ip);

    }
    char stassid[33] = {0};
    char stapass[64] = {0};
    strncpy(stassid, config.getValue("sta_ssid","familia_rodrigues").c_str(), 32);
    strncpy(stapass, config.getValue("sta_pass","258456wifi").c_str(), 63);
    int stachnl = config.getValue("sta_channel","0").toInt();
    WiFi.begin(stassid, stapass, stachnl);
    WiFi.setAutoReconnect(true);
    WiFi.waitForConnectResult();
  
    addLoopFunction(WiFiMonitor, "WiFiMonitor agendado.");
    
    if (!MDNS.begin(HOST_NAME)) {
       logger(ERRO, "Iniciando MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
 MDNS.addService("http", "tcp", 80);
 logger(OK, "MDNS iniciado http://" + String(HOST_NAME) + ".local/index.html");
config.saveJson();
}



void WiFiMonitor()
{
 if (_useDNS){
        DNS.processNextRequest();
 }

if (_wifimon_ct < 60){
    _wifimon_ct++;
    return;
}

_wifimon_ct=0;
  int wifistatus = WiFi.status();
  if (_staStatus != wifistatus)
  {
   String buf = "STA Status wifi alterado de ";
    buf += wifiStatus(_staStatus);
    buf += " para " ;
    buf += wifiStatus(wifistatus);
    logger("WIFI", buf);
    _staStatus = wifistatus;
    if (wifistatus == WL_CONNECTED)
    {
    char shared[512];
     sprintf(shared, "STA WiFi conectado IP:%s SUBNET:%s GATEWAY:%s DNS(s):%s ,%s", WiFi.localIP().toString().c_str(), WiFi.subnetMask().toString().c_str(), WiFi.gatewayIP().toString().c_str(), WiFi.dnsIP().toString().c_str(), WiFi.dnsIP(1).toString().c_str());
      logger("WIFI", shared);
        SSDP.setSchemaURL("description.xml");
        SSDP.setHTTPPort(80);
        SSDP.setName(SSDP_NAME);
       // SSDP.setSerialNumber("00000000001");
        SSDP.setURL("index.html");
        SSDP.setModelName(SSDP_MODEL);
       // SSDP.setModelNumber("00000000001");
        //SSDP.setModelURL("http://www.meethue.com");
        SSDP.setManufacturer("PROTOCOM");
        SSDP.setManufacturerURL("https://protocom.tech.br");
        SSDP.setServerName("SSDPServer/1.0");

        

     #ifdef ESP32
        SSDP.setUUID("50524f54-4f43-4f4d-9000-cda0e6");
        SSDP.setModelDescription("PROTOCOM_CORE");
        
        SSDP.setIcons(  "<icon>"
                        "<mimetype>image/png</mimetype>"
                        "<height>48</height>"
                        "<width>48</width>"
                        "<depth>24</depth>"
                        "<url>icon48.png</url>"
                        "</icon>");
           
        SSDP.setServices(  "<service>"
                           "<serviceType>urn:schemas-upnp-org:service:SwitchPower:1</serviceType>"
                           "<serviceId>urn:upnp-org:serviceId:SwitchPower:1</serviceId>"
                           "<SCPDURL>/SwitchPower1.xml</SCPDURL>"
                           "<controlURL>/SwitchPower/Control</controlURL>"
                           "<eventSubURL>/SwitchPower/Event</eventSubURL>"
                           "</service>");
     SSDP.setDeviceType("rootdevice"); //to appear as root device, other examples: MediaRenderer, MediaServer ...
     #elif defined(ESP8266)
      SSDP.setDeviceType("upnp:rootdevice"); //to appear as root device, other examples: MediaRenderer, MediaServer ...
     #endif
        SSDP.begin();
       //  MDNS.addService("http", "tcp", 80);
        logger("WIFI", "SSDP configurado.");
        stopAP();
      }else{
        startAP();
      }
    }
  }
//}

void startAP(){
if(_startAP){
logger("WIFI", "AP iniciando...");
WiFi.mode(WIFI_AP_STA);
_startAP=false;
    IPAddress ip, subnet;
    ip.fromString("172.217.28.1"); //fix captive
    subnet.fromString("255.255.255.0");
    WiFi.softAPConfig(ip, ip, subnet);
     if (strlen(AP_PASS) < 8 ) {
      WiFi.softAP(HOST_NAME);
    } else {
      WiFi.softAP(HOST_NAME, AP_PASS);
    }
char pout[256]={0};
      sprintf(pout, "AP IP:%s GATEWAY:%s SUBNET:%s ATUAL_IP:%s AP_NAME:%s AP_PASS:%s",
              ip.toString().c_str(),
              ip.toString().c_str(),
              subnet.toString().c_str(),
              WiFi.softAPIP().toString().c_str(),HOST_NAME,
              AP_PASS);
      logger("WIFI", pout);
       delay(100);
       DNS.stop();
		DNS.setErrorReplyCode(DNSReplyCode::NoError);
		_useDNS = DNS.start(53, "*", WiFi.softAPIP());
		logger((_useDNS) ? OK:ERRO, "AP Iniciando DNS.");
}

}

void stopAP(){
if(!_startAP){
logger("WIFI", "AP finalizando...");
DNS.stop();
_startAP=true;
_useDNS=false;
WiFi.mode(WIFI_STA);
}

}



String wifiStatus(int status){
  return wifistsdesc[status];
}

String getMacAddr(int sz){
  int idx=12 - sz;
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  return mac.substring(idx);

}

#ifdef ESP32
void scanWIFI(AsyncWebServerRequest *_request){
 xTaskCreate(scanWIFI_ESP32,    // Função a ser chamada
    "Scan WiFi",   // Nome da tarefa
    10000,            // Tamanho (bytes)
    (void*) _request,            // Parametro a ser passado
    1,               // Prioridade da Tarefa
    NULL             // Task handle
  );

}
void scanWIFI_ESP32(void *pvParameters){
  vTaskDelay(100/portTICK_PERIOD_MS); 
  AsyncWebServerRequest *a_request = (AsyncWebServerRequest *) pvParameters;
    
  WiFi.scanDelete(); 
  WiFi.scanNetworks(false);
    int  n = WiFi.scanComplete();
     if (n == WIFI_SCAN_FAILED){
      n=0;
    }
AsyncResponseStream *response = a_request->beginResponseStream("application/json");
    response->print("{\"message\":\"OK - Busca wifi terminada.\",\"command\":\"scanwifi\",\"values\":[");
    for (int i = 0; i < n; i++) {
        if(i > 0){ response->print(","); }
    response->printf("[\"%s\",%d,%d,%d,%d]",
    WiFi.SSID(i).c_str(),
    getQuality(WiFi.RSSI(i)),
    WiFi.encryptionType(i),
    WiFi.channel(i),
    WiFi.RSSI(i));
     }
     response->print("]}");
     a_request->send(response);
     WiFi.scanDelete(); 
     vTaskDelete(NULL);
}
#elif defined(ESP8266)
void scanWIFI(AsyncWebServerRequest *_request){
    WiFi.scanNetworksAsync([_request](int n) {
    JsonDocument json;
    json["message"] = "OK - Busca wifi terminada.";
    json["command"] = "scanwifi";
    JsonArray list =  json["values"].to<JsonArray>();
    for (int i = 0; i < n; i++) {
      JsonArray flobj = list.add<JsonArray>();
      flobj.add(WiFi.SSID(i));
      flobj.add(getQuality(WiFi.RSSI(i)));
      flobj.add(WiFi.encryptionType(i));
      flobj.add(WiFi.channel(i));
      flobj.add(WiFi.RSSI(i));
      
    }
    endExec(json, _request);
  });
}
#endif



int getQuality(int i)
{
  int dBm = i;
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

void basicServer(){
  logger("INIT","HTTP Server setup...");
  /* SERVER CONFIG */
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");

  server.rewrite("/", "/index.html");
  server.rewrite("/index.htm", "/index.html");
	server.rewrite("/generate_204", "/index.html");
	server.rewrite("/fwlink", "/index.html");

   serviceAdd("/service",  HTTP_GET | HTTP_POST, [&](AsyncWebServerRequest *request) {
            request->send(200, "text/plain", serviceList );
    });	

   serviceAdd("/index.html", HTTP_GET, [&](AsyncWebServerRequest *request) {
            request->send(200, "text/html", "<html><header></header><body><h1>INDEX<hr><form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form></h1></body></html>" );
    });	

  serviceAdd("/scanwifi", HTTP_GET, [&](AsyncWebServerRequest *request) {
            request->client()->setRxTimeout(60000); //fix for timeout
            scanWIFI(request);
        });	
	server.serveStatic("/", LittleFS, "/").setDefaultFile("/index.html");

    serviceAdd("/files",  HTTP_GET | HTTP_POST, [](AsyncWebServerRequest * request) {
    subExecFiles(request, NULL);
  });

    serviceAdd("/delete", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest * request) {
    subExecDelete(request,(void *) request->arg("file").c_str());
  });

    serviceAdd("/reboot",  HTTP_GET | HTTP_POST, [](AsyncWebServerRequest * request) {
	subExecReboot(request, NULL);
  });

  serviceAdd("/format",  HTTP_GET | HTTP_POST, [](AsyncWebServerRequest * request) {
	subExecFormat(request, NULL);
  });

  server.onNotFound([](AsyncWebServerRequest * request)
  {
    if (request->method() == HTTP_OPTIONS) {
      logger(INFO,"Response OPTIONS for:" + request->url());
	    request->send(200);
    }else{
	    logger(INFO,"Response NOT FOUND for:" + request->url());
      request->send(404, "text/plain", "PROTOCOM 404 ->" + request->url());
    }

  });
	

   serviceAdd("/description.xml", HTTP_GET, [&](AsyncWebServerRequest *request) {
            request->send(200, "text/xml", SSDP.getSchema());
 });

serviceAdd(
    "/upload", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    uploadData *obj = (uploadData *)request->_tempObject;
    JsonDocument json;
    json["command"] = "upload";
    json["message"] = obj->message;
    json["type"] = (obj->isFirmware) ? "FIRMWARE" : "ARQUIVO";
    json["nome"] = obj->filename;
    endExec(json, request);
  },
  handleUpload);

server.begin();
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  /*
    #define UPDATE_ERROR_OK                 (0)
    #define UPDATE_ERROR_WRITE              (1)
    #define UPDATE_ERROR_ERASE              (2)
    #define UPDATE_ERROR_READ               (3)
    #define UPDATE_ERROR_SPACE              (4)
    #define UPDATE_ERROR_SIZE               (5)
    #define UPDATE_ERROR_STREAM             (6)
    #define UPDATE_ERROR_MD5                (7)
    #define UPDATE_ERROR_MAGIC_BYTE         (8)
    #define UPDATE_ERROR_ACTIVATE           (9)
    #define UPDATE_ERROR_NO_PARTITION       (10)
    #define UPDATE_ERROR_BAD_ARGUMENT       (11)
    #define UPDATE_ERROR_ABORT              (12)
  */
  if (!index)
  {
    //yield();
   // FSInfo fs_info;
   // LittleFS.info(fs_info);
   // request->client()->setRxTimeout(60000); //fix for timeout
    request->_tempObject = new uploadData;
    uploadData *obj = (uploadData *)request->_tempObject;
    obj->message = "OK - Finalizado com sucesso.";
    obj->client_addr = request->client()->remoteIP().toString();
    obj->filename = filename;
    obj->isFirmware = filename.endsWith(".bin");
    obj->hasError = false;
    obj->total_sz = request->contentLength();
    char sysout[250] = "";
    sprintf(sysout, "Recebendo arquivo %s , cliente %s, tipo %s, tamanho %u", obj->filename.c_str(), obj->client_addr.c_str(), (obj->isFirmware) ? "FIRMWARE" : "ARQUIVO", obj->total_sz);
    logger(INFO, sysout);
    if (obj->isFirmware)
    {
     uint32_t maxSketchSpace = ESP.getFreeSketchSpace();
      if (!Update.begin(maxSketchSpace)){ 
        // start with max available size
        uint8_t updateerror = Update.getError();
        obj->hasError = true;
        obj->message = "ERRO - inciando atualização cod:" + String(updateerror);
        return;
      }
      else
      {
          #if defined(ESP8266)
          Update.runAsync(true);
         #endif
           ;;
      }
    }
    else
    {
     size_t usable=1000000;
    #ifdef ESP32
           usable =  (LittleFS.totalBytes() - LittleFS.usedBytes()); 
     #elif defined(ESP8266)
              FSInfo fs_info;
              LittleFS.info(fs_info);
              usable = (fs_info.totalBytes - fs_info.usedBytes); 
    #endif

     
      if (obj->total_sz > usable)
      {
        obj->message = "ERRO - espaço insuficiente para gravar o arquivo.";
        obj->hasError = true;
        return;
      }
      if (!filename.startsWith("/")){
        filename="/" + filename;
      }
      request->_tempFile = LittleFS.open(filename, "w");
      if (!request->_tempFile)
      {
        obj->message = "ERRO - criando o arquivo";
        obj->hasError = true;
        return;
      }
    }
  }
  if (len)
  {
    //yield();
    uploadData *obj = (uploadData *)request->_tempObject;
    if (obj->hasError)
    {
      return;
    }

    if (obj->isFirmware)
    {
      if (Update.write(data, len) != len)
      {
       String updateerror = "NOT_SET";
            #ifdef ESP32
            updateerror = Update.errorString();
            #elif defined(ESP8266)
            updateerror = Update.getErrorString();
            #endif
        
        obj->hasError = true;
        obj->message = "ERRO - aplicando atualização cod:" + updateerror;
        return;
      }
    }
    else
    {
      if (request->_tempFile.write(data, len) == 0)
      {
        request->_tempFile.close();
        LittleFS.remove(filename);
        obj->message = "ERRO - gravando o arquivo.";
        obj->hasError = true;
        return;
      }
    }
	
	int curr =  (index + len);
	int perc = (curr / (float)obj->total_sz ) * 100;
	//Serial.print(perc);
	//Serial.print("  ");
	//Serial.println(_update_step);
	if (perc > _update_step){
		Serial.print("\rEscrevendo :");
        Serial.print(curr);
        Serial.print(" de:");
        Serial.print(obj->total_sz);
		Serial.print(" (");
		_update_step =(_update_step + 5);
		Serial.print(_update_step);
		Serial.println("%)");
		
	}

    
  }

  if (final)
  {
    //yield();
    uploadData *obj = (uploadData *)request->_tempObject;
    if (!obj->hasError)
    { // no error
      if (obj->isFirmware)
      {
        if (!Update.end(true))
        { // true to set the size to the current progress
          uint8_t updateerror = Update.getError();
          obj->message = "ERRO - finalizando atualização cod:" + String(updateerror);
          obj->hasError = true;
        }
        else
        {
          obj->message = "OK - Atualização concluída.";
          obj->hasError = false;
          DO_ESP_RESET = true;
        }
      }
      else
      {
        if (request->_tempFile)
        {
          request->_tempFile.close();
          obj->message = "OK - Arquivo gravado com sucesso.";
          obj->hasError = false;
        }
      }
    }
  }
}

void serviceAdd(const char *uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest){
  serviceList += uri;
  logger(INFO,"Created endpoint for " + String(uri));
  serviceList += "\r\n";
 // services["values"]=uri;
  server.on(uri, method, onRequest);	
}

void serviceAdd(const char* uri, ArRequestHandlerFunction onRequest){
  serviceList += uri;
  logger(INFO,"Created endpoint for " + serviceList);
  serviceList += "\r\n";
  server.on(uri, onRequest);	
}
void serviceAdd(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload){
  serviceList += uri;
  logger(INFO,"Created endpoint for " + serviceList);
  serviceList += "\r\n";
  server.on(uri, method, onRequest,onUpload);	
}
void serviceAdd(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload, ArBodyHandlerFunction onBody){
  serviceList += uri;
  logger(INFO,"Created endpoint for " + serviceList);
  serviceList += "\r\n";
  server.on(uri, method, onRequest,onUpload,onBody);	
}


#endif


