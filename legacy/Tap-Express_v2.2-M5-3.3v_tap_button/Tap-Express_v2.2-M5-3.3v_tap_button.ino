/* Tap Express is an Arduino Nano based "expression pedal input modulator"
The goal is to use LFO based automation to feed into the expression pedal input
for a (usually) digital multi-effect. Specifically developed for the Line6 M5 and M9
but either directly compatible or adaptable for many other expression inputs.
You might need a different value for the expression pot that runs parallel to the LDR output.*/

#include <OneButton.h>            // The OneButton Library
#include <jled.h>                 // The Jled Library

// Naming and setting the pins
/* If you're using different pins then an Arduino Nano (I'm using a chinese Arduino Nano clone "DCCduino")
You only have to change pin numbers here, all further instances will go back here. Just make sure the pins
have the same capabilities (digital, analog, pwm, etc) as the ones currently specified.*/

const int effectbutton = 12;   // Footswitch to turn the effect on and off (and with 'shift function' active, switch to the Line6 preset screen.)
const int tapbutton = 8;       // Footswitch tap tempo input
const int effectled = 10;      // The output driving the LED/LDR
const int previewled = 9;      // Shows the selected waveform continuously
const int tempoled = 7;        // The Led providing visual tempo feedback
const int tempopot = A0;       // The connection for the manual tempo pot (10kB, middle lug to the pin, with ground and 5v either side)

/* I've included 3 inputs for waveform selection, which (if you have the right switching setup) 
gives you 8 selectable presets. However, if using a 'shift function' (one extra input) this doubles to 16.
A future version is planned with some 'single shot' presets that might use such a setup.
Pins are high by default and pulled to ground to set.*/

const int waveselect1 = 4;      // First input line for waveform selection
const int waveselect2 = 5;      // Second input line for waveform selection
const int waveselect3 = 6;      // Third input line for waveform selection

const int line6effect = 16;      // This DOES NOT go straight to the Line6 footswitch (as it uses 3.3v logic) first to a CD4066 that pulls the Line6 footswitch LOW
const int line6tap = 11;         // This DOES NOT go straight to the Line6 footswitch (as it uses 3.3v logic) first to a CD4066 that pulls the Line6 footswitch LOW

/* You can use the shift function for the above mentioned expanded presets (from 8 to 16) 
if you're not using the footswitch to select presets on a Line6 M5 or M9. Will need the appropriate coding, which is currently in development*/
const int shiftfunction = 13;   // Double tapping the effect footswitch engages the 'shift function' and lights the shift LED to show it is active

//Declaring the waveforms
/* This will vary on how you set up the waveform selection. Based on what type of waveform selection solution
you come up with you can theoretically have x amount of options as Jled is extremely flexible when it comes
to generating waveforms :-) check out: https://github.com/jandelgado/jled */

auto ledtempo = JLed(tempoled);      // This Led is always on and provides visual tempo feedback
auto ledon = JLed(effectled);        // Technically just 'always on' so not really a 'waveform'
auto ledsquare = JLed(effectled);    // 50% on 50% off
auto ledsine = JLed(effectled);      // Smooth cycle between 0% and 100%
auto ledrampup = JLed(effectled);    // Smooth cycle up to 100% for the duration of the tap, then start over
auto ledrampdown = JLed(effectled);  // Start at 100% with a smooth cycle down to 0%, then start over
auto ledrandom = JLed(effectled);    // Based on the Jled 'candle' setting, basically random values with user setting for 'jitter'

//Declaring the waveform previews
/* Mimics the previous but on a different pin so you can preview the waveform before turning on the effect */

auto previewon = JLed(previewled);        // Technically just 'always on' so not really a 'waveform'
auto previewsquare = JLed(previewled);    // 50% on 50% off
auto previewsine = JLed(previewled);      // Smooth cycle between 0% and 100%
auto previewrampup = JLed(previewled);    // Smooth cycle up to 100% for the duration of the tap, then start over
auto previewrampdown = JLed(previewled);  // Start at 100% with a smooth cycle down to 0%, then start over
auto previewrandom = JLed(previewled);    // Based on the Jled 'candle' setting, basically random values with user setting for 'jitter'

//Defining values for the tap tempo functions

#define THRESHOLD 30    // Threshold for the 'tempo pot' to overtake the tapped tempo. 20 is default, increase if too glitchy
#define MAXDELAY 3000   // Maximum delay time in Msec that can be achieved with the tempo pot
#define MINDELAY 10     // Maximum delay time in Msec that can be achieved with the tempo pot
#define MAXTAP 8000     // Maximum delay time in Msec that can be achieved with the tap tempo


//Declaring the 'Onebutton' footswitch
OneButton effect = OneButton(
  effectbutton,   // Input pin for the button
  true,           // Button is active LOW
  true            // Enable internal pull-up resistor
);

// Variables that will change when running:

