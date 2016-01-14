#include <PinChangeInterrupt.h>

#define OUTPUT_CHANNELS 3
#define LPF_FACTOR 0.8
#define MAX_TICK 127
#define CHANGES_PER_CHANNEL 8

/*
 * Define pins used to provide RC PWM signal to Arduino
 * Pins 8, 9 are used since they work on both ATMega328 and 
 * ATMega32u4 board. So this code will work on Uno/Mini/Nano/Micro/Leonardo
 * See PinChangeInterrupt documentation for usable pins on other boards
 */
const byte channel_pin[OUTPUT_CHANNELS] = {8, 9};
volatile unsigned long rising_start[OUTPUT_CHANNELS] = {0, 0};
volatile unsigned int channel_length[OUTPUT_CHANNELS] = {0, 0};

const byte output_pin[OUTPUT_CHANNELS] = {10, 16, 14};

/*
 * Use -1 to light it always
 * 255 no light never
 */
int patterns[9][CHANGES_PER_CHANNEL] = {
  {255}, //CH1 LOW, CH2 LOW or not connected
  {9, 10, 19, 20, 29, 30, 39, 40}, //CH1 MID, CH2 LOW or not connected
  {-1}, //CH1 HIGH, CH2 LOW or not connected
  {255}, //CH1 LOW, CH2 MID
  {5, 10, 15, 20, 25, 30, 35, 40}, //CH1 MID, CH2 MID
  {-1}, //CH1 HIGH, CH2 MID
  {255}, //CH1 LOW, CH2 HIGH
  {20,21,22,23,43,44,45,46}, //CH1 MID, CH2 HIGH
  {-1} //CH1 HIGH, CH2 HIGH
};

//{9, 10, 19, 20, 29, 30, 39, 40},
//{5, 10, 15, 20, 25, 30, 35, 40},
//{20,21,22,23,43,44,45,46},

void setup() {
  Serial.begin(57600);

  pinMode(channel_pin[0], INPUT);
  pinMode(channel_pin[1], INPUT);

  pinMode(output_pin[0], OUTPUT);
  pinMode(output_pin[1], OUTPUT);
  pinMode(output_pin[2], OUTPUT);

  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[0]), onRising0, CHANGE);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(channel_pin[1]), onRising1, CHANGE);
  
}

int smooth(int data, float filterVal, float smoothedVal){

  if (filterVal > 1){      // check to make sure params are within range
    filterVal = .99;
  }
  else if (filterVal <= 0){
    filterVal = 0;
  }

  smoothedVal = (data * (1 - filterVal)) + (smoothedVal  *  filterVal);

  return (int)smoothedVal;
}

void processPin(byte pin) {
  uint8_t trigger = getPinChangeInterruptTrigger(digitalPinToPCINT(channel_pin[pin]));

  if(trigger == RISING) {
    rising_start[pin] = micros();
  } else if(trigger == FALLING) {
    unsigned int val = micros() - rising_start[pin];
    channel_length[pin] = smooth(val, LPF_FACTOR, channel_length[pin]);
  }
}

void onRising0(void) {
  processPin(0);
}

void onRising1(void) {
  processPin(1);
}

bool masterSwitch = false;

byte output_table[3] = {false, false, false};

byte pattern = 2;
byte patternPrevious = 0;
byte patternIndex = 0;

unsigned int tick = 0;

int currentPattern = 0;

void resetOutput() {
  output_table[0] = false;
  output_table[1] = false;
  output_table[2] = false;
  tick = 0;
  patternIndex = 0;
}

byte channelLengthToPosition(unsigned int channelLength) {

  if (channelLength < 1250 || channelLength > 2400) {
    return 0;
  } else if (channelLength < 1750) {
    return 1;
  } else {
    return 2;
  }
  
}

void loop() {

  if (channel_length[0] > 600 && channel_length[0] < 2400) {
    masterSwitch = true;
  } else {
    masterSwitch = false;
  }

  if (masterSwitch) {

    pattern = (3 * channelLengthToPosition(channel_length[1])) + channelLengthToPosition(channel_length[0]);

    if (pattern != patternPrevious) {
      resetOutput();
    }
  
    currentPattern = patterns[pattern][patternIndex];

    if (currentPattern == -1) {
      for (int i = 0; i < sizeof(output_table); i++) {
        output_table[i] = true;
      }
    } else if (tick == currentPattern) {
      patternIndex++;
  
      for (int i = 0; i < sizeof(output_table); i++) {
        output_table[i] = !output_table[i];
      }
      
    }

    tick++;

    if (patternIndex == CHANGES_PER_CHANNEL) {
      tick = 0;
      patternIndex = 0;
    }

    if (tick == MAX_TICK) {
      tick = 0;
      patternIndex = 0;
    }
  }

  patternPrevious = pattern;

  for (int i = 0; i < OUTPUT_CHANNELS; i++) {
    digitalWrite(output_pin[i], (byte) output_table[i]);
  }
  
  delay(100);
}
