/************************************************************************/
/*                                                                      */
/*    WebServer.cpp                                                     */
/*                                                                      */
/*    A chipKIT WiFi HTTP Web Server implementation                     */
/*    This sketch is designed to work with web browsers                 */
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
#define DEFINEIPVARHERE
#include    "NetworkConfig.h"

#define CBDATETIME          32
#define TooMuchTime()       (millis() > cMSecEnd)
#define RestartTimer()      cMSecEnd = millis() + cSecTimeout * 1000
#define SetTimeout(cSec)    {cSecTimeout = cSec;}

#define cSecFastRest        10
#define cSecDefault         30
#define cSecInitRest        30
#define cSecIncRest         30
#define cSecMaxRest         600

static int cSecTimeout = cSecDefault;
static int cMSecEnd = 0;
static int cSecRest = cSecInitRest;

static IPv4         ipMy;
static IPEndPoint   ipEP;
static IPv4         ipRemote;
static char *       szRemoteURL;

typedef enum 
{
    // enumerate these number to make sure we match the host
    // sending these commands
    NONE = 0,
    STARTSTATE,
    WIFISCAN,
    PRINTAPINFO,
    WIFICHECKKEY,
    WIFICALPSK,
    WIFICONNECTWITHKEY,
    WIFICONNECT,
    LINKED,
    DYNAMICIPBEGIN,
    STATICIPBEGIN,
    ENDPASS1,
    INITIALIZE,
    WAITFORTIME,
    GETNETADDRESSES,
    PRINTADDRESSES,
    MAKESTATICIP,
    PRINTWIFICONFIG,
    ISIPREADY,
    RESTARTNOW,
    RESTARTREST,
    TERMINATE,
    SHUTDOWN,
    RESTFORAWHILE,
    TRYAGAIN,
    DONOTHING,
    REBOOT,
    SOFTMCLR,
    MCLRWAIT,
    WAITFORTIMEOUT,
    ERROR    
} STATECMD;

#if defined(USING_WIFI)
WPA2KEY wpa2Key;
STATECMD state = WIFISCAN;              // Scan WiFi First
// STATECMD state = WIFICONNECT;        // No WiFi Scan
#else
STATECMD state = DYNAMICIPBEGIN;
#endif

// Start with DHCP to get our addresses
// then restart with a static IP
// this is the initial state machine starting point.
STATECMD stateNext = RESTARTREST;
STATECMD stateTimeOut = RESTARTREST;

IPSTATUS status = ipsSuccess;
unsigned int epochTime = 0;

// scan variables
int  cNetworks = 0;
int iNetwork = 0;

//******************************************************************************************
//******************************************************************************************
//*****************************  Supported TcpClients  *************************************
//******************************************************************************************
//******************************************************************************************

static char szTemp[256];            // needs to be long enough to take a WiFi Security key printed out.

/***    void ServerSetup()
 *
 *    Parameters:
 *          None
 *              
 *    Return Values:
 *          None
 *
 *    Description: 
 *    
 *      Initialized the Web Server Network parameters
 *      
 *      
 * ------------------------------------------------------------ */
void NetworkSetup(Print * pPrint) 
{
    int i = 0;

    // Set the LED off, for not initialized
    pinMode(PIN_LED2, OUTPUT);
    pinMode(PIN_LED3, OUTPUT);
    pinMode(PIN_LED4, OUTPUT);
    pinMode(PWR_MON_PIN, INPUT);
    SetLED(SLED::NOTREADY);     

#if defined(USING_WIFI)
    if(pPrint != NULL) 
    {
        state = WIFISCAN;          // Scan WiFi First, verify the WiFi connection
        pPrint->println("Start WiFi Scan");
    }
    else
    {
        state = WIFICHECKKEY;
    }
    RestartTimer();
    cNetworks = 0;
#else
    state = DYNAMICIPBEGIN;         // just start with the wired network
#endif

    SetTimeout(cSecDefault);
    stateNext = RESTARTREST;
    stateTimeOut = RESTARTREST;
    RestartTimer();

    cSecRest = cSecInitRest;
}

