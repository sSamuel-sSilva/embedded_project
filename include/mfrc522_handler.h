#ifndef MFRC522HANDLER_H
#define MFRC522HANDLER_H

#include "mfrc522.h"


enum RESPONSE {
    FAILED,
    SUCCESS,
    NOTHING,
};


typedef struct b button;

typedef struct card_event {
    char card_uid[16];
    // time time; hora que foi feito a leitura
} card_event;


int read_card(char** card, MFRC522Ptr_t mfrc);


#endif