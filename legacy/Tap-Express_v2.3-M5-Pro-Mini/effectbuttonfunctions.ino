void effectsingleclick()
{   
  Serial.print("click");
  buttonPushCounter++;      // every time the effect turns on, increment the counter
  if (buttonPushCounter %2 != 0 && doubleTapCounter == 0) 
  {  // Determines if the effect is currently off and is not in the double click function
   
   waslongpressed = false;        // Used to determine where to go in case a rampdown was aborted and needs to return to either momentary or latching state
   Serial.println(cancelrampdown);
   if (cancelrampdown == false)
     {
     Serial.print("gets here");
     effectlongpressstart();  // Run through longpressstart in case 'ramp up' function is selected
     }  
   else if (cancelrampup == false)
     {
     cancelrampdown = false;
     Serial.print("going to update effect >>");
     update_effect();         // Run the chosen waveform until the effect is turned off
     }
   else if (cancelrampup == true)
   {  // If 'effectlongpressstart' was canceled and 'ramped down' to zero, this will reset everything for the next effectbutton event
    ledon.Stop();
    ledrandom.Stop();
    ledsquare.Stop();
    ledsine.Stop();
    ledrampup.Stop();
    ledrampdown.Stop();
    cancelrampup = false;
    buttonPushCounter = 0;
   }
   Serial.print("nothing applied");
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
  digitalWrite(tapdisable, !digitalRead(tapdisable));                   // Disables Tap tempo when activating preset menu through a CD4066 switch
  //delay(10);
  digitalWrite(line6bankselect, !digitalRead(line6bankselect));         // Pulls Line6 Effect On/Off and Tap button low (to ground) via CD4066 switch
  //delay(300);                                                           // little pause to simulate a momentary footswitch press/depress
  //digitalWrite(line6bankselect, !digitalRead(line6bankselect));         // Pulls Line6 Effect On/Off and Tap button low (to ground) via CD4066 switch
  //delay(10);
  //digitalWrite(tapdisable, !digitalRead(tapdisable));                   // Re-enables Tap tempo when activating preset menu through a CD4066 switch
  }
/* If there's no doubleclick active, another single click will go to longpresstop,
in case there is a 'ramp down' selected.*/
  else 
  {
    ledtempo.Update();    // update the tempo led continously
    effectlongpressstop();
      if (LowActiveLED == true)
        {
          digitalWrite(effectled,HIGH);  // Switch on the LED
        }
        else
        {
          digitalWrite(effectled,LOW);  // Switch on the LED
        }     
  } 
}

