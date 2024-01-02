#ifndef ESPBASIC_INC_H
#define ESPBASIC_INC_H

int testct=0;

#include <Arduino.h>
#include <LittleFS.h>
#include "FS.h"
#include <StreamString.h>

#include <variables.h>
#include <utils.h>
#include <basicWifi.h>
#include <basicLoop.h>

AsyncWebServer server(80);

void startBasic();



void startBasic(){

  Serial.begin(SERIAL_PORT_SPEED);
      Serial.println("PROTOCOM - www.protocom.tech.br");
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
  LittleFS.format();

//********* CONFIG WIFI ***********
configWIFI();

    /* SERVER CONFIG */
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");


  server.rewrite("/", "/index.html");
	
	server.rewrite("/generate_204", "/index.html");
	
	server.rewrite("/fwlink", "/index.html");

   server.on("/index.html", HTTP_GET, [&](AsyncWebServerRequest *request) {
            request->send(200, "text/html", "<h1>INDEX</h1>" );
        });	
	server.serveStatic("/", LittleFS, "/").setDefaultFile("/index.html");

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
	

 server.on("/description.xml", HTTP_GET, [&](AsyncWebServerRequest *request) {
            request->send(200, "text/xml", SSDP.getSchema());
        });

server.begin();
basicLoop();
}



#endif