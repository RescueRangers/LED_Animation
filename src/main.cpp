#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Bounce.h>

#define PROGRAM_PIN1 2
#define PROGRAM_PIN2 3
#define PROGRAM_PIN3 4
#define PROGRAM_PIN4 5
#define PROGRAM_PIN5 6
#define PROGRAM_PIN6 7
#define PROGRAM_PIN7 8
#define PROGRAM_PIN8 9
#define PROGRAM_PIN9 10
#define PAUSE_PIN 20
#define STOP_PIN 12
#define START_PIN 17

#define PIXEL_PIN 19 // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 899



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

class NeoPatterns : public Adafruit_NeoPixel
{
public:
  // Member Variables:
  program ActivePattern; // which pattern is running

  unsigned long Interval;   // milliseconds between updates
  unsigned long lastUpdate; // last update of position

  uint32_t Color1, Color2; // What colors are in use
  uint16_t Index;          // current step within the pattern

  void (*OnComplete)(); // Callback on completion of pattern

  bool isOffset = true;
  bool isPixels = false;
  bool *pattern_pointer;
  int currentOffset = 0;
  uint16_t pixel, pixelOffset;

  // Constructor - calls base-class constructor to initialize strip
  NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
      : Adafruit_NeoPixel(pixels, pin, type)
  {
    pattern_pointer = new bool[numPixels()];
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

    for (int i = numPixels(); i >= 1; i--)
    {
      pattern_pointer[i] = pattern_pointer[i - 1];
    }
    pattern_pointer[0] = currentColor;
  }

  void FillPattern(int pixels, int pixelOffset)
  {
    for (int j = 0; j < numPixels(); j += (pixels + pixelOffset))
    {
      for (int i = 0; i < pixels; i++)
      {
        pattern_pointer[i + j] = 1;
      }
      for (int i = 0; i < pixelOffset; i++)
      {
        pattern_pointer[i + j + pixels] = 0;
      }
    }
  }

  void ShowPattern()
  {
    for (int j = 0; j < numPixels(); j++)
    {
      uint32_t currentColor;
      if (pattern_pointer[j % numPixels()])
      {
        currentColor = Color1;
      }
      else
      {
        currentColor = Color2;
      }
      setPixelColor(j, currentColor);
    }
    show();
  }
  // Cycle through the animation frames
  void Cycle(int pixels, int pixelOffset)
  {
    for (int j = 0; j < numPixels(); j++)
    {
      uint32_t currentColor;
      if (pattern_pointer[j % numPixels()])
      {
        currentColor = Color1;
      }
      else
      {
        currentColor = Color2;
      }
      setPixelColor(j, currentColor);
    }
    ShiftArray(pixels, pixelOffset);
    show();
    if (OnComplete != NULL)
    {
      OnComplete(); // call the completion callback
    }
  }

  // Set all pixels to a color (synchronously)
  void ColorSet(uint32_t color)
  {
    for (int i = 0; i < numPixels(); i++)
    {
      setPixelColor(i, color);
      pattern_pointer[i] = 0;
    }
    show();
  }
};

void ProgramComplete();

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
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
Bounce pauseButton = Bounce(PAUSE_PIN, 10);
Bounce stopButton = Bounce(STOP_PIN, 10);
Bounce startButton = Bounce(START_PIN, 10);

program ActiveProgram;

int minDelay = 17;
int analogWait = 0;
bool isOffset = true;
bool isPixels = false;
bool pattern[PIXEL_COUNT];
int currentOffset = 0;
bool stopped = true;
bool paused = false;
int prevColorRead = 0;
int colorRead = 10;
int prevWait = 0;

void ProgramComplete()
{
  if (pauseButton.update())
  {
    if (pauseButton.fallingEdge())
    {
      paused = !paused;
    }
  }
  if (stopButton.update())
  {
    if (stopButton.fallingEdge())
    {
      stopped = true;
      paused = true;
      Program.ActivePattern = NONE;
      Program.ColorSet(Program.Color2);
    }
  }

  // Check for color changes
  int colorIntensity = 0;
  for (size_t i = 0; i < 32; i++)
  {
    colorIntensity += analogRead(1);
  }
  colorRead = ((colorIntensity / 32) >> 2);
  if (colorRead != prevColorRead)
  {
    Program.Color1 = Program.Color(constrain(colorRead, 10, 250), 0, 0);
    prevColorRead = colorRead;
  }

  // Check for speed changes
  for (size_t i = 0; i < 32; i++)
  {
    analogWait += analogRead(0);
  }

  analogWait = analogWait / 32;

  if (analogWait != prevWait)
  {
    Program.Interval = minDelay + analogWait;
    prevWait = analogWait;
  }
}

void setup()
{
  // put your setup code here, to run once:
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
  Program.Color1 = Program.Color(10, 0, 0);
  Program.Color2 = Program.Color(0, 0, 0);
  Program.ColorSet(Program.Color2);
  Program.Interval = 1;
}

void loop()
{
  
  if (stopButton.update())
  {
    if (stopButton.fallingEdge())
    {
      stopped = true;
      paused = true;
      Program.ActivePattern = NONE;
      Program.ColorSet(Program.Color2);
    }
  }

  if (stopped)
  {
    // Check for the program to run
    if (program1Button.update())
    {
      if (program1Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM1;
        Program.FillPattern(36, 36);
        paused = true;
        Program.ShowPattern();
      }
    }
    if (program2Button.update())
    {
      if (program2Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM2;
        Program.FillPattern(48, 36);
        paused = true;
        Program.ShowPattern();
      }
    }
    if (program3Button.update())
    {
      if (program3Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM3;
        Program.FillPattern(36, 60);
        paused = true;
        Program.ShowPattern();
      }
    }
    if (program4Button.update())
    {
      if (program4Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM4;
        Program.FillPattern(48, 60);
        paused = true;
        Program.ShowPattern();
      }
    }
    if (program5Button.update())
    {
      if (program5Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM5;
        Program.FillPattern(60, 60);
        paused = true;
        Program.ShowPattern();
      }
    }
    if (program6Button.update())
    {
      if (program6Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM6;
        Program.FillPattern(48, 84);
        paused = true;
        Program.ShowPattern();
      }
    }
    if (program7Button.update())
    {
      if (program7Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM7;
        Program.FillPattern(60, 84);
        paused = true;
        Program.ShowPattern();
      }
    }
    if (program8Button.update())
    {
      if (program8Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM8;
        Program.FillPattern(72, 84);
        paused = true;
        Program.ShowPattern();
      }
    }
    if (program9Button.update())
    {
      if (program9Button.fallingEdge())
      {
        Program.ActivePattern = PROGRAM9;
        Program.FillPattern(60, 108);
        paused = true;
        Program.ShowPattern();
      }
    }
  }

  if (startButton.update())
  {
    if (startButton.fallingEdge())
    {
      paused = false;
      stopped = false;
    }
  }

  if (stopped == false && pauseButton.update())
  {
    if (pauseButton.fallingEdge())
    {
      paused = !paused;
    }
  }

  if (!paused)
  {
    Program.Update();
  }
}
