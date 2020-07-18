#include <Arduino.h>
#include "pins.h"
#include "rele.h"

rele::rele(int p, bool s, bool t) : digitalPin(p, s){
    type = t;
}
