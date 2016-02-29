/* -*- Mode:C++; c- le-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "arduino.h"
#include "FFTAnalyzerDisplay.h"

#if (USB_VID == 0x2341) && (USB_PID == 0x803C) TFT EsploraTFT(7, 0, 1);
#endif

FFTAnalyzerDisplay::FFTAnalyzerDisplay(uint8_t CS, uint8_t RS, uint8_t RST): Adafruit_ST7735(CS, RS, RST)
{
// as we already know the orientation (landscape, therefore rotated),
// set default width and height without need to call begin()  first.

_width = ST7735_TFTHEIGHT;
_height = ST7735_TFTWIDTH;
}

void FFTAnalyzerDisplay::begin()
{
static void* BEGIN_CONTEXT;
// initR(INITR_REDTAB);
initG();
setRotation(1);

context = &BEGIN_CONTEXT;
}

void FFTAnalyzerDisplay::drawMenu(const char *MENU_TITLE, const char *MENU_TEXT[],int indexLength, unsigned int index)
{
static void* DRAW_MENU_CONTEXT;

// Error check
if ( indexLength < 0 || indexLength > 4 ) return;

// if only Draw index Arrow
if ( (context == &DRAW_MENU_CONTEXT) && (lastDrawnMenu == MENU_TITLE))
{
setTextSize(1);
// Erase the Last drawn Arrow
stroke(0x , 0x , 0x );
text("->", 5, 30+20*lastDrawnMenuIndex);

// Draw New Arrow
stroke(0x33, 0x33, 0x33);
text("->", 5, 30+20*index);

// Rec Current State
lastDrawnMenu = MENU_TITLE;
lastDrawnMenuIndex = index;

return;
}

context = &DRAW_MENU_CONTEXT;

//---------------------------------------------------------------------------------------------
background(0x , 0x , 0x );

// Draw thema color
noStroke();
fill(0xb1, 0x63, 0xa3);
rect(0, 0, width(), 15);
rect(0, height() -3, width(), 3);

// Draw menu title
stroke(0x , 0x , 0x );
setTextSize(1);
text(MENU_TITLE, 5, 3);

// Draw menu strings
stroke(0x33, 0x33, 0x33);
setTextSize(1);

for (int i=0; i<indexLength; i++)
{
text(MENU_TEXT[i], 25, 30+20*i);
}

// Draw index Arrow
text("->", 5, 30+20*index);

// Version Info
text("v0.82b", 110, 110);

// Rec Current State
lastDrawnMenu = MENU_TITLE;
lastDrawnMenuIndex = index;

}
void FFTAnalyzerDisplay::drawFFTAnalyzer(uint8_t* fhtData, int dataLength)
{
static void* DRAW_FFT_ANALYZER;
const double FHT_DATA_MAX = 255.0;
const int DRAW_DATA_MAX = 120;
const char* MENU_TITLE = "FFT Analyzer";

// only draw graph area
if (context == &DRAW_FFT_ANALYZER)
{
if (dataLength < DRAW_DATA_MAX) return;

// Erase Graph Area stroke(0x88, 0x88, 0x88);
fill(0x , 0x , 0x );
rect(25, 30, 120, 80);

// Draw horizontal line
line(25, 90, 145, 90);
line(25, 70, 145, 70);
line(25, 50, 145, 50);

// Draw vartical line
line(115, 30, 115, 110);
line(85, 30, 85, 110);
line(55, 30, 55, 110);

// Plot Values
for (int i=1; i<DRAW_DATA_MAX; i++)
{
stroke(0x0, 0x , 0x0);
int tempPlotY = (int)(110 - (fhtData[i] / FHT_DATA_MAX ) * 80);
if (tempPlotY > 108) tempPlotY = 108;
if (tempPlotY < 30) tempPlotY = 31;
line(25 + i, 109, 25 + i, tempPlotY);
}
return;
}

context = &DRAW_FFT_ANALYZER;

//---------------------------------------------------------------------------------------------

background(0x , 0x , 0x );

// Draw thema color
noStroke();
fill(0xb1, 0x63, 0xa3);
rect(0, 0, width(), 15);
rect(0, height() - 3, width(), 3);

// Draw menu title
stroke(0x , 0x , 0x );
setTextSize(1);
text(MENU_TITLE, 5, 3);

// Draw Vertical Axis Num
stroke(0x55, 0x55, 0x55);
setTextSize(1);
text("100", 3, 25);
text("75", 10, 45);
text("50", 10, 65);
text("25", 10, 85);
text("0", 15, 105);

// Draw Horizontal Axis Num
stroke(0x55, 0x55, 0x55);
setTextSize(1);
text("0", 20, 113);
text("4.0", 40, 113);
text("8.0", 70, 113);
text("12.0", 100, 113);
text("16", 130, 113);
text("kHz", 143, 113);

// Call Recursive func To Draw Graph Area
drawFFTAnalyzer(fhtData, dataLength);
}

void FFTAnalyzerDisplay::drawMusicScaleAnalyzer(uint8_t* fhtData, int dataLength)
{
static void* DRAW_MUSIC_SCALE_ANALYZER;
const double FHT_DATA_MAX = 255.0;
const int DRAW_DATA_MAX = 120;
const char* MENU_TITLE = "Music Scale Analyzer";
static char drawData[5];

// only draw view area
if (context == &DRAW_MUSIC_SCALE_ANALYZER)
{
if (dataLength < DRAW_DATA_MAX) return;
// Erase Data Area
setTextSize(4);
stroke(0x , 0x , 0x );
text(drawData, 25, 30);

int max = 0;
for (int i=10; i< dataLength; i++)
{
if(fhtData[max] < fhtData[i]) max = i;
}

// Draw Data Area
dtostrf(max, 1, 1, drawData);
stroke(0x33, 0x33, 0x33);
text(drawData, 25, 30);
return;
}
context = &DRAW_MUSIC_SCALE_ANALYZER;

//--------------------------------------------------------------------------------------------- background(0x , 0x , 0x );
// Draw thema color
noStroke();
fill(0xb1, 0x63, 0xa3);
rect(0, 0, width(), 15);
rect(0, height() - 3, width(), 3);

// Draw menu title
stroke(0x , 0x , 0x );
setTextSize(1);
text(MENU_TITLE, 5, 3);

// Draw Vertical Axis Num
stroke(0x55, 0x55, 0x55);
setTextSize(2);
text("kHz", 120, 100);

// Call Recursive func To Draw Graph Area
drawMusicScaleAnalyzer(fhtData, dataLength);
}
void FFTAnalyzerDisplay::drawThermoHygrometer( oat *thermoData,  oat *hygroData, int data_length, bool thermoMode)
{
static void* DRAW_THERMO_HYGROMETER_CONTEXT;
const char* MENU_TITLE = "Thermo-Hygrometer";
float maxThermo, minThermo, aveThermo, maxHygro, minHygro;
float graphMinValue;

// Error check
if ( data_length > 120 ) return;

// Only neccesary draw graph area
if (context == &DRAW_THERMO_HYGROMETER_CONTEXT)
{
// Data check
for (int i=0; i<data_length; i++)
{
// Init Parameter
if(i==0)
{
maxThermo = minThermo = aveThermo = thermoData[i];
maxHygro = minHygro = hygroData[i];
continue;
}

if (maxThermo < thermoData[i]) maxThermo = thermoData[i];
if (minThermo > thermoData[i]) minThermo = thermoData[i];
aveThermo = (aveThermo + thermoData[i]) / 2;

if (maxHygro < hygroData[i]) maxHygro = hygroData[i];
if (minHygro > hygroData[i]) minHygro = hygroData[i];
}

//  find graphMinValue
for(graphMinValue = 0; graphMinValue<=40; graphMinValue++)
{
float idealButtomValue = aveThermo - 8.0;
if (idealButtomValue - graphMinValue < 0) break;
}

// Draw vertical axis value area
noStroke();
fill(0x , 0x , 0x );
rect(0, 15, 25, 100);

if (thermoMode)
{
stroke(0x55, 0x55, 0x55);
setTextSize(1);

text("C", 15, 17); //TODO: adjust

for (int i=0; i<5; i++)
{
//TODO: Need adjustment horizontal and/or vartical value.
char temp[16];
dtostrf(graphMinValue + i*4.0, 1, 1, temp);
text(temp, 1, 105 - i*20);
}
} else {
stroke(0x55, 0x55, 0x55);
setTextSize(1);
text("%", 15, 17); //TODO: adjust
for (int i=0; i<5; i++)
{
//TODO: Need adjustment horizontal and/or vartical value.
char temp[16];
dtostrf(10 + i*20, 1, 1, temp);
text(temp, 1, 105 - i*20);
}
}

// Erase Graph Area
stroke(0x88, 0x88, 0x88);
fill(0x , 0x , 0x );
rect(25, 30, 120, 80);

// Draw horizontal line
line(25, 90, 145, 90);
line(25, 70, 145, 70);
line(25, 50, 145, 50);

// Draw vartical line
line(115, 30, 115, 110);
line(85, 30, 85, 110);
line(55, 30, 55, 110);

// Plot Values
for (int i=1; i<data_length; i++)
{
stroke(0xf0, 0x1c, 0x1c);
int tempPlotY = 110 - ((thermoData[i] - graphMinValue) / 0.2);
if (tempPlotY > 110) tempPlotY = 119;
if (tempPlotY < 30) tempPlotY = 31;
point(145 - i, tempPlotY);

//TODO: plot values
stroke(0x5d, 0xaf, 0xc7);
const int hygroMinValue = 10;
int hygroPlotY = 110 - ((hygroData[i] - hygroMinValue));
if (hygroPlotY > 110) hygroPlotY = 119;
if (hygroPlotY < 30) hygroPlotY = 31;
point(145 - i, hygroPlotY);
}

// Draw current State noStroke();
fill(0x , 0x , 0x );
rect(86, 31, 58, 19);
if (thermoMode) {
char temp[16];
dtostrf(thermoData[0], 1, 1, temp);
stroke(0xf0, 0x1c, 0x1c);
setTextSize(2);
text(temp, 92, 34);
} else {
char temp[16];
dtostrf(hygroData[0], 1, 1, temp);
stroke(0x5d, 0xaf, 0xc7);
setTextSize(2);
text(temp, 92, 34);
}

return;
}
context = &DRAW_THERMO_HYGROMETER_CONTEXT;
//---------------------------------------------------------------------------------------------

background(0x , 0x , 0x );

// Draw thema color
noStroke();
fill(0xb1, 0x63, 0xa3);
rect(0, 0, width(), 15);
rect(0, height() - 3, width(), 3);

// Draw menu title
stroke(0x , 0x , 0x );
setTextSize(1);
text(MENU_TITLE, 5, 3);

// Draw Horizontal Axis Num
stroke(0x55, 0x55, 0x55);
setTextSize(1);
text("120", 20, 115);
text("90", 50, 115);
text("60", 80, 115);
text("30", 110, 115);
text("0", 140, 115);
text("s", 150, 115);

// Call Recursive func To Draw Graph Area
drawThermoHygrometer(thermoData, hygroData, data_length, thermoMode);
}
void FFTAnalyzerDisplay::drawDisabledDialogbox(void)
{
static void* DRAW_DISABLED_MENU;

// only draw once
if (context == &DRAW_DISABLED_MENU) return;
context = &DRAW_DISABLED_MENU;
//---------------------------------------------------------------------------------------------

// Draw Caution Rect
stroke(0xfc, 0x93, 0x33);
fill(0x , 0x , 0x );
rect(15, 50, width() - 30, height() - 100);

// Draw Caution Text
stroke(0x55, 0x55, 0x55);
setTextSize(1);
text("sorry, This Function", 20, 55);
text("is now disabled.", 20, 65);
}
