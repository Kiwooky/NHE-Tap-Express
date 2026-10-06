#include <OneButton.h> // The OneButton Library
#include <jled.h> // The Jled Library

OneButton effectbutton = OneButton(
  7,        // Input pin for the button
  true,     // Button is active LOW
  true      // Enable internal pull-up resistor
);

#define THRESHOLD 30    // Threshold for the 'tempo pot' to overtake the tapped tempo
#define EXP1 A0         // Tempo pot attached to pin A0
#define MAXDELAY 3000   // Maximum delay time in Msec that can be achieved with the tempo pot
#define MINDELAY 1      // Maximum delay time in Msec that can be achieved with the tempo pot
#define MAXTAP 8000      // Maximum delay time in Msec that can be achieved with the tap tempo

//Declaring the names of the waveforms and pins
/* This will vary on how you set up the waveform selection. I've used a 6 position rotary switch that has
two pairs of common pins at opposite sides having 4 contacts each.(bit of a weird setup, but that's what I had lying around)
Should also work with a standard 2 pin 6 position rotary dial wired to 3 digital inputs. Each has a unique combination
of pin states. Pins are high by default and pulled to ground to set. Based on what type of waveform selection solution
you come up with you can theoretically have x amount of options as Jled allows for 'user function' waveforms :-)*/
auto ledtempo = JLed(6);      // This Led is always on and provides visual tempo feedback
auto ledon = JLed(10);        // Technically just 'always on' so not really a 'waveform'
auto ledsquare = JLed(10);    // 50% on 50% off
auto ledsine = JLed(10);      // Smooth cycle between 0% and 100%
auto ledrampup = JLed(10);    // Smooth cycle up to 100% for the duration of the tap, then start over
auto ledrampdown = JLed(10);  // Start at 100% with a smooth cycle down to 0%, then start over
auto ledrandom = JLed(10);    // Based on the Jled 'candle' setting, basically random values with user setting for 'jitter'

int period = 2000;            // Default start time for the tempo
int lastVal = 0;
int tempAnalog = 0;
boolean PotControls = true;   // Allows the tempo pot to take over the tapped tempo

// Variables will change:

int buttonPushCounter = 1;    // counter for the number of button presses
int doubleClickCounter = 0;   // counter for the number of doubleclicks
int buttonState = 0;          // current state of the button
int lastButtonState = 0;      // previous state of the button

void setup()
{
  
  effectbutton.attachClick(effectsingleclick);                  // link the function to be called on a singleclick event.
  effectbutton.attachDoubleClick(effectdoubleclick);            // link the function to be called on a doubleclick event.
  effectbutton.attachLongPressStart(effectlongpressstart);      // link the function to be called at the start of a longpress event.
  effectbutton.attachLongPressStop(effectlongpressstop);        // link the function to be called at the end of a longpress event.
  effectbutton.attachDuringLongPress(effectlongpress);          // link the function to be called during a longpress event.
  
  effectbutton.setClickTicks(200);   // Time for registering single Click
  effectbutton.setPressTicks(500);   // Time for registering Long Press
  
  pinMode(2, INPUT_PULLUP );         // Used for waveform selection with a two pin, 6 position rotary dial
  pinMode(3, INPUT_PULLUP );         // Used for waveform selection with a two pin, 6 position rotary dial
  pinMode(4, INPUT_PULLUP );         // Used for waveform selection with a two pin, 6 position rotary dial
  pinMode(5, INPUT_PULLUP );         // tap button - press it to set the tempo
  pinMode(6, OUTPUT);                // This is the LED showing the current tempo
  digitalWrite(8, LOW);              // Make sure this starts low to not trigger the Line6 effect on/off button
  pinMode(8, OUTPUT);                // This goes to Line6 Effect On/Off button
  digitalWrite(9, LOW);              // Make sure this starts low to not trigger the Line6 tap button
  pinMode(9, OUTPUT);                // This goes to Line6 Tap button
  pinMode(10, OUTPUT);               // This goes to the LED/LDR that controls the expression pedal
  digitalWrite(13, LOW);             // Make sure this starts low at startup
  pinMode(13, OUTPUT);               // LED that shows when doubleclick function is active
  
  buttonPushCounter = 1;              // counter for the number of button presses
  doubleClickCounter = 0;             // counter for the number of double clicks
  
  // initialize serial communication (for debugging):
  Serial.begin(9600);
 
}
int lastTapState = LOW;  /* the last tap button state */
unsigned long currentTimer[2] = { 500, 500 };  /* array of most recent tap counts */

