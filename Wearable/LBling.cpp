/************************************************************************/
/*                                                                      */
/*    LBling.pde -- Larissa's Bling                                     */
/*                                                                      */
/*    Test code to verify the WS2812 bit stream                         */
/*                                                                      */
/************************************************************************/
/*    Author:     Keith Vogel, Brian L. Thomas                                           */
/*    Copyright 2014, Digilent Inc.                                     */
/************************************************************************/
/*
*
* Copyright (c) 2014, Digilent <www.digilentinc.com>
* Contact Digilent for the latest version.
*
* This program is free software; distributed under the terms of
* BSD 3-clause license ("Revised BSD License", "New BSD License", or "Modified BSD License")
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1.    Redistributions of source code must retain the above copyright notice, this
*        list of conditions and the following disclaimer.
* 2.    Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
* 3.    Neither the name(s) of the above-listed copyright holder(s) nor the names
*        of its contributors may be used to endorse or promote products derived
*        from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/************************************************************************/
/*  Revision History:                                                   */
/*                                                                      */
/*    5/1/2014(KeithV): Created                                        */
/*    5/16/2014(BLThomas): Rewrote LED patterns                             */
/************************************************************************/
/************************************************************************/
/*                                                                      */
/*  Supports the WS2812 signaling to drive up to 1000 devices           */
/*                                                                      */
/************************************************************************/
/************************************************************************/
/*                       Supported hardware:                            */
/*                                                                      */
/*  chipKIT WF32  Dout pin 11; unusable pins 12, 13-LED1                */
/*  chipKIT Max32 Dout pin 43 or (51 with JP4 in master);               */
/*          unusable pins 29,50,52                                      */
/*                                                                      */
/*  WARNING: currently this code assumes SPI2. Of the chipKIT boards    */
/*  this works on (WF32/Max32), the standard Arduino SPI just happens   */
/*  to always be SPI2, but this is NOT generalized code!                */
/*                                                                      */
/*  The spec says that Dout Vih = .7Vdd and Vdd = 6v-7v However...      */
/*  it seems to work with Vdd == 4.5v -> 5v and Vih == 3.3              */
/*  But this is "out of spec" operation. If you must be in              */
/*  spec you will need to put a level shifter on Dout to bring 3.3v     */
/*  up to .7Vdd. If your level shifter also inverts the data signal     */
/*  you can specify fInvert=true on begin() to invert the 3.3v signal   */
/*                                                                      */
/************************************************************************/
#include <WS2812.h>
#include "Helper.h"

#define CPATTERNS 6
#define CDEVICES 60

#if (CPATTERNS > 6)
#error Only support a max of 6 patterns
#endif

WS2812::GRB rgPatrgGRB[CPATTERNS][CDEVICES] =
{
{
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {255,   0,      0       },
    {255,   0,      0       },
    {85,    85,     85      },
    {85,    85,     85      }
},
{
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {0,     255,    0       },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {85,    85,     85      },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     },
    {0,     0,      255     }
},
{
    {0,     255,    0       },
    {127,   255,    0       },
    {255,   255,    0       },
    {255,   0,      0       },
    {0,     0,      255     },
    {0,     75,     130     },
    {0,     143,    255     },
    {0,     0,      0       },
    {85,    85,     85      },
    {0,     0,      0       },
    {0,     255,    0       },
    {127,   255,    0       },
    {255,   255,    0       },
    {255,   0,      0       },
    {0,     0,      255     },
    {0,     75,     130     },
    {0,     143,    255     },
    {0,     0,      0       },
    {85,    85,     85      },
    {0,     0,      0       },
    {0,     255,    0       },
    {127,   255,    0       },
    {255,   255,    0       },
    {255,   0,      0       },
    {0,     0,      255     },
    {0,     75,     130     },
    {0,     143,    255     },
    {0,     0,      0       },
    {85,    85,     85      },
    {0,     0,      0       },
    {0,     255,    0       },
    {127,   255,    0       },
    {255,   255,    0       },
    {255,   0,      0       },
    {0,     0,      255     },
    {0,     75,     130     },
    {0,     143,    255     },
    {0,     0,      0       },
    {85,    85,     85      },
    {0,     0,      0       },
    {0,     255,    0       },
    {127,   255,    0       },
    {255,   255,    0       },
    {255,   0,      0       },
    {0,     0,      255     },
    {0,     75,     130     },
    {0,     143,    255     },
    {0,     0,      0       },
    {85,    85,     85      },
    {0,     0,      0       },
    {0,     255,    0       },
    {127,   255,    0       },
    {255,   255,    0       },
    {255,   0,      0       },
    {0,     0,      255     },
    {0,     75,     130     },
    {0,     143,    255     },
    {0,     0,      0       },
    {85,    85,     85      },
    {0,     0,      0       }
}
};

