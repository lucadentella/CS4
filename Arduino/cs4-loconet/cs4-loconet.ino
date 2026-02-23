#include <LocoNet.h>

// Number of channels (default 4 = one CS4 module)
const uint8_t NUM_CH = 4;

// Arduino Pins connected to each channel
const uint8_t inputPins[NUM_CH] = {2, 3, 4, 5};

// Loconet Sensor ID for each channel
const uint16_t sensorIDs[NUM_CH] = {1, 2, 3, 4};

// Sensitivity: number of millisecond an input must be stable to send an update
unsigned long debounceMs = 500;

// TX pin for Loconet (my Loconet shield/interfaces use pin 7)
const uint8_t LOCONET_TX_PIN = 7; 

// Channel information
struct Channel {
  
  uint8_t  pin;
  uint16_t id;

  bool     rawNow;          // last read value
  bool     rawStable;       // last stable value
  unsigned long lastChange;   // time of the last change
};
Channel ch[NUM_CH];

void setup() {
  
  Serial.begin(115200);
  Serial.println(F("cs4-loconet"));
  Serial.println("");

  LocoNet.init(LOCONET_TX_PIN);

  // channel configuration
  for (uint8_t i = 0; i < NUM_CH; i++) {
    ch[i].pin = inputPins[i];
    ch[i].id  = sensorIDs[i];
    pinMode(ch[i].pin, INPUT);

    // read initial status
    bool r = digitalRead(ch[i].pin);
    ch[i].rawNow    = r;
    ch[i].rawStable = r;
    ch[i].lastChange  = millis();

    // send initial status on the loconet bus
    bool active = !ch[i].rawStable;
    LocoNet.reportSensor(ch[i].id, active);
    Serial.print(F("[INIT] Sensor ")); Serial.print(ch[i].id);
    Serial.print(F(" → ")); Serial.println(active ? F("ACTIVE") : F("INACTIVE"));
  }

  Serial.println(F("ready, monitoring channels..."));
}

void loop() {
  
  const unsigned long now = millis();

  // read new channel values
  for (uint8_t i = 0; i < NUM_CH; i++) {
    
    bool r = digitalRead(ch[i].pin);

    // if value has changed, update the rawNow value and the timestamp
    if (r != ch[i].rawNow) {
      ch[i].rawNow = r;
      ch[i].lastChange = now;
    }

    // if channel value is "stable" (has been constant for more than debounceMs), send it to the loconet bus
    if ((now - ch[i].lastChange) >= debounceMs) {

      if (ch[i].rawStable != ch[i].rawNow) {
        ch[i].rawStable = ch[i].rawNow;

        bool active = !ch[i].rawStable;
        LocoNet.reportSensor(ch[i].id, active);

        Serial.print(F("[LN] Sensor ")); Serial.print(ch[i].id);
        Serial.print(F(" → ")); Serial.println(active ? F("ACTIVE") : F("INACTIVE"));
      }
    }
  }

  lnMsg *rx = LocoNet.receive();
  if (rx) {
    // add debug messages...
  }
}