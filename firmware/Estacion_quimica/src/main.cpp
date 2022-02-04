/*
Author: WAC@MODULAR LAB
VERSION: V0.0.1

NAME:
DESCRPIPTIONS:

LIBS:

TODO:

*/

/*-----------------
    Libs
-----------------*/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>


/*----------------
    Defines
-----------------*/
#define SENSOR_PH_1     // sensor ph gravity
#define SENSOR_PH_2     // sensor ph gravity  lab
#define SENSOR_PH_3     // sensor ph aliexpres
#define SENSOR_PH_OPR   // sensor ph gravity
#define SENSOR_DO_1     // sensor DO atlas scientific
#define SENSOR_DO_2     // sensor DO gravity


#define PH_1_PIN     A0// sensor ph gravity
#define PH_2_PIN     A1// sensor ph gravity  lab
#define PH_3_PIN     A2// sensor ph aliexpres
#define PH_4_PIN     A3// sensor ph+opr gravity
#define DOTX         2// sensor DO atlas scientific
#define DORX         3// sensor DO atlas scientific
#define DO_2_PIN     A4// sensor DO gravity
#define NO3_PIN      A5// sensor NO3
#define LED_PIN      13// on board led
#define TEMP_PIN     4

#define N_SAMPLES   10
#define BAUDRATE    9600
#define SAMPLE_TIME 5000
#define RESET       asm("jmp 0x0000")
#define VERSION     "V 0.0.1" // Version (mayor, minor, patch, build)

#define PH_OFFSET 0.6

#define DEBUG     1

#define IDLE    0
#define RUNNING 1

//----------------------------
//    Instances
//----------------------------
const size_t capacity_rx = JSON_OBJECT_SIZE(5) + 50;
DynamicJsonDocument doc_rx(capacity_rx);
const char* json_rx = "{\"cmd\":\"mode\",\"arg\":1}";
DeserializationError error_rx;


SoftwareSerial Serial_DO(DOTX,DORX);
OneWire ow(TEMP_PIN);                //Se establece el pin 2  como bus OneWire
DallasTemperature temp_sens(&ow); //Se declara una variable u objeto para nuestro sensor

/*----------------
    VARIABLES
----------------*/
//Parse var
const char* cmd;
int arg = 0;

int state = RUNNING;

//Sensor var
int ph_sum[] = {0, 0, 0, 0};
int do_sum[] = {0 , 0};
int no3_sum = 0;


float ph_avr[] = {0.0, 0.0, 0.0, 0.0};
float do_avr[] = {0.0 , 0.0};
float no3_avr = 0.0;

int ph_raw[]  = {0,0,0,0};
int do_raw[]  = {0,0};
int no3_raw = 0;

float ph_volt  = 0.0;
float do_volt  = 0.0;
float no3_volt = 0.0;

float temp  = 0.0;

String do_string = "";
bool sensor_string_complete= false;
int ph_pins[]  = {A0,A1,A2,A3};

int counter = 0;

/*----------------
    FUNCTIONS
----------------*/
float getDO(uint8_t ch);
float getpH(uint8_t ch);
float getNO3();
float getTemp();
void readSensors();
void pubSensors();

void setup()
{
  Serial.begin(9600);
  Serial_DO.begin(9600);
  do_string.reserve(30);
  temp_sens.begin();

}

void loop()
{
  readSensors();
  pubSensors();
  delay(SAMPLE_TIME);
}


void readSensors()
{
  ph_avr[0] = getpH(0);
  ph_avr[1] = getpH(1);
  ph_avr[2] = getpH(2);
  ph_avr[3] = getpH(3);

  //do_avr[0] = getDO(0);
  //do_avr[1] = getDO(1);

  //no3_avr = getNO3();

  temp = getTemp();
  //Serial.println(temp);
}

void pubSensors()
{

  StaticJsonDocument<128> doc_tx;

  doc_tx["id"] = "ok";
  doc_tx["time"] = 111231;
  doc_tx["temp"]  = temp;
  JsonArray ph = doc_tx.createNestedArray("ph");
  ph.add(ph_avr[0]);
  ph.add(ph_avr[1]);
  ph.add(ph_avr[2]);
  ph.add(ph_avr[3]);

  //JsonArray do_array = doc_tx.createNestedArray("do");
  //do_array.add(do_avr[0]);
  //do_array.add(do_avr[1]);

  //doc_tx["nh3"]   = no3_avr;


  String json;
  serializeJson(doc_tx, json);
  if(DEBUG)Serial.println(json);

}


float getpH(uint8_t ch)
{
  counter = 0;
  while(counter < N_SAMPLES)
  {
      ph_raw[ch]  =   analogRead(ph_pins[ch]);
      ph_sum[ch]  +=  ph_raw[ch];
      counter     +=  1;
  }

  ph_volt  = 5.0 * (ph_sum[ch]/N_SAMPLES) / 1024.0;
  ph_volt = 3.5*ph_volt-PH_OFFSET;
  if(ph_volt < 0){ ph_volt = 0; }
  ph_sum[ch] = 0;
  //ADD LINE OF CALIBRATION
  return ph_volt;
}

float getDO(uint8_t ch)
{
  if(ch == 1)
  {
    counter = 0;
    while(counter < N_SAMPLES)
    {
        do_raw[1]  =   analogRead(DO_2_PIN);
        do_sum[1]  +=  do_raw[1];
        counter     +=  1;
    }

    do_volt  = 5.0 * (do_sum[1]/N_SAMPLES) / 1024;
    do_sum[1] = 0;
    return do_volt;
  }
  else if(ch ==0)
  {
    while (Serial_DO.available() > 0 && !sensor_string_complete)
    {                                                  //if we see that the Atlas Scientific product has sent a character
      char inchar = (char)Serial_DO.read();              //get the char we just received
      do_string += inchar;                           //add the char to the var called sensorstring
      if (inchar == '\r')
      {                                                //if the incoming character is a <CR>
        sensor_string_complete = true;                 //set the flag
        Serial.println(do_string);
      }
    }
    if (sensor_string_complete == true)
    {
      if (isdigit(do_string[1]))
      {
        Serial.println(do_string.toFloat());
        do_avr[0] = do_string.toFloat();
      }
      do_string = "";                                //clear the string
      sensor_string_complete = false;

    }
                     //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
  }
}

float getNO3()
{
  counter = 0;
  while(counter < N_SAMPLES)
  {
      no3_raw  =   analogRead(NO3_PIN);
      no3_sum  +=  no3_raw;
      counter  +=  1;
  }

  no3_volt  = 5.0 * (no3_sum/N_SAMPLES) / 1024;
  no3_sum = 0;
  //ADD LINE OF CALIBRATION
  return no3_volt;
}

float getTemp()
{
  float t = 0.0;
  //Serial.print("getting temp: ");
  temp_sens.requestTemperatures();
  t  = temp_sens.getTempCByIndex(0);
  //Serial.println(t);
  if (isnan(t)){t = 0.0;}
  return t;
}
