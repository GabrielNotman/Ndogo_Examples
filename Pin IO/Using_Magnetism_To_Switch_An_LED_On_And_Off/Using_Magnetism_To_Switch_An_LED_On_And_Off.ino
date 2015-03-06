#define LED_PIN 5 //Use digital pin 5 for the LED
#define MAGNETIC_SWITCH 7 //Use digital pin 7 for the magnetic switch

 
void setup()
{
  //Set the digital pin modes
  pinMode(MAGNETIC_SWITCH, INPUT);
  pinMode(LED_PIN,OUTPUT);
}
 
void loop()
{
  //Read the current state of the magnetic switch
  int sensorValue = digitalRead(MAGNETIC_SWITCH);
  
  if(sensorValue == HIGH)
  {
    //If the switch is activated turn the LED on
    digitalWrite(LED_PIN, HIGH);
  }
  else
  {
    //If not, turn the LED off
    digitalWrite(LED_PIN, LOW);
  }
}
