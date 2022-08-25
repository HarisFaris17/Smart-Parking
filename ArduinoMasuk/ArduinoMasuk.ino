#define RPWM 12
#define LPWM 13
#define PWM 11
#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"
#include <Keypad.h>
#include "Arduino.h"
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display


SoftwareSerial mySerial(26, 28); //rx tx
Adafruit_Thermal printer(&mySerial);
/////////////////////////////////////////////////////////////
//ubah rx tx
//SoftwareSerial Serial(3,4);

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {44, 42, 40, 38}; //{9, 8, 7, 6};
byte colPins[COLS] = {36, 34, 32, 30}; //{5, 4, 3, 2};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

char data[25];

const int  buttonParkirOffline = 24;
/////////////////////////////////////////////////////
//ubah untuk infrared deteksi mobil masul
const int irDeteksiMobilMasuk = 50;

boolean buttonState = HIGH;
boolean lastButtonState = HIGH;
boolean irDeteksiMobilMasukState = 0;
boolean lastIrDeteksiMobilMasukState = 0;
boolean isMobilParkirMasuk = 0;
boolean isMotorMoving = 0;
boolean isAfterPrinting = 0;
String input_password;
unsigned long previousMillisPrinter = 0UL;
unsigned long previousMillisDeteksiMobilMasuk = 0UL;
unsigned long previousMillisMotor = 0UL;
/////////////////////////////////////////////////////
//ubah untuk adjust waktu motor dari 0 sampai 90 derajat atau dari 90 derajat ke 0 derajat
unsigned long waktuMotor = 3000UL;
unsigned long waktuDeteksiMobil = 30000UL;
unsigned long waktuPrinter = 3000UL;
unsigned long haha = 10000UL;
unsigned long previous = 0;
void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);
  mySerial.begin(9600);
  printer.begin();
  Serial.println("START");
  pinMode (RPWM, OUTPUT);
  pinMode (PWM, OUTPUT);
  pinMode (LPWM, OUTPUT);
  pinMode (buttonParkirOffline, INPUT_PULLUP);
  pinMode (irDeteksiMobilMasuk, INPUT);
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello, world!");
}

void loop() {
  parkirOffline();
  parkirOnline();
  wakePrinter();
  stopMotorAfterMoving();
  tutupPalangKetikaMobilMasuk();
}

void parkirOffline() {

  buttonState = digitalRead(buttonParkirOffline);
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      lcd.setCursor(0, 0);
      lcd.print("Parkir offline!");
      komunikasiESPParkirOffline();
    }
    delay(50);
  }
  lastButtonState = buttonState;
}

void parkirOnline() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key Pressed : ");
    Serial.println(key);
    if (key == '*') {
      input_password = "";
    }
    else if (key == '#') {
      komunikasiESPParkirOnline();
    }
    else {
      input_password += key;
    }
  }
}

void komunikasiESPParkirOffline() {
  Serial.write("request parkir offline");
  while (Serial3.available() > 0 || Serial.available() > 0) {
    Serial3.read();
    Serial.read();
  };
  Serial3.write('1');


  while (Serial3.available() == 0) {};
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("request");
  Serial.println("request diterima");
  delay(100);
  char charRead = Serial3.read();
  lcd.setCursor(8, 0);
  lcd.print(charRead, HEX);
  //jika parkir tersedia
  if (charRead == '1') {
    lcd.setCursor(0, 1);
    lcd.print("data");
    while (Serial3.available() < 20) {}
    int i = 0;
    for (; i < 20; i++) {
      data[i] = (char)Serial3.read();
    }
    data[i] = '\0';
    lcd.print(data);
    Serial.println(data);
    printKarcisOffline();
    parkirBenar();
  }
  else if (charRead == '0') {
    lcd.setCursor(0, 1);
    lcd.print("parjir tidak tersedia");
    Serial.println("Parkir tidak ada yang kosong");
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print("data tidak masuk");
    Serial.println("data tidak masuk");

  }
  Serial.println("request berhasil");
}

void komunikasiESPParkirOnline() {
  Serial.println("request parkir online");
  while (Serial3.available() > 0 || Serial.available() > 0) {
    Serial3.read();
    Serial.read();
  };
  Serial3.print('0');
  while (Serial3.available() == 0) {}
  Serial3.read();
  Serial3.print(input_password);
  Serial.print(input_password);
  while (Serial3.available() == 0) {}
  char charRead = (char)Serial3.read();
  if (charRead == '0') {
    Serial.println("kode booking invalid");
  }
  else if (charRead == '1') {
    Serial.println("kode booking benar");
    while (Serial3.available() < 22) {}
    int i = 0;
    for (; i < 22; i++) {
      data[i] = (char)Serial3.read();
    }
    data[i] = '\0';
    Serial.print("Data yang diterima adalah ");
    Serial.println(data);
    printKarcisOnline();
    parkirBenar();


  }
  else if (charRead == '2') {
    Serial.println("parkir yang dipesan belum waktunya");
  }
  input_password = "";
}

void parkirBenar() {
  isMobilParkirMasuk = true;
  previousMillisDeteksiMobilMasuk = millis();
  motorCW();
}

