void tap()
{
  /* we keep two of these around to average together later */
  currentTimer[1] = currentTimer[0];
  currentTimer[0] = millis() - lastTap;
  lastTap = millis();
 
  period = ((currentTimer[0] + currentTimer[1])/2);   // establish new period

  PotControls = false;   // override the pot when tapping a tempo

  update_factor(); /* we got a HIGH-LOW transition, call our tap() function */
}

void poll_pot() 
{
total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(tempopot);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits
 
  delay(1);        // delay in between reads for stability

    // Convert 10 bit to milliseconds, 
  average = map(average, 0, 1023, 0, MAXDELAY );
  average = constrain(average, MINDELAY, MAXDELAY );

  if((abs(average-lastVal)>THRESHOLD))
  {
  period = average;
    //return control to Pot   
    PotControls = true;
    // Store current value to be used laters
    lastVal = average;
    // update leds with new timing
     update_factor();
     Serial.println(period);
  }
}
