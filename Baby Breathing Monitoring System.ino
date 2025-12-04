/* 
  Baby Breathing Monitoring System
  Responsibilities:
  - detect breathing movement (rise/fall of chest)
  - display breaths per minute
  - display warning and trigger alert LED and BUZZER if breathing stops for >= 10 seconds

  Source on MPU6050: doc.sunfounder.com
  We are only using the MPU6050 as an accelerometer. It detects acceleration in the x axis, y
  axis, and z axis. We are concerned with the z axis only (up/down). "The z axis acceleration is 
  1 gravity unit." +1 g for rightside up and -1 g for upside down. 1 g is about 9.81 m/s^2.
  Any change in g will indicate BREATHING status. However, for more accurate readings, an infants
  inhale will read about 1.005 g; exhale will read about 0.995 g. 1.005-0.995 = 0.01. 
  We will need to install the MPU6050 Library: https://github.com/adafruit/Adafruit_MPU6050

  Sources on normal breathing patterns of an infant:
  stanfordchildrens.org, article "Brething Problems" 
  my.clevelandclinic.org, article "Periodic Breathing in Newborns"

  Sources on hardware connections and code initializations:
  MPU6050: https://docs.wokwi.com/parts/wokwi-mpu6050
  LED's: https://docs.wokwi.com/parts/wokwi-led
  Push Buttons: https://docs.wokwi.com/parts/wokwi-pushbutton
  Buzzer: https://docs.wokwi.com/parts/wokwi-buzzer
*/
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// constants
#define ALERT_LED 6
#define NORMAL_LED 7
#define BUZZER 8
#define START_BUTTON 5
#define STOP_BUTTON 3
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define DEBOUNCE_DELAY 200
const unsigned long BPM_PRINT = 10000; // print every 10 seconds
const unsigned long APNEA_TIMEOUT = 10000; // apnea detected after 10 seconds of no breathing
const float MOVE_THRESHOLD = 0.08; // VERY slight movement should indicate breathing
// accelerometer
Adafruit_MPU6050 mpu;
// system
bool start = false;
bool isMessageSent = false;
bool notMessageSent = false;
bool monitorMessageSent = false;
unsigned long prevMoveTime = 0;
bool not_breathing = false;
float AccelZ = 0;
/* 
  it's normal for there to be pauses in breathing. An infant's breathing may pause for 5-10 
  seconds before continuing. Apnea timeout should therefore be 10 seconds or 10,000 milliseconds.
  (my.clevelandclinic.org, "Periodic Breathing in Newborns").
*/
unsigned long lastPress = 0;
unsigned long lastBreathTime = 0;
float breathCount = 0.0;
unsigned long lastBreathPerMinTime = 0;
/* 
  an infants breathing should be between 40-60 breaths per minute when awake and between
  30-40 breaths per minute when sleeping. (stanfordchildrens.org, "Brething Problems"). 
  The minimum amount of time between breaths when sleeping is therefore 30 breaths per min = 
  0.5 breaths per sec = 500 milliseconds.
*/
const unsigned long minTimeBetweenBreaths = 500;

void setup() {
  Serial.begin(9600);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(F("_Breathing Monitor_"));
  display.display();

  pinMode(ALERT_LED, OUTPUT);
  pinMode(NORMAL_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(STOP_BUTTON, INPUT_PULLUP);
  
  Wire.begin();
  if(!mpu.begin()){
    Serial.println(F("MPU6050 failed"));
    while(1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  AccelZ = a.acceleration.z;
  prevMoveTime = millis(); 
}

void system_reset(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("_Breathing Monitor_"));
  display.println(F("Monitoring stopped."));
  display.display();
  Serial.println("Monitoring stopped.");

  digitalWrite(ALERT_LED, LOW);
  digitalWrite(NORMAL_LED, LOW);
  digitalWrite(BUZZER, LOW);
  monitorMessageSent = false;
  isMessageSent = false;
  notMessageSent = false;
  start = false;
  prevMoveTime = millis();
  lastPress = 0;
  lastBreathTime = 0;
  breathCount = 0.0;
  lastBreathPerMinTime = 0;
}

void loop() {
  if(millis() - lastPress > DEBOUNCE_DELAY){
    if(digitalRead(START_BUTTON) == LOW){
      start = true;
      if(!monitorMessageSent){
        display.println(F("Monitoring..."));
        display.display();
        Serial.println("Monitoring...");
        monitorMessageSent = true;
      }
      lastPress = millis();
    }
    if(digitalRead(STOP_BUTTON) == LOW){
      start = false;
      system_reset();
      lastPress = millis();
    }
  }
  if(start){
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float curAccelZ = a.acceleration.z;
    float change = abs(curAccelZ - AccelZ);
    unsigned long curTime = millis();
    if(change >= MOVE_THRESHOLD){ // change in g detected; 1.005-0.995=0.01=MOVE_THRESHOLD
      digitalWrite(NORMAL_LED, HIGH);
      digitalWrite(ALERT_LED, LOW);
      digitalWrite(BUZZER, LOW);
      if(millis() - lastBreathTime > minTimeBetweenBreaths){ // count each breath
        breathCount += 0.5; // so that rise+fall = 1 full breath (0.5+0.5 = 1)
        Serial.print("Breath count: ");
        Serial.println(breathCount);
        lastBreathTime = curTime;
      }
      prevMoveTime = millis();
    }
    if(millis() - prevMoveTime > APNEA_TIMEOUT){
      not_breathing = true;
      /*digitalWrite(NORMAL_LED, LOW);
      digitalWrite(ALERT_LED, HIGH);
      tone(BUZZER, 260, 260);*/
    }
    else{
      not_breathing = false;
    }
    if(not_breathing){
      digitalWrite(NORMAL_LED, LOW);
      digitalWrite(ALERT_LED, HIGH);
      tone(BUZZER, 260, 260);
      display.clearDisplay();
      display.setCursor(0,0);
      display.println(F("_Breathing Monitor_"));
      display.println(F("BREATHING STOPPED"));
      display.display();
    }
    else if(millis() - lastBreathPerMinTime >= BPM_PRINT){ // if its been 10 sec since last print
      lastBreathPerMinTime = curTime;
      /* 
        Breath count is reset after 10 seconds. 10 seconds * 6 = 60 seconds = 1 minute. 
        bpm is breath count (aquired over 10 seconds) * 6.
      */
      float bpm = (breathCount * 6.0); // NOTE: if we change BPM_PRINT, we MUST change 6.0 too.
      
      display.clearDisplay();
      display.setCursor(0,0);
      display.println(F("_Breathing Monitor_"));
      display.println(F("Monitoring..."));
      display.print(F("BPM: "));
      display.println(bpm);
      display.display();
      /* 
        Checking for abnormal bpm (for both sleeping and awake, for case if child wakes up).
        Recall: 30-40 when sleeping; 40-60 when awake */
      if(bpm < 30 || bpm > 60){
        display.println(F("Irregular bpm!!"));
        display.display();
      }
      Serial.print("Breaths per minute: ");
      Serial.println(bpm);
      breathCount = 0; // reset breath count
    }
    AccelZ = curAccelZ;
  }
}