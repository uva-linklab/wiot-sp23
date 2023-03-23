// prints the Heltec WiFi Lora ESP32's internal temperature
#include <Arduino.h>
#include "driver/temp_sensor.h"    

void initTempSensor(){
    temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
    temp_sensor.dac_offset = TSENS_DAC_L2; //TSENS_DAC_L2 is default, range: -10 to 80 degree celcius
    temp_sensor_set_config(temp_sensor);
    temp_sensor_start();
}

void setup() {
  Serial.begin(115200);
  initTempSensor();
}

void loop() {
    float temp = 0;
    temp_sensor_read_celsius(&temp); 
    Serial.println(temp);

    delay(1000);
}