#include <Adafruit_NeoPixel.h>

#define STRIP_PIN 7
#define ALARM_BTN_PIN 2
#define SONAR_PIN 4

const int knockSensor = A0; // the piezo is connected to analog pin 0
const int knockThreshold = 300;  // threshold value to decide when the detected sound is a knock or not

const int flexSensor = A1;

#define ALARM_TIME 3000

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(59, STRIP_PIN, NEO_GRB + NEO_KHZ800);
bool isAlarm = false;
unsigned long alarmTriggerTime = 0;
uint32_t alarmColor = Adafruit_NeoPixel::Color(200, 0, 0);


void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  pinMode(ALARM_BTN_PIN, INPUT);
  pinMode(SONAR_PIN, INPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned long now = millis();

  checkTilt();
  checkPezo();
  checkFlex();
  //checkSonar(now);

  if (now - alarmTriggerTime > ALARM_TIME) {
    isAlarm = false;
  }

  bool needLEDUpdate = false;
  if(isAlarm) {
     needLEDUpdate = alarm(now);
  }
  else {
    needLEDUpdate = flame(now);
  }
  
  if (needLEDUpdate) {
    strip.show();
  } 
}

void triggerAlarm()
{
  isAlarm = true;
  alarmTriggerTime = millis();
}

void checkTilt()
{
  int buttonState = digitalRead(ALARM_BTN_PIN);
  if (buttonState == HIGH) {
    triggerAlarm();
  }
}

void checkPezo()
{
  int sensorReading = analogRead(knockSensor);
  if (sensorReading >= knockThreshold) {
    triggerAlarm();
  }
}

void checkSonar(unsigned long now)
{
  static unsigned long lastSonarCheck = 0;
  if (now - lastSonarCheck < 100)
    return;
  lastSonarCheck = now;
  long pulse = pulseIn(SONAR_PIN, HIGH);
  int brightness = map(pulse, 500, 15000, 255, 25);
  Serial.println(brightness);
  strip.setBrightness(brightness);
  //if (pulse < 1200)
    //triggerAlarm();
}

void checkFlex()
{
  int sensor = analogRead(flexSensor);
  int degrees = map(sensor, 768, 853, 90, 0);
  if (degrees < 250)
    triggerAlarm();
  Serial.println(degrees);
}

bool flame(unsigned long now)
{
  static unsigned long lastInitialUpdate = 0;
  static unsigned long lastPropagateUpdate = 0;

  static const unsigned PROPAGATE_DELAY = 150;
  static const unsigned UPDATE_DELAY = 500;

  bool updated = false;

  if (now - lastInitialUpdate > UPDATE_DELAY) {
    strip.setPixelColor(0, strip.Color(random(255), random(255), random(255)));
    lastInitialUpdate = now;
    updated = true;
  }

  if (now - lastPropagateUpdate > PROPAGATE_DELAY) {
    for (uint16_t i = 1; i < strip.numPixels(); i++) {
      uint32_t color = strip.getPixelColor(i) / 2 + strip.getPixelColor(i - 1) / 2;
      strip.setPixelColor(i, color);
    }
    lastPropagateUpdate = now;
    updated = true;
  }
  return updated;
}

bool alarm(unsigned long now)
{
  static unsigned long lastUpdate = 0;
  static bool isOn = false;

  static const int PERIOD = 300;

  if (now - lastUpdate > PERIOD) {
    isOn = !isOn;
    uint32_t color = isOn ? alarmColor : strip.Color(0, 0, 0);
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, color);
    }
    lastUpdate = now;
    return true;
  }
  return false;
}

