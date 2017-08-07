#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Bounce.h>

#define AUDIO_THRU
#define I2C_ADDR          123 

const int NUM_LEDS(3);
const int LED_PINS[NUM_LEDS] = { 7, 11, 29 };

const int NUM_SWITCHES(2);
const int SWITCH_PINS[NUM_SWITCHES] = { 1, 2 };

Bounce SWITCH_BOUNCE[NUM_SWITCHES] = { Bounce( SWITCH_PINS[0], 10 ), Bounce( SWITCH_PINS[1], 10 ) };



#ifdef AUDIO_THRU
AudioInputAnalog      audio_input(A0);
AudioOutputAnalog     analog_out;   // must be constructed after AudioInputAnalog
AudioConnection       patchCord1(audio_input, 0,  analog_out, 0);
#else
AudioSynthWaveform    waveform;
AudioOutputAnalog     analog_out;   // must be constructed after AudioInputAnalog
AudioConnection       patchCord1(waveform, 0,  analog_out, 0);
#endif

void setup()
{
  AudioMemory(12);

  Wire.begin();
  
  Serial.begin(9600);

  pinMode( LED_BUILTIN, OUTPUT );

  for( int i = 0; i < NUM_LEDS; ++i )
  {
    pinMode( LED_PINS[i], OUTPUT );
  }

  for( int i = 0; i < NUM_SWITCHES; ++i )
  {
    pinMode( SWITCH_PINS[i], INPUT_PULLUP );
  }

#ifdef AUDIO_THRU
  analogReference(INTERNAL);
  digitalWrite( LED_BUILTIN, HIGH );
#else
  waveform.begin(WAVEFORM_SINE);
#endif
  
  delay(1000);

  Serial.println("Setup");
}

void cycle_leds()
{
  static int led = 0;

  for( int i = 0; i < NUM_LEDS; ++i )
  {
    if( i == led )
    {
      digitalWrite( LED_PINS[i], HIGH );
    }
    else
    {
      digitalWrite( LED_PINS[i], LOW );
    }
  }

  led = ( led + 1 ) % NUM_LEDS;
}

void test_switches()
{
  for( int i = 0; i < NUM_SWITCHES; ++i )
  {
    SWITCH_BOUNCE[i].update();

    if( SWITCH_BOUNCE[i].fallingEdge() )
    {
      Serial.print("Switch ");
      Serial.print(i);
      Serial.println(" down");
    }

    if( SWITCH_BOUNCE[i].risingEdge() )
    {
      Serial.print("Switch ");
      Serial.print(i);
      Serial.println(" up");
    }
  }
}

void loop()
{ 
  Wire.requestFrom(123, 16);    // request

  unsigned char x[16];
  int i;
  for( i=0; i<16; ++i )
  {
    x[i] = Wire.read(); // receive a byte as character
  }


  for( i=12; i<14; )
  {
    unsigned int b = x[i++];
    b |= ((unsigned int)x[i++])<<8;
    Serial.print(b);
    Serial.print(",");
  }
  
  Serial.println("");


  const uint16_t pot = x[12] | ( x[13] << 8 );
  const float speed = pot / 1024.0f;
  const uint16_t time = 50 + ( 1000 * speed );

  static bool led_on = true;
  led_on = !led_on;
  if( led_on )
  {
 #ifndef AUDIO_THRU
    waveform.frequency(440);
    waveform.amplitude(0.9);
 #endif
    
    digitalWrite( LED_BUILTIN, HIGH );
  }
  else
  {
 #ifndef AUDIO_THRU
    waveform.amplitude(0);
#endif
    
    digitalWrite( LED_BUILTIN, LOW );
  }

  cycle_leds();

  test_switches();
  
  delay(time);
}

