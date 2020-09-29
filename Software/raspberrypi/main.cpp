#include <iostream>
#include <errno.h>
#include <wiringPiSPI.h>
#include <signal.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

using namespace std;
/**
hashes to test
84dfb6902da20f8bbefd102e73509aa0 - bergera
527bd5b5d689e2c32ae974c6229ff785 - john
6d696d3f9b54c9c51fb23b7a5de2ed5e - pivot123
**/

// channel is the wiringPi name for the chip select (or chip enable) pin.
// Set this to 0 or 1, depending on how it's connected.
static const int CHANNEL = 0;

static const int TCP_PORT = 5230;

bool running = true;

int listenfd = 0, connfd = 0;
struct sockaddr_in serv_addr; 

unsigned char hextobytel(char s);

void SigHandler(int sig)
{
   switch(sig)
   {
      case SIGHUP:
         break;
      case SIGTERM:
      case SIGINT:
      if(running)
      {
         running = false;
         printf("\nShutdown received.");
      }
      else
      { // we already sent the shutdown signal
         printf("Emergency shutdown.");
         printf("close socket\n");
         close(connfd);
         exit(1);
      }
      break;
   }
}

void SetupSigHandler()
{
    signal(SIGCHLD, SIG_IGN);  // ignore child
    signal(SIGTSTP, SIG_IGN);  // ignore tty signals
    signal(SIGTTOU, SIG_IGN);  // ignore background write attempts
    signal(SIGTTIN, SIG_IGN);  // ignore background read attempts
    signal(SIGHUP,  SigHandler);
    signal(SIGTERM, SigHandler);
    signal(SIGINT,  SigHandler);
}

int fd_spi = 0;
void SPI_Setup()
{
   printf("SPI Initializing\n");
   // Configure the interface.
   // CHANNEL insicates chip select,
   // 500000 indicates bus speed.
   fd_spi = wiringPiSPISetupMode(CHANNEL, 5000,0);//5000 //500000
   printf("fd_spi:%d\n",fd_spi);
   sleep(1);
}

void SPI_write(unsigned char * buffer,int len)
{
/**
   unsigned char endien_buffer[4];
   int ctr = len-1;
   for(int xp=0;xp<len;xp++)
   {
      endien_buffer[ctr] = buffer[xp];
      ctr--;
   }
   for(int xp=0;xp<len;xp++)
   {
      buffer[xp] = endien_buffer[xp];
   }
/**/
   unsigned char local_buffer[4];
   for(int xp=0;xp<len;xp++)
      local_buffer[xp] = buffer[xp];

   printf("write to the spi len:%d ",len);
   for(int xp=0;xp<len;xp++)
      printf("%02X ",local_buffer[xp]);
   printf(" ");
   wiringPiSPIDataRW(CHANNEL, local_buffer, len);

   printf("returned len:%d ",len);
   for(int xp=0;xp<len;xp++)
      printf("%02X ",local_buffer[xp]);
   printf("\n");
}

