#include <PinChangeInterrupt.h>

const byte channel_pin[] = {8, 9, 10};
volatile unsigned long rising_start[] = {0, 0, 0};
volatile long channel_length[] = {0, 0, 0};

void setup() {
  Serial.begin(57600);

  pinMode(channel_pin[0], INPUT);
  pinMode(channel_pin[1], INPUT);
  pinMode(channel_pin[2], INPUT);

  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[0]), onRising0, CHANGE);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[1]), onRising1, CHANGE);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[2]), onRising2, CHANGE);
}

void onRising0(void) {
  uint8_t trigger = getPinChangeInterruptTrigger(digitalPinToPCINT(channel_pin[0]));

  if(trigger == RISING) {
    rising_start[0] = micros();
  } else if(trigger == FALLING) {
    channel_length[0] = micros() - rising_start[0];
  }
  
}

void onRising1(void) {
  uint8_t trigger = getPinChangeInterruptTrigger(digitalPinToPCINT(channel_pin[1]));

  if(trigger == RISING) {
    rising_start[1] = micros();
  } else if(trigger == FALLING) {
    channel_length[1] = micros() - rising_start[1];
  }
  
}

void onRising2(void) {
  uint8_t trigger = getPinChangeInterruptTrigger(digitalPinToPCINT(channel_pin[2]));

  if(trigger == RISING) {
    rising_start[2] = micros();
  } else if(trigger == FALLING) {
    channel_length[2] = micros() - rising_start[2];
  }
  
}

void loop() {
  Serial.print(channel_length[0]);
  Serial.print(" | ");
  Serial.print(channel_length[1]);
  Serial.print(" | ");
  Serial.print(channel_length[2]);
  
  Serial.println("");
  delay(1000);
}
