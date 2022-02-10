
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <FastLED.h>



/*----------------
    Defines
-----------------*/
#define SENSOR_PH_1    1 // sensor ph gravity
#define SENSOR_PH_2    2 // sensor ph gravity  lab
#define SENSOR_PH_3    3 // sensor ph aliexpres
#define SENSOR_PH_OPR  4 // sensor ph gravity
#define SENSOR_DO_1    5 // sensor DO atlas scientific
#define SENSOR_DO_2    6 // sensor DO gravity
#define SENSOR_NO3     7 // sensor NO3 aliexpress

#define ANALOG_PIN   A0
#define DOTX         A1 // sensor DO atlas scientific
#define DORX         A2 // sensor DO atlas scientific
#define LED_PIN      A3// on board led
#define TEMP_PIN     6
#define RGB_PIN      4

#define N_SAMPLES   10
#define BAUDRATE    9600
#define SAMPLE_TIME 100
#define RESET       asm("jmp 0x0000")
#define VERSION     "V 1.0.1" // Version (mayor, minor, patch, build)

#define NUM_LEDS  1


#define MV_OFFSET 0.6
#define MV_M      3.5

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
OneWire ow(TEMP_PIN);              //bus de onewire
DallasTemperature temp_sens(&ow); //sensor de temperatura
CRGB rgb_strip[NUM_LEDS];         //RGB strip


/*----------------
    VARIABLES
----------------*/
//Parse var
const char* cmd;
int arg = 0;

//Sensor variables
int val_raw       = 0;
float val_avr     = 0.0;
float val_sensor  = 0.0;
float val_mv      = 0.0;

float m = MV_M;
float n = MV_OFFSET;

float temp        = 0.0;
float temp_avr    = 0.0;

String do_string = "";
bool sensor_string_complete= false;

const char* response = "ok";

int state           = RUNNING;
long sample_time    = SAMPLE_TIME;
uint8_t sensor_type = SENSOR_PH_1;

/*----------------
    FUNCTIONS
----------------*/
void getDOAtlas();
void getAnalogSensor();
void getTemp();
void readSensors();
void pubSensors();
void setSensorType(uint8_t new_type);
void setSampleTime(long new_time);
void setOffset(float new_n);
void setSlope(float new_m);
void processCmd();
void updateColor(int val);

void setup()
{
  Serial.begin(9600);
  delay(10);
  Serial_DO.begin(9600);
  do_string.reserve(30);
  delay(10);

  FastLED.addLeds<NEOPIXEL,RGB_PIN>(rgb_strip,NUM_LEDS);
  pinMode(RGB_PIN,OUTPUT);

  temp_sens.begin();
}

void loop()
{
  readSensors();
  updateColor(int(val_sensor));
  pubSensors();
  delay(sample_time);
  if( Serial.available() > 0 ){ processCmd(); }
}

void readSensors()
{
  if(sensor_type == SENSOR_DO_1 ) { getDOAtlas();}
  else{ getAnalogSensor();}
  getTemp();
  response = "ok";
}

void updateColor(int val)
{
  int v = constrain(val, 0, 15);
  switch (v) {
    case 0:
      rgb_strip[0] = CRGB::Red;
      break;
    case 1:
      rgb_strip[0] = CRGB::OrangeRed;
      break;
    case 2:
      rgb_strip[0] = CRGB::Orange;
      break;
    case 3:
      rgb_strip[0] = CRGB::Gold;
      break;
    case 4:
      rgb_strip[0] = CRGB::GreenYellow;
      break;
    case 5:
      rgb_strip[0] = CRGB::GreenYellow;
      break;
    case 6:
      rgb_strip[0] = CRGB::GreenYellow;
      break;
    case 7:
      rgb_strip[0] = CRGB::GreenYellow;
      break;
    case 8:
      rgb_strip[0] = CRGB::GreenYellow;
      break;
    case 9:
      rgb_strip[0] = CRGB::GreenYellow;
      break;
    case 10:
      rgb_strip[0] = CRGB::RoyalBlue;
      break;
    case 11:
      rgb_strip[0] = CRGB::DarkBlue;
      break;
    case 12:
      rgb_strip[0] = CRGB::DarkSlateBlue;
      break;
    case 13:
      rgb_strip[0] = CRGB::DarkOrchid;
      break;
    case 14:
      rgb_strip[0] = CRGB::Indigo;
      break;
    default:
      rgb_strip[0] = CRGB::Black;
      break;
  }
  FastLED.show();
}

