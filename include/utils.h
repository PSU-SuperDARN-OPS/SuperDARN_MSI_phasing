//
// Created by bgklug on 8/2/19.
//

#ifndef PHASINGCALIBRATION_UTILS_H
#define PHASINGCALIBRATION_UTILS_H

//#define MSG_NOSIGNAL 0x4000

void mypause ( void );
int opentcpsock(char *hostip, int port);
int send_data(int fd,void  *buf,size_t buflen);
int recv_data(int fd,void *buf,size_t buflen);




#endif //PHASINGCALIBRATION_UTILS_H
