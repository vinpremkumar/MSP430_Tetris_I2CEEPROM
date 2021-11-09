/******************************************************************************/
/*  Communication with an EEPROM (e.g. 2465) via I2C bus                      */
/*  The I2C module of the MSP430F169 is used to communicate with the EEPROM.  */
/*  The "Byte Write", "Page Write", "Current Address Read", "Random Read",    */
/*  "Sequential Read" and "Acknowledge Polling" commands or the EEPROM are    */
/*  realized.                                                                 */
/*                                                                            */
/*  developed with IAR Embedded Workbench V4.21.8                             */
/*                                                                            */
/*  Texas Instruments                                                         */
/*  William Goh                                                               */
/*  April 2009                                                                */
/*----------------------------------------------------------------------------*/
/*  updates                                                                   */
/*    Jan 2005:                                                               */
/*        - updated initialization sequence                                   */
/*    March 2009:                                                             */
/*        - updated code for 1xx USART module                                 */
/*        - added Page Write and Sequential Read functions                    */
/*******************************************************************************
;
; THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
; REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
; INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
; FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
; COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE.
; TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET
; POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY
; INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR
; YOUR USE OF THE PROGRAM.
;
; IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
; CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY
; THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED
; OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT
; OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM.
; EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF
; REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS
; OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF
; USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S
; AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF
; YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS
; (U.S.$500).
;
; Unless otherwise stated, the Program written and copyrighted
; by Texas Instruments is distributed as "freeware".  You may,
; only under TI's copyright in the Program, use and modify the
; Program without any charge or restriction.  You may
; distribute to third parties, provided that you transfer a
; copy of this license to the third party and the third party
; agrees to these terms by its first use of the Program. You
; must reproduce the copyright notice and any other legend of
; ownership on each copy or partial copy, of the Program.
;
; You acknowledge and agree that the Program contains
; copyrighted material, trade secrets and other TI proprietary
; information and is protected by copyright laws,
; international copyright treaties, and trade secret laws, as
; well as other intellectual property laws.  To protect TI's
; rights in the Program, you agree not to decompile, reverse
; engineer, disassemble or otherwise translate any object code
; versions of the Program to a human-readable form.  You agree
; that in no event will you alter, remove or destroy any
; copyright notice included in the Program.  TI reserves all
; rights not specifically granted under this license. Except
; as specifically provided herein, nothing in this agreement
; shall be construed as conferring by implication, estoppel,
; or otherwise, upon you, any license or other right under any
; TI patents, copyrights or trade secrets.
;
; You may not use the Program in non-TI devices.
;
*******************************************************************************/
#include "msp430x16x.h"

#define I2C_PORT_SEL  P3SEL
#define I2C_PORT_OUT  P3OUT
#define I2C_PORT_REN  P3REN
#define I2C_PORT_DIR  P3DIR
#define SDA_PIN       BIT1                  // SDA pin
#define SCL_PIN       BIT3                  // SCL pin

#define     MAXPAGEWRITE   64

int PtrTransmit;
volatile unsigned char I2CBufferArray[66];
volatile unsigned char I2CBuffer;

/*----------------------------------------------------------------------------*/
// Description:
//   Initialization of the I2C Module
/*----------------------------------------------------------------------------*/
void InitI2C(unsigned char eeprom_i2c_address)
{
  I2C_PORT_SEL |= SDA_PIN + SCL_PIN;        // Assign I2C pins to USART
  I2C_PORT_DIR &= ~(SDA_PIN + SCL_PIN);

  // Recommended initialisation steps of I2C module as shown in User Guide:
  U0CTL |= I2C+SYNC;                        // (1) Select I2C mode with SWRST=1
  U0CTL &= ~I2CEN;                          // (2) disable the I2C module
                                            // (3) Configure the I2C module with I2CEN=0 :
                                            // U0CTL default settings:
                                            // 7-bit addressing, no DMA, no feedback
  I2CTCTL = I2CTRX+I2CSSEL_2;               // byte mode, repeat mode, clock source = SMCLK,
                                            // transmit mode
  I2CSA = eeprom_i2c_address;               // Slave Address - defines the
                                            // control byte that is sent to the
                                            // EEPROM.
  I2COA = 0x01A5;                           // own address.
  I2CPSC = 0x00;                            // I2C clock = clock source/1
  I2CSCLH = 0x03;                           // SCL high period = 5*I2C clock
  I2CSCLL = 0x03;                           // SCL low period  = 5*I2C clock
  U0CTL |= I2CEN;                           // (4) set I2CEN via software

  if (I2CDCTL&I2CBUSY)                      // test if bus to be free
  {                                         // otherwise a manual Clock on is
                                            // generated
    I2C_PORT_SEL &= ~SCL_PIN;               // Select Port function for SCL
    I2C_PORT_OUT &= ~SCL_PIN;               //
    I2C_PORT_DIR |= SCL_PIN;                // drive SCL low
    I2C_PORT_SEL |= SDA_PIN + SCL_PIN;      // select module function for the
                                            // used I2C pins
  };
}

