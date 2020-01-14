/*
 *  This sketch demonstrates how to scan WiFi networks. 
 *  The API is almost the same as with the WiFi Shield library, 
 *  the most obvious difference being the different file you need to include:
 */
#include "ESP8266WiFi.h"
#include <Arduino.h>
#include "tempsensor.h"
#include "MyESP.h"
#include "utils.h"
#include "version.h"
#include "display.h"

//DS18 temperature sensor config
#define DS18_GPIO     D5 //default pin
#define DS18_PARASITE false //default not parasite

//// Function definitions:
bool LoadSaveCallback(MYESP_FSACTION action, JsonObject settings);
bool SetListCallback(MYESP_FSACTION action, uint8_t wc, const char * setting, const char * value);
void OTACallback_pre();
void OTACallback_post();
void TelnetCallback(uint8_t event);
void TelnetCommandCallback(uint8_t wc, const char * commandLine);
void MQTTCallback(unsigned int type, const char * topic, const char * message);

static const command_t project_cmds[] PROGMEM = {
    {true, "led <on | off>", "toggle status LED on/off"},
    {true, "led_gpio <gpio>", "set the LED pin. Default is the onboard LED 2. For external D1 use 5"},
    {false, "info", "show current captured on the devices"},

};
uint8_t _project_cmds_count = ArraySize(project_cmds);

typedef struct {
    uint32_t timestamp;      // for internal timings, via millis()
    uint8_t  ds18Sensors;    // count of dallas sensors
    DS18 ds18;               //ds18 object
    MyESP myESP;
    Display display;
//    Service service;
} Admin;


//Local administration object of struct
Admin m_admin;


void setup() {
    // set up myESP for Wifi, MQTT, MDNS and Telnet callbacks
    m_admin.myESP.setTelnet(TelnetCommandCallback, TelnetCallback);      // set up Telnet commands
    //m_admin.myESP.setWIFI(WIFICallback);                                 // wifi callback
    m_admin.myESP.setMQTT(MQTTCallback);                                 // MQTT ip, username and password taken from the SPIFFS settings
    m_admin.myESP.setSettings(LoadSaveCallback, SetListCallback, false); // default is Serial off
    m_admin.myESP.setOTA(OTACallback_pre, OTACallback_post);             // OTA callback which is called when OTA is starting and stopping
    m_admin.myESP.begin(APP_HOSTNAME, APP_NAME, APP_VERSION, APP_URL, APP_UPDATEURL);
  //  m_admin.service.setupServices(); //Connect all required services (web / telnet / OTA / MQTT)
    m_admin.ds18Sensors = m_admin.ds18.setup(DS18_GPIO, DS18_PARASITE); // returns #sensors
    m_admin.display.initialize();
}

void loop() {
    m_admin.myESP.loop(); //Keep WiFI, MQTT and stuf active..
    /* Process sensors if any */
    if (m_admin.ds18Sensors) {
        m_admin.ds18.loop();
    }

    m_admin.display.Process();
}



// callback for loading/saving settings to the file system (SPIFFS)
bool LoadSaveCallback(MYESP_FSACTION action, JsonObject settings) {
#ifdef EXAMPLE_SAVE_LOAD
    if (action == MYESP_FSACTION_LOAD) {
        // check for valid json
        if (settings.isNull()) {
            myDebug_P(PSTR("Error processing json settings"));
            return false;
        }

        // serializeJsonPretty(settings, Serial); // for debugging

        EMSESP_Settings.led             = settings["led"];
        EMSESP_Settings.led_gpio        = settings["led_gpio"] | EMSESP_LED_GPIO;
        EMSESP_Settings.dallas_gpio     = settings["dallas_gpio"] | EMSESP_DALLAS_GPIO;
        EMSESP_Settings.dallas_parasite = settings["dallas_parasite"] | EMSESP_DALLAS_PARASITE;
        EMSESP_Settings.shower_timer    = settings["shower_timer"];
        EMSESP_Settings.shower_alert    = settings["shower_alert"];
        EMSESP_Settings.publish_time    = settings["publish_time"] | DEFAULT_PUBLISHTIME;

        EMSESP_Settings.listen_mode = settings["listen_mode"];
        ems_setTxDisabled(EMSESP_Settings.listen_mode);

        EMSESP_Settings.tx_mode = settings["tx_mode"] | EMS_TXMODE_DEFAULT; // default to 1 (generic)
        ems_setTxMode(EMSESP_Settings.tx_mode);

        return true;
    }

    if (action == MYESP_FSACTION_SAVE) {
        settings["led"]             = EMSESP_Settings.led;
        settings["led_gpio"]        = EMSESP_Settings.led_gpio;
        settings["dallas_gpio"]     = EMSESP_Settings.dallas_gpio;
        settings["dallas_parasite"] = EMSESP_Settings.dallas_parasite;
        settings["listen_mode"]     = EMSESP_Settings.listen_mode;
        settings["shower_timer"]    = EMSESP_Settings.shower_timer;
        settings["shower_alert"]    = EMSESP_Settings.shower_alert;
        settings["publish_time"]    = EMSESP_Settings.publish_time;
        settings["tx_mode"]         = EMSESP_Settings.tx_mode;

        return true;
    }
#endif
    return false;
}

