/**
 * @Author: Anderson Juncowski <bigujun>
 * @Date:   2018-02
 * @Email:  andersonj@hotmail.rs
 */
#include "PinsController.h"
#include "DebugUtils.h"

PinControl PinsControllerClass::ledEA = PinControl(LED_EA, OUTPUT);
PinControl PinsControllerClass::ledDA = PinControl(LED_DA, OUTPUT);
PinControl PinsControllerClass::ledEB = PinControl(LED_EB, OUTPUT);
PinControl PinsControllerClass::ledDB = PinControl(LED_DB, OUTPUT);
PinsControllerClass PinsController;



void PinsControllerClass::setup(){
        ledEA.setup();
        ledEB.setup();
        ledDA.setup();
        ledDB.setup();
}

void PinsControllerClass::loop(){

}

PinControl::PinControl(Pin _pin, uint8_t _mode,bool _inverse){
	pin=_pin;
        mode=_mode;
	inverse=_inverse;
}

PinControl::PinControl(Pin _pin, uint8_t _mode){
	pin=_pin;
	mode=_mode;
}

void PinControl::setup(){
        if(mode==OUTPUT)off();
	pinMode(pin,mode);
        // DEBUG_PRINTF("SETUP %d , %s \n",pin,mode==OUTPUT?"OUTPUT":"INPUT");
}

void PinControl::write(uint8_t val){
	digitalWrite(pin,val);
        // Serial.printf("WRITE %d -> %d \n",pin,val);
}

int PinControl::read(){
        uint8_t val = digitalRead(pin);
        return inverse?!val:val;
}

void PinControl::setInverse(bool val){
	inverse = val;
}

void PinControl::on(){
	bool val = inverse?LOW:HIGH;
	write(val);
}

void PinControl::off(){
	bool val = inverse?HIGH:LOW;
	write(val);
}
