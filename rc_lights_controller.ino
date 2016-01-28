#include <EEPROM.h>
#include <PinChangeInterrupt.h>

#define OUTPUT_CHANNELS 2
#define LPF_FACTOR 0.8
#define MAX_TICK 127
#define CHANGES_PER_CHANNEL 8
#define CYCLE_OFF 255
#define CYCLE_ON 0
#define BUTTON_PIN 0
#define EEPROM_BUTTON_MODE_ADDRESS 0

/*
 * Define pins used to provide RC PWM signal to Arduino
 * Pins 8, 9 are used since they work on both ATMega328 and 
 * ATMega32u4 board. So this code will work on Uno/Mini/Nano/Micro/Leonardo
 * See PinChangeInterrupt documentation for usable pins on other boards
 */
const byte channel_pin[OUTPUT_CHANNELS] = {2, 1};
volatile unsigned long rising_start[OUTPUT_CHANNELS] = {0, 0};
volatile unsigned int channel_length[OUTPUT_CHANNELS] = {0, 0};

const byte output_pin[OUTPUT_CHANNELS] = {3, 4};

/*
 * Do not use 0 or values above MAX_TICK
 * 255 no light never
 */
const byte patterns[9][OUTPUT_CHANNELS][CHANGES_PER_CHANNEL] = {
  {{CYCLE_OFF}, {CYCLE_OFF}}, //CH1 LOW, CH2 LOW or not connected
  {{9, 10, 19, 20, 29, 30, 39, 40}, {CYCLE_OFF}}, //CH1 MID, CH2 LOW or not connected
  {{CYCLE_ON}, {CYCLE_ON}}, //CH1 HIGH, CH2 LOW or not connected
  {{CYCLE_OFF}, {CYCLE_OFF}}, //CH1 LOW, CH2 MID
  {{5, 10, 15, 20, 25, 30, 35, 40}, {CYCLE_OFF}}, //CH1 MID, CH2 MID
  {{CYCLE_ON}, {CYCLE_OFF}}, //CH1 HIGH, CH2 MID
  {{CYCLE_OFF}, {CYCLE_OFF}}, //CH1 LOW, CH2 HIGH
  {{20,21,22,23,43,44,45,46}, {CYCLE_OFF}}, //CH1 MID, CH2 HIGH
  {{CYCLE_OFF}, {CYCLE_ON}} //CH1 HIGH, CH2 HIGH
};

const byte buttonModes[6] = {0, 1, 4, 7, 5, 8};
byte buttonMode = 0;
byte previousButtonState = HIGH;
byte currentButtonState = LOW;

void setup() {

  /*
   * read stored button mode from EEPROM
   */
  buttonMode = EEPROM.read(EEPROM_BUTTON_MODE_ADDRESS);
  if (buttonMode >= sizeof(buttonModes)) {
    buttonMode = 0;  
  }

  pinMode(channel_pin[0], INPUT);
  pinMode(channel_pin[1], INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(output_pin[0], OUTPUT);
  pinMode(output_pin[1], OUTPUT);
  pinMode(output_pin[2], OUTPUT);

  attachPinChangeInterrupt(channel_pin[0], onRising0, CHANGE);
  attachPinChangeInterrupt(channel_pin[1], onRising1, CHANGE);
}

int smooth(int data, float filterVal, float smoothedVal){

  if (filterVal > 1){ // check to make sure params are within range
    filterVal = .99;
  }
  else if (filterVal <= 0){
    filterVal = 0;
  }

  smoothedVal = (data * (1 - filterVal)) + (smoothedVal  *  filterVal);

  return (int)smoothedVal;
}

void processPin(byte pin) {
  uint8_t trigger = getPinChangeInterruptTrigger(channel_pin[pin]);

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

bool rcModeEnabled = false;

byte output_table[3] = {false, false, false};

byte pattern = 0;
byte patternPrevious = 0;
byte patternIndex[OUTPUT_CHANNELS] = {0, 0};

unsigned long tick[OUTPUT_CHANNELS] = {0, 0};

int currentPattern = 0;

void resetOutput() {

  for (int i = 0; i < OUTPUT_CHANNELS; i++) {
    output_table[i] = false;
    tick[i] = 0;
    patternIndex[i] = 0;
  }
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
    rcModeEnabled = true;
  } else {
    rcModeEnabled = false;
  }

  if (rcModeEnabled) {
    pattern = (3 * channelLengthToPosition(channel_length[1])) + channelLengthToPosition(channel_length[0]);
  } else {
    pattern = buttonModes[buttonMode];
  }
  if (pattern != patternPrevious) {
    resetOutput();
  }

  for (byte i = 0; i < OUTPUT_CHANNELS; i++) {
    currentPattern = patterns[pattern][i][patternIndex[i]];

    if (currentPattern == CYCLE_ON) {
      output_table[i] = true;
    } else if (tick[i] == currentPattern) {
      patternIndex[i]++;    
      output_table[i] = !output_table[i];
    }

    tick[i]++;

    if (patternIndex[i] == CHANGES_PER_CHANNEL) {
      tick[i] = 0;
      patternIndex[i] = 0;
    }

    if (tick[i] == MAX_TICK) {
      tick[i] = 0;
      patternIndex[i] = 0;
    }
  }

  if (!rcModeEnabled) {

    currentButtonState = digitalRead(BUTTON_PIN);

    if (currentButtonState == LOW && previousButtonState == HIGH) {
      buttonMode++;
      if (buttonMode >= sizeof(buttonModes)) {
        buttonMode = 0;
      }
      /*
       * Also, save current mode to EEPROM
       * Let's assume that few thousad writes is safe in this case. Like 100,000 ? A lot!
       */
       EEPROM.write(EEPROM_BUTTON_MODE_ADDRESS, buttonMode);
    }
    previousButtonState = currentButtonState;
  }

  patternPrevious = pattern;

  for (int i = 0; i < OUTPUT_CHANNELS; i++) {
    digitalWrite(output_pin[i], (byte) output_table[i]);
  }
  
  delay(100);
}
