A simple fishtank controller for Arduino.
Tested on Mega2560, should work with Uno as well.
Works since 2013 quite well - just pay attention to below-average precision of RTC: actually it drifts about a minute a month, but error is icreased greatly upon power outage more than a few minutes (backup CR2032 battery on RTC modules does little help here).
Typical time adjustment is to uncomment time setting string, upload sketch, then comment it out and upload again.

Required libraries:
Adafruit_Sensor
DallasTemperature
DHT-sensor-library-1.3.0
LCDKeypad
RTClib

Default wiring:
Double DS18B20 (one for relay and one for water) - data pin 40
DHT22 - data pin 41
DS1307 RTC on its default pin
Relays on pins 24 to 32
Others:
Pins 46 and 53 to buzzer
Pins 18 and 19 to RTC power
Pins 34, 36, 37 to sensors power
LCD-keypad module as a shield.