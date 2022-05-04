#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Bounce.h>

#define BUTTON_PIN 20 // Digital IO pin connected to the button.  This will be
                      // driven with a pull-up resistor so the switch should
                      // pull the pin to ground momentarily.  On a high -> low
                      // transition the button press logic will execute.

#define program1 2
#define program2 3
#define program3 4
#define program4 5
#define program5 6
#define program6 7
#define program7 8
#define program8 9
#define program9 10

#define PIXEL_PIN 19 // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 900

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Deffine buttons used ffor program selection
Bounce program1Button = Bounce(program1, 10);
Bounce program2Button = Bounce(program2, 10);
Bounce program3Button = Bounce(program3, 10);
Bounce program4Button = Bounce(program4, 10);
Bounce program5Button = Bounce(program5, 10);
Bounce program6Button = Bounce(program6, 10);
Bounce program7Button = Bounce(program7, 10);
Bounce program8Button = Bounce(program8, 10);
Bounce program9Button = Bounce(program9, 10);
Bounce startButton = Bounce(BUTTON_PIN, 10);

int minDelay = 17;
int analogWait = 0;
uint32_t color = strip.Color(10, 0, 0);
uint32_t blank = strip.Color(0, 0, 0);
bool isOffset = true;
bool isPixels = false;
bool pattern[PIXEL_COUNT];
int currentOffset = 0;
bool show = true;

void fillPattern(int pixels, int pixelOffset)
{
  for (int j = 0; j < strip.numPixels(); j += (pixels + pixelOffset))
  {
    for (int i = 0; i < pixels; i++)
    {
      pattern[i + j] = 1;
    }
    for (int i = 0; i < pixelOffset; i++)
    {
      pattern[i + j + pixels] = 0;
    }
  }
}

void shiftArray(int pixels, int pixelOffset)
{
  int currentColor;
  if (isOffset && currentOffset == pixelOffset)
  {
    currentOffset = 0;
    isOffset = false;
    isPixels = true;
  }
  if (isPixels && currentOffset == pixels)
  {
    currentOffset = 0;
    isOffset = true;
    isPixels = false;
  }

  if (isOffset)
  {
    currentColor = 0;
    currentOffset++;
  }

  if (isPixels)
  {
    currentColor = 1;
    currentOffset++;
  }

  for (int i = PIXEL_COUNT; i >= 1; i--)
  {
    pattern[i] = pattern[i - 1];
  }
  pattern[0] = currentColor;
}

void cycle(int pixels, int pixelOffset)
{
  fillPattern(pixels, pixelOffset);
  while (show)
  {
    int colorIntensity = 0;
    for (size_t i = 0; i < 32; i++)
    {
      colorIntensity += analogRead(1);
    }

    colorIntensity = ((colorIntensity / 32) >> 2) - 10;
    color = strip.Color(colorIntensity + 10, 0, 0);
    for (int j = 0; j < PIXEL_COUNT; j++)
    {
      uint32_t currentColor;
      if (pattern[j % PIXEL_COUNT])
      {
        currentColor = color;
      }
      else
      {
        currentColor = blank;
      }
      strip.setPixelColor(j, currentColor);
    }
    strip.show();

    if (startButton.update())
    {
      if (startButton.fallingEdge())
      {
        show = false;
      }
    }
  }
  while (true)
  {
    for (int i = 0; i < (pixels + pixelOffset); i++)
    {
      int colorIntensity = 0;
      for (size_t i = 0; i < 32; i++)
      {
        colorIntensity += analogRead(1);
      }

      colorIntensity = ((colorIntensity / 32) >> 2) - 10;
      color = strip.Color(colorIntensity + 10, 0, 0);
      for (int j = 0; j < PIXEL_COUNT; j++)
      {
        uint32_t currentColor;
        if (pattern[j % PIXEL_COUNT])
        {
          currentColor = color;
        }
        else
        {
          currentColor = blank;
        }
        strip.setPixelColor(j, currentColor);
      }
      strip.show();
      shiftArray(pixels, pixelOffset);

      for (size_t i = 0; i < 32; i++)
      {
        analogWait += analogRead(0);
      }

      analogWait = analogWait / 32;
      delay(analogWait + minDelay);
    }
  }
}

void setup()
{
  // put your setup code here, to run once:
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(program1, INPUT_PULLUP);
  pinMode(program2, INPUT_PULLUP);
  pinMode(program3, INPUT_PULLUP);
  pinMode(program4, INPUT_PULLUP);
  pinMode(program5, INPUT_PULLUP);
  pinMode(program6, INPUT_PULLUP);
  pinMode(program7, INPUT_PULLUP);
  pinMode(program8, INPUT_PULLUP);
  pinMode(program9, INPUT_PULLUP);
}

void loop()
{
  // Get current button state.
  if (program1Button.update())
  {
    if (program1Button.fallingEdge())
    {
      cycle(36, 36);
    }
  }
  if (program2Button.update())
  {
    if (program2Button.fallingEdge())
    {
      cycle(48, 36);
    }
  }
  if (program3Button.update())
  {
    if (program3Button.fallingEdge())
    {
      cycle(36, 60);
    }
  }
  if (program4Button.update())
  {
    if (program4Button.fallingEdge())
    {
      cycle(48, 60);
    }
  }
  if (program5Button.update())
  {
    if (program5Button.fallingEdge())
    {
      cycle(60, 60);
    }
  }
  if (program6Button.update())
  {
    if (program6Button.fallingEdge())
    {
      cycle(48, 84);
    }
  }
  if (program7Button.update())
  {
    if (program7Button.fallingEdge())
    {
      cycle(60, 84);
    }
  }
  if (program8Button.update())
  {
    if (program8Button.fallingEdge())
    {
      cycle(72, 84);
    }
  }
  if (program9Button.update())
  {
    if (program9Button.fallingEdge())
    {
      cycle(60, 108);
    }
  }
}
