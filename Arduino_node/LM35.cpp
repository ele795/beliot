/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
* modified by Congduc Pham, University of Pau, France
*/

#include "LM35.h"

LM35::LM35(int id, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power):Sensor(id, is_analog, is_connected, is_low_power, pin_read, pin_power){
  if (get_is_connected()){

    pinMode(get_pin_read(), INPUT);
    pinMode(get_pin_power(),OUTPUT);
    
	if(get_is_low_power())
       digitalWrite(get_pin_power(),LOW);
    else
		digitalWrite(get_pin_power(),HIGH);
	
    set_wait_time(500);
  }
}

void LM35::update_data()
{
  if (get_is_connected()) {
  	
    float aux_temp = 0.;
    float temperature = 0.;
    
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);     
    
    // wait
    delay(get_wait_time());
    
    // you can decice to have less samples
    uint8_t nb_sample=500;
    float a = 0.019; //coefs calcul temp
    float b = -16.95; //offset calcul temp
   
    for(int i=0; i<nb_sample; i++) {

	      int sensorValue = analogRead(get_pin_read());
        float voltage =  (sensorValue/1024.0) * 5.;
        aux_temp = voltage/a + b;  // calcul avec étalonnage thermomètre      
        //aux_temp = (voltage) * 100;
        temperature += aux_temp;
        delay(10);
    }
    
    if (get_is_low_power())    
        digitalWrite(get_pin_power(),LOW);
  
    	// getting the average
		set_data((temperature / (double)nb_sample));
	}
	else {
  		// if not connected, set a random value (for testing)  	
  		if (has_fake_data())
    		set_data((double)random(-10, 30));
	}
}

double LM35::get_value()
{
  update_data();
  return get_data();
}
