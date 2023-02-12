#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

// Real time Clock 
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// SD Card
File Data;
const int PinCs = 10; // Chip Select Pin , must be 10!

// Sensor
int sensor_1 = 2;  // digital pin of sensor 1 Hall
int sensor_2 = 4; // digital pin of sensor 2 Licht
int last_sensor_check = 0; // save last sensor which detects a change

// Evaluation
int Hall = 0;
int Licht = 0;
unsigned long T_last_h = 0;
unsigned long T_now_h = 0;
unsigned long T_now_l = 0;
unsigned long T_last_l = 0;
float Laufzeit = 0;
float Velocity = 0;
int Count= 0;
int Total = 0;
int day_old = 0;
int hour_old = 0;
int minute_old = 0;
float Velocity_mean = 0;

// String definition
char C = 'C';
char V = 'V';
char Dash = '/';
char Space = ' ';
char Point = ':';

void setup() {
     Serial.begin(9600);
     //LCD
     lcd.init();
     lcd.backlight();
     delay(250);
     lcd.noBacklight();
     delay(250);
     lcd.backlight();

     lcd.setCursor(0, 0);
     lcd.print("Setup:");
     
     
     // Real Time Clock
     if (! rtc.begin())
     {  
        lcd.setCursor(0, 1);
        lcd.print("RTC not found");
        Serial.println("Couldn't find RTC");
     }
     if (rtc.lostPower()) 
     {  
        lcd.setCursor(0, 1);
        lcd.print("Lost Power");
        
        Serial.println("RTC lost power, let's set the time!");
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
      }

      // SD CARD
      pinMode(PinCs,OUTPUT);
      //SD Card initialization;
      if (SD.begin(PinCs))
      {
          Serial.println("SD card is ready to use.");
      }
      else
      {  
         lcd.setCursor(0, 1);
         lcd.print("SD failed");
         
         Serial.println("SD card initialization failed");
         return;
      }

      // Create/Open file
      Data = SD.open("Data.txt", FILE_WRITE); // oder DATA.CSV??? 
      if(Data)
      {
          Serial.println("Writing to file...");
          Data.print("Date");
          Data.print(",");
          Data.print("Time");
          Data.print(",");
          Data.print("Count");
          Data.print(",");
          Data.println("Velocity");
          Data.close();
       }
      else
      {
           Serial.println("error opening Data.txt");
           
           lcd.setCursor(0, 1);
           lcd.print("Error Data");
      }


      ///SENSOR
      pinMode (sensor_1, INPUT) ; 
      pinMode(sensor_2, INPUT);  

      //LCD
      lcd.setCursor(0, 1);
      lcd.print("Setup end");
}

void loop() {
     Laufzeit = millis();
     if(digitalRead(sensor_2) == LOW and Licht == 0)// LOW for licht, High for Magnet
     {  
        Licht = 1;
        T_last_l = millis();
        Delay(150);
        last_sensor_check = sensor_2;    
     }
     else if(digitalRead(sensor_1)== LOW and Hall == 0)
     {
        Hall = 1;
        T_last_h = millis();
        Delay(200);
        last_sensor_check = sensor_1;
     }
     else if(digitalRead(sensor_1)== LOW and Hall == 1 and Licht == 1 and last_sensor_check = sensor_2)
     {  
        T_now_h = millis();
        Hall = 2;
        Velocity = 1/((T_now_h - T_last_h)/(1000.0));
        Delay(200);
     }
     else if (digitalRead(sensor_2) == LOW and Licht == 1 and Hall == 1 and last_sensor_check = sensor_1)
     {   
        T_now_l = millis();
        Licht = 2;
        Velocity = 1/((T_now_l - T_last_l)/1000.0);
        Delay(200);
     }
        DateTime now = rtc.now();    

        if(day_old == now.day())
        {
         
        if(hour_old == now.hour()and (abs(minute_old - now.minute())<1) )
        {
           if(Hall == 2)
           { 
             Total++;
             Count++;
             if(Count == 1)
             {
             Velocity_mean = Velocity;
             }
             else
             {
              Velocity_mean = (Velocity_mean + Velocity)/2.0;
             }
             //Reset
             Hall = 1;
             Licht = 1;
             Velocity = 0;
             //LCD
             lcd.setCursor(0, 1);
             lcd.print( String(C) + Point +  String(Count) + Space + V + Point + String(Velocity_mean) );
             Serial.println(Count);
           }
           else if(Licht == 2)
           { 
             Total++;
             Count++;
             if(Count == 1)
             {
             Velocity_mean = Velocity;
             }
             else
             {
              Velocity_mean = (Velocity_mean + Velocity)/2.0;
             }
             //Reset
             Hall = 0;
             Licht = 1;
             Velocity = 0;
             //LCD
             lcd.setCursor(0, 1);
             lcd.print( String(C) + Point +  String(Count) + Space + V + Point + String(Velocity_mean) );
             Serial.println(Count);
           }
        }
        else
       {

         if(Hall == 2)
           { 
             Total++;
             Count++;
             if(Count == 1)
             {
             Velocity_mean = Velocity;
             }
             else
             {
              Velocity_mean = (Velocity_mean + Velocity)/2.0;
             }
             //Reset
             Hall = 1;
             Licht = 0;
             Velocity = 0; 
           }
           else if(Licht == 2) 
           {
             Total++;
             Count++;
             if(Count == 1)
             {
             Velocity_mean = Velocity;
             }
             else
             {
              Velocity_mean = (Velocity_mean + Velocity)/2.0;
             }
             //Reset
             Hall = 0;
             Licht = 1;
             Velocity = 0;
            }
   
       // SD Card
       Data = SD.open("Data.txt", FILE_WRITE);
       Data.print(now.year(), DEC);
       Data.print('/');
       Data.print(now.month(), DEC);
       Data.print('/');
       Data.print(now.day(), DEC);
       Data.print(" (");
       Data.print(daysOfTheWeek[now.dayOfTheWeek()]);
       Data.print(") ");
       Data.print(",");
       Data.print(now.hour(), DEC);
       Data.print(':');
       Data.print(now.minute(), DEC);
       Data.print(",");
       Data.print(Count);
       Data.print(",");
       Data.println(Velocity_mean);
       Data.close();
       
       // LCD
       lcd.setCursor(0, 0);
       lcd.print(String(now.hour())+ Point + String(now.minute()) + Point + Space + String(Count) + Dash + String(Total));
        
       // Reset
       Count = 0;
       Velocity_mean = 0;
       hour_old = now.hour();
       minute_old = now.minute();
       }
       
       }
       else
       {
        day_old = now.day();
        Total = 0;
        }
}

void Delay(unsigned long ms)
{
    uint32_t start = micros();

    while (ms > 0) {
        yield();
        while ( ms > 0 && (micros() - start) >= 1000) {
            ms--;
            start += 1000;
        }
    }
}
