#include <stdio.h>
#include <stdlib.h>
#include "../header/header.h"
#include "../header/crc.h"
#include "../header/reading.h"
#include "../header/reading.h"
/**
Reading from the serial port. To check the incoming packet, use the Motorola protocol
*/

void  readingFromSerial(void *arg)
{
    QueueData *receivingData=NULL;
    unsigned char data,j=0;
    Crc packetCrc,calculateCrc;
    calculateCrc=packetCrc=0;
    Statistic Packetstatistic= {0};
    packetState State=EmptyState;
    Threadcommon *common=arg;
    int error=0;

    if (!common || common->fd <0 )
        {
            syslog(LOG_ERR,"%s\n",strerror(errno));
            Packetstatistic.rError++;
            return;
        }
    int i=0;

    while(error==read(common->fd,&data,ONEBYTE))
        {
            if (error==-1)
            {
                syslog(LOG_ERR,"%s",strerror(errno));
                exit(EXIT_FAILURE);
            }
            switch (State)
                {

                case EmptyState:
                    if (data == 0x55)
                        State= moto55;
                        continue;

                case moto55:
                    if (data == 0x55)
                        {
                            j++;
                            if(i==5)
                                {
                                    syslog(LOG_ERR,"Too much 0x55 received\tReceived numbers of data[i]:%d\n",i);
                                    Packetstatistic.rError++;
                                    break;
                                }
                            continue;
                        }
                    if (data== FF)
                        {
                            State=motoFF;
                            continue;
                        }
                    else
                        {
                            Packetstatistic.packetError++;
                            syslog(LOG_ERR,"After 0x55 did not receive proper data(0xFF)\n");
                            break;
                        }

                case motoFF:
                    if(data==1)
                        {
                            State=moto1;
                            continue;
                        }
                    else
                        {
                            Packetstatistic.packetError++;
                            syslog(LOG_ERR,"After 0xFF did not receive proper data(0x01)\n\n");
                            break;
                        }


                case moto1:
                    if(data)
                        {
                            State= address;
                            Packetstatistic.packet++;
                        }
                    else
                        break;

                case address:
                    if (!receivingData)
                        {
                            calculateCrc = addCRC(calculateCrc, data);
                            receivingData=reserve(data);
                            if (!receivingData)
                                {
                                    syslog(LOG_ERR,"Cannot reserved memory to receivingData\n");
                                    break;
                                }

                            State = command;
                            continue;
                        }
                    else
                        Packetstatistic.overrun++;         //Packets are incoming too fast, should take bigger hold time between readings
                    break;

                case command :
                    calculateCrc = addCRC(calculateCrc,data);
                    receivingData->cmd = data;
                    State = DLenLow;
                    continue;

                case DLenLow :
                    calculateCrc = addCRC(calculateCrc, data);
                    receivingData->dlen = data;
                    State = DLenHigh;
                    continue;

                case DLenHigh :
                    calculateCrc = addCRC(calculateCrc, data);
                    receivingData->dlen|= (data << BYTE) ;

                    if (receivingData->dlen > 0)
                        {
                            if (receivingData->dlen <= LIMIT)
                                {
                                    receivingData->data =(float *)malloc(receivingData->dlen*sizeof(QueueData));
                                    if(!receivingData->data)
                                        {
                                            syslog(LOG_ERR,"No enough memory\n");
                                            Packetstatistic.packetError++;
                                            break;
                                        }
                                    State = Data;
                                }
                            else
                                {
                                    syslog(LOG_ERR,"Too big the datalength\n");
                                    break;
                                }
                        }
                    else
                        {
                            State =  CrcLow;
                            continue;
                        }
                case Data :
                    calculateCrc = addCRC(calculateCrc, data);
                    *(receivingData->data) = data;
                    State = CrcLow;
                    continue;

                case CrcLow :
                    packetCrc = data;
                    State = CrcHigh;
                    continue;

                case CrcHigh:
                    packetCrc |=( data<< BYTE);
                    if (compareCRC(packetCrc, calculateCrc))
                        {
                            if(receivingData->cmd==1 && *(receivingData->data))           //cmdTerm =1, not polling
                                {
                                    pthread_mutex_lock(&common->mutex);
                                    TAILQ_INSERT_TAIL(&common->head,receivingData,entries);
                                    pthread_mutex_unlock(&common->mutex);
                                    Packetstatistic.validPacket++;
                                }
                            else
                                {
                                    Packetstatistic.pollPacket++;
                                    syslog(LOG_NOTICE,"Slave Keep Alive:%c\n",receivingData->address);
                                }

                        }
                    break;
                }

            receivingData=NULL;
            State = EmptyState;
            syslog(LOG_NOTICE,"\n Packetstatistic\n packetError=%d\n",Packetstatistic.packetError);
            syslog(LOG_NOTICE,"packet=%d\n",Packetstatistic.packet);
            syslog(LOG_NOTICE,"validPacket=%d\n",Packetstatistic.validPacket);
            syslog(LOG_NOTICE,"overrun=%d\n",Packetstatistic.overrun);
            syslog(LOG_NOTICE,"emptyPacket=%d\n",Packetstatistic.pollPacket);
            syslog(LOG_NOTICE,"Error=%d\n",Packetstatistic.rError);

            sleep(common->samplingTime);    // alvoido
        }


}

