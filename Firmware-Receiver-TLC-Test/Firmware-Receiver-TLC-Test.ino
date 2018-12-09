#include <Adafruit_TLC59711.h>

#define SERIALOUT 0
#define LED_BUILTIN 13

// SPI

#define dataPin   A4
#define clockPin  A3

#define NUM_TLC59711 2  // number of chained TLCs

Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, clockPin, dataPin);
const int actors[10] = {0, 1, 2, 3, 4, 12, 13, 14, 15, 16};

#define maxStrength 65535

void setup() {
  Serial.begin(9600);
  Serial.println("Start ");

  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  tlc.begin();
  tlc.write();

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  for (int i = 0; i < 10; i++) {
    Serial.print("TLC #");
    Serial.println(actors[i]);
    tlc.setPWM(actors[i], maxStrength);
    tlc.write();
    delay(500);
    tlc.setPWM(actors[i], 0);
    tlc.write();
  }
}