// callback for custom settings when showing Stored Settings with the 'set' command
// wc is number of arguments after the 'set' command
// returns true if the setting was recognized and changed and should be saved back to SPIFFs
bool SetListCallback(MYESP_FSACTION action, uint8_t wc, const char * setting, const char * value) {
    bool ok = false;
#ifdef EXAMPLE_SAVE_LOAD
    if (action == MYESP_FSACTION_SET) {
        // led
        if ((strcmp(setting, "led") == 0) && (wc == 2)) {
            if (strcmp(value, "on") == 0) {
                EMSESP_Settings.led = true;
                ok                  = true;
            } else if (strcmp(value, "off") == 0) {
                EMSESP_Settings.led = false;
                ok                  = true;
                // let's make sure LED is really off - For onboard high=off
                digitalWrite(EMSESP_Settings.led_gpio, (EMSESP_Settings.led_gpio == LED_BUILTIN) ? HIGH : LOW);
            } else {
                myDebug_P(PSTR("Error. Usage: set led <on | off>"));
            }
        }

        // test mode
        if ((strcmp(setting, "listen_mode") == 0) && (wc == 2)) {
            if (strcmp(value, "on") == 0) {
                EMSESP_Settings.listen_mode = true;
                ok                          = true;
                myDebug_P(PSTR("* in listen mode. All Tx is disabled."));
                ems_setTxDisabled(true);
            } else if (strcmp(value, "off") == 0) {
                EMSESP_Settings.listen_mode = false;
                ok                          = true;
                ems_setTxDisabled(false);
                myDebug_P(PSTR("* out of listen mode. Tx is now enabled."));
            } else {
                myDebug_P(PSTR("Error. Usage: set listen_mode <on | off>"));
            }
        }

        // led_gpio
        if ((strcmp(setting, "led_gpio") == 0) && (wc == 2)) {
            EMSESP_Settings.led_gpio = atoi(value);
            // reset pin
            pinMode(EMSESP_Settings.led_gpio, OUTPUT);
            digitalWrite(EMSESP_Settings.led_gpio, (EMSESP_Settings.led_gpio == LED_BUILTIN) ? HIGH : LOW); // light off. For onboard high=off
            ok = true;
        }

        // dallas_gpio
        if ((strcmp(setting, "dallas_gpio") == 0) && (wc == 2)) {
            EMSESP_Settings.dallas_gpio = atoi(value);
            ok                          = true;
        }

        // dallas_parasite
        if ((strcmp(setting, "dallas_parasite") == 0) && (wc == 2)) {
            if (strcmp(value, "on") == 0) {
                EMSESP_Settings.dallas_parasite = true;
                ok                              = true;
            } else if (strcmp(value, "off") == 0) {
                EMSESP_Settings.dallas_parasite = false;
                ok                              = true;
            } else {
                myDebug_P(PSTR("Error. Usage: set dallas_parasite <on | off>"));
            }
        }

        // shower timer
        if ((strcmp(setting, "shower_timer") == 0) && (wc == 2)) {
            if (strcmp(value, "on") == 0) {
                EMSESP_Settings.shower_timer = true;
                ok                           = do_publishShowerData();
            } else if (strcmp(value, "off") == 0) {
                EMSESP_Settings.shower_timer = false;
                ok                           = do_publishShowerData();
            } else {
                myDebug_P(PSTR("Error. Usage: set shower_timer <on | off>"));
            }
        }

        // shower alert
        if ((strcmp(setting, "shower_alert") == 0) && (wc == 2)) {
            if (strcmp(value, "on") == 0) {
                EMSESP_Settings.shower_alert = true;
                ok                           = do_publishShowerData();
            } else if (strcmp(value, "off") == 0) {
                EMSESP_Settings.shower_alert = false;
                ok                           = do_publishShowerData();
            } else {
                myDebug_P(PSTR("Error. Usage: set shower_alert <on | off>"));
            }
        }

        // publish_time
        if ((strcmp(setting, "publish_time") == 0) && (wc == 2)) {
            EMSESP_Settings.publish_time = atoi(value);
            ok                           = true;
        }

        // tx_mode
        if ((strcmp(setting, "tx_mode") == 0) && (wc == 2)) {
            uint8_t mode = atoi(value);
            if ((mode >= 1) && (mode <= 3)) { // see ems.h for definitions
                EMSESP_Settings.tx_mode = mode;
                ems_setTxMode(mode);
                ok = true;
            } else {
                myDebug_P(PSTR("Error. Usage: set tx_mode <1 | 2 | 3>"));
            }
        }
    }

    if (action == MYESP_FSACTION_LIST) {
        myDebug_P(PSTR("  led=%s"), EMSESP_Settings.led ? "on" : "off");
        myDebug_P(PSTR("  led_gpio=%d"), EMSESP_Settings.led_gpio);
        myDebug_P(PSTR("  dallas_gpio=%d"), EMSESP_Settings.dallas_gpio);
        myDebug_P(PSTR("  dallas_parasite=%s"), EMSESP_Settings.dallas_parasite ? "on" : "off");
        myDebug_P(PSTR("  tx_mode=%d"), EMSESP_Settings.tx_mode);
        myDebug_P(PSTR("  listen_mode=%s"), EMSESP_Settings.listen_mode ? "on" : "off");
        myDebug_P(PSTR("  shower_timer=%s"), EMSESP_Settings.shower_timer ? "on" : "off");
        myDebug_P(PSTR("  shower_alert=%s"), EMSESP_Settings.shower_alert ? "on" : "off");
        myDebug_P(PSTR("  publish_time=%d"), EMSESP_Settings.publish_time);
    }
#endif
    return ok;
}

