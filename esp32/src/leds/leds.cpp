#include <Arduino.h>
#include "../pins/pins.h"
#include "leds.h"
#include "../ArduinoJson/ArduinoJson.h"


//analogDiode

    void analogDiode::setTarget(int t){
        value_target = t;
    }

    void analogDiode::setPure(int t){
        value_pure = t;
    }

    bool analogDiode::refresh(){
        if (getValue() == value_target){
            return true;
        }
        if (getValue() > value_target){
            setValue(getValue()-1);
        }else{
            setValue(getValue()+1);
        }
        return false;
    }
    int analogDiode::getTarget(){
        return value_target;
    }

    int analogDiode::getPure(){
        return value_pure;
    }

//analogRGB TESTED

    analogStrip::analogStrip(int t, int p[]){
        nDiodes = t;

        diodes = (analogDiode**)(malloc(sizeof(*diodes) * (nDiodes)));

        for (int i = 0; i < nDiodes; ++i){
            diodes[i] = new analogDiode(p[i], 0);
        }
    }

    int analogStrip::setPure(int v[]){
        for (int i = 0; i < nDiodes; ++i){
            if(analogPin::checkValue(v[i])){
                diodes[i]->setPure(v[i]);

                diodes[i]->setTarget(calBright(v[i]));

            }else{
                return 1;
            }
        }
        calDif();
        return 0;
    }

    int analogStrip::setBright(int b){
        if (analogPin::checkValue(b)){
            valueBright = b;

            for (int i = 0; i < nDiodes; ++i){ 
                Serial.println("-");
                int a = diodes[i]->getPure();
                int b = calBright(a);
                diodes[i]->setTarget(b);
                Serial.println(a);
                Serial.println(b);
            }
            calDif();

        }else{
            return 1;
        }
        return 0;
    }

    int analogStrip::setMS(unsigned long m){
        ms = m;
        return 0;
    }


    DynamicJsonDocument analogStrip::getValues(){
        String answer;
        DynamicJsonDocument json(1024);

        JsonArray array = json.to<JsonArray>();

        for (int i = 0; i < nDiodes; ++i){
            array.add(diodes[i]->getPure());
            Serial.println(diodes[i]->getPure());
        }

        return json;
    }

    int analogStrip::getBright(){
        return valueBright;
    }
    unsigned long analogStrip::getMS(){
        return ms;
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

    void analogStrip::printState(){
        for (int i = 0; i < nDiodes; ++i){
            Serial.print("  ");
            Serial.print(diodes[i]->getPin());
            Serial.print(" -> ");
            Serial.print(diodes[i]->getValue());
            Serial.println("");
        }
    }

    int analogStrip::calBright(int n){

        return (int)lrint(n*(float)valueBright/(float)(analogPin::max_pinValue));
    }

    void analogStrip::calDif(){

        unsigned long minimum = ms;
        bool dif = false;

        for (int i = 0; i < nDiodes; ++i){ //por cada diodo
            int d = diodes[i]->getTarget()-diodes[i]->getValue();
            if (d != 0){
                minimum = min(minimum, (unsigned long)(abs(lrint( ms/abs(d) ))));
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

//Pnp

    admLed::admLed(){
        nStrips = 0;
        strips = (analogStrip**)(malloc(sizeof(*strips) * (nStrips)));
    }

    DynamicJsonDocument admLed::newLed(int t, int p[]){
        nStrips++;
        strips = (analogStrip**)(realloc(strips, sizeof(*strips) * (nStrips)));
        strips[nStrips-1] = new analogStrip(t, p);

        String answer;    
        DynamicJsonDocument json(1024);

        json["id"]   = nStrips-1;

        return json;
    }

    
    DynamicJsonDocument admLed::setTarget(int id, int v[]){
        DynamicJsonDocument json(256);
        json["result"] = strips[id]->setPure(v);
        return json;
    }

    void admLed::dg(){
        Serial.println("ADM LED DEBUG: ");
        for (int i = 0; i < nStrips; ++i){
            Serial.print("STRIP:  ");
            Serial.println(i+1);
            strips[i]->printState();
        }
        
        
    }

    DynamicJsonDocument admLed::setBright(int id, int b){
        DynamicJsonDocument json(1024);
        json["result"] = strips[id]->setBright(b);
        return json;
    }

    DynamicJsonDocument admLed::setMS(int id, int ms){
        DynamicJsonDocument json(1024);
        json["result"] = strips[id]->setMS((unsigned long)ms);
        return json;
    }

    void admLed::checkLeds(){
        for (int i = 0; i < nStrips; ++i){
            strips[i]->check();
        }
    }

    DynamicJsonDocument admLed::getValues(int id){
        return strips[id]->getValues();
    }

    DynamicJsonDocument admLed::getBright(int id){
        DynamicJsonDocument json(1024);
        json["bright"] = strips[id]->getBright();
        return json;
    }

    DynamicJsonDocument admLed::getMS(int id){
        DynamicJsonDocument json(1024);
        json["ms"] = strips[id]->getMS();
        return json;
    }

    DynamicJsonDocument admLed::input(String seccion[]){
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
            return setTarget(id,v);
            
        }

        if(seccion[3] == "setBright"){
            int b = seccion[5].toInt();
            return setBright(id, b);
        }  

        if(seccion[3] == "setMS"){
            int b = seccion[5].toInt();
            return setMS(id, b);
        }  


        if(seccion[3] == "getValue"){
            return getValues(id);  
        }   

        if(seccion[3] == "getBright"){
            return getBright(id);
        }

        if(seccion[3] == "getMS"){
            return getMS(id);  
        } 

    }
        



