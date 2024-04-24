#include <ESPbasic.h>
#include <class/json.h>


void setup() {
/*defaul start*/    
startBasic();

//ADD http service
/*
serviceAdd("/service", HTTP_ANY, [&](AsyncWebServerRequest *request) {
            request->send(200, "text/html", serviceList );
});	
*/

delay(3000);
JsonHelper f;

f.setValue("teste","teste");
f.setValue("abacate","ok");
Serial.println(f.getValue("teste"));
f.printJson();
JsonHelper p;
p.openJSON("/cavaco.json",true);

JsonHelper k;
k.openJSON("/wifi.json",true);

}

void loop() {
  // put your main code here, to run repeatedly:
}
