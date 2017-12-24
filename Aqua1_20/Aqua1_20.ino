//Часы - с 18 по 21 выводы
//Пищалка - 46 - плюс, 53 - минус
//Реле - с 24 до 32 выводы (управляющие)
//Датчик DS - 40 вывод
//Датчик DHT - 41 вывод

#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#include <Wire.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include <avr/wdt.h>
#define ONE_WIRE_BUS 40
#define DHTPIN 41 
#define DHTTYPE DHT22
#define DS_RES 9
#define START_AUX 9
#define START_MAIN 11
#define STOP_AUX 21
#define STOP_MAIN 19
#define START_AUX_WEEKEND 10
#define START_MAIN_WEEKEND 12
#define STOP_MAIN_WEEKEND 20
#define STOP_AUX_WEEKEND 22
#define T_NORM 25
#define T_DELTA 2
#define RELAY_COUNT 8

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
DeviceAddress relayThermometer;
RTC_DS1307 RTC;
LCDKeypad lcd;

byte c_sel[8] = {
  B11000,
  B11100,
  B11110,
  B11111,
  B11111,
  B11110,
  B11100,
  B11000,
};

byte c_clr[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte c_up[8] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
};

byte c_down[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
};

byte c_relayon[8] = {
  B01110,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B01110,
};

byte c_relayoff[8] = {
  B01110,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B01110,
};
//Счётчики миллоисекунд
unsigned long ct, lt1, lt2, lt3, lt4, btntm, almtm, dswitch, dswitch_old;
//Переменная "Первого запуска"
int init1=1;
//Остальные переменные
int h, wd, m, i1, relnum, relco, tempmode;
int relstate[8];
int reldir=0;
int a=20;
int b=0;
int s=0;
int selt=4;
float t=0;
float relt=0;
float tdht=0;
float hdht=0;
int cnt=0;
int pressed=-1;
int btnprs=-1;
int alm=1;
String wd_str;


