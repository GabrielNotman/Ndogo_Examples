#define BUZZER_PIN 4 //Use digital pin 4 for the buzzer
#define BUTTON_PIN 6 //Use digital pin 6 for the button
int buttonState = LOW; 

void setup()
{
    //Set the digital pin modes
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
}
 
void loop()
{
    //Read the current state of the button
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == HIGH) 
    {
        //If the button is pressed, turn the buzzer on
        digitalWrite(BUZZER_PIN, HIGH);
    }
    else 
    {
        //If not, turn the buzzer off
        digitalWrite(BUZZER_PIN, LOW);
    }
}
