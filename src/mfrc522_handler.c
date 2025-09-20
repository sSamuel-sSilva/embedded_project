#include "pico/stdlib.h"
#include "../include/connection.h"
#include "../include/led.h"
#include "../include/button.h"
#include "../include/mfrc522_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>



int read_card(char** card, MFRC522Ptr_t mfrc)
{
    if (PICC_IsNewCardPresent(mfrc))
    {
        char buffer[32];
        buffer[0] = '\0';
        int offset = 0;

        PICC_ReadCardSerial(mfrc);
        for (int i = 0; i < mfrc->uid.size; i++)
        {
            offset += snprintf(
                buffer + offset, 
                sizeof(buffer) - offset, 
                i == mfrc->uid.size - 1 ? "%02X" : "%02X ",
                mfrc->uid.uidByte[i]);
        }



        printf("%s\n", buffer);
        
        if (strlen(buffer) == 0) return FAILED;
        
        card = (char)malloc(sizeof(strlen(buffer)) + 1);
        
        
        strcpy(card, buffer);
        return SUCCESS;
    }

    return NOTHING;
}