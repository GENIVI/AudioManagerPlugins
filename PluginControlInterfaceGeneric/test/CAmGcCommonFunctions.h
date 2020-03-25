/*******************************************************************************
 *  \copyright (c) 2018 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018
 *           Kapildev Patel, Satish Dwivedi <kpatel@jp.adit-jv.com> 2018
 *
 *  \copyright The MIT License (MIT)
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 ******************************************************************************/

/*
 * This file contains the default initialization of the Elements.
 */

#ifndef CAMCOMMONFUNCTIONS_H_
#define CAMCOMMONFUNCTIONS_H_

#include "audiomanagertypes.h"
#include "CAmTypes.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am {
namespace gc {

struct ut_gc_Sink_s : public gc_Sink_s
{
    ut_gc_Sink_s(std::string elementname)
    {
        _Init();
        name = elementname;
    }

    /*
     * This Constructor will be used to Construct the sink with Test Specific AM SoundProperty
     */
    ut_gc_Sink_s(std::string elementname, std::vector<am_SoundProperty_s> soundPropertiesList)
    {
        _Init();
        name                = elementname;
        listSoundProperties = soundPropertiesList;
    }

private:
    void _Init()
    {
        name                                 = "AnySink";
        domainID                             = 0;
        available.availability               = A_UNKNOWN;
        available.availabilityReason         = AR_UNKNOWN;
        sinkClassID                          = 0;
        muteState                            = MS_UNKNOWN;
        visible                              = true;
        volume                               = 0;
        sinkID                               = 0;
        domainName                           = " ";
        className                            = " ";
        registrationType                     = REG_CONTROLLER;
        isVolumeChangeSupported              = true;
        priority                             = 1;
        mapUserVolumeToNormalizedVolume[0]   = 0.0;
        mapUserVolumeToNormalizedVolume[1]   = 0.01;
        mapUserVolumeToNormalizedVolume[2]   = 0.02;
        mapUserVolumeToNormalizedVolume[3]   = 0.03;
        mapUserVolumeToNormalizedVolume[4]   = 0.04;
        mapUserVolumeToNormalizedVolume[5]   = 0.05;
        mapUserVolumeToNormalizedVolume[6]   = 0.06;
        mapUserVolumeToNormalizedVolume[7]   = 0.07;
        mapUserVolumeToNormalizedVolume[8]   = 0.08;
        mapUserVolumeToNormalizedVolume[9]   = 0.09;
        mapUserVolumeToNormalizedVolume[10]  = 0.1;
        mapUserVolumeToNormalizedVolume[11]  = 0.11;
        mapUserVolumeToNormalizedVolume[12]  = 0.12;
        mapUserVolumeToNormalizedVolume[14]  = 0.14;
        mapUserVolumeToNormalizedVolume[15]  = 0.15;
        mapUserVolumeToNormalizedVolume[17]  = 0.17;
        mapUserVolumeToNormalizedVolume[20]  = 0.2;
        mapUserVolumeToNormalizedVolume[22]  = 0.22;
        mapUserVolumeToNormalizedVolume[25]  = 0.25;
        mapUserVolumeToNormalizedVolume[28]  = 0.28;
        mapUserVolumeToNormalizedVolume[31]  = 0.31;
        mapUserVolumeToNormalizedVolume[35]  = 0.35;
        mapUserVolumeToNormalizedVolume[40]  = 0.4;
        mapUserVolumeToNormalizedVolume[45]  = 0.45;
        mapUserVolumeToNormalizedVolume[50]  = 0.5;
        mapUserVolumeToNormalizedVolume[56]  = 0.56;
        mapUserVolumeToNormalizedVolume[62]  = 0.62;
        mapUserVolumeToNormalizedVolume[70]  = 0.7;
        mapUserVolumeToNormalizedVolume[80]  = 0.8;
        mapUserVolumeToNormalizedVolume[90]  = 0.9;
        mapUserVolumeToNormalizedVolume[100] = 1.0;

        mapNormalizedVolumeToDecibelVolume[0.0]  = -192.0;
        mapNormalizedVolumeToDecibelVolume[0.01] = -40.0;
        mapNormalizedVolumeToDecibelVolume[0.02] = -34.0;
        mapNormalizedVolumeToDecibelVolume[0.03] = -30.0;
        mapNormalizedVolumeToDecibelVolume[0.04] = -27.0;
        mapNormalizedVolumeToDecibelVolume[0.05] = -26.0;
        mapNormalizedVolumeToDecibelVolume[0.06] = -24.0;
        mapNormalizedVolumeToDecibelVolume[0.07] = -23.0;
        mapNormalizedVolumeToDecibelVolume[0.08] = -22.0;
        mapNormalizedVolumeToDecibelVolume[0.09] = -21.0;
        mapNormalizedVolumeToDecibelVolume[0.1]  = -20.0;
        mapNormalizedVolumeToDecibelVolume[0.11] = -19.0;
        mapNormalizedVolumeToDecibelVolume[0.12] = -18.0;
        mapNormalizedVolumeToDecibelVolume[0.14] = -17.0;
        mapNormalizedVolumeToDecibelVolume[0.15] = -16.0;
        mapNormalizedVolumeToDecibelVolume[0.17] = -15.0;
        mapNormalizedVolumeToDecibelVolume[0.2]  = -14.0;
        mapNormalizedVolumeToDecibelVolume[0.22] = -13.0;
        mapNormalizedVolumeToDecibelVolume[0.25] = -12.0;
        mapNormalizedVolumeToDecibelVolume[0.28] = -11.0;
        mapNormalizedVolumeToDecibelVolume[0.31] = -10.0;
        mapNormalizedVolumeToDecibelVolume[0.35] = -9.0;
        mapNormalizedVolumeToDecibelVolume[0.4]  = -8.0;
        mapNormalizedVolumeToDecibelVolume[0.45] = -7.0;
        mapNormalizedVolumeToDecibelVolume[0.5]  = -6.0;
        mapNormalizedVolumeToDecibelVolume[0.56] = -5.0;
        mapNormalizedVolumeToDecibelVolume[0.62] = -4.0;
        mapNormalizedVolumeToDecibelVolume[0.7]  = -3.0;
        mapNormalizedVolumeToDecibelVolume[0.8]  = -2.0;
        mapNormalizedVolumeToDecibelVolume[0.9]  = -1.0;
        mapNormalizedVolumeToDecibelVolume[1.0]  = 0.0;
    }

};

struct ut_gc_Source_s : public gc_Source_s
{
    ut_gc_Source_s(std::string elementname)
    {
        _Init();
        name = elementname;
    }