void effectlongpressstart()
{
  /* At the start of momentary or latching (they both cycle through longpresstart)
  Reset the LED so the timing starts from the moment you turn it on, and ramp up
  to the waveform if 'ramp up' is selected.*/
  ledtempo.Update();    // update the tempo led continouslyledon.Reset();
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
  int rampvalue = 0;                                // This value stores the current brightness of the LED during ramp up/down, always starts at 0 (off) 
  int buttonState = digitalRead(effectbutton);      // This value stores the current state of the button for state change detection
  int lastButtonState = digitalRead(effectbutton);  // This value stores the previous state of the button for state change detection
  bool rampback = false;                            // rampback is used to toggle between ramp up / ramp down as long as either is not finished. 
  cancelrampup = false;                             // Reset if a ramp up was cancelled
  
  // 3bit value that selects the right switch case based on sum of the waveselect pins read below
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3); 
  /*Currently there is only one switch case (3), that corresponds to the 'always on' waveform setting.
  As all the other wave forms either start at 0, are random, or do contunous ramp up or ramp down.
  So it didn't seem as useful there, but might add functionalities to this in future versions*/
  switch (waveVal)
    {
      case 7:
      // Fading in the LED
      // This while loop makes sure the only ways to exit the loop are a completed 'abort' back to off, or a succesful 'ramp up' to full brightness
      while ( rampback == true && rampvalue != 0 || rampback == false && rampvalue != 255 && cancelrampdown == false)
         {
           // This while loop makes sure we chose to 'ramp up' the LED and it's not already at full brightness (not sure if it needs the brightness statement)
           while ( rampback == false && rampvalue != 255)
            {
            // Reset the button states (default state is high, state change on low)
            buttonState = 1;
            lastButtonState = 1;
            // A bit of delay to make sure there is no bounce interference from the button
            delay(300);
            Serial.print("starting ramp up >>");
            // Until rampvalue reaches 255 (full brightness) keep increasing brightness
            for(rampvalue; rampvalue<255; rampvalue++)
              {
                buttonState = digitalRead(effectbutton);        // Read the effect button to see if it has changed state
                analogWrite(effectled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                analogWrite(previewled, rampvalue);             // Rampvalue keeps increasing until full brightness (255)
                delay(rampuptime);                             // Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
                ledtempo.Update();    // update the tempo led continously
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
          Serial.print("going back down >>");
          // Until rampvalue reaches 0 (off) keep decreasing brightness
          for(rampvalue; rampvalue>0; rampvalue--)
            {
                buttonState = digitalRead(effectbutton);        // Read the effect button to see if it has changed state
                analogWrite(effectled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                analogWrite(previewled, rampvalue);              // Rampvalue keeps increasing until full brightness (255)
                delay(rampuptime);                               // Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
                ledtempo.Update();    // update the tempo led continously
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
              Serial.print("rampup fully aborted >>");
              cancelrampup = true;                                   // Set a boolean so the 'single click' event is reset when it returns from here.
            if (LowActiveLED == true)
                {
                  buttonPushCounter = 0;
                  digitalWrite(effectled,HIGH);  // Switch on the LED
                }
            else
                {
                  buttonPushCounter = 0;
                  digitalWrite(effectled,LOW);  // Switch on the LED
                }
        return;                                             // Go back to the function that called it, in this case effectsingleclick
            }
          }
        }
       Serial.print("Completed Ramp up event >>");
       cancelrampup = false;
       break;
    }
}

void effectlongpress()
// Effectlongpress is a momentary action, so as long as the footswitch is pressed it will loop and update the led
{
  cancelrampdown = false;
  ledtempo.Update();    // update the tempo led
  // 3bit value that selects the right switch case based on sum of the waveselect pins read below
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3); 
  // Light the correct LED
  switch (waveVal) 
  {
  case 1:
    ledsine.Update();
    break;
  case 2:
    ledrampup.Update();
    break;
  case 3:
    ledsquare.Update();
    break;
  case 4:
    break;
  case 5:
    ledrampdown.Update();
    break;
  case 6:
    ledrandom.Update();
    break;
  case 7:
    ledon.Update();
    break;
  }  
}

void simplerampdown()  // what happens when momentary effect engagement ends (ramp down)
{
  ledtempo.Stop();    // stop the tempo led during ramp
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3);

  switch (waveVal) 
  {
    case 7:
    for(int rampvalue=255; rampvalue>0; rampvalue--)
          {
          analogWrite(previewled, rampvalue);  
          analogWrite(effectled, rampvalue);
          delay(rampdowntime); //Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
          }
    buttonPushCounter = 0;        // Reset the counter
    break;
  }
  digitalWrite(effectled,LOW);  // Switch off the LED
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
  ledtempo.Update();    // update the tempo led continously                            
  
  // 3bit value that selects the right switch case based on sum of the waveselect pins read below
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3); 
  /*Currently there is only one switch case (3), that corresponds to the 'always on' waveform setting.
  As all the other wave forms either start at 0, are random, or do contunous ramp up or ramp down.
  So it didn't seem as useful there, but might add functionalities to this in future versions*/
  if(cancelrampup == false)
  {
  switch (waveVal)
    {
      case 7:
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
                ledtempo.Update();    // update the tempo led continously
                delay(rampdowntime);                               // Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
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
                ledtempo.Update();    // update the tempo led continously
                delay(rampdowntime);                               // Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
                lastButtonState = digitalRead(effectbutton);    // Read the effect button to see if it has changed state
                // If during ramp up the switch changes state, we switch back to the 'ramp up function' 
                if(buttonState != lastButtonState)
                {
                  rampback = false;                             // Set rampback to true will change the while loop from 'ramp down' to 'ramp up'
                  break;                                        // break the while loop to initiate based on the new boolean statement (rampback = false)
                }
            }
          }
        }
       // If the ramp down is canceled in full (aborted during ramp down and ramped up back to '255') we need to do back to updating the effect
       if (rampvalue == 255)
        {
             Serial.print("waslongpressedvalue at switch time");
   Serial.println(waslongpressed);
          if ( waslongpressed == false)
            {
              Serial.print("Ramp down fully aborted >>");
              Serial.print("going back to latching >>");
              waslongpressed = true;
              cancelrampdown = true;
              buttonPushCounter = 0;
              effectsingleclick();
              return;
            }
          else 
            { 
              Serial.print("Ramp down fully aborted >>");
              Serial.print("going back to momentary >>");
              cancelrampdown = true;
              effectlongpressstart();
              return;
            }                                          // Go back to the function that called it, in this case effectsingleclick
        }
        else {
        if (LowActiveLED == true)
          {
            Serial.print("Completed LowActive Rampdown >>");
            cancelrampdown = false;
            ledtempo.Reset();
            digitalWrite(effectled,HIGH);  // Switch on the LED
          }
        else
          {
            Serial.print("Completed HighActive Rampdown >>");
            cancelrampdown = false;
            ledtempo.Reset();
            digitalWrite(effectled,LOW);  // Switch off the LED
          }
       break;
       }
    }
  }
  if (LowActiveLED == true)
    {
      Serial.print("Completed LowActive Rampdown >>");
      cancelrampdown = false;
      digitalWrite(effectled,HIGH);  // Switch on the LED
    }
  else
    {
      Serial.print("Completed HighActive Rampdown >>");
      cancelrampdown = false;
      digitalWrite(effectled,LOW);  // Switch off the LED
    }
}

void effectdoubleclick()
{
   Serial.print("Shift is on >>");
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
