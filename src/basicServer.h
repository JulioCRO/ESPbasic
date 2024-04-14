#ifndef ESPBASIC_SERVER_H
#define ESPBASIC_SERVER_H
#include <basicWifi.h>


void basicServer();
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void serviceAdd(const char* uri, ArRequestHandlerFunction onRequest);
void serviceAdd(const char *uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest);
void serviceAdd(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload);
void serviceAdd(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload, ArBodyHandlerFunction onBody);



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
//DynamicJsonDocument services(512);

void basicServer(){
  /* SERVER CONFIG */
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");

  server.rewrite("/", "/index.html");
  server.rewrite("/index.htm", "/index.html");
	server.rewrite("/generate_204", "/index.html");
	server.rewrite("/fwlink", "/index.html");

   serviceAdd("/service", HTTP_ANY, [&](AsyncWebServerRequest *request) {
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

    serviceAdd("/files", HTTP_ANY, [](AsyncWebServerRequest * request) {
    subExecFiles(request, NULL);
  });

    serviceAdd("/delete", HTTP_ANY, [](AsyncWebServerRequest * request) {
    subExecDelete(request,(void *) request->arg("file").c_str());
  });

    serviceAdd("/reboot", HTTP_ANY, [](AsyncWebServerRequest * request) {
	subExecReboot(request, NULL);
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
    DynamicJsonDocument json(256);
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
      /**
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (obj->total_sz > maxSketchSpace)
      {
        obj->message = "ERRO - FIRMWARE maior que o espaço disponível.";
        obj->hasError = true;
        return;
      }
      if (!Update.begin(maxSketchSpace))
      **/
     uint32_t maxSketchSpace = ESP.getFreeSketchSpace();
     /**
     #ifdef ESP32
     ESP.getFreeSketchSpace();
           maxSketchSpace = UPDATE_SIZE_UNKNOWN;
     #elif defined(ESP8266)
         //maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
         maxSketchSpace = ESP.getFreeSketchSpace();
    #endif
     **/
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
  serviceList += "\r\n";
 // services["values"]=uri;
  server.on(uri, method, onRequest);	
}

void serviceAdd(const char* uri, ArRequestHandlerFunction onRequest){
  serviceList += uri;
  serviceList += "\r\n";
  server.on(uri, onRequest);	
}
void serviceAdd(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload){
  serviceList += uri;
  serviceList += "\r\n";
  server.on(uri, method, onRequest,onUpload);	
}
void serviceAdd(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, ArUploadHandlerFunction onUpload, ArBodyHandlerFunction onBody){
  serviceList += uri;
  serviceList += "\r\n";
  server.on(uri, method, onRequest,onUpload,onBody);	
}
#endif