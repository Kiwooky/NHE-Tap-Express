#include <OneButton.h>
#include <jled.h>

OneButton effectbutton(A2, true);                         //attach a button on pin A2 to the library

#define THRESHOLD 20
#define EXP1 A0
#define MAXDELAY 3000
#define MINDELAY 1
#define MAXTAP 8000

int period = 2000;

//auto ledtempo = JLed(6).Blink(period/2, period/2).Forever();
auto ledtempo = JLed(6);
auto ledon = JLed(10).On().Forever();
auto ledsquare = JLed(10).Blink(period/2, period/2).Forever();
auto ledsine = JLed(10).Breathe(period).DelayAfter(0).Forever();
auto ledrampup = JLed(10).FadeOn(period).DelayBefore(0).Forever();
auto ledrampdown = JLed(10).FadeOff(period).DelayBefore(0).Forever();
auto ledrandom = JLed(10).Candle(period/286 /*speed*/, 255 /* jitter*/);

int lastVal = 0;
int tempAnalog = 0;
boolean PotControls = true; 

void setup()
{
  
  effectbutton.attachClick(effectsingleclick);                  // link the function to be called on a singleclick event.
  effectbutton.attachLongPressStart(effectlongpressstart);            // link the function to be called on a longpress event.
  effectbutton.attachLongPressStop(effectlongpressstop);            // link the function to be called on a longpress event.
  effectbutton.attachDuringLongPress(effectlongpress);

  effectbutton.setClickTicks(100);
  effectbutton.setPressTicks(500);

  pinMode(2, INPUT_PULLUP );
  pinMode(3, INPUT_PULLUP );
  pinMode(4, INPUT_PULLUP );
  pinMode(5, INPUT_PULLUP );   /* tap button - press it to set the tempo */
  pinMode(6, OUTPUT);                              // sets the digital pin as output
  pinMode(10, OUTPUT);                              // sets the digital pin as output
  
}


int lastTapState = LOW;  /* the last tap button state */
unsigned long currentTimer[2] = { 500, 500 };  /* array of most recent tap counts */

void loop()
{
  /* read the button on pin 12, and only pay attention to the
     HIGH-LOW transition so that we only register when the
     button is first pressed down */
  int tapState = digitalRead( 5 );
  if( tapState == LOW && tapState != lastTapState )
  {
    tap(); /* we got a HIGH-LOW transition, call our tap() function */
  }
  lastTapState = tapState; /* keep track of the state */
 
    // update leds
    ledtempo.Update();
    delay(1);
    effectbutton.tick();

        // check pot value
    poll_pot();
}

unsigned long lastTap = 0; /* when the last tap happened */

void tap()
{
  /* we keep two of these around to average together later */
  currentTimer[1] = currentTimer[0];
  currentTimer[0] = millis() - lastTap;
  lastTap = millis();
 
  // establish new period
  period = ((currentTimer[0] + currentTimer[1])/2);

  PotControls = false;

  // update leds period
  update_leds();
}

void update_leds(){
  ledtempo = JLed(6).Candle(period/250, 255).Forever();
//  ledtempo = JLed(6).Blink(period/2, period/2).Forever(),
  ledon = JLed(10).On().Forever();
  ledsquare = JLed(10).Blink(period/2, period/2).Forever();
  ledsine = JLed(10).Breathe(period).DelayAfter(0).Forever();
  ledrampup = JLed(10).FadeOn(period).DelayBefore(0).Forever();
  ledrampdown = JLed(10).FadeOff(period).DelayBefore(0).Forever();
  ledrandom = JLed(10).Candle(period/286 /*speed*/, 255 /* jitter*/);
}

void poll_pot() {
  // Read analog value
  tempAnalog = analogRead(EXP1);
 
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
   
  //delay(5); 
}


// effect button actions

void effectsingleclick(){                                 // what happens when the button is clicked
  digitalWrite(6,!digitalRead(6));
  digitalWrite(7,!digitalRead(7));
}

void effectlongpressstart(){                                   // what happens when buton is long-pressed
 //Fading the LED
  for(int i=0; i<255; i++){
    analogWrite(10, i);
    delay(10);
  }
}

void effectlongpress(){
  if (digitalRead(2) == LOW && digitalRead(3) == HIGH && digitalRead(4) == HIGH) { // if BOTH the switches read HIGH
  ledon.Update(); // statements
}
if (digitalRead(2) == HIGH && digitalRead(3) == LOW && digitalRead(4) == HIGH) { // if BOTH the switches read HIGH
  ledrandom.Update(); // statements
}
if (digitalRead(2) == LOW && digitalRead(3) == HIGH && digitalRead(4) == LOW) { // if BOTH the switches read HIGH
  ledsquare.Update(); // statements
}
if (digitalRead(2) == HIGH && digitalRead(3) == LOW && digitalRead(4) == LOW) { // if BOTH the switches read HIGH
  ledsine.Update(); // statements
}
if (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(4) == LOW) { // if BOTH the switches read HIGH
  ledrampup.Update(); // statements
}
if (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(4) == HIGH) { // if BOTH the switches read HIGH
  ledrampdown.Update(); // statements
}
}

void effectlongpressstop(){                                   // what happens when buton is long-pressed
  for(int i=255; i>0; i--){
    analogWrite(10, i);
    delay(10);
    digitalWrite(10,LOW);
}
} 
