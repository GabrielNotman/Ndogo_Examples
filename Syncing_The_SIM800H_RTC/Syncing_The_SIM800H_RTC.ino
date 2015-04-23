#include <Wire.h>
#include <GPRSbee.h>

#define BAUD_RATE 57600

#define APN "everywhere"
#define APN_USERNAME ""
#define APN_PASSWORD ""

#define TIME_URL "time.sodaq.net"
#define TIME_ZONE 1.0
#define TIME_ZONE_SECONDS (TIME_ZONE * 3600)

#define UNIX_OFFSET 946684800

void setup() 
{
  //Start Serial for serial monitor
  Serial.begin(BAUD_RATE);

  //Setup the GPRSbee
  setupComms();
  
  //Sync time
  syncRTCwithServer();
}

void loop() 
{
}

void setupComms()
{
  //Start Serial1 the Bee port
  Serial1.begin(BAUD_RATE);
  
  //Intialise the GPRSbee
  gprsbee.initNdogoSIM800(Serial1, SIM800PWRKEY, SIM800VBAT, SIM800STATUS);
  gprsbee.setSkipCGATT(true);
  
  //Uncomment the line below to debug the GPRSbee with the serial monitor
  //gprsbee.setDiag(Serial);
}

void syncRTCwithServer()
{
  char buffer[20];
  
  Serial.println("Retrieving timestamp from server...");
  
  if (gprsbee.doHTTPGET(APN, APN_USERNAME, APN_PASSWORD, TIME_URL, buffer, sizeof(buffer)))
  {
    //Display data retrieved
    Serial.print("Retrieved (Unix): " + String(buffer)); 
    
    //Convert the time stamp to unsigned long
    char *ptr;
    uint32_t newTs = strtoul(buffer, &ptr, 0);
    
    //Add timezone offset
    newTs += TIME_ZONE_SECONDS;
    Serial.println("With timezone offset: " + String(newTs));
        
    //If conversion was successful
    if (ptr != buffer) 
    {
      //Get the old time stamp
      uint32_t oldTs = gprsbee.getUnixEpoch();
     
      //Display old and new time stamps
      Serial.print("Updating RTC, old=" + String(oldTs));
      Serial.println(" new=" + String(newTs));

      //Update the Sim800H rtc
      SIMDateTime dt(newTs - UNIX_OFFSET);
      gprsbee.setCCLK(dt);
    }
  }
  else
    Serial.println("Error retrieving timestamp from the server.");
    
  //Print out new current date/time
  Serial.println("The clock is now set to: " + getDateTime());
  Serial.println("Format = YY/MM/DD,HH:MM:SS+TZ");
}

String getDateTime()
{
  String dateTimeStr;
  
  //Create a DateTime object from the current time
  SIMDateTime dt(gprsbee.getUnixEpoch() - UNIX_OFFSET);

  //Convert it to a String
  dt.addToString(dateTimeStr);
  
  return dateTimeStr;  
}
