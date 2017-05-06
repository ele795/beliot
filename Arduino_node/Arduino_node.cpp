/*
 * Multiple sensor (temp/humidity/light) node for arduino
 * last update: Jan 23 2017 by B. Laporte-Fauret
 */
#include <SPI.h> 
#include "SX1272.h"
#include "Sensor.h"
#include "LM35.h"
#include "LM35_2.h"
#include "LM35_3.h"
#include "LM35_4.h"

#define ETSI_EUROPE_REGULATION

#define BAND868
//#define BAND900
//#define BAND433

#define MAX_DBM 14

// If you are using InAir9B, uncomment the following line. You can also try setting MAX_DBM to 20, and check is Setting Power state is still 0
#define PABOOST

//ifdef BAND868
const uint32_t DEFAULT_CHANNEL=CH_10_868;
//#elif defined BAND900
//const uint32_t DEFAULT_CHANNEL=CH_05_900;
//#elif defined BAND433
//const uint32_t DEFAULT_CHANNEL=CH_00_433;
//#endif

#if not defined _VARIANT_ARDUINO_DUE_X_ && not defined __SAMD21G18A__
#define WITH_EEPROM
#endif
#define WITH_APPKEY
#define FLOAT_TEMP
#define LOW_POWER
#define LOW_POWER_HIBERNATE
//#define WITH_ACK

#define LORAMODE  1
#define node_addr 13

unsigned int idlePeriodInMin = 2; // in minute
unsigned int id_node = 7;
unsigned short id_frame = 0;

#ifdef WITH_APPKEY
uint8_t my_appKey[4]={5, 6, 7, 8};
#endif

#if defined __SAMD21G18A__
#define PRINTLN                   SerialUSB.println("")              
#define PRINT_CSTSTR(fmt,param)   SerialUSB.print(F(param))
#define PRINT_STR(fmt,param)      SerialUSB.print(param)
#define PRINT_VALUE(fmt,param)    SerialUSB.print(param)
#define FLUSHOUTPUT               SerialUSB.flush();
#else
#define PRINTLN                   Serial.println("")              
#define PRINT_CSTSTR(fmt,param)   Serial.print(F(param))
#define PRINT_STR(fmt,param)      Serial.print(param)
#define PRINT_VALUE(fmt,param)    Serial.print(param)
#define FLUSHOUTPUT               Serial.flush();
#endif

#ifdef WITH_EEPROM
#include <EEPROM.h>
#endif

#define DEFAULT_DEST_ADDR 1

#ifdef WITH_ACK
#define NB_RETRIES 2
#endif

#ifdef LOW_POWER
#if defined __MK20DX256__ || defined __MKL26Z64__
#define LOW_POWER_PERIOD 60
#include <Snooze.h>
SnoozeBlock sleep_config;
#else
#define LOW_POWER_PERIOD 8
#include "LowPower.h"

#ifdef __SAMD21G18A__
#include "RTCZero.h"
RTCZero rtc;
#endif
#endif
unsigned int nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD;
#endif

unsigned long nextTransmissionTime=0L;
uint8_t message[100];
int loraMode=LORAMODE;

#ifdef WITH_EEPROM
struct sx1272config {
  uint8_t flag1;
  uint8_t flag2;
  uint8_t seq;
};

sx1272config my_sx1272config;
#endif

#if not defined _VARIANT_ARDUINO_DUE_X_ && defined FLOAT_TEMP

char *ftoa(char *a, double f, int precision)
{
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 itoa(desimal, a, 10);
 return ret;
}
#endif

// Sensors definition
const int number_of_sensors = 3;
Sensor* sensor_ptrs[number_of_sensors];

