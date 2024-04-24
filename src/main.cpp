#include <ESPbasic.h>
#include <class/parameter.h>


void setup() {
/*defaul start*/    
startBasic();

//ADD http service
/*
serviceAdd("/service", HTTP_ANY, [&](AsyncWebServerRequest *request) {
            request->send(200, "text/html", serviceList );
});	
*/

Fahrenheit f;
f.c2f(10);

}

void loop() {
  // put your main code here, to run repeatedly:
}
