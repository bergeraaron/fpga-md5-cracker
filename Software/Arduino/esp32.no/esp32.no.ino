#include <SPI.h>

uint8_t cmd[4];

uint8_t cmd_reset[4] = {0x52,0x30,0x00,0x00};
uint8_t cmd_start[4] = {0x52,0x30,0x00,0x01};
uint8_t cmd_setExpectedA[4] = {0x52,0x30,0x10,0x00};
uint8_t cmd_setExpectedB[4] = {0x52,0x30,0x10,0x01};
uint8_t cmd_setExpectedC[4] = {0x52,0x30,0x10,0x02};
uint8_t cmd_setExpectedD[4] = {0x52,0x30,0x10,0x03};
uint8_t cmd_setRange[4] = {0x52,0x30,0x20,0x00};
uint8_t cmd_GetCountLow[4] = {0x52,0x30,0x30,0x00};
uint8_t cmd_GetCountHigh[4] = {0x52,0x30,0x30,0x01};
uint8_t cmd_Nop[4] = {0x00,0x00,0x00,0x00};

uint64_t ReadCount = 0;

uint8_t payload[4];

uint8_t retval[4];

uint8_t md5hash[16] = {
  0x84,
  0xDF,
  0xB6,
  0x90,
  0x2D,
  0xA2,
  0x0F,
  0x8B,
  0xBE,
  0xFD,
  0x10,
  0x2E,
  0x73,
  0x50,
  0x9A,
  0xA0
  };


#define CSPIN  17

void setup(void)
{
  pinMode(CSPIN, OUTPUT);

  SPI.begin(18, 19, 23, 17); // sck, miso, mosi, ss (ss can be any GPIO)
  pinMode(CSPIN, OUTPUT);
  digitalWrite(CSPIN, HIGH);
  
  Serial.begin(115200);
  delay(4000);
  Serial.println("Serial enabled!");
/**
  Serial.print("Send a reset\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_reset,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  Serial.print("\n");
  delay(4000);

  //write the different expected hashes
  Serial.print("Send cmd_setExpectedA\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_setExpectedA,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  for(int i=0;i<4;i++)
    payload[i] = md5hash[i];
  digitalWrite(CSPIN, LOW);
  SPI.transfer(payload,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  Serial.print("\n");
  delay(4000);

  Serial.print("Send cmd_setExpectedB\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_setExpectedB,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  for(int i=0;i<4;i++)
    payload[i] = md5hash[4+i];
  digitalWrite(CSPIN, LOW);
  SPI.transfer(payload,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  Serial.print("\n");
  delay(4000);

  Serial.print("Send cmd_setExpectedC\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_setExpectedC,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  for(int i=0;i<4;i++)
    payload[i] = md5hash[8+i];
  digitalWrite(CSPIN, LOW);
  SPI.transfer(payload,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  Serial.print("\n");
  delay(4000);

  Serial.print("Send cmd_setExpectedD\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_setExpectedD,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  for(int i=0;i<4;i++)
    payload[i] = md5hash[12+i];
  digitalWrite(CSPIN, LOW);
  SPI.transfer(payload,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  Serial.print("\n");
  delay(4000);


  Serial.print("Send a start\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_start,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  Serial.print("\n");
/**/
}

void loop(void)
{
  Serial.print("main loop\n");
  delay(4000);
/**
  Serial.print("Send a reset\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_reset,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  Serial.print("\n");
  delay(10000);

  Serial.print("Send a start\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_start,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  Serial.print("\n");
  delay(10000);
**/
/**/
  Serial.print("Send a cmd_GetCountHigh\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_GetCountHigh,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);  
  Serial.print("\n");
  
  Serial.print("Send a cmd_GetCountLow\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_GetCountLow,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  Serial.print("\n");

  Serial.print("Send a cmd_Nop\n");
  digitalWrite(CSPIN, LOW);
  SPI.transfer(cmd_Nop,4);
  retval[0] = SPI.transfer(0);
  retval[1] = SPI.transfer(0);
  retval[2] = SPI.transfer(0);
  retval[3] = SPI.transfer(0);
  digitalWrite(CSPIN, HIGH);
  Serial.print(retval[0], HEX); Serial.print(" "); Serial.print(retval[1], HEX);  Serial.print(" ");  Serial.print(retval[2], HEX); Serial.print(" ");  Serial.println(retval[3], HEX);
  Serial.print("\n");

/**/
}
