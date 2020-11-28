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
                minimum = min(minimum, abs(lrint( ms/abs(d) )));
                dif = true;
            }

        }
        if (dif){
            msInterval = minimum;
            if(msInterval == 0){
                for (int i = 0; i < nDiodes; ++i){ //por cada diodo
                    diodes[i]->setValue(diodes[i]->getTarget());
                }
                return;
            }
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

    void analogStrip::printState(){
        for (int i = 0; i < nDiodes; ++i){
            Serial.print("  ");
            Serial.print(diodes[i]->getPin());
            Serial.print(" -> ");
            Serial.print(diodes[i]->getValue());
            Serial.println("");
        }
    }

    void analogStrip::getValues(){
        for (int i = 0; i < nDiodes; ++i){
            Serial.print(diodes[i]->getValue());
            if(i != nDiodes-1) Serial.print(',');
        }
    }

    void analogStrip::getBright(){
        Serial.print(valueBright);
    }
    void analogStrip::getMS(){
        Serial.print(ms);
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

        return nStrips-1;
    }

    void admLed::getValues(int id){
        strips[id]->getValues();
    }
    void admLed::getBright(int id){
        strips[id]->getBright();
    }
    void admLed::getMS(int id){
        strips[id]->getMS();
    }
    void admLed::setTarget(int id, int v[]){
        strips[id]->setTarget(v);
    }

    void admLed::dg(){
        Serial.println("ADM LED DEBUG: ");
        for (int i = 0; i < nStrips; ++i){
            Serial.print("STRIP:  ");
            Serial.println(i+1);
            strips[i]->printState();
        }
        
        
    }

    void admLed::setBright(int id, int b){
        strips[id]->setBright(b);
    }

    void admLed::setMS(int id, int ms){
        strips[id]->setMS((unsigned long)ms);
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
            Serial.print("[");
            if(seccion[3] == "debug"){
                dg();

                
            }
            if(seccion[3] == "new"){
                int leds = id;
                int v[leds];
                for (int i = 0; i < leds; ++i){
                    v[i] = seccion[5+i].toInt();
                }
                Serial.print(newLed(leds, v));
                
            }

            if(seccion[3] == "setTarget"){
                int leds = seccion[5].toInt();
                int v[leds];
                for (int i = 0; i < leds; ++i){
                    v[i] = seccion[6+i].toInt();
                }
                setTarget(id,v);
                Serial.print("'OK'");
                
            }

            if(seccion[3] == "setBright"){
                int b = seccion[5].toInt();
                setBright(id, b);
                Serial.print("'OK'");
                
            }  
            if(seccion[3] == "setMS"){
                int b = seccion[5].toInt();
                setMS(id, b);
                Serial.print("'OK'");
                
            }  

            if(seccion[3] == "getValue"){
                getValues(id);
                
            }   
            if(seccion[3] == "getBright"){
                getBright(id);
                
            } 
            if(seccion[3] == "getMS"){
                getMS(id);
                
            }         
            Serial.println("]");
            return 1;
        }
        



