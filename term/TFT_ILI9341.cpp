/*
 ILI9341
*/


#include "TFT_ILI9341.h"
#include <SPI.h>
#include <msp430.h>
#include <Energia.h>


//#if defined(__MSP430_HAS_USCI_B0__) || defined(__MSP430_HAS_USCI_B1__) || defined(__MSP430_HAS_USCI__)
//#error "USCI"
//#define

#define _cs P2_6 
#define _dc P2_7 

/*
#define TFT_CS_LOW  {digitalWrite(_cs, LOW);}
#define TFT_CS_HIGH {digitalWrite(_cs, HIGH);}
#define TFT_DC_LOW  {digitalWrite(_dc, LOW);}
#define TFT_DC_HIGH {digitalWrite(_dc, HIGH);}
*/

#define TFT_CS_LOW  {P2OUT &= ~BIT6;}
#define TFT_CS_HIGH {P2OUT |= BIT6;}
#define TFT_DC_LOW  {P2OUT &= ~BIT7;}
#define TFT_DC_HIGH {P2OUT |= BIT7;}

#ifdef UC0IFG
/* redefine or older 2xx devices where the flags are in SFR */
#define UCB0IFG  UC0IFG   
#define UCRXIFG  UCB0RXIFG
#define UCTXIFG  UCB0TXIFG
#endif

inline void spi_transmit(const uint8_t _data) {
  	UCB0TXBUF = _data; // setting TXBUF clears the TXIFG flag

	//while (UCB0STAT & UCBUSY); // wait for SPI TX/RX to finish
        while (!(UCB0IFG & UCTXIFG));

	// clear RXIFG flag
        //UCB0IFG &= ~UCRXIFG;
}

//inline static void spi_transmit16_msb(const uint16_t data)
//{
//	/* Wait for previous tx to complete. */
//	//while (!(UCB0IFG & UCTXIFG));   // do we need it?
//	/* Setting TXBUF clears the TXIFG flag. */
//        UCB0TXBUF = data >> 8;	/* Wait for previous tx to complete. */
//	while (!(UCB0IFG & UCTXIFG));
//	/* Setting TXBUF clears the TXIFG flag. */	
//        UCB0TXBUF = data | 0xFF;
//	while (UCB0STAT & UCBUSY); // wait for SPI TX/RX to finish
//	// clear RXIFG flag
//	UCB0IFG &= ~UCRXIFG;
//}

const INT8U seq[]={
      4, 0xEF,    0x03,0x80,0x02, // try to uncomment
      4, 0xCF,    0x00,0x8B,0x30,
      5, 0xED,    0x67,0x03,0x12,0x81,
      4, 0xE8,    0x85,0x10,0x7A,
      6, 0xCB,    0x39,0x2C,0x00,0x34,0x02,
      2, 0xF7,    0x20,
      3, 0xEA,    0x00, 0x00,
      2, 0xC0,    0x1B, // Power control     VRH[5:0]              
      2, 0xC1,    0x10, // Power control    SAP[2:0];BT[3:0]            
      3, 0xC5,    0x3F, 0x3C,  // VCM control                  
      2, 0xC7,    0xB7,       // VCM control2                 
      2, 0x36,    0x08,        // Memory Access Control   portrait     
      2, 0x3A,    0x55,
      3, 0xB1,    0x00, 0x1B,
      3, 0xB6,    0x0A, 0xA2,    // Display Function Control     
      2, 0xF2,    0x00,          // 3Gamma Function Disable      
      2, 0x26,    0x01,          // Gamma curve selected      
      16, 0xE0,   0x0F,0x2A,0x28,0x08,0x0E,0x08,0x54,0xA9,0x43,0x0A,0x0F,0x00,0x00,0x00,0x00, // Set Gamma                    
      16, 0xE1,   0x00,0x15,0x17,0x07,0x11,0x06,0x2B,0x56,0x3C,0x05,0x10,0x0F,0x3F,0x3F,0x0F, // Set Gamma
      0      
    };


inline static void sendCMD(INT8U index)
{
    TFT_DC_LOW;
    TFT_CS_LOW;
    spi_transmit(index);
    TFT_CS_HIGH;
}

inline static void WRITE_DATA(INT8U data) 
{
    TFT_DC_HIGH;
    TFT_CS_LOW;
    spi_transmit(data);
    TFT_CS_HIGH;
}

inline static void sendData(INT16U data)
{
    TFT_DC_HIGH;
    TFT_CS_LOW;
    spi_transmit(data>>8);
    spi_transmit(data&0xff);
    //spi_transmit16_msb(data);
    TFT_CS_HIGH;
    
}

void TFT::WriteCmdSeq(const INT8U *data)
{
  while(*data) {
    INT8U data_len1=(*data++);
    sendCMD(*data++);
    while(--data_len1) WRITE_DATA(*data++);    
  }
}

void TFT::TFTinit (/*INT8U cs, INT8U dc*/)
{
    _flags = 0;
	
     // disable XTAL to use pins P2_6/7
    P2SEL &= ~BIT6; //XIN
    P2SEL &= ~BIT7; //XOUT
 
    pinMode(_cs,OUTPUT);
    pinMode(_dc,OUTPUT);
	
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2);
	
    delay(5);
	
    WriteCmdSeq(seq);

    sendCMD(0x11);                                                      /* Exit Sleep                   */
    delay(120);
    sendCMD(0x29);                                                      /* Display on                   */
    fillScreen();
}

