#include <stdio.h>
#include <stdlib.h>
#include "../header/header.h"
#include "../header/counting.h"
#include "../header/reading.h"

void takeoutFromQueue(void *arg)
{
    Threadcommon *common=arg;
    movAverage devices[common->numbOfDev];
    float temp;
    float finalResult;
    int i=1;
    QueueData *tempPacket;
    time_t now;
    time(&now);
    char *time;

    while (i<=common->numbOfDev)
    {
        devices[i].k=0;
        devices[i].k_prev=0;
        devices[i].k_next=0;
        devices[i].k_fourth=0;
        devices[i].summary=0;
        i++;
    }

    time=(char*)malloc(TIMELINE*sizeof(char));
    time=timeToString(time);
    FILE * log_file=fopen(time,"a");

    while(!TAILQ_EMPTY(&common->head))
    {
        pthread_mutex_lock(&common->mutex);
        tempPacket=(QueueData *)TAILQ_FIRST(&common->head);
        TAILQ_REMOVE(&common->head,tempPacket,entries);
        pthread_mutex_unlock(&common->mutex);

        devices[tempPacket->address].k_next=*(tempPacket->data);

        temp=mov_average(&devices[tempPacket->address],common->members);
        finalResult=moving_hysteresis(common,temp);

        fprintf(log_file,"Measured temperature from %d address of device with moving average and moving hysteresis :%.2f\t %s\n",
                tempPacket->address,finalResult,ctime(&now));

        sleep(common->samplingTime);
    }

    fclose(log_file);
    free(tempPacket);
    free(time);
}

