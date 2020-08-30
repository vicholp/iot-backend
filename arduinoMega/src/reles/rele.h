#ifndef rele_h
#define rele_h

#include <Arduino.h>
#include "../pins/pins.h"

class rele : public digitalPin { //Defined
        private:
            bool type;
        public:
            rele(int p, bool s, bool t);
};

#endif
