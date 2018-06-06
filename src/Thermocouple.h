/**
 * @Author: Anderson Juncowski <anderson>
 * @Date:   2018-06-04T22:58:34-03:00
 * @Email:  andersonj@hotmail.rs
 */

#ifndef Thermocouple_h
#define Thermocouple_h
#include <Arduino.h>
#include "tables.h"

class Thermocouple{
private:
        uint8_t pin;
        uint16_t leituiras = 1000;
        ThermocoupleType type;


        enum MIN_MAX{
                MIN,
                MAX
        };

        uint8_t coefSize = sizeof(coefs)/sizeof(mvToC_coeficients);
        float miliVoltToCelsius(float mv);
        mvToC_coeficients * selectTable(float mv);
        float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
public:
        struct Calibrate{
                long in_min;
                long in_max;
                long out_min;
                long out_max;
        }calibrate{0,1023,0,400};

        Thermocouple(uint8_t _pin, ThermocoupleType _type = TypeK):pin(_pin),type(_type){}


        void begin();
        float read();
        float readmv();
        void setReadings(uint16_t _leituras);


};
#endif
