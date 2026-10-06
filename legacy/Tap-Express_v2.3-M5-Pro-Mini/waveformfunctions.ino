void update_leds()
{   // Update the led's tempo information after tapping or setting the tempo pot
  if (LowActiveLED == true)
    {
      ledon = JLed(effectled).On().Forever().LowActive();
      ledsquare = JLed(effectled).Blink(period*factor/2, period*factor/2).Forever().LowActive();
      ledsine = JLed(effectled).Breathe(period*factor).DelayAfter(0).Forever().LowActive();
      ledrampup = JLed(effectled).FadeOn(period*factor).DelayBefore(0).Forever().LowActive();
      ledrampdown = JLed(effectled).FadeOff(period*factor).DelayBefore(0).Forever().LowActive();
      ledrandom = JLed(effectled).Candle(period*factor/286 /*speed*/, 255 /* jitter*/).Forever().LowActive();
    }
  else
    {
      ledon = JLed(effectled).On().Forever();
      ledsquare = JLed(effectled).Blink(period*factor/2, period*factor/2).Forever();
      ledsine = JLed(effectled).Breathe(period*factor).DelayAfter(0).Forever();
      ledrampup = JLed(effectled).FadeOn(period*factor).DelayBefore(0).Forever();
      ledrampdown = JLed(effectled).FadeOff(period*factor).DelayBefore(0).Forever();
      ledrandom = JLed(effectled).Candle(period*factor/286 /*speed*/, 255 /* jitter*/).Forever();
    }

    previewon = JLed(previewled).On().Forever().MaxBrightness(127);
    previewsquare = JLed(previewled).Blink(period*factor/2, period*factor/2).Forever().MaxBrightness(127);
    previewsine = JLed(previewled).Breathe(period*factor).DelayAfter(0).Forever().MaxBrightness(127);
    previewrampup = JLed(previewled).FadeOn(period*factor).DelayBefore(0).Forever().MaxBrightness(127);
    previewrampdown = JLed(previewled).FadeOff(period*factor).DelayBefore(0).Forever().MaxBrightness(127);
    previewrandom = JLed(previewled).Candle(period*factor/286 /*speed*/, 255 /* jitter*/).Forever().MaxBrightness(127);
      

//    Serial.print("new period:");
//    Serial.println(period*factor);  
//    Serial.print("new ramp time:");
//    Serial.println(rampdowntime);
}

void update_effect()
{   // Keep the led waveform updated in the loop until it's turned off
int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3);
ledtempo.Update();    // update the tempo led
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

void update_preview()
{   // Keep the preview waveform led updated in the loop
int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3);
  // Light the correct LED
//  Serial.println(waveVal);
  switch (waveVal) 
  {
    case 1:
    previewsine.Update();
    ledramp.Stop();
    ledramp.Reset();
    break;
  case 2:
    previewrampup.Update();
    ledramp.Stop();
    ledramp.Reset();
    break;
  case 3:
    previewsquare.Update();
    ledramp.Stop();
    ledramp.Reset();
    break;
  case 4:
    break;
  case 5:
    previewrampdown.Update();
    ledramp.Stop();
    ledramp.Reset();
    break;
  case 6:
    previewrandom.Update();
    ledramp.Stop();
    ledramp.Reset();
    break;
  case 7:
    previewon.Update();
    ledramp.Update();
    break;
  }  
}



void update_factor()
{
  int factorVal = digitalRead(factorselect1) << 1 | digitalRead(factorselect2);
  // Light the correct LED
  //Serial.println(factorVal);
  switch (factorVal) 
  {
  case 1:
  factor = 4;
 ledtempo = JLed(tempoled).Blink((period/4)/20, (period/4)/20*19).Forever();

    break;
  case 2:
  factor = 0.5;
  ledtempo = JLed(tempoled).Blink((period*2)/20, (period*2)/20*19).Forever();

    break;
  case 3:
  factor = 1;
  ledtempo = JLed(tempoled).Blink(period/20, period/20*19).Forever();
    break;
  }
 Serial.println(period*factor);  
 update_leds();
  
  }

  void update_ramptime()
{

  
  int rampVal = digitalRead(rampselect1) << 1 | digitalRead(rampselect2);
  // Light the correct LED
  //Serial.println(rampVal);
  switch (rampVal) 
  {
  case 1:

    break;
  case 2:

    break;
  case 3:
  ledramp = JLed(rampled).Breathe(period*4).DelayAfter(period*2).Forever().MaxBrightness(127);
  rampuptime = period * rampupfactor/255;
    rampdowntime = period * rampdownfactor/255;
    break;
  }  
}