void loop()
{
  /* read the button on pin 5, and only pay attention to the
     HIGH-LOW transition so that we only register when the
     button is first pressed down */
  buttonState = digitalRead(7);
  int tapState = digitalRead( 5 );
  if( tapState == LOW && tapState != lastTapState )
    {
      tap(); /* we got a HIGH-LOW transition, call our tap() function */
    }
  lastTapState = tapState; /* keep track of the state */
  
  ledtempo.Update();    // update leds
  delay(1);
  
  effectbutton.tick();  // Check the effect button for click/doubleclick/longpress events
  
  poll_pot();           // check pot value

  /* If the effect is switched on in latching mode, toggle between continuously updating the waveform
  or incrementing the counter when switched off so it's ready for the next event*/
  
  if (buttonPushCounter %2 == 0) {
    update_effect(); }
  else {
      buttonPushCounter = 1;
    }
}

unsigned long lastTap = 0; /* when the last tap happened */

void tap()
{
  /* we keep two of these around to average together later */
  currentTimer[1] = currentTimer[0];
  currentTimer[0] = millis() - lastTap;
  lastTap = millis();
 
  period = ((currentTimer[0] + currentTimer[1])/2);   // establish new period

  PotControls = false;   // override the pot when tapping a tempo

  update_leds();   // update leds period
}

  void update_leds(){   // Update the led's tempo information after tapping or setting the tempo pot
  
  ledtempo = JLed(6).Blink(period/20, period/20*19).Forever(),
  ledon = JLed(10).On().Forever();
  ledsquare = JLed(10).Blink(period/2, period/2).Forever();
  ledsine = JLed(10).Breathe(period).DelayAfter(0).Forever();
  ledrampup = JLed(10).FadeOn(period).DelayBefore(0).Forever();
  ledrampdown = JLed(10).FadeOff(period).DelayBefore(0).Forever();
  ledrandom = JLed(10).Candle(period/286 /*speed*/, 255 /* jitter*/);
}

void update_effect(){   // Keep the led waveform updated in the loop until it's turned off
  
   if (digitalRead(2) == LOW && digitalRead(3) == HIGH && digitalRead(4) == HIGH) { // Always on LED is selected
      ledon.Update(); 
   }
    if (digitalRead(2) == HIGH && digitalRead(3) == LOW && digitalRead(4) == HIGH) { // Random LED is selected
      ledrandom.Update(); 
   }
    if (digitalRead(2) == LOW && digitalRead(3) == HIGH && digitalRead(4) == LOW) { // Square wave LED is selected
      ledsquare.Update(); 
   }
    if (digitalRead(2) == HIGH && digitalRead(3) == LOW && digitalRead(4) == LOW) { // Sine wave LED is selected
      ledsine.Update(); 
   }
    if (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(4) == LOW) { // Ramp up LED is selected
      ledrampup.Update(); 
   }
    if (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(4) == HIGH) { // Ramp down LED is selected
      ledrampdown.Update(); 
   }
  
}

void poll_pot() {                   // Allow tempo pot reading to override tapped tempo

  tempAnalog = analogRead(EXP1);    // Read analog value
 
  // Convert 10 bit to milliseconds, 3 seconds maximum
  tempAnalog = map(tempAnalog, 0, 1023, 0, MAXDELAY);
  tempAnalog = constrain(tempAnalog, MINDELAY, MAXDELAY);
 
  // Check pot value and compare variation with threshold
  if((abs(tempAnalog-lastVal)>THRESHOLD))
  {
    // update period with pot value
    period = tempAnalog;
    //return control to Pot   
    PotControls = true;
    // Store current value to be used laters
    lastVal = tempAnalog;
    // update leds with new timing
    update_leds();
  }
}


// effect button actions

