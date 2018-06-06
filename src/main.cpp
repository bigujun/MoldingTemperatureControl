/**
 * @Author: Anderson Juncowski <anderson>
 * @Date:   2018-05-31T02:14:09-03:00
 * @Email:  andersonj@hotmail.rs
 * @Last modified by:   anderson
 * @Last modified time: 2018-05-31T02:17:02-03:00
 */



#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include "PinsController.h"
#include <JC_Button.h>
#include <menu.h>//menu macros and objects
#include <menuIO/PCF8574Out.h>//arduino I2C LCD
#include <menuIO/serialOut.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialIn.h>
#include <menuIO/softKeyIn.h>
#include <EEPROM.h>
#include "Thermocouple.h"

#define START_DALAY 1000

LiquidCrystal_PCF8574 lcd(0x3F);

void updateButtons();

enum TastAtual{
        CONFIG,
        PAINEL
}taskAtual;

//-------SENSOR

Thermocouple termopar(PINS::SENSOR1);


//----------CONTROLE

class Controlador{
public:
        int screenTemp=0;
        char* screenTime = new char[10];
        char* screenStatus = new char[16];

        Controlador(){
                setScreenStatus(CARREGANDO);
        };
        enum Status{
                AQUECENDO,
                CONCLUIDO,
                PAUSADO,
                TEMPORIZADOR,
                ERRO,
                CARREGANDO
        };

        void run(){
                updateButtons();
                if(update()){
                        screen();
                }
        }

        void screen(){
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print(screenTemp);
                lcd.print("\337C");
                lcd.setCursor(11,0);
                lcd.print(screenTime);
                lcd.setCursor(0,1);
                lcd.print(screenStatus);
        }
        void setScreenStatus(Status _status){
                String out;
                switch(_status){
                        case AQUECENDO:
                                out = "AQUECENDO";
                        break;
                        case CONCLUIDO:
                                out = "CONCLUIDO| RST";
                        break;
                        case PAUSADO:
                                out = "PAUSADO";
                        break;
                        case TEMPORIZADOR:
                                out = "TEMPORIZADOR";
                        break;
                        case ERRO:
                                out = "ERRO";
                        break;
                        default:
                                out = "CARREGANDO";
                        break;
                }
                strcpy(screenStatus,out.c_str());
        }

        void setScreenTime(uint8_t minute,uint8_t second){
                String out = String(minute<10?"0":"") + String(minute) + ":" + String(second<10?"0":"") + String(second);
                strcpy(screenTime,out.c_str());
        }

        bool update(){
                bool upAux=false;
                if(!runnning)return false;


                uint16_t passedTime = (millis()-startTime)/1000;
                int minuteNow = passedTime / 60;
                int secondNow = passedTime % 60;
                if((minuteNow!=minute) | (secondNow!=second)){
                        minute = minuteNow;
                        second = secondNow;
                        setScreenTime(minute, second);
                        upAux = true;
                        screenTemp = termopar.read();
                        if(screenTemp>rampa.temp_c || passedTime > rampa.time_s){
                                setScreenStatus(CONCLUIDO);
                        }else if(screenTemp>rampa.temp_c){
                                setScreenStatus(TEMPORIZADOR);
                        }else if(passedTime > rampa.time_s){
                                setScreenStatus(ERRO);
                        }else{
                                setScreenStatus(AQUECENDO);
                        }
                }
                return upAux;
        }

        void restart(){
                startTime = millis();
        }

        void setRunning(bool run){
                runnning = run;
        }

        struct Rampa{
                int time_s;
                int temp_c;
        }rampa;
private:
        bool runnning=true;
        uint32_t startTime=0;
        int minute,second;


}controlador;


//-----SAVE AND LOAD CONFIGS

static const int EEPROM_KEY=8520; //Chave aleatória verificar se já salvou algo

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}

struct ConfisToSave{
        int key;
        typeof(Controlador::rampa.time_s) time;
        typeof(Controlador::rampa.temp_c) temp;
        typeof(Thermocouple::calibrate.in_min) in_min;
        typeof(Thermocouple::calibrate.in_max) in_max;
        typeof(Thermocouple::calibrate.out_min) out_min;
        typeof(Thermocouple::calibrate.out_max) out_max;
}configsToSave;

result saveConfigs(eventMask e) {

        configsToSave.key = EEPROM_KEY;
        configsToSave.time = controlador.rampa.time_s;
        configsToSave.temp = controlador.rampa.temp_c;
        configsToSave.in_min = termopar.calibrate.in_min;
        configsToSave.in_max = termopar.calibrate.in_max;
        configsToSave.out_min = termopar.calibrate.out_min;
        configsToSave.out_max = termopar.calibrate.out_max;
        EEPROM_writeAnything(0, configsToSave);
        DEBUG_PRINTLN("SAVE");
        return quit;
}

void defaultConfig(){
        controlador.rampa.time_s = 5*60;
        controlador.rampa.temp_c = 110;
        termopar.calibrate.in_min=0;
        termopar.calibrate.in_max=1023;
        termopar.calibrate.out_min=0;
        termopar.calibrate.out_max=400;
}

