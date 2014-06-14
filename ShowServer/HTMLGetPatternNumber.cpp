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
    WRITECONTENT,
    DONE
} STATE;

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
GCMD::ACTION ComposeHTMLGetPatternNbrText(CLIENTINFO * pClientInfo)
{
    GCMD::ACTION retCMD = GCMD::DONE;

    // a word of caution... DO NOT cast htmlState to your enum type!
    // the compiler will silently remove the HTTPSTART case as 
    // that state is not part of your enum. Keep the switch on typed
    // aginst the generic uint32_t.
    switch(pClientInfo->htmlState)
    {

        // Every Compose function will start at the magic HTTPSTART state
        // we MUST support this state.
        case HTTPSTART:
            Serial.println("Sample Page Detected");

            // here we make an HTTP header directive for an HTML (.htm) MIME type; our action for this state will be to write out the HTTP header
            pClientInfo->cbWrite = BuildHTTPOKStr(false, 1, ".txt", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;

            // say we want to write cbWrite bytes from pbOut
            retCMD = GCMD::WRITE;

            // after writing our HTTP directive, return to this compose functions
            // and execute the WRITECONTENT state
            pClientInfo->htmlState = WRITECONTENT;

            break;

        // Now we write our simple HTML page out
        // do this by setting pbOut to the string or bytes we
        // want to write out on the connection, and set the size in cbWrite
         case WRITECONTENT:

            // Put the 1 char of text in
            if(itoa(iPattern, (char *) pClientInfo->rgbOut, 10) == NULL)
            {
                pClientInfo->rgbOut[0] = '\0';
            }

            // do not want to include the null terminator in the size
            // we could use strlen, but here I can have the compiler calculate 
            // the size as a constant; less the null terminator
            pClientInfo->pbOut = pClientInfo->rgbOut;
            pClientInfo->cbWrite = 1;

            // say we want to write cbWrite bytes from pbOut
            retCMD = GCMD::WRITE;

            // after writing the body of our HTML page, return to this compose functions
            // and execute the DONE state
            pClientInfo->htmlState = DONE;             
            break;
    
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