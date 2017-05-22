#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/queue.h>
#include <time.h>
#include "header/header.h"
#include "header/counting.h"
#define MAXLINE 1000


int Initalization(struct termios *old_term, struct termios *term,int *filedesp,struct config fileConfig,char *lf)
{

    time_t now;
    time(&now);
    char *serial="/dev/ttyS0";
    int fd;

    FILE * errorfile=fopen(lf,"w");

    if(!filedesp)
        {
        fprintf(errorfile,"filedescriptor is NULL\n",ctime(&now));
        return 1;
        }

    else
        fd=*filedesp;
        fd=open(serial,O_RDWR);
            if(fd<0)
        {
            fprintf(errorfile,"Cannot open the filedescriptor \n",ctime(&now));
            return 1;
        }

    tcgetattr(fd,old_term);
    term=malloc(sizeof(struct termios));
        if(!term)
        {
            fprintf(errorfile,"NO enough memory to allocate term\n",ctime(&now));
            return 1;
        }
    term->c_cflag = CS8 | CLOCAL | CREAD ;
    term->c_iflag =0;
    term->c_oflag= 0;
    cfsetispeed(term,fileConfig.BAUD);
    cfsetospeed(term,fileConfig.BAUD);

    tcflush(fd, TCIFLUSH);
    if(!tcsetattr(fd,TCSANOW,term))
    {
            free(fileConfig.BAUD);
            fclose(errorfile);
            return 0;
    }
    else
    {
        free(fileConfig.BAUD);
        fprintf(errorfile,"RS-485 config error:\n",ctime(&now));
        fclose(errorfile);
        return 1;
    }


}

void ReadConfig(struct config fileConfig,float values[][TOPMEASURES])
{
char *buffer;
char *p=NULL;
char pathOfConfig[]="/media/herczig/ACA24E17A24DE704/Thesis/config.txt";  /*or in root "config.txt" */
int i,j=0;
for(i!=veg,i++)
{
    for(j!=veg;j++)
    values[i][j]
}
char comment='=';
FILE * fconfig,errorfile;

    errorfile=fopen(lf,"w");

    fconfig=fopen(pathOfConfig,"r");
    buffer=malloc(sizeof(MAXLINE));
        if(!buffer)
        {
            fprintf(errorfile,"Can not allocate memory to buffer\n",ctime(&now));
            return 1;
        }
        if(fconfig)
        {

            while(fgets(buffer,MAXLINE,fconfig))
            {

                if(*buffer!='\n')      //minimum the second function
                {
                   switch(*buffer)
                    {
                        case 'R':       //Request Time
                            p=strchr(buffer,comment);
                            p++;
                            fileConfig.time=atoi(p);
                            if(fileConfig.time<1000 || fileConfig.time>1000000)
                                fileConfig.time=1000;
                        continue;

                        case 'n':       //Number of devices
                            p=strchr(buffer,comment);
                            p++;
                            fileConfig.numbOfDev=atoi(p);
                            if(fileConfig.numbOfDev<DevMin || fileConfig.numbOfDev>DevMax)
                                fileConfig.numbOfDev=DevMin;
                        continue;

                        case 's':       //SamplingTime
                            p=strchr(buffer,comment);
                            p++;
                            fileConfig.samplingTime=atoi(p);
                            if(fileConfig.samplingTime<DEFTIME || fileConfig.samplingTime>MAXTIME)
                                fileConfig.samplingTime=DEFTIME;
                        continue;

                       case 'B':       //Baudrate
                            p=strchr(buffer,comment);
                            p++;
                            fileConfig.BAUD=malloc(sizeof(p));
                            strcpy(fileConfig.BAUD,p);
                            if(fileConfig.BAUD!="B9600" ||fileConfig.BAUD!="B38400" ||fileConfig.BAUD!="B57600" ||fileConfig.BAUD!="B115200" )
                                fileConfig.BAUD=DefBaud;

                        continue;

                        case 'D':           //Delta for moving histeresys
                            p=strchr(buffer,comment);
                            p++;
                            fileConfig.Delta=atoi(p);
                            if(fileConfig.Delta<DELTAMIN || fileConfig.Delta>DELTAMAX)
                                fileConfig.Delta=DELTAMIN;
                            continue;

                       case 'm':           //members to flowchart
                            p=strchr(buffer,comment);
                            p++;
                            fileConfig.members=atoi(p);
                            if(fileConfig.members<MEMBERSMIN || fileConfig.members>MEMBERSMAX)
                                fileConfig.members=MEMBERSMIN;
                            continue;

                    }
                }
            }
        }
        else
            fprintf(fconfig,"Cannot open config file\n",ctime(&now));



        free(buffer);
        fclose(fconfig);


}
/**Direction=0 if OUTPUT, Direction =1 if INPUT*/

int queueInit(incoming_data *data,char direction)       //direction INPUT or OUTPUT
{
    data=malloc(sizeof(incoming_data));
        if(!data)
            return 1;

    struct tailhead InHd, OutHd;

    if(!direction)
    {
        TAILQ_HEAD(OutHd, incoming_data) head;
        TAILQ_INIT(&OutHd);
    }
	else
    {
        TAILQ_HEAD(InHd, incoming_data) head;
        TAILQ_INIT(&InHd);
    }
    pthread_mutex_init(data->mutex,NULL);
    return 0;
}


}
int takeoutFromQueue(incoming_data *Received_data)
{



  while (1)
    {
         pthread_mutex_lock(Received_data->mutex);
         data=TAILQ_FIRST(&head);
         pthread_mutex_unlock(Received_data->mutex);
         moving_hysteresis(conffile.Delta, Received_data->data );
	     TAILQ_REMOVE(&head, n1, entries);

     }

}
int controllingProcess()
{

}



