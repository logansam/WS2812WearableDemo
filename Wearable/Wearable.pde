/************************************************************************/
/*                                                                      */
/*    Wearable                                                          */
/*                                                                      */
/*    A Example of a wearable client talking to a                       */
/*    HTTP Server                                                       */
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
/*    4/29/2014(KeithV): Created                                        */
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
/*  WF32                                                                */
/*  WiFire                                                               */
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
/*    You MUST put NetworkConnect.h in your sketch director             */
/*    And you MUST configure it with your network parameters            */
/*                                                                      */
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
//#include    <SD.h>

// and this is the HTTPServer library code.
// this is a required library
#include    <NetworkConfig.h>

void lblingSetup(void);
void lblingUpdate(char chPattern);
void lblingPeriodicTask(void);
void lblingBump(void);

//static const uint8_t serverDomanName[] = "192.168.50.236";
//static const uint8_t serverDomanName[] = "192.168.1.205";
static IPv4 serverDomanName = ((const IPv4) {192,168,1,205});

//#define IPv4NONE ((const IPv4) {0x00, 0x00, 0x00, 0x00}) 

//static const uint16_t serverPort  = 44236;
static const uint16_t serverPort = 80;
static const uint8_t  getRequest[] = "GET /PatNbr.txt HTTP/1.1\r\nHost:ShowServer\r\nConnection: close\r\n\r\n";

static const uint8_t szConLen[] = "Content-Length: ";

typedef enum
{
    INITIALIZE,
    SENDGET,
    READHEADER,
    READDATA,
    CLOSE,
    WAIT,
    EXIT,
    SPIN
} LKS;

static LKS linkState = INITIALIZE;
static uint32_t networkState = 0;
static uint8_t rgbInput[1024];

static bool fDebounce = false;
static uint32_t tDebounce = 0;
#define msDebounce 500

static TCPSocket    tcpHost;
static IPSTATUS     status = ipsSuccess;
static int32_t      cbContent = 0;
static int32_t      cbReadIn = 0;

#define cMsWait     1500
static uint32_t     tWait = 0;

//#define cMsTimeout  5000
#define cMsTimeout 500
static uint32_t     tTimeout = 0;

