#define RELAY_PIN 4 //Use digital pin 4 for the relay
#define SWITCH_PIN 6 //Use digital pin 6 for the switch
int switchState = LOW;
 
void setup()
{
  //Set the digital pin modes
  pinMode(SWITCH_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
}
 
void loop()
{
  //Read the current state of the switch
  switchState = digitalRead(SWITCH_PIN);
  
  if (switchState == HIGH)
  {
    //If the switch is set to HIGH, turn the relay on
    digitalWrite(RELAY_PIN, HIGH); 
    delay(100);
  }
  else
  {
    //If not, turn the relay off
    digitalWrite(RELAY_PIN, LOW);
  }
}
