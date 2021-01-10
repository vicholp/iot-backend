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
        int value = 0;
        short int channel = 0;

        int pwm_frec = 5000;
        int pwm_res = 10;


    public:
        static const int max_pinValue = 1023;
        static const int min_pinValue = 0;

        analogPin(int p, int v = 0);
        int getValue();
        void setValue(int v);
        short int getPin();
        short int getPWMChannel(int p);

        static bool checkValue(int v);
};

class analogPinIn {
    protected:
        int pin = 0;

    public:
        analogPinIn(int p);
        int getValue();
        int getPin();
};


#endif
