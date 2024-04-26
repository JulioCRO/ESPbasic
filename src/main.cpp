#include <ESPbasic.h>
#include <utils/json.h>


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
p.openJSON("/cavaco.json");
p.setValue("teste","teste");
p.setValue("pppppp","ok");
p.printJson();


JsonHelper k;
k.openJSON("/wifi.json");
k.setValue("kkkkkkkkkk","ok");
k.printJson();

JsonHelper y;
y.openJSON("{\"key\":1}");
y.setValue("yyyyyyyyy","ok");
y.printJson();

}

void loop() {
  // put your main code here, to run repeatedly:
}
