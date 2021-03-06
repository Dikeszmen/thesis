#ifndef READING_H_INCLUDED
#define READING_H_INCLUDED
#include <string.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <syslog.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "Init.h"
#define LIMIT 1024
#define FF 0xFF
#define ONE 1
#define SAMPTIME 1000
#define ZERO 0
#define MAXU 5
#define PING 0x69
#define TERMCMD 0x01
typedef enum PacketState
{
    /*! Default condition*/
    EmptyState,

    /*! min. one 0x55 received*/
    moto55,

    /*! after 0xFF, 0x01 received*/
    moto1,

    /*! Address */
    address,

    /*! Command*/
    command,

    /*! Low byte of datalength*/
    DLenLow,

    /*! High byte of datalength*/
    DLenHigh,

    /*! Databyte */
    Data,

    /*! low byte of crc  Packet*/
    CrcLow,

    /*! high byte of crc  Packet*/
    CrcHigh

} packetState;

typedef struct statistic
{
    int packetError;
    int packet;
    int validPacket;
    int overrun;
    int send_PollPacket;
    int send_TermPacket;
    int received_PollPacket;
    int received_TermPacket;
} Statistic;


typedef struct queueData
{
    /** packet item data */
    char address;
    char cmd;
    uint16_t dlen;
    char *data;
    /**  packet item use a TAILQ. This is for TAILQ entry */
    TAILQ_ENTRY(queueData) entries;
} QueueData;


void  readingFromSerial(void *arg);

//QueueData *reserve(char data);

#endif // READING_H_INCLUDED
