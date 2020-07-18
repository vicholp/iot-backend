#ifndef leds_h
#define leds_h

#include <Arduino.h>
#include "pins.h"

class rele : public digitalPin { //Defined
        private:
            bool type;
        public:
            rele(int p, bool s, bool t);
};

#endif
