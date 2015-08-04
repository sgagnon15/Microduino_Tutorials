#include <U8glib.h>
#include <Rtc_Pcf8563.h>
#include <Wire.h>
#define INTERVAL_Time 1000
#define INTERVAL_OLED 1000


#define setFont_L u8g.setFont(u8g_font_courB14)
#define setFont_M u8g.setFont(u8g_font_fixed_v0r)
#define setFont_S u8g.setFont(u8g_font_chikitar)

#define buzzer_pin 6

int micPin = A2; 
int micValue=0;
bool isRoar=false;

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);//定义OLED连接方式

unsigned long Time_millis = millis();
unsigned long OLEDShowTime = millis();

int timeHH, timeMM, timeSS;
int year,month,day;
String stringDate;
String stringTime;
Rtc_Pcf8563 rtc;


const unsigned char u8g_logo_bits[] U8G_PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x01,0xE0,
  0x03,0x00,0x00,0x00,0x00,0x7E,0x00,0xF0,0x01,0x00,0x00,0x00,
  0x00,0xFE,0xF9,0xF7,0x07,0x00,0x00,0x00,0x00,0x3C,0x00,0xF8,
  0x03,0x00,0x00,0x00,0x00,0xFC,0xF9,0xE1,0x03,0x00,0x00,0x00,
  0x00,0x38,0x00,0xF0,0x01,0x00,0x00,0x00,0x00,0xFC,0xFF,0x01,
  0x00,0x00,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0xFC,0xEF,0xF9,0x8F,0xD7,0x73,0xF1,0xC1,0x3B,0x9F,0xFF,
  0xFF,0x1E,0x3E,0x00,0x00,0xBC,0xEF,0xC1,0xE1,0x9F,0xFF,0xDD,
  0xE3,0x3F,0xCC,0xE1,0xF0,0xBF,0x7B,0x00,0x00,0x3C,0xF7,0xE1,
  0xE1,0x9F,0xFF,0xC6,0xF7,0x3E,0x8E,0xF3,0xF0,0xFF,0xF8,0x00,
  0x00,0x3C,0xF3,0xE1,0xF1,0x93,0xFF,0xE6,0xF7,0x3C,0x8F,0xF7,
  0xF0,0xFF,0xFC,0x00,0x00,0x7C,0xF2,0xE1,0xF1,0x83,0x87,0xFE,
  0xF7,0x39,0xFF,0xF7,0xF0,0xFF,0xFF,0x00,0x00,0x7C,0xF0,0xE3,
  0xF3,0xA3,0x03,0xFE,0xF7,0x3F,0xFF,0xF7,0x71,0xFC,0xFF,0x00,
  0x00,0x7C,0xF8,0xE3,0xF3,0xBF,0x03,0xFE,0xE3,0x3F,0xFF,0xF3,
  0x71,0xDC,0x7F,0x00,0x00,0x7E,0xFC,0xE7,0xE3,0xBF,0x03,0xFC,
  0xE3,0x3F,0xFE,0xF3,0x71,0x9C,0x7F,0x00,0x00,0xC1,0x03,0xF8,
  0xCF,0xE7,0x0F,0xF0,0x00,0x7F,0xFC,0xFC,0xFF,0x3E,0x1E,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

void osd_logo()     
{
  u8g.firstPage();  
  do 
  {
    u8g.drawXBMP( 0, 32, 128, 18, u8g_logo_bits);
  } 
  while( u8g.nextPage() );
}

void setup() {
  Serial.begin(9600);
  pinMode(buzzer_pin,OUTPUT);
  pinMode(micPin,INPUT);

  osd_logo();
  delay(3000);

  initTime();
}

void loop() {
  updateMic();
	updateTime();
	updateOLED();
  updateAlarm();
}


void initTime() {
  rtc.initClock();
  //set a time to start with.
  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  rtc.setDate(11, 4, 6, 0, 15);
  //hr, min, sec
  rtc.setTime(12, 0, 0);
}


void getCurrentTime() {

  rtc.formatDate();
  rtc.formatTime();

  timeHH=rtc.getHour();
  timeMM=rtc.getMinute();
  timeSS=rtc.getSecond();
  year=2000+rtc.getYear();
  month=rtc.getMonth();
  day=rtc.getDay();


  stringDate=String(year);
  stringDate+="/";
  stringDate+=month;
  stringDate+="/";
  stringDate+=day;

  stringTime=String(timeHH);
  stringTime+=":";
  stringTime+=timeMM;
  stringTime+=":";
  stringTime+=timeSS;

}

void updateTime() {
  //update GPS and UT during INTERVAL_GPS
  if (Time_millis > millis()) Time_millis = millis();
  if(millis()-Time_millis>INTERVAL_Time) {
    getCurrentTime();
    Time_millis = millis();
  } 
}

void updateOLED() {
  //OLED display
  if (OLEDShowTime > millis()) OLEDShowTime = millis();
  if(millis()-OLEDShowTime>INTERVAL_OLED) {
    //convert RA DEC to AZ Alt
    OLEDShow(); //调用显示库
    OLEDShowTime = millis();
  } 
}

void OLEDShow() {
    u8g.firstPage();
    do {
      setFont_L;
      u8g.setPrintPos(5, 20);
      u8g.print(stringDate);

      u8g.setPrintPos(5, 40);
      u8g.print(stringTime);

    } while( u8g.nextPage() );
}


void updateAlarm() {
  if(timeHH==7&&timeMM<5) {
      if(micValue>500) {
      isRoar=true;
    }
    if(isRoar) {
      noTone(buzzer_pin);
    } else {
      tone(buzzer_pin,500); //在端口输出频率
    }
  } else {
    isRoar=false;
    noTone(buzzer_pin);
  }
}

void updateMic() {
  micValue = analogRead(micPin);
  //value = map(micValue, 0, 1023, 0, 255);
  Serial.println(micValue);
}