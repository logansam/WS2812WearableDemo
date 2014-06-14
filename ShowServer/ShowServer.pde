/************************************************************************/
/*                                                                      */
/*    WebServer                                                         */
/*                                                                      */
/*    A Example chipKIT HTTP Server implementation                      */
/*    This sketch is designed to work with web browser clients          */
/*                                                                      */
/************************************************************************/
/*    Author:     Keith Vogel                                           */
/*    Copyright 2013, Digilent Inc.                                     */
/************************************************************************/
/* 
*
* Copyright (c) 2013-2014, Digilent <www.digilentinc.com>
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
/*    7/15/2013(KeithV): Created                                        */
/************************************************************************/
/************************************************************************/
/*                                                                      */
/*  This HTTP Server Example contains sample content in the content     */
/*  subdirectory under the sketch directory. The files in the content   */
/*  directory should be copied to the root of your SD card. However,    */
/*  you can view the content by opening HomePage.htm page               */
/*  with your browser right out of the content subdirectory before      */
/*  loading it on the SD card. The sample pages contain                 */
/*  instructions on how to set up this example and therefore may be     */
/*  valuable to read before proceeding.                                 */
/*                                                                      */
/************************************************************************/
/************************************************************************/
/*                       Supported hardware:                            */
/*                                                                      */
/*  Most / All Uno32 Shield form factor boards with a WiFiShield        */
/*          For example the Uno32, uC32, WF32                           */
/*  Max32 with a WiFiShield                                             */
/*  MX7cK with a pmodSD on JPF                                          */
/*                                                                      */
/*  NOTE: you can NOT stack a NetworkShield and a WiFiShield on a Max32 */
/************************************************************************/

//************************************************************************
//************************************************************************
//********  SET THESE LIBRARIES FOR YOUR HARDWARE CONFIGURATION **********
//************************************************************************
//************************************************************************

/************************************************************************/
/*                                                                      */
/*    Network Hardware libraries                                        */
/*    INCLUDE ONLY ONE                                                  */
/*                                                                      */
/************************************************************************/
// You MUST select 1 and ONLY 1 of the following hardware libraries
// they are here so that MPIDE will put the lib path on the compiler include path.
#include <MRF24G.h>                         // This is for the MRF24WGxx on a pmodWiFi or WiFiShield
//#include <IM8720PHY.h>                      // This is for the the Internal MAC and SMSC 8720 PHY

/************************************************************************/
/*    Network libraries                                                 */
/************************************************************************/

// this will increase our socket buffers to 200 pages of 64 bytes
// or 12,800 bytes of socket space; this number must not exceed 254
//#define cPagesSocketBuffer 200

// The base network library
// this is a required library
// Do not comment out this library
#include <DEIPcK.h>

//  -----  COMMENT THIS OUT IF YOU ARE NOT USING WIFI  -----
#include <DEWFcK.h>

//************************************************************************
//************************************************************************
//**************** END OF LIBRARY CONFIGURATION **************************
//************************************************************************
//************************************************************************

/************************************************************************/
/*                                                                      */
/*           YOU MUST.....                                              */
/*                                                                      */
/*    You MUST put HTTPServerConfig.h in your sketch director           */
/*    And you MUST configure it with your network parameters            */
/*                                                                      */
/*    You also MUST load your content onto your SD card and             */
/*    the file HomePage.htm MUST exist at the root of the SD            */
/*    file structure. Of course you must insert your SD card            */
/*    into the SD reader on the chipKIT board                           */
/*                                                                      */
/*    Go do this now....                                                */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*    Other libraries; Required libraries                               */
/************************************************************************/
// You must have an SD card reader somewhere
// as the HTTP server uses the SD card to hold the HTML pages.
// this is a required library
#include    <SD.h>

// and this is the HTTPServer library code.
// this is a required library
#include    <HTTPServer.h>

static bool fDebounce = false;
static uint32_t tDebounce = 0;
#define msDebounce 500

