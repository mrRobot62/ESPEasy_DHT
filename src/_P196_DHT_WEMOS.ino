//#######################################################################################################
//######################## Plugin 196: Temperature and Humidity sensor DHT 11
//(Adafruit DHT Unified) #####################
//#######################################################################################################

#define PLUGIN_196
#define PLUGIN_ID_196 196
#define PLUGIN_NAME_196 "Temperature & Humidity - DHT(WeMos)"
#define PLUGIN_VALUENAME1_196 "Temperature"
#define PLUGIN_VALUENAME2_196 "Humidity"
#define PLUGIN_VALUENAME3_196 "Fahrenheit"
#define PLUGIN_VALUENAME4_196 "HeatIndex"

boolean Plugin_196_init = false;

#include <DHT.h>

// DHT dht(DHT_PIN,DHT_TYPE);
DHT *dht;
long sensor_delay_ms;
long last_millis;
// unsigned long last_millis;

boolean Plugin_196_DHT_init(struct EventStruct *event) {
  String log = "Plugin_196_DHT_init: ";
  uint8_t pin = Settings.TaskDevicePin1[event->TaskIndex];
  uint8_t dev = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
  // pin = D4;
  // dev = 2;
  log += "DHT";
  log += String(dev) + " on pin ";
  log += String(pin);
  log += " - PIN(";
  log += String(Settings.TaskDevicePin1[event->TaskIndex]) + ")";
  log += " - DEV(";
  log += String(Settings.TaskDevicePluginConfig[event->TaskIndex][0]) + ")";

  addLog(LOG_LEVEL_INFO, log);
  //
  dht = new DHT(pin, dev);
  if (dht != NULL) {
    dht->begin();
    readSensor(event);
    Dump_DHT_Sensor(event);
  }
}

void readSensor(struct EventStruct *event) {
  float t, h, f, hi;
  t = dht->readTemperature();
  h = dht->readHumidity();
  f = dht->readTemperature(true);
  hi = dht->computeHeatIndex(t, h, false);
  UserVar[event->BaseVarIndex] = t;
  UserVar[event->BaseVarIndex + 1] = h;
  UserVar[event->BaseVarIndex + 2] = f;
  UserVar[event->BaseVarIndex + 3] = hi;
}

void Dump_DHT_Sensor(struct EventStruct *event) {
  String log = "read DHT-Sensor --> ";
  if (dht != NULL) {
    log = "Temp: " + String(UserVar[event->BaseVarIndex]) + "C";
    log += "\tH: " + String(UserVar[event->BaseVarIndex + 1]) + "%";
    log += "\tHeatIndex: " + String(UserVar[event->BaseVarIndex + 3]);
    addLog(LOG_LEVEL_INFO, log);
  } else {
    addLog(LOG_LEVEL_ERROR, "can't DUMP sensor - no object found");
    addLog(LOG_LEVEL_INFO, "--- TRY 1 ---");
    Plugin_196_DHT_init(event);
  }
}

boolean Plugin_196(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {
  case PLUGIN_DEVICE_ADD: {
    Device[++deviceCount].Number = PLUGIN_ID_196;
    Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
    Device[deviceCount].VType = SENSOR_TYPE_QUAD;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = false;
    Device[deviceCount].ValueCount = 4;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = true;
    addLog(LOG_LEVEL_INFO, "PLUGIN_DEVICE_ADD");
    break;
  }

  case PLUGIN_GET_DEVICENAME: {
    string = F(PLUGIN_NAME_196);
    break;
  }

  case PLUGIN_GET_DEVICEVALUENAMES: {
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0],
             PSTR(PLUGIN_VALUENAME1_196));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1],
             PSTR(PLUGIN_VALUENAME2_196));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2],
             PSTR(PLUGIN_VALUENAME3_196));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3],
             PSTR(PLUGIN_VALUENAME4_196));
    break;
  }

  case PLUGIN_INIT: {
    String log = "";
    Plugin_196_DHT_init(event);
    log = "initialize P196_DHT_WEMOS plugin... ";
    addLog(LOG_LEVEL_INFO, log);
    success = true;
    break;
  }
  case PLUGIN_WEBFORM_LOAD: {
    byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
    String options[3];
    options[0] = F("DHT11");
    options[1] = F("DHT21 (AM2301)");
    options[2] = F("DHT22 (AM2302)");
    int optionValues[3];
    optionValues[0] = 11;
    optionValues[1] = 21;
    optionValues[2] = 22;
    string += F("<TR><TD>DHT Type:<TD><select name='plugin_196_type'>");
    for (byte x = 0; x < 3; x++) {
      string += F("<option value='");
      string += optionValues[x];
      string += "'";
      if (choice == optionValues[x])
        string += F(" selected");
      string += ">";
      string += options[x];
      string += F("</option>");
    }
    string += F("</select>");
    addLog(LOG_LEVEL_INFO, "PLUGIN_WEBFORM_LOAD");
    Plugin_196_DHT_init(event);
    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE: {
    String log = "";
    String plug1 = WebServer.arg("plugin_196_type");
    Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plug1.toInt();
    addLog(LOG_LEVEL_INFO, "PLUGIN_WEBFORM_SAVE");
    Plugin_196_DHT_init(event);
    success = true;
    break;
  }

  case PLUGIN_ONCE_A_SECOND: {
    long wait = Settings.TaskDeviceTimer[event->TaskIndex] * 1000;
    if ((millis() - last_millis) >= wait) {
      readSensor(event);
      Dump_DHT_Sensor(event);
      last_millis = millis();
    }
    success = true;
    break;
  }

  case PLUGIN_READ: {
    String log = "-------- PLUGIN_READ --------";
    addLog(LOG_LEVEL_INFO, log);
    readSensor(event);
    Dump_DHT_Sensor(event);
    success = true;
    break;
  }
  }
  return success;
}
