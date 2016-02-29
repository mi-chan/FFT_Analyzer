/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General public License version 2 as
 * published by the Free Software Foundation;
 *
 *
 * Author: Yamato Suzuki
 */

#include "arduino.h"
#include "FFTAnalyzerDisplay.h"

#if (USB_VID == 0x2341) && (USB_PID == 0x803C)
TFT EsploraTFT(7, 0, 1);
#endif

FFTAnalyzerDisplay::FFTAnalyzerDisplay(uint8_t CS, uint8_t RS, uint8_t RST) : Adafruit_ST7735(CS, RS, RST)
{
  // as we already know the orientation (landscape, therefore rotated),
  // set default width and height without need to call begin() first.
  
  _width = ST7735_TFTHEIGHT;
  _height = ST7735_TFTWIDTH;
}

void FFTAnalyzerDisplay::begin()
{
  static void* BEGIN_CONTEXT;

  //  initR(INITR_REDTAB);
  initG();
  setRotation(1);

  context = &BEGIN_CONTEXT;
}


void FFTAnalyzerDisplay::drawMenu(const char *MENU_TITLE, const char *MENU_TEXT[], int indexLength, unsigned int index)
{
  static void* DRAW_MENU_CONTEXT;

  // Error check
  if ( indexLength < 0  || indexLength > 4 ) return;

  // if only Draw index Arrow
  if ( (context == &DRAW_MENU_CONTEXT) && (lastDrawnMenu == MENU_TITLE))
    {
      setTextSize(1);
      // Erase the Last drawn Arrow
      stroke(0xff, 0xff, 0xff);
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
  background(0xff, 0xff, 0xff);

  // Draw thema color
  noStroke();
  fill(0xb1, 0x63, 0xa3);
  rect(0, 0, width(), 15);
  rect(0, height() -3, width(), 3);

  // Draw menu title
  stroke(0xff, 0xff, 0xff);
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
  text("v0.51b", 110, 110);

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

      // Erase Graph Area
      stroke(0x88, 0x88, 0x88);
      fill(0xff, 0xff, 0xff);
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
          stroke(0x0, 0xff, 0x0);
          int tempPlotY = (int)(fhtData[i] / FHT_DATA_MAX ) * 80 + 30;
          if (tempPlotY > 110) tempPlotY = 119;
          if (tempPlotY < 30) tempPlotY = 31;
          point(25 + i, tempPlotY);
        }

      return;
    }

  context = &DRAW_FFT_ANALYZER;

  //---------------------------------------------------------------------------------------------

  background(0xff, 0xff, 0xff);

  // Draw thema color
  noStroke();
  fill(0xb1, 0x63, 0xa3);
  rect(0, 0, width(), 15);
  rect(0, height() - 3, width(), 3);

  // Draw menu title
  stroke(0xff, 0xff, 0xff);
  setTextSize(1);
  text(MENU_TITLE, 5, 3);

  // Draw Vertical Axis Num
  stroke(0x55, 0x55, 0x55);
  setTextSize(1);
  text("db", 15, 17);
  text("100", 1, 25);
  text("75", 1, 45);
  text("50", 1, 65);
  text("25", 1, 85);
  text("0", 1, 105);

  // Draw Horizontal Axis Num
  stroke(0x55, 0x55, 0x55);
  setTextSize(1);
  text("0", 20, 115);
  text("250", 50, 115);
  text("500", 80, 115);
  text("1k", 110, 115);
  text("2k", 140, 115);
  text("Hz", 150, 115);

  // Call Recursive func To Draw Graph Area
  drawFFTAnalyzer(fhtData, dataLength);
}



void FFTAnalyzerDisplay::drawThermoHygrometer(float *thermoData, float *hygroData, int data_length, bool thermoMode)
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

    // find graphMinValue
    for(graphMinValue = 0; graphMinValue<=40; graphMinValue++)
    {
      float idealButtomValue = aveThermo - 8.0;
      if (idealButtomValue - graphMinValue < 0) break;
    }

    // Draw vertical axis value area
    noStroke();
    fill(0xff, 0xff, 0xff);
    rect(0, 15, 25, 100);

    if (thermoMode)
    {
      stroke(0x55, 0x55, 0x55);
      setTextSize(1);

      text("C", 15, 17);  //TODO: adjust

      for (int i=0; i<5; i++)
      {
        //TODO: Need adjustment horizontal and/or vartical value.
        char temp[5];
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
        char temp[5];
        dtostrf(10 + i*20, 1, 1, temp);
        text(temp, 1, 105 - i*20);
      }
    }

    // Erase Graph Area
    stroke(0x88, 0x88, 0x88);
    fill(0xff, 0xff, 0xff);
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

    // Draw current State
    noStroke();
    fill(0xff, 0xff, 0xff);
    rect(86, 31, 58, 19);

    if (thermoMode)
    {
      char temp[5];
      dtostrf(thermoData[0], 1, 1, temp);
      stroke(0xf0, 0x1c, 0x1c);
      setTextSize(2);
      text(temp, 92, 34);
    } else {
      char temp[5];
      dtostrf(hygroData[0], 1, 1, temp);
      stroke(0x5d, 0xaf, 0xc7);
      setTextSize(2);
      text(temp, 92, 34);
    }

    return;
  }

  context = &DRAW_THERMO_HYGROMETER_CONTEXT;

  //---------------------------------------------------------------------------------------------
  
  background(0xff, 0xff, 0xff);

  // Draw thema color
  noStroke();
  fill(0xb1, 0x63, 0xa3);
  rect(0, 0, width(), 15);
  rect(0, height() - 3, width(), 3);

  // Draw menu title
  stroke(0xff, 0xff, 0xff);
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