WS2812::GRB * pGRB      = rgPatrgGRB[0];
WS2812::GRB * pGRBNew   = rgPatrgGRB[0];

WS2812      ws2812;
uint8_t     rgbPatternBuffer[CBWS2812PATBUF(CDEVICES)];

typedef enum {
    LOADPAT,
    SHIFT,
    WAIT,
    SPIN
} BSTATE;

static BSTATE bstate = LOADPAT;
static uint32_t tWaitShift = 0;
#define MSSHIFT 25 //250

byte imgData[CDEVICES * 3];
byte fxIdx = 0;
byte r,g,b;
int fxVars[50];
byte *imgPtr;


void reConfetti();
void reUSAConfetti();
void reBeadChase();
void reRainbowWrap();
void reDigilentSineChase();
void reDigilentBeadChase();
void reDigilentConfetti();
void reDigilentSolidTwinkle();
byte gamma(byte x);
long hsv2rgb(long h, byte s, byte v);
char fixSin(int angle);
char fixCos(int angle);




/***    void lblingSetup(void) 
 *
 *    Parameters:
 *          None
 *              
 *    Return Values:
 *          None
 *
 *    Description: 
 *    
 *      Initialize the LED output pins and SPI and DMA
 *      
 * ------------------------------------------------------------ */
void lblingSetup(void) 
{
    memset(imgData, 0, sizeof(imgData));
    ws2812.begin(CDEVICES, rgbPatternBuffer, sizeof(rgbPatternBuffer), false);
    tWaitShift = millis();
}

/***    void lblingPeriodicTask(void)
 *
 *    Parameters:
 *          None
 *              
 *    Return Values:
 *          None
 *
 *    Description: 
 *    
 *      Shifts the LED pattern
 *      And updates to a new pattern if one is
 *      selected.
 *
 *      This should be run whenenver there is
 *      a possibility of an update and on
 *      a regular basis to case the pattern to shift
 *
 * ------------------------------------------------------------ */
void lblingPeriodicTask(void) 
{
    WS2812::GRB grbT;

    switch(bstate)
    {
        case LOADPAT:
            if(ws2812.updateLEDs(pGRB))
            {
                bstate = SHIFT;
            }
            break;

        case SHIFT:
            // Always render based on current effect index
            switch(fxIdx)
            {
              case 0:
                reRainbowWrap();
                break;
              case 1:
                reDigilentConfetti();
                break;
              case 2:
                reConfetti();
                break;
              case 3:
                reDigilentSolidTwinkle();
                break;
              case 4:
                reBeadChase();
                break;
              case 5:
                reUSAConfetti();
                break;
              default:
                break;
            }
            imgPtr = &imgData[0];
            for (int i=0; i<CDEVICES; i++)
            {
              /*r = *imgPtr++;
              g = *imgPtr++;
              b = *imgPtr++;
              */
              WS2812::GRB pixel;
              
              pixel.red = (uint8_t) *imgPtr++;
              pixel.green = (uint8_t) *imgPtr++;
              pixel.blue = (uint8_t) *imgPtr++;
              //memcpy(&pixel, &pGRB[i], sizeof(WS2812::GRB));
              memcpy(&pGRB[i], &pixel, sizeof(WS2812::GRB));
            }
            /*memcpy(&grbT, &pGRB[CDEVICES-1], sizeof(WS2812::GRB));
            for(int i = CDEVICES-2; i>=0; i--)
            {
                memcpy(&pGRB[i+1], &pGRB[i], sizeof(WS2812::GRB));
            }
            memcpy(&pGRB[0], &grbT, sizeof(WS2812::GRB));
            */
            bstate = WAIT;
            break;
        case WAIT:
            if(millis() - tWaitShift >= MSSHIFT)
            {
                tWaitShift = millis();
                //pGRB = pGRBNew;
                bstate = LOADPAT;
            }
            break;

        case SPIN:
        default:
            break;

    }
}

