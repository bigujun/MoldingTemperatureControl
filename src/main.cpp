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

#define START_DALAY 1000


//-------SENSOR

class Termopar{
private:
        uint8_t pin;
        uint16_t leituiras = 1000;

public:
        struct Calibrate{
                long in_min;
                long in_max;
                long out_min;
                long out_max;
        }calibrate{0,1023,0,1023};

        Termopar(uint8_t _pin){
                pin=_pin;
        }

        void begin(){
                pinMode(pin,INPUT);
        }

        float read(){
                unsigned long value = 0;
                for(uint16_t i=0;i<leituiras;i++){
                        value += analogRead(pin);
                }
                return map(value/leituiras,calibrate.in_min,calibrate.in_max,calibrate.out_min,calibrate.out_max);
        }

        void setLeituras(uint16_t _leituras){
                leituiras = _leituras;
        }

}termopar(PINS::SENSOR1);


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
                long time_s;
                long temp_c;
        }rampa;
private:
        bool runnning=true;
        uint32_t startTime=0;
        int minute,second;


}controlador;


//---MENU-------------
using namespace Menu;
#define MAX_DEPTH 2
keyMap encBtn_map[]={   {-PINS::BTN1,defaultNavCodes[downCmd].ch},
                        {-PINS::BTN2,defaultNavCodes[upCmd].ch},
                        {-PINS::BTN3,defaultNavCodes[enterCmd].ch}
                };//negative pin numbers use internal pull-up, this is on when low
softKeyIn<3> encButton(encBtn_map);//3 is the number of keys

LiquidCrystal_PCF8574 lcd(0x3F);

#define LEDPIN LED_BUILTIN


int test=50;

MENU(configSensor, "Sensor Temp", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,FIELD(termopar.calibrate.in_min,"MinIn","",0,1023,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,FIELD(termopar.calibrate.out_min,"MinOut","\337C",0,500,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,FIELD(termopar.calibrate.in_max,"MaxIn","\337C",0,1023,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,FIELD(termopar.calibrate.out_max,"MaxOut","",0,500,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,EXIT("<Back")
);

MENU(mainMenu, "Configurações", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,FIELD(controlador.rampa.time_s,"Tempo","s",0,3600,60,1, Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,FIELD(controlador.rampa.temp_c,"Temperatura","\337C",0,500,10,1,Menu::doNothing, Menu::noEvent, Menu::noStyle)
  ,SUBMENU(configSensor)
  ,EXIT("<Back")
);

serialIn serial(Serial);
MENU_INPUTS(in,&serial,&encButton);

MENU_OUTPUTS(out,MAX_DEPTH
  ,SERIAL_OUT(Serial)
  ,LCD_OUT(lcd,{0,0,16,2})//must have 2 items at least
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

void startScreen(){
        lcd.setBacklight(255);
        lcd.setCursor(0, 0);
        lcd.print("Tiggu Cooker");
        lcd.setCursor(0, 1);
        lcd.print("v:1.0");
        delay(START_DALAY);
        nav.showTitle=false;

}


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
        termopar.begin();
        lcd.begin(16,2);
        btnUp.begin();
        btnDown.begin();
        btnEnter.begin();
        startScreen();
}

void loop() {
        nav.poll();
        updateButtons();
        if(controlador.update()){
                // menu.update();
        }
}
