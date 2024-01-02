#ifndef ESPBASIC_LOOP_H
#define ESPBASIC_LOOP_H
#include <Arduino.h>
#include <utils.h>
#include <variables.h>

#ifndef CUSTOM_FUNC_QT
#define CUSTOM_FUNC_QT 25
#endif

typedef void (*functionHolderPtr) ();
functionHolderPtr functionHolder[CUSTOM_FUNC_QT];
void addLoopFunction(functionHolderPtr obj, const char msg[64]);
void loopFunction();




#ifdef ESP32
TaskHandle_t basicLoopTask;
void basicLoopSched( void * pvParameters );
void basicLoop();

void basicLoopSched( void * pvParameters ){
 for(;;){
    loopFunction();
     vTaskDelay(100/portTICK_PERIOD_MS);
 }
}

void basicLoop(){
  xTaskCreatePinnedToCore(
                    basicLoopSched,   /* Função da tarefa */
                    "LoopSched",  /* nome da tarefa */
                    10000,       /* Tamanho (bytes) */
                    NULL,        /* parâmetro da tarefa */
                    1,           /* prioridade da tarefa */
                    &basicLoopTask,      /* observa a tarefa criada */
                    0);          /* tarefa alocada ao núcleo 0 */  
}

#elif defined(ESP8266)
void wifimon();
void wifimon(){

}

#endif

void addLoopFunction(functionHolderPtr obj, const char msg[64]){
for (int i = 0; i < CUSTOM_FUNC_QT ; i++) {
    if (functionHolder[i] == NULL) {
      functionHolder[i] = obj;
      logger(INFO, msg);
      return;
    }
  }

}

void loopFunction(){
    for (int i = 0; i < CUSTOM_FUNC_QT; i++){
		if (functionHolder[i] == NULL)
		{
			break;
		}
		functionHolder[i]();
	}
}

#endif