/***    void lblingUpdate(char chPattern)
 *
 *    Parameters:
 *          chPattern
 *              takes a number from 1 to the number
 *              of supported patterns (max 6)
 *              and switch the pattern to the selected pattern
 *
 *    Return Values:
 *          None
 *
 *    Description:
 *
 *      Changes the pattern and updated the on board
 *      LEDs to reflect the pattern number
 *
 * ------------------------------------------------------------ */
void lblingUpdate(char chPattern)
{
    int i = chPattern - '1';
    
    if(0<= i && i<CPATTERNS)
    {
        if ((byte)i != fxIdx)
        {
          fxIdx = (byte)i;
          fxVars[0] = 0;  // Effect not yet initialized
          pGRBNew = rgPatrgGRB[i];
        }
        SetLEDValue(i+1);
    }
}

/***    void lblingBump(void)
 *
 *    Parameters:
 *          None
 *
 *    Return Values:
 *          None
 *
 *    Description:
 *
 *      Manually bumps the pattern up to the
 *      next pattern. This will usually be overwritten
 *      on the next pattern request from the server
 *      but if the server is down allows for the pattern
 *      to be changed.
 *
 * ------------------------------------------------------------ */
void lblingBump(void)
{
    pGRBNew += CDEVICES;

    if(pGRBNew >= rgPatrgGRB[CPATTERNS])
    {
        pGRBNew = rgPatrgGRB[0];
    }
    fxIdx++;
    if (fxIdx >= CPATTERNS)
    {
      fxIdx = 0;
    }
    fxVars[0] = 0;  // Effect not yet initialized.
    SetLEDValue((int)fxIdx+1);
}

void lblingResetPattern(void)
{
  fxVars[0] = 0;
}


void reConfetti()
{
  long i;
  byte *ptr;
  if(fxVars[0] == 0) { // Initialize effect?
    fxVars[1] = random(1536); // Random hue
    // Probability of adding a new sparkle.
    fxVars[2] = 60 + random(50);
    // Rate to fade out pixel.
    fxVars[3] = 2 + random(3);
    // Initialize the pattern to black LEDs.
    ptr = &imgData[0];
    for(long i=0; i<CDEVICES; i++) {
      *ptr++ = 0; 
      *ptr++ = 0; 
      *ptr++ = 0;
    }
    fxVars[0] = 1; // Effect initialized
  }

  ptr = &imgData[0];
  byte a,b,c;
  long color;
  for(long i=0; i<CDEVICES; i++) {
    a = *ptr++;
    b = *ptr++;
    c = *ptr++;
    if (a > byte(fxVars[3])) {
      a = a - byte(fxVars[3]);
    } 
    else {
      a = 0;
    }
    if (b > byte(fxVars[3])) {
      b = b - byte(fxVars[3]);
    } 
    else {
      b = 0;
    }
    if (c > byte(fxVars[3])) {
      c = c - byte(fxVars[3]);
    } 
    else {
      c = 0;
    }
    ptr--;
    ptr--;
    ptr--;
    
    if (random(fxVars[2]) == 0) {
      color = hsv2rgb(random(1536), 255, 255);  // Random hue
      *ptr++ = color >> 16; 
      *ptr++ = color >> 8; 
      *ptr++ = color;
    }
    else {
      *ptr++ = a;
      *ptr++ = b;
      *ptr++ = c;
    }
  }
}

