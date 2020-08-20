#include <Arduino.h>
#include "../pins/pins.h"
#include "leds.h"


//analogDiode

    void analogDiode::setTarget(int t){

        target = t;
    }

    bool analogDiode::refresh(){
        if (getValue() == target){
            return true;
        }
        if (getValue() > target){
            setValue(getValue()-1);
        }else{
            setValue(getValue()+1);
        }
        return false;
    }
    short int analogDiode::getTarget(){

        return target;
    }

//analogRGB TESTED

    analogStrip::analogStrip(int t, int p[]){
        nDiodes = t;

        diodes = malloc(sizeof(*diodes) * (nDiodes));

        for (int i = 0; i < nDiodes; ++i){
            diodes[i] = new analogDiode(p[i], 0);
        }

        ms, msNext, msInterval = 500,0,500;
        state = true;
        changing = false;

    }

    void analogStrip::setTarget(int v[]){
        for (int i = 0; i < nDiodes; ++i){ //por cada diodo
            if(v[i] < 256 && v[i] > -1){ //valor valido
                diodes[i]->setTarget(calBright(v[i]));
            }else{
                //return 2;
            }
        }
        calDif();
    }

    void analogStrip::calDif(){

        unsigned long minimum = ms;
        bool dif = false;

        for (int i = 0; i < nDiodes; ++i){ //por cada diodo
            int d = diodes[i]->getTarget()-diodes[i]->getValue();
            if (d != 0){
                minimum = min(minimum, abs(lrint( ms/(d) )));
                dif = true;
            }

        }
        if (dif){
            msInterval = minimum;

            msNext = millis() + msInterval;
            changing = true;
        }
    }

    void analogStrip::setBright(int b){
        if (b>=0 && b <=255){
            valueBright = b;

            for (int i = 0; i < nDiodes; ++i){ //por cada diodo
                diodes[i]->setTarget(calBright(diodes[i]->getValue()));
            }
            calDif();

        }else{
            //return 0;
        }
    }

    void analogStrip::setMS(unsigned long m){

        ms = m;
    }

    void analogStrip::check(){
        if (changing && millis() > msNext){
            changing = false;
            for (int i = 0; i < nDiodes; ++i){
                if(!(diodes[i]->refresh())) changing = true;
            }
            msNext = millis() + msInterval;
        }
    }

    int analogStrip::calBright(int n){

        return (int)lrint(n*(float)valueBright/255.0);
    }

    void analogStrip::debug(){
        for (int i = 0; i < nDiodes; ++i){
            Serial.print(diodes[i]->getValue());
            Serial.print(" - ");
        }
    }

//Pnp

    admLed::admLed(){
        nStrips = 0;
        strips = malloc(sizeof(*strips) * (nStrips));
    }

    int admLed::newLed(int t, int p[]){
        nStrips++;
        strips = realloc(strips, sizeof(*strips) * (nStrips));

        strips[nStrips-1] = new analogStrip(t, p);

        return nStrips;
    }

    void admLed::getValue(int id){
        
        strips[id]->debug();
    }
    void admLed::setTarget(int id, int v[]){
        strips[id]->setTarget(v);
    }

    void admLed::dg(){
        Serial.print("ADM LED DEBUG: ");
        for (int i = 0; i < nStrips; ++i){
            Serial.print(i+1);
            Serial.print(": ");
            strips[i]->debug();
            Serial.println();
        }
        
        
    }

    int admLed::setBright(int id, int b){
        strips[id]->setBright(b);

        return 1;
    }

    void admLed::setMs(int id, int ms){

    }
    void admLed::checkLeds(){
        for (int i = 0; i < nStrips; ++i){
            strips[i]->check();
        }
    }

    int  admLed::input(String seccion[]){
            if(seccion[2] != "led"){
                return false;
            }
            int id = seccion[4].toInt();
            if(seccion[3] == "debug"){
                dg();

                return 1;
            }
            if(seccion[3] == "new"){
                int leds = id;
                int v[leds];
                for (int i = 0; i < leds; ++i){
                    v[i] = seccion[5+i].toInt();
                }
                return newLed(leds, v);
            }

            if(seccion[3] == "setTarget"){
                int leds = seccion[5].toInt();
                int v[leds];
                for (int i = 0; i < leds; ++i){
                    v[i] = seccion[6+i].toInt();
                }
                setTarget(id,v);
                return 1;
            }

            if(seccion[3] == "setBright"){
                int b = seccion[5].toInt();
                setBright(id, b);

                return 1;
            }

            if(seccion[3] == "getValue"){
                getValue(id);

                return 1;
            }
            

            Serial.print(seccion[3]);
            Serial.println(" NOT DEFINED");
        }
        



