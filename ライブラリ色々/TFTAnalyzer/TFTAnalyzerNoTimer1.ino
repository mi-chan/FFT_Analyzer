#include "DHT.h"
#include <FFTAnalyzerDisplay.h>
#include <SPI.h>

//FHT Definition
#define LOG_OUT (1) // use the log output function
#define FHT_N (256) // set to 256 point fht
#include <FHT.h> // necessary to define After LOG_OUT and FHT_N defined

// TFT LCD Desplay Definition and Class Instatiate
#define cs  (10)
#define dc  (9)
#define rst (8)

FFTAnalyzerDisplay screen = FFTAnalyzerDisplay(cs, dc, rst);

// DHT Definition and Class Instatiate
#define DHTPIN (2)
#define DHTTYPE (DHT22)

DHT dht(DHTPIN, DHTTYPE);
char templaturePrintOut[5];

// Button Definition
#define BUTTON_INT (1)
#define BUTTON_DOWN (5)
#define BUTTON_UP (7)
#define BUTTON_SELECT (6)

#define UNPUSH (1)
#define PUSHED (0)

// enum of "State" is defined in <FFTAnalyzerDisplay.h>
State state_ = STATE_TITLE;

// Int Button State
volatile bool button_down = false;
volatile bool button_up = false;
volatile bool button_select = false;

void setup()
{
  // Instatnce.begin
  screen.begin();
  dht.begin();
  
  // Setup pin mode
  pinMode(BUTTON_DOWN, INPUT);
  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_SELECT, INPUT);
  
  // Setup Int
  attachInterrupt(BUTTON_INT, button_isr, FALLING);
  
  // Setup Debug Code
  Serial.begin(9600);
  Serial.println("Start");
}

//---------- ISR and Related Functions ----------

void button_isr()
{
  static volatile long lastIntTime;
  const long DEBOUNCE_TIME = 200;
  
  if(millis() - lastIntTime < DEBOUNCE_TIME) return;

  Serial.println("Interrupt Code is executed!");
  
  button_down = !digitalRead(BUTTON_DOWN);
  button_up = !digitalRead(BUTTON_UP);
  button_select = !digitalRead(BUTTON_SELECT);
  
  lastIntTime = millis();
}

void resetButtonState()
{
  Serial.println("All button flags are cleared!");
  button_down = button_up = button_select = false;
}

//---------- MainLoop ----------

void loop()
{
  switch (state_)
  {
    case STATE_TITLE:
      Serial.println("MainLoop: Dive to STATE_TITLE!");
      state_ = mainMenu();
      break;
    case STATE_FFT:
      Serial.println("MainLoop: Dive to STATE_FFT!");
      state_ = fftAnalyzer();
      break;
    case STATE_LOG:
      Serial.println("MainLoop: Dive to STATE_LOG!");
      break;
    case STATE_OTHERS:
      Serial.println("MainLoop: Dive to STATE_OTHERS!");
      state_ = othersMenu();
      break;
    case STATE_MUSIC:
      Serial.println("MainLoop: Dive to STATE_MUSIC!");
      break;
    case STATE_THERMO_HYGRO:
      Serial.println("MainLoop: Dive to STATE_THERMO_HYGRO!");
      state_ = thermoHygrometer();
      break;
    case STATE_CONFIG:
      Serial.println("MainLoop: Dive to STATE_CONFIG!");
      break;
    default:
      Serial.println("[Error] MainLoop: Dive to DEFAULT!");
      state_ = STATE_TITLE;
      break;
  }
}

//---------- MENU Functions ----------