/************************************************************************/
/*    HTTP URL Matching Strings                                         */
/************************************************************************/
// These are the HTTP URL match strings for the dynamically created
// HTML rendering functions.
// Make these static const so they get put in flash
static const char szHTMLRestart[]           = "GET /Restart ";
static const char szHTMLTerminate[]         = "GET /Terminate ";
static const char szHTMLReboot[]            = "GET /Reboot ";
static const char szHTMLFavicon[]           = "GET /favicon.ico ";
static const char szHTMLGetPattern[]        = "GET /GetPatn ";
static const char szHTMLGetHome1[]          = "GET / ";
static const char szHTMLGetHome2[]          = "GET /HomePage.htm ";
static const char szHTMLPostPattern1[]      = "POST / ";
static const char szHTMLPostPattern2[]      = "POST /HomePage.htm ";
static const char szHTMLGetPatternNbr[]     = "GET /PatNbr.txt ";

// here is our sample/example dynamically created HTML page
GCMD::ACTION ComposeHTMLHomePattern(CLIENTINFO * pClientInfo);
GCMD::ACTION ComposeHTMLGetPatternPage(CLIENTINFO * pClientInfo);
GCMD::ACTION ComposeHTMLPostPattern(CLIENTINFO * pClientInfo);
GCMD::ACTION ComposeHTMLGetPatternNbrText(CLIENTINFO * pClientInfo);

bool InitializeHomePage(void);

/***    void setup(void)
 *
 *    Parameters:
 *          None
 *              
 *    Return Values:
 *          None
 *
 *    Description: 
 *    
 *      Arduino Master Initialization routine
 *      
 *      
 * ------------------------------------------------------------ */
void setup(void) 
{   
    pinMode(PIN_BTN1, INPUT);

    // Must do a Serial.begin because the HTTP Server
    // has diagnostic prints in it.
    Serial.begin(9600);
    Serial.println("ShowServer v1.0");
    Serial.println("Copyright 2014, Digilent Inc.");
    Serial.println("Written by Keith Vogel");
    Serial.println();

    // add rendering functions for dynamically created web pages
    // max of 10 AddHTMLPage() allowed 

    // This is adding our sample dynamically HTML page
    // It will get invoked when http://<IP>/Sample is specified on the browser
    AddHTMLPage(szHTMLGetHome1,     ComposeHTMLHomePattern);
    AddHTMLPage(szHTMLGetHome2,     ComposeHTMLHomePattern);
    AddHTMLPage(szHTMLGetPattern,   ComposeHTMLGetPatternPage);
    AddHTMLPage(szHTMLPostPattern1, ComposeHTMLPostPattern);
    AddHTMLPage(szHTMLPostPattern2, ComposeHTMLPostPattern);
    AddHTMLPage(szHTMLGetPatternNbr,ComposeHTMLGetPatternNbrText);

    // comment this out if you do not want to support
    // restarting the network stack from a browser
    AddHTMLPage(szHTMLRestart,      ComposeHTMLRestartPage);

    // comment this out if you do not want to support
    // terminating the server from a browser
    AddHTMLPage(szHTMLTerminate,    ComposeHTMLTerminatePage);

    // comment this out if you do not want to support
    // rebooting (effectively hitting MCLR) the server from a browser
    AddHTMLPage(szHTMLReboot,       ComposeHTMLRebootPage);

    // This example supports favorite ICONs, 
    // those are those icon's next to the URL in the address line 
    // on the browser once the page is displayed.
    // To support those icons, have at the root of the SD file direcotory
    // an ICON (.ico) file with your ICON in it. The file MUST be named
    // favicon.ico. If you do not have an icon, then uncomment the following
    // line so the server will tell the browser with an HTTP file not found
    // error that we don't have a favoite ICON.
    // AddHTMLPage(szHTMLFavicon,      ComposeHTTP404Error);

    // Make reading files from the SD card the default compose function
    SetDefaultHTMLPage(ComposeHTMLSDPage);

    // Initialize the SD card
    SDSetup();

    InitializeHomePage();

    // Initialize the HTTP server
    ServerSetup();
}

/***    void loop(void) 
 *
 *    Parameters:
 *          None
 *              
 *    Return Values:
 *          None
 *
 *    Description: 
 *    
 *      Arduino Master Loop routine
 *      
 *      
 * ------------------------------------------------------------ */
void loop(void) 
{
    // process the HTTP Server
    ProcessServer();

    // see if we are to bump up the pattern
    if(digitalRead(PIN_BTN1))
    {
        if(!fDebounce)
        {
            iPattern++;
            if(iPattern > iMaxPattern)
            {
                iPattern = 1;
            }   
            fDebounce = true;
        }
        tDebounce = millis();
    }
    else if(fDebounce && millis() - tDebounce >= msDebounce)
    {
        fDebounce = false;
    }

}
