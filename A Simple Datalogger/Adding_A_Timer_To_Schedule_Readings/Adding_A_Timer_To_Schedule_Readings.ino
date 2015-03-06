#include <Wire.h>
#include <SPI.h>

//SODAQ Mbili libraries
#include <RTCTimer.h>
#include <Sodaq_BMP085.h>
#include <Sodaq_SHT2x.h>
#include <Sodaq_DS3231.h>
#include <Sodaq_dataflash.h>

//DataFlash constants
#define NUM_PAGES 4096
#define PAGE_SIZE 528
#define NEW_LINE "\r\n"

//The delay between the sensor readings 
#define READ_DELAY 1000

//Data header
#define DATA_HEADER "TimeDate, TempSHT21, TempBMP, PressureBMP, HumiditySHT21"

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
  
  //Echo the data to the serial connection
  Serial.println(dataRec);
  
  //Save the data record to the log file
  logData(dataRec);
}

void setupSensors()
{
  //Initialise the wire protocol for the TPH sensors
  Wire.begin();
  
  //Initialise the TPH BMP sensor
  bmp.begin();
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
  //Schedule the reading every second
  timer.every(READ_DELAY, takeReading);

  //Instruct the RTCTimer how to get the current time reading
  timer.setNowCallback(getNow);
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

String getDateTime()
{
  String dateTimeStr;
  
  //Todo get DateTime from Sim800
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
