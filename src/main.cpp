#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Bounce.h>

#define START_PIN 20
#define PROGRAM_PIN1 2
#define PROGRAM_PIN2 3
#define PROGRAM_PIN3 4
#define PROGRAM_PIN4 5
#define PROGRAM_PIN5 6
#define PROGRAM_PIN6 7
#define PROGRAM_PIN7 8
#define PROGRAM_PIN8 9
#define PROGRAM_PIN9 10
#define PAUSE_PIN 11
#define STOP_PIN 12

#define PIXEL_PIN 19 // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 900

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick

// Pattern types supported:
enum program
{
  NONE,
  PROGRAM1,
  PROGRAM2,
  PROGRAM3,
  PROGRAM4,
  PROGRAM5,
  PROGRAM6,
  PROGRAM7,
  PROGRAM8,
  PROGRAM9
};
// Pattern directions supported:
enum direction
{
  FORWARD,
  REVERSE
};
void ProgramComplete();
class NeoPatterns : public Adafruit_NeoPixel
{
public:
  // Member Variables:
  program ActivePattern; // which pattern is running
  direction Direction;   // direction to run the pattern

  unsigned long Interval;   // milliseconds between updates
  unsigned long lastUpdate; // last update of position

  uint32_t Color1, Color2; // What colors are in use
  uint16_t TotalSteps;     // total number of steps in the pattern
  uint16_t Index;          // current step within the pattern

  void (*OnComplete)(); // Callback on completion of pattern

  bool isOffset = true;
  bool isPixels = false;
  bool *PatternPtr = new bool[TotalSteps];
  int currentOffset = 0;
  uint16_t pixel, pixelOffset;
  uint32_t color;
  uint32_t blank;

  // Constructor - calls base-class constructor to initialize strip
  NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
      : Adafruit_NeoPixel(pixels, pin, type)
  {
    TotalSteps = pixels;
    PatternPtr = new bool[pixels];
    color = Color(10, 0, 0);
    blank = Color(0, 0, 0);
    OnComplete = callback;
  }

  // Update the pattern
  void Update()
  {
    if ((millis() - lastUpdate) > Interval) // time to update
    {
      lastUpdate = millis();
      switch (ActivePattern)
      {
      case PROGRAM1:
        Cycle(36, 36);
        break;
      case PROGRAM2:
        Cycle(48, 36);
        break;
      case PROGRAM3:
        Cycle(36, 60);
        break;
      case PROGRAM4:
        Cycle(48, 60);
        break;
      case PROGRAM5:
        Cycle(60, 60);
        break;
      case PROGRAM6:
        Cycle(48, 84);
        break;
      case PROGRAM7:
        Cycle(60, 84);
        break;
      case PROGRAM8:
        Cycle(72, 84);
        break;
      case PROGRAM9:
        Cycle(60, 108);
        break;
      default:
        break;
      }
    }
  }

  // Increment the Index and reset at the end
  void Increment()
  {
    if (Direction == FORWARD)
    {
      Index++;
      if (Index >= TotalSteps)
      {
        Index = 0;
        if (OnComplete != NULL)
        {
          OnComplete(); // call the completion callback
        }
      }
    }
    else // Direction == REVERSE
    {
      --Index;
      if (Index <= 0)
      {
        Index = TotalSteps - 1;
        if (OnComplete != NULL)
        {
          OnComplete(); // call the completion callback
        }
      }
    }
  }

  // Reverse pattern direction
  void Reverse()
  {
    if (Direction == FORWARD)
    {
      Direction = REVERSE;
      Index = TotalSteps - 1;
    }
    else
    {
      Direction = FORWARD;
      Index = 0;
    }
  }

  void ShiftArray(int pixels, int pixelOffset)
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