// Show command - display stats on an 's' command
void showInfo() {
    myDebug_P(PSTR("%sESP-Valve system stats:%s"), COLOR_BOLD_ON, COLOR_BOLD_OFF);
    myDebug_P(PSTR("  TO BE IMPLMENTED"));
}

// print settings
void _showCommands(uint8_t event) {
    bool      mode = (event == TELNET_EVENT_SHOWSET); // show set commands or normal commands
    command_t cmd;

    // find the longest key length so we can right-align the text
    uint8_t max_len = 0;
    uint8_t i;
    for (i = 0; i < _project_cmds_count; i++) {
        memcpy_P(&cmd, &project_cmds[i], sizeof(cmd));
        if ((strlen(cmd.key) > max_len) && (cmd.set == mode)) {
            max_len = strlen(cmd.key);
        }
    }

    char line[200] = {0};
    for (i = 0; i < _project_cmds_count; i++) {
        memcpy_P(&cmd, &project_cmds[i], sizeof(cmd));
        if (cmd.set == mode) {
            if (event == TELNET_EVENT_SHOWSET) {
                strlcpy(line, "  set ", sizeof(line));
            } else {
                strlcpy(line, "*  ", sizeof(line));
            }
            strlcat(line, cmd.key, sizeof(line));
            for (uint8_t j = 0; j < ((max_len + 5) - strlen(cmd.key)); j++) { // account for longest string length
                strlcat(line, " ", sizeof(line));                             // padding
            }
            strlcat(line, cmd.description, sizeof(line));
            myDebug(line); // print the line
        }
    }
}

