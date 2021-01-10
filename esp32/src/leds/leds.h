    #ifndef leds_h
#define leds_h

#include <Arduino.h>
#include "../pins/pins.h"
#include "../ArduinoJson/ArduinoJson.h"


class analogDiode : public analogPin { //Defined
    private:
        int value_target = 0;
        int value_pure = 0;

        
    public:
        int getTarget();
        analogDiode(int p, int v = 0) : analogPin(p, v){}

        int getPure();
        void setPure(int t);
        void setTarget(int t);
        bool refresh();
};

class analogStrip{ //Definido
    private:
        int nDiodes = 0; //cantidad de diodos;
        analogDiode **diodes;

        bool state = true;
        unsigned long ms = 1500;
        unsigned long msNext = 0;
        unsigned long msInterval = 0;

        bool changing = false;

        int valueBright = 1023;

        int max_brightValue =1023;
        int min_brightValue = 0;

        int calBright(int n); //pondera el voltaje segun brillo y rgb
        void calDif(); //calcula los ms de cambio

    public:
        analogStrip(int t, int p[]);

        int setPure(int v[]);
        int setBright(int b);
        int setMS(unsigned long n);

        DynamicJsonDocument getValues();
        unsigned long getMS();
        int getBright();

        void check();
        void printState();


};

class admLed{
    private:
        analogStrip **strips;
        int nStrips;
    public:
        admLed();
        DynamicJsonDocument newLed(int t, int p[]);
        DynamicJsonDocument getValues(int id);
        DynamicJsonDocument getMS(int id);
        DynamicJsonDocument getBright(int id);
        void dg();

        DynamicJsonDocument setTarget(int id, int v[]);
        DynamicJsonDocument setBright(int id, int b);
        DynamicJsonDocument setMS(int id, int ms);

        DynamicJsonDocument input(String seccion[]);
        void checkLeds();
};

#endif