    for (int i = TotalSteps; i >= 1; i--)
    {
      PatternPtr[i] = PatternPtr[i - 1];
    }
    PatternPtr[0] = currentColor;
  }

  void FillPattern(int pixels, int pixelOffset)
  {
    for (int j = 0; j < TotalSteps; j += (pixels + pixelOffset))
    {
      for (int i = 0; i < pixels; i++)
      {
        PatternPtr[i + j] = 1;
      }
      for (int i = 0; i < pixelOffset; i++)
      {
        PatternPtr[i + j + pixels] = 0;
      }
    }
  }

  // Cycle through the animation frames
  void Cycle(int pixels, int pixelOffset)
  {
    FillPattern(pixels, pixelOffset);

    // First show the pattern without starting the animation until the start button is pressed

    int colorIntensity = 0;
    for (size_t i = 0; i < 32; i++)
    {
      colorIntensity += analogRead(1);
    }

    colorIntensity = ((colorIntensity / 32) >> 2) - 10;
    color = Color(colorIntensity + 10, 0, 0);
    for (int j = 0; j < TotalSteps; j++)
    {
      uint32_t currentColor;
      if (PatternPtr[j % TotalSteps])
      {
        currentColor = color;
      }
      else
      {
        currentColor = blank;
      }
      setPixelColor(j, currentColor);
    }
    show();
  }

  // Calculate 50% dimmed version of a color (used by ScannerUpdate)
  uint32_t DimColor(uint32_t color)
  {
    // Shift R, G and B components one bit to the right
    uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
    return dimColor;
  }

  // Set all pixels to a color (synchronously)
  void ColorSet(uint32_t color)
  {
    for (int i = 0; i < numPixels(); i++)
    {
      setPixelColor(i, color);
    }
    show();
  }

  // Returns the Red component of a 32-bit color
  uint8_t Red(uint32_t color)
  {
    return (color >> 16) & 0xFF;
  }

  // Returns the Green component of a 32-bit color
  uint8_t Green(uint32_t color)
  {
    return (color >> 8) & 0xFF;
  }

  // Returns the Blue component of a 32-bit color
  uint8_t Blue(uint32_t color)
  {
    return color & 0xFF;
  }

  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos)
  {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
      return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if (WheelPos < 170)
    {
      WheelPos -= 85;
      return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else
    {
      WheelPos -= 170;
      return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
  }
};

NeoPatterns Program(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800, &ProgramComplete);

// Deffine buttons used for program selection
Bounce program1Button = Bounce(PROGRAM_PIN1, 10);
Bounce program2Button = Bounce(PROGRAM_PIN2, 10);
Bounce program3Button = Bounce(PROGRAM_PIN3, 10);
Bounce program4Button = Bounce(PROGRAM_PIN4, 10);
Bounce program5Button = Bounce(PROGRAM_PIN5, 10);
Bounce program6Button = Bounce(PROGRAM_PIN6, 10);
Bounce program7Button = Bounce(PROGRAM_PIN7, 10);
Bounce program8Button = Bounce(PROGRAM_PIN8, 10);
Bounce program9Button = Bounce(PROGRAM_PIN9, 10);
Bounce startButton = Bounce(START_PIN, 10);
Bounce pauseButton = Bounce(PAUSE_PIN, 10);
Bounce stopButton = Bounce(STOP_PIN, 10);

int minDelay = 17;
int analogWait = 0;
bool isOffset = true;
bool isPixels = false;
bool pattern[PIXEL_COUNT];
int currentOffset = 0;
bool stopped = true;
bool paused = false;

void setup()
{
  // put your setup code here, to run once:
  pinMode(START_PIN, INPUT_PULLUP);
  pinMode(PROGRAM_PIN1, INPUT_PULLUP);
  pinMode(PROGRAM_PIN2, INPUT_PULLUP);
  pinMode(PROGRAM_PIN3, INPUT_PULLUP);
  pinMode(PROGRAM_PIN4, INPUT_PULLUP);
  pinMode(PROGRAM_PIN5, INPUT_PULLUP);
  pinMode(PROGRAM_PIN6, INPUT_PULLUP);
  pinMode(PROGRAM_PIN7, INPUT_PULLUP);
  pinMode(PROGRAM_PIN8, INPUT_PULLUP);
  pinMode(PROGRAM_PIN9, INPUT_PULLUP);
  pinMode(PAUSE_PIN, INPUT_PULLUP);
  pinMode(STOP_PIN, INPUT_PULLUP);
  Program.begin();
}

void loop()
{
  if (!paused)
    Program.Update();

  if (pauseButton.update())
  {
    if (pauseButton.fallingEdge())
    {
      paused = !paused;
    }
  }
  if (!stopped)
  {
    // Check for the program to run
    if (program1Button.update())
    {
      if (program1Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM1;
      }
    }
    if (program2Button.update())
    {
      if (program2Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM2;
      }
    }
    if (program3Button.update())
    {
      if (program3Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM3;
      }
    }
    if (program4Button.update())
    {
      if (program4Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM4;
      }
    }
    if (program5Button.update())
    {
      if (program5Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM5;
      }
    }
    if (program6Button.update())
    {
      if (program6Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM6;
      }
    }
    if (program7Button.update())
    {
      if (program7Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM7;
      }
    }
    if (program8Button.update())
    {
      if (program8Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM8;
      }
    }
    if (program9Button.update())
    {
      if (program9Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM9;
      }
    }
  }
}
