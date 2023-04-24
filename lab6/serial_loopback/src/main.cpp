#include <Arduino.h>
#include <RadioLib.h>
#include <esp_timer.h>

#define LED 35

int counter = 0;
const uint64_t TIMER_PERIOD_US = 1000000;


HardwareSerial GpioSerial(2);

void IRAM_ATTR timerISR(void* arg) {
  GpioSerial.print('A');
  char byte = GpioSerial.read();
  Serial.println("RECV: " + String(byte));
}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello LoRa!");
  pinMode(LED, OUTPUT);

  GpioSerial.begin(115200, SERIAL_8N1, 21, 26);

  esp_timer_handle_t timer;
  esp_timer_create_args_t timer_args = {
      .callback = &timerISR,
      .arg = NULL,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "Hello LoRa Timer Printer"  //<whatever you want to name your timer>
  };

  esp_timer_create(&timer_args, &timer); 

  esp_timer_start_periodic(timer, TIMER_PERIOD_US);  
}

void loop() {
  //nothing to do here
}