/************************************************************************/
/*                                                                      */
/*    HTMLSample.cpp                                                    */
/*                                                                      */
/*    Renders a simple dynamically generated HTML page                  */
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
/*    2/1/2013(KeithV): Created                                         */
/************************************************************************/
// Required header
#include    <HTTPServer.h>

// this is used in HTMLGetPatn.cpp
uint32_t iPattern = 1;

// constant strings
static const char szHomePageURL[]   = "GET / HTTP\r\n";
static const char szContentLength[] = "Content-Length: ";
static const char szRadioButton[]   = "Radio1=id";
static const char szChecked[]       = "checked=\"checked\" ";
static const char szValue[]         = "value=id";
static const char   szHomePage[]    = "HomePage.htm";

// values set up at setup time
static uint32_t cbFileHome = 0;
static char * rgpHTML[iMaxPattern+1];
static uint32_t rgcbHTML[iMaxPattern+1];
static char szHomePageCache[4096];

/************************************************************************/
/*    HTML Strings                                                      */
/************************************************************************/

/************************************************************************/
/*    State machine states                                              */
/************************************************************************/

/*
    There are 3 magic predefined states:

        HTTPSTART: 
            This is a required state to have.
            This is always the starting state and the first state
            in your compose function.

        HTTPDISCONNECT:
            Not a required state to have, but this state will be called whenever
            your conneciton is closed. This will always be called even if you
            return GCMD::DONE requesting the connection to be closed. This is
            nice because you can do clean up steps here, because no matter how
            the connection was closed, you get a chance to clean up.

        HTTPTIMEOUT:
            This is not a required state to have.
            This is called if the connection timed out, that is, the connection was
            made, your compose function got called but some pending data did not come in.
            For example if you returned a GCMD::GETLINE yet a line never came in.
*/

// these are local to this file, and we can duplicate the names
// in other files as the compiler will not see these in other files.
// These are your local state machine states.
typedef enum {
    CONTLEN,
    ENDHDR,
    DATA,
    PARSE,
    FORM,
    DONE
} STATE;

typedef enum {
    HMSTART,
    HMWRITE,
    HMDONE
} HMSTATE;

/***    bool InitializeHomePage(void)
 *
 *    Parameters:
 *          none
 *
 *    Return Values:
 *          True if initialized, false otherwise
 *
 *    Description:
 *
 *      Reads the HomePage from the SD card and moves it into memory
 *      then parses the place for the checked and starts with nothing
 *      checked.
 *
 * ------------------------------------------------------------ */
bool InitializeHomePage(void)
{
    File fileHome;

    // open the file
    if((fileHome = SD.open(szHomePage, FILE_READ)) && fileHome.seek(0) )
    { 
        uint32_t cbRead         = 0;
        char *  pNext           = NULL;
        int i                   = 0;

        cbFileHome     = fileHome.size();

        Serial.print(szHomePage);
        Serial.print(" is ");
        Serial.print(cbFileHome, DEC);
        Serial.println(" bytes long");

        // get its size
        if(sizeof(szHomePageCache) <= cbFileHome)
        {
            Serial.print("Home page buffer is only ");
            Serial.print(sizeof(szHomePageCache), DEC);
            Serial.println(" bytes long");
            return(false);
        }

        // read in the file into an internal buffer
        while(cbRead < cbFileHome)
        {
            cbRead += SDRead(fileHome, (uint8_t *) &szHomePageCache[cbRead], (cbFileHome - cbRead));
        }
        fileHome.close();
        szHomePageCache[cbRead] = '\0';
        
        rgpHTML[0] = szHomePageCache;
        rgpHTML[1] = strstr(szHomePageCache, szChecked);
        rgcbHTML[0] = rgpHTML[1] - rgpHTML[0];

        // skip passed the checked
        rgpHTML[1] = strstr(rgpHTML[1], szValue);

        for(i=1; i<iMaxPattern; i++)
        {
                rgpHTML[i+1]  = strstr(rgpHTML[i] + sizeof(szValue)-1, szValue);
                rgcbHTML[i] = rgpHTML[i+1] - rgpHTML[i];
        }
        rgcbHTML[i] = szHomePageCache + cbFileHome - rgpHTML[iMaxPattern];
        return(true);
    }
    else
    {
        Serial.print("Unable to open HTML page:");
        Serial.println(szHomePage);
        return(false);
    }

    return(false);
}

