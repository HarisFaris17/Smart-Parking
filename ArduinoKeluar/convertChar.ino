#define ribuan ((uint16_t)1000)
#define ratusan ((uint8_t)100)
#define puluhan ((uint8_t)10)
#define char_to_int(char) ((uint8_t)(char-48))

uint16_t string_to_ribuan(char *data){
    uint16_t result;
    result = char_to_int(data[0])*ribuan+char_to_int(data[1])*ratusan+char_to_int(data[2])*puluhan+char_to_int(data[3]);
    return result;
}

uint8_t string_to_puluhan(char *data){
    uint16_t result;
    result = char_to_int(data[0])*puluhan+char_to_int(data[1]);
}