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
                   _filename = obj;
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

        void setValue(const char* key, String value){
          _isChanged = true;
          jsonDoc[key]=value;
        }

        String getValue(const char* key,String defValue = ""){
          bool keynotfound = !jsonDoc.containsKey(key);
          if (keynotfound){
            jsonDoc[key]=defValue;
             _isChanged = true;
             }
          char sysout[250] = "";
          sprintf(sysout, "Key %s value:%s%s", key,jsonDoc[key].as<const char *>(), (keynotfound) ? " - ERROR key not found(using default).":" - OK");
          logger("JSON", sysout);
          return jsonDoc[key];
        }

        bool saveJson(String obj = ""){
          if(obj == ""){
            obj = _filename;
          }
          if(!_isChanged){
            logger("JSON", "No changes to save.");
            return true;
          }
          File jsonFile = LittleFS.open(obj, "w");
          if (!jsonFile){
            logger("JSON", "Error opening file to save.");
            return false;
          }
          serializeJson(jsonDoc, jsonFile);
          jsonFile.close();
          logger("JSON", "OK File saved.");
          return true;
        }

        void printJson(){
          serializeJsonPretty(jsonDoc,Serial);
          Serial.println("");
        }

    private:
    bool _initialize = true;
    String _filename="";
    bool _isChanged = false;
};

#endif