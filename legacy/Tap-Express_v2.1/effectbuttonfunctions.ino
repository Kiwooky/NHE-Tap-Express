void effectsingleclick()
{   
  buttonPushCounter++;      // every time the effect turns on, increment the counter
  Serial.println(buttonPushCounter);
  if (buttonPushCounter %2 != 0 && doubleTapCounter == 0 ) 
  {  // Determines if the effect is currently off and is not in the double click function
   effectlongpressstart();  // Run through longpressstart in case 'ramp up' function is selected
   if (cancelrampup == 0){
   update_effect();         // Run the chosen waveform until the effect is turned off
   }
   if (cancelrampup == 1){  // If 'effectlongpressstart' was canceled and 'ramped down' to zero, this will reset everything for the next effectbutton event
    ledon.Stop();
    ledrandom.Stop();
    ledsquare.Stop();
    ledsine.Stop();
    ledrampup.Stop();
    ledrampdown.Stop();
    cancelrampup = 0;
    buttonPushCounter = 0;
   }
  } 
  /* If a doubleclick is active, make sure the led stays off and fire a momentary pulse 
  (in my case to a CD4066 cmos switch) to switch the Line6 effect and tap buttons   
  simultenously to trigger the preset select.*/
  else if (doubleTapCounter != 0 ) 
  {  
  ledon.Stop();
  ledrandom.Stop();
  ledsquare.Stop();
  ledsine.Stop();
  ledrampup.Stop();
  ledrampdown.Stop();
  digitalWrite(line6effect, !digitalRead(line6effect));         // Pulls Line6 Effect On/Off button low (to ground) via CD4066 switch
  digitalWrite(line6tap, !digitalRead(line6tap));               // Pulls Line6 Tap button low (to ground) via CD4066 switch
  delay(100);                                                   // little pause to simulate a momentary footswitch press/depress
  digitalWrite(line6effect, !digitalRead(line6effect));         // Pulls Line6 Effect On/Off button low (to ground) via CD4066 switch
  digitalWrite(line6tap, !digitalRead(line6tap));               // Pulls Line6 Tap button low (to ground) via CD4066 switch
  }
/* If there's no doubleclick active, another single click will go to longpresstop,
in case there is a 'ramp down' selected.*/
  else 
  {
    effectlongpressstop();
    digitalWrite(effectled,LOW);  // Switch off the LED
  } 
}