void setup() {
//Задаём ноги для реле - с 24 по 32
  for (i1=0; i1<=7; i1++) {
    pinMode(24+i1,OUTPUT);
    digitalWrite(24+i1, LOW);
    relstate[i1]=0;
  }
  Wire.begin();
  RTC.begin();
  dht.begin();
//Ноги на пищалку
  pinMode(46, OUTPUT);
  pinMode(53, OUTPUT);
//Ноги на питание часов
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
//Ноги на питание датчиков
  pinMode(34, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
//Начальное состояние выводов
  digitalWrite(46, LOW);
  digitalWrite(53, LOW);
  digitalWrite(19, HIGH);
  digitalWrite(18, LOW);
  digitalWrite(34, HIGH);
  digitalWrite(36, LOW);
  digitalWrite(37, LOW);
  sensors.begin();
  sensors.getAddress(relayThermometer, 0);
  sensors.getAddress(insideThermometer, 1);
  sensors.setResolution(relayThermometer, DS_RES);
  sensors.setResolution(insideThermometer, DS_RES);


  //##########################################################################
  //
  //
  // Раскомментировать для задания времени, равного времени компиляции скетча 
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  //
  //
  //##########################################################################

  //Настройка и инициализация экрана
  lcd.createChar(1,c_sel);
  lcd.createChar(2,c_clr);
  lcd.createChar(3,c_up);
  lcd.createChar(4,c_down);
  lcd.createChar(5,c_relayon);
  lcd.createChar(6,c_relayoff);
  lcd.begin(16,2);
  lcd.clear();
  lcd.print("Aqua1-20171224W");
  delay(1000);
  lcd.clear();
  lcd.print("WHOA");
  delay(700);
  lcd.clear();
  lcd.print("IT STILL");
  delay(700);
  lcd.clear();
  lcd.print("ROCKS!");
  delay(700);
  lcd.clear();
  //Инициализация таймера
  ct=millis();
  dswitch_old=millis();
  //Ещё кое-что
  tempmode=0;
  lt1=ct;
  lt2=ct;
  
  //WATCHDOG
  wdt_enable(WDTO_4S);
}

//Основной цикл
void loop() {
  lcd.setCursor(0,0);
  //digitalWrite(30,LOW);
  shTime();
  shTemp();
  getTemp();
  btnPrs();
  alarm();
}

// Отображаем время
void shTime() {
  ct=millis();
  DateTime now = RTC.now();
// Время перерисовыввается только при смене часов, минут, секунд
  if(ct>=lt1+250||init1==1) {
    if (now.second()!=s||init1==1) {
      //Каждую секуну сбрасываем пса
      //Если через 4 секунды не сбросить - будет ресет
      wdt_reset();
      lcd.setCursor(0,0);
      if (now.hour()<10) {
        lcd.print("0");
      }
      lcd.print(now.hour());
      lcd.print(":");
      lcd.setCursor(3,0);
      if (now.minute()<10) {
        lcd.print("0");
      }
      lcd.print(now.minute());
      lcd.print(":");
      lcd.setCursor(6,0);
      if (now.second()<10) {
        lcd.print("0");
      }
      lcd.print(now.second());
      init1=0;
//Возращаем счётчик миллисекунд
      lt1=ct;
    }
  }
//Запоминаем текущее время
  s=now.second();
  m=now.minute();
  h=now.hour();
  wd=now.dayOfTheWeek();
  //Задаём словесный вид дня недели
  switch(wd)
  {
    case 0:
    wd_str="SU";
    break;
    case 1:
    wd_str="MO";
    break;
    case 2:
    wd_str="TU";
    break;
    case 3:
    wd_str="WE";
    break;
    case 4:
    wd_str="TH";
    break;
    case 5:
    wd_str="FR";
    break;
    case 6:
    wd_str="SA";
    break;
  }
}
//Проверяем нажатие кнопок
int checkBtn() {
  if (ct>=lt2+50) {
    if(lcd.button()!=KEYPAD_NONE) {
      pressed=lcd.button();
      btntm=millis();
      // Действие только при отпускании кнопки
        while(lcd.button()!=KEYPAD_NONE) {
        }
      while((millis()<(btntm+50))) {
      }
    }
    else {
      pressed=-1;
    }
    return pressed;
    lt2=ct;
  }
}
//Обрабатываем нажатие кнопок
void btnPrs() {
  btnprs=checkBtn();
  lcd.setCursor(0,1);
  switch (btnprs) {
    //RIGHT
    case 0:
    selt=1;
    strClear(1);
    break;
    //UP
    case 1:
    if(selt<4) {
      selt++;
    }
    else {
      selt=0;
    }
    strClear(1);
    break;
    //DOWN
    case 2:
    if(selt>0) {
      selt--;
    }
    else {
      selt=4;
    }
    strClear(1);
    break;
    //LEFT
    case 3:
    selt=2;
    strClear(1);
    break;
    //SELECT
    case 4:
    strClear(1);
    selt=0;
    break;
  }
  btnprs=-1;
}
//Управление реле
void alarm() {
  //Воскресенье
  if(wd==0)
  {
    //Основной свет
    if(h>=START_MAIN_WEEKEND&&h<STOP_MAIN) {
      RelayOn(0);
    }
    if(h>=STOP_MAIN||h<START_MAIN_WEEKEND) {
      RelayOff(0);
    }
    //Дополнительный свет
    if(h>=START_AUX_WEEKEND&&h<STOP_AUX) {
      RelayOn(1);
    }
    if(h>=STOP_AUX||h<START_AUX_WEEKEND) {
      RelayOff(1);
    }
  }
  //Суббота
  else if(wd==6)
  {
    //Основной свет
    if(h>=START_MAIN_WEEKEND&&h<STOP_MAIN_WEEKEND) {
      RelayOn(0);
    }
    if(h>=STOP_MAIN_WEEKEND||h<START_MAIN_WEEKEND) {
      RelayOff(0);
    }
    //Дополнительный свет
    if(h>=START_AUX_WEEKEND&&h<STOP_AUX_WEEKEND) {
      RelayOn(1);
    }
    if(h>=STOP_AUX_WEEKEND||h<START_AUX_WEEKEND) {
      RelayOff(1);
    }
  }
  //Пятница
  else if(wd==5)
  {
    //Основной свет
    if(h>=START_MAIN&&h<STOP_MAIN_WEEKEND) {
      RelayOn(0);
    }
    if(h>=STOP_MAIN_WEEKEND||h<START_MAIN) {
      RelayOff(0);
    }
    //Дополнительный свет
    if(h>=START_AUX&&h<STOP_AUX_WEEKEND) {
      RelayOn(1);
    }
    if(h>=STOP_AUX_WEEKEND||h<START_AUX) {
      RelayOff(1);
    }
  }
  else
  {
     //Основной свет
    if(h>=START_MAIN&&h<STOP_MAIN) {
      RelayOn(0);
    }
    if(h>=STOP_MAIN||h<START_MAIN) {
      RelayOff(0);
    }
    //Дополнительный свет
    if(h>=START_AUX&&h<STOP_AUX) {
      RelayOn(1);
    }
    if(h>=STOP_AUX||h<START_AUX) {
      RelayOff(1);
    }
  }

  //Светодиодная лента
  //if(h>=7&&h<22) {
  //  RelayOn(4);
  //}
  //if(h>=22||h<7) {
  //  RelayOff(4);
  //}
//Вентиляторы  
 // if(t>=(T_NORM+T_DELTA)&&t<50) {
 //   RelayOn(7);
 // }
 // if(t<=T_NORM) {
 //   RelayOff(7);
  //}

  //Обогреватель
  if(t<=(T_NORM-T_DELTA)&&t>0) {
    RelayOn(2);    
  }
  if(t>=T_NORM) {
    RelayOff(2);
  }
}
//Включение реле
int RelayOn(int relnum) {
  //Проверяем состояние вывода
  if (relstate[relnum]==0) {
    digitalWrite(24+relnum, HIGH);
    relstate[relnum]=1;
    return 0;
  }
  else {
    //Возвращаем 1, если реле уже было включено
    return 1;
  }
}
//Выключение реле
int RelayOff(int relnum) {
  //Проверяем состояние вывода
  if (relstate[relnum]==1) {
    digitalWrite(24+relnum, LOW);
    relstate[relnum]=0;
    return 0;
  }
  else {
    //Возвращаем 1, если реле уже выключено
    return 1;
  }
}

//Отображение на экране
void shTemp() {
  dswitch=millis();
  if((dswitch-dswitch_old)>3000)
  {
    dswitch_old=dswitch;
    if(tempmode==5)
    {
      tempmode=0;
    }
    else
    {
      tempmode++;
    }
  }
  //Устраняем мерцание
  ct=millis();
  if(ct>=lt4+1000) {
    lcd.setCursor(0,1);
    //Выбор кнопками
    switch(selt) {
      case 0:
      shRelay();
      break;
      case 1:
      lcd.print(t);
      break;
      case 2:
      shTR();
      break;
      case 3:
      lcd.print(hdht);
      lcd.print("%/DHT");
      break;
      case 4:
      //Циклическое переключение по времени
      switch(tempmode)
      {
        case 0:
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("Water: ");       
        lcd.print(t);
        break;
        case 1:
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("Ambient: ");       
        lcd.print(tdht);
        break;
        case 2:
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("Humidity: ");       
        lcd.print(hdht);
        break;
        case 3:
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("Relay: ");       
        lcd.print(relt);
        break;
        case 4:
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("Status: ");
        shRelay();
        break;
        case 5:
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("Weekday: ");
        lcd.print(wd_str);
        break;
      }
      break;
    }
    lt4=ct;
  }
}

void getTemp() {
  ct=millis();
  if(ct>=lt3+10000) {
    sensors.requestTemperatures();
    t=sensors.getTempC(insideThermometer);
    relt=sensors.getTempC(relayThermometer);
    hdht=dht.readHumidity();
    tdht=dht.readTemperature();
    lt3=ct;
  }
}
void strClear(int dispstr) {
  lcd.setCursor(0,1);
  int clrc;
  for(clrc=0;clrc<=15;clrc++) {
    lcd.setCursor(clrc, dispstr);
    lcd.write(2);
  }
}

void shTR() {
  strClear(1);
  lcd.setCursor(0,1);
  lcd.print("Ht ");
  if(relstate[3]==1) {
    lcd.print("ON ");
  }
  if(relstate[3]==0) {
    lcd.print("OFF ");
  }
  lcd.print("Vnt ");
  if(relstate[7]==1) {
    lcd.print("ON ");
  }
  if(relstate[7]==0) {
    lcd.print("OFF");
  }
}

void shRelay() {
  strClear(1);
  int relcnt;
  lcd.setCursor(0,1);
  for(relcnt=0;relcnt<=(RELAY_COUNT - 1);relcnt++) {
    if(relstate[relcnt]==1) {
      if(relcnt==3) {
        lcd.write(3);
      }
      if(relcnt==7) {
        lcd.write(4);
      }
      else {
        lcd.write(5);
      }
    }
    if(relstate[relcnt]==0) {
      lcd.write(6);
    }
  }
}


