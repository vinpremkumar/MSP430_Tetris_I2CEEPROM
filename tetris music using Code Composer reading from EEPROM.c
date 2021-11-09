#include<msp430g2553.h>
#include<I2CfunctionsforEEPROM.h>

#define     SlaveAddress   0x50
unsigned int address;

void sound(int a,int d)	    //a is freq,d is delay
{
int i;
for(i=0;i<=d*2;i++)
{
	if(a==0)0.
		TACCR0 = 0;
	if(a!=0)
	{
		TACCR0 = 0x7A12/a;
		TACCR1 = 0x7A12/a;
	}
}
}

int main(void)
{
  unsigned int i;

  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  TACTL = TAIE + MC_3 + ID_3 + TASSEL_2;
  TACCTL1= CCIE+CAP;
  P1DIR = 0X04;
  P1OUT &=0x00;
  _BIS_SR(LPM0_bits + GIE) ;		// Low power mode 0 + global interrupts
  
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
int k;
address = 0x0000;

if((P1IN & 0x01) == 0x00)
{
	for(k=0x0000; k<0x004A; k++)
	{
 
	 sound(EEPROM_RandomRead(k),EEPROM_RandomRead(k+0x4A) ;
	// EEPROM_RandomRead(k) Reads from address 0x0001 onwards
 
	}
}

 __bis_SR_register(LPM0 + GIE);
  __no_operation();
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void timerainterr ()
{
	if(TAIV)   // interrupt control register...all timer interrupts are defined here
	   P1OUT ^= 0x04;
}
