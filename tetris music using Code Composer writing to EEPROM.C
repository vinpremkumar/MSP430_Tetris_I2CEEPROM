#include<msp430g2553.h>
#include<I2CfunctionsforEEPROM.h>

//_____________take to external flash memory_______________
char unsigned music_note[] = {1100, 825, 870, 980, 870, 825, 735, 735, 870, 1100, 980, 870, 825, 825, 870, 980, 110, 870, 735, 735, 0, 980, 1165, 1470, 1470, 1310, 1165, 1100, 870, 1100, 1100, 980, 870, 825, 825, 870, 980, 1100, 870, 735, 735, 0, 870, 735, 690, 825, 1100, 870, 980, 825, 870, 1100, 1100, 825, 870, 980, 870, 825, 735, 735, 870, 1100, 980, 870, 825, 825, 870, 980, 1100, 870, 735, 735, 0}

char unsigned timeperiod[] = {500, 250, 250, 500, 250, 250, 500, 250, 250, 500,250, 250, 500, 250, 250, 500, 500, 500, 500, 1000, 250, 500, 250, 250, 250, 250, 250, 750, 250, 250, 250, 250, 250, 500, 250, 250, 500, 500, 500, 500, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 500, 250, 250, 500, 250, 250, 500, 250, 250, 500, 250, 250, 500, 250, 250, 500, 500, 500, 500, 1000, 1000}
//_____________take to external flash memory_______________

#define     SlaveAddress   0x50
unsigned int address;

int main(void)
{
  unsigned int i;

  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

  InitI2C(SlaveAddress);                    // Initialize I2C module

  EEPROM_ByteWrite(0x0000,0x12);
  EEPROM_AckPolling();                      // Wait for EEPROM write cycle
                                            // completion
  EEPROM_ByteWrite(0x0001,0x34);
  EEPROM_AckPolling();                      // Wait for EEPROM write cycle
                                            // completion
  EEPROM_ByteWrite(0x0002,0x56);
  EEPROM_AckPolling();                      // Wait for EEPROM write cycle
                                            // completion
  EEPROM_ByteWrite(0x0003,0x78);
  EEPROM_AckPolling();                      // Wait for EEPROM write cycle
                                            // completion
  EEPROM_ByteWrite(0x0004,0x9A);
  EEPROM_AckPolling();                      // Wait for EEPROM write cycle
                                            // completion
  EEPROM_ByteWrite(0x0005,0xBC);
  EEPROM_AckPolling();                      // Wait for EEPROM write cycle
                                            // completion

 address = 0x0000;                         // Set starting address at 0
  // Write a sequence of data array
  EEPROM_PageWrite(address , music_note , sizeof(music_note));
 address = 0x0000;                         // Set starting address at 0x004A
  // Write a sequence of data array
  EEPROM_PageWrite(address , timeperiod , sizeof(timeperiod));

 __bis_SR_register(LPM0 + GIE);		//Low power mode 0
  __no_operation();
}