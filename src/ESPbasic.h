#ifndef ESPBASIC_INC_H
#define ESPBASIC_INC_H

int testct=0;

#include <common.h>

#include <Arduino.h>
#include <LittleFS.h>
#include "FS.h"


#include <basicWifi.h>
#include <basicServer.h>


void startBasic();



void startBasic(){
//disableCore0WDT();
//disableCore1WDT();
//disableLoopWDT();
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
  LittleFS.format();

//********* CONFIG WIFI ***********
configWIFI();
basicServer();
basicLoop();
}



#endif