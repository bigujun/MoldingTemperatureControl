/**
 * @Author: Anderson Juncowski <bigujun>
 * @Date:   2018-04-10T19:25:47-03:00
 * @Email:  andersonj@hotmail.rs
 * @Last modified by:   bigujun
 * @Last modified time: 2018-04-14T17:28:26-03:00
 */



#ifndef PINS_CONSTROLLER
#define  PINS_CONSTROLLER
#include <Arduino.h>
#include "DebugUtils.h"


//TODO COLOCAR TYPEDEF E USAR NA FUNÇAO

typedef enum PINS{
        BTN1 = 8,
        BTN2 = 9,
        BTN3 = 10,
        SENSOR1 = A1,
        SENSOR2 = A2,
        LED_EA = 5,   //PWM
        LED_EB = 6,  //PWM
        LED_DA = 7,
        LED_DB = 8,
        SERVO = 9
}Pin;


class PinControl{
private:
        uint8_t pin;
        uint8_t mode;
        bool inverse=false;
public:
        PinControl(){}
        PinControl(Pin _pin, uint8_t _mode,bool _inverse);
        PinControl(Pin _pin, uint8_t _mode);
        void setup();
        void write(uint8_t val);
        int read();
        void setInverse(bool val);
        void on();
        void off();
};


class PinsControllerClass{
public:
        static void setup();
        static PinControl ledEA;
        static PinControl ledEB;
        static PinControl ledDA;
        static PinControl ledDB;


        static void loop();
        PinsControllerClass(){}
private:

};

extern PinsControllerClass PinsController;

#endif
