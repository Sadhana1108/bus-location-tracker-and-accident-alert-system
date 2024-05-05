#include <TinyGPS++.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <SoftwareSerial.h>

SoftwareSerial GPS_SoftSerial(5, 6);
SoftwareSerial gsmSerial(8, 9);

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

TinyGPSPlus gps;

double lat_val, lng_val ,lat_org,lon_org;

String msg1 = "Fire Detected";
String msg2 = "Smoke Detected";
String msg3 = "Accident Detected";

bool loc_valid;


const int fire_sensor=2;
const int smoke_sensor=A0;

int smoke,fire;

int sm=1,ac=1,fi=1;

int xsample=0;
int ysample=0;
int zsample=0;

int x_adc_value, y_adc_value, z_adc_value; 

#define samples 10

#define minVal -10
#define MaxVal 10

void setup() 
{
  Serial.begin(9600);
  gsmSerial.begin(9600);
  GPS_SoftSerial.begin(9600);
  
  accel.begin();
  accel.setRange(ADXL345_RANGE_16_G);

  pinMode(smoke_sensor,INPUT);
  pinMode(fire_sensor,INPUT);

  sensors_event_t event; 
  accel.getEvent(&event);
  
  for(int i=0;i<samples;i++)
  {
    xsample+=event.acceleration.x;
    ysample+=event.acceleration.y;
    zsample+=event.acceleration.z;
  }
  xsample/=samples;
  ysample/=samples;
  zsample/=samples;
}

void loop() 
{
  smoke = analogRead(smoke_sensor);
  fire = digitalRead(fire_sensor);
//  Serial.println(smoke);
//  Serial.println(fire);

  if(smoke > 300)
  {
//    Serial.println("Smoke Detected");
    sm = 0;
  }
  else
  {
//    Serial.println("Smoke Safe");
    sm = 1;
  }

  if(fire == 1)
  {
    fi = 0;
//    Serial.println("Fire Detected");
  }
  else
  {
    fi = 1;
//    Serial.println("Fire Safe");
  }
  
  smartDelay(1000);
  loc_valid = gps.location.isValid();
  lat_val = gps.location.lat();
  lng_val = gps.location.lng();

  if (!loc_valid)
  {          
//    Serial.print("Latitude : ");
//    Serial.print("*****");
//    Serial.print("  : Longitude: ");
//    Serial.println("*****");
  }
  else
  {
    lat_org = lat_val;
    lon_org = lng_val;
//    Serial.print("Latitude: ");
//    Serial.print(lat_org, 6);
//    Serial.print("  : Longitude: ");
//    Serial.println(lon_org, 6);
  }

  sensors_event_t event; 
  accel.getEvent(&event);
  x_adc_value=event.acceleration.x;
  y_adc_value=event.acceleration.y;
  z_adc_value=event.acceleration.z;

  int xValue=xsample-x_adc_value;
  int yValue=ysample-y_adc_value;
  int zValue=zsample-z_adc_value;
  if(xValue < minVal || xValue > MaxVal  || yValue < minVal || yValue > MaxVal  || zValue < minVal || zValue > MaxVal)
  {
//    Serial.println("Accident Detected");
    ac=0;
  }
  else
  {
    ac=1;
  }

  StaticJsonDocument<200> doc;
  doc["count1"] = lat_org;
  doc["count2"] = lon_org;
  doc["count3"] = sm;
  doc["count4"] = fi;
  doc["count5"] = ac;
  serializeJson(doc, Serial);

  if(fi == 0 || sm == 0 || ac == 0)
  {
    gsmSerial.println("AT+CMGF=1");
    delay(1000);
    gsmSerial.println("AT+CMGS=\"8374947859\"\r");
    delay(1000);
    if(fi == 0)
    {
      gsmSerial.println(msg1);
    }
    if(sm == 0)
    {
      gsmSerial.println(msg2);
    }
    if(ac == 0)
    {
      gsmSerial.println(msg3);
    }
    gsmSerial.print("NEED HELP @ http://maps.google.com/maps?q=");
    gsmSerial.print(lat_org,6);
    gsmSerial.print(",");
    gsmSerial.print(lon_org,6);
    delay(1000);
    gsmSerial.println((char)26);// ASCII code of CTRL+Z
    delay(1000);
  }
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (GPS_SoftSerial.available())  /* Encode data read from GPS while data is available on serial port */
      gps.encode(GPS_SoftSerial.read());
/* Encode basically is used to parse the string received by the GPS and to store it in a buffer so that information can be extracted from it */
  } while (millis() - start < ms);
}
