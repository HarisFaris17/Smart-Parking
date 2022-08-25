#include <lorawan.h>
#include <sstream>
//#include <lmic.h>
#include <DS3231.h>
//#include <TimeLib.h>
#include <SoftwareSerial.h>
#include<List.hpp>
//#include<Time.h>
#include "convertChar.h"
#include "classParkir.h"

String readKodeBooking = "";
String as, ba, dataMasuk;

int indexPlusOne = 0;

//ganti menjadi pin yang berhubungan dengan arduino
//SoftwareSerial arduinoSerial(26, 28); //rx tx

RTClib myRTC;

//ganti menjadi dev addrs KeluarParkir
const char *devAddr = "eed04193";
const char *nwkSKey = "ccb392823def4d6d0000000000000000";
const char *appSKey = "00000000000000006f06abccd5a3cc52";

char data[25];
char myStr[50];
char dataMasukChar[25];
byte outStr[255];//byte
byte recvStatus = 0;
int port, channel, freq;
const unsigned long intervalPing = 10000;
const unsigned long interval = 1000;    // 10 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter
String dataSend = "";
unsigned long previousMillisParkir = 0;

const sRFM_pins RFM_pins = {
  .CS = 2,
  .RST = 32,
  .DIO0 = 14,
  .DIO1 = 12,
};

Parkir parkir;

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  randomSeed(10);
  delay(2000);
  if (!lora.init()) {
    Serial.println("RFM95 not detected");
    delay(5000);
    return;
  }

  lora.setDeviceClass(CLASS_A);
  lora.setDataRate(SF10BW125);
  lora.setFramePortTx(5);

  // set channel to random
  lora.setChannel(MULTI);
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
  Wire.begin();
}

void pingAntares() {
  if (millis() - previousMillis > intervalPing) {
    previousMillis = millis();

    sprintf(myStr, "ping", counter++);//A112342022072006420199

    Serial.println(myStr);
    lora.sendUplink(myStr, strlen(myStr), 0);

    /*  byte myByte[] = {0xFF};
      Serial.print("Sending: ");
      for (int i = 0; i < sizeof(myByte); i++)
      {
        Serial.print(myByte[i]); Serial.print(" ");
      }
      Serial.println();
      lora.sendUplinkHex(myByte, sizeof(myByte), 0);*/

    /*   port = lora.getFramePortTx();
       channel = lora.getChannel();
       freq = lora.getChannelFreq(channel);
       Serial.print(F("fport: "));    Serial.print(port); Serial.print(" ");
       Serial.print(F("Ch: "));    Serial.print(channel); Serial.print(" ");
       Serial.print(F("Freq: "));    Serial.print(freq); Serial.println(" ");
      }*/
    /* // Serial.print(parkirMasuk, BIN);
      Serial.println(parkirMasuk, BIN);
      //  String a = parkirMasuk, BIN;
      dataSend = parkirMasuk;
      dataSend.toCharArray(myStr, 50);
      //Serial.print("Sending: ");
      //Serial.println(dataSend);
      lora.sendUplink(myStr, strlen(myStr), 0);
      port = lora.getFramePortTx();
      channel = lora.getChannel();
      freq = lora.getChannelFreq(channel); */
  }
}
void loop() {
  if (Serial2.available() > 0) {
    //tanda bahwa ada pengendara yang memasukkan kode booking
    //untuk menghilangkan karakter penanda kode booking masuk
    char charRead = Serial2.read();
    Serial.println(charRead);
    if (charRead == '1') {
      readKodeBooking = "";
      //siap menerima kode booking
      Serial2.write('1');
      while (Serial2.available() < 4) {};
      for (int i = 0; i < 4; i++) {
        readKodeBooking.concat((char)Serial2.read());
      }
      Serial.println("kode booking : " + readKodeBooking);
      indexPlusOne = parkir.indexPlusOneListFromKodeBooking(readKodeBooking);
      Serial.println("searching kode booking sudah selesai");
      if (indexPlusOne == 0) {
        Serial.println("kode booking salah");
        Serial2.write('0'); //menandakan bahwa kode booking salah
      }
      else {
        //sepertinya tidak dibutuhkan handshake
        //memberitahu bahwa kode booking benar
        Serial.println("kode booking benar");
        Serial2.write('1');
        //mengecek apakah arduino siap mengirim
        while (Serial2.available() == 0) {}
        //dihapus dummy charread
        //jika siap
        Serial2.read();
        //kirim harga
        Serial2.print(parkir.harga(myRTC.now(), indexPlusOne - 1));
        Serial.println("Harga tercantum");
      }
    }//pengendara telah membayar
    else if (charRead == '2') {
      afterBayar(indexPlusOne - 1);
    }
  }
  
  lora.update();
  char outchar[255] = {};
  recvStatus = lora.readDataByte(outStr);
  String cstring ;//= String (outStr);
 // String as, ba, wa;
  if (recvStatus) {
    int counter = 0;
    port = lora.getFramePortRx();
    channel = lora.getChannelRx();
    freq = lora.getChannelRxFreq(channel);

    auto outstr = std::ostringstream{};
    outstr << std::hex << 0xC697C63Bul;
    auto myHexString = outstr.str();

    for (int i = 0; i < recvStatus; i++)
    {
      if (((outStr[i] >= 32) && (outStr[i] <= 126)) || (outStr[i] == 10) || (outStr[i] == 13))
        counter++;
    }
    if (port != 0)
    {
      if (counter == recvStatus)
      {
        Serial.print(F("Received String : "));
           for (int i = 0; i < recvStatus; i++)
        {
          Serial.print(char(outStr[i]));
        }
//        int i = 0;
//        for (; i < recvStatus; i++)
//        {
//          Serial.print(char(outStr[i]));
//          data[i] = char(outStr[i]);
//        }
//        data[i] = '\0';
//        saveDataToParkir();
      }
      else
      {
        Serial.print(F("Received Hex: "));
        as = "";
        for (int i = 0; i < recvStatus; i++)
        {
          Serial.print(outStr[i], HEX); Serial.print(" ");
          ba = String(outStr[i], HEX);
          char sda[20];
          sprintf(sda, "%02s", ba);
          as += sda;
          dataMasuk = as;
          dataMasuk.toUpperCase();
          dataMasuk.trim(); //dataMasuk berupa string
        }
        dataMasuk.toCharArray(dataMasukChar, dataMasuk.length() + 1);
        Serial.print(" dataMasukChar : ");
        Serial.println(dataMasukChar);
        int a = 0;
        for (; a < dataMasuk.length() + 1; a++) {
          //Serial.print(dataMasukChar[a]);
          data[a] = dataMasukChar[a];
        }
        data[a] = '\0';
        saveDataToParkir();
      }
    }
    else
    {
      Serial.print(F("Received Mac Cmd : "));
      for (int i = 0; i < recvStatus; i++)
      {
        Serial.print(outStr[i], HEX); Serial.print(" ");
      }
    }
    Serial.println();
    Serial.print(F("fport: "));    Serial.print(port); Serial.print(" ");
    Serial.print(F("Ch: "));    Serial.print(channel); Serial.print(" ");
    Serial.print(F("Freq: "));    Serial.println(freq); Serial.println(" ");
//    Serial.print(wa);
  }
  if (millis() - previousMillisParkir > 3000) {
    Serial.println("cetak isi parkir:");
    parkir.cetakIsiParkir();
    previousMillisParkir = millis();
    Serial.println(waktuSekarangRTC());
  }
  pingAntares();
}

