#ifndef temp_h
#define temp_h

#include <Arduino.h>

class sensorTemp : public analogPinIn{
	private:
		float convert(int v);
	public:	
		sensorTemp(int p) : analogPinIn(p){}
		float getValue();
		int getRaw();
		
};

#endif