void printKarcisOffline() {
  Serial.println("Printing......");
  printer.begin();
  printer.justify('L');
  printer.setSize('M');
  printer.println("Sparkindo");
  printer.println("=================================");
  printer.setLineHeight(50);
  printer.setSize('L');
  printer.justify('L');
  printer.print("Parkir");
  printer.setLineHeight(30);
  printer.justify('R');
  printer.println(kodePark());
  printer.justify('L');
  printer.print("Kode Booking");
  printer.justify('R');
  printer.println(kodeBook());
  printer.justify('L');
  printer.print("Waktu Booking");
  printer.justify('R');
  printer.println(waktuBook());
  printer.justify('L');
  printer.setSize('M');
  printer.setLineHeight(50);
  printer.println("=================================");
  printer.setSize('S');
  printer.justify('R');
  printer.println("*Note : parkir hanya berlaku 3 jam");
  printer.sleep();
  isAfterPrinting = true;
  previousMillisPrinter = millis();
}

void printKarcisOnline() {
  Serial.println("Printing......");
  printer.begin();
  printer.justify('L');
  printer.setSize('M');
  printer.println("Sparkindo");
  printer.println("=================================");
  printer.setLineHeight(50);
  printer.setSize('L');
  printer.justify('L');
  printer.print("Parkir");
  printer.setLineHeight(30);
  printer.justify('R');
  printer.println(kodePark());
  printer.justify('L');
  printer.print("Kode Booking");
  printer.justify('R');
  printer.println(kodeBook());
  printer.justify('L');
  printer.print("Waktu Booking");
  printer.justify('R');
  printer.println(waktuBook());
  printer.justify('L');
  printer.print("Waktu Keluar");
  printer.justify('R');
  printer.println(waktuKeluar());
  printer.justify('L');
  printer.setSize('M');
  printer.println("=================================");
  printer.sleep();
  isAfterPrinting = true;
  previousMillisPrinter = millis();
}

void wakePrinter() {
  if (millis() - previousMillisPrinter > waktuPrinter && isAfterPrinting) {
    printer.wake();
    printer.setDefault();
    isAfterPrinting = false;
  }
}

String kodePark() {
  return String(data[0]) + String(data[1]);
}

String kodeBook() {
  return String(data[2]) + String(data[3]) + String(data[4]) + String(data[5]);
}

String waktuBook() {
  return String(data[12]) + String(data[13]) + "/" + String(data[10]) + String(data[11]) + "/" + String(data[6]) + String(data[7]) + String(data[8]) + String(data[9]) + " "
         + String(data[14]) + String(data[15]) + ":" + String(data[16]) + String(data[17]) + ":" + String(data[18]) + String(data[19]);
}

String waktuKeluar() {
  String waktuKeluarParkir;
  char waktuKeluarChar[15] = "";
  uint16_t tahun = string_to_ribuan(&(data[6]));
  uint8_t bulan = string_to_puluhan(&(data[10]));
  uint8_t hari = string_to_puluhan(&(data[12]));
  uint8_t jam = string_to_puluhan(&(data[14]));
  uint8_t menit = string_to_puluhan(&(data[16]));
  uint8_t detik = string_to_puluhan(&(data[18]));
  DateTime waktuBook(tahun, bulan, hari, jam, menit, detik);
  String waktubooking = "";
//  waktubooking = waktuBook.year();
  waktubooking += tahun;
  waktubooking += " ";
//  waktubooking += waktuBook.month();
  waktubooking +=bulan;
  waktubooking += " ";
//  waktubooking += waktuBook.day();
  waktubooking +=hari;
  waktubooking += " ";
//  waktubooking += waktuBook.hour();
  waktubooking +=jam;
  waktubooking += " ";
  waktubooking += menit;
  Serial.print("waktu book:");
  Serial.println(waktubooking);
  Serial.print("hhh:");
  uint8_t lamaBook = string_to_puluhan(&(data[20]));
  DateTime waktuKeluar(waktuBook + TimeSpan(0, lamaBook, 0, 0));
  sprintf(waktuKeluarChar, "%02d/%02d/%04d %02d:%02d:%02d", waktuKeluar.day(), waktuKeluar.month(), waktuKeluar.year(), waktuKeluar.hour(), waktuKeluar.minute(), waktuKeluar.second());
  Serial.print(waktuKeluarChar);
  return String(waktuKeluarChar);
}

void tutupPalangKetikaMobilMasuk() {
  //adakan logika ketika palang hanya menutup ketika mobil telah melewati IR pendeteksi mobil
  if (isMobilParkirMasuk) {
    irDeteksiMobilMasukState = digitalRead(irDeteksiMobilMasuk);
    if (irDeteksiMobilMasukState != lastIrDeteksiMobilMasukState) {
      if (irDeteksiMobilMasukState == HIGH) {
        Serial.println("Palang menutup karena mobil masuk");
        motorCCW();
        isMobilParkirMasuk = false;
      }
    } else if ((millis() - previousMillisDeteksiMobilMasuk) > waktuDeteksiMobil) {
      Serial.println("Palang menutup karena tidak ada mobil yang masuk selama 30 detik");
      motorCCW();
      isMobilParkirMasuk = false;

    }
    lastIrDeteksiMobilMasukState = irDeteksiMobilMasukState;
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
