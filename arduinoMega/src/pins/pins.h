#ifndef pins_h
#define pins_h

class digitalPin {
    protected:
        short int pin;
        bool state;
    public:
        digitalPin(int p, bool s = false); //pin, defaultState
        void toggle();
        void on();
        void off();
        bool getState();
        short int getPin();
};

class analogPin {
    protected:
        short int pin = 0;
        short int value = 0;

    public:
        analogPin(int p, int v = 0);
        short int getValue();
        void setValue(int v);
        short int getPin();
};


#endif
