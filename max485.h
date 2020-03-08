
#ifndef MAX485_H
#define MAX485_H


//max485 init
void max485Init();

//max485 communication handler
void max485Handler();

void sendData(unsigned char *tx_buff, unsigned char length);
bool dataSent();

void setRxBuff(unsigned char *rx_buff);
unsigned char dataAvailable();


#endif //MAX485_H
