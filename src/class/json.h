#ifndef ESPBASIC_JSONHELPER_H
#define ESPBASIC_JSONHELPER_H
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <StreamString.h>

class JsonHelper{
    public:
    JsonDocument  jsonDoc;

        JsonHelper() {
          DeserializationError error = deserializeJson(jsonDoc, "{}");
        };


        bool openJSON(const char* name, bool isFile=false){
          DeserializationError error;
          if(isFile){
              File jsonFile = LittleFS.open(name, "r");
              error = deserializeJson(jsonDoc, jsonFile);
              if(jsonFile){
                   jsonFile.close();
              }
              Serial.println(name);
              Serial.println(error.c_str());
          }else{

          }
        return true;
        }


        void setValue(const char* key, const char* value){
          jsonDoc[key]=value;
        }

        String getValue(const char* key){
          return jsonDoc[key];
        }

        void printJson(){
          serializeJsonPretty(jsonDoc,Serial);
        }

    private:
    bool _initialize = true;
    String _errdes = "";
};

#endif