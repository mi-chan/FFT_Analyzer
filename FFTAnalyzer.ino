#include "DHT.h"
#include <FFTAnalyzerDisplay.h> #include <SPI.h>
#include <TimerOne.h>

/* FHT De nition */
#de ne NYQUIST_FREQUENCY (30) // set a timer of length 100us = 1kHz
#de ne LOG_OUT (1)            // use the log output function
#de ne FHT_N (256)            // set to 256 point fht
#include <FHT.h>              // necessary to de ne After LOG_OUT and FHT_N de ned

/* TFT LCD Desplay De nition and Class Instatiate */
#de ne cs (10)
#de ne dc (9)
#de ne rst (8)

FFTAnalyzerDisplay screen = FFTAnalyzerDisplay(cs, dc, rst);

/* DHT De nition and Class Instatiate */
#de ne DHTPIN (2)
#de ne DHTTYPE (DHT22)

DHT dht(DHTPIN, DHTTYPE);
char templaturePrintOut[5];

/* Button De nition */
#de ne BUTTON_INT (1)
#de ne BUTTON_DOWN (5)
#de ne BUTTON_UP (7)
#de ne BUTTON_SELECT (6)

#de ne UNPUSH (1)
#de ne PUSHED (0)

/* enum of "State" is de ned in <FFTAnalyzerDisplay.h> */
State state_ = STATE_TITLE;
/* Int Button State */
volatile bool button_down = false;
volatile bool button_up = false;
volatile bool button_select = false;

/* FFT related variables */
volatile int adcIndex;

void setup() {
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
Serial.begin(115200);
Serial.println("Start");
}

// ------------------------------ ISR and Related Functions ------------------------------
void button_isr() {
static volatile long lastIntTime;
const long DEBOUNCE_TIME = 200;

if(millis() - lastIntTime < DEBOUNCE_TIME) return;

Serial.println("Interrupt Code is executed!");
button_down = !digitalRead(BUTTON_DOWN);
button_up = !digitalRead(BUTTON_UP);
button_select = !digitalRead(BUTTON_SELECT);
lastIntTime = millis();
}

void resetButtonState() {
Serial.println("All button  ags are cleared!");
button_down = button_up = button_select = false;
}

void  tTimerIsr() {
while(!(ADCSRA & 0x10)); // wait for ADC conversion completed
ADCSRA = 0xf5; // restart ADC

if (adcIndex < FHT_N) {
byte low = ADCL;
byte high = ADCH;
int adcData = (high << 8) | low;
adcData -= 0x0200; // from into a signed int
adcData <<= 6; // from into a 16b signed int
fht_input[adcIndex] = adcData;
adcIndex++;
}
}

// ------------------------------ MainLoop ------------------------------
void loop() {
switch (state_) {
case STATE_TITLE:
Serial.println("MainLoop: Dive into STATE_TITLE!");
state_ = mainMenu();
break;
case STATE_FFT:
Serial.println("MainLoop: Dive into STATE_FFT!");
state_ =  tAnalyzer();
break;
case STATE_LOG:
Serial.println("MainLoop: Dive into STATE_LOG!");
state_ = logView();
break;
case STATE_OTHERS:
Serial.println("MainLoop: Dive into STATE_OTHERS!");
state_ = othersMenu();
break;
case STATE_MUSIC:
Serial.println("MainLoop: Dive into STATE_MUSIC!");
state_ = musicScaler();
break;
case STATE_THERMO_HYGRO:
Serial.println("MainLoop: Dive into STATE_THERMO_HYGRO!");
state_ = thermoHygrometer();
break;
case STATE_CONFIG:
Serial.println("MainLoop: Dive into STATE_CONFIG!");
state_ = con gure();
break;
default:
Serial.println("[Error] MainLoop: Dive into DEFAULT!");
state_ = STATE_TITLE;
break;
}
}

// ------------------------------ MENU Functions ------------------------------

State mainMenu(void)
{
static const char *MAIN_MENU_TITLE = "Hello FFT Analyzer";
static const char *MAIN_MENU_TEXT[] = { "FFT Analyzer", "LogView Mode", "Other Functions", "Con gure" };
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

while(!button_select) {
screen.drawMenu(MENU_TITLE, MENU_TEXT, INDEX_LENGTH, index);

while(1) {
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

// ------------------------------ Thermo-Hygrometer Function ------------------------------

State thermoHygrometer(void)
{
// Init variables
const int DATA_MAX = 120;
oat thermoData[DATA_MAX]; // BOF
oat hygroData[DATA_MAX]; // BOF
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

// Register buckup variables byte timsk0;
byte adcsra;
byte admux;
byte didr0;

State  tAnalyzer(void)
{
while(!button_select)
{
fhtAnalyzerInit();
uint8_t* fht_result = fhtFunction();
fhtAnalyzerRestore();

Serial.print("255: ");
for(int i=0; i<128; i++)
{
Serial.print(fht_log_out[i]);
Serial.print(" ");
}
Serial.println();
screen.drawFFTAnalyzer(fht_result, FHT_N/2);
delay(150);
}

resetButtonState();
return STATE_TITLE;
}

void fhtAnalyzerInit(void)
{
timsk0 = TIMSK0;
adcsra = ADCSRA;
admux = ADMUX;
didr0 = DIDR0;

TIMSK0 = 0;       // turn o  timer0 for lower jitter
ADCSRA = 0xe5;    // set the adc to free running mode
ADMUX = 0x40;     // use adc0
DIDR0 = 0x01;     // turn o  the digital input for adc0

Timer1.initialize(NYQUIST_FREQUENCY);
}

void fhtAnalyzerRestore(void) {
TIMSK0 = timsk0;
ADCSRA = adcsra;
ADMUX = admux;
DIDR0 = didr0;
}

uint8_t* fhtFunction() {
adcIndex = 0;
Timer1.attachInterrupt( tTimerIsr);
while(adcIndex < FHT_N);
Timer1.detachInterrupt();

fht_window(); // window the data for better frequency response
fht_reorder(); // reorder the data before doing the fht
fht_run(); // process the data in the fht
fht_mag_log(); // take the output of the fht

return fht_log_out;
}

// ------------------------------ musicScaler Function ------------------------------
State musicScaler(void)
{
while(!button_select)
{
screen.drawDisabledDialogbox();
}
resetButtonState();
return STATE_TITLE;
}
