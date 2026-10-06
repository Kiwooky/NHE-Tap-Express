void rising() 
{
  attachInterrupt(0, falling, FALLING);
  prev_time = micros();
}
 
void falling() 
{
  attachInterrupt(0, rising, RISING);
  pwm_value = micros()-prev_time;
  pwm_value = map(pwm_value, 8, 1828, 0, 127);
  //Serial.println(pwm_value);
}
