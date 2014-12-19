#ifndef LOCALTRACKERSETTINGS_H
#define LOCALTRACKERSETTINGS_H

#include <iostream>
#include <list>
#include "settingsIO.h"



class TrackerSetting {

public:

    TrackerSetting(const std::string& subClass,
                   const std::string& name,
                   double dMin,
                   double dMax,
                   double dValue,
                   double dInc)
    {
        this->name     = name;
        this->subClass = subClass;

        m_dMin      = dMin;
        m_dMax      = dMax;
        m_dValue    = dValue;
        m_dInc      = dInc;

    }

    const std::string& getName() { return this->name; }
    const std::string& getSubClass() { return this->subClass; }

    double getMin() { return m_dMin; }
    double getMax() { return m_dMax; }
    double getValue() { return this->m_dValue;}
    double getInc() { return this->m_dInc;}
    void setValue(double value) { this->m_dValue = value; }

private:

    double m_dValue;
    double m_dMin;
    double m_dMax;
    double m_dInc;
    std::string name;
    std::string subClass;


};


class LocalTrackerSettings {
public:
    LocalTrackerSettings(void);

    void open(SettingsIO& settingsFile);
    void save(SettingsIO& settingsFile);
    std::list<TrackerSetting>::iterator getSettingObject(const std::string& name);
    std::list<TrackerSetting>::iterator getSettingObject(const char * name);
    std::list<TrackerSetting>::iterator getFirstObject();
    std::list<TrackerSetting>::iterator getLastObject();
    std::list<TrackerSetting>& getSettingObjects() { return this->varList; }

private:
    std::list<TrackerSetting> varList;
    void addSetting(const char * file, const char * parameter, double min, double max, double defaultValue, double dInc);
};

#endif
