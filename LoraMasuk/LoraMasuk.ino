#include <lorawan.h>
#include <sstream>
//#include <lmic.h>
#include <DS3231.h>
#include <TimeLib.h>
#include <SoftwareSerial.h>
//#include<Time.h>
#include "convertChar.h"
#include "classParkir.h"
#include"Wire.h"

String readKodeBooking = "";
String as, ba, dataMasuk;


//SoftwareSerial arduinoSerial(26, 28); //rx tx

RTClib myRTC;

const char *devAddr = "c44ed0ca";
const char *nwkSKey = "ccb392823def4d6d0000000000000000";
const char *appSKey = "00000000000000006f06abccd5a3cc52";

char data[25];
char dataMasukChar[25];
byte outStr[255];//byte
byte recvStatus = 0;
int port, channel, freq;

const unsigned long interval = 1000;    // 10 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter
String dataSend = "";

const sRFM_pins RFM_pins = {
  .CS = 2,
  .RST = 32,
  .DIO0 = 14,
  .DIO1 = 12,
};

Parkir parkir;
unsigned long prevMillis = 0;
void setup() {
  Serial.begin(115200);
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
  lora.setTxPower(15);
  // set channel to random
  lora.setChannel(MULTI);
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
  Wire.begin();
}

void loop() {
  if (millis() - prevMillis > 10000) {
    byte myByte[] = {0xFF, 0xA2, 0x33, 0xD4, 0x55};
    prevMillis = millis();
    Serial.println("Sending LoRa : ");
    lora.sendUplinkHex(myByte, sizeof(myByte), 0);
  }
  if (millis() - previousMillis > interval) {
    previousMillis = millis();
    parkir.cetakIsiParkir();

    //    Serial.println("Sending: ");
    Serial.println(waktuSekarang());

    //    for (int i = 0; i < sizeof(myByte); i++)
    //    {
    //      Serial.print(myByte[i]); Serial.print(" ");
    //    }
    //    Serial.println();

    port = lora.getFramePortTx();
    channel = lora.getChannel();
    freq = lora.getChannelFreq(channel);
    //    Serial.print(F("fport: "));    Serial.print(port); Serial.print(" ");

    //    Serial.print(F("Ch: "));    Serial.print(channel); Serial.print(" ");
    //    Serial.print(F("Freq: "));    Serial.print(freq); Serial.println(" ");
  }
  if (Serial2.available() > 0) {
    char charRead = Serial2.read();
    //    Serial.println(charRead);
    //apakah pengendara masuk offline
    if (charRead == '1') {
      if (parkir.checkAndTambahParkirOffline(myRTC.now(), data[0])) {
        //parkir tidak penuh
        //parkir.checkAndTambahParkirOffline(myRTC.now(), data
        Serial2.write('1');
        Serial2.write(data);
        Serial.print("parkir tidak penuh");
        Serial.print(data);
        Serial.print(sizeof(data));//"A1123420210517170511");//data
        //menandakan offline
        data[20] = '9';
        data[21] = '9';
        kirimDataAntares();
      }
      else {
        //parkir penuh
        Serial2.write('0');
        Serial.println("parkir penuh");
      }
    }
    //pengendara jika masuk online
    else if (charRead == '0') {
      readKodeBooking = "";
      //beritahu arduino bahwa data booking siap diterima
      //sebenarnya komunikasi handshake khusus disini tidak diperlukan
      Serial2.write('1');
      while (Serial2.available() < 4) {};
      for (int i = 0; i < 4; i++) {
        readKodeBooking.concat((char)Serial2.read());
      }
      Serial.println("kode booking : " + readKodeBooking);
      int indexPlusOne = parkir.indexPlusOneListFromKodeBooking(readKodeBooking);
      Serial.print("indexplusone :");
      Serial.println(indexPlusOne);
      if (indexPlusOne == 0) {
        Serial2.write('0'); //menandakan bahwa kode booking salah
      }
      else {
        //cek jika memang parkir tersebut dalam range waktu sekarang
        if (parkir.checkParkirIfInRangeWaktuBook(indexPlusOne - 1, myRTC.now())) {
          Serial2.write('1'); //beritahu kepada arduino bahwa kode booking benar dan akan siap mengirim kodeparkirkodebookingwaktubooking
          //kenapa indexplusone-1 karena kita memerlukan 0 untuk memberitahu bahwa kodebooking salah
          parkir.getParkir(indexPlusOne - 1, data[0]);
          Serial2.write(data);
          kirimDataAntares();
        }
        //kode parkir benar namun parkir belum dalam range sekarang
        else {
          Serial2.write('2');
        }
      }
    }
  }
  lora.update();
  char outchar[255] = {};
  String stringout;
  String sss;
  recvStatus = lora.readDataByte(outStr);
  String cstring ;//= String (outStr);
  //String as, ba, wa;
  if (recvStatus) {
    Serial.println("Data diterima");
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
    //Serial.print(F("fport: "));    Serial.print(port); Serial.print(" ");
    //Serial.print(F("Ch: "));    Serial.print(channel); Serial.print(" ");
    //Serial.print(F("Freq: "));    Serial.println(freq); Serial.println(" ");
    //Serial.print(wa);
  }

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
  //  if (millis() - previousMillis > interval) {
  //    previousMillis = millis();
  Serial.print("Sending data parkir: ");
  Serial.println(data);
  lora.sendUplink(data, strlen(data), 0);
  port = lora.getFramePortTx();
  channel = lora.getChannel();
  freq = lora.getChannelFreq(channel);
  Serial.print(F("fport: "));    Serial.print(port); Serial.print(" ");
  Serial.print(F("Ch: "));    Serial.print(channel); Serial.print(" ");
  Serial.print(F("Freq: "));    Serial.print(freq); Serial.println(" ");
  //  }
}

String waktuSekarang() {
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