/***    GCMD::ACTION ComposeHTMLHomePattern(CLIENTINFO * pClientInfo)
 *
 *    Parameters:
 *          pClientInfo - the client info representing this connection and web page
 *
 *    Return Values:
 *          GCMD::ACTION    - GCMD::CONTINUE, just return with no outside action
 *                          - GCMD::READ, non-blocking read of input data into the rgbIn buffer appended to the end of rgbIn[] which has a predefined size of 256 bytes
 *                              when we return to this compose function cbRead will have the number of bytes read, and likely could be zero.
 *                          - GCMD::GETLINE, blocking read until a line of input is read or until the rgbIn buffer is full, always the line starts at the beginnig of the rgbIn
 *                              cbRead has the number of bytes read
 *                          - GCMD::WRITE, loop writing until all cbWrite bytes are written from the pbOut buffer
 *                              pbOut can point to any valid buffer that will remain unchanged until execution returns to this function. We could get a TIMOUT
 *                              if we can't write the data. cbWritten will have the number of bytes actually written. As part of each connection there is a
 *                              scratch buffer of 256 provide at rgbOut; it is optional to point pbOut to rgbOut. PbOut can point anywhere and that is what will be written
 *                              cbWrite must be set to the number of bytes to write.
 *                          - GCMD::DONE, we are done processing and the connection can be closed
 *
 *    Description:
 *
 *      This composes the sample HTML page
 *
 * ------------------------------------------------------------ */
