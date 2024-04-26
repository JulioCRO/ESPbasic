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
Serial.println(f.getValue("operacao","updated"));
f.printJson();


JsonHelper p;
p.openJSON("/cavaco.json");
p.setValue("teste","teste");
p.setValue("pppppp","ok");
p.printJson();
p.saveJson();


JsonHelper k;
k.openJSON("/wifi.json");
k.printJson();
k.saveJson("/wifi.json");

JsonHelper g;
g.openJSON("/wifi.json");
p.setValue("teste","teste");
g.printJson();
g.saveJson("/wifi2.json");

JsonHelper y;
y.openJSON("{\"key\":1}");
y.setValue("yyyyyyyyy","ok");
y.printJson();
y.saveJson("/teste.json");

}

void loop() {
  // put your main code here, to run repeatedly:
}
