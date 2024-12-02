unsigned char crc;
unsigned char i,j,checksum,data1=0;
unsigned char *data2;
sbit my_data at P0.b4;
unsigned char bit1, k;
unsigned char *strPointer;
unsigned char str[] = {0xFF, 0, 0, 0, 0, 0,0,0,0,0,0};
unsigned char bitochki;
sbit presence at bitochki.b0;

/* void UART_Init()
{

        T2CON = 0x34;
        TMOD = 0x20;
        T2MOD = 0x00;
        TH2 = RCAP2H = 0xff;
        TL2 = RCAP2L = 0xEF;   // 19200 - ffef, 9600 - ffdf
        SCON = 0x50;            //   Mode 1, reception enable
        PCON = 0x80;
        TR2_bit = 1;
        ES_bit = 1;
        EA_bit = 1;            //    Start timer 1

} */

void UART_init() {
    SCON = 0x50;
    T2CON = 0x30;
    RCAP2H = 0xFF;
    RCAP2L = 0xDF;
    TH2 = 0xFF;
    TL2 = 0xDF;
    TR2_bit = 1;
}

/*
void UART_ISR() iv IVT_ADDR_ES ilevel 1 ics ICS_AUTO
{
     if (RI_bit)
     {
         received_data = SBUF;
         RI_bit = 0;
         P1 = received_data;
     }
     if (TI_bit)
     {
         TI_bit = 0;

         if (*transmit_buffer != '\0')
         {
            SBUF = *transmit_buffer++;
         }
         else
         {
            transmit_in_progress = 0;
         }
     }
}

*/

void UART_Write1() {
    strPointer = str;
    for (i = 0; i < 4; i++) {
        SBUF = *strPointer;
        while (!TI_bit);
        TI_bit = 0;
        strPointer++;
    }
}

/*
void UART_WriteString(unsigned char *str)
{
 if (!transmit_in_progress)
 {
    transmit_buffer = str;
    transmit_in_progress = 1;
    SBUF = *transmit_buffer++;

 }

}*/

void Wire_Write() {
    for (i = 0; i < 8; i++) {
        my_data = 0;
        delay_us(2);
        my_data = data1.b0;
        delay_us(60);
        my_data = 1;
        data1 = data1 >> 1;
    }
}

unsigned char Wire_Read() {
    data1 = 0;
    for (i = 0; i < 8; i++) {
        my_data = 0;
        delay_us(2);
        my_data = 1;
        delay_us(12);
        if (my_data) {
            data1 |= (1 << i);
        }
        delay_us(60);
    }
    return data1;
}


unsigned char Wire_Reset() {
    my_data = 0;
    delay_us(480);
    my_data = 1;
    delay_us(60);
    presence = my_data;
    delay_us(440);
    return !presence;
}


unsigned char CRC_checksum() {
    crc = 0;
    for (k = 0; k < 8; k++) {
        crc ^= data2[k];
        for (bit1 = 0; bit1 < 8; bit1++) {
            if (crc & 0x01) {
                crc = (crc >> 1) ^ 0x8C;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void main() {
    UART_init();
    while (1)
    {
    Wire_Reset();
    data1 = 0xCC;
    Wire_Write();
    data1 = 0x44;
    Wire_Write();
    delay_ms(750);
    Wire_Reset();
    data1 = 0xCC;
    Wire_Write();
    data1 = 0xBE;
    Wire_Write();
    for (j = 1; j <= 9; j++)
    {
        str[j] = Wire_Read();
    }
    data2 = str + 1;
    if (str[9] != CRC_checksum())
    {
        return;
    }
    str[3] = (0xFF + str[1] + str[2]) % 256;
    UART_Write1();
    
    delay_ms(250);
    }
}