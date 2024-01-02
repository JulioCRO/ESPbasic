/*
  uuid:50524f54-4f43-4f4d-9000-cda0e6%02x%02x%02x
  PROTOCOM HEX 50524f544f434f4d
*/

#ifndef ESPBASIC_WIFIMON_H
#define ESPBASIC_WIFIMON_H
#include <variables.h>
#include <utils.h>
#include <basicLoop.h>


#ifdef ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include "ESP32SSDP.h"
#include <DNSServer.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#endif
#include <ESPAsyncWebServer.h>

DNSServer DNS;


int _staStatus=8, _wifimon_ct=0;
bool _startAP=true, _useDNS=false;
void configWIFI();
void WiFiMonitor();
void startAP();
void stopAP();
String wifiStatus(int status);
String getMacAddr(int sz=12);


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
        SSDP.setModelDescription("PROTOCOM_CORE");
        //SSDP.setModelNumber("00000000001");
        //SSDP.setModelURL("http://www.meethue.com");
        SSDP.setManufacturer("PROTOCOM");
        SSDP.setManufacturerURL("https://protocom.tech.br");
        SSDP.setDeviceType("rootdevice"); //to appear as root device, other examples: MediaRenderer, MediaServer ...
        SSDP.setServerName("SSDPServer/1.0");
        SSDP.setUUID("50524f54-4f43-4f4d-9000-cda0e6");
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

void configWIFI(){
  logger("WIFI", "Iniciando...");
   strcat(UUIDCHAR,getMacAddr(6).c_str());
  DynamicJsonDocument config(768);
  if (!parseJSON(CONFIG_FILE, config)) {
    logger("WIFI", "Criando config padrão.");
    String def_sid_host = "protocom-" + getMacAddr(6);
    config["wifi_hostname"] = def_sid_host;
    config["sta_ssid"] = "familia_rodrigues";
    config["sta_pass"] = "258456wifi";
    config["sta_dhcp"] = "1";
    config["sta_channel"] = "0";
    config["sta_ip"] = "0.0.0.0";
    config["sta_gateway_ip"] = "0.0.0.0";
    config["sta_subnet_ip"] = "0.0.0.0";
    config["sta_dns1_ip"] = "0.0.0.0";
    config["sta_dns2_ip"] = "0.0.0.0";
    config["ap_pass"] = getMacAddr(8);
    config["ssdp_name"] = def_sid_host;
    config["ssdp_model"] = "default";
    File jsonFile = LittleFS.open(CONFIG_FILE, "w");
    serializeJsonPretty(config, jsonFile);
    jsonFile.close();
  } else {
	logger("WIFI", "Utilizando config existente.");
  }
  JsonObject root = config.as<JsonObject>();
  for (JsonPair p : root) {
	  String key=p.key().c_str(), val=p.value().as<const char *>();
	  logger(INFO, key + ": " + val);
}

// ############# WIFI SETUP ###############
WiFi.setAutoReconnect(false);
WiFi.mode(WIFI_STA); //default mode
if (WiFi.isConnected()){
  String msg="Desconectando do wifi:" + WiFi.SSID();
  WiFi.disconnect();
  logger("WIFI", msg);
}
//  WiFi.softAPdisconnect();

    strncpy(HOST_NAME, config["wifi_hostname"].as<const char *>(), 32);
    strncpy(SSDP_NAME, config["ssdp_name"].as<const char *>(), 64);
    strncpy(SSDP_MODEL, config["ssdp_model"].as<const char *>(), 64);
    strncpy(AP_PASS, config["ap_pass"].as<const char *>(), 32);
    
    WiFi.hostname(HOST_NAME);


  char pout[512]={0};
  
    logger("WIFI", "STA config.");
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
    char stassid[33] = {0};
    char stapass[64] = {0};
    strncpy(stassid, config["sta_ssid"].as<const char *>(), 32);
    strncpy(stapass, config["sta_pass"].as<const char *>(), 63);
    int stachnl = config["sta_channel"].as<const int>();
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

#endif


