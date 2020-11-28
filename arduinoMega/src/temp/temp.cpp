#include <Arduino.h>
#include "../pins/pins.h"
#include "temp.h"

float sensorTemp::convert(int v){
	return ((v / 1023.0) * 500);
}

float sensorTemp::getValue(){
	return convert(analogPinIn::getValue());
}

int sensorTemp::getRaw(){
	return analogPinIn::getValue();
}