/*---------------------------------------------------------------------------*/
// Description:
//   Initialization of the I2C Module for Write operation.
/*---------------------------------------------------------------------------*/
void I2CWriteInit(void)
{
  U0CTL |= MST;                             // define Master Mode
  I2CTCTL |= I2CTRX;                        // I2CTRX=1 => Transmit Mode (R/W bit = 0)
  I2CIFG &= ~TXRDYIFG;
  I2CIE = TXRDYIE;                          // enable Transmit ready interrupt
}

/*---------------------------------------------------------------------------*/
// Description:
//   Initialization of the I2C Module for Read operation.
/*---------------------------------------------------------------------------*/
void I2CReadInit(void)
{
  I2CTCTL &= ~I2CTRX;                       // I2CTRX=0 => Receive Mode
                                            //   (R/W bit = 1)
  U0CTL |= MST;                             // define Master Mode
  I2CIFG &= ~RXRDYIFG;
  I2CIE &= ~TXRDYIE;                        // disable Transmit ready interrupt
  I2CIE |= RXRDYIE;                         // enable Receive ready interrupt
}

/*----------------------------------------------------------------------------*/
// Description:
//   Byte Write Operation. The communication via the I2C bus with an EEPROM
//   (2465) is realized. A data byte is written into a user defined address.
/*----------------------------------------------------------------------------*/
void EEPROM_ByteWrite(unsigned int Address, unsigned char Data)
{
  unsigned char adr_hi;
  unsigned char adr_lo;

  while (I2CDCTL&I2CBUSY);                  // wait until I2C module has
                                            // finished all operations

  adr_hi = Address >> 8;                    // calculate high byte
  adr_lo = Address & 0xFF;                  //   and low byte of address

  I2CBufferArray[2] = adr_hi;               // store single bytes that have to
  I2CBufferArray[1] = adr_lo;               // be sent in the I2CBufferArray.
  I2CBufferArray[0] = Data;
  PtrTransmit = 2;                          // set I2CBufferArray Pointer

  I2CWriteInit();
  I2CNDAT = 3;                              // 1 control byte + 3 bytes should
                                            //   be transmitted
  I2CTCTL |= I2CSTT;                        // start condition generation
                                            // => I2C communication is started
  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupts
  while(I2CTCTL & I2CSTP);                  // Ensure stop condition got sent
}

/*----------------------------------------------------------------------------*/
// Description:
//   Page Write Operation. The communication via the I2C bus with an EEPROM
//   (24xx65) is realized. A data byte is written into a user defined address.
/*----------------------------------------------------------------------------*/
void EEPROM_PageWrite(unsigned int StartAddress, unsigned char * Data, unsigned char Size)
{
  volatile unsigned int i = 0;
  volatile unsigned char counterI2cBuffer;
  unsigned char adr_hi;
  unsigned char adr_lo;
  unsigned int currentAddress = StartAddress;
  unsigned char currentSize = Size;
  unsigned char bufferPtr = 0;
  unsigned char moreDataToRead = 1;

  while (I2CDCTL&I2CBUSY);                  // wait until I2C module has
                                            // finished all operations.

  // Execute until no more data in Data buffer
  while(moreDataToRead)
  {
    adr_hi = currentAddress >> 8;           // calculate high byte
    adr_lo = currentAddress & 0xFF;         // and low byte of address

    // Chop data down to 64-byte packets to be transmitted at a time
    // Maintain pointer of current startaddress
    if(currentSize > MAXPAGEWRITE)
    {
      bufferPtr = bufferPtr + MAXPAGEWRITE;
      counterI2cBuffer = MAXPAGEWRITE - 1;
      PtrTransmit = MAXPAGEWRITE + 1;       // set I2CBufferArray Pointer
      I2CNDAT = MAXPAGEWRITE + 2;           // set I2CBufferArray Pointer
      currentSize = currentSize - MAXPAGEWRITE;
      currentAddress = currentAddress + MAXPAGEWRITE;

      // Get start address
      I2CBufferArray[MAXPAGEWRITE + 1] = adr_hi; // High byte address.
      I2CBufferArray[MAXPAGEWRITE] = adr_lo; // Low byte address.
    }
    else
    {
      bufferPtr = bufferPtr + currentSize;
      counterI2cBuffer = currentSize - 1;
      PtrTransmit = currentSize + 1;        // set I2CBufferArray Pointer.
      I2CNDAT = currentSize + 2;            // set I2CBufferArray Pointer.
      moreDataToRead = 0;
      currentAddress = currentAddress + currentSize;

      // Get start address
      I2CBufferArray[currentSize + 1] = adr_hi; // High byte address.
      I2CBufferArray[currentSize] = adr_lo; // Low byte address.
    }

    // Copy data to I2CBufferArray
    unsigned char temp;
    for(i ; i < bufferPtr ; i++)
    {
      temp = Data[i];                       // Required or else IAR throws a
                                            // warning [Pa082]
      I2CBufferArray[counterI2cBuffer] = temp;
      counterI2cBuffer--;
    }

    I2CWriteInit();
    I2CTCTL |= I2CSTT;                      // start condition generation
                                            // => I2C communication is started
    __bis_SR_register(LPM0_bits + GIE);     // Enter LPM0 w/ interrupts
    while(I2CTCTL & I2CSTP);                // Ensure stop condition got sent

    EEPROM_AckPolling();                    // Ensure data is written in EEPROM
  }
}
/*----------------------------------------------------------------------------*/
// Description:
//   Current Address Read Operation. Data is read from the EEPROM. The current
//   address from the EEPROM is used.
/*----------------------------------------------------------------------------*/
unsigned char EEPROM_CurrentAddressRead(void)
{
  while (I2CDCTL&I2CBUSY);                  // wait until I2C module has
                                            //   finished all operations
  I2CReadInit();
  I2CNDAT = 1;                              // 1 byte should be received
  I2CTCTL |= I2CSTT;                        // start receiving
                                            // re-start condition
  while(I2CTCTL & I2CSTT);                  // Start condition sent?
  I2CTCTL |= I2CSTP;                        // I2C stop condition
  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupts
  while(I2CTCTL & I2CSTP);                  // Ensure stop condition got sent

  return I2CBuffer;
}

