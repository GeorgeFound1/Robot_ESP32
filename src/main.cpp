#include <Arduino.h>

// Класс для координат
class Coords {
  int x;
  int y;
};


// Левый мотор
const int AIN1 = 16;
const int AIN2 = 17;
const int wirePWMA = 25;
const int encoderLeftA = 32;
const int encoderLeftB = 33;

// Правый мотор
const int BIN1 = 18;
const int BIN2 = 19;
const int wirePWMB = 26;
const int encoderRightA = 34;
const int encoderRightB = 35;


const int pwmChannelLeft = 0;  // Виртуальный канал 0
const int pwmChannelRight = 1; // Виртуальный канал 1
const int pwmFreq = 5000;      // 5 кГц
const int pwmResolution = 8;   // 8 бит (0-255)


const double translate = 51.42; // перевод тиков в сантиметры 
const double baseLenght = 16.4; // расстояние между серединами колес
volatile long double leftTicks = 0;
volatile long double rightTicks = 0;

void IRAM_ATTR readLeftEncoder() {
  // Если сигналы на А и B несовпадают, значит крутимся в одну сторону, если разные — в другую
  if (digitalRead(encoderLeftA) != digitalRead(encoderLeftB)) {
    leftTicks++;
  } else {
    leftTicks--;
  }
}

void IRAM_ATTR readRightEncoder() {
  if (digitalRead(encoderRightA) == digitalRead(encoderRightB)) {
    rightTicks++;
  } else {
    rightTicks--;
  }
}


void setMotors(int leftSpeed, int rightSpeed) {
  // Левый мотор
  if (leftSpeed >= 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    ledcWrite(pwmChannelLeft, leftSpeed);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    ledcWrite(pwmChannelLeft, -leftSpeed);
  }

  // Правый мотор
  if (rightSpeed >= 0) {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    ledcWrite(pwmChannelRight, rightSpeed);
  } else {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    ledcWrite(pwmChannelRight, -rightSpeed);
  }
}

void goStraight(const int distance) {
  leftTicks = 0;
  rightTicks = 0;
  
  long targetTicks = distance * translate; 

  while (leftTicks < targetTicks && rightTicks < targetTicks) {
    setMotors(150, 150);
  }
  setMotors(0, 0);
}

void letTurn(const int angle) {
  leftTicks = 0;
  rightTicks = 0;

  double targetDistance = (abs(angle) / 360.0) * (3.1415 * baseLenght);
  long targetTicks = targetDistance * translate;

  if (angle > 0) {
    while (leftTicks < targetTicks && rightTicks > -targetTicks) {
      setMotors(150, -150);
    }
  } else {
    while (leftTicks > -targetTicks && rightTicks < targetTicks) {
      setMotors(-150, 150);
    }
  }
  setMotors(0, 0);
}


void setup() {
  Serial.begin(115200);
  
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  ledcSetup(pwmChannelLeft, pwmFreq, pwmResolution);
  ledcSetup(pwmChannelRight, pwmFreq, pwmResolution);
  
  ledcAttachPin(wirePWMA, pwmChannelLeft);
  ledcAttachPin(wirePWMB, pwmChannelRight);

  pinMode(encoderLeftA, INPUT_PULLUP);
  pinMode(encoderLeftB, INPUT_PULLUP);
  pinMode(encoderRightA, INPUT_PULLUP);
  pinMode(encoderRightB, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderLeftA), readLeftEncoder, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderRightA), readRightEncoder, RISING);

  Serial.println("--- ТЕСТ ЖЕЛЕЗА ИСПРАВЛЕН ---");
  delay(1000);
}


void loop() {
  const int sideLength = 70; 
  const int turnAngle = 90; 

  Serial.println("=== НАЧАЛО ДВИЖЕНИЯ ПО КВАДРАТУ ===");

  for (int i = 0; i < 4; i++) {
    Serial.printf("Сторона %d: Едем вперед на %d см\n", i + 1, sideLength);
    goStraight(sideLength);
    
    delay(500); 

    Serial.printf("Поворот %d: Разворот на %d градусов\n", i + 1, turnAngle);
    letTurn(turnAngle);
    
    delay(500); 
  }

  Serial.println("=== КВАДРАТ ЗАВЕРШЕН ===");
  Serial.println("Ожидание 10 секунд перед следующим кругом...");
  
  delay(10000); 
}