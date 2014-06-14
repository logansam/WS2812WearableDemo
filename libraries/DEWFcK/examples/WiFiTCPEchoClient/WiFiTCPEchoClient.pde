/************************************************************************/
/*                                                                      */
/*        TCPEchoClient                                                 */
/*                                                                      */
/*        A chipKIT DEIPcK TCP Client application to                    */
/*        demonstrate how to use the TcpClient Class.                   */
/*        This can be used in conjuction  with TCPEchoServer            */                
/*                                                                      */
/************************************************************************/
/*        Author:         Keith Vogel                                   */
/*        Copyright 2011, Digilent Inc.                                 */
/************************************************************************/
/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/************************************************************************/
/*                                                                      */
/*                                                                      */
/************************************************************************/
/*  Revision History:                                                   */
/*                                                                      */
/*    12/19/2011(KeithV): Created                                       */
/*    2/7/2012(KeithV): Updated for WiFi                                */
/*    11/13/2012(KeithV): Modified to be generic for all HW libraries   */
/*                                                                      */
/************************************************************************/

//******************************************************************************************
//******************************************************************************************
//***************************** SET YOUR CONFIGURATION *************************************
//******************************************************************************************
//******************************************************************************************

/************************************************************************/
/*                                                                      */
/*              Include ONLY 1 hardware library that matches            */
/*              the network hardware you are using                      */
/*                                                                      */
/*              Refer to the hardware library header file               */
/*              for supported boards and hardware configurations        */
/*                                                                      */
/************************************************************************/
#include <DEWFMRF24G.h>                     // This is for the MRF24WGxx on a pmodWiFi or WiFiShield

/************************************************************************/
/*                    Required libraries, Do NOT comment out            */
/************************************************************************/
#include <DEIPcK.h>
#include <DEWFcK.h>

/************************************************************************/
/*                                                                      */
/*              SET THESE VALUES FOR YOUR NETWORK                       */
/*                                                                      */
/************************************************************************/

const char * szIPServer = "10.22.2.10";    // server to connect to
uint16_t portServer = DEIPcK::iPersonalPorts44 + 300;     // port 44300

// Specify the SSID
const char * szSsid = "Internet";

// select 1 for the security you want, or none for no security
#define USE_WPA2_PASSPHRASE
//#define USE_WPA2_KEY
//#define USE_WEP40
//#define USE_WEP104
//#define USE_WF_CONFIG_H

// modify the security key to what you have.
#if defined(USE_WPA2_PASSPHRASE)

    const char * szPassPhrase = "National!nstrument5";
    #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, szPassPhrase, &status)

#elif defined(USE_WPA2_KEY)

    DEWFcK::WPA2KEY key = { 0x27, 0x2C, 0x89, 0xCC, 0xE9, 0x56, 0x31, 0x1E, 
                            0x3B, 0xAD, 0x79, 0xF7, 0x1D, 0xC4, 0xB9, 0x05, 
                            0x7A, 0x34, 0x4C, 0x3E, 0xB5, 0xFA, 0x38, 0xC2, 
                            0x0F, 0x0A, 0xB0, 0x90, 0xDC, 0x62, 0xAD, 0x58 };
    #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, key, &status)

#elif defined(USE_WEP40)

    const int iWEPKey = 0;
    DEWFcK::WEP40KEY keySet = {    0xBE, 0xC9, 0x58, 0x06, 0x97,     // Key 0
                                    0x00, 0x00, 0x00, 0x00, 0x00,     // Key 1
                                    0x00, 0x00, 0x00, 0x00, 0x00,     // Key 2
                                    0x00, 0x00, 0x00, 0x00, 0x00 };   // Key 3
    #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, keySet, iWEPKey, &status)

