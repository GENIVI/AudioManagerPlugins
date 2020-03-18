/******************************************************************************
 * @file: CAmPersistenceDefault.h
 *
 * This file contains the declaration of Persistence default class
 * (member functions and data members) used to implement the logic of
 * reading and writing data for last main connection and last volume
 * from the file system (RFS)
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Naohiro Nishiguchi<nnishiguchi@jp.adit-jv.com>
 *          Kapildev Patel, Yogesh Sharma <kpatel@jp.adit-jv.com>
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/
#ifndef GC_PERSISTENCE_DEFAULT_H_
#define GC_PERSISTENCE_DEFAULT_H_

#include "IAmPersistence.h"
#include "CAmXmlParserCommon.h"
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <sys/stat.h>
#include <libxml/parser.h>


#ifndef GENERIC_CONTROLLER_PERSISTENCE_FILE
#  define GENERIC_CONTROLLER_PERSISTENCE_FILE  "/var/persistence/gc_persistence_data.xml"
#endif
#define PERISTENCE_FILE_PATH_ENV_VAR_NAME      "GENERIC_CONTROLLER_PERISTENCE_FILE_PATH"
#define FILE_ACCESS_MODE                       0477


using namespace std;

namespace am {
namespace gc {

#define TAG_ROOT_PERSISTENCE      "persistence"
#define TAG_CLASS                 "class"
#define TAG_STACK                 "stack"
#define TAG_SYSTEM_PROPERTY       "systemProperty"
#define TAG_MAIN_SOUND_PROPERTIES "mainSoundProperties"
#define TAG_MAIN_SOUND_PROPERTY   "mainSoundProperty"
#define ATTRIBUTE_NAME            "name"
#define ATTRIBUTE_VOLUME          "volume"
#define TAG_CONNECTION            "connection"
#define ATTRIBUTE_SINK            "sink"
#define ATTRIBUTE_SOURCE          "source"
#define ATTRIBUTE_ORDER           "order"
#define ATTRIBUTE_LASTCON_VALID   "lastconvalid"
#define ATTRIBUTE_TYPE            "type"
#define ATTRIBUTE_VALUE           "value"
#define MAX_NEG_VOL               -32768
struct gc_persistence_connectionData_s
{
    gc_persistence_connectionData_s()
        : sourceName("")
        , volume(MAX_NEG_VOL)
        , order(-1)
        , lastConnectionValid(MAX_NEG_VOL){}
    std::string sourceName;
    int16_t volume;
    int16_t order;
    int16_t lastConnectionValid;
};

struct gc_persistence_stackData_s
{
    gc_persistence_stackData_s()
        : sinkName("")
        , volume(MAX_NEG_VOL){}
    std::vector< gc_persistence_connectionData_s > listConnection;
    std::string sinkName;
    int16_t volume;
};

struct gc_persistence_mainSoundPropertyData_s
{
    std::string type;
    std::string value;
};

struct gc_persistence_mainSoundProData_s
{
    std::vector<gc_persistence_mainSoundPropertyData_s> listMainSoundProperty;
    std::string sinkName;
    std::string sourceName;
};

struct gc_persistence_systemProperty_s
{
    std::string type;
    std::string value;
};

struct gc_persistence_classData_s
{
    gc_persistence_classData_s()
        : name("")
        , volume(MAX_NEG_VOL){}
    std::string name;
    int16_t volume;
    std::vector< gc_persistence_stackData_s > listConnections;
    std::vector<gc_persistence_mainSoundProData_s> listMainSoundProperties;
};

typedef std::map<std::string, std::string> MapData;

class CAmPersistenceNode;
class CAmPersistenceDefault : public IAmPersistence, public CAmXmlConfigParserCommon
{

public:
    CAmPersistenceDefault();
    ~CAmPersistenceDefault();
    am_Error_e open(const std::string &appName) final;
    am_Error_e read(const std::string &keyName, std::string &readData, int dataSize = READ_SIZE) final;
    am_Error_e write(const std::string &keyName, const std::string &writeData, int dataSize = WRITE_SIZE) final;
    am_Error_e close() final;
    am_Error_e _createDirectory(const std::string &path, mode_t mode);
    void _createSubDirectories();

private:
    bool _writeTOXML();
    am_Error_e _readFromXML();
    bool _stringToStruct(std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS);
    bool _lastMainConnectionVolumeStruct(std::string inputStr, std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS);
    bool _lastClassVolumeStruct(std::string inputStr, std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS);
    bool _lastMainConnectionStruct(std::string inputStr, std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS);
    bool _lastMainSoundPropertyStruct(std::string inputStr, std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS);
    bool _lastSystemPropertyStruct(std::string inputStr, std::vector<gc_persistence_classData_s > &outVClassDataS, std::vector<gc_persistence_systemProperty_s > &outVSystemPropertyS);

    void _updateMainConnectionVolume(std::string className, std::string sourceName, std::string sinkName, int16_t volume, std::vector<gc_persistence_classData_s > &outVClassDataS);
    void _updateClassVolume(gc_persistence_classData_s classData, std::vector<gc_persistence_classData_s > &outVClassDataS);
    void _updateMainSoundProperty(gc_persistence_classData_s classData, std::vector<gc_persistence_classData_s > &outVClassDataS);
    void _updateMainConnection(gc_persistence_classData_s classData, std::vector<gc_persistence_classData_s > &outVClassDataS);
    am_Error_e _structTostring(std::vector<gc_persistence_classData_s> &outVClassDataS, std::vector<gc_persistence_systemProperty_s> &outVSystemPropertyS);

    MapData             mFileData;
    std::string         mFileName;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_PERSISTENCE_DEFAULT_H_ */
