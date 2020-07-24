/***************************************************************************//**
 *  @file  CAmConfigurations.h
 *  
 *  Provides system topology and policy configuration for validating the 
 *  policy engine behavior.
 *  
 *  The configuration can be composed freely from predefined or test-specific
 *  modules. It is written to file /tmp/gc_utest.xml within constructor of class
 *  ConfigDocument and the environment variable GENERIC_CONTROLLER_CONFIGURATION
 *  is set accordingly to cause the configuration parser taking this file
 *  instead of the usual one. Redirection will effective during lifetime of
 *  the CAmConfigDocument instance.
 *
 *  Recommended usage is to instantiate the Default configuration inside the main()
 *  function of test cases. Nested instances can be used inside body of test cases
 *  if necessary.
 *
 *  @copyright (c) 2020 Advanced Driver Information Technology.\n
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  @authors     Martin Koch     <mkoch@de.adit-jv.com>
 *
 *  @copyright The MIT License (MIT)
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
 *//***************************************************************************/

#ifndef _GC_TEST_CONFIGURATIONS_H_
#define _GC_TEST_CONFIGURATIONS_H_


#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#include "CAmConfigurationReader.h"

namespace gc_utest
{

class ConfigTag    : public std::string
{
public:
    ConfigTag()
    {

    }

    ConfigTag(const std::string &tagName, const std::string & attributes, const std::string &content)
    {
        std::ostringstream s;
        s << "<" << tagName << " " << attributes << ">" << std::endl
          << content << std::endl
          << "</" << tagName << ">" << std::endl;

        *this += s.str();
    }

    ConfigTag(const std::string &tagName, const std::string & attributes, const std::vector<ConfigTag> &children)
    {
        std::ostringstream s;
        s << "<" << tagName << " " << attributes << ">" << std::endl;
        for (auto &child : children)
        {
           s << static_cast<std::string>(child);
        }
        s << "</" << tagName << ">" << std::endl;

        *this  += s.str();
    }

    // copy constructor and assignment operator
    ConfigTag(const std::string &tag)
        : std::string(tag)
    {

    }
    ConfigTag &operator=(const std::string &rhs)
    {
        if (this != &rhs)
        {
            static_cast<std::string>(*this) = rhs;
        }

        return *this;
    }

    ///  Predefined default configuration modules
    static const std::vector<ConfigTag> DefaultClasses;
    static const std::vector<ConfigTag> DefaultTemplates;
    static const std::vector<ConfigTag> DefaultSystem;

    static const ConfigTag              DefaultConnectPolicy;
    static const ConfigTag              DefaultDisonnectPolicy;
    static const ConfigTag              DefaultVolumePolicy;
    static const ConfigTag              DefaultMutePolicy;
    static const ConfigTag              DefaultSinkPropertyPolicy;
    static const ConfigTag              DefaultSinkPropertiesPolicy;
    static const ConfigTag              DefaultSourcePropertyPolicy;
    static const ConfigTag              DefaultSourcePropertiesPolicy;
    static const ConfigTag              DefaultSystemPolicy;
    static const std::vector<ConfigTag> DefaultPolicies;

    static const std::vector<ConfigTag> DefaultProperties;
};

#define TMP_CONFIG_PATH "/tmp/gc_utest.xml"

class ConfigDocument
{
public:
    ConfigDocument(const std::vector<ConfigTag> &children)
    {
        // memorize current configuration settings
        const char* pPath = getenv("GENERIC_CONTROLLER_CONFIGURATION");
        if (pPath)
        {
            _outerPath = pPath;
            if (_outerPath == TMP_CONFIG_PATH)  // nested configuration replacement
            {
                // memorize content of previous temporary configuration
                _outerXml = static_cast<std::ostringstream&>
                    (std::ostringstream{} << std::ifstream{_outerPath}.rdbuf()).str();
            }
        }

        // establish temporary configuration file and redirect search path
        std::ofstream s(TMP_CONFIG_PATH);
        s << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
          << "<c:generic xmlns:c=\"controller\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
          << " xsi:schemaLocation=\"controller audiomanagertypes.xsd \">" << std::endl;
        for (auto &child : children)
        {
            s << static_cast<std::string>(child);
        }
        s << "</c:generic>" << std::endl;
        setenv("GENERIC_CONTROLLER_CONFIGURATION", TMP_CONFIG_PATH, true);

        if (_outerPath == TMP_CONFIG_PATH)  // nested configuration replacement
        {
            am::gc::CAmConfigurationReader::instance().reload();
        }
    }