void TFT::setOrientation(int flags) 
{
  INT8U madctl = 0x08;
  if(flags & LCD_FLIP_X) madctl &= ~(1 << 6);
  if(flags & LCD_FLIP_Y) madctl |= 1 << 7;
  if(flags & LCD_SWITCH_XY) madctl |= 1 << 5;
  sendCMD(0x36);
  WRITE_DATA(madctl);
  _flags &= ~LCD_ORIENTATION;
  _flags |= flags&LCD_ORIENTATION;
}

void TFT::fillScreen(void)
{
   setBgColor(BLACK);
   setFillColor(LCD_BG);
   fillScreen(0, getMaxX(), 0, getMaxY());
}

void TFT::fillScreen(INT16 XL, INT16 XR, INT16 YU, INT16 YD)
{
    if(XL<0) XL=0;
    if(XR<XL) XR=XL;
    
    sendCMD(0x2A);                                                      
    sendData(XL); 
    sendData(XR);
    sendCMD(0x2B);                                                      
    sendData(YU);
    sendData(YD);
    sendCMD(0x2c);
    
    XR=XR-XL+1;
    YD=YD-YU+1;
    
    TFT_DC_HIGH;
    TFT_CS_LOW;
    
    if(_flags&LCD_BG) {
      while(YD--) {
        XL=XR;
        while(XL--) {
          spi_transmit(_bgColorH);
          spi_transmit(_bgColorL);
        }
      }      
    } else {
      while(YD--) {
        XL=XR;
        while(XL--) {
          spi_transmit(_fgColorH);
          spi_transmit(_fgColorL);        
          }
      }
    }
}

// ##############################################################################################
// Setup a portion of the screen for vertical scrolling
// ##############################################################################################
// We are using a hardware feature of the display, so we can only scroll in portrait orientation
void TFT::setupScrollArea(INT16U vsz, INT16U tfa, INT16U bfa) {
  vsz-=(tfa+bfa);
  sendCMD(ILI9341_VSCRDEF); // Vertical scroll definition
  sendData(tfa);
  sendData(vsz);
  sendData(bfa);
}

// ##############################################################################################
// Setup the vertical scrolling start address pointer
// ##############################################################################################
void TFT::scrollAddress(INT16U vsp) {
  sendCMD(ILI9341_VSCRSADD); // Vertical scrolling pointer
  sendData(vsp);
}


void TFT::drawCharLowRAM( INT8U ascii, INT16U poX, INT16U poY)
{   
    if(poY<0 || poX<0) return;
    if((ascii<32)||(ascii>129)) ascii = '?';
    //INT16U y; //!!!
    INT8U i, temp;
    INT8U nb, f;
    
    for (i=0; i<FONT_SZ; i++, poX+=_size_mask_thick ) {
        temp = simpleFont[ascii-0x20][i];
        if(!temp) continue;
  /*      
        y=poY;        
        for(f=0;f<8;f++, y+=_size_mask_thick)
        {
            if((temp>>f)&0x01) // if bit is set in the font mask
            {
                sendCMD(0x2A); sendData(poX); sendData(poX+_size_mask_thick-1);
                sendCMD(0x2B); sendData(y); sendData(y+_size_mask_thick-1);
                sendCMD(0x2c);
                TFT_DC_HIGH;
                TFT_CS_LOW;
                nb=(_size_mask_thick)*(_size_mask_thick);
                while(nb--) {
                  spi_transmit(_fgColorH);
                  spi_transmit(_fgColorL);
                }
            }
        }
        
*/        
                    // todo - try to glue continuois pixels together
        INT8U from=0x10;
        for(f=0;f<8;f++)
        {
            if((temp>>f)&0x01) // if bit is set in the font mask
            {
              if(from==0x10) { // new line
                from = f;
              }
              else { ; } // continue line
            } // if bit is set in the font mask
            else { // if bit is not set in the font mask
              if(from==0x10) { // continue empty
                ;
              }
              else { // stop line              
                sendCMD(0x2A); sendData(poX); sendData(poX+_size_mask_thick-1);
                sendCMD(0x2B); sendData(poY+from*_size_mask_thick); sendData(poY+f*_size_mask_thick-1);
                sendCMD(0x2c);
                TFT_DC_HIGH;
                TFT_CS_LOW;
                nb=(_size_mask_thick*(f-from))*(_size_mask_thick);
                while(nb--) {
                  spi_transmit(_fgColorH);
                  spi_transmit(_fgColorL);
                }
                from=0x10;
              } // stop line
            } // if bit is not set
        } //for y
        
        if(from!=0x10) { // unclosed line
              sendCMD(0x2A); sendData(poX); sendData(poX+_size_mask_thick-1);
              sendCMD(0x2B); sendData(poY+from*_size_mask_thick); sendData(poY+f*_size_mask_thick-1);
              sendCMD(0x2c);
              TFT_DC_HIGH;
              TFT_CS_LOW;
              nb=(_size_mask_thick*(f-from))*(_size_mask_thick);
              while(nb--) {
                spi_transmit(_fgColorH);
                spi_transmit(_fgColorL);
              }
         }
          
    } // for x
}

