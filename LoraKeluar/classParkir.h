uint32_t SECS_PER_HOUR = 3600;
uint32_t HARGA_OFFLINE_PER_HOUR = 4000;
uint32_t HARGA_ONLINE_PER_HOUR = 3000;

String listKodeParkir[] = {"A1", "A2", "A3", "A4", "B1", "B2", "B3", "B4"};
String characterBooking = "0123456789ABCD";
const int banyakParkir = 20;
class Parkir {
  private:
    String *kodeParkir[banyakParkir]={new String("A1"),new String("B1")};
    String *kodeBooking[banyakParkir]={new String("B123"),new String("A199")};
    DateTime *waktuBooking[banyakParkir]={new DateTime(2022, 7, 19, 15, 00, 00),new DateTime(2022, 7, 15, 14, 38, 00)};
    int *lamaBooking[banyakParkir]={new int(3),new int(3)};
  public:
    Parkir() {
//      kodeParkir[0] = new String("A1");//,new String("A2")};
//      kodeParkir[1]=new String("B5");
//      kodeBooking[0] = new String("B123");//, new String("B490")};
//      kodeBooking[1]=new String("A199");
//      waktuBooking[0] = new DateTime(2022, 7, 19, 15, 00, 00);//, new DateTime(2022, 7, 15, 14, 38, 00)};
//      waktuBooking[1]=new DateTime(2022, 7, 19, 15, 00, 00);
//      lamaBooking[0]=new int(3);
//      lamaBooking[0] =new int(3);//,new int(3)};
    }

    void getParkir(int index, char &destination) {
      parkirToString(*kodeParkir[index], *kodeBooking[index], *waktuBooking[index], *lamaBooking[index], destination);
    }

    void parkirToString(String kodeParkir, String kodeBook, DateTime waktuBook, int lamaBook, char &destination) {
      char waktuBookingChar[17] = "";
      sprintf(waktuBookingChar, "%04d%02d%02d%02d%02d%02d%02d", waktuBook.year(), waktuBook.month(), waktuBook.day(), waktuBook.hour(), waktuBook.minute(), waktuBook.second(), lamaBook);
      String kode = kodeParkir + kodeBook;
      for (int i = 0; i < 16; i++) {
        kode += waktuBookingChar[i];
      }
      //kenapa 23 ? karena data yang ingin dikirim adalah 22 karakter. dan tiap array karakter harus memiliki null-terminating \0 karakter di akhir arraynya oleh karena itu memerlukan 21
      kode.toCharArray(&destination, 23);
    }

//    boolean checkParkir(String kodePark) {
//      for (int i = 0; i < banyakParkir; i++) {
//        if (*kodeParkir[i] == kodePark ) {
//          return true;
//        }
//      }
//      return false;
//    }

    //check apakah waktu sekarang untuk parkir dengan index tertentu apakah bisa masuk
//    boolean checkParkirIfInRangeWaktuBook(int index, DateTime now) {
//      DateTime waktuBook = this->waktuBooking.getValue(index);
//      int lamaBook = this->lamaBooking.getValue(index);
//      // jika dia merupakan parkir offline, berarti harus dalam range 3 jam.
//      if (now.unixtime() >= waktuBook.unixtime() && (waktuBook.unixtime() + 3600U * (lamaBook == 99 ? 3 : lamaBook)) > now.unixtime()) {
//        return true;
//      }
//      return false;
//    }

    boolean checkKodeBooking(String kodeBook) {
      for (int i = 0; i < banyakParkir; i++) {
        if (kodeBook == *kodeBooking[i]) {
          return true;
        }
      }
      return false;
    }

    int indexPlusOneListFromKodeBooking(String kodeBook) {
      for (int i = 0; i < banyakParkir; i++) {
        if (kodeBooking[i] != NULL) {
          if (kodeBook == *kodeBooking[i]) return i + 1;
        }
      }
      return 0;
    }

    void tambahParkir(String kodePark, String kodeBook, DateTime waktuBook, int lamaBook) {
      for (int i = 0; i < banyakParkir; i++) {
        //pada index ini kodeParkir, kodeBooking, waktuBooking dan lamaBooking pointernya menunjuk NULL, sehingga bisa diisi data baru
        if (kodeParkir[i] == NULL) {
          kodeParkir[i] = new String(kodePark);
          kodeBooking[i] = new String(kodeBook);
          waktuBooking[i] = new DateTime(waktuBook);
          lamaBooking[i] = new int(lamaBook);
          break;
        }
      }
      Serial.println("Parkir ditambahkan");
    }