void effectsingleclick(){
  buttonPushCounter++;      // every time the effect turns on, increment the counter
  if (buttonPushCounter %2 == 0 && doubleClickCounter == 0 ) {  // Determines if the effect is currently off and is not in the double click function

   effectlongpressstart();  // Run through longpressstart in case 'ramp up' function is selected
   update_effect();         // Run the chosen waveform until the effect is turned off
  } 
/* If a doubleclick is active, make sure the led stays off and fire a momentary pulse 
(in my case to a CD4066 cmos switch) to switch the Line6 effect and tap buttons   
simultenously to trigger the preset select.*/
 else if (doubleClickCounter != 0 ) {  
  ledon.Stop();
  ledrandom.Stop();
  ledsquare.Stop();
  ledsine.Stop();
  ledrampup.Stop();
  ledrampdown.Stop();
  digitalWrite(8, !digitalRead(8));         // This goes to Line6 Effect On/Off button
  digitalWrite(9, !digitalRead(9));        // This goes to Line6 Tap button
  delay(100);
  digitalWrite(8, !digitalRead(8));         // This goes to Line6 Effect On/Off button
  digitalWrite(9, !digitalRead(9));        // This goes to Line6 Tap button
  }
/* If there's no doubleclick active, another single click will go to longpresstop,
in case there is a 'ramp down' selected.*/
  else {
    effectlongpressstop();
  }
  
    }

 void effectdoubleclick(){
/* When a double click is detected increment the counter, otherwise reset the 
counter to 0, effectively a toggle.*/
  if (doubleClickCounter %2 == 0) {
    doubleClickCounter++;
     Serial.print("double click on");
     Serial.println(doubleClickCounter);
  }
  else {
    doubleClickCounter = 0;
    Serial.print("reset the counter");
  }
  digitalWrite(13, !digitalRead(13));         // Toggle the indicator light for doubleclick on/off
}
   

void effectlongpressstart(){
  /* At the start of momentary or latching (they both cycle through longpresstart)
  Reset the LED so the timing starts from the moment you turn it on, and ramp up
  to the waveform if 'ramp up' is selected.*/
  ledon.Reset();
  ledrandom.Reset();
  ledsquare.Reset();
  ledsine.Reset();
  ledrampup.Reset();
  ledrampdown.Reset();
  // what happens when momentary effect engagement starts (ramp up)
 if (digitalRead(2) == LOW && digitalRead(3) == HIGH && digitalRead(4) == HIGH) { // if Always on LED is selected
  //Fading in the LED
  for(int i=0; i<255; i++){
    analogWrite(10, i);
    delay(period/64); //Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
  }
}
}

void effectlongpress(){
  Serial.println(buttonState);

if (digitalRead(2) == LOW && digitalRead(3) == HIGH && digitalRead(4) == HIGH) { // Always on LED is selected
  ledon.Update(); 
}
if (digitalRead(2) == HIGH && digitalRead(3) == LOW && digitalRead(4) == HIGH) { // Random LED is selected
  ledrandom.Update(); 
}
if (digitalRead(2) == LOW && digitalRead(3) == HIGH && digitalRead(4) == LOW) { // Square wave LED is selected
  ledsquare.Update(); 
}
if (digitalRead(2) == HIGH && digitalRead(3) == LOW && digitalRead(4) == LOW) { // Sine wave LED is selected
  ledsine.Update(); 
}
if (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(4) == LOW) { // Ramp up LED is selected
  ledrampup.Update(); 
}
if (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(4) == HIGH) { // Ramp down LED is selected
  ledrampdown.Update(); 
}

}

void effectlongpressstop(){
    /* At the end of momentary or latching (they both cycle through longpresstop)
  Increment the counter, and ramp down if 'ramp down' is selected.
  Then turn off the LED completely with a digitalwrite as sometimes it kept glowing. :-) */
  buttonPushCounter = 1;

  // what happens when momentary effect engagement ends (ramp down)
 if (digitalRead(2) == LOW && digitalRead(3) == HIGH && digitalRead(4) == HIGH) { // Always On LED is selected
  for(int i=255; i>0; i--){
    analogWrite(10, i);
    delay(10);  //Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
}
}
digitalWrite(10,LOW);  // Switch off the LED

} 
