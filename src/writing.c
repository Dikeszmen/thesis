#include <stdio.h>
#include <stdlib.h>
#include "../header/reading.h"
#include "../header/crc.h"

int sendPacket(int fd, unsigned char address, unsigned char cmd,char *data, uint16_t dLen)
{
    if ( !data || fd <0 || dLen <0 )
        {
            printf("Sending Packet not happened\n data is null or invalid Filedescriptor or datalength\n");
            syslog(LOG_ERR,"sendPacket, data is NULL or invalid Filedescriptor or datalength.Writing Error");
            return -1;
        }
    char *buff=(char*)malloc((dLen+13)*sizeof(char));
    if(!buff)
        {
            printf("No enough memory\n");
            syslog(LOG_ERR,"No enough memory");
            return -1;
        }
    int i=0;
    int dataElement=11;
    uint16_t crc=0;
    unsigned char len1,len2,crc1,crc2;
    crc = addCRC(crc, address);
    crc = addCRC(crc, cmd);
    len1= dLen & 0xff;
    crc = addCRC(crc,len1);
    len2 = (dLen >> BYTE) & 0xff;
    crc = addCRC(crc, len2);
    if(dLen>=0)
        {
            int j=0;
	    do
	    {
                buff[dataElement]=*data;
                crc = addCRC(crc, *data);
                dataElement++;
	    }
            while(j<dLen)
	    ;
        }

    crc1=crc & 0xff;
    crc2=(crc>>BYTE) & 0xff;
    while(i!=5)
        {
            buff[i]=0x55;
            i++;
        }
    buff[5]=0xFF;
    buff[6]=0x01;
    buff[7]=address;
    buff[8]=cmd;
    buff[9]=len1;
    buff[10]=len2;
    
    buff[dataElement]=crc1;
    dataElement++;
    buff[dataElement]=crc2;
    dataElement++;
    i=write(fd,buff,dataElement);
    if(i!=dataElement)
        {
            syslog(LOG_ERR,"%s",strerror(i));
            free(buff);
            return -1;
        }
    else
        {
            free(buff);
            return 1;
        }
}