    ~ConfigDocument()
    {
        // undo temporary configuration redirection
        if (_outerPath.empty())
        {
            unsetenv("GENERIC_CONTROLLER_CONFIGURATION");
        }
        else if (_outerPath == TMP_CONFIG_PATH)  // nested configuration replacement
        {
            // restore content of previous temporary configuration
            std::ofstream(TMP_CONFIG_PATH) << _outerXml;
            am::gc::CAmConfigurationReader::instance().reload();
        }
        else
        {
            setenv("GENERIC_CONTROLLER_CONFIGURATION", _outerPath.c_str(), true);
        }
    }

    static const std::vector<ConfigTag> Default;

private:
    std::string       _outerPath;
    std::string       _outerXml;
};

#undef TMP_CONFIG_PATH

/***************************************************************************//**
 *                   Predefined Default Configuration Modules
 *
 *//***************************************************************************/

const std::vector<ConfigTag> ConfigTag::DefaultClasses
  = {
        ConfigTag("class", "name=\"BASE\" defaultVolume=\"10\" priority=\"1\" type=\"C_PLAYBACK\""
                    , "<topology>AMP=Gateway0(MediaPlayer)</topology>\n"
                        "<property type=\"CP_PER_SINK_CLASS_VOLUME_SUPPORT\" value=\"1\" />\n")
      , ConfigTag("class", "name=\"NAVI\" defaultVolume=\"25\" priority=\"0\" type=\"C_PLAYBACK\""
                 , "<topology>AMP=Gateway1(Navigation)</topology>\n"
                   "<property type=\"CP_PER_SINK_CLASS_VOLUME_SUPPORT\" value=\"0\" />\n")
    };

const std::vector<ConfigTag> ConfigTag::DefaultTemplates
  = {
        ConfigTag("scaleConversionMap", "name=\"MainVolumeScale\"", "0,0.0;1,0.8;10,1.0")
      , ConfigTag("source", "name=\"SourceTemplate\" visible=\"TRUE\" sourceState=\"SS_OFF\""
                  " connectionFormat=\"CF_GENIVI_STEREO\" priority=\"1\" minVolume=\"-3000\""
                  " maxVolume=\"3000\" sourceVolumeMap=\"-3000,-3000;0.0,-100.0;3000.0,0.0\""
                  " registrationType=\"REG_CONTROLLER\" isVolumeSupported=\"FALSE\""
              , "<availability availability=\"A_AVAILABLE\" availabilityReason=\"AR_UNKNOWN\" />\n"
                "<soundProperty type=\"SP_GENIVI_TREBLE\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<soundProperty type=\"SP_GENIVI_MID\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<soundProperty type=\"SP_GENIVI_BASS\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<mainSoundProperty type=\"MSP_GENIVI_TREBLE\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<mainSoundProperty type=\"MSP_GENIVI_MID\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<mainSoundProperty type=\"MSP_GENIVI_BASS\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<mapMSPtoSP mappingDirection=\"MD_BOTH\">MSP_GENIVI_TREBLE,SP_GENIVI_TREBLE</mapMSPtoSP>\n"
                "<mapMSPtoSP mappingDirection=\"MD_BOTH\">MSP_GENIVI_MID,SP_GENIVI_MID</mapMSPtoSP>\n"
                "<mapMSPtoSP mappingDirection=\"MD_BOTH\">MSP_GENIVI_BASS,SP_GENIVI_BASS</mapMSPtoSP>\n"
                "<property type=\"CP_PER_SINK_CLASS_VOLUME_SUPPORT\" value=\"1\" />\n")
      , ConfigTag("sink", "name=\"SinkTemplate\" mainVolume=\"10\" muteState=\"MS_UNMUTED\""
                  " connectionFormat=\"CF_GENIVI_STEREO\" isVolumeSupported=\"FALSE\" priority=\"1\""
                  " registrationType=\"REG_CONTROLLER\""
              , "<availability availability=\"A_AVAILABLE\" availabilityReason=\"AR_GENIVI_NEWMEDIA\" />\n"
                "<soundProperty type=\"SP_GENIVI_TREBLE\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<soundProperty type=\"SP_GENIVI_MID\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<soundProperty type=\"SP_GENIVI_BASS\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<mainSoundProperty type=\"MSP_GENIVI_TREBLE\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<mainSoundProperty type=\"MSP_GENIVI_MID\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<mainSoundProperty type=\"MSP_GENIVI_BASS\" value=\"5\" minValue=\"0\" maxValue=\"10\" />\n"
                "<mapMSPtoSP mappingDirection=\"MD_BOTH\">MSP_GENIVI_TREBLE,SP_GENIVI_TREBLE</mapMSPtoSP>\n"
                "<mapMSPtoSP mappingDirection=\"MD_BOTH\">MSP_GENIVI_MID,SP_GENIVI_MID</mapMSPtoSP>\n"
                "<mapMSPtoSP mappingDirection=\"MD_BOTH\">MSP_GENIVI_BASS,SP_GENIVI_BASS</mapMSPtoSP>\n")
       , ConfigTag("gateway", "name=\"GatewayTemplate\" conversionMatrix=\"CF_GENIVI_STEREO,CF_GENIVI_STEREO\"", "")
    };

const std::vector<ConfigTag> ConfigTag::DefaultSystem
  = {
          ConfigTag("templates", "", ConfigTag::DefaultTemplates)
        , ConfigTag("domain", "name=\"Applications\" busName=\"busName1\" nodeName=\"Cpu\""
              , "<source id=\"1\" baseName=\"SourceTemplate\" name=\"MediaPlayer\" className=\"BASE\">\n"
                "<gateway name=\"Gateway0\" sinkName=\"Gateway0\" />\n"
                "</source>\n"
                "<source id=\"2\" baseName=\"SourceTemplate\" name=\"Navigation\" className=\"NAVI\">\n"
                "<gateway name=\"Gateway1\" sinkName=\"Gateway1\" />\n"
                "</source>\n"
                "<sink baseName=\"SinkTemplate\" id=\"0\" name=\"Gateway0\" className=\"BASE\" />\n"
                "<sink baseName=\"SinkTemplate\" id=\"0\" name=\"Gateway1\" className=\"NAVI\" />\n")
        , ConfigTag("domain", "name=\"VirtDSP\" busName=\"busName2\" nodeName=\"Cpu\""
              , "<source baseName=\"SourceTemplate\" name=\"Gateway0\" className=\"BASE\""
                  " registrationType=\"REG_ROUTER\" isVolumeSupported=\"TRUE\" />\n"
                "<source baseName=\"SourceTemplate\" name=\"Gateway1\" className=\"NAVI\""
                  " registrationType=\"REG_ROUTER\" isVolumeSupported=\"TRUE\" />\n"
                "<sink baseName=\"SinkTemplate\" id=\"1\" name=\"AMP\" className=\"BASE\""
                  " isVolumeSupported=\"TRUE\" listMainVolumesToNormalizedVolumes=\"MainVolumeScale\""
                  " listNormalizedVolumesToDecibelVolumes=\"0.0,-3000.0;1.0,0.0\" />\n")
    };

const ConfigTag ConfigTag::DefaultConnectPolicy("policy", "trigger=\"USER_CONNECTION_REQUEST\""
        , "<process name=\"BASE class connection policy\">\n"
          "  <condition>name(CLASS, REQUESTING) EQ \"BASE\"</condition>\n"
          "  <condition>connectionState(CONNECTIONOFCLASS, REQUESTING, OTHERS) INC CS_CONNECTED</condition>\n"
          "  <action type=\"ACTION_DISCONNECT\" className=\"N:BASE\" connectionState=\"CS_CONNECTED\" />\n"
          "</process>\n"
          "<process name=\"NAVI class connection policy\">\n"
          "  <condition>name(CLASS, REQUESTING) EQ \"NAVI\"</condition>\n"
          "  <action type=\"ACTION_LIMIT\" className=\"N:BASE\" volume=\"-1500\" rampTime=\"1000\" rampType=\"RAMP_GENIVI_LINEAR\" />\n"
          "</process>\n"
          "<process>\n"
          "  <action type=\"ACTION_CONNECT\" className=\"REQUESTING\" timeOut=\"5000\" />\n"
          "</process>");

const ConfigTag ConfigTag::DefaultDisonnectPolicy("policy", "trigger=\"USER_DISCONNECTION_REQUEST\""
        , "<process name=\"Default disconneciton policy\">\n"
          "  <action type=\"ACTION_DISCONNECT\" sourceName=\"REQUESTING\" sinkName=\"REQUESTING\" />\n"
          "</process>\n"
          "<process>\n"
          "  <condition>name(CLASS, REQUESTING) EQ \"NAVI\"</condition>\n"
          "  <action type=\"ACTION_UNLIMIT\" className=\"N:BASE\" rampTime=\"1000\" rampType=\"RAMP_GENIVI_LINEAR\" />\n"
          "</process>");

const ConfigTag ConfigTag::DefaultVolumePolicy("policy", "trigger=\"USER_SET_VOLUME\""
        , "<process name=\"SINK volume policy\">\n"
          "  <action type=\"ACTION_SET_VOLUME\" sinkName=\"REQUESTING\" />\n"
          "</process>");

const ConfigTag ConfigTag::DefaultMutePolicy("policy", "trigger=\"USER_SET_SINK_MUTE_STATE\""
        , "<process name=\"Navi class mute policy\">\n"
          "  <condition>muteState(USER,REQUESTING) EQ MS_MUTED</condition>\n"
          "  <condition>connectionState(CONNECTIONOFCLASS, \"NAVI\") INC CS_CONNECTED</condition>\n"
          "  <action type=\"ACTION_MUTE\" className=\"NAVI\" />\n"
          "  <action type=\"ACTION_DISCONNECT\" className=\"NAVI\" />\n"
          "</process>\n"
          "<process>\n"
          "  <condition>muteState(USER,REQUESTING) EQ MS_MUTED</condition>\n"
          "  <condition>connectionState(CONNECTIONOFCLASS, \"NAVI\") INC CS_CONNECTED</condition>\n"
          "  <condition>connectionState(CONNECTIONOFCLASS, \"BASE\") INC CS_CONNECTED</condition>\n"
          "  <action type=\"ACTION_UNLIMIT\" className=\"N:BASE\" rampTime=\"1000\" rampType=\"RAMP_GENIVI_LINEAR\" />\n"
          "  <break>TRUE</break>\n"
          "</process>\n"
          "<process>\n"
          "  <condition>muteState(USER,REQUESTING) EQ MS_MUTED</condition>\n"
          "  <condition>connectionState(CONNECTIONOFCLASS, \"BASE\") INC CS_CONNECTED</condition>\n"
          "  <action type=\"ACTION_SUSPEND\" className=\"N:BASE\" />\n"
          "</process>\n"
          "<process>\n"
          "  <condition>muteState(USER,REQUESTING) EQ MS_UNMUTED</condition>\n"
          "  <condition>connectionState(CONNECTIONOFCLASS, \"BASE\") INC CS_SUSPENDED</condition>\n"
          "  <action type=\"ACTION_RESUME\" className=\"BASE\" order=\"O_NEWEST\" />\n"
          "</process>");

const ConfigTag ConfigTag::DefaultSinkPropertyPolicy("policy", "trigger=\"USER_SET_SINK_MAIN_SOUND_PROPERTY\""
        , "<process>\n"
          "  <action type=\"ACTION_SET_PROPERTY\" sinkName=\"REQUESTING\" />\n"
          "</process>");

const ConfigTag ConfigTag::DefaultSinkPropertiesPolicy("policy", "trigger=\"USER_SET_SINK_MAIN_SOUND_PROPERTIES\""
        , "<process>\n"
          "  <action type=\"ACTION_SET_PROPERTIES\" sinkName=\"REQUESTING\" listMainSoundProperties=\"REQUESTING\" />\n"
          "</process>");

const ConfigTag ConfigTag::DefaultSourcePropertyPolicy("policy", "trigger=\"USER_SET_SOURCE_MAIN_SOUND_PROPERTY\""
        , "<process>\n"
          "  <action type=\"ACTION_SET_PROPERTY\" sourceName=\"REQUESTING\" />\n"
          "</process>");

const ConfigTag ConfigTag::DefaultSourcePropertiesPolicy("policy", "trigger=\"USER_SET_SOURCE_MAIN_SOUND_PROPERTIES\""
        , "<process>\n"
          "  <action type=\"ACTION_SET_PROPERTIES\" sourceName=\"REQUESTING\" listMainSoundProperties=\"REQUESTING\" />\n"
          "</process>");

const ConfigTag ConfigTag::DefaultSystemPolicy("policy", "trigger=\"USER_SET_SYSTEM_PROPERTIES\""
        , "<process>\n"
          "  <action type=\"ACTION_SET_SYSTEM_PROPERTIES\" listSystemProperties=\"REQUESTING\" />\n"
          "</process>");

const std::vector<ConfigTag> ConfigTag::DefaultPolicies
  = {
          DefaultConnectPolicy
        , DefaultDisonnectPolicy
        , DefaultVolumePolicy
        , DefaultMutePolicy
        , DefaultSinkPropertyPolicy
        , DefaultSinkPropertiesPolicy
        , DefaultSourcePropertyPolicy
        , DefaultSourcePropertiesPolicy
        , DefaultSystemPolicy
    };

const std::vector<ConfigTag> ConfigTag::DefaultProperties
  = {
          ConfigTag("systemProperty", "type=\"SYP_GLOBAL_LOG_THRESHOLD\" value=\"4\"", "")
        , ConfigTag("systemProperty", "type=\"SYP_REGISTRATION_ALLOW_UNKNOWN_ELEMENT\" value=\"1\"", "")
    };

const std::vector<ConfigTag> ConfigDocument::Default
  = {
          ConfigTag("classes", "", ConfigTag::DefaultClasses)
        , ConfigTag("system", "", ConfigTag::DefaultSystem)
        , ConfigTag("policies", "", ConfigTag::DefaultPolicies)
        , ConfigTag("properties", "", ConfigTag::DefaultProperties)
    };


}  // namespace gc_utest


#endif /* _GC_TEST_CONFIGURATIONS_H_ */
