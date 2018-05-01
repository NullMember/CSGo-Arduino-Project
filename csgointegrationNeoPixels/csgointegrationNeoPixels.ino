#include <Adafruit_NeoPixel.h>

uint8_t pixels = 8;

Adafruit_NeoPixel neoPixel = Adafruit_NeoPixel(pixels, 6, NEO_GRB + NEO_KHZ800);

char Buffer[3];
uint8_t health, bombStatus, bombBegin, ammoBegin, ammoStatus;
uint16_t bombBlinkLength;
unsigned long bombPreviousLength, ammoBlinkLength;
uint8_t hRed, hGreen, hBlue;
uint8_t ledBrightness = 127;

void readSerialData(){
  Serial.readBytes(Buffer, 1);
  if(Buffer[0] == 'b'){
    readBombData();
	Serial.write('b');
  }
  else if(Buffer[0] == 'h'){
    readHealthData();
	Serial.write('h');
  }
  else if(Buffer[0] == 'a'){
    readAmmoData();
	Serial.write('a');
  }
}

uint8_t chartoInt(){
  Serial.readBytes(Buffer, 3);
  Buffer[0] = (Buffer[0] - '0');
  Buffer[1] = (Buffer[1] - '0');
  Buffer[2] = (Buffer[2] - '0');
  Buffer[0] *= 100;
  Buffer[1] *= 10;
  return Buffer[0] + Buffer[1] + Buffer[2];
}

void readHealthData(){
  health = chartoInt();
  if(health > 50){
    health = health - 50;
    hRed = 50 - health;
    hGreen = 50 + health;
    hBlue = 0;
  }
  else if (health <= 50){
    health = 50 - health;
    hRed = 50 + health;
    hGreen = 50 - health;
    hBlue = 0;
  }
}

void driveHealth(){
  driveRGB(hRed, hGreen, hBlue);
}

void readBombData(){
  Serial.readBytes(Buffer, 1);
  if(Buffer[0] == 'p'){
    bombStatus = 1;
    bombBegin = 1;
  }
  else if(Buffer[0] == 'd'){
    bombStatus = 2;
  }
  else if(Buffer[0] == 'e'){
    bombStatus = 3;
  }
  else if(Buffer[0] == 'n'){
    bombStatus = 0;
    bombBlinkLength = 1000;
  }
}

void bombBlink(){
  for(size_t i = 0; i < 100; i++){
    driveRGB(i, 0, 0);
    delay(2);
  }
  for(size_t i = 100; i > 0; i--){
    driveRGB(i, 0, 0);
    delay(2);
  }
  driveRGB(hRed, hGreen, hBlue);
}

void driveBomb(){
  if(bombBegin == 1){
    bombBlink();
    bombPreviousLength = millis();
    bombBegin = 0;
  }
  else if(bombStatus == 1){
    if((millis() - bombPreviousLength) > bombBlinkLength){
      bombBlink();
      bombBlinkLength -= 25;
      bombPreviousLength = millis();
    }
  }
  else if(bombStatus == 2){
    bombPreviousLength = millis();
    while((millis() - bombPreviousLength) < 5000){
      driveRGB(0, 100, 0);
    }
    bombStatus = 0;
  }
  else if(bombStatus == 3){
    bombPreviousLength = millis();
    while((millis() - bombPreviousLength) < 5000){
      driveRGB(100, 0, 0);
    }
    bombStatus = 0;
  }
}

void readAmmoData(){
  Serial.readBytes(Buffer, 1);
  if((Buffer[0] - '0') == 0){
    ammoStatus = 0;
  }
  else if((Buffer[0] - '0') == 1){
    ammoStatus = 1;
  }
}

void ammoBlink(){
  for(size_t i = 0; i < 100; i++){
    driveRGB(0, 0, i);
    delay(1);
  }
  for(size_t i = 100; i > 0; i--){
    driveRGB(0, 0, i);
    delay(1);
  }
  driveRGB(hRed, hGreen, hBlue);
}

void driveAmmo(){
  if(ammoStatus == 1){
    if(ammoBegin == 1){
      ammoBlink();
      ammoBlinkLength = millis();
      ammoBegin = 0;
    }
    if((millis() - ammoBlinkLength) > 1000){
      ammoBlink();
      ammoBlinkLength = millis();
    }
  }
  if(ammoStatus == 0){
    ammoBegin = 1;
  }
}

void driveRGB(uint8_t red, uint8_t green, uint8_t blue){
  red = map(red, 0, 100, 0, ledBrightness);
  green = map(green, 0, 100, 0, ledBrightness);
  blue = map(blue, 0, 100, 0, ledBrightness);
  for(size_t i = 0; i < pixels; i++){
    neoPixel.setPixelColor(i, red, green, blue);
  }
  neoPixel.show();
}

void setup() {
  neoPixel.begin();
  neoPixel.show();
  Serial.begin(115200);
  Serial.setTimeout(10);
}

void loop() {
  readSerialData();
  driveHealth();
  driveBomb();
  driveAmmo();
}
