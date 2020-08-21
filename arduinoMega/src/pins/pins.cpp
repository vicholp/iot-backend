#include <Arduino.h>
#include "pins.h"

//analogPin

    analogPin::analogPin(int p, int v = 0){
        pinMode(p, OUTPUT);
        pin = p;
        setValue(v);
    }
    void analogPin::setValue(int v){
        value = v;
        analogWrite(pin, v);
    }
    short int analogPin::getValue(){

        return value;
    }

    short int analogPin::getPin(){
        return pin;
    }

//digitalPin

    digitalPin::digitalPin(int p, bool s = false){
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
