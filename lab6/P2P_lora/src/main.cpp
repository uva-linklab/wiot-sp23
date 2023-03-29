/*
   To successfully receive data, the following settings have to be the same
   on both transmitter and receiver:
    - carrier frequency
    - bandwidth
    - spreading factor
*/

// include the library
#include <RadioLib.h>

/****************Pin assignment for the Heltec V3 board******************/
static const int LORA_CS    = 8;      // Chip select pin
static const int LORA_MOSI  = 10;     
static const int LORA_MISO  = 11;
static const int LORA_SCK   = 9;
static const int LORA_NRST  = 12;      // Reset pin
static const int LORA_DIO1  = 14;      // DIO1 switch
static const int LORA_BUSY  = 13; 
static const int BUTTON     = 0;

/****************LoRa parameters (you need to fill these params)******************/
// static const float FREQ = ;
// static const float BW = ;
// static const uint8_t SF = ;
// static const int8_t TX_PWR = 20;
// static const uint8_t CR = 5;
// static const uint8_t SYNC_WORD = (uint8_t)0x34;
// static const uint16_t PREAMBLE = 8;

/****************Payload******************/
String tx_payload = "Hello LoRa!";    // change this to something unique to your group/something you want to say to each other

/****************transceiver flags******************/
volatile bool tx_flag = false;
volatile bool rx_flag = false;

SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_NRST, LORA_BUSY);

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  // we got a packet, set the flag. However, this might cause the interrupt to be called when a transmission takes place too
  // luckily, there is a simple fix. check if the device just transmitted. if it did, then it is probably not the rx interrupt
  // if not, we know it is the rx interrupt and therefore can set the flag
    rx_flag = true;
}

void buttonISR(){
  tx_flag = true;
}

// It is important to remember that ISRs are supposed to be short and are mostly used for triggering flags. 
// doing Serial.print() might fail and cause device resets. This is because Serial.print() uses interrupt to read data, 
// but in an ISR, all other interrupts are suspended, and therefore the operation fails. 

// Helper function to print error messages
void error_message(const char* message, int16_t state) {
  Serial.printf("ERROR!!! %s with error code %d\n", message, state);
  while(true); // loop forever
}

void setup() {
  Serial.begin(115200);
  // Create a transmit interrupt when the button is pressed. 
  pinMode(BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonISR, FALLING);

  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  state = radio.setBandwidth(BW);
  if (state != RADIOLIB_ERR_NONE) {
      error_message("BW intialization failed", state);
  }
  state = radio.setFrequency(FREQ);
  if (state != RADIOLIB_ERR_NONE) {
      error_message("Frequency intialization failed", state);
  }
  state = radio.setSpreadingFactor(SF);
  if (state != RADIOLIB_ERR_NONE) {
      error_message("SF intialization failed", state);
  }
  state = radio.setOutputPower(TX_PWR);
  if (state != RADIOLIB_ERR_NONE) {
      error_message("Output Power intialization failed", state);
  }
 
  state = radio.setCurrentLimit(140.0);
  if (state != RADIOLIB_ERR_NONE) {
      error_message("Current limit intialization failed", state);
  }
   radio.setDio1Action(setFlag);    // callback when the RF interrupt is triggered

  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
}

void loop() {
  if (tx_flag){
    Serial.print(F("[SX1262] Transmitting packet ... "));

    // you can transmit C-string or Arduino string up to
    // 256 characters long
    // NOTE: transmit() is a blocking method!
    int state = radio.transmit(tx_payload);

    if (state == RADIOLIB_ERR_NONE) {
      // the packet was successfully transmitted
      Serial.println(F("success!"));

      // print measured data rate
      Serial.print(F("[SX1262] Datarate:\t"));
      Serial.print(radio.getDataRate());
      Serial.println(F(" bps"));

    } else if (state == RADIOLIB_ERR_PACKET_TOO_LONG) {
      // the supplied packet was longer than 256 bytes
      Serial.println(F("too long!"));

    } else if (state == RADIOLIB_ERR_TX_TIMEOUT) {
      // timeout occured while transmitting packet
      Serial.println(F("timeout!"));

    } else {
      // some other error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);
    }
    tx_flag = false;
    radio.startReceive();   // you can put a return value on this and check if the device was set to receive mode if needed
}
if(rx_flag) {
    // reset flag
    rx_flag = false;
    // you can read received data as an Arduino String
    String rx_data;
    int state = radio.readData(rx_data);

    if (state == RADIOLIB_ERR_NONE) {
      // packet was successfully received
      Serial.println(F("[SX1262] Received packet!"));

      // print data of the packet
      Serial.print(F("[SX1262] Data:\t\t"));
      Serial.println(rx_data);

      // print RSSI (Received Signal Strength Indicator)
      Serial.print(F("[SX1262] RSSI:\t\t"));
      Serial.print(radio.getRSSI());
      Serial.println(F(" dBm"));

      // print SNR (Signal-to-Noise Ratio)
      Serial.print(F("[SX1262] SNR:\t\t"));
      Serial.print(radio.getSNR());
      Serial.println(F(" dB"));

    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      Serial.println(F("CRC error!"));

    } else {
      // some other error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);
    }

    // put module back to listen mode
    radio.startReceive();
  }
}