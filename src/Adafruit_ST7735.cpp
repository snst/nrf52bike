/***************************************************
  This is a library for the Adafruit 1.8" SPI display.
  This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
  as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "mbed.h"
#include "Adafruit_ST7735.h"

inline uint16_t swapcolor(uint16_t x)
{
  return (x << 11) | (x & 0x07E0) | (x >> 11);
}

// Constructor
Adafruit_ST7735::Adafruit_ST7735(PinName mosi, PinName miso, PinName sck, PinName cs, PinName rs, PinName rst)
    : lcdPort(mosi, miso, sck), _cs(cs), _rs(rs), _rst(rst), GFX(ST7735_TFTWIDTH_80, ST7735_TFTHEIGHT_160)
{
}

void Adafruit_ST7735::writecommand(uint8_t c)
{
  _rs = 0;
  _cs = 0;
  lcdPort.write(c);
  _cs = 1;
}

void Adafruit_ST7735::writedata(uint8_t c)
{
  _rs = 1;
  _cs = 0;
  lcdPort.write(c);

  _cs = 1;
}

void Adafruit_ST7735::writedata32(uint32_t c)
{
  _rs = 1;
  _cs = 0;
  lcdPort.write(c);

  _cs = 1;
}

// SCREEN INITIALIZATION ***************************************************

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80
static const uint8_t
    Bcmd[] = {                            // Init commands for 7735B screens
        18,                               // 18 commands in list:
        ST77XX_SWRESET, ST_CMD_DELAY,     //  1: Software reset, no args, w/delay
        50,                               //     50 ms delay
        ST77XX_SLPOUT, ST_CMD_DELAY,      //  2: Out of sleep mode, no args, w/delay
        255,                              //     255 = max (500 ms) delay
        ST77XX_COLMOD, 1 + ST_CMD_DELAY,  //  3: Set color mode, 1 arg + delay:
        0x05,                             //     16-bit color
        10,                               //     10 ms delay
        ST7735_FRMCTR1, 3 + ST_CMD_DELAY, //  4: Frame rate control, 3 args + delay:
        0x00,                             //     fastest refresh
        0x06,                             //     6 lines front porch
        0x03,                             //     3 lines back porch
        10,                               //     10 ms delay
        ST77XX_MADCTL, 1,                 //  5: Mem access ctl (directions), 1 arg:
        0x08,                             //     Row/col addr, bottom-top refresh
        ST7735_DISSET5, 2,                //  6: Display settings #5, 2 args:
        0x15,                             //     1 clk cycle nonoverlap, 2 cycle gate
                                          //     rise, 3 cycle osc equalize
        0x02,                             //     Fix on VTL
        ST7735_INVCTR, 1,                 //  7: Display inversion control, 1 arg:
        0x0,                              //     Line inversion
        ST7735_PWCTR1, 2 + ST_CMD_DELAY,  //  8: Power control, 2 args + delay:
        0x02,                             //     GVDD = 4.7V
        0x70,                             //     1.0uA
        10,                               //     10 ms delay
        ST7735_PWCTR2, 1,                 //  9: Power control, 1 arg, no delay:
        0x05,                             //     VGH = 14.7V, VGL = -7.35V
        ST7735_PWCTR3, 2,                 // 10: Power control, 2 args, no delay:
        0x01,                             //     Opamp current small
        0x02,                             //     Boost frequency
        ST7735_VMCTR1, 2 + ST_CMD_DELAY,  // 11: Power control, 2 args + delay:
        0x3C,                             //     VCOMH = 4V
        0x38,                             //     VCOML = -1.1V
        10,                               //     10 ms delay
        ST7735_PWCTR6, 2,                 // 12: Power control, 2 args, no delay:
        0x11, 0x15,
        ST7735_GMCTRP1, 16, // 13: Gamma settings, I think?
        0x09, 0x16, 0x09, 0x20,
        0x21, 0x1B, 0x13, 0x19,
        0x17, 0x15, 0x1E, 0x2B,
        0x04, 0x05, 0x02, 0x0E,
        ST7735_GMCTRN1, 16 + ST_CMD_DELAY, // 14: More gamma settings?
        0x0B, 0x14, 0x08, 0x1E,
        0x22, 0x1D, 0x18, 0x1E,
        0x1B, 0x1A, 0x24, 0x2B,
        0x06, 0x06, 0x02, 0x0F,
        10,                          //     10 ms delay
        ST77XX_CASET, 4,             // 15: Column addr set, 4 args, no delay:
        0x00, 0x02,                  //     XSTART = 2
        0x00, 0x81,                  //     XEND = 129
        ST77XX_RASET, 4,             // 16: Row addr set, 4 args, no delay:
        0x00, 0x02,                  //     XSTART = 1
        0x00, 0x81,                  //     XEND = 160
        ST77XX_NORON, ST_CMD_DELAY,  // 17: Normal display on, no args, w/delay
        10,                          //     10 ms delay
        ST77XX_DISPON, ST_CMD_DELAY, // 18: Main screen turn on, no args, delay
        255},                        //     255 = max (500 ms) delay

    Rcmd1[] = {                       // 7735R init, part 1 (red or green tab)
        15,                           // 15 commands in list:
        ST77XX_SWRESET, ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
        150,                          //     150 ms delay
        ST77XX_SLPOUT, ST_CMD_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
        255,                          //     500 ms delay
        ST7735_FRMCTR1, 3,            //  3: Framerate ctrl - normal mode, 3 arg:
        0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR2, 3,            //  4: Framerate ctrl - idle mode, 3 args:
        0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR3, 6,            //  5: Framerate - partial mode, 6 args:
        0x01, 0x2C, 0x2D,             //     Dot inversion mode
        0x01, 0x2C, 0x2D,             //     Line inversion mode
        ST7735_INVCTR, 1,             //  6: Display inversion ctrl, 1 arg:
        0x07,                         //     No inversion
        ST7735_PWCTR1, 3,             //  7: Power control, 3 args, no delay:
        0xA2,
        0x02,                         //     -4.6V
        0x84,                         //     AUTO mode
        ST7735_PWCTR2, 1,             //  8: Power control, 1 arg, no delay:
        0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
        ST7735_PWCTR3, 2,             //  9: Power control, 2 args, no delay:
        0x0A,                         //     Opamp current small
        0x00,                         //     Boost frequency
        ST7735_PWCTR4, 2,             // 10: Power control, 2 args, no delay:
        0x8A,                         //     BCLK/2,
        0x2A,                         //     opamp current small & medium low
        ST7735_PWCTR5, 2,             // 11: Power control, 2 args, no delay:
        0x8A, 0xEE, ST7735_VMCTR1, 1, // 12: Power control, 1 arg, no delay:
        0x0E, ST77XX_INVOFF, 0,       // 13: Don't invert display, no args
        ST77XX_MADCTL, 1,             // 14: Mem access ctl (directions), 1 arg:
        0xC8,                         //     row/col addr, bottom-top refresh
        ST77XX_COLMOD, 1,             // 15: set color mode, 1 arg, no delay:
        0x05},                        //     16-bit color

    Rcmd2green[] = {        // 7735R init, part 2 (green tab only)
        2,                  //  2 commands in list:
        ST77XX_CASET, 4,    //  1: Column addr set, 4 args, no delay:
        0x00, 0x02,         //     XSTART = 0
        0x00, 0x7F + 0x02,  //     XEND = 127
        ST77XX_RASET, 4,    //  2: Row addr set, 4 args, no delay:
        0x00, 0x01,         //     XSTART = 0
        0x00, 0x9F + 0x01}, //     XEND = 159

    Rcmd2red[] = {       // 7735R init, part 2 (red tab only)
        2,               //  2 commands in list:
        ST77XX_CASET, 4, //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,      //     XSTART = 0
        0x00, 0x7F,      //     XEND = 127
        ST77XX_RASET, 4, //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,      //     XSTART = 0
        0x00, 0x9F},     //     XEND = 159

    Rcmd2green144[] = {  // 7735R init, part 2 (green 1.44 tab)
        2,               //  2 commands in list:
        ST77XX_CASET, 4, //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,      //     XSTART = 0
        0x00, 0x7F,      //     XEND = 127
        ST77XX_RASET, 4, //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,      //     XSTART = 0
        0x00, 0x7F},     //     XEND = 127

    Rcmd2green160x80[] = { // 7735R init, part 2 (mini 160x80)
        2,                 //  2 commands in list:
        ST77XX_CASET, 4,   //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,        //     XSTART = 0
        0x00, 0x7F,        //     XEND = 79
        ST77XX_RASET, 4,   //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,        //     XSTART = 0
        0x00, 0x9F},       //     XEND = 159

    Rcmd3[] = {                                                                                                                     // 7735R init, part 3 (red or green tab)
        4,                                                                                                                          //  4 commands in list:
        ST7735_GMCTRP1, 16,                                                                                                         //  1: Gamma setting, I think?
        0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10, ST7735_GMCTRN1, 16,         //  2: More gamma?
        0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10, ST77XX_NORON, ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
        10,                                                                                                                         //     10 ms delay
        ST77XX_DISPON, ST_CMD_DELAY,                                                                                                //  4: Main screen turn on, no args w/delay
        100};                                                                                                                       //     100 ms delay

/**************************************************************************/
/*!
    @brief  Companion code to the initiliazation tables. Reads and issues
            a series of LCD commands stored in PROGMEM byte array.
    @param  addr  Flash memory array with commands and data to send
*/
/**************************************************************************/
void Adafruit_ST7735::displayInit(const uint8_t *addr)
{

  uint8_t numCommands, numArgs;
  uint16_t ms;

  numCommands = *addr++; // Number of commands to follow
  while (numCommands--)
  {                        // For each command...
    writecommand(*addr++); //   Read, issue command
    numArgs = *addr++;     //   Number of args to follow
    ms = numArgs & DELAY;  //   If hibit set, delay follows args
    numArgs &= ~DELAY;     //   Mask out delay bit
    while (numArgs--)
    {                     //   For each argument...
      writedata(*addr++); //     Read, issue argument
    }

    if (ms)
    {
      ms = *addr++; // Read post-command delay time (ms)
      if (ms == 255)
        ms = 500; // If 255, delay for 500 ms
      wait_ms(ms);
    }
  }
}

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST77XX displays
    @param  cmdList  Flash memory array with commands and data to send
