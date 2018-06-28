#!/bin/bash

# Общие настройки

LOG_FILE='/var/tmp/narodmon.log'
TEMP_FILE='/var/tmp/narodmon'

# Настройки для narodmon.ru
DEVICE_NAME='domoticz'
DEVICE_MAC='YOUMACADDRESS'
SENSOR_TEMP_ID1='T1'
SENSOR_TEMP_ID2='T2'
SENSOR_HUM_ID='H1'
SENSOR_SPEED_ID='S1'
SENSOR_DIRECTION_ID='D1'

# Номер датчика, который достаем
IDX=1

#===============================================================================
# Достаем нужный параметр из ответа
function jsonValue() {
KEY=$1
num=$2
awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'$KEY'\042/){print $(i+1)}}}' | tr -d '"' | sed -n ${num}p
}
#===============================================================================
# Запрашиваем данные с датчика IDX
DOMOTICZ=`curl -s --connect-timeout 2 --max-time 5 "http://127.0.0.1:8080/json.htm?type=devices&rid=$IDX"`
#echo $DOMOTICZ
if [ -z "$DOMOTICZ" ]
then
exit 1
else
TEMP1=$(echo $DOMOTICZ | jsonValue Temp 2)
# Округляем температуру до 1 знака после запятой
TEMP1=$(echo "scale=1 ; $TEMP1/1" |bc)
echo $TEMP1
fi

# Номер датчика, который достаем
IDX=2

#===============================================================================
# Достаем нужный параметр из ответа
function jsonValue() {
KEY=$1
num=$2
awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'$KEY'\042/){print $(i+1)}}}' | tr -d '"' | sed -n ${num}p
}
#===============================================================================
# Запрашиваем данные с датчика IDX
DOMOTICZ=`curl -s --connect-timeout 2 --max-time 5 "http://127.0.0.1:8080/json.htm?type=devices&rid=$IDX"`
#echo $DOMOTICZ
if [ -z "$DOMOTICZ" ]
then
exit 1
else
TEMP2=$(echo $DOMOTICZ | jsonValue Temp 2)
# Округляем температуру до 1 знака после запятой
TEMP2=$(echo "scale=1 ; $TEMP2/1" |bc)
echo $TEMP2
fi

# Номер датчика, который достаем
IDX=3

#===============================================================================
# Достаем нужный параметр из ответа
function jsonValue() {
KEY=$1
num=$2
awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'$KEY'\042/){print $(i+1)}}}' | tr -d '"' | sed -n ${num}p
}
#===============================================================================
# Запрашиваем данные с датчика IDX
DOMOTICZ=`curl -s --connect-timeout 2 --max-time 5 "http://127.0.0.1:8080/json.htm?type=devices&rid=$IDX"`
#echo $DOMOTICZ
if [ -z "$DOMOTICZ" ]
then
exit 1
else
HUM=$(echo $DOMOTICZ | jsonValue Humidity 1)
echo $HUM
fi

# Номер датчика, который достаем
IDX=4

#===============================================================================
# Достаем нужный параметр из ответа
function jsonValue() {
KEY=$1
num=$2
awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'$KEY'\042/){print $(i+1)}}}' | tr -d '"' | sed -n ${num}p
}
#===============================================================================
# Запрашиваем данные с датчика IDX
DOMOTICZ=`curl -s --connect-timeout 2 --max-time 5 "http://127.0.0.1:8080/json.htm?type=devices&rid=$IDX"`
#echo $DOMOTICZ
if [ -z "$DOMOTICZ" ]
then
exit 1
else
SPEED=$(echo $DOMOTICZ | jsonValue Speed 1)
DIRECTION=$(echo $DOMOTICZ | jsonValue Direction 1)
echo $SPEED
echo $DIRECTION
fi

# В TEMP температура с датчика в HUM влажность
# В SPEED скорость ветра в DIRECTION направление
# Формируем временный файл

echo "#$DEVICE_MAC#$DEVICE_NAME" >> $TEMP_FILE
echo "#$SENSOR_TEMP_ID1#$TEMP1" >> $TEMP_FILE
echo "#$SENSOR_TEMP_ID2#$TEMP2" >> $TEMP_FILE
echo "#$SENSOR_HUM_ID#$HUM" >> $TEMP_FILE
echo "#$SENSOR_SPEED_ID#$SPEED" >> $TEMP_FILE
echo "#$SENSOR_DIRECTION_ID#$DIRECTION" >> $TEMP_FILE

#===============================================================================

# Передача данных на Narodmon.ru
# В случае успеха сервер отвечает "ОК", иначе возвращает текст ошибки

# Перед отправкой проверяем:
# 1) в файле нет повторяющихся строк, иначе убираем дубликаты;
# 2) что передаем не пустой файл
# LC=`cat /var/log/narodmon | wc -l`
# if [ $LC -ge 2 ]
# then
#echo "$(sed '/##/d' $TEMP_FILE | awk '!($0 in a) {a[$0];print}')" > $TEMP_FILE
echo "##" >> $TEMP_FILE
#cat $TEMP_FILE >> $LOG_FILE
RESULT=$(cat $TEMP_FILE | nc narodmon.ru 8283)
#echo "$RESULT\n" >> $LOG_FILE
echo $RESULT
#if [ "$RESULT" == 'OK' ]
#then
cp /dev/null $TEMP_FILE
#fi
#if [ "$RESULT" == "ERROR NO CHANGES" ] || [ "$RESULT" == "429 Too Many Requests" ]
#then
#cp /dev/null /tmp/narodmon
#fi
#fi