#elif defined(USE_WEP104)

    const int iWEPKey = 0;
    DEWFcK::WEP104KEY keySet = {   0x3E, 0xCD, 0x30, 0xB2, 0x55, 0x2D, 0x3C, 0x50, 0x52, 0x71, 0xE8, 0x83, 0x91,   // Key 0
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Key 1
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Key 2
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Key 3
    #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, keySet, iWEPKey, &status)

#elif defined(USE_WF_CONFIG_H)

    #define WiFiConnectMacro() deIPcK.wfConnect(0, &status)

#else   // no security - OPEN

    #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, &status)

#endif
   
//******************************************************************************************
//******************************************************************************************
//***************************** END OF CONFIGURATION ***************************************
//******************************************************************************************
//******************************************************************************************

typedef enum
{
    NONE = 0,
    CONNECT,
    WRITE,
    READ,
    CLOSE,
    DONE,
} STATE;

STATE state = CONNECT;

unsigned tStart = 0;
unsigned tWait = 5000;

TCPSocket tcpSocket;
byte rgbRead[1024];
int cbRead = 0;

// this is for Print.write to print
byte rgbWrite[] = {'*','W','r','o','t','e',' ','f','r','o','m',' ','p','r','i','n','t','.','w','r','i','t','e','*','\n'};
int cbWrite = sizeof(rgbWrite);

// this is for tcpSocket.writeStream to print
byte rgbWriteStream[] = {'*','W','r','o','t','e',' ','f','r','o','m',' ','t','c','p','C','l','i','e','n','t','.','w','r','i','t','e','S','t','r','e','a','m','*','\n'};
int cbWriteStream = sizeof(rgbWriteStream);

/***        void setup()
 *
 *        Parameters:
 *          None
 *              
 *        Return Values:
 *          None
 *
 *        Description: 
 *        
 *      Arduino setup function.
 *      
 *      Initialize the Serial Monitor, and initializes the
 *      connection to the TCPEchoServer
 *      Use DHCP to get the IP, mask, and gateway
 *      by default we connect to port 44300
 *      
 * ------------------------------------------------------------ */
void setup()
{
    Serial.begin(9600);
    Serial.println("WiFiTCPEchoClient 1.0");
    Serial.println("Digilent, Copyright 2012");
    Serial.println("");
}

/***        void loop()
 *
 *        Parameters:
 *          None
 *              
 *        Return Values:
 *          None
 *
 *        Description: 
 *        
 *      Arduino loop function.
 *      
 *      We are using the default timeout values for the DEIPcK and TcpClient class
 *      which usually is enough time for the Tcp functions to complete on their first call.
 *
 *      This code will write  some stings to the server and have the server echo it back
 *      
 * ------------------------------------------------------------ */
void loop() {
    IPSTATUS status;
    int cbRead = 0;

    switch(state)
    {

        case CONNECT:
            if(WiFiConnectMacro())
            {
                Serial.print("Connection Created");
                deIPcK.begin();
                deIPcK.tcpConnect(szIPServer, portServer, tcpSocket);
                state = WRITE;
            }
            else if(IsIPStatusAnError(status))
            {
                Serial.print("Unable to connection, status: ");
                Serial.println(status, DEC);
                state = CLOSE;
            }
            break;

        // write out the strings
        case WRITE:
            if(tcpSocket.isConnected())
                {     
                Serial.println("Got Connection");
  
                tcpSocket.writeStream(rgbWriteStream, cbWriteStream);
 
                // write() should all fail to compile
                // while write is inherited from Print, we Hide this in the TcpClient class
                // as writeStream should be used; and we don't want confusing and competing calls
                // we just want to inherit the print() and println() methods from Print in TcpClient
                //tcpSocket.write("Printed from tcpSocket.write");
                //tcpSocket.write(rgbWrite, cbWrite);
                //tcpSocket.write((uint8_t) 'b');

                // check that print() and println() work
                tcpSocket.print("*Printed from tcpSocket.print*\n");
                tcpSocket.println("*Printed from tcpSocket.println*");

                // however, tcpSocket "is-a" Print, so if I pass it as a Print
                // interally it should work as-a Print.
                printWrite(tcpSocket);

                Serial.println("Bytes Read Back:");
                state = READ;
                tStart = (unsigned) millis();
                }
            break;

            // look for the echo back
            case READ:

                // see if we got anything to read
                if((cbRead = tcpSocket.available()) > 0)
                {
                    cbRead = cbRead < sizeof(rgbRead) ? cbRead : sizeof(rgbRead);
                    cbRead = tcpSocket.readStream(rgbRead, cbRead);

                    for(int i=0; i < cbRead; i++)
                    {
                        Serial.print(rgbRead[i], BYTE);
                    }
                }

                // give us some time to get everything echo'ed back
                else if( (((unsigned) millis()) - tStart) > tWait )
                {
                    Serial.println("");
                    state = CLOSE;
                }
                break;

        // done, so close up the tcpSocket
        case CLOSE:
            tcpSocket.close();
            Serial.println("Closing TcpClient, Done with sketch.");
            state = DONE;
            break;

        case DONE:
        default:
            break;
    }

    // keep the stack alive each pass through the loop()
    DEIPcK::periodicTasks();
}

/***        void printWrite(Print& print)
 *
 *        Parameters:
 *          print - This is a Print object to check to see if it works
 *              
 *        Return Values:
 *          None
 *
 *        Description: 
 *        
 *      
 *      If we pass a TcpClient into a fuction taking Print, all Print methods should
 *      work as TcpClient "is-a" Print.
 *      
 * ------------------------------------------------------------ */
void printWrite(Print& print)
{

    // check the print() and println() methods
    tcpSocket.print("*Printed from print.print*\n");
    tcpSocket.println("*Printed from print.println*");

    // While these are hidden from TcpClient
    // they should not be hidden from Print
    // these should all work.
    print.write((uint8_t) 'b');
    print.write("\n*Wrote from print.write*\n");
    print.write(rgbWrite, cbWrite);
}
