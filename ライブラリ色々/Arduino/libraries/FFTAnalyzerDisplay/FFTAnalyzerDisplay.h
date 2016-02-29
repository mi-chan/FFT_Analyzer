#ifndef FFTAnalyzerDisplay_H
#define FFTAnalyzerDisplay_H
#include "arduino.h"
#include "utility/Adafruit_GFX.h"
#include "utility/Adafruit_ST7735.h"

enum State
  {
    STATE_DEMO = 0,
    STATE_TITLE = 1,
    STATE_FFT = 10,
    STATE_LOG = 11,
    STATE_OTHERS = 12,
    STATE_CONFIG = 13,
    STATE_MUSIC = 20,
    STATE_THERMO_HYGRO = 21,
    STATE_RETURN = 22,
  };

class FFTAnalyzerDisplay : public Adafruit_ST7735
{
 public:
  FFTAnalyzerDisplay(uint8_t CS, uint8_t RS, uint8_t RST);
  void begin();
  void drawMenu(const char *MENU_TITLE, const char *MENU_TEXT[], int indexLength, unsigned int index);
  void drawFFTAnalyzer(uint8_t* fhtData, int dataLength);
  void drawThermoHygrometer(float *thermoData, float *hygroData, int DATA_LENGTH, bool ThermoMode);

 private:
  void** context;
  const char *lastDrawnMenu;
  int lastDrawnMenuIndex;
  int LED_PinNo;

  float m_oldTemperature;
  float m_oldHumidity;
};

#if (USB_VID == 0x2341) && (USB_PID == 0x803C) // are we building for Esplora?  
extern TFT EsploraTFT;
#endif

#endif
