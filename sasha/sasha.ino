#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal.h>

#define DHTPIN1           11         // пин для подключения 1-го датчика
#define DHTPIN2           12         // пин для подключения 2-го датчика
#define THYRISTORPIN      13

#define TEMP_POWER_ON     3         // минимальная температура после которой надо включить обогрев (хотя бы один датчик)
#define TEMP_POWER_OFF    6.5       // максимальная температура до которой надо нагревать (оба датчика)
#define TEMP_FAIL_OFF     10        // если какой то из датчиков показывает температуру больше 10 градусов - аварийно выключаем

#define DHTTYPE           DHT22     // DHT 22 (AM2302)

DHT_Unified dht1(DHTPIN1, DHTTYPE);
DHT_Unified dht2(DHTPIN2, DHTTYPE);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

uint32_t last_query;
bool POWER_STATE_ON;
unsigned long CHANGE_STATE_TIME;
float t1, t2;

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  last_query = millis();
  dht1.begin();
  dht2.begin();
  CHANGE_STATE_TIME = 0;
  POWER_STATE_ON = false;
  pinMode(THYRISTORPIN, OUTPUT);
  digitalWrite(THYRISTORPIN, POWER_STATE_ON);
  t1 = 5;
  t2 = 5;
}

void loop() {
  sensors_event_t event1;  
  sensors_event_t event2;  

  if(millis()<CHANGE_STATE_TIME) CHANGE_STATE_TIME = millis(); // проверка на переполнение счетчика (может переполнится через 50 дней)

  dht1.temperature().getEvent(&event1);
  dht2.temperature().getEvent(&event2);
  if((millis() - last_query > 2000)||(millis()<last_query))
  {
    dht1.temperature().getEvent(&event1);
    if (isnan(event1.temperature)) {
      Serial.println("Error reading temperature1!");
    } else {
      t1 = event1.temperature;
      Serial.print("Temperature1: ");
      Serial.print(t1);
      Serial.println(" *C");
      lcd.setCursor(0, 0);
      lcd.print("temp1 = ");
      lcd.print(t1);
    }
    dht2.temperature().getEvent(&event2);
    if (isnan(event2.temperature)) {
      Serial.println("Error reading temperature2!");
    } else {
      t2 = event2.temperature;
      Serial.print("Temperature2: ");
      Serial.print(t2);
      Serial.println(" *C");
      lcd.setCursor(0, 1);
      lcd.print("temp2 = ");
      lcd.print(t2);
    }
    if(millis() - CHANGE_STATE_TIME > 120000) // прошло 120сек = 2 минуты после смены состояния, можно проверять
    {
      if(POWER_STATE_ON)
      {
        if(  ((t1 > TEMP_POWER_OFF)&&(t2 > TEMP_POWER_OFF)) // можно выключать если обе температуры превысили 6.5 градусов
           ||(t1 > TEMP_FAIL_OFF) || (t2 > TEMP_FAIL_OFF) ) // или какая то из температур почему то стала больше 10 градусов
        {
          POWER_STATE_ON = false;
          CHANGE_STATE_TIME = millis(); // запоминаем время последней смены состояния
          digitalWrite(THYRISTORPIN, LOW);
          Serial.println("Power OFF");
        }
      } else {
        if ( ((t1 < TEMP_POWER_ON) && (t2 < TEMP_FAIL_OFF))   // если первый датчик показывает меньше 3 градусов но при этом второй датчик не превышает 10 градусов
           ||((t2 < TEMP_POWER_ON) && (t1 < TEMP_FAIL_OFF)) ) // или если второй датчик показывает меньше 3 градусов но при этом первый датчик не превышает 10 градусов
        {                                                     // значит можно и нужно включать обогрев
          POWER_STATE_ON = true;
          CHANGE_STATE_TIME = millis(); // запоминаем время последней смены состояния
          digitalWrite(THYRISTORPIN, HIGH);
          Serial.println("Power ON");
        }
      }
    }
/*
    Serial.print("millis: ");Serial.print(millis());
    Serial.print(", last: ");Serial.print(last_query);
    Serial.print(", change: ");Serial.print(CHANGE_STATE_TIME);
    Serial.print(", power: ");Serial.println(POWER_STATE_ON);
*/    
    last_query = millis();
  }

}
