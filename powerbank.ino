#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Select in IDE  Tools->Board 'NodeMCU-32S' 
// or 'Nologo ESP32C3 Super Mini' target 
// #define ESP32_C3_SUPERMINI_TARGET

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#ifdef ESP32_C3_SUPERMINI_TARGET
const int portPinAB = 1;  //  for A+B: 1 at ESP32-C3
const int portPinB = 3;   //  for B:   3 at ESP32-C3 
const int coefAB = 132;   //  to adjust ADC channel AB
const int coefB = 138;    //  to adjust ADC channel B
#else
const int coefAB = 107;  //  to adjust ADC channel AB
const int coefB = 103;   //  to adjust ADC channel B
const int portPinAB = 34;  //  for A+B: 34 at ESP32
const int portPinB = 35;  //  for B :  35 at ESP32
#endif

const int lowV = 10500;
const int rUp = 1000;
const int rDown = 68;
const int rSum = rUp + rDown;

bool oledOk = false;
bool zerosV = false;

void setup() {
  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  // Set the resolution to 12 bits (0-4095)
  // Configure attenuation to 11dB for 0-3.3V range  
#ifdef ESP32_C3_SUPERMINI_TARGET
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
#else
  analogReadResolution(12);
#endif
  delay(500);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  } else {
    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
    delay(1000);

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    oledOk = true;
  }
}

void loop() {
  // read the analog / millivolts value for pin portPin:
#ifdef ESP32_C3_SUPERMINI_TARGET
  int analogValueAB = analogRead(portPinAB);
  int analogValueB = analogRead(portPinB);
#else
  int analogValueAB = analogRead(portPinAB);
  int analogValueB = analogRead(portPinB);
#endif

  // avoid noice near zero 
  if (analogValueB < 80 && analogValueB > 0)
    analogValueB = 0;
  if (analogValueAB < 80 && analogValueAB > 0)
    analogValueAB = 0;

  // print out the values you read:
  int sumVolt = analogValueAB * rSum / rDown * 100 / coefAB;
  int bVolt = analogValueB * rSum / rDown * 100 / coefB;
  int aVolt = sumVolt -  bVolt;

  Serial.printf("ADC B+A  mV: %d\n", sumVolt);
  Serial.printf("ADC B mV: %d\n", bVolt);
  if (aVolt > 0) {
     Serial.printf("ADC A mV: %d\n", aVolt);
  }

  if (oledOk) {
        
    display.clearDisplay();
    display.setCursor(0,0);             // Start at top-left corner

    // 2 channel monitoring cases
    if (bVolt > 0 && sumVolt > 0 || bVolt > 0 && sumVolt == 0) {
      if (sumVolt > 0) {
        display.print(sumVolt/1000);
        display.print(F("." ));
        display.print((sumVolt%1000)/100);
        display.print(F("V "));
                
        if ((bVolt > 0 && bVolt < lowV) || (aVolt > 0 && aVolt < lowV )) {
          if (!zerosV) {
            if (bVolt > 0 && bVolt < lowV)
              display.print(F("B"));
            if (aVolt > 0 && aVolt < lowV)
              display.print(F("A"));
            display.print(F("!"));  
          }
          zerosV = !zerosV;
          display.println(F(""));
        } else {
          zerosV = false;
          display.println(F("B+A" ));
        }
      }
     
      display.print(bVolt/1000);
      display.print(F("." ));
      display.print((bVolt%1000)/100);
 
      if (aVolt > 0) {
        display.print(F("+" ));
        display.print(aVolt/1000);
        display.print(F("." ));
        display.print((aVolt%1000)/100);
      } else {
        display.print(F(" A|B" ));
      }
  
      display.display();
    }
    
    if (bVolt == 0 && sumVolt > 0) {
      display.print(sumVolt/1000);
      display.print(F("." ));
      display.print((sumVolt%1000)/100);
      display.println(F("V B+A:" ));
      display.display();
    }
    
    if ((bVolt == 0 && sumVolt == 0) || (sumVolt < 800) && bVolt < 500 ) {
      if (!zerosV) {
        display.println(F("0V" ));
      }
      zerosV = !zerosV;
      display.display();
    }

     
  } // oledOk

  delay(500);  // delay in between reads
}

