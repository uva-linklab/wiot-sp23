#include <Arduino.h>
#include <RadioLib.h>
#include <esp_timer.h>

#define LED 35

const uint64_t TIMER_PERIOD_US = 1000000;

void toggle_led(int count){
  for (int i = 0; i < count; i++){
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
  }
}

void IRAM_ATTR timerISR(void* arg) {
    // do something here
}


/*************************
 * 
 * The second argument for the timerBegin function is the prescaler value. In this case, we are using a prescaler of 1024, which lets us 
 * run the device at 16MHz. The base clock frequency is 15.625KHz. 
 * 
**************************/

void setup() {
  Serial.begin(115200);
  Serial.println("Hello LoRa!");

  esp_timer_handle_t timer;
  esp_timer_create_args_t timer_args = {
      .callback = &timerISR,
      .arg = NULL,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "<whatever you want to name your timer>"
  };
  esp_timer_create(&timer_args, &timer); 
   //esp_timer_start_periodic(<timer handle>, <period>);
  
  
}

void loop() {
  //nothing to do here
}