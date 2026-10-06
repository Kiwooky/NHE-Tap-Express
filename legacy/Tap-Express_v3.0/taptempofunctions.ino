void tap()
{
  /* we keep two of these around to average together later */
  currentTimer[1] = currentTimer[0];
  currentTimer[0] = millis() - lastTap;
  lastTap = millis();
 
  period = ((currentTimer[0] + currentTimer[1])/2);   // establish new period

  PotControls = false;   // override the pot when tapping a tempo

  update_leds();         // update leds period
}

void poll_pot() 
{
// Allow tempo pot reading to override tapped tempo
  tempAnalog = analogRead(tempopot);    // Read analog value
 
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