/***    void ProcessNetwork()
 *
 *    Parameters:
 *          None
 *              
 *    Return Values:
 *          None
 *
 *    Description: 
 *    
 *      This is the main server loop. It:
 *          1. Scans for WiFi connections
 *          2. Connects to a WiFi by SSID
 *          3. Optionally creates a server IP on the detected subnet and dynamically assigns DNS and subnets.
 *          4. Or uses the static IP you assign; then you must supply DNS and subnet
 *          5. Starts listening on the supplied server port.
 *          6. Accepts client connections
 *          7. Schedules the processing on client connections in a round robin yet fashion (cooperative execution).
 *          8. Automatically restart if the network goes down
 *      
 *      This illistrates how to write a state machine like loop
 *      so that the PeriodicTask is called everytime through the loop
 *      so the stack stay alive and responsive.
 *
 *      In the loop we listen for a request, verify it to a limited degree
 *      and then broadcast the Magic Packet to wake the request machine.
 *      
 * ------------------------------------------------------------ */
uint32_t ProcessNetwork(Print * pPrint)
{   
    static uint32_t fNWState    = NWInitializing;     
    int             i           = 0;

  // see if we exceeded our timeout value.
  // then just be done and close the socket 
  // by default, a closed client is never connected
  // so it is safe to call isConnected() even if it is closed 
  if(stateTimeOut != NONE && TooMuchTime())
  {
    if(pPrint != NULL) pPrint->println("Timeout occured");
    state = stateTimeOut;
    stateTimeOut = NONE;
    stateNext = RESTARTREST;
  }

  switch(state)
  {    

#if defined(USING_WIFI)

        case WIFISCAN:
            if(deIPcK.wfScan(&cNetworks, &status))
            {
                pPrint->print("Scan Done, ");
                pPrint->print(cNetworks, DEC);
                pPrint->println(" Networks Found");
                state = PRINTAPINFO;
                RestartTimer();
                iNetwork = 0;
            }
            else if(IsIPStatusAnError(status))
            {
                pPrint->println("Scan Failed");
                pPrint->println("");
                state = WIFICONNECT;
                RestartTimer();
            }
            break;

        case PRINTAPINFO:

            if(iNetwork < cNetworks)
            {
                SCANINFO scanInfo;
                int j = 0;
                {
                    t_deviceInfo dvInfo;
                    WF_DeviceInfoGet(&dvInfo);

                    pPrint->println("Device Info");
                    pPrint->print("DeviceType: 0x");
                    pPrint->print((int) dvInfo.deviceType, HEX);
                    pPrint->print(" Rom Version: 0x");
                    pPrint->print((int) dvInfo.romVersion, HEX);
                    pPrint->print(" Patch Version: 0x");
                    pPrint->println((int) dvInfo.patchVersion, HEX);
                }

                if(deIPcK.getScanInfo(iNetwork, &scanInfo))
                {
                    pPrint->println("");
                    pPrint->print("Scan info for index: ");
                    pPrint->println(iNetwork, DEC);
                 
                    pPrint->print("SSID: ");

                    for(j=0; j<scanInfo.ssidLen; j++)
                    {
                        pPrint->print(scanInfo.ssid[j]);
                    }

                    pPrint->println();

                    pPrint->print("BSSID / MAC: ");

                    for(j=0; j<sizeof(scanInfo.bssid); j++)
                    {
                        if(scanInfo.bssid[j] < 16 && pPrint != NULL)
                        {
                            pPrint->print(0, HEX);
                        }
                        pPrint->print(scanInfo.bssid[j], HEX);
                    }
                    pPrint->println("");

                    pPrint->print("Channel: ");
                    pPrint->println(scanInfo.channel, DEC);

                    pPrint->print("Signal Strength: ");
                    pPrint->println(scanInfo.rssi, DEC);

                    if(scanInfo.bssType == DEWF_INFRASTRUCTURE)
                    {
                        pPrint->println("Infrastructure Network");
                    }
                    else if(scanInfo.bssType == DEWF_ADHOC)
                    {
                        pPrint->println("AdHoc Network");
                    }
                    else
                    {
                        pPrint->println("Unknown Network Type");
                    }

                    pPrint->print("Beacon Period: ");
                    pPrint->println(scanInfo.beaconPeriod, DEC);

                    pPrint->print("dtimPeriod: ");
                    pPrint->println(scanInfo.dtimPeriod, DEC);

                    pPrint->print("atimWindow: ");
                    pPrint->println(scanInfo.atimWindow, DEC);

                    pPrint->println("Secuity info: WPA2  WPA  Preamble  Privacy  Reserved  Reserved  Reserved  IE");
                      pPrint->print("                ");
                      pPrint->print((scanInfo.apConfig & 0b10000000) >> 7, DEC);
                                       pPrint->print("    ");
                                       pPrint->print((scanInfo.apConfig & 0b01000000) >> 6, DEC);
                                            pPrint->print("       ");
                                            pPrint->print((scanInfo.apConfig & 0b00100000) >> 5, DEC);
                                                    pPrint->print("        ");
                                                    pPrint->print((scanInfo.apConfig & 0b00010000) >> 4, DEC);
                                                             pPrint->print("         ");
                                                             pPrint->print((scanInfo.apConfig & 0b00001000) >> 3, DEC);
                                                                       pPrint->print("         ");
                                                                        pPrint->print((scanInfo.apConfig & 0b00000100) >> 2, DEC);
                                                                                 pPrint->print("         ");
                                                                                 pPrint->print((scanInfo.apConfig & 0b00000010) >> 1, DEC);
                                                                                           pPrint->print("      ");
                                                                                           pPrint->println((scanInfo.apConfig & 0b00000001), DEC);

                    pPrint->print("Count of support bit rates: ");
                    pPrint->println(scanInfo.cBasicRates, DEC);    

                    for( j= 0; j< scanInfo.cBasicRates; j++)
                    {
                        uint32_t rate = (scanInfo.basicRateSet[j] & 0x7F) * 500;
                        pPrint->print("\tSupported Rate: ");
                        pPrint->print(rate, DEC);
                        pPrint->println(" kbps");
                    }
                }
                else
                {
                    pPrint->print("Unable to get scan info for iNetwork: ");
                    pPrint->println(iNetwork, DEC);
                }

                iNetwork++;
            }
            else
            {
                
                pPrint->println("");
                state = WIFICHECKKEY;
            }
            RestartTimer();
            break;

        case WIFICHECKKEY:
// not all WiFi adpators support calculating the PSK key for you
// however, if it does then we can caculate it, print it out and then
// then we know what it is and can connect with a key.
#if defined(USE_WPA2_PASSPHRASE) && defined(WPACALPSK)
                state = WIFICALPSK;
                if(pPrint != NULL) pPrint->println("Calculating PSK");
#else
                state = WIFICONNECT;
                pPrint->println("Connecting to WiFi Network");
                pPrint->println("Wait up to 40 seconds to establish the connection");
#endif
           break;

#if defined(USE_WPA2_PASSPHRASE) && defined(WPACALPSK)
    case WIFICALPSK:
        {
            if(deIPcK.wpaCalPSK(szSsid, szPassPhrase, wpa2Key))
            {
                if(pPrint != NULL) pPrint->print("PSK value: ");
                GetNumb(wpa2Key.rgbKey, sizeof(wpa2Key), ':', szTemp);
                if(pPrint != NULL) pPrint->println(szTemp);
                state = WIFICONNECTWITHKEY;

                RestartTimer();
            }
        }
        break;
#endif

        case WIFICONNECTWITHKEY:

            if(deIPcK.wfConnect(szSsid, wpa2Key, &status))
            {
                if(pPrint != NULL) pPrint->println("WiFi Connection Established");
                state = DYNAMICIPBEGIN;
                RestartTimer();
            }
            else if(IsIPStatusAnError(status))
            {
                if(pPrint != NULL) pPrint->println("Unable to get a WiFi connection with key");
                state = ERROR;
                RestartTimer();
            }
            break;

        case WIFICONNECT:
     
            if(WiFiConnectMacro())
            {
                if(pPrint != NULL) pPrint->print("WiFi Connection Created");
                state = DYNAMICIPBEGIN;
                RestartTimer();
            }
            else if(IsIPStatusAnError(status))
            {
                if(pPrint != NULL) pPrint->println("Unable to get a WiFi connection");
                state = ERROR;
                RestartTimer();
            }
            break;

#endif // USING_WIFI

    case DYNAMICIPBEGIN:

        // if I don't have a static IP, then 
        // dynamically connect and calcuate our IP.
        if(ipMyStatic.u32 == 0)
        {
            if(pPrint != NULL) pPrint->println("Dynamic begin");

            // ultimately I want to to have a static IP address 
            // but first I want to get my network addresses from DHCP
            // so to start, lets use DHCP
            deIPcK.begin();
            state = LINKED;
        }

        // otherwise go directly to the static begin
        else
        {
            state = STATICIPBEGIN;
        }

        RestartTimer();
        break;

    case STATICIPBEGIN: 
        
        if(pPrint != NULL) pPrint->println("Static begin");

        // start again with static IP addresses
        deIPcK.begin(ipMyStatic, ipGateway, subnetMask);

        // these are either the DNS servers that were statically initalized
        // or the ones return by DHCP

        // if we did not call DHCP then cInitDNS maybe < cDNSServers
        // if rgIpDNS[] was not fully populated, if this is the case
        // lets just put our gateway as the first DNS server because there
        // is a good chance our gateway is a DNS server
        {
            int iOffset = 0;

            if(cInitDNS < cDNSMax)
            {
                deIPcK.setDNS(0, ipGateway);
                iOffset = 1;
            }

            // put in the preintialized DNS servers
            for(i=0; i < cInitDNS && i < cDNSMax; i++)
            {
                deIPcK.setDNS((i+iOffset), rgIpDNS[i]);
            }
        }

        state = LINKED;
        RestartTimer();
        break;

        case LINKED:
            if(deIPcK.isLinked(&status))
            {
                 if(pPrint != NULL)
                {
                    pPrint->println("Is Linked to the physical network");
                    pPrint->print("Link status: 0x");
                    pPrint->println(status, HEX);
                }
                state = INITIALIZE;
                cSecRest = cSecInitRest;
                RestartTimer();
            }
            else if(IsIPStatusAnError(status))
            {
                if(pPrint != NULL) pPrint->println("Lost linkage to network");
                state = RESTARTREST;
                RestartTimer();
            }
            break;

    case INITIALIZE:

        // wait for initialization of the internet engine to complete
        if(deIPcK.isIPReady(&status))
            {
                if(pPrint != NULL) pPrint->println("Network Initialized");
                state = GETNETADDRESSES;
                SetLED(SLED::WORKING);
                RestartTimer();
            }
        else if(IsIPStatusAnError(status))
            {
                if(pPrint != NULL) pPrint->println("Not Initialized");
                state = ERROR;
                RestartTimer();
            }
        break;

    case GETNETADDRESSES:
        
        // at this point we know we are initialized and
        // I can get my network address as assigned by DHCP
        // I want to save them so I can restart with them
        // there is no reason for this to fail

        // This is also called during the static IP begin
        // just to get in sync with what the underlying system thinks we have.
        deIPcK.getMyIP(ipMy);
        deIPcK.getGateway(ipGateway);
        deIPcK.getSubnetMask(subnetMask);

        if(ipMyStatic.u32 == 0)
        {
            state = MAKESTATICIP;
        }
        else
        {
            stateTimeOut = PRINTADDRESSES;
            state = WAITFORTIME;
        }

        RestartTimer();
        break;
    
    case MAKESTATICIP:

        // build the requested IP for this subnet
        ipMyStatic = ipGateway;
        ipMyStatic.u8[3] = localStaticIP;

        // make sure what we built is in fact on our subnet
        if(localStaticIP != 0 && (ipMyStatic.u32 & subnetMask.u32) == (ipGateway.u32 & subnetMask.u32))
        {
            // if so, restart the IP stack and
            // use our static IP  
            state = ENDPASS1;
        }

        // if not just use our dynamaically assigned IP
        else
        {
            // otherwise just continue with our DHCP assiged IP
            ipMyStatic = ipMy;
            stateTimeOut = PRINTADDRESSES;
            state = WAITFORTIME;
        }
        RestartTimer();
        break;

    case ENDPASS1:

        SetLED(SLED::NOTREADY);

        // terminate our internet engine
        deIPcK.end();

        // if we were asked to shut down the WiFi channel as well, then disconnect
        // we should be careful to do this after DNETcK:end().
#if defined(USING_WIFI) && defined(RECONNECTWIFI)
        // disconnect the WiFi, this will free the connection ID as well.
        DWIFIcK::disconnect(conID);
        state = WIFICONNECTWITHKEY;
#else
        state = STATICIPBEGIN;
#endif 

        stateTimeOut = RESTARTREST;
        RestartTimer();
        break;

    case WAITFORTIME:
        epochTime = deIPcK.secondsSinceEpoch(&status);
        if(status == ipsTimeSinceEpoch)
        {
            GetDayAndTime(epochTime, szTemp);
            if(pPrint != NULL) pPrint->println(szTemp);
            state = PRINTADDRESSES;
            RestartTimer();
        }

        break;

    case PRINTADDRESSES:

        if(pPrint != NULL) 
        {
            IPv4    ip;
            MACADDR mac;

            pPrint->println("");

            deIPcK.getMyIP(ip);
            pPrint->print("My ");
            GetIP(ip, szTemp);
            pPrint->println(szTemp);

            deIPcK.getGateway(ip);
            pPrint->print("Gateway ");
            GetIP(ip, szTemp);
            pPrint->println(szTemp);

            deIPcK.getSubnetMask(ip);
            pPrint->print("Subnet mask: ");
            GetNumb(ip.u8, 4, '.', szTemp);
            pPrint->println(szTemp);

            // print out all of the DNS servers
            for(i=0; i<cDNSMax; i++)
            {
                deIPcK.getDNS(i, ip);
                pPrint->print("Dns");
                pPrint->print(i, DEC);
                pPrint->print(": ");
                GetIP(ip, szTemp);
                pPrint->println(szTemp);
            }

            deIPcK.getMyMac(mac);
            pPrint->print("My ");
            GetMAC(mac, szTemp);
            pPrint->println(szTemp);

            pPrint->println("");
        }

        stateTimeOut = RESTARTREST;
        RestartTimer();
        state = ISIPREADY;
        break;

    case ISIPREADY:
        if(!deIPcK.isIPReady(&status))
        {
            if(IsIPStatusAnError(status))
            {
                if(pPrint != NULL) pPrint->println("Lost connections, restarting");
                state       = RESTARTREST;
                fNWState    = NWRestarting;
                RestartTimer();
            }
            SetLED(SLED::NOTREADY);
        }
        else
        {
            if(fNWState != NWLinked)
            {
                SetLED(SLED::READY);
            }

            fNWState    = NWLinked;
            stateTimeOut = NONE;
        }

        break;

    case RESTARTNOW:
        fNWState    = NWRestarting;
        stateTimeOut = NONE;
        stateNext = TRYAGAIN;
        state = SHUTDOWN;
        break;

    case TERMINATE:
        fNWState    = NWHardError;
        stateTimeOut = NONE;
        stateNext = DONOTHING;
        state = SHUTDOWN;
        break;

    case REBOOT:
        fNWState    = NWHardError;
        stateTimeOut = NONE;
        stateNext = MCLRWAIT;
        state = SHUTDOWN;
        break;

    case RESTARTREST:
        fNWState    = NWRestarting;
        stateTimeOut = NONE;
        stateNext = RESTFORAWHILE;
        state = SHUTDOWN;
        break;

    case SHUTDOWN:  // nobody should call this but TEMINATE and RESTARTREST
 
        SetLED(SLED::NOTREADY);

        if(pPrint != NULL) pPrint->println("Shutting down");

        // terminate our internet engine
        deIPcK.end();

#if defined(USING_WIFI)
        // disconnect the WiFi, this will free the connection ID as well.
        deIPcK.wfDisconnect();
#endif

        // make sure we don't hit our timeout code
        stateTimeOut = NONE;
        state = stateNext;
        stateNext = RESTARTREST;
        break;

    case RESTFORAWHILE:
        {
            static bool fFirstEntry = true;
            static bool fPrintCountDown = true;
            static unsigned int tRestingStart = 0;
            uint32_t tCur = millis();

            if(fFirstEntry)
            {
                fFirstEntry = false;
                fPrintCountDown = true;
                 if(pPrint != NULL)
                {
               if(pPrint != NULL) pPrint->print("Resting for ");
                if(pPrint != NULL) pPrint->print(cSecRest, DEC);
                if(pPrint != NULL) pPrint->println(" seconds");
                 }
                tRestingStart = tCur;
                stateTimeOut = NONE;
            }

            // see if we are done resting
            else if((tCur - tRestingStart) >= (cSecRest * 1000))
            {
                fFirstEntry = true;
                fPrintCountDown = true;

                if(pPrint != NULL) pPrint->println("Done resting");

                cSecRest += cSecIncRest;
                if(cSecRest > cSecMaxRest) cSecRest = cSecMaxRest;

                SetTimeout(cSecDefault);
                state = TRYAGAIN;
             }

            // see if we should print out a countdown time
            else if(((tCur - tRestingStart) % 10000) == 0)
            {
                if(fPrintCountDown)
                {
                 if(pPrint != NULL)
                {
                   if(pPrint != NULL) pPrint->print(cSecRest - ((tCur - tRestingStart)/1000), DEC);
                    if(pPrint != NULL) pPrint->println(" seconds until restart.");
                 }
                    fPrintCountDown = false;
                }
            }

            // so we will print the countdown at the next interval
            else
            {
                fPrintCountDown = true;
            }
        }
        break;

    case TRYAGAIN:
        stateNext = RESTARTREST;
        stateTimeOut = RESTARTREST;
#if defined(USING_WIFI)
#if defined(USE_WPA2_PASSPHRASE) && defined(WPACALPSK)
        state = WIFICONNECTWITHKEY;
#else
        state = WIFICONNECT;
#endif
#else
        state = STATICIPBEGIN;
#endif 
        fNWState    = NWInitializing;
        RestartTimer();
        break;

    case DONOTHING:
        fNWState    = NWHardError;
        stateTimeOut = NONE;
        stateNext = DONOTHING;
        break;

    case WAITFORTIMEOUT:
        break;

    case MCLRWAIT:
        stateTimeOut = SOFTMCLR;
        state = WAITFORTIMEOUT;
        SetTimeout(1);
        RestartTimer();
        break;

    case SOFTMCLR:
        executeSoftReset(RUN_SKETCH_ON_BOOT);
        fNWState    = NWHardError;
        stateTimeOut = NONE;
        stateNext = DONOTHING;
        break;

    case ERROR:
    default:
        SetLED(SLED::NOTREADY);
        if(pPrint != NULL)
        {
            pPrint->print("Hard Error status 0x");
            pPrint->print(status, HEX);
            pPrint->println(" occurred.");
        }
        stateTimeOut    = NONE;
        state           = RESTARTREST;
        fNWState        = NWRestarting;
        break;
    }
  
    // Keep the Ethernet stack alive
    DEIPcK::periodicTasks();
    SetLED(SLED::PROCESS);

    return(fNWState);
}