//hapus dari list parkirnya
//kirim ke KeluarParkir
void afterBayar(int index) {
  parkir.getParkir(index, data[0]);
  kirimDataAntares();
  removeParkir(index);
}

void removeParkir(int index) {
  parkir.removeParkir(index);
}

void saveDataToParkir() {
  String kodePark = String(data[0]) + String(data[1]);
  String kodeBook = String(data[2]) + String(data[3]) + String(data[4]) + String(data[5]);
  uint16_t tahun = string_to_ribuan(&(data[6]));
  uint8_t bulan = string_to_puluhan(&(data[10]));
  uint8_t hari = string_to_puluhan(&(data[12]));
  uint8_t jam = string_to_puluhan(&(data[14]));
  uint8_t menit = string_to_puluhan(&(data[16]));
  uint8_t detik = string_to_puluhan(&(data[18]));
  DateTime waktuBook(tahun, bulan, hari, jam, menit, detik);
  int lamaBook = (int)string_to_puluhan(&(data[20]));
  parkir.tambahParkir(kodePark, kodeBook, waktuBook, lamaBook);
}

void kirimDataAntares() {
  if (millis() - previousMillis > interval) {
    previousMillis = millis();
    Serial.print("Sending: ");
    Serial.println(data);
    lora.sendUplink(data, strlen(data), 0);
    port = lora.getFramePortTx();
    channel = lora.getChannel();
    freq = lora.getChannelFreq(channel);
    Serial.print(F("fport: "));    Serial.print(port); Serial.print(" ");
    Serial.print(F("Ch: "));    Serial.print(channel); Serial.print(" ");
    Serial.print(F("Freq: "));    Serial.print(freq); Serial.println(" ");
  }
}

String waktuSekarangRTC() {
  String waktuSekarang = "";
  waktuSekarang += String(myRTC.now().year());
  waktuSekarang += " ";
  waktuSekarang += String(myRTC.now().month());
  waktuSekarang += " ";
  waktuSekarang += String(myRTC.now().day());
  waktuSekarang += " ";
  waktuSekarang += String(myRTC.now().hour());
  waktuSekarang += " ";
  waktuSekarang += String(myRTC.now().minute());
  waktuSekarang += " ";
  waktuSekarang += String(myRTC.now().second());
  return waktuSekarang;
}