void setup()
{  
  int e;

#ifdef LOW_POWER
  bool low_power_status = IS_LOWPOWER;
#ifdef __SAMD21G18A__
  rtc.begin();
#endif  
#else
  bool low_power_status = IS_NOT_LOWPOWER;
#endif

// Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger=-1)
  sensor_ptrs[0] = new LM35(21, IS_ANALOG, IS_CONNECTED, IS_LOWPOWER, (uint8_t) A4, (uint8_t) 3);
  sensor_ptrs[1] = new LM35_2(999, IS_ANALOG, IS_CONNECTED, IS_LOWPOWER, (uint8_t) A1, (uint8_t) 9);
  sensor_ptrs[2] = new LM35_3(22, IS_ANALOG, IS_CONNECTED, IS_LOWPOWER, (uint8_t) A0, (uint8_t) 3);
  delay(3000);

#ifdef __SAMD21G18A__  
  SerialUSB.begin(38400);
#else
  Serial.begin(38400);  
#endif 
  PRINT_CSTSTR("%s","Generic LoRa sensor\n");

#ifdef ARDUINO_AVR_PRO
  PRINT_CSTSTR("%s","Arduino Pro Mini detected\n");
#endif

#ifdef ARDUINO_AVR_NANO
  PRINT_CSTSTR("%s","Arduino Nano detected\n");
#endif

#ifdef ARDUINO_AVR_MINI
  PRINT_CSTSTR("%s","Arduino MINI/Nexus detected\n");
#endif

#ifdef __MK20DX256__
  PRINT_CSTSTR("%s","Teensy31/32 detected\n");
#endif

#ifdef __MKL26Z64__
  PRINT_CSTSTR("%s","TeensyLC detected\n");
#endif

#ifdef __SAMD21G18A__ 
  PRINT_CSTSTR("%s","Arduino M0/Zero detected\n");
#endif

  sx1272.ON();

#ifdef WITH_EEPROM
  // get config from EEPROM
  EEPROM.get(0, my_sx1272config);

  if (my_sx1272config.flag1==0x12 && my_sx1272config.flag2==0x34) {
    PRINT_CSTSTR("%s","Get back previous sx1272 config\n");
    sx1272._packetNumber=my_sx1272config.seq;
    PRINT_CSTSTR("%s","Using packet sequence number of ");
    PRINT_VALUE("%d", sx1272._packetNumber);
    PRINTLN;
  }
  else {
    my_sx1272config.flag1=0x12;
    my_sx1272config.flag2=0x34;
    my_sx1272config.seq=sx1272._packetNumber;
  }
#endif
  e = sx1272.setMode(loraMode);
  PRINT_CSTSTR("%s","Setting Mode: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  sx1272._enableCarrierSense=true;
#ifdef LOW_POWER
  sx1272._RSSIonSend=false;
#endif 
  e = sx1272.setChannel(DEFAULT_CHANNEL);
  PRINT_CSTSTR("%s","Setting Channel: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
#ifdef PABOOST
  sx1272._needPABOOST=true;
#else
#endif
  e = sx1272.setPowerDBM((uint8_t)MAX_DBM);
  PRINT_CSTSTR("%s","Setting Power: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  e = sx1272.setNodeAddress(node_addr);
  PRINT_CSTSTR("%s","Setting node addr: state ");
  PRINT_VALUE("%d", e);
  PRINTLN;
  PRINT_CSTSTR("%s","SX1272 successfully configured\n");
  delay(500);
}

void loop(void)
{
  long startSend;
  long endSend;
  uint8_t app_key_offset=0;
  int e;

#ifndef LOW_POWER
  // 600000+random(15,60)*1000
  if (millis() > nextTransmissionTime) {
#endif

#ifdef WITH_APPKEY
      app_key_offset = sizeof(my_appKey);
      memcpy(message,my_appKey,app_key_offset);
#endif
      uint8_t r_size;
      char final_str[80] = "\\";
      char aux[6] = "";
      char id[1] = "";
      sprintf(final_str, "%s!%i!%hd", final_str,id_node, id_frame++);
      for (int i=0; i<number_of_sensors; i++) {
          if (sensor_ptrs[i]->get_is_connected() || sensor_ptrs[i]->has_fake_data()) {
              ftoa(aux, sensor_ptrs[i]->get_value(), 2);
              sprintf(final_str, "%s#%d/%s", final_str, sensor_ptrs[i]->get_id(), aux);
          }
          else
            strcpy(aux,"");
      }
      r_size=sprintf((char*)message+app_key_offset, final_str);

      PRINT_CSTSTR("%s","Sending ");
      PRINT_STR("%s",(char*)(message+app_key_offset));
      PRINTLN;
      
      PRINT_CSTSTR("%s","Real payload size is ");
      PRINT_VALUE("%d", r_size);
      PRINTLN;
      
      int pl=r_size+app_key_offset;   
      sx1272.CarrierSense(); 
      startSend=millis();
      sx1272.setPacketType(PKT_TYPE_DATA | PKT_FLAG_DATA_WAPPKEY);
      
#ifdef WITH_ACK
      int n_retry=NB_RETRIES;
      do {
        e = sx1272.sendPacketTimeoutACK(DEFAULT_DEST_ADDR, message, pl);
        if (e==3)
          PRINT_CSTSTR("%s","No ACK");       
        n_retry--;       
        if (n_retry)
          PRINT_CSTSTR("%s","Retry");
        else
          PRINT_CSTSTR("%s","Abort");  
          
      } while (e && n_retry);          
#else      
      e = sx1272.sendPacketTimeout(DEFAULT_DEST_ADDR, message, pl);
#endif
  
      endSend=millis();
    
#ifdef WITH_EEPROM
      my_sx1272config.seq=sx1272._packetNumber;
      EEPROM.put(0, my_sx1272config);
#endif
      
      PRINT_CSTSTR("%s","LoRa pkt seq ");
      PRINT_VALUE("%d", sx1272.packet_sent.packnum);
      PRINTLN;
    
      PRINT_CSTSTR("%s","LoRa Sent in ");
      PRINT_VALUE("%ld", endSend-startSend);
      PRINTLN;
          
      PRINT_CSTSTR("%s","LoRa Sent w/CAD in ");
      PRINT_VALUE("%ld", endSend-sx1272._startDoCad);
      PRINTLN;

      PRINT_CSTSTR("%s","Packet sent, state ");
      PRINT_VALUE("%d", e);
      PRINTLN;

#ifdef LOW_POWER
      PRINT_CSTSTR("%s","Switch to power saving mode\n");

      e = sx1272.setSleepMode();

      if (!e)
        PRINT_CSTSTR("%s","Successfully switch LoRa module in sleep mode\n");
      else  
        PRINT_CSTSTR("%s","Could not switch LoRa module in sleep mode\n");
        
      FLUSHOUTPUT
      delay(50);

#ifdef __SAMD21G18A__
      rtc.setTime(17, 0, 0);
      rtc.setDate(1, 1, 2000);
      rtc.setAlarmTime(17, idlePeriodInMin, 0);
      rtc.enableAlarm(rtc.MATCH_HHMMSS);
      rtc.standbyMode();
      PRINT_CSTSTR("%s","SAMD21G18A wakes up from standby\n");      
      FLUSHOUTPUT
#else      
      nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD + random(2,4);
#if defined __MK20DX256__ || defined __MKL26Z64__
      sleep_config.setTimer(LOW_POWER_PERIOD*1000 + random(1,5)*1000);// milliseconds

      nCycle = idlePeriodInMin*60/LOW_POWER_PERIOD;
#endif          
      for (int i=0; i<nCycle; i++) {  

#if defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_NANO || ARDUINO_AVR_UNO || ARDUINO_AVR_MINI  
          LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

#elif defined ARDUINO_AVR_MEGA2560
          LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
#elif defined __MK20DX256__ || defined __MKL26Z64__  
#ifdef LOW_POWER_HIBERNATE
          Snooze.hibernate(sleep_config);
#else            
          Snooze.deepSleep(sleep_config);
#endif  
#else
          delay(LOW_POWER_PERIOD*1000);
#endif                        
          PRINT_CSTSTR("%s",".");
          FLUSHOUTPUT; 
          delay(10);                        
      }     
      delay(50);
#endif 

#else
      PRINT_VALUE("%ld", nextTransmissionTime);
      PRINTLN;
      PRINT_CSTSTR("%s","Will send next value at\n");
      // use a random part also to avoid collision
      nextTransmissionTime=millis()+(unsigned long)idlePeriodInMin*60*1000+(unsigned long)random(15,60)*1000;
      PRINT_VALUE("%ld", nextTransmissionTime);
      PRINTLN;
  }
#endif
  delay(50);
}
