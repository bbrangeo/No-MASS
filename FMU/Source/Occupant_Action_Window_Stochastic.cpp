// Copyright 2015 Jacob Chapman

#include <vector>
#include <iostream>
#include "Configuration.hpp"
#include "Utility.hpp"
#include "DataStore.hpp"
#include "Environment.hpp"
#include "Occupant_Action_Window_Stochastic.hpp"

Occupant_Action_Window_Stochastic::Occupant_Action_Window_Stochastic() {
}

void Occupant_Action_Window_Stochastic::setup(int windowID, int id) {
    ConfigStructWindow ws = Configuration::windows.at(windowID);
    m_window.setDurationVars(ws.aop, ws.bopout, ws.shapeop);
    m_window.setArrivalVars(ws.a01arr, ws.b01inarr, ws.b01outarr,
                            ws.b01absprevarr, ws.b01rnarr);
    m_window.setInterVars(ws.a01int, ws.b01inint, ws.b01outint,
                          ws.b01presint, ws.b01rnint);
    m_window.setDepartureVars(ws.a01dep, ws.b01outdep, ws.b01absdep,
                              ws.b01gddep, ws.a10dep, ws.b10indep, ws.b10outdep, ws.b10absdep,
                              ws.b10gddep);

    variableNameWindowDesire = DataStore::addVariable("Occupant_Window_Desire_"
                                                      + std::to_string(id) + "_"
                                                      + std::to_string(windowID));

    std::cerr << "Occupant_Action_Window_Stochastic::setup variableNameWindowDesire " << "Occupant_Window_Desire_"
                                                      + std::to_string(id) + "_"
                                                      + std::to_string(windowID) << std::endl;
}

void Occupant_Action_Window_Stochastic::step(const Building_Zone &zone,
                                             const bool inZone,
                                             const bool previouslyInZone, const std::vector<double> &activities) {
    double outdoorTemperature = Environment::getOutdoorAirDrybulbTemperature();

    double rain = DataStore::getValueS("EnvironmentSiteRainStatus");
    // double rain = 0;
    double indoorTemperature = zone.getMeanAirTemperature();
    double timeStepLengthInMinutes = Configuration::lengthOfTimestep() / 60;

    m_window.setWindowState(zone.getWindowState());
    if (m_window.getWindowState() == 0) {
        m_window.setDurationOpen(0);
    }
    if (inZone && !previouslyInZone) {
        double previousDuration = getPreviousDurationOfAbsenceState(activities);
        m_window.arrival(indoorTemperature,
                         outdoorTemperature, previousDuration, rain, timeStepLengthInMinutes);
    } else if (inZone && previouslyInZone) {
        double currentDuration = getCurrentDurationOfPresenceState(activities);
        m_window.intermediate(indoorTemperature,
                              outdoorTemperature, currentDuration, rain, timeStepLengthInMinutes);
    } else if (!inZone && previouslyInZone) {
        double groundFloor = zone.getGroundFloor();
        double futureDuration = getFutureDurationOfAbsenceState(activities);
        m_window.departure(
            indoorTemperature, dailyMeanTemperature, futureDuration, groundFloor);
    }
    result = m_window.getWindowState();
}
