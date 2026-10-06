void effectsingleclick()
{   
  buttonPushCounter++;      // every time the effect turns on, increment the counter
  if (buttonPushCounter %2 != 0 && doubleTapCounter == 0 ) 
  {  // Determines if the effect is currently off and is not in the double click function
   effectlongpressstart();  // Run through longpressstart in case 'ramp up' function is selected
   update_effect();         // Run the chosen waveform until the effect is turned off
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
  digitalWrite(line6effect, !digitalRead(line6effect));         // This goes to Line6 Effect On/Off button
  digitalWrite(line6tap, !digitalRead(line6tap));               // This goes to Line6 Tap button
  delay(100);
  digitalWrite(line6effect, !digitalRead(line6effect));         // This goes to Line6 Effect On/Off button
  digitalWrite(line6tap, !digitalRead(line6tap));               // This goes to Line6 Tap button
  }
/* If there's no doubleclick active, another single click will go to longpresstop,
in case there is a 'ramp down' selected.*/
  else 
  {
    effectlongpressstop();
  } 
}

void effectdoubleclick()
{
/* When a double click is detected increment the counter, otherwise reset the 
counter to 0, effectively a toggle.*/
  if (doubleTapCounter %2 == 0) 
  {
    doubleTapCounter++;
     Serial.print("double click on");
     Serial.println(doubleTapCounter);
  }
  else 
  {
    doubleTapCounter = 0;
    Serial.print("reset the counter");
  }
  digitalWrite(shiftfunction, !digitalRead(shiftfunction));         // Toggle the indicator light for doubleclick on/off
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
  // what happens when momentary effect engagement starts (ramp up)
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3); // 3bit value that selects the right preset based on sum of the waveselect read below
  // Light the correct LED
  switch (waveVal) 
  {
    case 3:
     //Fading in the LED
    for(int i=0; i<255; i++)
      analogWrite(effectled, i);
      delay(period/64); //Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
    break;
  }
}  

void effectlongpress()
{
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3); // 3bit value that selects the right preset based on sum of the waveselect read below
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

void effectlongpressstop()
{
  /* At the end of momentary or latching (they both cycle through longpresstop)
  Increment the counter, and ramp down if 'ramp down' is selected.
  Then turn off the LED completely with a digitalwrite as sometimes it kept glowing. :-) */
  buttonPushCounter = 0;

  // what happens when momentary effect engagement ends (ramp down)
  int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3); // 3bit value that selects the right preset based on sum of the waveselect read below

  // Light the correct LED
  switch (waveVal) 
  {
    case 3:
     //Fading in the LED
    for(int i=255; i>0; i--)
    {
      analogWrite(effectled, i);
      delay(period/64);  //Increasing this value will give a longer ramp up time, can set to time in msec (10 default) or 'period/xxx' for tap related length
    }
    digitalWrite(effectled,LOW);  // Switch off the LED
    break;
  }
} 
