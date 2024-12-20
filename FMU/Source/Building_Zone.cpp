// Copyright 2016 Jacob Chapman

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include "DataStore.hpp"
#include "Configuration.hpp"
#include "Building_Zone.hpp"

Building_Zone::Building_Zone() {
}

/**
 * @brief Set up the zone from the configuration struct
 * @details Setup the zone, adds the zone values to the data store
 * @param zoneStruct The configuration struct which values have been populated form configuration file
 */
void Building_Zone::setup(const ConfigStructZone &zoneStruct) {
    id = zoneStruct.id;
    activities = zoneStruct.activities;
    lightPower = zoneStruct.lightPower;
    occupantFraction = 0;
    currentOccupantGains = 0;
    blindState = 1;
    lightState = 0;
    windowState = 0;

    if (active) {
        int windowCount = zoneStruct.windowCount;
        setGroundFloor(Configuration::isZoneGroundFloor(&name));
        for (int i = 0; i < windowCount; i++) {
            int windowid =
                    DataStore::addVariable(name + "WindowState" + std::to_string(i));
            variableNameWindow.push_back(windowid);
        }

        variableNameAppFraction =
                DataStore::addVariable(name + "AppFraction");
        variableNameBlindFraction =
                DataStore::addVariable(name + "BlindFraction");
        variableNameLight =
                DataStore::addVariable(name + "LightState");
        variableNameHeating =
                DataStore::addVariable(name + "Heating");
        variableNameNumberOfOccupants =
                DataStore::addVariable(name + "NumberOfOccupants");
        variableNameAverageGains =
                DataStore::addVariable(name + "AverageGains");
        variableNameZoneMeanAirTemperature =
                DataStore::addVariable(name + "ZoneMeanAirTemperature");
        variableNameZoneAirRelativeHumidity =
                DataStore::addVariable(name + "ZoneAirRelativeHumidity");
        variableNameZoneMeanRadiantTemp =
                DataStore::addVariable(name + "ZoneMeanRadiantTemperature");
        variableNameDaylighting =
                DataStore::addVariable(name + "DaylightingReferencePoint1Illuminance");
        variableNameZoneAirHeating =
                DataStore::addVariable(name + "ZoneAirSystemSensibleHeatingRate");
    }
}

/**
 * @brief the zones step function
 * @details saves the zone values to the data store at each timestep
 */
void Building_Zone::step() {
    if (active) {
        DataStore::addValue(variableNameNumberOfOccupants, occupantFraction);
        DataStore::addValue(variableNameAverageGains, currentOccupantGains);
        for (int wname: variableNameWindow) {
            DataStore::addValue(wname, windowState);
        }
        DataStore::addValue(variableNameLight, lightState * lightPower);
        // Shade in EP is
        // 1 is closed
        // 0 is open
        // Shade in No-mass unshaded fraction
        // 1 is open
        // 0 is closed
        DataStore::addValue(variableNameBlindFraction, 1 - blindState);
        DataStore::addValue(variableNameHeating, heatingState);
        DataStore::addValue(variableNameAppFraction, appFraction);
    }
}

void Building_Zone::setName(const std::string &name) {
    this->name = name;
}

bool Building_Zone::isNamed(const std::string &name) const {
    return this->name == name;
}

int Building_Zone::getId() const {
    return id;
}

void Building_Zone::setCurrentOccupantGains(double currentOccupantGains) {
    this->currentOccupantGains = currentOccupantGains;
}

double Building_Zone::getCurrentOccupantGains() const {
    return currentOccupantGains;
}

double Building_Zone::getWindowState() const {
    return windowState;
}

double Building_Zone::getBlindState() const {
    return blindState;
}

double Building_Zone::getHeatingState() const {
    return heatingState;
}

std::vector<int> Building_Zone::getActivities() const {
    return activities;
}

void Building_Zone::setOccupantFraction(double occupantFraction) {
    this->occupantFraction = occupantFraction;
}

double Building_Zone::getMeanAirTemperature() const {
    return DataStore::getValue(variableNameZoneMeanAirTemperature);
}

double Building_Zone::getAirRelativeHumidity() const {
    return DataStore::getValue(variableNameZoneAirRelativeHumidity);
}

double Building_Zone::getMeanRadiantTemperature() const {
    return DataStore::getValue(variableNameZoneMeanRadiantTemp);
}

double Building_Zone::getDaylightingReferencePoint1Illuminance() const {
    return DataStore::getValue(variableNameDaylighting);
}

double Building_Zone::getAirSystemSensibleHeatingRate() const {
    return DataStore::getValue(variableNameZoneAirHeating);
}

float Building_Zone::getOccupantFraction() const {
    return occupantFraction;
}

void Building_Zone::setActive(bool active) {
    this->active = active;
}

bool Building_Zone::isActive() const {
    return active;
}

bool Building_Zone::hasActivity(int activity) const {
    return std::find(activities.begin(), activities.end(), activity)
           != activities.end();
}

void Building_Zone::setGroundFloor(bool groundFloor) {
    this->groundFloor = groundFloor;
}

double Building_Zone::getGroundFloor() const {
    return groundFloor;
}

void Building_Zone::setLightState(bool lightState) {
    this->lightState = lightState;
}

void Building_Zone::setHeatingState(double state) {
    this->heatingState = state;
}

double Building_Zone::getLightState() const {
    return lightState;
}

void Building_Zone::setWindowState(bool windowState) {
    this->windowState = windowState;
}

void Building_Zone::setBlindState(double state) {
    this->blindState = state;
}

void Building_Zone::setAppFraction(double appFraction) {
    this->appFraction = appFraction;
}

int Building_Zone::getNumberOfActivities() const {
    return activities.size();
}

void Building_Zone::setIDString(const std::string &idString) {
    this->idString = idString;
}
