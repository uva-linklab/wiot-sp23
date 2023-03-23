#include <Arduino.h> // Required for all code
#include <WiFi.h> // Required for all wifi code

#include "heltec.h"


void setup()
{
    // Set WiFi to station mode and disconnect from an AP if it was previously connected.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // Initialize display and serial. 
    Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void loop()
{
    Serial.println("Scan start");

    // WiFi.scanNetworks will return the number of networks found.
    int n = WiFi.scanNetworks();
    Serial.println("Scan done");
    if (n == 0) {
        Serial.println("no networks found");

        Heltec.display->clear();
        Heltec.display->drawStringMaxWidth(0, 0, 128, "no networks found.");
        Heltec.display->display();
    } else {

        Heltec.display->clear();
        Heltec.display->drawStringMaxWidth(0, 0, 128, "WiFi Networks");
        Heltec.display->display();

        Serial.print(n);
        Serial.println(" Networks found:");

        for (int i = 0; i < n; ++i) {

            // Format network names for the OLED display.
            int column = i / 5;
            int row = i % 5;

            char ssid_name[100];
            uint16_t width;
            uint16_t char_count;
            int ssid_len;

            ssid_len = strlen(WiFi.SSID(i).c_str());
            for (char_count=5; char_count<ssid_len; char_count++) {
                width = Heltec.display->getStringWidth(WiFi.SSID(i).c_str(), char_count);

                if (width > 63) {
                    char_count = char_count - 1;
                    break;
                }
            }

            memcpy(ssid_name, WiFi.SSID(i).c_str(), char_count);
            ssid_name[char_count] = '\0';

            // Display network names on the screen until the screen is full.
            if (column < 2) {
                Heltec.display->drawStringMaxWidth(column * 64, (row+1) * 10, 63, ssid_name);
                Heltec.display->display();
            }

            // Also print on serial.
            Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
            delay(1);
        }
    }

    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();

    // Wait a bit before scanning again.
    delay(5000);
}