void loadConfig(){
        EEPROM_readAnything(0, configsToSave);
        if(configsToSave.key == EEPROM_KEY){
                controlador.rampa.time_s = configsToSave.time;
                controlador.rampa.temp_c =configsToSave.temp;
                termopar.calibrate.in_min=configsToSave.in_min;
                termopar.calibrate.in_max=configsToSave.in_max;
                termopar.calibrate.out_min=configsToSave.out_min;
                termopar.calibrate.out_max=configsToSave.out_max;
        }else{
                defaultConfig();
        }
}





//---MENU-------------
using namespace Menu;
#define MAX_DEPTH 2
keyMap encBtn_map[]={   {-PINS::BTN1,defaultNavCodes[downCmd].ch},
                        {-PINS::BTN2,defaultNavCodes[upCmd].ch},
                        {-PINS::BTN3,defaultNavCodes[enterCmd].ch}
                };//negative pin numbers use internal pull-up, this is on when low
softKeyIn<3> encButton(encBtn_map);//3 is the number of keys



#define LEDPIN LED_BUILTIN


int test=50;

MENU(configSensor, "Sensor Config", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,FIELD(termopar.calibrate.in_min,"MinIn","",0,1023,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,FIELD(termopar.calibrate.out_min,"MinOut","mV",0,5000,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,FIELD(termopar.calibrate.in_max,"MaxIn","",0,1023,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,FIELD(termopar.calibrate.out_max,"MaxOut","mV",0,5000,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,EXIT("<Back")
);

MENU(mainMenu, "Configurações", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,FIELD(controlador.rampa.time_s,"Tempo","s",0,3600,60,1, Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,FIELD(controlador.rampa.temp_c,"Tempera.","\337C",0,500,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,SUBMENU(configSensor)
  ,OP("Salvar Config",saveConfigs,enterEvent)
  ,OP("Padrão Config",defaultConfig,enterEvent)
  ,EXIT(" <-Sair")
);

serialIn serial(Serial);
MENU_INPUTS(in,&serial,&encButton);

MENU_OUTPUTS(out,MAX_DEPTH
  ,SERIAL_OUT(Serial)
  ,LCD_OUT(lcd,{0,0,16,2})//must have 2 items at least
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);



//-----BOTOES---------

const uint32_t LONG_PRESS(3000);

Button btnDown(PINS::BTN1,25,true);
Button btnUp(PINS::BTN2,25,true);
Button btnEnter(PINS::BTN3,25,true);

void updateButtons(){
        btnUp.read();
        btnDown.read();
        btnEnter.read();
        static bool upLong,downLong,enterLong;

        if(btnUp.wasReleased()){
                if(upLong){
                        DEBUG_PRINT("LONG_");
                        // menu.call_function(FunctionsTypes::longUp);

                }else{
                        DEBUG_PRINT("SHORT_");
                        // menu.call_function(FunctionsTypes::up);
                }
                DEBUG_PRINTLN("UP");
        }
        if(btnDown.wasReleased()){
                if(downLong){
                        DEBUG_PRINT("LONG_");
                        // menu.call_function(FunctionsTypes::longDown);
                }else{
                        DEBUG_PRINT("SHORT_");
                        // menu.call_function(FunctionsTypes::down);
                }
                DEBUG_PRINTLN("DOWN");
        }
        if(btnEnter.wasReleased()){
                if(enterLong){
                        DEBUG_PRINT("LONG_");
                        // menu.call_function(FunctionsTypes::longEnter);
                        taskAtual = CONFIG;
                        nav.idleOff();
                }else{
                        DEBUG_PRINT("SHORT_");
                        // menu.call_function(FunctionsTypes::enter);
                        // menu.switch_focus(true);
                }
                DEBUG_PRINTLN("ENTER");
        }

        upLong = btnUp.pressedFor(LONG_PRESS);
        downLong = btnDown.pressedFor(LONG_PRESS);
        enterLong = btnEnter.pressedFor(LONG_PRESS);
}


void setup() {
        DEBUG_BEGIN(115200);
        DEBUG_PRINTLN("STARTING");
        PinsController.setup();
        analogReference(INTERNAL);
        termopar.begin();
        lcd.begin(16,2);
        btnUp.begin();
        btnDown.begin();
        btnEnter.begin();

        lcd.setBacklight(255);
        lcd.setCursor(0, 0);
        lcd.print("Tiggu Cooker");
        lcd.setCursor(0, 1);
        lcd.print("v:1.0");
        delay(START_DALAY);
        nav.showTitle=false;
        taskAtual = PAINEL;
        loadConfig();
}


void loop() {

        switch (taskAtual) {
                case CONFIG:
                        nav.poll();
                break;
                case PAINEL:
                        controlador.run();
                break;
                default:
                        taskAtual=PAINEL;
        }
        if(nav.sleepTask) {
          taskAtual = PAINEL;
        }
}