GCMD::ACTION ComposeHTMLHomePattern(CLIENTINFO * pClientInfo)
{
    GCMD::ACTION retCMD = GCMD::WRITE;

    // a word of caution... DO NOT cast htmlState to your enum type!
    // the compiler will silently remove the HTTPSTART case as
    // that state is not part of your enum. Keep the switch on typed
    // aginst the generic uint32_t.
    switch(pClientInfo->htmlState)
    {

        // Every Compose function will start at the magic HTTPSTART state
        // we MUST support this state.
        case HTTPSTART:
            Serial.println("Home Pattern Page Detected");

            if(!(0 < iPattern && iPattern <= iMaxPattern))
            {
                cbFileHome -= sizeof(szChecked) - 1;
            }
            // here we make an HTTP header directive for an HTML (.htm) MIME type; our action for this state will be to write out the HTTP header
            pClientInfo->cbWrite = BuildHTTPOKStr(false, cbFileHome, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;
            pClientInfo->htmlState = HMSTART;
            break;

        case HMSTART:
            // we are not using the rgbOut buffer
            // so this is some state info I can use
            // per this connection
            pClientInfo->rgbOut[0] = 0;
            pClientInfo->rgbOut[1] = 0;
            pClientInfo->htmlState = HMWRITE;
            retCMD = GCMD::CONTINUE;
            break;

        case HMWRITE:
            if(pClientInfo->rgbOut[0] == iPattern && pClientInfo->rgbOut[1] == 0 && 0 < iPattern && iPattern <= iMaxPattern)
            {
                pClientInfo->rgbOut[1]  = 1;
                pClientInfo->pbOut      = (const byte *) szChecked;
                pClientInfo->cbWrite    = sizeof(szChecked) - 1;
                
            }
            else if(pClientInfo->rgbOut[0] <= iMaxPattern)
            {               
                pClientInfo->pbOut      = (byte *) rgpHTML[pClientInfo->rgbOut[0]];
                pClientInfo->cbWrite    = rgcbHTML[pClientInfo->rgbOut[0]];
                pClientInfo->rgbOut[0]++;
            }
            else
            {
                pClientInfo->htmlState = HMDONE;
                retCMD = GCMD::DONE;
            }
            break;

        case HTTPDISCONNECT:

        // the done state is were we say we are done, and that
        // the connection can be closed
        case HMDONE:
        default:

            // by returning DONE, we will close the connection
            // and be done with this page
            retCMD = GCMD::DONE;
            break;
    }

    return(retCMD);
}
/***    GCMD::ACTION ComposeHTMLSamplePage(CLIENTINFO * pClientInfo)
 *
 *    Parameters:
 *          pClientInfo - the client info representing this connection and web page
 *
 *    Return Values:
 *          GCMD::ACTION    - GCMD::CONTINUE, just return with no outside action
 *                          - GCMD::READ, non-blocking read of input data into the rgbIn buffer appended to the end of rgbIn[] which has a predefined size of 256 bytes
 *                              when we return to this compose function cbRead will have the number of bytes read, and likely could be zero.
 *                          - GCMD::GETLINE, blocking read until a line of input is read or until the rgbIn buffer is full, always the line starts at the beginnig of the rgbIn
 *                              cbRead has the number of bytes read
 *                          - GCMD::WRITE, loop writing until all cbWrite bytes are written from the pbOut buffer
 *                              pbOut can point to any valid buffer that will remain unchanged until execution returns to this function. We could get a TIMOUT
 *                              if we can't write the data. cbWritten will have the number of bytes actually written. As part of each connection there is a
 *                              scratch buffer of 256 provide at rgbOut; it is optional to point pbOut to rgbOut. PbOut can point anywhere and that is what will be written
 *                              cbWrite must be set to the number of bytes to write.
 *                          - GCMD::DONE, we are done processing and the connection can be closed
 *
 *    Description:
 *
 *      This composes the sample HTML page
 *
 * ------------------------------------------------------------ */
GCMD::ACTION ComposeHTMLPostPattern(CLIENTINFO * pClientInfo)
{
    GCMD::ACTION retCMD = GCMD::CONTINUE;
    char * pPattern = NULL;

    // a word of caution... DO NOT cast htmlState to your enum type!
    // the compiler will silently remove the HTTPSTART case as
    // that state is not part of your enum. Keep the switch on typed
    // aginst the generic uint32_t.
    switch(pClientInfo->htmlState)
    {

        // Every Compose function will start at the magic HTTPSTART state
        // we MUST support this state.
        case HTTPSTART:
            Serial.println(" Post Pattern Page Detected");
            pClientInfo->htmlState = CONTLEN;
            retCMD = GCMD::GETLINE;
            break;

        case CONTLEN:

            if(memcmp((byte *) szContentLength, pClientInfo->rgbIn, sizeof(szContentLength)-1) == 0)
            {

                *((uint32_t *) pClientInfo->rgbOut) = atoi((char *) &pClientInfo->rgbIn[sizeof(szContentLength)-1]);
                pClientInfo->htmlState = ENDHDR;
            }
            retCMD = GCMD::GETLINE;
            break;

        case ENDHDR:

            // the header is ended with a double \r\n\r\n, so I will get
            // a zero length line.
            if(strlen((char *) pClientInfo->rgbIn) == 0)
            {
                uint32_t i = 0;

                // go to beyond the \0
                for(i = 0; i < pClientInfo->cbRead && pClientInfo->rgbIn[i] == '\0'; i++);

                // move the buffer to the front
                pClientInfo->cbRead -= i;
                if(pClientInfo->cbRead > 0)
                {
                    memcpy(pClientInfo->rgbIn, &pClientInfo->rgbIn[i], pClientInfo->cbRead);
                }

                pClientInfo->htmlState = DATA;
            }
            else
            {
                retCMD = GCMD::GETLINE;
            }
            break;

        case DATA:
            if(pClientInfo->cbRead < *((uint32_t *) pClientInfo->rgbOut))
            {
                pClientInfo->rgbIn[*((uint32_t *) pClientInfo->rgbOut)] = '\0';
                retCMD = GCMD::READ;
            }
            else
            {
                pClientInfo->htmlState = PARSE;
            }
            break;

        case PARSE:

            pPattern = strstr((char *) pClientInfo->rgbIn, szRadioButton);

            if(pPattern != NULL)
            {
                int i = 0;
                pPattern += sizeof(szRadioButton) - 1;
                pPattern[1] = '\0';
                i = atoi(pPattern);
                if(i > 0 && i <= iMaxPattern)
                {
                    iPattern = i;
                }
            }
            pClientInfo->htmlState = FORM;
            break;


        case FORM:
            pClientInfo->cbRead = sizeof(szHomePageURL)-1;
            memcpy(pClientInfo->rgbIn, szHomePageURL, pClientInfo->cbRead);
            pClientInfo->ComposeHTMLPage = ComposeHTMLHomePattern;
            pClientInfo->htmlState = HTTPSTART;
            Serial.println("Jumping to Home page");
            break;

        case HTTPDISCONNECT:

        // the done state is were we say we are done, and that
        // the connection can be closed
        case DONE:
        default:

            // by returning DONE, we will close the connection
            // and be done with this page
            retCMD = GCMD::DONE;
            break;
    }

    // Return the command we want to do
    // like WRITE, or DONE
    return(retCMD);
}
