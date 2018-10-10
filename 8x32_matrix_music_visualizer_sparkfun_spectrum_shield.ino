#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
 
#define PIN 6

// PANEL SETUP
int width = 32;
int height = 8;
int NUM_LEDS = width * height;
int band_width = 4;

// AUDIO INPUT SETUP
int strobe = 4;
int reset = 5;
int audio1 = A0;
int audio2 = A1;
int left[7];
int right[7];
int band;
int audio_input = 0;
int freq = 0;
int react_arr[] = {0, 0, 0, 0, 0, 0, 0};
int prereact_arr[] = {0, 0, 0, 0, 0, 0, 0};


// STANDARD VISUALIZER VARIABLES
float modifier = 2.5; // CONTROLS SENSITIVITY OF THE LED REACTION
int loop_max = 0;
int k = 255; // COLOR WHEEL POSITION
int decay = 1; // HOW MANY MS BEFORE ONE LIGHT DECAY
int decay_check = 0;
//long pre_react = 0; // NEW SPIKE CONVERSION
//long react = 0; // NUMBER OF LEDs BEING LIT
long post_react = 0; // OLD SPIKE CONVERSION


// RAINBOW WAVE SETTINGS
int wheel_speed = 2;
 
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(width, height, PIN,
  NEO_MATRIX_BOTTOM    + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);
 
const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(255, 255, 0),matrix.Color(0, 0, 255), matrix.Color(255, 0, 255), matrix.Color(0, 255, 255), matrix.Color(255, 255, 255)};


void readMSGEQ7()
// Function to read 7 band equalizers
{
  digitalWrite(reset, HIGH);
  digitalWrite(reset, LOW);
  for(band=0; band <7; band++)
  {
    digitalWrite(strobe, LOW); // strobe pin on the shield - kicks the IC up to the next band 
    delayMicroseconds(30); // 
    left[band] = analogRead(audio1) * modifier; // store left band reading
    right[band] = analogRead(audio2) * modifier; // ... and the right

    if (left[band] > 1023)
      left[band] = 1023;
    if (right[band] > 1023)
      right[band] = 1023;
      
    digitalWrite(strobe, HIGH); 
  }
}



uint32_t Scroll(int pos) {
  int g;
  int r;
  int b;
  if(pos < 85) {
    g = 0;
    r = ((float)pos / 85.0f) * 255.0f;
    b = 255 - r;
  } else if(pos < 170) {
    g = ((float)(pos - 85) / 85.0f) * 255.0f;
    r = 255 - g;
    b = 0;
  } else if(pos < 256) {
    b = ((float)(pos - 170) / 85.0f) * 255.0f;
    g = 255 - b;
    r = 1;
  }
  return matrix.Color(g, r, b);
}




void singleRainbow()
{
  int band_width = width / 7;
  for(int x = 6, x2 = 0; x >= 0, x2 < 7; x--, x2++)
  {
    for(int i = height - 1; i >= 0; i--) {
      for(int w = 1; w <= band_width; w++)
      {
        if (i < react_arr[x])
          matrix.drawPixel((x2 * band_width) + w, i, Scroll((i * 256 / 50 + k) % 256));
          //matrix.setPixelColor((8 * x) + i, Scroll((i * 256 / 50 + k) % 256));
          
        else
          matrix.drawPixel((x2 * band_width) + w, i, (0, 0, 0));
          //matrix.setPixelColor((8 * x) + i, (0, 0, 0));   
      }   
    }
  }
  matrix.show(); 
}


void convertSingle(int i)
{
  if (left[i] > right[i])
    audio_input = left[i];
  else
    audio_input = right[i];

  if (audio_input > 80 * modifier)
  {
    prereact_arr[i] = ((long)height * (long)audio_input) / 1023L; // TRANSLATE AUDIO LEVEL TO NUMBER OF LEDs

    if (prereact_arr[i] > react_arr[i]) // ONLY ADJUST LEVEL OF LED IF LEVEL HIGHER THAN CURRENT LEVEL
      react_arr[i] = prereact_arr[i];

    Serial.print(audio_input);
    Serial.print(" -> ");
    Serial.println(prereact_arr[i]);
  }
}



// FUNCTION TO VISUALIZE WITH A SINGLE LEVEL
void singleLevel()
{
  readMSGEQ7();

  for (int i = 0; i < 7; i++)
    convertSingle(i);

  singleRainbow(); // APPLY COLOR

  k = k - wheel_speed; // SPEED OF COLOR WHEEL
  if (k < 0) // RESET COLOR WHEEL
    k = 255;

  // REMOVE LEDs
  decay_check++;
  if (decay_check > decay)
  {
    decay_check = 0;
    for (int x = 0; x < 7; x++)
    {
      if (react_arr[x] > 0)
        react_arr[x]--;
    }
  }
}



void setup() {
  // SPECTRUM SETUP
  pinMode(audio1, INPUT);
  pinMode(audio2, INPUT);
  pinMode(strobe, OUTPUT);
  pinMode(reset, OUTPUT);
  digitalWrite(reset, LOW);
  digitalWrite(strobe, HIGH);
  
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(5);
  matrix.setTextColor(colors[0]);

  // SERIAL AND INPUT SETUP
  Serial.begin(115200);
  Serial.println("\nListening...");
}



void loop()
{  
  singleLevel();
  //doubleLevel();
  //delay(1);
}