int main(int argc, char * argv[])
{
   int fd, result, ret;
   unsigned char buffer[100];
   unsigned char out_buffer[8];
   unsigned char spi_buffer[4];

   unsigned char reset_spi_buffer[4] = {0x52,0x30,0x00,0x00};
   unsigned char start_spi_buffer[4] = {0x52,0x30,0x00,0x01};
   unsigned char set_exp_a_spi_buffer[4] = {0x52,0x30,0x10,0x00};
   unsigned char set_exp_b_spi_buffer[4] = {0x52,0x30,0x10,0x01};
   unsigned char set_exp_c_spi_buffer[4] = {0x52,0x30,0x10,0x02};
   unsigned char set_exp_d_spi_buffer[4] = {0x52,0x30,0x10,0x03};
   unsigned char set_range_spi_buffer[4] = {0x52,0x30,0x20,0x00};
   unsigned char get_cnt_low_spi_buffer[4] = {0x52,0x30,0x30,0x00};//low
   unsigned char get_cnt_high_spi_buffer[4] = {0x52,0x30,0x10,0x01};//high

   unsigned char hash_offset[32] = {
      0x67,0x45,0x23,0x01,
      0xef,0xcd,0xab,0x89,
      0x98,0xba,0xdc,0xfe,
      0x10,0x32,0x54,0x76
   }

   bool tcp_server = true;

   unsigned char md5_hash_in[64];memset(md5_hash_in,0x00,64);
   unsigned char md5_hash[32];memset(md5_hash,0x00,32);
   //read in our command line arguments
   for(int we=;we<argc;we++)
   {
      if(strcmp(argv[we],"-h") == 0)
      {
         //the hash we are going to look at
         tcp_server = false;
         snprintf(md5_hash_in,64,"%s",argv[we+1]);
      }
   }

   char sendBuff[1025];

   SetupSigHandler();

   SPI_Setup();

   //we need to parse out the hash into something useable
   //convert the string payload to bytes
   unsigned char tmpc[2];
   int c = 0;
   for(int i = 0;i < 32;i++)
   {
      tmpc[0] = hextobytel(md5_hash_in[i]);i++;
      tmpc[1] = hextobytel(md5_hash_in[i]);
      md5_hash[c] = ((tmpc[0] << 4) | tmpc[1]);
      c++;
   }

   printf("md5_has_in:%s\n",md5_hash_in);
   
   printf("md5_hash ");
   for(int xp = 0;xp < 32;xp++)
   {
      printf("%02",md5_hash[xp]);
   }
   printf("\n");

   //the fpga expects the parts to be offset a certain way
   printf("md5_hash minus");
   for(int xp = 0;xp < 32;xp++)
   {
      md5_hash[xp] = md5_hash[xp] - hash_offset[xp];
      printf("%02",md5_hash[xp]);
   }
   printf("\n");

   printf("hopefully we have something we can send now...\n");


   //original UI program talks to a tcp server
/**
   while(running && tcp_server)
   {
      printf("open up a socket to listen on\n");

      listenfd = socket(AF_INET, SOCK_STREAM, 0);
      memset(&serv_addr, '0', sizeof(serv_addr));
      memset(sendBuff, '0', sizeof(sendBuff)); 

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      serv_addr.sin_port = htons(TCP_PORT); 

      bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

      listen(listenfd, 10); 

      connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

      while(1)
      {
         //read
         memset(buffer,0x00,100);
         ret = read(connfd,buffer,100);
         if(ret > 0)
         {
            printf("tcp ret:%d ",ret);
            for(int xp=0;xp<ret;xp++)
               printf("%02X ",buffer[xp]);
            printf("\n");

            //parse
            for(int xp=0;xp<ret;xp++)
            {
               if(buffer[xp] == 0x01)//reset generator
               {
                  printf("reset generator\n");
                  SPI_write(reset_spi_buffer,4);
               }
               else if(buffer[xp] == 0x02)//start generator
               {
                  printf("start generator\n");
                  SPI_write(start_spi_buffer,4);
               }
               else if(buffer[xp] == 0x10)//set expected A
               {
                  printf("set expected A\n");
                  SPI_write(set_exp_a_spi_buffer,4);
                  xp++;
                  spi_buffer[3] = buffer[xp];xp++;
                  spi_buffer[2] = buffer[xp];xp++;
                  spi_buffer[1] = buffer[xp];xp++;
                  spi_buffer[0] = buffer[xp];
                  SPI_write(spi_buffer,4);
               }
               else if(buffer[xp] == 0x11)//set expected B
               {
                  printf("set expected B\n");
                  SPI_write(set_exp_b_spi_buffer,4);
                  xp++;
                  spi_buffer[3] = buffer[xp];xp++;
                  spi_buffer[2] = buffer[xp];xp++;
                  spi_buffer[1] = buffer[xp];xp++;
                  spi_buffer[0] = buffer[xp];
                  SPI_write(spi_buffer,4);
               }
               else if(buffer[xp] == 0x12)//set expected C
               {
                  printf("set expected C\n");
                  SPI_write(set_exp_c_spi_buffer,4);
                  xp++;
                  spi_buffer[3] = buffer[xp];xp++;
                  spi_buffer[2] = buffer[xp];xp++;
                  spi_buffer[1] = buffer[xp];xp++;
                  spi_buffer[0] = buffer[xp];
                  SPI_write(spi_buffer,4);
               }
               else if(buffer[xp] == 0x13)//set expected D
               {
                  printf("set expected D\n");
                  SPI_write(set_exp_d_spi_buffer,4);
                  xp++;
                  spi_buffer[3] = buffer[xp];xp++;
                  spi_buffer[2] = buffer[xp];xp++;
                  spi_buffer[1] = buffer[xp];xp++;
                  spi_buffer[0] = buffer[xp];
                  SPI_write(spi_buffer,4);
               }
               else if(buffer[xp] == 0x20)//set range
               {
                  printf("set range\n");
                  SPI_write(set_range_spi_buffer,4);
                  xp++;
                  spi_buffer[0] = buffer[xp];xp++;
                  spi_buffer[1] = buffer[xp];xp++;
                  spi_buffer[2] = buffer[xp];xp++;
                  spi_buffer[3] = buffer[xp];
		  SPI_write(spi_buffer,4);
               }
               else if(buffer[xp] == 0x30)//get count
               {
                  printf("get count\n");
		  //SPI_write(spi_buffer,4);
                  SPI_write(get_cnt_low_spi_buffer,4);
                  SPI_write(get_cnt_high_spi_buffer,4);

                  xp+=3;
               }
               else if(buffer[xp] == 0xFF)//close
               {
                  printf("close\n");
               }
	       printf("xp:%d\n",xp);
	       sleep(1);
            }

            //printf("write back\n");
            //just write back
            memset(out_buffer,0x00,8);
            write(connfd, out_buffer, 8); 
         }

         sleep(1);
      }
      close(connfd);
   }
**/

}

unsigned char hextobytel(char s)
{
    if(s == '0')
        return 0x0;
    else if(s == '1')
        return 0x1;
    else if(s == '2')
        return 0x2;
    else if(s == '3')
        return 0x3;
    else if(s == '4')
        return 0x4;
    else if(s == '5')
        return 0x5;
    else if(s == '6')
        return 0x6;
    else if(s == '7')
        return 0x7;
    else if(s == '8')
        return 0x8;
    else if(s == '9')
        return 0x9;
    else if(s == 'A')
        return 0xA;
    else if(s == 'B')
        return 0xB;
    else if(s == 'C')
        return 0xC;
    else if(s == 'D')
        return 0xD;
    else if(s == 'E')
        return 0xE;
    else if(s == 'F')
        return 0xF;
    else
	return 0x0;
}
