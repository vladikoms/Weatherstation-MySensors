#include <EEPROM.h>                          // Подключаем библиотеку EEPROM
#include <OneWire.h>                         // Подключаем библиотеку OneWire
#include <DallasTemperature.h>               // Подключаем библиотеку DallasTemperature
#include "SparkFunHTU21D.h"                  // Подключаем библиотеку SparkFunHTU21D
#include <Wire.h>                            // Подключаем библиотеку Wire

#define ONE_WIRE_BUS 3                       // Датчик температуры подключен на pin 3

int AS5600_ADR = 0x36;                       // Адрес флюгера на шине I2C
int HTU21D_ADR = 0x80;                       // Адрес датчика влажности на шине I2C

const int raw_ang_hi = 0x0c;
const int raw_ang_lo = 0x0d;

OneWire oneWire(ONE_WIRE_BUS);                // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);          // Pass our oneWire reference to Dallas Temperature.

HTU21D myHumidity;                             // Указываем тип датчика влажности                        
byte Humidity=0;                              // Объявляем переменную Humidity - влажность HTU21D
float Temperature1=0;                          // Объявляем переменную Temperature1 - температура DS18S20
float Temperature2=0;                          // Объявляем переменную Temperature1 - температура HTU21D

byte tachPIN=4;                               // Объявлем переменную tachPIN - Вывод, используемый для подключения анемометра = 4
unsigned long sampleTime=1000;                // Объявлем переменную sampleTime - время, в течении которого происходит замер скорости ветра
unsigned int count;                           // Объявляем переменную count - счетчик количества импульсов оптопары анемометра
boolean kflag;                                // Объявляем переменную kflag - флаг импульса
unsigned long currentTime;                    // Объявляем переменную currentTime - текущее время
unsigned long startTime;                      // Объявляем переменную startTime - время начала замера скорости

float maxspeed;                               // Объявляем переменную maxspeed - максимальное значение скорости ветра
float wind;                                   // Объявляем переменную wind - значение скорости ветра
int x;                                        // Объявляем переменную x
boolean error;                                // Объявляем переменную error - флаг ошибки

// ******* Настройка MySensors *****

//#define MY_DEBUG
#define MY_RADIO_RFM95                                      // Указываем тип радиомодуля
#define MY_RFM95_FREQUENCY   433900000                      // Рабочая частота: 433.9 МГц
#define MY_RFM95_MODEM_CONFIGRUATION RFM95_BW125CR45SF128   // Конфигурируем радиомодуль
#define MY_RFM95_TX_POWER_DBM (20u)                         // Устанавливаем выходную мощность 20dBm (100mW)
#define MY_RFM95_ATC_MODE_DISABLED                          // Отключаем режим ATC

#include <MySensors.h>
#define CHILD_ID 1                                          // Указываем ID-адрес устройства
#define V_ID1 1                                             // Указываем ID-адрес основного датчика температуры
#define V_ID2 2                                             // Указываем ID-адрес резервного датчика температуры
#define V_ID3 3                                             // Указываем ID-адрес датчика влажности
#define V_ID4 4                                             // Указываем ID-адрес датчика скорости ветра
#define V_ID5 5                                             // Указываем ID-адрес датчика направления ветра

MyMessage msg1(V_ID1, V_TEMP);                              // Конфигурируем тип сообщения для основного датчика температуры
MyMessage msg2(V_ID2, V_TEMP);                              // Конфигурируем тип сообщения для резервного датчика температуры
MyMessage msg3(V_ID3, V_HUM);                               // Конфигурируем тип сообщения для датчика влажности
MyMessage msg4(V_ID4, V_WIND);                              // Конфигурируем тип сообщения для датчика скорости ветра
MyMessage msg5(V_ID5, V_DIRECTION);                         // Конфигурируем тип сообщения для датчика направления ветра

void presentation()                                         // Выполняем презентацию устройства контроллеру
{ 
  present(CHILD_ID, S_CUSTOM);                              
}

void setup()
{  
pinMode(tachPIN, INPUT);                          // Конфигурируем tachPIN как вход
digitalWrite(tachPIN, HIGH);                      // Подключаем подтягивающий резистор на tachPIN
Wire.begin();
myHumidity.begin();                               // Запускаем HTU21D
}

