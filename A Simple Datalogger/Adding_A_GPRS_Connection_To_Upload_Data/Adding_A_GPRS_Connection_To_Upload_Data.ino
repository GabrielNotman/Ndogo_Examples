#include <Wire.h>
#include <SPI.h>

//SODAQ Mbili libraries
#include <RTCTimer.h>
#include <Sodaq_BMP085.h>
#include <Sodaq_SHT2x.h>
#include <Sodaq_DS3231.h>
#include <Sodaq_dataflash.h>
#include <GPRSbee.h>

//DataFlash constants
#define NUM_PAGES 4096
#define PAGE_SIZE 528
#define NEW_LINE "\r\n"

//The delay between the sensor readings 
#define READ_DELAY 60000

//Data header
#define DATA_HEADER "TimeDate, TempSHT21, TempBMP, PressureBMP, HumiditySHT21"

//Network constants
#define APN "internet"
#define APN_USERNAME ""
#define APN_PASSWORD ""

//SpeakThings constants
#define URL "api.thingspeak.com/update"
#define WRITE_API_KEY "XXXXXXXXXXXXXXXX" //Change to your channel's key

//Seperators
#define FIRST_SEP "?"
#define OTHER_SEP "&"
#define LABEL_DATA_SEP "="

//Data labels, cannot change for ThingSpeak
#define LABEL1 "field1"
#define LABEL2 "field2"
#define LABEL3 "field3"
#define LABEL4 "field4"

//TPH BMP sensor
Sodaq_BMP085 bmp;

//RTC Timer
RTCTimer timer;

//DataFlash variables
int Page = 0; 
int Address = 0;

void setup() 
{
  //Initialise the serial connection
  Serial.begin(9600);
  
  //Initialise sensors
  setupSensors();
    
  //Initialise log file
  setupLogFile();
  
  //Setup timer events
  setupTimer();
  
  //Setup GPRSbee
  setupComms();
  
  //Echo the data header to the serial connection
  Serial.println(DATA_HEADER);
  
  //Take first reading immediately
  takeReading(getNow());
}

void loop() 
{
  //Update the timer 
  timer.update();  
}

void takeReading(uint32_t ts)
{
  //Create the data record
  String dataRec = createDataRecord();
  
  //Save the data record to the log file
  logData(dataRec);
  
  //Echo the data to the serial connection
  Serial.println(dataRec);
  
  //Get the data record as a URL
  String url = createDataURL();
  
  //Send it over the GPRS connection
  sendURLData(url);
}

void setupSensors()
{
  //Initialise the wire protocol for the TPH sensors
  Wire.begin();
  
  //Initialise the TPH BMP sensor
  bmp.begin();

  //Initialise the DS3231 RTC
  rtc.begin();
}

void setupLogFile()
{
  //Todo remove this in the new Ndogo revision
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH); 
        
  //Initialise the Data Flash
  dflash.init(MISO, MOSI, SCK, SS);
}

void setupTimer()
{
  //Instruct the RTCTimer how to get the current time reading
  timer.setNowCallback(getNow);
  
  //Schedule the reading every second
  timer.every(READ_DELAY, takeReading);
}

void setupComms()
{
  //Start Serial1 the Bee port
  Serial1.begin(9600);
  
  //Intialise the GPRSbee
  #ifdef ARDUINO_AVR_SODAQ_NDOGO
    gprsbee.initNdogoSIM800(Serial1, SIM800PWRKEY, SIM800VBAT, SIM800STATUS);
  #endif
  #ifdef ARDUINO_AVR_SODAQ_MBILI 
    gprsbee.init(Serial1, BEECTS, BEEDTR);
    gprsbee.setPowerSwitchedOnOff(true); 
  #endif

  //uncomment this line to debug the GPRSbee with the serial monitor
  //gprsbee.setDiag(Serial);
}

void logData(String rec)
{
  //Add new line characters
  rec += NEW_LINE;
  
  //Copy rec to an array  
  int len = rec.length();
  uint8_t buff[len+1]; //+1 for null termination character
  rec.getBytes(buff, len+1);
  
  //If it fits on the current page
  if ((Address + len) < PAGE_SIZE)
  {
    dflash.writeStrBuf1(Address, buff, len);
    dflash.writeBuf1ToPage(Page);
    Address += len;
  }
  else
  {
    //Work out the amount to be written to the 
    //current and next page
    int part = PAGE_SIZE - Address;
    int other = len - part;
    
    //Copy the first part to the current page
    dflash.writeStrBuf1(Address, buff, part);
    dflash.writeBuf1ToPage(Page);
    
    //Increment the page number and wrap if needed
    Page++;
    Page = Page % NUM_PAGES;

    //Copy the remainder to the next page
    dflash.writeStrBuf1(0, &buff[part], other);
    dflash.writeBuf1ToPage(Page);
    Address = other;
  }
}

String createDataRecord()
{
  //Create a String type data record in csv format
  //TimeDate, TempSHT21, TempBMP, PressureBMP, HumiditySHT21
  String data = getDateTime() + ", ";
  data += String(SHT2x.GetTemperature())  + ", ";
  data += String(bmp.readTemperature()) + ", ";
  data += String(bmp.readPressure() / 100)  + ", ";
  data += String(SHT2x.GetHumidity());
  
  return data;
}

String createDataURL()
{
  //Construct data URL
  String url = URL;
 
  //Add key followed by each field
  url += String(FIRST_SEP) + String("key");
  url += String(LABEL_DATA_SEP) + String(WRITE_API_KEY);
  
  url += String(OTHER_SEP) + String(LABEL1);
  url += String(LABEL_DATA_SEP) + String(SHT2x.GetTemperature());
  
  url += String(OTHER_SEP) + String(LABEL2);
  url += String(LABEL_DATA_SEP) + String(bmp.readTemperature());
  
  url += String(OTHER_SEP) + String(LABEL3);
  url += String(LABEL_DATA_SEP) + String(bmp.readPressure() / 100);
  
  url += String(OTHER_SEP) + String(LABEL4);
  url += String(LABEL_DATA_SEP) + String(SHT2x.GetHumidity());

  return url;  
}

void sendURLData(String url)
{
  char result[20] = "";
  gprsbee.doHTTPGET(APN, APN_USERNAME, APN_PASSWORD, url.c_str(), result, sizeof(result));
  
  Serial.println("Received: " + String(result));
}

String getDateTime()
{
  String dateTimeStr;
  
  //Create a DateTime object from the current time
  DateTime dt(rtc.makeDateTime(rtc.now().getEpoch()));

  //Convert it to a String
  dt.addToString(dateTimeStr);
  
  return dateTimeStr;  
}

uint32_t getNow()
{
  return millis();
}