void effectlongpressstart()
{
  /* At the start of momentary or latching (they both cycle through longpresstart)
  Reset the LED so the timing starts from the moment you turn it on, and ramp up
  to the waveform if 'ramp up' is selected.*/
  ledon.Reset();
  ledrandom.Reset();
  ledsquare.Reset();
  ledsine.Reset();
  ledrampup.Reset();
  ledrampdown.Reset();
  previewrandom.Reset();
  previewsquare.Reset();
  previewsine.Reset();
  previewrampup.Reset();
  previewrampdown.Reset();
  int waslongpressed = true;
  int rampvalue = 0;                                // This value stores the current brightness of the LED during ramp up/down, always starts at 0 (off) 
  int buttonState = digitalRead(effectbutton);      // This value stores the current state of the button for state change detection
  int lastButtonState = digitalRead(effectbutton);  // This value stores the previous state of the button for state change detection
  bool rampback = false;                            // rampback is used to toggle between ramp up / ramp down as long as either is not finished. 
  
  // 3bit value that selects the right switch case based on sum of the waveselect pins read below
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3); 
  /*Currently there is only one switch case (3), that corresponds to the 'always on' waveform setting.
  As all the other wave forms either start at 0, are random, or do contunous ramp up or ramp down.
  So it didn't seem as useful there, but might add functionalities to this in future versions*/
  switch (waveVal)
    {
      case 3:
      // Fading in the LED
      // This while loop makes sure the only ways to exit the loop are a completed 'abort' back to off, or a succesful 'ramp up' to full brightness
      while ( rampback == true && rampvalue != 0 || rampback == false && rampvalue != 255 )
         {
           // This while loop makes sure we chose to 'ramp up' the LED and it's not already at full brightness (not sure if it needs the brightness statement)
           while ( rampback == false && rampvalue != 255)
            {
            // Reset the button states (default state is high, state change on low)
            buttonState = 1;
            lastButtonState = 1;
            // A bit of delay to make sure there is no bounce interference from the button
            delay(300);
            // Until rampvalue reaches 255 (full brightness) keep increasing brightness
            for(rampvalue; rampvalue<255; rampvalue++)
              {
                buttonState = digitalRead(effectbutton);        // Read the effect button to see if it has changed state
                analogWrite(effectled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                analogWrite(previewled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                delay(period/16);                               // Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
                lastButtonState = digitalRead(effectbutton);    // Read the effect button to see if it has changed state
                // If during ramp up the switch changes state, we initiate an 'abort function' 
                if(buttonState != lastButtonState)
                  {
                   rampback = true;                             // Set rampback to true will change the while loop from 'ramp up' to 'ramp down'
                   break;                                       // break the while loop to initiate based on the new boolean statement (rampback = true)
                  }
              }
          }
      // This while loop makes sure we chose to 'ramp down' the LED and it's not already off (not sure if it needs the brightness statement)
      while ( rampback == true && rampvalue != 0 )
          {
          // Reset the button states (default state is high, state change on low)
          buttonState = 1;
          lastButtonState = 1;
          // A bit of delay to make sure there is no bounce interference from the button
          delay(300);
          // Until rampvalue reaches 0 (off) keep decreasing brightness
          for(rampvalue; rampvalue>0; rampvalue--)
            {
                buttonState = digitalRead(effectbutton);        // Read the effect button to see if it has changed state
                analogWrite(effectled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                analogWrite(previewled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                delay(period/16);                               // Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
                lastButtonState = digitalRead(effectbutton);    // Read the effect button to see if it has changed state
                // If during ramp up the switch changes state, we switch back to the 'ramp up function' 
                if(buttonState != lastButtonState)
                {
                  rampback = false;                             // Set rampback to true will change the while loop from 'ramp down' to 'ramp up'
                  break;                                        // break the while loop to initiate based on the new boolean statement (rampback = false)
                }
            }
           // If the ramp up is canceled in full (aborted during ramp up and ramped down to '0') we need to do a couple of things to reset the system
           if (rampvalue == 0)
            {
            cancelrampup = 1;                                   // Set a boolean so the 'single click' event is reset when it returns from here.
//            buttonPushCounter = 2;                            // I think we don't need this as it's set to '0' when we do the 'return'
            digitalWrite(effectled,LOW);                        // Switch off the LED
            return;                                             // Go back to the function that called it, in this case effectsingleclick
            }
          }
        }
       break;
    }
}

void effectlongpress()
// Effectlongpress is a momentary action, so as long as the footswitch is pressed it will loop and update the led
{
  // 3bit value that selects the right switch case based on sum of the waveselect pins read below
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3); 
  // Light the correct LED
  switch (waveVal) 
  {
  case 2:
    ledsquare.Update();
    break;
  case 3:
    ledon.Update();
    break;
  case 4:
    ledsine.Update();
    break;
  case 5:
    ledrandom.Update();
    break;
  case 6:
    ledrampup.Update();
    break;
  case 7:
    ledrampdown.Update();
    break;
  }  
}

void effectlongpressstopOLD()  // what happens when momentary effect engagement ends (ramp down)
{
    if (cancelrampup == 1)
  {
    int rampvalue = 0;
    cancelrampup = 0;
    buttonPushCounter = 0;
    return;
  }
  /* At the end of momentary or latching (they both cycle through longpresstop)
  Increment the counter, and ramp down if 'ramp down' is selected.
  Then turn off the LED completely with a digitalwrite as sometimes it kept glowing. :-) */
  // 3bit value that selects the right preset based on sum of the waveselect read below
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3);

  switch (waveVal) 
  {
    case 3:
     //Fading out the LED
    for(int rampvalue=255; rampvalue>0; rampvalue--)
          {
          analogWrite(effectled, rampvalue);
          delay(period/64); //Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
          }
    buttonPushCounter = 0;        // Reset the counter
    break;
  }
  digitalWrite(effectled,LOW);  // Switch off the LED
  Serial.println(rampvalue);
} 

void effectlongpressstop()
{
  /* At the start of momentary or latching (they both cycle through longpresstart)
  Reset the LED so the timing starts from the moment you turn it on, and ramp up
  to the waveform if 'ramp up' is selected.*/
  int rampvalue = 255;                              // This value stores the current brightness of the LED during ramp up/down, always starts at 255 (on) 
  int buttonState = digitalRead(effectbutton);      // This value stores the current state of the button for state change detection
  int lastButtonState = digitalRead(effectbutton);  // This value stores the previous state of the button for state change detection
  bool rampback = false;                            // rampback is used to toggle between ramp up / ramp down as long as either is not finished. 
  
  // 3bit value that selects the right switch case based on sum of the waveselect pins read below
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3); 
  /*Currently there is only one switch case (3), that corresponds to the 'always on' waveform setting.
  As all the other wave forms either start at 0, are random, or do contunous ramp up or ramp down.
  So it didn't seem as useful there, but might add functionalities to this in future versions*/
  switch (waveVal)
    {
      case 3:
      // Fading out the LED
      // This while loop makes sure the only ways to exit the loop are a completed 'abort' back up to full brightness, or a succesful 'ramp down' to zero
      while ( rampback == true && rampvalue != 255 || rampback == false && rampvalue != 0 )
         {
           // This while loop makes sure we chose to 'ramp down' the LED and it's not already at full brightness (not sure if it needs the brightness statement)
           while ( rampback == false && rampvalue != 0)
            {
            // Reset the button states (default state is high, state change on low)
            buttonState = 1;
            lastButtonState = 1;
            // A bit of delay to make sure there is no bounce interference from the button
            delay(300);
            Serial.print("starting ramp down >>");
            // Until rampvalue reaches 255 (full brightness) keep increasing brightness
            for(rampvalue; rampvalue>0; rampvalue--)
              {
                buttonState = digitalRead(effectbutton);        // Read the effect button to see if it has changed state
                analogWrite(effectled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                analogWrite(previewled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                delay(period/16);                               // Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
                lastButtonState = digitalRead(effectbutton);    // Read the effect button to see if it has changed state
                // If during ramp up the switch changes state, we initiate an 'abort function' 
                if(buttonState != lastButtonState)
                  {
                   rampback = true;                             // Set rampback to true will change the while loop from 'ramp up' to 'ramp down'
                   break;                                       // break the while loop to initiate based on the new boolean statement (rampback = true)
                  }
              }
          }
      // This while loop makes sure we chose to 'ramp down' the LED and it's not already off (not sure if it needs the brightness statement)
      while ( rampback == true && rampvalue != 255 )
          {
          // Reset the button states (default state is high, state change on low)
          buttonState = 1;
          lastButtonState = 1;
          // A bit of delay to make sure there is no bounce interference from the button
          delay(300);
          Serial.print("going back up >>");
          // Until rampvalue reaches 0 (off) keep decreasing brightness
          for(rampvalue; rampvalue<255; rampvalue++)
            {
                buttonState = digitalRead(effectbutton);        // Read the effect button to see if it has changed state
                analogWrite(effectled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                analogWrite(previewled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                delay(period/16);                               // Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
                lastButtonState = digitalRead(effectbutton);    // Read the effect button to see if it has changed state
                // If during ramp up the switch changes state, we switch back to the 'ramp up function' 
                if(buttonState != lastButtonState)
                {
                  rampback = false;                             // Set rampback to true will change the while loop from 'ramp down' to 'ramp up'
                  break;                                        // break the while loop to initiate based on the new boolean statement (rampback = false)
                }
            }
           // If the ramp down is canceled in full (aborted during ramp down and ramped up back to '255') we need to do back to updating the effect
           if (rampvalue == 255)
           Serial.print("fully aborted >>");
            {
              if ( waslongpressed == true)
                {
                  Serial.print("going back to momentary >>");
                effectlongpress();
                }
              else 
              {
                Serial.print("going back to latching >>");
                update_effect();
              }                                          // Go back to the function that called it, in this case effectsingleclick
            }
          }
        }
        Serial.print("reset was longpressed >>");
       waslongpressed = false;
       break;
    }
}

void effectdoubleclick()
{
/* When a double click is detected increment the counter, otherwise reset the counter to 0, 
 effectively a toggle. Double click initiates a 'shift function' where the latching 
 (single click) performs a different function, the momentary functions don't change */
  if (doubleTapCounter %2 == 0) 
  {
    doubleTapCounter++;         // Turn on the 'shift function'
  }
  else 
  {
    doubleTapCounter = 0;       // Reset the 'shift function' (off)
  }
  digitalWrite(shiftfunction, !digitalRead(shiftfunction));         // Toggle the indicator light for doubleclick on/off
}