*/
/**************************************************************************/
void Adafruit_ST7735::commonInit(const uint8_t *cmdList)
{
  _rs = 1;
  _cs = 1;

  // use default SPI format
  lcdPort.format(8, 0);
  lcdPort.frequency(4000000); // Lets try 4MHz

  // toggle RST low to reset; CS low so it'll listen to us
  _cs = 0;
  _rst = 1;
  wait_ms(500);
  _rst = 0;
  wait_ms(500);
  _rst = 1;
  wait_ms(500);

  if (cmdList)
    displayInit(cmdList);
}

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST7735B displays
*/
/**************************************************************************/
void Adafruit_ST7735::initB(void)
{
  commonInit(Bcmd);
  setRotation(0);
}

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST7735R displays
    @param  options  Tab color from adafruit purchase
*/
/**************************************************************************/
void Adafruit_ST7735::initR(uint8_t options)
{
  commonInit(Rcmd1);
  if (options == INITR_GREENTAB)
  {
    displayInit(Rcmd2green);
    _colstart = 2;
    _rowstart = 1;
  }
  else if ((options == INITR_144GREENTAB) || (options == INITR_HALLOWING))
  {
    _height = ST7735_TFTHEIGHT_128;
    _width = ST7735_TFTWIDTH_128;
    displayInit(Rcmd2green144);
    _colstart = 2;
    _rowstart = 3; // For default rotation 0
  }
  else if (options == INITR_MINI160x80)
  {
    _height = ST7735_TFTHEIGHT_160;
    _width = ST7735_TFTWIDTH_80;
    displayInit(Rcmd2green160x80);
    _colstart = 24;
    _rowstart = 0;
  }
  else
  {
    // colstart, rowstart left at default '0' values
    displayInit(Rcmd2red);
  }
  displayInit(Rcmd3);

  // Black tab, change MADCTL color filter
  if ((options == INITR_BLACKTAB) || (options == INITR_MINI160x80))
  {
    writecommand(ST77XX_MADCTL);
    writedata(0xC0);
  }

  if (options == INITR_HALLOWING)
  {
    // Hallowing is simply a 1.44" green tab upside-down:
    tabcolor = INITR_144GREENTAB;
    setRotation(2);
  }
  else
  {
    tabcolor = options;
    setRotation(0);
  }
}

