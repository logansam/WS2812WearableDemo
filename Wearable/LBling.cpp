/************************************************************************/
/*                                                                      */
/*    LBling.pde -- Larissa's Bling                                     */
/*                                                                      */
/*    Test code to verify the WS2812 bit stream                         */
/*                                                                      */
/************************************************************************/
/*    Author:     Keith Vogel                                           */
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

#define CPATTERNS 3
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
#define MSSHIFT 250

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
            memcpy(&grbT, &pGRB[CDEVICES-1], sizeof(WS2812::GRB));
            for(int i = CDEVICES-2; i>=0; i--)
            {
                memcpy(&pGRB[i+1], &pGRB[i], sizeof(WS2812::GRB));
            }
            memcpy(&pGRB[0], &grbT, sizeof(WS2812::GRB));
            bstate = WAIT;
            break;

        case WAIT:
            if(millis() - tWaitShift >= MSSHIFT)
            {
                tWaitShift = millis();
                pGRB = pGRBNew;
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
        pGRBNew = rgPatrgGRB[i];
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
}
