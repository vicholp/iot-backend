#include <Arduino.h>
#include "pins.h"
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
            return false;
        }else{

            setValue(getValue()+1);
            return false;
        }
    }
    short int analogDiode::getTarget(){

        return target;
    }

//analogRGB TESTED

    analogRGB::analogRGB(int t, int p[]){
        nDiodes = t;

        diodes = malloc(sizeof(*diodes) * (nDiodes));

        for (int i = 0; i < nDiodes; ++i){
            diodes[i] = new analogDiode(p, 0);

        }

        ms, msNext, msInterval = 500,0,500;
        state = true;
        changing = false;

    }

    void analogRGB::setTarget(int v[]){
        for (int i = 0; i < nDiodes; ++i){ //por cada diodo

            if(v[i] < 256 && v[i] > -1){ //valor valido

                diodes[i]->setTarget(calBright(v[i]));

            }else{
                //return 2;
            }

        }
        calDif();
    }

    void analogRGB::calDif(){

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

    void analogRGB::setBright(int b){
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

    void analogRGB::setMS(unsigned long m){

        ms = m;
    }

    void analogRGB::check(){
        if(changing){

            if (millis() > msNext){
                changing = false;
                for (int i = 0; i < nDiodes; ++i){
                    if(!(diodes[i]->refresh())) changing = true;
                }
                msNext = millis() + msInterval;
            }
        }
    }

    int analogRGB::calBright(int n){

        return (int)lrint(n*(float)valueBright/255.0);
    }

    void analogRGB::debug(){
        Serial.print("IMPRIMIENDO VALORES LED: ");
        for (int i = 0; i < nDiodes; ++i){
            Serial.print(diodes[i]->getValue());
            Serial.print(" - ");

        }
        Serial.println();
    }

//digitalRGB

    digitalRGB::digitalRGB(int t, int p[], int b = 0){

        type = t;

        diodes = malloc(sizeof(*diodes) * (type));

        for (int i = 0; i < type; ++i){
            diodes[i] = new digitalPin(p[i], false);
        }

        if (b != 0) brightPin = new analogPin(b, 100);

    }

    void digitalRGB::setTarget(bool v[], int b=100){
        for (int i = 0; i < type; ++i){
            if (v[i]){
                diodes[i]->on();
            }else{
                diodes[i]->off();
            }
        }
    }

//Pnp

    admLed::admLed(){
        nLedsRGB = 0;
        ledsRGB = malloc(sizeof(*ledsRGB) * (nLedsRGB));
    }

    int admLed::newLed(int t, int p[]){

        nLedsRGB++;
        ledsRGB = realloc(ledsRGB, sizeof(*ledsRGB) * (nLedsRGB));

        ledsRGB[nLedsRGB-1] = new analogRGB(t, p);

        return nLedsRGB;
    }

    void admLed::getValue(int id){
        ledsRGB[id]->debug();
    }
    void admLed::setTarget(int id, int v[]){
        ledsRGB[id]->setTarget(v);
    }

    int admLed::setBright(int id, int b){
        ledsRGB[id]->setBright(b);

        return 1;
    }

    void admLed::setMs(int id, int ms){

    }
    void admLed::checkLeds(){
        for (int i = 0; i < nLedsRGB; ++i){
            ledsRGB[i]->check();
        }
    }

    int  admLed::input(String seccion[]){

            if(seccion[2] == "led"){

                int id = seccion[4].toInt();

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
                Serial.println("NOT DEFINED");

            }
        }