void reUSAConfetti()
{
  long i;
  byte *ptr;
  if(fxVars[0] == 0) { // Initialize effect?
    fxVars[1] = random(1536); // Random hue
    // Probability of adding a new sparkle.
    fxVars[2] = 60 + random(50);
    // Rate of fade out.
    fxVars[3] = 2;
    ptr = &imgData[0];
    for(long i=0; i<CDEVICES; i++) {
      *ptr++ = 0; 
      *ptr++ = 0; 
      *ptr++ = 0;
    }
    fxVars[0] = 1; // Effect initialized
  }

  ptr = &imgData[0];
  byte a,b,c,usa;
  long color;
  for(long i=0; i<CDEVICES; i++) {
    if (random(fxVars[2]) == 0) {
      usa = random(3);
      if (usa == 0) { // Red
        *ptr++ = 255;
        *ptr++ = 0;
        *ptr++ = 0;
      } else if (usa == 1) { // White
        *ptr++ = 200;
        *ptr++ = 200;
        *ptr++ = 200;
      } else { // Blue
        *ptr++ = 0;
        *ptr++ = 0;
        *ptr++ = 255;
      }
    } 
    else {
      a = *ptr++;
      b = *ptr++;
      c = *ptr++;
      if (a > byte(fxVars[3])) {
        a = a - byte(fxVars[3]);
      } 
      else {
        a = 0;
      }
      if (b > byte(fxVars[3])) {
        b = b - byte(fxVars[3]);
      } 
      else {
        b = 0;
      }
      if (c > byte(fxVars[3])) {
        c = c - byte(fxVars[3]);
      } 
      else {
        c = 0;
      }
      ptr--;
      ptr--;
      ptr--;
      *ptr++ = a;
      *ptr++ = b;
      *ptr++ = c;
    }
  }
}


void reBeadChase()
{
  long i,j;
  byte *ptr;
  if(fxVars[0] == 0) { // Initialize effect?
    fxVars[1] = 6 + random(4); // Number of beads
    fxVars[2] = 4 + random(8); // Fade out speed
    for(long i=0; i<long(fxVars[1]); i++) {
      fxVars[3+i*3] = random(1536); // Random hue
      fxVars[3+i*3+1] = 3 + random(8); // Speed
      // Positions are represented on a number scale 20*CDEVICES
      fxVars[3+i*3+2] = random(20*CDEVICES);
      // Reverse direction half the time.
      if(random(2) == 0) fxVars[3+i*3+1] = -fxVars[3+i*3+1];
    }
    ptr = &imgData[0];
    for(long i=0; i<CDEVICES; i++) {
      *ptr++ = 0; 
      *ptr++ = 0; 
      *ptr++ = 0;
    }
    fxVars[0] = 1; // Effect initialized
  }

  ptr = &imgData[0];
  byte a,b,c;
  long color;
  boolean found = false;
  // Dim all the pixels on the belt before placing beads with full
  // brightness at their current locations.
  for(long i=0; i<CDEVICES; i++) {
    a = *ptr++;
    b = *ptr++;
    c = *ptr++;

    if (a > 150) {
      a = 150;
    }
    if (a > byte(fxVars[2])) {
      a = a - byte(fxVars[2]);
    } 
    else {
      a = 0;
    }

    if (b > 150) {
      b = 150;
    }
    if (b > byte(fxVars[2])) {
      b = b - byte(fxVars[2]);
    } 
    else {
      b = 0;
    }

    if (c > 150) {
      c = 150;
    }
    if (c > byte(fxVars[2])) {
      c = c - byte(fxVars[2]);
    } 
    else {
      c = 0;
    }
    ptr--;
    ptr--;
    ptr--;
    *ptr++ = a;
    *ptr++ = b;
    *ptr++ = c;
  }

  ptr = &imgData[0];
  for (long i=0; i<CDEVICES; i++)
  {
    // Using bool 'found' to ensure that the ptr is only
    // incremented once during each iteration of the
    // outer loop
    found = false;
    for (long j=0; j<long(fxVars[1]); j++) {
      if (!found) {
        if (i == fxVars[3+j*3+2]/20) {
          color = hsv2rgb(fxVars[3+j*3], 255, 255);
          *ptr++ = color >> 16; 
          *ptr++ = color >> 8; 
          *ptr++ = color;
          found = true;
        }
      }
    }

    if (!found) {
      ptr++;
      ptr++;
      ptr++;
    }
  }

  // Update positions for all the beads
  for (long j=0; j<long(fxVars[1]); j++) {
    // Position + Speed mod by 20x length of the belt
    fxVars[3+j*3+2] = (fxVars[3+j*3+2] + fxVars[3+j*3+1]) % (20*CDEVICES);
    if (fxVars[3+j*3+2] < 0) {
      fxVars[3+j*3+2] = (20*CDEVICES - 1) - fxVars[3+j*3+2];
    }
  }
}

