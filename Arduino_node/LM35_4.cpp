
/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
* modified by Congduc Pham, University of Pau, France
*/

#include "LM35_4.h"

LM35_4::LM35_4(int id, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power):Sensor(id, is_analog, is_connected, is_low_power, pin_read, pin_power){
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

float photocellReading; // the analog reading from the analog resistor divider

// Compteur d'impulsions
int imp = 500 ;

//Consommation electrique
float conso = 0;

// Flag using to compute the number of "pics" 
boolean flag_peak = false;
boolean flag_tresh = false;

// Determine a threshold to identify an impulsion
int threshold;

// Function used to find the correct threshold
// Find the min and max of a table among sizetable values and compute the mean
int LM35_4::find_treshold()
  {
    
  int threshold = 0;
  long time = millis();
  int photocellvalue = 0;
  int min_value = analogRead(get_pin_read());
  int max_value = min_value;
    

      while( (millis() - time) < 65000 )
      {
     // Searching a max and a min value among the values 
        photocellvalue = analogRead(get_pin_read()); 
          if(photocellvalue < min_value)
          {
            min_value = photocellvalue;
          }
          else if(photocellvalue > max_value)
          {
            max_value = photocellvalue;
          }
        
      }  
   
   
  threshold = (max_value+min_value)/2;
  
  
  return threshold;
    
  }

void LM35_4::update_data()
{
  if (get_is_connected()) {

    float test_tresh = analogRead(get_pin_read());
    float aux_lum = 0;
    float luminosity = 0;
    
    // if we use a digital pin to power the sensor...
    if (get_is_low_power())
    	digitalWrite(get_pin_power(),HIGH);     
    
    // wait
    delay(get_wait_time());
    
    // you can decice to have less samples
    //uint8_t nb_sample=500;
    //float R = 10; // resistance en KOhm
   
    //for(int i=0; i<nb_sample; i++) {
        
  if ( test_tresh > 150 && flag_tresh == false) 
  {
    threshold = LM35_4::find_treshold();
    flag_tresh = true;
  }
    //}

    // Read the value of the photocell 
  photocellReading = analogRead(get_pin_read());

  if (flag_tresh == true)
  {
    
  if (photocellReading > threshold) 
  {
      
    if(flag_peak == true)
    {
      
    }
    else
    {
      flag_peak = true;
      imp = imp+1;
      
    }
       
  }
  else   
  {
    flag_peak = false;
  }

  if(imp >= 500)
  {
    conso = conso + 1;
    imp = 0;
  }
  
  }
    
    if (get_is_low_power())    
        digitalWrite(get_pin_power(),LOW);
  
    	// getting the average
		set_data(conso);
	}
	else {
  		// if not connected, set a random value (for testing)  	
  		if (has_fake_data())
    		set_data((double)random(-10, 30));
	}
}

double LM35_4::get_value()
{
  update_data();
  return get_data();
}