/*----------------------------------------------------------------------------*/
// Description:
//   Random Read Operation. Data is read from the EEPROM. The EEPROM
//   address is defined with the parameter Address.
/*----------------------------------------------------------------------------*/
unsigned char EEPROM_RandomRead(unsigned int Address)
{
  unsigned char adr_hi;
  unsigned char adr_lo;

  while (I2CDCTL&I2CBUSY);                  // wait until I2C module has
                                            //   finished all operations

  adr_hi = Address >> 8;                    // calculate high byte
  adr_lo = Address & 0xFF;                  //   and low byte of address

  I2CBufferArray[1] = adr_hi;               // store single bytes that have to
  I2CBufferArray[0] = adr_lo;               // be sent in the I2CBufferArray.
  PtrTransmit = 1;                          // set I2CBufferArray Pointer

  I2CWriteInit();
  I2CNDAT = 2;                              // 1 control byte + 2 bytes should
                                            //   be transmitted
  I2CTCTL |= I2CSTT;                        // start condition generation
                                            // => I2C communication is started
  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupts
  while(I2CTCTL & I2CSTP);                  // Ensure stop condition got sent

  I2CReadInit();
  I2CNDAT = 1;                              // 1 byte should be received
  I2CTCTL |= I2CSTT;                        // start receiving
                                            // re-start condition
  while(I2CTCTL & I2CSTT);                  // Start condition sent?
  I2CTCTL |= I2CSTP;                        // I2C stop condition
  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupts
  while(I2CTCTL & I2CSTP);                  // Ensure stop condition got sent

  return I2CBuffer;
}

/*----------------------------------------------------------------------------*/
// Description:
//   Sequential Read Operation. Data is read from the EEPROM in a sequential
//   form from the parameter address as a starting point. Specify the size to
//   be read and populate to a Data buffer.
/*----------------------------------------------------------------------------*/
void EEPROM_SequentialRead(unsigned int Address , unsigned char * Data , unsigned int Size)
{
  unsigned char adr_hi;
  unsigned char adr_lo;
  unsigned int counterSize;

  while (I2CDCTL&I2CBUSY);                  // wait until I2C module has
                                            // finished all operations.

  adr_hi = Address >> 8;                    // calculate high byte
  adr_lo = Address & 0xFF;                  // and low byte of address

  I2CBufferArray[1] = adr_hi;               // store single bytes that have to
  I2CBufferArray[0] = adr_lo;               // be sent in the I2CBuffer.
  PtrTransmit = 1;                          // set I2CBufferArray Pointer

  // Write Address first
  I2CWriteInit();
  I2CNDAT = 2;                              // 1 control byte + 2 bytes should
                                            //   be transmitted
  I2CTCTL |= I2CSTT;                        // start condition generation
                                            // => I2C communication is started
  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupts
  while(I2CTCTL & I2CSTP);                  // Ensure stop condition got sent

  // Read Data byte
  I2CReadInit();
  U0CTL &= ~I2CEN;                          // clear I2CEN bit => necessary to
                                            //   re-configure I2C module
  I2CTCTL |= I2CRM;                         // transmission is software
                                            //   controlled
  U0CTL |= I2CEN;                           // enable I2C module

  I2CTCTL |= I2CSTT;                        // I2C start condition
  while(I2CTCTL & I2CSTT);                  // Start condition sent?

  for(counterSize = 0 ; counterSize < Size ; counterSize++)
  {
    __bis_SR_register(LPM0_bits + GIE);     // Enter LPM0 w/ interrupts
    Data[counterSize] = I2CBuffer;
  }

  I2CTCTL |= I2CSTP;                        // I2C stop condition
  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupts
  while(I2CTCTL & I2CSTP);                  // Ensure stop condition got sent
  U0CTL &= ~I2CEN;                          // clear I2CEN bit => necessary to
                                            //  re-configure I2C module
  I2CTCTL &= ~I2CRM;                        // transmission is by the I2C module
  U0CTL |= I2CEN;                           // enable I2C module
}

