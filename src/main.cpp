#include <Arduino.h>

void updateEncoder();
void sendIBUS(uint16_t ch1, uint16_t ch2, uint16_t ch3, uint16_t ch4);
void updateEncoder();
void sendIBUS(uint16_t ch1, uint16_t ch2, uint16_t ch3, uint16_t ch4);

const int pinA = 2; 
const int pinB = 3; 
const int gearShiftPin = 4; 
const int gasPin = A0;   // The B10K potentiometer
const int brakePin = A7; // The B100K potentiometer

volatile int steeringPosition = 1500; 

void setup() {
  Serial.begin(115200);
  
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(gearShiftPin, INPUT_PULLUP); 
  
  attachInterrupt(digitalPinToInterrupt(pinA), updateEncoder, CHANGE);
}

void loop() {
  // 1. Steering & Button Data
  int constrainedSteering = constrain(steeringPosition, 1000, 2000);
  bool isPressed = (digitalRead(gearShiftPin) == LOW);
  int buttonValue = isPressed ? 2000 : 1000;
  
  // 2. Read the raw Analog Pedals (0 to 1023)
  int rawGas = analogRead(gasPin);
  int rawBrake = analogRead(brakePin);
  
  // 3. Map the raw hardware values to the IBUS protocol range (1000 to 2000)
  int ibusGas = map(rawGas, 0, 1023, 1000, 2000);
  int ibusBrake = map(rawBrake, 0, 1023, 1000, 2000);

  // 4. Send all 4 channels
  sendIBUS(constrainedSteering, buttonValue, ibusGas, ibusBrake);
  
  delay(10); 
}

void updateEncoder() {
  if (digitalRead(pinA) != digitalRead(pinB)) {
    steeringPosition += 5; 
  } else {
    steeringPosition -= 5; 
  }
}

// The updated binary packet constructor
void sendIBUS(uint16_t ch1, uint16_t ch2, uint16_t ch3, uint16_t ch4) {
  uint8_t buf[32];
  
  buf[0] = 0x20; 
  buf[1] = 0x40; 
  
  // Channel 1: Steering
  buf[2] = ch1 & 0xFF; 
  buf[3] = (ch1 >> 8) & 0xFF;
  
  // Channel 2: Button
  buf[4] = ch2 & 0xFF;
  buf[5] = (ch2 >> 8) & 0xFF;

  // Channel 3: Gas Pedal (using bitwise shifts to split the 16-bit integer)
  buf[6] = ch3 & 0xFF;
  buf[7] = (ch3 >> 8) & 0xFF;
  
  // Channel 4: Brake Pedal
  buf[8] = ch4 & 0xFF;
  buf[9] = (ch4 >> 8) & 0xFF;

  // Fill the remaining unused channels (Channels 5 through 14) with 1500 (0x05DC)
  for (int i = 10; i < 30; i += 2) {
    buf[i] = 0xDC;
    buf[i+1] = 0x05;
  }
  
  // Calculate Checksum
  uint16_t checksum = 0xFFFF;
  for (int i = 0; i < 30; i++) {
    checksum -= buf[i];
  }
  
  buf[30] = checksum & 0xFF;
  buf[31] = (checksum >> 8) & 0xFF;

  Serial.write(buf, 32); 
}