#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>  // library for I2C/SPI temperture and humidity sensor - BMP280
#include <BH1750.h>           // library for I2C light intensiy sensor - BH1750

#define mhz19pin D3           // PWM pin input for CO2 sensor - MH-Z19
#define preheattime 90        // (seconds) pre-heat startup time for MH-Z19
#define preheatstatus 0        // 1 - enable preheat ; 0 - disable preheat

Adafruit_BMP280 bmp280; 
BH1750 bh1750;

long lastMillis = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Weather Station v.2 test"));

  pinMode(mhz19pin, INPUT);
  bh1750.begin();
  bmp280.begin();

#if  preheatstatus 
  Serial.println(F("Preheating..."));
  for (int8_t i = 0; i < preheattime; i++) {
    uint8_t timeremain = preheattime - i;
    Serial.println(timeremain);
    delay(1000);
  }
#endif

}

void loop() {

  long nowMillis = millis(); 
  if (nowMillis - lastMillis > 1000) { // update value every 1 second

    lastMillis = nowMillis;

    uint16_t lux = bh1750.readLightLevel();
    uint16_t co2ppm = readCO2PWM();

    Serial.print(bmp280.readTemperature());
    Serial.println(" *C");

    Serial.print(bmp280.readPressure());
    Serial.println(" Pa");

    Serial.print(lux);
    Serial.println(" lux");

    Serial.print(co2ppm);
    Serial.println(" ppm");

    Serial.println(F("----------------"));
  }
}
// read CO2 
int readCO2PWM() {
  unsigned long th, tl, ppm_pwm = 0;
  do {
    th = pulseIn(mhz19pin, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm_pwm = 5000 * (th - 2) / (th + tl - 4);
  }
  while (th == 0);

  return ppm_pwm;
}

