#ifndef ESPBASIC_JSONHELPER_H
#define ESPBASIC_JSONHELPER_H
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <StreamString.h>

class JsonHelper{
    public:
    JsonDocument  jsonDoc;
    String parseError="";

        JsonHelper() {
          //DeserializationError error = deserializeJson(jsonDoc, "{}");
        };

       bool openJSON(String obj){
          DeserializationError error;
          if(obj.indexOf("{") > -1){
            error = deserializeJson(jsonDoc, obj);
            parseError = "Parse string " + String(error.c_str());
          }else{
                    parseError = "Parse file[" + obj + "] ";
                   File jsonFile = LittleFS.open(obj, "r");
                     error = deserializeJson(jsonDoc, jsonFile);
                    parseError += (!jsonFile)?"not exists.": error.c_str();
                     if(jsonFile){
                          jsonFile.close();
                     }              
                    
          }
          logger("JSON", parseError) ;
        return !error;
        }


        void setValue(const char* key, const char* value){
          jsonDoc[key]=value;
        }

        String getValue(const char* key){
          return jsonDoc[key];
        }

        void printJson(){
          serializeJsonPretty(jsonDoc,Serial);
          Serial.println("");
        }

    private:
    bool _initialize = true;
    String _errdes = "";
};

#endif