QueueData *reserve(char data)
{
    QueueData *temp;
    temp=(QueueData *)malloc(sizeof(QueueData));
    if (!temp)
        return NULL;
    temp->address=data;
    temp->dlen = 0;
    temp->data = NULL;
    return temp;
}



int sendPacket(int *fd, unsigned char address, unsigned char cmd, unsigned char *data, char dLen)
{
    Statistic packet;
    packet.wError=0;

    if ( (!(fd && data) || dLen<0 ) || *fd<0  )
        {
        syslog(LOG_ERR,"%s\nStatistics:%d",strerror(errno),packet.wError++);
        return -1;
        }

    int i;
    uint16_t crc=0;
    uint16_t datalength;

    char motorola55=0x55,motorolaFF=0XFF,motorola1=0x01;

    for (i=0; i<5; i++)
        write(*fd,&motorola55,ONEBYTE);

    write(*fd,&motorolaFF,ONEBYTE);
    write(*fd,&motorola1,ONEBYTE);
    crc = addCRC(crc, address);
    write (*fd,&address,ONEBYTE);
    crc = addCRC(crc, cmd);
    write(*fd,&cmd,ONEBYTE);
    datalength= dLen;
    crc = addCRC(crc,datalength);
    datalength = (dLen >> BYTE);
    crc = addCRC(crc, datalength);
    write(*fd, &datalength, ONEBYTE);

    if(dLen>0)
        {
            for (i=0; i<dLen; i++,data++)
                {
                    crc = addCRC(crc, *data);
                    write(*fd,data,ONEBYTE);
                }
        }
    else if(!datalength)
        {
            *data=ZERO;
            crc = addCRC(crc, *data);
            write(*fd,data,ONEBYTE);

        }


    write(*fd,&crc,ONEBYTE);
    crc=(crc >>BYTE);
    write(*fd,&crc,ONEBYTE);


    return 1;
}

void sendRequest(void *arg)
{

    Threadcommon *common=arg;
    int addresses=1;
    unsigned char requestType;
    unsigned char requestCounter=0;
    const char cmdPing=0;
    const char cmdTerm=1;
    int retvalue;
    Statistic packet;
    packet.TermPacket=0;
    packet.pollPacket=0;
    while(1)
        {

            if(requestCounter==MAXREQUEST)
                requestCounter=0;


            requestType = requestCounter % 3;
            ++requestCounter;

            if(!requestType)
                {
                    while(addresses<=common->numbOfDev)
                        {
                            if(common->sensors[addresses].state)
                               retvalue=sendPacket(&common->fd,addresses, cmdTerm, NULL,0);
                                if(retvalue>0);
                                {
                                packet.TermPacket++;
                                syslog(LOG_NOTICE,"Asking Term packet transmitted :%d\n",packet.TermPacket);
                                addresses++;
                                sleep(common->time);
                                }
                                if(retvalue==-1)
                                    syslog(LOG_ERR,"Shit happened:%s\n",strerror(errno));


                        }

                    requestCounter++;

                }
            else
                {
                    while(addresses<=common->numbOfDev)
                        {
                            if(common->sensors[addresses].state)
                                sendPacket(&common->fd,addresses, cmdPing, NULL,0);
                                packet.pollPacket++;
                                syslog(LOG_NOTICE,"Asking Polling packet transmitted :%d\n",packet.pollPacket);
                                addresses++;
                                sleep(common->time);
                        }

                    requestCounter++;
                }


            addresses=0;
        }
}