void loop()
{ 
 maxspeed=0;                                   // Обнуляем значение максимальной скорости ветра
 for (x=0;x<360; x++)                          // Цикл получения данных скорости ветра
{    
  wind = getWind();                            
  maxspeed = max(wind, maxspeed);              // Определение максимального значения скорости
//  Serial.print ("get Wind = ");
//  Serial.println (wind);
//  Serial.print ("max speed = ");
//  Serial.println (maxspeed);
}                                          

sensors.requestTemperatures();                 // Запрашиваем температуру с датчика DS18S20
Temperature1 = sensors.getTempCByIndex(0);     // Записываем температуру в переменную Temperature1

if (Temperature1 != -127.00 && Temperature1 != 85.00) // Проверяем исправность датчика температуры
{
Serial.print ("T DS18S20 = ");                  // Если датчик исправен, передаём температуру
Serial.println (Temperature1);                  // Записываем температуру DS18S20 в порт
send(msg1.set(Temperature1,1));                 // Передаём температуру с округлением до 1 знака после запятой
}

Wire.beginTransmission(HTU21D_ADR);             // Проверка состояния датчика HTU21D
error = Wire.endTransmission();
if (error == 0)                                 // Если датчик ответил, считываем показания
{
Temperature2 = myHumidity.readTemperature();    //  Запрашиваем температуру с датчика HTU21D
Humidity = myHumidity.readHumidity();           //  Запрашиваем влажность с датчика HTU21D
Serial.print ("Temp HTU21D = ");                // Записываем  n#T2#
Serial.println (Temperature2);                  // Записываем температуру HTU21D в порт
send(msg2.set(Temperature2,1));                 // Передаём температуру с округлением до 1 знака после запятой

Serial.print ("Humidity = ");                   // Записываем  n#H1#
Serial.println (Humidity);                      // Записываем влажность HTU21D в порт
send(msg3.set(Humidity));                       // Передаём влажность
}   

Serial.print ("Max Speed = "); 
Serial.println (maxspeed);                      // Записываем максимальную скорость ветра в порт
send(msg4.set(maxspeed,1));                     // Передаём скорость с округлением до 1 знака после запятой

Wire.beginTransmission(AS5600_ADR);             // Проверка состояния датчика AS5600
error = Wire.endTransmission();
if (error == 0)                                 // Если датчик ответил, считываем показания
{
Serial.print("Angle = "); 
Serial.println(getAngle());
send(msg5.set(getAngle(),1));                   // Передаём значение угла с округлением до 1 знака после запятой
}
Serial.println();

//delay (10000);                                  // Пауза 10c
}  
                                   
float getWind()                                   // Функция getWind - замер скорости ветра
{
count=0;                                          // Обнуляем счетчик количества импульсов
kflag=LOW;                                        // Сбрасываем флаг импульса в 0

if (digitalRead(tachPIN)==HIGH)                   // Считываем значение на входе tachPIN
{
kflag=HIGH;                                       // В случае если на входе tachpin логическая единица, устанавливаем kflag = 1
}

currentTime=0;                                    // Обнуляем текущее время замера
startTime=millis();                               // Записываем в переменную startTime текущее значение времени
while (currentTime<=sampleTime)                   // Цикл будет выполнятся до тех пор пока значение currentTime не превысит sampleTime
{
 
if (digitalRead(tachPIN)==HIGH)                   //  Если на входе "1"
{                                                
if (kflag==LOW)                                   //  Если состояние kflag = '0'
{
count++;                                          // Инкрементируем значение счетчика импульсов
}                                                
kflag=HIGH;                                       // Устанавливаем kflag = '1'
}                                                 
else                                              // В противном случае (если на входе "0")
{
if (kflag==HIGH)                                  // Если состояние kflag = '1'
{
  count++;                                        // Инкрементируем значение счетчика импульсов
}
  kflag=LOW;                                      // Устанавливаем kflag = '0'                                        
}

currentTime= millis()-startTime;                  // Определяем - сколько прошло времени от момента времени начала измерения, указанного в startTime
}
                         
return (count*0.42);                              // Формула расчета скорости ветра в зависимости от количества поступивших импульсов
}


float getAngle()                                  // Функция getAngle - считывание угла флюгера
{
// Read Raw Angle Low Byte
Wire.beginTransmission(AS5600_ADR);
Wire.write(raw_ang_lo);
Wire.endTransmission();
Wire.requestFrom(AS5600_ADR, 1);
while(Wire.available() == 0);
int lo_raw = Wire.read();
// Read Raw Angle High Byte
Wire.beginTransmission(AS5600_ADR);
Wire.write(raw_ang_hi);
Wire.endTransmission();
Wire.requestFrom(AS5600_ADR, 1);
while(Wire.available() == 0);
word hi_raw = Wire.read();
hi_raw = hi_raw << 8; //shift raw angle hi 8 left
hi_raw = hi_raw | lo_raw; //AND high and low raw angle value
return hi_raw *0.0879;
}
