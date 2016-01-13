#include <PinChangeInterrupt.h>

#define LPF_FACTOR 0.8

/*
 * Define pins used to provide RC PWM signal to Arduino
 * Pins 8, 9 are used since they work on both ATMega328 and 
 * ATMega32u4 board. So this code will work on Uno/Mini/Nano/Micro/Leonardo
 * See PinChangeInterrupt documentation for usable pins on other boards
 */
const byte channel_pin[] = {8, 9};
volatile unsigned long rising_start[] = {0, 0};
volatile unsigned int channel_length[] = {0, 0};

const byte output_pin[] = {10, 16, 14};

/*
 * Use -1 to light it always
 * 255 no light never
 */
#define MAX_TICK 127
#define CHANGES_PER_CHANNEL 8
int patterns[3][CHANGES_PER_CHANNEL] = {{9, 10, 19, 20, 29, 30, 39, 40},{5, 10, 15, 20, 25, 30, 35, 40},{15,16,17,18,33,34,35,36}};
//5, 10, 15, 20, 25, 30, 35, 40
//15,16,17,18,33,34,35,36


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

/*
 * modes:
 * 0 - blinking according to CH2 input
 * 1 - constant light
 */
byte mode = 0;
byte modePrevious = 0;
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

void loop() {
//  Serial.print(channel_length[0]);
//  Serial.print(" | ");
//  Serial.print(channel_length[1]);
//  Serial.println("");

  if (channel_length[0] > 1300 && channel_length[0] > 900 && channel_length[0] < 2400) {
    masterSwitch = true;
  } else {
    masterSwitch = false;
  }

  if (masterSwitch) {

    if (channel_length[0] > 1750) {
      mode = 1;
    } else {
      mode = 0;
    }

    if (channel_length[1] < 1300) {
      pattern = 0;
    } else if (channel_length[1] < 1750) {
      pattern = 1;
    } else {
      pattern = 2;
    }

    if (mode != modePrevious || pattern != patternPrevious) {
      resetOutput();
    }

    /*
     * Process mode
     */
    if (mode == 1) {
      output_table[0] = true;
      output_table[1] = true;
      output_table[2] = true;
      tick = 0;
      patternIndex = 0;
    } else {
    
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

    modePrevious = mode;
    patternPrevious = pattern;
  } else {
    resetOutput();
  }

  for (int i = 0; i < sizeof(output_table); i++) {
    digitalWrite(output_pin[i], (int) output_table[i]);
  }
  
  delay(100);
}
