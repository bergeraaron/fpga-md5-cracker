#include <iostream>
#include <errno.h>
#include <wiringPiSPI.h>
#include <wiringPi.h>
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

#include <openssl/md5.h>

using namespace std;
/**
hashes to test
84dfb6902da20f8bbefd102e73509aa0 - bergera
527bd5b5d689e2c32ae974c6229ff785 - john
**/
//pins for the hash match and the resetting of the generator
#define PIN_HM 17 
#define PIN_RG 27

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
   fd_spi = wiringPiSPISetupMode(CHANNEL, 500000,0);//5000 //500000
   printf("fd_spi:%d\n",fd_spi);
   sleep(1);
}

void SPI_write(unsigned char * buffer,int len)
{
   //noop
   //unsigned char noop[4]={0x00,0x00,0x00,0x00};
   //wiringPiSPIDataRW(CHANNEL, noop, 4);
     
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

void SPI_write_and_read(unsigned char * buffer, int * len)
{
   //unsigned char noop[4]={0x00,0x00,0x00,0x00};
   //wiringPiSPIDataRW(CHANNEL, noop, 4);
   printf("write to the spi len:%d ",*len);
   for(int xp=0;xp<*len;xp++)
      printf("%02X ",buffer[xp]);
   printf(" ");
   wiringPiSPIDataRW(CHANNEL, buffer, *len);
   printf("returned len:%d ",*len);
   for(int xp=0;xp<*len;xp++)
      printf("%02X ",buffer[xp]);
   printf("\n");
}

void monitor_pins_setup()
{
	wiringPiSetupGpio();
	pinMode(PIN_HM,INPUT);//Hash Matched
	pullUpDnControl(PIN_HM,PUD_DOWN);
	pinMode(PIN_RG,INPUT);//Reset Generator
	pullUpDnControl(PIN_RG,PUD_DOWN);
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
   unsigned char get_cnt_low_spi_buffer[4] = {0x52,0x30,0x10,0x00};//low
   unsigned char get_cnt_high_spi_buffer[4] = {0x52,0x30,0x10,0x01};//high
   unsigned char get_text_char[4] = {0x44,0x33,0x22,0x11};
   unsigned char nop[4] = {0x00,0x00,0x00,0x00};

   unsigned char get_txt_char[4][4] = { 
                                          {0x44,0x00,0x00,0x01},
                                          {0x44,0x00,0x00,0x02},
                                          {0x44,0x00,0x00,0x03},
					  {0x44,0x00,0x00,0x04}
                                      };
   uint32_t expected_off[4];
   expected_off[0] = 0x67452301;
   expected_off[1] = 0xefcdab89;
   expected_off[2] = 0x98badcfe;
   expected_off[3] = 0x10325476;

   unsigned char plain_text[64]; memset(plain_text,0x00,64);
   uint8_t pt_ctr = 0;
   unsigned char empty[64];memset(empty,0x00,64);

   bool tcp_server = true;

   char md5_hash_in[64];memset(md5_hash_in,0x00,64);
   unsigned char md5_hash[32];memset(md5_hash,0x00,32);
   //read in our command line arguments
   for(int we = 0;we < argc;we++)
   {
      if(strcmp(argv[we],"-h") == 0)
      {
         //the hash we are going to look at
         tcp_server = false;
         snprintf(md5_hash_in,64,"%s",argv[we+1]);

         if(strlen(md5_hash_in) == 0)
         {
            printf("no hash supplied\n");
            return 0;
         }
      }
      if(strcmp(argv[we],"-p") == 0)
      {
	  if(strlen(argv[we+1]) == 0)
	  {
              printf("no password provided\n");
	      return 0;
	  }
          tcp_server = false;
	  MD5(((unsigned char*)(argv[we+1])),strlen(argv[we+1]),(unsigned char*)md5_hash_in);
	  char tmpbuf[64];memset(tmpbuf,0x00,64);
	  for(int xp=0;xp <(strlen(md5_hash_in));xp++)
	  {
		  printf("%02X",md5_hash_in[xp]);
		  sprintf(tmpbuf,"%s%02X",tmpbuf,md5_hash_in[xp]);
	  }
	  printf("\n");
	  printf("tmpbuf:%s\n",tmpbuf);
	  memset(md5_hash_in,0x00,64);
          for(int xp=0;xp < strlen(tmpbuf);xp++)
		md5_hash_in[xp] = tmpbuf[xp];	  
	  //return 0;
      }
   }

   char sendBuff[1025];

   SetupSigHandler();

   SPI_Setup();

   monitor_pins_setup();

   if(tcp_server)
   {
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
                     int len = 4;
                     memset(out_buffer,0x00,8);
                     int buf_ctr = 0;
                     printf("get count\n");
                     memset(spi_buffer,0x00,4);
                     memcpy(spi_buffer,get_cnt_high_spi_buffer,4);
                     SPI_write(spi_buffer,4);
                     memset(spi_buffer,0x00,4); 
                     memcpy(spi_buffer,get_cnt_low_spi_buffer,4);
                     len = 4;
                     SPI_write_and_read(spi_buffer,&len);
                     for(int re = 0;re < len;re++)
                     {
                        out_buffer[buf_ctr] = spi_buffer[len-re-1];
                        buf_ctr++;
                     }

                     memcpy(spi_buffer,nop,4);
                     len = 4;
                     SPI_write_and_read(spi_buffer,&len);
                     for(int re = 0;re < len;re++)
                     {
                        out_buffer[buf_ctr] = spi_buffer[len-re-1];
                        buf_ctr++;
                     }

                     xp+=3;
                  }
                  else if(buffer[xp] == 0xFF)//close
                  {
                     printf("close\n");
                     break;
                  }
                  printf("xp:%d\n",xp);
                  sleep(1);
               }

               //printf("write back\n");
               //just write back
               write(connfd, out_buffer, 8); 
            }

            sleep(1);
         }
         close(connfd);
      }
   }
   else
   {
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
/**
      printf("md5_has_in:%s\n",md5_hash_in);
      
      printf("md5_hash ");
      for(int xp = 0;xp < 16;xp++)
      {
         printf("%02X",md5_hash[xp]);
      }
      printf("\n");
**/
      //the fpga expects the parts to be offset a certain way
      uint32_t temp_hash = 0;
      int off = 0;
      for(int xp=0;xp<4;xp++)
      {
         temp_hash = (uint8_t)md5_hash[off+0] | 
                     (uint8_t)md5_hash[off+1] << 8 | 
                     (uint8_t)md5_hash[off+2] << 16 | 
                     (uint8_t)md5_hash[off+3] << 24;

         temp_hash -= expected_off[xp];

         md5_hash[off+3] = temp_hash & 0xFF;
         md5_hash[off+2] = (temp_hash >> 8) & 0xFF;
         md5_hash[off+1] = (temp_hash >> 16) & 0xFF;
         md5_hash[off+0] = (temp_hash >> 24) & 0xFF;
         off+=4;
      }
/**
      printf("md5_hash minus ");
      for(int xp = 0;xp < 16;xp++)
      {
         printf("%02X",md5_hash[xp]);
      }
      printf("\n");
**/
      printf("hopefully we have something we can send now...\n");

      printf("check the pins\n");
      printf("PIN_HM:%d\n",digitalRead(PIN_HM));
      printf("PIN_RG:%d\n",digitalRead(PIN_RG));

      printf("send the reset command");
      printf("reset generator\n");
      SPI_write(reset_spi_buffer,4);
sleep(1);

      printf("PIN_HM:%d\n",digitalRead(PIN_HM));
      printf("PIN_RG:%d\n",digitalRead(PIN_RG));

      printf("set expected A\n");
      SPI_write(set_exp_a_spi_buffer,4);
      memcpy(spi_buffer,&md5_hash[0],4);
      SPI_write(spi_buffer,4);
sleep(1);
      printf("set expected B\n");
      SPI_write(set_exp_b_spi_buffer,4);
      memcpy(spi_buffer,&md5_hash[4],4);
      SPI_write(spi_buffer,4);
sleep(1);
      printf("set expected C\n");
      SPI_write(set_exp_c_spi_buffer,4);
      memcpy(spi_buffer,&md5_hash[8],4);
      SPI_write(spi_buffer,4);
sleep(1);
      printf("set expected D\n");
      SPI_write(set_exp_d_spi_buffer,4);
      memcpy(spi_buffer,&md5_hash[12],4);
      SPI_write(spi_buffer,4);
sleep(1);
      printf("start generator\n");
      SPI_write(start_spi_buffer,4);

      //loop and look for a response
      while(true)
      {
         //printf("check pins\n");

         if(digitalRead(PIN_HM))
         {
            printf("hash matched\n");
            printf("pull the plain text off\n");
            int cctr=0;
            int len = 4;
            unsigned char buffer[4];

            //when we right we don't get the reponse back on the initial one...
            memcpy(buffer,get_txt_char[0],4);
            SPI_write(buffer,4);
            //read this one
            memcpy(buffer,get_txt_char[1],4);
            SPI_write_and_read(buffer,&len);
            for(int re=0;re<len;re++)
            {
               plain_text[pt_ctr] = buffer[len-re-1];
               pt_ctr++;
            }
            //read this one
            memcpy(buffer,get_txt_char[2],4);
            SPI_write_and_read(buffer,&len);
            for(int re=0;re<len;re++)
            {
               plain_text[pt_ctr] = buffer[len-re-1];
               pt_ctr++;
            }
	    //read this one
	    memcpy(buffer,get_txt_char[3],4);
	    SPI_write_and_read(buffer,&len);
	    for(int re=0;re<len;re++)
	    {
               plain_text[pt_ctr] = buffer[len-re-1];
	       pt_ctr++;
	    }
            //nop but read this one too
            //read this one
            memcpy(buffer,nop,4);
            SPI_write_and_read(buffer,&len);
            for(int re=0;re<len;re++)
            {
               plain_text[pt_ctr] = buffer[len-re-1];
               pt_ctr++;
            }

            printf("print it back out\n");
            for(int re=0;re<pt_ctr;re++)
               printf("%02X",plain_text[re]);
            printf("\n");

	         printf("plain_text:%s\n",plain_text);
            break;
         }
         else
         {
            if(digitalRead(PIN_RG))
               printf("reset generator\n");            
            else
            {
/**
               //we should be running so
               //we can try and get the count
               unsigned char c_buffer[8];
               int len = 4;
               int buf_ctr = 0;
               memset(c_buffer,0x00,8);
               uint64_t counter = 0;
               printf("get count\n");
               memset(spi_buffer,0x00,4);
               memcpy(spi_buffer,get_cnt_high_spi_buffer,4);
               SPI_write(spi_buffer,4);
               memset(spi_buffer,0x00,4);
               memcpy(spi_buffer,get_cnt_low_spi_buffer,4);
               len = 4;
               SPI_write_and_read(spi_buffer,&len);
               for(int re = 0;re < len;re++)
               {
                  c_buffer[buf_ctr] = spi_buffer[len-re-1];
                  buf_ctr++;
               }

               memcpy(spi_buffer,nop,4);
               len = 4;
               SPI_write_and_read(spi_buffer,&len);
               for(int re = 0;re < len;re++)
               {
                  c_buffer[buf_ctr] = spi_buffer[len-re-1];
                  buf_ctr++;
               }
               counter = (uint64_t)(*(uint64_t*)&c_buffer[0]);
               printf("count:%d\n",counter);
**/
            }  
         }

           sleep(1);
      }
   }

   return 0;
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
    else if(s == 'A' || s == 'a')
        return 0xA;
    else if(s == 'B' || s == 'b')
        return 0xB;
    else if(s == 'C' || s == 'c')
        return 0xC;
    else if(s == 'D' || s == 'd')
        return 0xD;
    else if(s == 'E' || s == 'e')
        return 0xE;
    else if(s == 'F' || s == 'f')
        return 0xF;
    else
	return 0x0;
}