/*----------------------------------------------------------------------------*/
// Description:
//   Acknowledge Polling. The EEPROM will not acknowledge if a write cycle is
//   in progress. It can be used to determine when a write cycle is completed.
/*----------------------------------------------------------------------------*/
void EEPROM_AckPolling(void)
{
  while (I2CDCTL&I2CBUSY);                  // wait until I2C module has
                                            //   finished all operations
  U0CTL &= ~I2CEN;                          // clear I2CEN bit => necessary to
                                            //   re-configure I2C module
  I2CTCTL |= I2CRM;                         // transmission is software
                                            //   controlled
  U0CTL |= I2CEN;                           // enable I2C module
  I2CIFG = NACKIFG;                         // set NACKIFG
  while (NACKIFG & I2CIFG)
  {
    I2CIFG=0x00;                            // clear I2C interrupt flags
    U0CTL |= MST;                           // define Master Mode
    I2CTCTL |= I2CTRX;                      // I2CTRX=1 => Transmit Mode
                                            //   (R/W bit = 0)
    I2CTCTL |= I2CSTT;                      // start condition is generated
    while (I2CTCTL&I2CSTT);                 // wait till I2CSTT bit was cleared
    I2CTCTL |= I2CSTP;                      // stop condition is generated after
                                            //   slave address was sent => I2C
                                            //   communication is started
    while (I2CDCTL&I2CBUSY);                // wait till stop bit is reset
    __delay_cycles(500);                    // Software delay
  }
  U0CTL &= ~I2CEN;                          // clear I2CEN bit => necessary to
                                            //   re-configure I2C module
  I2CTCTL &= ~I2CRM;                        // transmission is by the I2C module
  U0CTL |= I2CEN;                           // enable I2C module
}

/*---------------------------------------------------------------------------*/
/*  Interrupt Service Routines                                               */
/*     Note that the Compiler version is checked in the following code and   */
/*     depending of the Compiler Version the correct Interrupt Service       */
/*     Routine definition is used.                                           */
/*---------------------------------------------------------------------------*/
#if __VER__ < 200
    interrupt [USART0TX_VECTOR] void ISR_I2C(void)
#else
    #pragma vector=USART0TX_VECTOR
    __interrupt void ISR_I2C(void)
#endif
{
  switch (I2CIV)
  {
    case I2CIV_AL:      /* I2C interrupt vector: Arbitration lost (ALIFG) */
      break;
    case I2CIV_NACK:    /* I2C interrupt vector: No acknowledge (NACKIFG) */
      break;
    case I2CIV_OA:      /* I2C interrupt vector: Own address (OAIFG) */
      break;
    case I2CIV_ARDY:    /* I2C interrupt vector: Access ready (ARDYIFG) */
      break;
    case I2CIV_RXRDY:   /* I2C interrupt vector: Receive ready (RXRDYIFG) */
      I2CBuffer = I2CDRB;                   // store received data in buffer
      __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
      break;
    case I2CIV_TXRDY:   /* I2C interrupt vector: Transmit ready (TXRDYIFG) */
      while(!(I2CDCTL & I2CTXUDF));         // Checks if TX buff is empty
      I2CDRB = I2CBufferArray[PtrTransmit];
      PtrTransmit--;
      if (PtrTransmit < 0)
      {
        I2CTCTL |= I2CSTP;                  // I2C stop condition
        I2CIE &= ~TXRDYIE;                  // disable interrupts
        I2CIFG &= ~TXRDYIFG;                // Clear USCI_B0 TX int flag
        __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
      }
      break;
    case I2CIV_GC:      /* I2C interrupt vector: General call (GCIFG) */
      break;
    case I2CIV_STT:     /* I2C interrupt vector: Start condition (STTIFG) */
      break;
  }
}