// Rainbow effect (1 or more full loops of color wheel at 100% saturation).
// Not a big fan of this pattern (it's way overused with LED stuff), but it's
// practically part of the Geneva Convention by now.
void reRainbowWrap()
{
  if(fxVars[0] == 0) { // Initialize effect?
    // Number of repetitions (complete loops around color wheel); any
    // more than 4 per meter just looks too chaotic and un-rainbow-like.
    // Store as hue 'distance' around complete belt:
    fxVars[1] = (1 + random(3 * ((CDEVICES + 31) / 32))) * 1536;
    // Frame-to-frame hue increment (speed) -- may be positive or negative,
    // but magnitude shouldn't be so small as to be boring.  It's generally
    // still less than a full pixel per frame, making motion very smooth.
    fxVars[2] = 10 + random(fxVars[1]) / CDEVICES;
    // Reverse speed and hue shift direction half the time.
    if(random(2) == 0) fxVars[1] = -fxVars[1];
    if(random(2) == 0) fxVars[2] = -fxVars[2];
    fxVars[3] = 0; // Current position
    fxVars[0] = 1; // Effect initialized
  }

  byte *ptr = &imgData[0];
  long color, i;
  for(i=0; i<CDEVICES; i++) {
    color = hsv2rgb(fxVars[3] + fxVars[1] * i / CDEVICES, 255, 255);
    *ptr++ = color >> 16;
    *ptr++ = color >> 8;
    *ptr++ = color;
  }
  fxVars[3] += fxVars[2];
}

void reDigilentSineChase()
{
    if(fxVars[0] == 0) { // Initialize effect?
    // Number of repetitions (complete loops around color wheel); any
    // more than 4 per meter just looks too chaotic and un-rainbow-like.
    // Store as hue 'distance' around complete belt:
    fxVars[1] = (1 + random(3 * ((CDEVICES + 31) / 32))) * 1536;
    // Frame-to-frame hue increment (speed) -- may be positive or negative,
    // but magnitude shouldn't be so small as to be boring.  It's generally
    // still less than a full pixel per frame, making motion very smooth.
    fxVars[2] = 5 + random(fxVars[1]) / CDEVICES;
    // Reverse speed and hue shift direction half the time.
    if(random(2) == 0) fxVars[1] = -fxVars[1];
    if(random(2) == 0) fxVars[2] = -fxVars[2];
    fxVars[3] = 0; // Current position
    fxVars[0] = 1; // Effect initialized
  }

  byte *ptr = &imgData[0];
  long color, i;
  for(i=0; i<CDEVICES; i++) {
    color = hsv2rgb(fxVars[3] + fxVars[1] * i / CDEVICES, 255, 255);
    *ptr++ = 0;
    *ptr++ = color >> 8;
    *ptr++ = color;
  }
  fxVars[3] += fxVars[2];

}

void reDigilentBeadChase()
{
}

void reDigilentConfetti()
{
  long i;
  byte *ptr;
  if(fxVars[0] == 0) { // Initialize effect?
    fxVars[1] = random(1536); // Random hue
    // Probability of adding a new sparkle.
    fxVars[2] = 60 + random(50);
    fxVars[3] = 2 + random(3);
    ptr = &imgData[0];
    for(long i=0; i<CDEVICES; i++) {
      *ptr++ = 0; 
      *ptr++ = 0; 
      *ptr++ = 0;
    }
    fxVars[0] = 1; // Effect initialized
  }

  ptr = &imgData[0];
  byte a,b,c;
  long color;
  for(long i=0; i<CDEVICES; i++) {
    if (random(fxVars[2]) == 0) {
      if (random(3) == 0) {
        *ptr++ = 200;
        *ptr++ = 200;
        *ptr++ = 200;
      } 
      else {
        *ptr++ = 0;
        *ptr++ = 255;
        *ptr++ = 0;
      }
    } 
    else {
      a = *ptr++;
      b = *ptr++;
      c = *ptr++;
      if (a > byte(fxVars[3])) {
        a = a - byte(fxVars[3]);
      } 
      else {
        a = 0;
      }
      if (b > byte(fxVars[3])) {
        b = b - byte(fxVars[3]);
      } 
      else {
        b = 0;
      }
      if (c > byte(fxVars[3])) {
        c = c - byte(fxVars[3]);
      } 
      else {
        c = 0;
      }
      ptr--;
      ptr--;
      ptr--;
      *ptr++ = a;
      *ptr++ = b;
      *ptr++ = c;
    }
  }
}

