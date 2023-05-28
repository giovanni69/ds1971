//  Maxim Dallas DS1971-F5 iButton Memory Filler
//  Fills iButton's EEPROM with a hardcoded byte. (default : FF)
//  Attach the 5V through 4.7k ohm resistor to pin 7, and connect GND.
//  Tested on Arduino Uno

#include <OneWire.h>
OneWire ds(7); //pin 7
bool buttonactive = false;
byte findAddr[8];
byte addr[8];
byte i;
byte data[512];
byte present = 0;
byte nextCmd = 0;

#define CMD_NULL 0x00
#define CMD_CHECK_DEVICE 0x02

void setup(void) {
memset(addr, 0, 8);
memset(data, 0, 512);
nextCmd = CMD_NULL;
Serial.begin(9600);
Serial.print("Ready, waiting for iButton...\n");
}

bool find()
{
ds.reset_search();
delay(250);
if (!ds.search(findAddr)) {
return false;
}
return true;
}

void info(byte addr[])
{
Serial.print("ADDR: ");
for (i = 0; i < 8; i++) {
Serial.print(addr[i], HEX);
if(i<7) Serial.print(":");
}
Serial.print("\n");
}

void sendCMD(int cmd)
{
present = ds.reset();
ds.select(addr);
ds.write(cmd,1); // Read the Scratchpad
delay(500);
memset(data, 0, 512);
}

bool memRead(byte type,byte pos=0x00, byte _size=0x01)
{
sendCMD(type);
ds.write(pos);
Serial.print("|");
for ( i = 0; i < _size; i++) {
data[i] = ds.read();
Serial.print(data[i], HEX);
Serial.print("|");
}
Serial.print("\n");
ds.reset();
delay(250);
return true;
}

bool status()
{
if (!find()) return true;
if (memcmp(findAddr, addr, 8) == 0) return false;
return true;
}

void ReadWrite()
{
 int i;
 ds.reset();
 ds.select(addr);
 ds.write(0x0F,1);  // Write in the Scratchpad
 ds.write(0x00,1);
 for ( i = 0; i < 32; i++) {
 ds.write(0xFF,1); // The byte we want to write to memory 
 }  
 delay(250);
 ds.reset();
 ds.select(addr);
 ds.write(0x55,1);  // Copy Scratchpad to Memory
 ds.write(0xA5,1);
 delay(250); 
 
}

void memoryVer()
{
ReadWrite();
Serial.print("-----------------------------------------------------------------------\n");
Serial.print("SCRATCHPAD: ");
memRead(0xaa,0,32); // Read the Scratchpad
Serial.print("MEMORY: ");
memRead(0xf0 ,0,32); // Read Memory
Serial.print("APPLICATION: ");
memRead(0xc3,0,8); // Read Application Registers
Serial.print("-----------------------------------------------------------------------\n");
}

void loop(void) {
if (!buttonactive)
{
nextCmd = CMD_NULL;

if (!find()) {
buttonactive = false;
Serial.print("Looking for...\n");
delay(500);
return;
}
memcpy(addr, findAddr, 8);
if ( OneWire::crc8( addr, 7) != addr[7]) {
buttonactive = false;
return;
};
Serial.print("Connected...\n");
buttonactive = true;
info(addr);
nextCmd = CMD_CHECK_DEVICE;
} 

switch(nextCmd)
{
case CMD_CHECK_DEVICE:
Serial.print("Please wait: taking a memory tissue...\n");
memoryVer();
nextCmd = CMD_NULL;
break;
}

if (status()) {
buttonactive = false;
Serial.print("iButton is not connected...\n");
return;
}
}
