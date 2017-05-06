/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*/

#ifndef SENSOR_H
#define SENSOR_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#define IS_ANALOG true
#define IS_NOT_ANALOG false
#define IS_CONNECTED true
#define IS_NOT_CONNECTED false
#define IS_LOWPOWER true
#define IS_NOT_LOWPOWER false

#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_MINI || defined __MK20DX256__  || defined __MKL26Z64__ || defined __SAMD21G18A__
  #define _BOARD_MVOLT_SCALE  3300.0
  #define _BOARD_VOLT_SCALE  3.3  
#else // ARDUINO_AVR_NANO || defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MEGA2560
  // also for all other boards, so change here if required.
  #define _BOARD_MVOLT_SCALE  5000.0 
  #define _BOARD_VOLT_SCALE  5.0   
#endif

class Sensor {
  public:  
    Sensor(int id, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, int pin_trigger=-1);
    
    // getters
    //////////
    int get_id();
    bool get_is_analog();
    bool get_is_connected();
    bool get_is_low_power();    
    uint8_t get_pin_read();
    uint8_t get_pin_power();
    int get_pin_trigger();
    //char* get_power_set();
    double get_data();
    unsigned long get_wait_time();
    bool has_fake_data();
    bool has_pin_trigger();
        
    //setters
    /////////
    void set_id(int id);
    void set_is_analog(bool b);
    void set_is_connected(bool b);
    void set_is_low_power(bool b);    
    void set_pin_read(uint8_t u);
    void set_pin_power(uint8_t u);
    void set_pin_trigger(int u);    
    //void set_power_set(char* c);
    void set_data(double d);
    void set_wait_time(unsigned long i);
    void set_fake_data(bool b);
    
    virtual void update_data();
    virtual double get_value();
    
  private:
    int _id;
    bool _is_analog;
    bool _is_connected;
    bool _is_low_power;    
    uint8_t _pin_read;
    uint8_t _pin_power;
    int _pin_trigger;
    bool _with_fake_data;
    //char* _power_set = NULL;
    double _data;
    unsigned long _wait_time;
};

#endif
