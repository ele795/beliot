/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
* Modified by C. Pham, University of Pau, France
*/

#include "Sensor.h"
    
Sensor::Sensor(int id, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, int pin_trigger){
  
  set_id(id);
  set_is_analog(is_analog);
  set_is_connected(is_connected);
  set_is_low_power(is_low_power);
  set_pin_read(pin_read);
  set_pin_power(pin_power);
  set_pin_trigger(pin_trigger);
  set_data(0);
  set_wait_time(0);
  set_fake_data(false);
  
}

/////////////
// GETTERS //
/////////////

int Sensor::get_id() {
  return _id;
}

bool Sensor::get_is_analog(){
  return _is_analog;
}

bool Sensor::get_is_connected(){
  return _is_connected;
}

bool Sensor::get_is_low_power(){
  return _is_low_power;
}

uint8_t Sensor::get_pin_read(){
  return _pin_read;
}

uint8_t Sensor::get_pin_power(){
  return _pin_power;
}

int Sensor::get_pin_trigger(){
  return _pin_trigger;
}

double Sensor::get_data(){
  return _data;
}

unsigned long Sensor::get_wait_time(){
  return _wait_time;
}

bool Sensor::has_fake_data(){
  return _with_fake_data;
}

bool Sensor::has_pin_trigger(){
  return (_pin_trigger==-1)?false:true;
}
    
/////////////
// SETTERS //
/////////////
void Sensor::set_id(int id){
  _id = id;
}

void Sensor::set_is_analog(bool b){
  _is_analog = b;
}

void Sensor::set_is_connected(bool b){
  _is_connected = b;
}

void Sensor::set_is_low_power(bool b){
  _is_low_power = b;
}

void Sensor::set_pin_read(uint8_t u){
  _pin_read = u;
}

void Sensor::set_pin_power(uint8_t u){
  _pin_power = u;
}

void Sensor::set_pin_trigger(int u){
  _pin_trigger = u;
}

void Sensor::set_data(double d) {

	if (_is_connected || _with_fake_data)		
  		_data = d;
  	else
  		_data = -1.0;	
}

void Sensor::set_wait_time(unsigned long i){
  _wait_time = i;
}

void Sensor::set_fake_data(bool b) {
	_with_fake_data=b;
}		

void Sensor::update_data(){
  
}

double Sensor::get_value(){
  return 0.0;
}