//#define cPostUpdate 10000
#define cPostUpdate 500
uint32_t cPost = 0;

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
    pinMode(PIN_BTN2, INPUT);

    tWait = millis();

    // Must do a Serial.begin because the HTTP Server
    // has diagnostic prints in it.
    Serial.begin(9600);
    Serial.println("Wareable v1.0");
    Serial.println("Copyright 2014, Digilent Inc.");
    Serial.println("Written by Keith Vogel");
    Serial.println();

    lblingSetup();

    // Initialize the HTTP server
    NetworkSetup(&Serial);
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
	lblingPeriodicTask();
	
    // see if we are to bump up the pattern
    if(digitalRead(PIN_BTN2))
    {
        if(!fDebounce)
        {
            lblingBump();
            fDebounce = true;
        }
        tDebounce = millis();
    }
    else if(fDebounce && millis() - tDebounce >= msDebounce)
    {
        fDebounce = false;
    }
	
    if(networkState == NWLinked)
    {   
        if( tcpHost.isEstablished() && millis() - tTimeout >= cMsTimeout)
        {
            linkState = CLOSE;
            Serial.println("Connection open too long without activity, closing");
        }
    }



    // process the HTTP Server
    switch((networkState = ProcessNetwork(&Serial)))
    {
                 
        case NWInitializing: 
            linkState = INITIALIZE;
            break;

        case NWLinked: 
            switch(linkState)
            {
                case INITIALIZE:

                    tTimeout = millis();

					Serial.println("I'm hanging forever");
					
                    //if(deIPcK.tcpConnect((char *) serverDomanName, serverPort, tcpHost, &status))
                    if(deIPcK.tcpConnect(serverDomanName, serverPort, tcpHost, &status)){
						Serial.println("done hangin");
                        linkState = SENDGET;
//                        Serial.println("Sending:");
//                        Serial.print((char *) getRequest);
                    }
                    else if(IsIPStatusAnError(status)){
                        Serial.print("Failed to connect to ");
                        Serial.print("I fix SHIT!");
                        //Serial.print((char *) serverDomanName);
                        Serial.print(" with error: 0x");
                        Serial.println(status, HEX);
                        linkState = SPIN;
                    }
					else{
					Serial.println("bullshit use of else if");
					Serial.print("status: 0x");
					Serial.println(status, HEX);
					
					//attempt to give the constant barrage of get requests to the server
					//linkState = CLOSE;
					}
                    break;

                case SENDGET:

                    if(tcpHost.isEstablished(&status)){
                        tcpHost.writeStream(getRequest, sizeof(getRequest) - 1, &status);
                        tcpHost.flush();
                        linkState = READHEADER;
                        tTimeout = millis();
                        Serial.println("Sent GET request");
                    }
                
					else if(IsIPStatusAnError(status)){
                        Serial.print("Failed to establish the TCP connection, error: 0x");
                        Serial.println(status, HEX);
                        linkState = CLOSE;
                    }
                    break;

                case READHEADER:

                    // in the HTTP header there is a length, it will be one of the first things
                    // in there and will come as part of the first data packets
                    // but we will wait until we recieve the full HTTP header

                    // the header is somewhere around 350 bytes long, we need to provide a buffer
                    // that will hold the whole header in one shot, so use a buffer of 1K

/*  The header will look something like this...
                    HTTP/1.1 200 OK
                    Content-Length: 733384
                    Content-Type: text/plain
                    Content-Location: http://www.microchip.com/mrfupdates/a2patch_3107_1029.txt
                    Last-Modified: Fri, 04 Apr 2014 18:57:10 GMT
                    Accept-Ranges: bytes
                    ETag: "d3bfceae3750cf1:13ecd"
                    Server: Microsoft-IIS/6.0
                    X-Powered-By: ASP.NET
                    Cache-Control: max-age=71458
                    Date: Tue, 22 Apr 2014 03:18:40 GMT
*/

                    // what we are doing here is waiting until the whole header is read in
                    // then we can parse out the lenght and move to the first byte of data

                    if(tcpHost.isConnected(&status)  && tcpHost.available() > 0)
                    {
                        uint32_t cbPeek = 0;

                        // just peek the data, don't read it out of the stream, because we are waiting for the whole buffer to come in
                        if((cbPeek = tcpHost.peekStream(rgbInput, sizeof(rgbInput)-1)) > 0)
                        {
                            uint8_t * pEndOfHeader = NULL;

                            // see if we got the whole header in. The header is terminated by a double \n\r
                            rgbInput[cbPeek] = '\0';

                            // if we found the header, then we want to read out the header
                            if((pEndOfHeader = (uint8_t *) strstr((char *) rgbInput, "\r\n\r\n")) != NULL)
                            {
                                uint32_t cbHeader = pEndOfHeader - rgbInput + 4;

                                //now read out of the input stream the header
                                if(tcpHost.readStream(rgbInput, cbHeader) == cbHeader)
                                {
                                    uint8_t * pConLen = 0;

                                    rgbInput[cbHeader] = '\0';

                                    // find the content length
                                    if((pConLen = (uint8_t *) strstr((const char *) rgbInput, (const char *) szConLen)) != NULL)
                                    {
                                        uint8_t * pEndConLen = NULL;

                                        // move forward to the value
                                        pConLen += (sizeof(szConLen) - 1);

                                        // find the end of the value
                                        if((pEndConLen = (uint8_t *) strstr((const char *) pConLen, "\r\n")) != NULL)
                                        {
                                            *pEndConLen = '\0';
                                            cbContent = atoi((const char *) pConLen);

                                            Serial.print("Content Length is: ");
                                            Serial.println(cbContent, DEC);

                                            cbReadIn = 0;
                                            linkState = READDATA;
                                            tTimeout = millis();
                                        }
                                        else
                                        {
                                            Serial.println("Error finding the end of Content Length");
                                            linkState = CLOSE;
                                        }
                                    }
                                    else
                                    {
                                        Serial.println("Error finding the Content Length");
                                        linkState = CLOSE;
                                    }
                                }
                                else
                                {
                                    Serial.println("Error reading header");
                                    linkState = CLOSE;
                                }
                            }
                        }
                    }
                    else if(IsIPStatusAnError(status))
                    {
                        Serial.print("Lost connection to ");
                        Serial.print("I fix Shit");
                        //Serial.print((char *) serverDomanName);
                        Serial.print(" with error: 0x");
                        Serial.println(status, HEX);
                        linkState = CLOSE;
                    }
                    break;

                case READDATA:

                    if(tcpHost.isConnected(&status))
                    {
                        int32_t cbReadThisPass = 0;

                        if(tcpHost.available() > 0)
                        {
                            uint32_t i = 0;
                            uint32_t cbReadThisPass = tcpHost.readStream(rgbInput, sizeof(rgbInput));

                            cbReadIn += cbReadThisPass;

                            lblingUpdate(rgbInput[0]);
                        }

                        // are we done reading stuff in
                        if(cbReadIn >= cbContent)
                        {
                            cbReadIn += tcpHost.available();

                            if(cbReadIn > cbContent)
                            {
                                Serial.print("Too many bytes in file: error: ");
                                Serial.println(cbReadIn, DEC);
                                linkState = CLOSE;
                            }
                            else
                            {
                                Serial.print("Successfully read: ");
                                Serial.print(cbContent, DEC);
                                Serial.println(" bytes");
                                Serial.print("Value ");
                                Serial.print((char) rgbInput[0]);
                                Serial.println(" recieved");
                                tTimeout = millis();
                                linkState = CLOSE;
                            }
                        }
                    }
                    else if(IsIPStatusAnError(status))
                    {
                        Serial.print("Lost connection to ");
						Serial.print("I fix Shit!");
                        //Serial.print((char *) serverDomanName);
                        Serial.print(" with error: 0x");
                        Serial.println(status, HEX);

                        Serial.print("Bytes read: ");
                        Serial.println(cbReadIn, DEC);

                        linkState = CLOSE;
                    }
                    break;

                case CLOSE:
                    tcpHost.close();
                    Serial.println("Closing connection\n\n");
//                    Serial.println();
//                    Serial.println();
                    tWait = millis();
                    linkState = WAIT;
                    break;

                case WAIT:
				
				Serial.print("in wait  ");
				Serial.print(tWait);
				Serial.print("  ");
				Serial.println(millis());
				
				if((millis() - tWait) > cMsWait)
                    {
                        linkState = INITIALIZE;
                    }
                    break;

                case EXIT:
                    tcpHost.close();
                    Serial.println("Exiting!");
                    linkState = SPIN;
                    break;

                default:
                case SPIN:
                    return;
                    break;
            }
            break;

        case NWHardError:
        case NWRestarting:
        default: 
            return;
            break;
    }
/*
    if(networkState == NWLinked)
    {   
        lblingPeriodicTask();

        if( tcpHost.isEstablished() && millis() - tTimeout >= cMsTimeout)
        {
            linkState = CLOSE;
            Serial.println("Connection open too long without activity, closing");
        }
    }

    // see if we are to bump up the pattern
    if(digitalRead(PIN_BTN2))
    {
        if(!fDebounce)
        {
            lblingBump();
            fDebounce = true;
        }
        tDebounce = millis();
    }
    else if(fDebounce && millis() - tDebounce >= msDebounce)
    {
        fDebounce = false;
    }
	*/
}
 
