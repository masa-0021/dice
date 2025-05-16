#include "LedControl.h"
#include "TimerOne.h"
#include <Arduino.h>

// #define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are
// defined as blank.
#ifdef DEBUG // Macros are usually in all capital letters.
#define DPRINT(...) Serial.print(__VA_ARGS__) // DPRINT is a macro, debug print
#define DPRINTLN(...)                                                          \
  Serial.println(__VA_ARGS__) // DPRINTLN is a macro, debug print with new line
#else
#define DPRINT(...)   // now defines a blank line
#define DPRINTLN(...) // now defines a blank line
#endif

// Time difference between edge up / edge down in ms - measured
const int dT_up_down = 56;

// Time difference between edge down / edge up in ms - measured
const int dT_down_up = 44;

// To simplify implementation, take minimum
const int TWait = (int)((float)min(dT_up_down, dT_down_up) / 2.5f);

// Indication for given [us] before end of dialing is discovered
const uint32_t waitDialing[2] = {100U, 10U};

// Used input pin
const uint8_t pin = 2U;

// Display number
const uint8_t DISP_ADDR = 0U;

void check_phone();
void displayDice(int no);

uint8_t state;
// Counts edge risings during dialing
volatile uint8_t cntrUp = 0U;
// State of dialing: 0 = no dialing, 1 = dialing, 2 = dialing just ended
volatile uint8_t isDialing = 0U;

// How many
volatile uint16_t waitCntr = 0U;

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn
 pin 11 is connected to the CLK
 pin 10 is connected to LOAD
 We have only a single MAX72XX.
 */
LedControl lc = LedControl(12, 11, 10, 1);
unsigned long delaytime = 250;
byte numberDisplay = 0;

void setup() {
  Serial.begin(9600U); // This  pipes to the serial monitor

  // Read initial state of the pin
  pinMode(pin, INPUT);
  state = digitalRead(pin);

  // Start timer and check phone switch status only every TWait [ms]
  Timer1.initialize(TWait * 1000UL);
  Timer1.attachInterrupt(check_phone);

  /*
 The MAX72XX is in power-saving mode on startup,
  we have to do a wakeup call
 */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 8);
  /* and clear the display */
  lc.clearDisplay(0);

  /*
  int seed = 0;
  for( int i=2; i<3; ++i) {
    seed += analogRead(i);
  }
  randomSeed(seed);
}

void loop() {
  if( isDialing == 2U ) {
    isDialing = 0U;
    Serial.println("Up");
    Serial.println(cntrUp);
    // Since dialing '0' is indicated by 10 edge risings -> convert 10 to 0
    //numberDisplay = (uint8_t)(cntrUp > 9U ? 0U : cntrUp);
    numberDisplay = (uint8_t)(cntrUp > 8U ? 8U : cntrUp);
    cntrUp = 0U;
    // Display output was unstable without delay
    displayDice(numberDisplay);
  }
}

void displayDice(int no) {
  lc.clearDisplay(0);
  for( int i = 0; i < no; ++i ) {
    lc.setDigit(DISP_ADDR, i, random(1,7), 1);
    delay(100);
  }
}


void check_phone() {
  uint8_t newState = digitalRead(pin);
  if (state != newState) {
    if (newState == LOW && isDialing == 0U) {
      isDialing = 1U;
      waitCntr = 0U;
    }
    else if( newState == HIGH && isDialing == 1U) {
      cntrUp++;
      waitCntr = 0U;
    } else {
    }

    state = newState;
  }
    
  if ( waitCntr >= waitDialing[ cntrUp > 0U ? 1U : 0U ] && newState == HIGH ) {
    isDialing = 2U;
    waitCntr = 0U;
  }

  if( isDialing ) {
    waitCntr++;
  }
    
  DPRINTLN("isDialing");
  DPRINTLN(isDialing);
  DPRINTLN("waitcntr");
  DPRINTLN(waitCntr);
}