    void removeParkir(String kodeBook) {
      int i = 0;
      boolean ada = false;
      for (i = 0; i < banyakParkir; i++) {
        if (kodeBook == *kodeBooking[i]) {
          ada = true;
          break;
        }
      }
      if (ada) {
        kodeParkir[i] = NULL;
        kodeBooking[i] = NULL;
        waktuBooking[i] = NULL;
        lamaBooking[i] = NULL;
      }
      else {
        Serial.println("Kode booking " + kodeBook + " tidak tersedia");
      }
    }

    void removeParkir(int index) {
      kodeParkir[index]=NULL;
      kodeBooking[index]=NULL;
      waktuBooking[index]=NULL;
      lamaBooking[index]=NULL;
    }

//    boolean checkAndTambahParkirOffline(DateTime timeNow, char *destination) {
//      long mulai = random(0, 15);
//      for (; mulai < this->kodeParkir.getSize() + mulai; mulai++) {
//        //3 jam karena maksimal parkir offline adalah 3 jam
//        if (!checkParkir(listKodeParkir[mulai % 16], timeNow, 3)) {
//          String kodeBooking = kodeBookingGenerator();
//          parkirToString(listKodeParkir[mulai % 16], kodeBooking, timeNow, 99, destination);
//          //lamaBook adalah 99 untuk menandakan bahwa parkir tersebut dibooking secara offline
//          tambahParkir(listKodeParkir[mulai % 16], kodeBooking, timeNow, 99);
//          return true;
//        }
//      }
//      return false;
//    }
//
//    String kodeBookingGenerator() {
//      boolean ketemu = false;
//      String kodeBooking = "";
//      int pointer;
//      while (!ketemu) {
//        for (int i = 0; i < 4; i++) {
//          pointer = random(0, 13);
//          kodeBooking += characterBooking.charAt(pointer);
//        }
//        if (!checkKodeBooking(kodeBooking)) {
//          ketemu = true;
//          Serial.println("Kode Booking baru" + kodeBooking);
//        }
//      }
//      return kodeBooking;
//    }

    String harga(DateTime now, int index) {
      int lamaBook = *lamaBooking[index];
      uint32_t waktuBookUnixTime = waktuBooking[index]->unixtime();
      uint32_t nowUnixTime = now.unixtime();
      char harga[6] = "";
      //parkir offline
      if (lamaBook == 99) {
        Serial.println(int(ceil(float(nowUnixTime - waktuBookUnixTime) / SECS_PER_HOUR))*HARGA_OFFLINE_PER_HOUR);
        sprintf(harga, "%05d", int(ceil(float(nowUnixTime - waktuBookUnixTime) / SECS_PER_HOUR))*HARGA_OFFLINE_PER_HOUR);
        Serial.println(harga);
        return harga;
      } else {
        if ((waktuBookUnixTime - nowUnixTime) <= (lamaBook * SECS_PER_HOUR)) {
          //menandakan bahwa pelanggan tidak perlu bayar
          return "00000";
        }
        else {
          Serial.println(int(ceil(float(nowUnixTime - waktuBookUnixTime) / SECS_PER_HOUR - float(lamaBook)))*HARGA_OFFLINE_PER_HOUR);
          sprintf(harga, "%05d", int(ceil(float(nowUnixTime - waktuBookUnixTime) / SECS_PER_HOUR - float(lamaBook)))*HARGA_OFFLINE_PER_HOUR);
          Serial.println(harga);
          return harga;
        }
      }
    }
    
    void cetakIsiParkir() {
      String cetakan = "";
      //      Serial.println("Cetakan = " + cetakan);
      //      int i = 0;
      //      if (kodeParkir[i] != NULL) {
      //        Serial.println("Chhhh = " + cetakan);
      //        Serial.println((*kodeParkir[i]));
      //        Serial.println("Cetakan = " );
      //        cetakan += *kodeBooking[i];
      //        cetakan += " ";
      //        cetakan += String((*waktuBooking[i]).year());
      //        cetakan += " ";
      //        cetakan += String(*lamaBooking[i]);
      //        cetakan += "\n";
      //      }
      //      Serial.println("Cetaeekan = " + cetakan);
      for (int i = 0; i < banyakParkir; i++) {
        if (kodeParkir[i] != NULL) {
          cetakan += *kodeParkir[i];
          cetakan += " ";
          cetakan += *kodeBooking[i];
          cetakan += " ";
          cetakan += String((*waktuBooking[i]).year());
          cetakan += " ";
          cetakan += String((*waktuBooking[i]).month());
          cetakan += " ";
          cetakan += String((*waktuBooking[i]).day());
          cetakan += " ";
          cetakan += String((*waktuBooking[i]).hour());
          cetakan += " ";
          cetakan += String((*waktuBooking[i]).minute());
          cetakan += " ";
          cetakan += String((*waktuBooking[i]).second());
          cetakan += " ";
          cetakan += String(*lamaBooking[i]);
          cetakan += "\n";
        }
      }
      Serial.println(cetakan);
    }

};
