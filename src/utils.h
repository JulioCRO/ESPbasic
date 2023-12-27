#include <Arduino.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>

#define OK  " OK "
#define ERRO  "ERRO"
#define INFO  "INFO"


void logger(const char type[10], String msg);
bool parseJSON(String fileORjson, DynamicJsonDocument &json);





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