void Adafruit_ST7735::setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
                                    uint16_t h)
{
  x += _xstart;
  y += _ystart;

  writecommand(ST77XX_CASET); // Column addr set
  writedata(x >> 8);
  writedata(x & 0xFF); // XSTART
  writedata((x + w - 1) >> 8);
  writedata((x + w - 1) & 0xFF); // XEND

  writecommand(ST77XX_RASET); // Row addr set
  writedata(y >> 8);
  writedata(y & 0xFF); // YSTART
  writedata((y + h - 1) >> 8);
  writedata((y + h - 1) & 0xFF); // YEND

  writecommand(ST77XX_RAMWR); // write to RAM
}

void Adafruit_ST7735::pushColor(uint16_t color)
{
  _rs = 1;
  _cs = 0;

  lcdPort.write(color >> 8);
  lcdPort.write(color);
  _cs = 1;
}

void Adafruit_ST7735::drawPixel(int16_t x, int16_t y, uint16_t color)
{

  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
    return;

  setAddrWindow(x, y, x + 1, y + 1);

  _rs = 1;
  _cs = 0;

  lcdPort.write(color >> 8);
  lcdPort.write(color);

  _cs = 1;
}

void Adafruit_ST7735::drawFastVLine(int16_t x, int16_t y, int16_t h,
                                    uint16_t color)
{

  // Rudimentary clipping
  if ((x >= _width) || (y >= _height))
    return;
  if ((y + h - 1) >= _height)
    h = _height - y;
  setAddrWindow(x, y, x, y + h - 1);

  uint8_t hi = color >> 8, lo = color;
  _rs = 1;
  _cs = 0;
  while (h--)
  {
    lcdPort.write(hi);
    lcdPort.write(lo);
  }
  _cs = 1;
}

