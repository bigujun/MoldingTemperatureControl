/**
 * @Author: Anderson Juncowski <anderson>
 * @Date:   2018-06-04T22:58:11-03:00
 * @Email:  andersonj@hotmail.rs
 */

#include "Thermocouple.h"
#include "DebugUtils.h"

float Thermocouple::read(){
  return miliVoltToCelsius(readmv());
}



float Thermocouple::readmv(){
        unsigned long value = 0;
        for(uint16_t i=0;i<leituiras;i++){
                value += analogRead(pin);
        }
        float ret = value/leituiras;
        DEBUG_PRINTLN( String("RAW") + String(ret));
        return mapFloat((float)value/leituiras,calibrate.in_min,calibrate.in_max,calibrate.out_min,calibrate.out_max);
}

float Thermocouple::miliVoltToCelsius(float mv){
        mvToC_coeficients *c = selectTable(mv);
        if(c==nullptr)return 0.0;
        DEBUG_PRINTLN(String(mv) + String("mV"));

        float v_v0 = mv-c->mv0;
        float P = v_v0*(c->p[0] + v_v0 * (c->p[1] + v_v0 * (c->p[2] + c->p[3] * v_v0)));
        float Q = 1 + v_v0*(c->q[0] + v_v0*(c->q[1]+c->q[2]*v_v0));
        return c->t0 + P/Q;
}

mvToC_coeficients * Thermocouple::selectTable(float mv){
        bool found=false;
        int i;
        for(i=0;i<coefSize;i++){
                if(mv > mvToC_coefs[i].mv[MIN] && mv < mvToC_coefs[i].mv[MAX]){
                        found = true;
                        break;
                }
        }
        DEBUG_PRINT("Scale:");
        DEBUG_PRINTLN(found?String(i):String("OUT"));
        return found? &mvToC_coefs[i] : nullptr;
}

void Thermocouple::setReadings(uint16_t _leituras){
        leituiras = _leituras;
}

void Thermocouple::begin(){
        pinMode(pin,INPUT);
}

float Thermocouple::mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
