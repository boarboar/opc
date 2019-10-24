/*
 2012 Copyright (c) Seeed Technology Inc.

 Authors: Albert.Miao & Loovee, 
 Visweswara R (with initializtion code from TFT vendor)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 ILI9341
 
*/
#ifndef TFTv2_h
#define TFTv2_h

#if defined(ARDUINO) && ARDUINO >= 100
#define SEEEDUINO
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
//#include <avr/pgmspace.h>

#include <SPI.h>

//Basic Colors
#define RED		0xf800
#define GREEN	0x07e0
#define BLUE	0x001f
#define BLACK	0x0000
#define YELLOW	0xffe0
#define WHITE	0xffff

//Other Colors
#define CYAN		0x07ff	
#define BRIGHT_RED	0xf810	
#define GRAY1		0x8410  
#define GRAY2		0x4208 

//TFT_RGB - take a traditional 8 bit per channel RGB colour, and turn it into 5/6/5 bit colour for the TFT
#define TFT_RGB(red, green, blue) (((((uint16_t)red>>3) & 0x001F)<<11)|((((uint16_t)green>>2) & 0x003F) << 5) | (((uint16_t)blue>>3) & 0x001F))

//TFT resolution 240*320
#define MIN_X	0
#define MIN_Y	0
#define MAX_X	239
#define MAX_Y	319

//!!!!!!!!!!! this should be fixed if flipXY

//SPI DEFS:
//2453
//MOSI P1_7
//MISO P1_6
//SCLK P1_5
//CS pin P2.0
//DC pin P2.5
//RST->VCC
//BL->VCC

//2452
//MOSI P1_7
//MISO P1_6
//SCLK P1_5
//CS pin P2.6  //XIN
//DC pin P2.7  //XOUT
//RST->VCC
//BL->VCC

#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0A
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF  0x20
#define ILI9341_INVON   0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON  0x29

#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_RAMRD   0x2E

#define ILI9341_PTLAR   0x30
#define ILI9341_MADCTL  0x36
#define ILI9341_PIXFMT  0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1
/*
#define ILI9341_PWCTR6  0xFC

*/


#define LCD_FLIP_X	0x01
#define LCD_FLIP_Y	0x02
#define LCD_SWITCH_XY   0x04
#define LCD_LANDSCAPE (LCD_SWITCH_XY+LCD_FLIP_Y)
#define LCD_PORTRAIT 0
#define LCD_ORIENTATION (LCD_FLIP_X+LCD_FLIP_Y+LCD_SWITCH_XY)
#define LCD_OPAQ        0x08        
#define LCD_TRANSP      0x00
#define LCD_BG          0x10        
#define LCD_FG          0x00

#define LCD_HORIZONTAL 0
#define LCD_VERTICAL 1

#ifndef INT8U
#define INT8U uint8_t
#endif
#ifndef INT16U
#define INT16U uint16_t
#endif
#ifndef INT16
#define INT16 int16_t
#endif

#define FONT_SPACE 6
#define FONT_Y 8
#define FONT_SZ 5


#define ILI9341_PTLAR   0x30
#define ILI9341_VSCRDEF 0x33
#define ILI9341_MADCTL  0x36
#define ILI9341_VSCRSADD 0x37
#define ILI9341_PIXFMT  0x3A

extern INT8U simpleFont[][FONT_SZ];

class TFT
{
public: enum Palette {
    P_BLACK  = 0,
    P_RED    = 1,
    P_GREEN  = 2,
    P_BLUE    = 3,
    P_YELLOW  = 4,
    P_WHITE   = 5,
    P_CYAN    = 6,
    };  
public:
        void TFTinit();
		void setOrientation(int value);
        void fillScreen(void);
		void fillScreen(INT16 XL,INT16 XR,INT16 YU,INT16 YD);
		void drawChar(INT8U ascii,INT16U poX, INT16U poY);
        void drawCharLowRAM(INT8U ascii,INT16U poX, INT16U poY);
        INT16U drawString(const char *string,INT16U poX, INT16U poY);
		inline void fillRectangle(INT16 poX, INT16 poY, INT16 length, INT16U width) { setFillColor(LCD_BG); fillScreen(poX, poX+length, poY, poY+width); }	
        void drawLineThick(INT16 x0,INT16 y0,INT16 x1,INT16 y1);
        void drawLineThickLowRAM(INT16 x0,INT16 y0,INT16 x1,INT16 y1);
        void drawLineThickLowRAM8Bit(INT16 x0,INT16 y0,INT16 x1,INT16 y1);
        inline void drawVerticalLine(INT16 poX, INT16 poY,INT16 length) { setFillColor(LCD_FG); fillScreen(poX,poX,poY,poY+length);}  
        inline void drawHorizontalLine(INT16 poX, INT16 poY,INT16 length) {setFillColor(LCD_FG); fillScreen(poX,poX+length,poY,poY);}   
        //void drawStraightDashedLine(INT8U dir, INT16 poX, INT16 poY, INT16U length);
        inline void drawRectangle(INT16 poX, INT16 poY, INT16U length,INT16U width) { \
          setFillColor(LCD_FG); \
          fillScreen(poX,poX+length,poY,poY); \
          fillScreen(poX,poX+length,poY+width,poY+width); \
          fillScreen(poX,poX,poY,poY+width); \
          fillScreen(poX+length,poX+length,poY,poY+width); \
        }	
        inline INT16U  getMinX() { return 0; }
        inline INT16U  getMinY() { return 0; }
        inline INT16U  getMaxX() { return _flags&LCD_SWITCH_XY ? MAX_Y : MAX_X; }
        inline INT16U  getMaxY() { return _flags&LCD_SWITCH_XY ? MAX_X : MAX_Y; }
        
        // Ctx
        inline void setColor(INT16U fgcolor) {_fgColorH = fgcolor>>8; _fgColorL=fgcolor&0xff;}
        inline void setFgColor(INT16U fgcolor) {_fgColorH = fgcolor>>8; _fgColorL=fgcolor&0xff;}	
        inline void setBgColor(INT16U bgcolor) {_bgColorH = bgcolor>>8; _bgColorL=bgcolor&0xff;}
        inline void setOpaq(INT8U opaq) {_flags&=~LCD_OPAQ; _flags|=LCD_OPAQ&opaq;} 
        inline void setFillColor(INT8U opaq) {_flags&=~LCD_BG; _flags|=LCD_BG&opaq;}
        inline void setSize(INT8U v) {_size_mask_thick=v;}
        inline void setMask(INT8U v) {_size_mask_thick=v;}
        inline void setThick(INT8U v) {_size_mask_thick=v;}
        inline INT8U getSize() { return _size_mask_thick; }
        
        void setupScrollArea(INT16U vsz, INT16U tfa, INT16U bfa);
        void scrollAddress(INT16U vsp);

protected:
        void WriteCmdSeq(const INT8U *data);
//        void setWindow(INT16U StartCol, INT16U EndCol, INT16U StartPage,INT16U EndPage);
	void sendCMD(INT8U index);
	void WRITE_DATA(INT8U data);
	void sendData(INT16U data);

       //Ctx 
        INT8U _flags;	
        INT8U _size_mask_thick;
        INT8U _fgColorL, _fgColorH, _bgColorL, _bgColorH;
        
};

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/