void pubSensors()
{

  StaticJsonDocument<128> doc_tx;

  doc_tx["id"]    = "wac01";
  doc_tx["time"]  = millis();
  doc_tx["temp"]  = temp;
  doc_tx["mv"]    = val_mv;
  doc_tx["val"]   = val_sensor;
  doc_tx["resp"]  = response ;

  String json;
  serializeJson(doc_tx, json);
  Serial.println(json);
  Serial.println("");

}

void getAnalogSensor()
{
  val_raw =   analogRead(ANALOG_PIN);
  val_avr = val_avr*0.94 + val_raw*0.06;

  val_mv      = 5.0 * ( val_avr/ 1024.0);
  val_sensor  = MV_M*val_mv-MV_OFFSET;
  if(val_sensor < 0){ val_sensor = 0; }

}

void getDOAtlas()
{
    while (Serial_DO.available() > 0 && !sensor_string_complete)
    {                                                  //if we see that the Atlas Scientific product has sent a character
      char inchar = (char)Serial_DO.read();              //get the char we just received
      do_string += inchar;                           //add the char to the var called sensorstring
      if (inchar == '\r')
      {                                                //if the incoming character is a <CR>
        sensor_string_complete = true;                 //set the flag
        if(DEBUG) Serial.println(do_string);
      }
    }
    if (sensor_string_complete == true)
    {
      if (isdigit(do_string[1]))
      {
        if(DEBUG)Serial.println(do_string.toFloat());
        val_sensor = do_string.toFloat();
      }
      do_string = "";                                //clear the string
      sensor_string_complete = false;
    }
                     //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
}

void getTemp()
{
  //Serial.print("getting temp: ");
  temp_sens.requestTemperatures();
  temp  = temp_sens.getTempCByIndex(0);
  //Serial.println(t);
  if (isnan(temp)){temp = 0.0;}

}

void setOffset(float new_n)
{
  n = new_n;
  response = "n_ok";
}

void setSlope(float new_m)
{
  m = new_m;
  response = "m_ok";
}

void setSampleTime(long new_time)
{
  sample_time = new_time;
}

void setSensorType(uint8_t new_type)
{
  sensor_type = new_type;
  response = "type_ok";
}

void processCmd()
{
  //check for error
  error_rx = deserializeJson(doc_rx, Serial);
  if (error_rx && DEBUG)
  {
    Serial.println("testing");
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error_rx.c_str());
    return;
  }

  //parsing incoming msg
  cmd = doc_rx["cmd"];
  arg = doc_rx["arg"];
  //Serial.println("Mensaje recibido");
  //prossesing incoming command
  if(strcmp(cmd,"offset")==0)
  {
    setOffset(float(arg));
    pubSensors();
  }
  else if(strcmp(cmd,"slope")==0)
  {
    setSlope(float(arg));
    pubSensors();
  }
  else if(strcmp(cmd,"time")==0)
  {
    setSampleTime(long(arg));
    pubSensors();
  }
  else if(strcmp(cmd,"type")==0)
  {
    setSensorType(uint8_t(arg));
    pubSensors();
  }
  else if(strcmp(cmd,"reset")==0)
  {
    RESET;
  }
  else
  {
    if(DEBUG){Serial.println("Command not valid");}
  }
  cmd = "";
}
