/**
 * @Author: Anderson Juncowski <anderson>
 * @Date:   2018-05-31T02:14:09-03:00
 * @Email:  andersonj@hotmail.rs
 * @Last modified by:   anderson
 * @Last modified time: 2018-05-31T02:17:02-03:00
 */



#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "PinsController.h"
#include <JC_Button.h>
#include <LiquidMenu.h>

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
                long out_max;
                long out_min;
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

//CONFIG

class ConfigHelper{
public:
        class ConfigNumber{
        public:
                ConfigNumber(char* _name, long* _value){};
                const char* name;
                const long* value;

                void up(){
                        value++;
                }
                void down(){
                        value--;
                }
        };

        ConfigNumber termoparOutMin =  ConfigNumber((char*)"TempOutMIN", &termopar.calibrate.out_min);
        ConfigNumber termoparOutMax =  ConfigNumber((char*)"TempOutMAX", &termopar.calibrate.out_max);
        ConfigNumber temperaturaObjetivo =  ConfigNumber((char*)"TempObj", &controlador.rampa.temp_c);
        ConfigNumber tempoObjetivo =  ConfigNumber((char*)"TimeObj", &controlador.rampa.time_s);

        const char*

        static void up(){
                // atual->up();
        }
        static void down(){
                // atual->down();
        }
        static void enter(){

        }
};

//---TELAS--------------

LiquidLine start1(1, 0, "LONG PRESS FOR");
LiquidLine start2(1, 1, "CONFIG");

LiquidScreen startScreen(start1,start2);

LiquidLine main1(1, 0, controlador.screenTemp,"\337C  ", controlador.screenTime);
LiquidLine main2(1, 1, controlador.screenStatus);

LiquidScreen mainScreen(main1,main2);


LiquidLine configSensor(1, 0, "SEN");
LiquidLine configLcd(1, 0, "BRILHO LCD");
LiquidLine configMenu(1,1,"<-- | --> |ENTER");

// LiquidLine configNumber(1,0,ConfigHelper::atual->name," = ",ConfigHelper::atual->value);
LiquidLine configNumberMenu(1,1," - | + | SAVE");

LiquidScreen configScreen(config1,configMenu);

// LiquidLine analogReading_line(0, 0, "Analog: ", termopar.read());
// LiquidScreen secondary_screen(analogReading_line);
LiquidCrystal_I2C lcd(0x3F,16,2);
const byte startingScreen = 1;
LiquidMenu menu(lcd,startingScreen);

enum FunctionsTypes{
        up,down,enter,longUp,longDown,longEnter
};

void xxx(){
        DEBUG_PRINTLN("XXX_UP");
        configScreen.
}


void startMenus(){
        configNumberMenu.attach_function(FunctionsTypes::up, ConfigHelper::up);
        configNumberMenu.attach_function(FunctionsTypes::down,ConfigHelper::down);
        configNumberMenu.attach_function(FunctionsTypes::enter,ConfigHelper::enter);
        config1.attach_function(FunctionsTypes::up, xxx);
        configSensor.attach_function(FunctionsTypes::up, xxx);
        configLcd.attach_function(FunctionsTypes::up, xxx);
        // main2.attach_function(FunctionsTypes::longEnter, teste);
        menu.add_screen(startScreen);
        menu.add_screen(mainScreen);
        menu.add_screen(configScreen);
        menu.add_screen(teste);
        menu.init();
        menu.update();
        menu.set_focusPosition(Position::LEFT);
        delay(START_DALAY);
        menu.change_screen(mainScreen);
        menu.update();
        menu.switch_focus(true);
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
                        menu.call_function(FunctionsTypes::longUp);
                }else{
                        DEBUG_PRINT("SHORT_");
                        menu.call_function(FunctionsTypes::up);
                }
                DEBUG_PRINTLN("UP");
        }
        if(btnDown.wasReleased()){
                if(downLong){
                        DEBUG_PRINT("LONG_");
                        menu.call_function(FunctionsTypes::longDown);
                }else{
                        DEBUG_PRINT("SHORT_");
                        menu.call_function(FunctionsTypes::down);
                }
                DEBUG_PRINTLN("DOWN");
        }
        if(btnEnter.wasReleased()){
                if(enterLong){
                        DEBUG_PRINT("LONG_");
                        menu.call_function(FunctionsTypes::longEnter);
                }else{
                        DEBUG_PRINT("SHORT_");
                        // menu.call_function(FunctionsTypes::enter);
                        menu.switch_focus(true);
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
        lcd.init();
        btnUp.begin();
        btnDown.begin();
        btnEnter.begin();
        startMenus();
}

void loop() {
        updateButtons();
        if(controlador.update()){
                menu.update();
        }
}
