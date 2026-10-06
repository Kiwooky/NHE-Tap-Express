void update_leds()
{   // Update the led's tempo information after tapping or setting the tempo pot
  
  ledtempo = JLed(tempoled).Blink(period/20, period/20*19).Forever(),
  ledon = JLed(effectled).On().Forever();
  ledsquare = JLed(effectled).Blink(period/2, period/2).Forever();
  ledsine = JLed(effectled).Breathe(period).DelayAfter(0).Forever();
  ledrampup = JLed(effectled).FadeOn(period).DelayBefore(0).Forever();
  ledrampdown = JLed(effectled).FadeOff(period).DelayBefore(0).Forever();
  ledrandom = JLed(effectled).Candle(period/286 /*speed*/, 255 /* jitter*/);

  previewon = JLed(previewled).On().Forever();
  previewsquare = JLed(previewled).Blink(period/2, period/2).Forever();
  previewsine = JLed(previewled).Breathe(period).DelayAfter(0).Forever();
  previewrampup = JLed(previewled).FadeOn(period).DelayBefore(0).Forever();
  previewrampdown = JLed(previewled).FadeOff(period).DelayBefore(0).Forever();
  previewrandom = JLed(previewled).Candle(period/286 /*speed*/, 255 /* jitter*/);
}

void update_effect()
{   // Keep the led waveform updated in the loop until it's turned off

  
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
  previewsine.Update();
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

void update_preview()
{   // Keep the preview waveform led updated in the loop
  
int waveVal = digitalRead(waveselect1) << 2 | digitalRead(waveselect2) << 1 | digitalRead(waveselect3);

  // Light the correct LED
  switch (waveVal) 
  {
  case 2:
    previewsquare.Update();
    break;
  case 3:
    previewon.Update();
    break;
  case 4:
    previewsine.Update();
    break;
  case 5:
    previewrandom.Update();
    break;
  case 6:
    previewrampup.Update();
    break;
  case 7:
    previewrampdown.Update();
    break;
  }  
}
