#define RPWM 12
#define LPWM 13
#define PWM 11
#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"
#include <Arduino.h>
#include <RTClib.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define I2CADDR 0x20

//SoftwareSerial mySerial(26, 28); //rx tx
//Adafruit_Thermal printer(&mySerial);
/////////////////////////////////////////////////////////////
//ubah rx tx
SoftwareSerial antaresSerial(3, 4);

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

LiquidCrystal_I2C lcd(0x27, 16, 2); //

// Digitran keypad, bit numbers of PCF8574 i/o port
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad

TwoWire *jwire = &Wire;   //test passing pointer to keypad lib
Keypad_I2C kpd( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR, PCF8574, jwire );
//Keypad_I2C kpd( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR );
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

char data[6];

/////////////////////////////////////////////////////
//ubah untuk infrared deteksi mobil masul
const int irDeteksiMobilKeluar = 8;

boolean buttonState = 0;
boolean lastButtonState = 0;
boolean irDeteksiMobilKeluarState = 0;
boolean lastIrDeteksiMobilKeluarState = 0;
boolean isMobilParkirKeluar = 0;
boolean isMotorMoving = 0;
boolean isWaitingBayar = 0;
String input_password;
unsigned long previousMillisPrinter = 0UL;
unsigned long previousMillisDeteksiMobilKeluar = 0UL;
unsigned long previousMillisMotor = 0UL;
unsigned long previousMillisBayar = 0UL;
/////////////////////////////////////////////////////
//ubah untuk adjust waktu motor dari 0 sampai 90 derajat atau dari 90 derajat ke 0 derajat
unsigned long waktuMotor = 3000UL;
unsigned long waktuDeteksiMobil = 30000UL;
unsigned long waktuPrinter = 3000UL;
unsigned long waktuBayar = 10000UL;

void setup() {
  Serial.begin(9600);
  antaresSerial.begin(9600);
  jwire->begin( );
   kpd.begin( );
  Serial.print( "start with pinState = " );
  Serial.println( kpd.pinState_set( ), HEX );
  Serial.println("START");
  pinMode (RPWM, OUTPUT);
  pinMode (PWM, OUTPUT);
  pinMode (LPWM, OUTPUT);
  pinMode (irDeteksiMobilKeluar, INPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello, world!");
}

void loop() {
  checker();
  checkBayar();
  stopMotorAfterMoving();
  tutupPalangKetikaMobilKeluar();
}


void checker() {
  char key = kpd.getKey();
  if (key) {
    Serial.print("Key Pressed : ");
    Serial.println(key);
    if (key == '*') {
      input_password = "";
    }
    else if (key == '#') {
      komunikasiESP();
    }
    else {
      input_password += key;
      Serial.println(input_password);
    }
  }
}

void komunikasiESP() {
  Serial.println("check kode booking...");
  antaresSerial.print('1');
//  while (antaresSerial.available() == 0) {}
//  antaresSerial.read();
//  Serial.println(input_password);
  Serial.println("megirim input_password");
  while (antaresSerial.available() > 0 || Serial.available() > 0){
    antaresSerial.read(); 
    Serial.read();
   }
  antaresSerial.print(input_password);

  while (antaresSerial.available() == 0) {}
  char charRead = (char)antaresSerial.read();
  if (charRead == '0') {
    Serial.println("kode booking invalid");
//    lcd.print("kode booking invalid");
  }
  else if (charRead == '1') {
    Serial.println("kode booking valid");
    antaresSerial.print('1');
//    lcd.print("kode booking valid");
    while (antaresSerial.available() < 5) {}
    int i = 0;
    for (; i < 5; i++) {
      data[i] = (char)antaresSerial.read();
    }
    data[i] = '\0';
    Serial.print("Data yang diterima adalah ");
    Serial.println(data);
    if (data == "00000") {
      sudahMembayar();
    } else {

      isWaitingBayar = 1;
      previousMillisBayar = millis();
    }
  }
  input_password = "";
}

void checkBayar() {
  if (isWaitingBayar == 1) {
    //saat ini masih memakai metode waktu, jika memakai metode ini pengguna mungkin tidak bayar, jika sewaktu waktu di masa yang akan datang ignin menggunakan mesin uang lebih baik
    if ((millis() - previousMillisBayar) > waktuBayar) {
      isWaitingBayar = 0;
      sudahMembayar();
      //beritahu esp bahwa sudah membayar
      antaresSerial.print('2');
    }
  }
}

void sudahMembayar() {
  isMobilParkirKeluar = true;
  previousMillisDeteksiMobilKeluar = millis();
  motorCW();
}

void tutupPalangKetikaMobilKeluar() {
  //adakan logika ketika palang hanya menutup ketika mobil telah melewati IR pendeteksi mobil
  if (isMobilParkirKeluar) {
    irDeteksiMobilKeluarState = digitalRead(irDeteksiMobilKeluar);
    if (irDeteksiMobilKeluarState != lastIrDeteksiMobilKeluarState) {
      if (irDeteksiMobilKeluarState == HIGH) {
        Serial.println("Palang menutup karena mobil keluar");
        motorCCW();
        isMobilParkirKeluar = false;
      }
    } else if ((millis() - previousMillisDeteksiMobilKeluar) > waktuDeteksiMobil) {
      Serial.println("Palang menutup karena tidak ada mobil yang masuk selama 30 detik");
      motorCCW();
      isMobilParkirKeluar = false;

    }
    lastIrDeteksiMobilKeluarState = irDeteksiMobilKeluarState;
  }
}

void stopMotorAfterMoving() {
  if ((millis() - previousMillisMotor > waktuMotor) && isMotorMoving) {
    motorStop();
  }
}

void motorCW() {             //fungsi motor membuka palang
  digitalWrite (LPWM, LOW);
  digitalWrite (RPWM, HIGH);
  analogWrite (PWM, 500);
  Serial.println ("MUTER KANAN");
  previousMillisMotor = millis();
  isMotorMoving = true;
}

void motorCCW() {             //fungsi motor menutup
  digitalWrite (LPWM, HIGH);
  digitalWrite (RPWM, LOW);
  analogWrite (PWM, 500);
  Serial.println ("MUTER KIRI");
  previousMillisMotor = millis();
  isMotorMoving = true;
}

void motorStop() {                 //motor stop
  digitalWrite (LPWM, LOW);
  digitalWrite (RPWM, LOW);
  analogWrite (PWM, 0);
  Serial.println ("MOTOR STOP");
  isMotorMoving = false;
}
