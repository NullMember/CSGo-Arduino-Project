char Buffer[3];
uint8_t health, bombStatus, bombBegin, ammoBegin, ammoStatus, ledLOW, ledHIGH;
uint16_t bombBlinkLength;
unsigned long bombPreviousLength, ammoBlinkLength;
const uint8_t bPin = 9;
const uint8_t gPin = 10;
const uint8_t rPin = 11;
uint8_t hRed, hGreen, hBlue;

char commonAC = 'a'; //if RGB led is common anode 'a' else 'c'

void setCommonAC(){
  if(commonAC == 'a'){
    ledLOW = 255;
    ledHIGH = 0;
  }
  else if(commonAC == 'c'){
    ledLOW = 0;
    ledHIGH = 255;
  }
}

void readSerialData(){
  Buffer[0] = NULL;
  Buffer[0] = Serial.read(); 
  
  if(Buffer[0] == 'b'){
    readBombData();
  }
  else if(Buffer[0] == 'h'){
    readHealthData();
  }
  else if(Buffer[0] == 'a'){
    readAmmoData();
  }
}

uint8_t chartoInt(){
  Buffer[0] = NULL; Buffer[1] = NULL; Buffer[2] = NULL;
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
  Buffer[0] = Serial.read();
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
  Buffer[0] = Serial.read();
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
  red = map(red, 0, 100, ledLOW, ledHIGH);
  green = map(green, 0, 100, ledLOW, ledHIGH);
  blue = map(blue, 0, 100, ledLOW, ledHIGH);
  analogWrite(rPin, red);
  analogWrite(gPin, green);
  analogWrite(bPin, blue);
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10);
  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
}

void loop() {
  readSerialData();
  driveHealth();
  driveBomb();
  driveAmmo();
}