// call back when a telnet client connects or disconnects
// we set the logging here
void TelnetCallback(uint8_t event) {
    if (event == TELNET_EVENT_CONNECT) {
        return;
    } else if (event == TELNET_EVENT_DISCONNECT) {
        return;
    } else if ((event == TELNET_EVENT_SHOWCMD) || (event == TELNET_EVENT_SHOWSET)) {
        _showCommands(event);
    }
}

// extra commands options for telnet debug window
// wc is the word count, i.e. number of arguments. Everything is in lower case.
void TelnetCommandCallback(uint8_t wc, const char * commandLine) {
    bool ok = false;
    // get first command argument
    char * first_cmd = strtok((char *)commandLine, ", \n");

    if (strcmp(first_cmd, "info") == 0) {
        showInfo();
        ok = true;
    }

    // check for invalid command
    if (!ok) {
        myDebug_P(PSTR("Unknown command or wrong number of arguments. Use ? for help."));
    }
}

// OTA callback when the OTA process starts
void OTACallback_pre() {
    
}

// OTA callback when the OTA process finishes
void OTACallback_post() {
    
}


// MQTT Callback to handle incoming/outgoing changes
void MQTTCallback(unsigned int type, const char * topic, const char * message) {
    // we're connected. lets subscribe to some topics
    if (type == MQTT_CONNECT_EVENT) {
#ifdef MQTT_EXAMPLES
        // subscribe to the 4 heating circuits for receiving setpoint temperature and modes
        char topic_s[50];
        char buffer[4];
        for (uint8_t hc = 1; hc <= EMS_THERMOSTAT_MAXHC; hc++) {
            strlcpy(topic_s, TOPIC_THERMOSTAT_CMD_TEMP, sizeof(topic_s));
            strlcat(topic_s, itoa(hc, buffer, 10), sizeof(topic_s));
            myESP.mqttSubscribe(topic_s);

            strlcpy(topic_s, TOPIC_THERMOSTAT_CMD_MODE, sizeof(topic_s));
            strlcat(topic_s, itoa(hc, buffer, 10), sizeof(topic_s));
            myESP.mqttSubscribe(topic_s);
        }

        // generic incoming MQTT command for Thermostat
        // this is used for example for setting daytemp, nighttemp, holidaytemp
        myESP.mqttSubscribe(TOPIC_THERMOSTAT_CMD);

        // generic incoming MQTT command for Boiler
        // this is used for example for comfort, flowtemp
        myESP.mqttSubscribe(TOPIC_BOILER_CMD);

        // these two need to be unqiue topics
        myESP.mqttSubscribe(TOPIC_BOILER_CMD_WWACTIVATED);
        myESP.mqttSubscribe(TOPIC_BOILER_CMD_WWTEMP);

        // generic incoming MQTT command for EMS-ESP
        // this is used for example for shower_coldshot
        myESP.mqttSubscribe(TOPIC_GENERIC_CMD);

        // shower data
        // for receiving shower_Timer and shower_alert switches
        myESP.mqttSubscribe(TOPIC_SHOWER_DATA);
#endif
        return;
    }

    // handle incoming MQTT publish events
    if (type != MQTT_MESSAGE_EVENT) {
        return;
    }
#ifdef MQTT_EXAMPLES
    // check first for generic commands
    if (strcmp(topic, TOPIC_GENERIC_CMD) == 0) {
        // convert JSON and get the command
        StaticJsonDocument<100> doc;
        DeserializationError    error = deserializeJson(doc, message); // Deserialize the JSON document
        if (error) {
            myDebug_P(PSTR("[MQTT] Invalid command from topic %s, payload %s, error %s"), topic, message, error.c_str());
            return;
        }
        const char * command = doc["cmd"];

        // Check whatever the command is and act accordingly
        if (strcmp(command, TOPIC_SHOWER_COLDSHOT) == 0) {
            _showerColdShotStart();
            return;
        }

        return; // no match for generic commands
    }

    // example
    if (strcmp(topic, "BLALAT") == 0) {
        uint8_t t = atoi((char *)message);
        //Perform Action
        publishValues(true);
        return;
    }
#endif
}


// Init callback, which is used to set functions and call methods after a wifi connection has been established
void WIFICallback() {
    // This is where we enable the UART service to scan the incoming serial Tx/Rx bus signals
    // This is done after we have a WiFi signal to avoid any resource conflicts
    // system_uart_swap();
}