    /*
     * This constructor is used to set the SourceState
     * during call of getSourceInfoDB
     */

    ut_gc_Source_s(am_SourceState_e state)
    {
        sourceState = state;
    }

    /*
     * This Constructor will be used to set the Source NotificationConfiguration with Source Name
     */
    ut_gc_Source_s(std::string elementname, std::vector<am_NotificationConfiguration_s> mainNotificationConfigurations)
    {
        _Init();
        name                               = elementname;
        listMainNotificationConfigurations = mainNotificationConfigurations;
    }

private:
    void _Init()
    {
        name                         = "AnySource";
        domainID                     = 0;
        available.availability       = A_UNKNOWN;
        available.availabilityReason = AR_UNKNOWN;
        sourceClassID                = 0;
        interruptState               = IS_OFF;
        visible                      = true;
        volume                       = 0;
        sourceID                     = 0;
        sourceState                  = SS_UNKNNOWN;
        domainName                   = " ";
        className                    = " ";
        registrationType             = REG_CONTROLLER;
        isVolumeChangeSupported      = true;
        priority                     = 1;
        minVolume                    = -3000;
        maxVolume                    = 3000;
        mapRoutingVolume[-3000.0]     = -3000.0;
        mapRoutingVolume[0.0]         = -100.0;
        mapRoutingVolume[3000.0]      = 0.0;
    }

};

struct ut_gc_Domain_s : public gc_Domain_s
{
    ut_gc_Domain_s(std::string elementname)
    {
        _Init();
        name = elementname;
    }

private:
    void _Init()
    {
        domainID         = 0;
        name             = "AnyDomain";
        busname          = " ";
        nodename         = " ";
        early            = true;
        complete         = false;
        state            = DS_UNKNOWN;
        registrationType = REG_CONTROLLER;
    }

};

struct ut_gc_Gateway_s : public gc_Gateway_s
{
    ut_gc_Gateway_s(std::string elementname)
    {
        _Init();
        name = elementname;
    }

private:
    void _Init()
    {
        gatewayID         = 0;
        name              = "AnyGateway";
        sinkID            = 0;
        sourceID          = 0;
        domainSinkID      = 0;
        domainSourceID    = 0;
        controlDomainID   = 0;
        listSourceFormats = { CF_UNKNOWN };
        listSinkFormats   = { CF_UNKNOWN };
        convertionMatrix  = { true };
        sinkName          = "AnySink";
        sourceName        = "AnySource";
        controlDomainName = "AnyDomain";
        registrationType  = REG_CONTROLLER;
    }

};

struct gc_ut_System_s : public gc_System_s
{
    // gc_ut_System_s(std::string SysPropname, std::vector<am_SystemProperty_s > SystemProperties)
    gc_ut_System_s(std::string SysPropname, std::vector<gc_SystemProperty_s > SystemProperties)
    {
        _Init();
        name = SysPropname;
        std::vector<gc_SystemProperty_s > listSystemProperties = SystemProperties;
    }

private:
    void _Init()
    {
        name = "";
    }

};

struct gc_ut_MainConnection_s : public am_MainConnection_s
{
    /*
     * This constructor is used to set the ConnectionState
     * during call of getMainConnectionInfoDB
     */
    gc_ut_MainConnection_s(am_ConnectionState_e connState)
    {
        connectionState = connState;
    }

};

}
}

#endif /* CAMCOMMONFUNCTIONS_H_ */