void reDigilentSolidTwinkle()
{
  long i;
  byte *ptr, a, b, c;
  if(fxVars[0] == 0) { // Initialize effect?
    fxVars[1] = random(1536); // Random hue
    // Probability of adding a new sparkle.
    fxVars[2] = 20 + random(50);
    // Rate of fade out to background color.
    fxVars[3] = 7 + random(10);
    fxVars[4] = 10;
    fxVars[5] = 240;
    fxVars[6] = 0;
    ptr = &imgData[0];
    for(long i=0; i<CDEVICES; i++) {
      *ptr++ = byte(fxVars[4]);
      *ptr++ = byte(fxVars[5]);
      *ptr++ = byte(fxVars[6]);
    }
    fxVars[0] = 1; // Effect initialized
  }

  ptr = &imgData[0];
  long color;
  for(long i=0; i<CDEVICES; i++) {
    if (random(fxVars[2]) == 0) {
      *ptr++ = 255;
      *ptr++ = 255;
      *ptr++ = 255;
    } 
    else {
      a = *ptr++;
      b = *ptr++;
      c = *ptr++;
      if (a < (byte(fxVars[4]) - byte(fxVars[3]))) {
        a = a + byte(fxVars[3]);
      } 
      else if (a > (byte(fxVars[4]) + byte(fxVars[3]))) {
        a = a - byte(fxVars[3]);
      } 
      else {
        a = byte(fxVars[4]);
      }
      if (b < (byte(fxVars[5]) - byte(fxVars[3]))) {
        b = b + byte(fxVars[3]);
      } 
      else if (b > (byte(fxVars[5]) + byte(fxVars[3]))) {
        b = b - byte(fxVars[3]);
      } 
      else {
        b = byte(fxVars[5]);
      }
      if (c < (byte(fxVars[6]) - byte(fxVars[3]))) {
        c = c + byte(fxVars[3]);
      } 
      else if (c > (byte(fxVars[6]) + byte(fxVars[3]))) {
        c = c - byte(fxVars[3]);
      } 
      else {
        c = byte(fxVars[6]);
      }
      ptr--;
      ptr--;
      ptr--;
      *ptr++ = a;
      *ptr++ = b;
      *ptr++ = c;
    }
  }
}


// ---------------------------------------------------------------------------
// Assorted fixed-point utilities below this line.  Not real interesting.

// Gamma correction compensates for our eyes' nonlinear perception of
// intensity.  It's the LAST step before a pixel value is stored, and
// allows intermediate rendering/processing to occur in linear space.
// The table contains 256 elements (8 bit input), though the outputs are
// only 7 bits (0 to 127).  This is normal and intentional by design: it
// allows all the rendering code to operate in the more familiar unsigned
// 8-bit colorspace (used in a lot of existing graphics code), and better
// preserves accuracy where repeated color blending operations occur.
// Only the final end product is converted to 7 bits, the native format
// for the LPD8806 LED driver.  Gamma correction and 7-bit decimation
// thus occur in a single operation.
byte gammaTable[]  = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,
  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  7,  7,
  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11,
  11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16,
  16, 17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22,
  23, 23, 24, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30,
  30, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 37, 37, 38, 38, 39,
  40, 40, 41, 41, 42, 43, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50,
  50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 58, 58, 59, 60, 61, 62,
  62, 63, 64, 65, 66, 67, 67, 68, 69, 70, 71, 72, 73, 74, 74, 75,
  76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
  92, 93, 94, 95, 96, 97, 98, 99,100,101,102,104,105,106,107,108,
  109,110,111,113,114,115,116,117,118,120,121,122,123,125,126,127
};

// This function (which actually gets 'inlined' anywhere it's called)
// exists so that gammaTable can reside out of the way down here in the
// utility code...didn't want that huge table distracting or intimidating
// folks before even getting into the real substance of the program, and
// the compiler permits forward references to functions but not data.
inline byte gamma(byte x) {
  return gammaTable[x];
}

