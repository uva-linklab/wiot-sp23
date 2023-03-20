#include <Arduino.h> // Required for all code

void setup()
{
    // setup baud rate for serial output
    Serial.begin(115200); 
    delay(100);
    Serial.println("hello world!");
}

void loop()
{
    Serial.println("waiting for 5s");
    delay(5000);
    Serial.println("hello again");
}