void Adafruit_ST7735::drawFastHLine(int16_t x, int16_t y, int16_t w,
                                    uint16_t color)
{

  // Rudimentary clipping
  if ((x >= _width) || (y >= _height))
    return;
  if ((x + w - 1) >= _width)
    w = _width - x;
  setAddrWindow(x, y, x + w - 1, y);

  uint8_t hi = color >> 8, lo = color;
  _rs = 1;
  _cs = 0;
  while (w--)
  {
    lcdPort.write(hi);
    lcdPort.write(lo);
  }
  _cs = 1;
}

void Adafruit_ST7735::fillScreen(uint16_t color)
{
  fillRect(0, 0, _width, _height, color);
}

void Adafruit_ST7735::drawBuffer(int16_t x, int16_t y, uint8_t* buf, int16_t w)
{
  setAddrWindow(x, y, x + w - 1, y);
  _rs = 1;
  _cs = 0;
  lcdPort.write((const char*)buf, 2*w, NULL, 0);
  _cs = 1;
}

// fill a rectangle
void Adafruit_ST7735::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color)
{

  // rudimentary clipping (drawChar w/big text requires this)
  if ((x >= _width) || (y >= _height))
    return;
  if ((x + w - 1) >= _width)
    w = _width - x;
  if ((y + h - 1) >= _height)
    h = _height - y;

  setAddrWindow(x, y, x + w, y + h);

  //uint8_t hi = color >> 8, lo = color;
  _rs = 1;
  _cs = 0;

  uint16_t *buf = (uint16_t *)malloc(2 * w);
  if (buf)
  {
    uint16_t col = color >> 8 | (color&0xFF) << 8;
    for(int16_t k=0; k<w; k++) {
      buf[k] = col;
    }
    //memset(buf, 0, 2*w);
    for (y = h; y > 0; y--) {
//      setAddrWindow(x, y, x + w, y + 1);
      lcdPort.write((const char*)buf, 2*w, NULL, 0);
    }
    /*
    for (y = h; y > 0; y--)
    {
      for (x = w; x > 0; x--)
      {
        lcdPort.write(hi);
        lcdPort.write(lo);
      }
    }*/
    free(buf);
  }

  _cs = 1;
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ST7735::Color565(uint8_t r, uint8_t g, uint8_t b)
{
  //    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  return ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3);
}

#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20
#define MADCTL_ML 0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH 0x04

// OTHER FUNCTIONS *********************************************************

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Adafruit_ST7735::setRotation(uint8_t m)
{
  uint8_t madctl = 0;

  rotation = m & 3; // can't be higher than 3

  // For ST7735 with GREEN TAB (including HalloWing)...
  if ((tabcolor == INITR_144GREENTAB) || (tabcolor == INITR_HALLOWING))
  {
    // ..._rowstart is 3 for rotations 0&1, 1 for rotations 2&3
    _rowstart = (rotation < 2) ? 3 : 1;
  }

  switch (rotation)
  {
  case 0:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80))
    {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    }
    else
    {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB)
    {
      _height = ST7735_TFTHEIGHT_128;
      _width = ST7735_TFTWIDTH_128;
    }
    else if (tabcolor == INITR_MINI160x80)
    {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_80;
    }
    else
    {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_128;
    }
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 1:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80))
    {
      madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    }
    else
    {
      madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB)
    {
      _width = ST7735_TFTHEIGHT_128;
      _height = ST7735_TFTWIDTH_128;
    }
    else if (tabcolor == INITR_MINI160x80)
    {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_80;
    }
    else
    {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_128;
    }
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  case 2:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80))
    {
      madctl = ST77XX_MADCTL_RGB;
    }
    else
    {
      madctl = ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB)
    {
      _height = ST7735_TFTHEIGHT_128;
      _width = ST7735_TFTWIDTH_128;
    }
    else if (tabcolor == INITR_MINI160x80)
    {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_80;
    }
    else
    {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_128;
    }
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 3:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80))
    {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    }
    else
    {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB)
    {
      _width = ST7735_TFTHEIGHT_128;
      _height = ST7735_TFTWIDTH_128;
    }
    else if (tabcolor == INITR_MINI160x80)
    {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_80;
    }
    else
    {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_128;
    }
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  }
  writecommand(ST77XX_MADCTL);
  writedata(madctl);
}
