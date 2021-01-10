#include <Arduino.h>
#include "pins.h"

//analogPin

    analogPin::analogPin(int p, int v){
        pinMode(p, OUTPUT);

        channel = getPWMChannel(p);
        ledcSetup(channel, pwm_frec, pwm_res);
		ledcAttachPin(p, channel);

        pin = p;
        setValue(v);
    }
    void analogPin::setValue(int v){
        value = v;
  	
        ledcWrite(channel,v);
    }
    int analogPin::getValue(){
        return value;
    }

    short int analogPin::getPin(){
        return pin;
    }
    short int analogPin::getPWMChannel(int p){
    	switch(p){
    		case 4:
    			return 0;
    		case 16:
    			return 1;
    		case 17:
    			return 2;
    		case 5:
    			return 3;
    		case 18:
    			return 4;
    		case 19:
    			return 5;
            case 2:
                return 6;
    	}
    	return 6;
    }

    bool analogPin::checkValue(int v){
        if (v >= min_pinValue && v <= max_pinValue){
            return true;
        }
        return false;
    }

//analogPinIn

    analogPinIn::analogPinIn(int p){
        pin = p;
    }
    int analogPinIn::getValue(){
        return analogRead(pin);
    }
    int analogPinIn::getPin(){
        return pin;
    }

//digitalPin

    digitalPin::digitalPin(int p, bool s){
        pinMode(p, OUTPUT);
        pin = p;

        if (s){
            on();
        }else{
            off();
        }
    }
    void digitalPin::on(){
        state = true;
        digitalWrite(pin, HIGH);
    }
    void digitalPin::off(){
        state = false;
        digitalWrite(pin, LOW);
    }
    void digitalPin::toggle(){
        if (state){
            off();
        }else{
            on();
        }
    }
    bool digitalPin::getState(){

        return state;
    }

    short int digitalPin::getPin(){
        return pin;
    }
