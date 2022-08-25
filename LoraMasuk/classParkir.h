String listKodeParkir[] = {"A1", "A2", "A3", "A4", "B1", "B2", "B3", "B4"};
String characterBooking = "0123456789ABCD";
const int banyakParkir = 20;
class Parkir {
  private:
    String *kodeParkir[banyakParkir];
    String *kodeBooking[banyakParkir];
    DateTime *waktuBooking[banyakParkir];
    int *lamaBooking[banyakParkir];

  public:
    Parkir() {
      *kodeParkir = {}//new String("A1")};//,new String("A2")};
      *kodeBooking = {}//new String("B123")};//, new String("B490")};
      *waktuBooking = {}//new DateTime(2022, 7, 19, 15, 00, 00)};//, new DateTime(2022, 7, 15, 14, 38, 00)};
      *lamaBooking = {}//new int(3)};//,new int(3)};
    }

    void getParkir(int index, char &destination) {
      parkirToString(*kodeParkir[index], *kodeBooking[index], *waktuBooking[index], *lamaBooking[index], destination);
    }

    //check apakah waktu sekarang untuk parkir dengan index tertentu apakah bisa masuk
    boolean checkParkirIfInRangeWaktuBook(int index, DateTime now) {
      DateTime waktuBook = *waktuBooking[index];
      int lamaBook = *lamaBooking[index];
      // jika dia merupakan parkir offline, berarti harus dalam range 3 jam.
      if (now.unixtime() >= waktuBook.unixtime() && (waktuBook.unixtime() + 3600U * (lamaBook == 99 ? 3 : lamaBook)) > now.unixtime()) {
        return true;
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

    boolean checkAndTambahParkirOffline(DateTime timeNow, char &destination) {
      long mulai = random(0, 7);
      int banyakSpotParkir = 8;//sizeof(listKodeParkir);
      for (int i = mulai; i < banyakSpotParkir + mulai; i++) {
        //        Serial.println(i);
        //        Serial.println(banyakSpotParkir + i);
        //        Serial.println("cek parkir");
        int spotParkir = i % banyakSpotParkir;
        //3 jam karena maksimal parkir offline adalah 3 jam
        if (!checkParkirTidakTersedia(listKodeParkir[spotParkir], timeNow, 3)) {
          //          Serial.println("cek parkirkkkk");
          String kodeBooking = kodeBookingGenerator();
          parkirToString(listKodeParkir[spotParkir], kodeBooking, timeNow, 99, destination);
          //lamaBook adalah 99 untuk menandakan bahwa parkir tersebut dibooking secara offline
          tambahParkir(listKodeParkir[spotParkir], kodeBooking, timeNow, 99);
          return true;
        }
      }
      return false;
      //      String kodeBooking = kodeBookingGenerator();
      //      parkirToString("A1", kodeBooking, timeNow, 99, destination);
      //      //      kode.toCharArray(&destination,21);
      //      //      for (int i = 0; i < 20; i++) {
      //      //        destination[i] = kode[i];
      //      //      }
      //      //      kode.toCharArray(destination,21);
      //      Serial.println(&destination);
      //      return true;
    }

    void parkirToString(String kodeParkir, String kodeBook, DateTime waktuBook, int lamaBook, char &destination) {
      char waktuBookingChar[17] = "";
      if (lamaBook == 99) {
        sprintf(waktuBookingChar, "%04d%02d%02d%02d%02d%02d", waktuBook.year(), waktuBook.month(), waktuBook.day(), waktuBook.hour(), waktuBook.minute(), waktuBook.second());
        //        Serial.print("waktu booking char : ");
        //        Serial.println(waktuBookingChar);
        String kode = kodeParkir + kodeBook;
        for (int i = 0; i < 14; i++) {
          kode += waktuBookingChar[i];
        }
        //kenapa 21 ? karena data yang ingin dikirim adalah 20 karakter dengan tanpa lamaBooking karena. dan tiap array karakter harus memiliki null-terminating \0 karakter di akhir arraynya oleh karena itu memerlukan 21
        kode.toCharArray(&destination, 21);
      } else {
        sprintf(waktuBookingChar, "%04d%02d%02d%02d%02d%02d%02d", waktuBook.year(), waktuBook.month(), waktuBook.day(), waktuBook.hour(), waktuBook.minute(), waktuBook.second(), lamaBook);
        //        Serial.print("waktu booking char : ");
        //        Serial.println(waktuBookingChar);
        String kode = kodeParkir + kodeBook;
        for (int i = 0; i < 16; i++) {
          kode += waktuBookingChar[i];
        }
        //kenapa 23 ? karena data yang ingin dikirim adalah 22 karakter. dan tiap array karakter harus memiliki null-terminating \0 karakter di akhir arraynya oleh karena itu memerlukan 22
        kode.toCharArray(&destination, 23);
      }
    }

    boolean checkParkirTidakTersedia(String kodePark, DateTime waktuBook, int lamaBook) {
      unsigned int waktuBookUnixTime = waktuBook.unixtime();
      for (int i = 0; i < banyakParkir; i++) {
        if (kodeParkir[i] != NULL) {
          if (*kodeParkir[i] == kodePark &&
              ((((*waktuBooking[i]).unixtime() < waktuBookUnixTime) &&
                ((*waktuBooking[i]).unixtime() + 3600U * (*lamaBooking[i]) > waktuBookUnixTime)) ||
               (((*waktuBooking[i]).unixtime() < waktuBookUnixTime + 3600U * (lamaBook == 99 ? 3 : lamaBook) &&
                 ((*waktuBooking[i]).unixtime() + 3600U * (*lamaBooking[i]) > waktuBookUnixTime + 3600U * (lamaBook == 99 ? 3 : lamaBook))
                )))) {
            return true;
          }
        }

      }
      return false;
    }

    String kodeBookingGenerator() {
      boolean ketemu = false;
      String kodeBooking = "";
      int pointer;
      while (!ketemu) {
        for (int i = 0; i < 4; i++) {
          pointer = random(0, 13);
          kodeBooking += characterBooking.charAt(pointer);
        }
        if (!checkKodeBooking(kodeBooking)) {
          ketemu = true;
          Serial.println("Kode Booking baru" + kodeBooking);
        }
      }
      return kodeBooking;
    }

    boolean checkKodeBooking(String kodeBook) {
      for (int i = 0; i < banyakParkir; i++) {
        if (kodeBooking[i] != NULL) {
          if (kodeBook == *kodeBooking[i]) return true;
        }
      }
      return false;
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
