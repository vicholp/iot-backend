#ifndef leds_h
#define leds_h

#include <Arduino.h>
#include "../pins/pins.h"


class analogDiode : public analogPin { //Defined
    private:
        short int target = 0;
    public:
        short int getTarget();
        analogDiode(int p, int v = 0) : analogPin(p, v){}

        void setTarget(int t);
        bool refresh();
};

class analogStrip{ //Definido
    private:
        int nDiodes = 0; //cantidad de diodos;
        analogDiode **diodes;

        bool state = true;
        unsigned long ms = 2000;
        unsigned long msNext;
        unsigned long msInterval;

        bool changing = false;

        short int valueBright = 255;

        int calBright(int n); //pondera el voltaje segun brillo y rgb
        void calDif(); //calcula los ms de cambio

    public:
        analogStrip(int t, int p[]);

        void setTarget(int v[]);
        void setBright(int b);
        void setMS(unsigned long n);

        void check();
        void printState();

};

class admLed{ //Plug 'n' Play
    private:
        analogStrip **strips;
        int nStrips;
    public:
        admLed();
        int newLed(int t, int p[]);
        void getValue(int id);
        void dg();

        void setTarget(int id, int v[]);
        int setBright(int id, int b);
        void setMs(int id, int ms);

        int input(String seccion[]);
        void checkLeds();
};

#endif