State mainMenu(void)
{
  static const char *MAIN_MENU_TITLE = "Hello FFT Analyzer";
  static const char *MAIN_MENU_TEXT[] = { "FFT Analyzer", "LogView Mode", "Other Functions", "Configure" };
  static const int MAIN_INDEX_LENGTH = (sizeof MAIN_MENU_TEXT) / (sizeof MAIN_MENU_TEXT[0]);
  
  static unsigned int index = 0;
  
  while(!button_select)
  {
    screen.drawMenu(MAIN_MENU_TITLE, MAIN_MENU_TEXT, MAIN_INDEX_LENGTH, index);
    
    while(1)
    {
      if(button_select) break;
      
      if(button_down)
      {
        if(index == MAIN_INDEX_LENGTH - 1)
        {
          index = 0;
        }else{
          index++;
        }
        resetButtonState();
        break;
      }
      
      if(button_up)
      {
        if(index == 0)
        {
          index = MAIN_INDEX_LENGTH - 1;
        }else{
          index--;
        }
        resetButtonState();
        break;
      }
    }
  }
  
  Serial.println("mainMenu: Select Button Pushed");
  resetButtonState();
  
  return (State)(STATE_FFT + index);
}

State othersMenu(void)
{
  static const char *MENU_TITLE = "OTHERS MENU";
  static const char *MENU_TEXT[] = { "Music Scale Analyzer", "Thermo-Hygrometer", "Return" };
  static const int INDEX_LENGTH = (sizeof MENU_TEXT) / (sizeof MENU_TEXT[0]);
  
  static unsigned int index = 0;
  
  while(!button_select)
  {
    screen.drawMenu(MENU_TITLE, MENU_TEXT, INDEX_LENGTH, index);
    
    while(1)
    {
      if(button_select) break;

      if(button_down)
      {
        if(index == INDEX_LENGTH - 1)
        {
          index = 0;
        }else{
          index++;
        }
        resetButtonState();
        break;
      }
      
      if(button_up)
      {
        if(index == 0)
        {
          index = INDEX_LENGTH - 1;
        }else{
          index--;
        }
        resetButtonState();
        break;
      }
    }
  }
  
  Serial.println("othersMenu: Select Button Pushed");
  resetButtonState();
  
  return (State)(STATE_MUSIC + index);
}

//---------- Thermo-Hygrometer Function ----------

State thermoHygrometer(void)
{
  // Init variables
  const int DATA_MAX = 120;
  float thermoData[DATA_MAX + 1];
  float hygroData[DATA_MAX + 1];
  int data_length = 0;
  bool thermoMode = true;
  
   while(!button_select)
  {
    // Shift DataArray
    for (int i=data_length; i>0; i--)
    { 
      thermoData[i] = thermoData[i - 1];
      hygroData[i] = hygroData[i - 1];
    }

    thermoData[0] = dht.readTemperature();
    hygroData[0] = dht.readHumidity();
    
    if(data_length < DATA_MAX) data_length++;
    
    screen.drawThermoHygrometer(thermoData, hygroData, data_length, thermoMode);

    thermoMode = !thermoMode;

    delay(1000);
  }
  
  resetButtonState();
  
  return STATE_OTHERS;
}


//---------- FFTAnalyzer Function ----------

// Register buckup variables
byte timsk0;
byte adcsra;
byte admux;
byte didr0;

State fftAnalyzer(void)
{
  fhtAnalyzerInit();
  
  while(!button_select)
  {
    uint8_t* fht_result = fhtFunction();
    screen.drawFFTAnalyzer(fht_result, FHT_N/2);    
    delay(500);
  }
  
  resetButtonState();
  fhtAnalyzerRestore();
  return STATE_TITLE;
}

void fhtAnalyzerInit(void)
{
  timsk0 = TIMSK0;
  adcsra = ADCSRA;
  admux = ADMUX;
  didr0 = DIDR0;
  
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void fhtAnalyzerRestore(void)
{  
  TIMSK0 = timsk0;
  ADCSRA = adcsra;
  ADMUX = admux;
  DIDR0 = didr0;
}

uint8_t* fhtFunction()
{
  for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
    while(!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fht_input[i] = k; // put real data into bins
  }
  fht_window(); // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run(); // process the data in the fht
  fht_mag_log(); // take the output of the fht
  
  return fht_log_out;
}