int buttonPushCounter = 0;    // counter for the number of button presses
int doubleTapCounter = 0;     // counter for the number of doubletaps (heh, Zombieland! :-)
int LowActiveLED = false;     // Inverts the LED to LowActive when true, so resistance on the output is '0' (zero) when effect is off
int rampvalue = 0;            // store how bright the LED
int rampupfactor = 4;         // Sets how many taps (beats/quarter notes) does it take to ramp up the led
int rampuptime = 31;          // Determines how many taps (beats/quarter notes) it takes to ramp up the led
int rampdownfactor = 4;       // Sets how many taps (beats/quarter notes) does it take to ramp up the led
int rampdowntime = 31;        // Determines how many taps (beats/quarter notes) it takes to ramp up the led
int cancelrampup = false;     // used to detect if the effect has been cancelled during ramp up
int cancelrampdown = false;   // used to detect if the effect has been cancelled during ramp down
int waslongpressed = true;    // used to detect if the effect was engaged as momentary or latching
int lastTapState = LOW;       // the last tap button state
int period = 2000;            // Default start time for the tempo
int lastVal = 0;              // Initialise the tap tempo button
int tempAnalog = 0;           // Initialise the manual tempo pot
boolean PotControls = true;   // Allows the tempo pot to take over the tapped tempo
unsigned long currentTimer[2] = { 500, 500 };  // array of most recent tap counts
unsigned long lastTap = 0;    // when the last tap happened */

//volatile int pwm_value = 0;
//volatile int prev_time = 0;

void setup()
{  
  // Binding functions to what happens to the effectbutton
  effect.attachClick(effectsingleclick);                  // link the function to be called on a singleclick event.
  effect.attachDoubleClick(effectdoubleclick);            // link the function to be called on a doubleclick event.
  effect.attachLongPressStart(effectlongpressstart);      // link the function to be called at the start of a longpress event.
  effect.attachLongPressStop(effectlongpressstop);        // link the function to be called at the end of a longpress event.
  effect.attachDuringLongPress(effectlongpress);          // link the function to be called during a longpress event.

  // Set timings for the effectbutton, you can tweak these to get it working just how you like it
  effect.setClickTicks(200);   // Time for registering single click. 200 works well but if your double clicks aren't reliable increase it a bit (start with 100 increment) 
  effect.setPressTicks(500);   // Time for registering Long Press. 500 (half a second) is good enough for me, but see if shorter still works for you)

  // Set pin modes for all the pins, and pull them high or low at the start if needed
  pinMode(waveselect1, INPUT_PULLUP );         // Used for waveform selection with a two pin, 6 position rotary dial
  pinMode(waveselect2, INPUT_PULLUP );         // Used for waveform selection with a two pin, 6 position rotary dial
  pinMode(waveselect3, INPUT_PULLUP );         // Used for waveform selection with a two pin, 6 position rotary dial
  pinMode(tapbutton, INPUT_PULLUP);           // tap button - press it to set the tempo
  pinMode(tempoled, OUTPUT);                   // This is the LED showing the current tempo
  pinMode(previewled, OUTPUT);                 // This is the LED showing the waveform preview
  digitalWrite(line6effect, LOW);              // Make sure this starts low to not trigger the Line6 effect on/off button
  pinMode(line6effect, OUTPUT);                // This goes to Line6 Effect On/Off button
  digitalWrite(line6tap, LOW);                 // Make sure this starts low to not trigger the Line6 tap button
  pinMode(line6tap, OUTPUT);
  if (LowActiveLED == false)
    {
    digitalWrite(effectled, LOW);               // Start with LED/LDR off to simulate pot on '10' (toe down)
    }
  else if (LowActiveLED == true)
    {
    digitalWrite(effectled, HIGH);               // Start with LED/LDR on to simulate pot on '0' (toe up)
    }
  pinMode(effectled, OUTPUT);                  // This goes to the LED/LDR that controls the expression pedal
  digitalWrite(shiftfunction, LOW);            // Make sure this starts low at startup
  pinMode(shiftfunction, OUTPUT);              // LED that shows when doubleclick function is active
  

Serial.begin(115200);

}

void loop()
{
  /* read the button on pin 5, and only pay attention to the
     HIGH-LOW transition so that we only register when the
     button is first pressed down */
  int tapState = digitalRead( tapbutton );
  if( tapState == LOW && tapState != lastTapState )
    {
      tap(); /* we got a HIGH-LOW transition, call our tap() function */
    }
  lastTapState = tapState; /* keep track of the state */
  
  ledtempo.Update();    // update the tempo led continously
  update_preview();     // update the preview led continously
  delay(1);
  
  effect.tick();  // Check the effect button for click/doubleclick/longpress events
  
  poll_pot();           // check pot value

  /* If the effect is switched on in latching mode, toggle between continuously updating the waveform
  or resetting the counter when switched off so it's ready for the next event*/
  
  if (buttonPushCounter %2 != 0 && doubleTapCounter %2 == 0) 
    {
     update_effect(); 
    }
  else 
    {
     buttonPushCounter = 0;
    }
}