// Fixed-point colorspace conversion: HSV (hue-saturation-value) to RGB.
// This is a bit like the 'Wheel' function from the original strandtest
// code on steroids.  The angular units for the hue parameter may seem a
// bit odd: there are 1536 increments around the full color wheel here --
// not degrees, radians, gradians or any other conventional unit I'm
// aware of.  These units make the conversion code simpler/faster, because
// the wheel can be divided into six sections of 256 values each, very
// easy to handle on an 8-bit microcontroller.  Math is math, and the
// rendering code elsehwere in this file was written to be aware of these
// units.  Saturation and value (brightness) range from 0 to 255.
long hsv2rgb(long h, byte s, byte v) {
  byte r, g, b, lo;
  int  s1;
  long v1;

  // Hue
  h %= 1536;           // -1535 to +1535
  if(h < 0) h += 1536; //     0 to +1535
  lo = h & 255;        // Low byte  = primary/secondary color mix
  switch(h >> 8) {     // High byte = sextant of colorwheel
  case 0 : 
    r = 255     ; 
    g =  lo     ; 
    b =   0     ; 
    break; // R to Y
  case 1 : 
    r = 255 - lo; 
    g = 255     ; 
    b =   0     ; 
    break; // Y to G
  case 2 : 
    r =   0     ; 
    g = 255     ; 
    b =  lo     ; 
    break; // G to C
  case 3 : 
    r =   0     ; 
    g = 255 - lo; 
    b = 255     ; 
    break; // C to B
  case 4 : 
    r =  lo     ; 
    g =   0     ; 
    b = 255     ; 
    break; // B to M
  default: 
    r = 255     ; 
    g =   0     ; 
    b = 255 - lo; 
    break; // M to R
  }

  // Saturation: add 1 so range is 1 to 256, allowig a quick shift operation
  // on the result rather than a costly divide, while the type upgrade to int
  // avoids repeated type conversions in both directions.
  s1 = s + 1;
  r = 255 - (((255 - r) * s1) >> 8);
  g = 255 - (((255 - g) * s1) >> 8);
  b = 255 - (((255 - b) * s1) >> 8);

  // Value (brightness) and 24-bit color concat merged: similar to above, add
  // 1 to allow shifts, and upgrade to long makes other conversions implicit.
  v1 = v + 1;
  return (((r * v1) & 0xff00) << 8) |
    ((g * v1) & 0xff00)       |
    ( (b * v1)           >> 8);
}

// The fixed-point sine and cosine functions use marginally more
// conventional units, equal to 1/2 degree (720 units around full circle),
// chosen because this gives a reasonable resolution for the given output
// range (-127 to +127).  Sine table intentionally contains 181 (not 180)
// elements: 0 to 180 *inclusive*.  This is normal.

byte sineTable[181]  = {
  0,  1,  2,  3,  5,  6,  7,  8,  9, 10, 11, 12, 13, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25, 27, 28, 29, 30, 31, 32, 33, 34,
  35, 36, 37, 38, 39, 40, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
  67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 77, 78, 79, 80, 81,
  82, 83, 83, 84, 85, 86, 87, 88, 88, 89, 90, 91, 92, 92, 93, 94,
  95, 95, 96, 97, 97, 98, 99,100,100,101,102,102,103,104,104,105,
  105,106,107,107,108,108,109,110,110,111,111,112,112,113,113,114,
  114,115,115,116,116,117,117,117,118,118,119,119,120,120,120,121,
  121,121,122,122,122,123,123,123,123,124,124,124,124,125,125,125,
  125,125,126,126,126,126,126,126,126,127,127,127,127,127,127,127,
  127,127,127,127,127
};

char fixSin(int angle) {
  angle %= 720;               // -719 to +719
  if(angle < 0) angle += 720; //    0 to +719
  return (angle <= 360) ?
    sineTable[(angle <= 180) ?
    angle          : // Quadrant 1
    (360 - angle)] : // Quadrant 2
  -sineTable[(angle <= 540) ?
  (angle - 360)   : // Quadrant 3
  (720 - angle)] ; // Quadrant 4
}

char fixCos(int angle) {
  angle %= 720;               // -719 to +719
  if(angle < 0) angle += 720; //    0 to +719
  return (angle <= 360) ?
  ((angle <= 180) ?  sineTable[180 - angle]  : // Quad 1
  -sineTable[angle - 180]) : // Quad 2
  ((angle <= 540) ? -sineTable[540 - angle]  : // Quad 3
  sineTable[angle - 540]) ; // Quad 4
}
































