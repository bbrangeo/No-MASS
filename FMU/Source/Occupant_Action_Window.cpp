// Copyright 2015 Jacob Chapman

#include <vector>
#include <iostream>
#include "Configuration.hpp"
#include "Utility.hpp"
#include "DataStore.hpp"
#include "Occupant_Action_Window.hpp"

Occupant_Action_Window::Occupant_Action_Window() {
}

void Occupant_Action_Window::setDailyMeanTemperature(
    double dailyMeanTemperature) {
    this->dailyMeanTemperature = dailyMeanTemperature;
}

void Occupant_Action_Window::setup(int windowID, int id) {
    variableNameWindowDesire = DataStore::addVariable("Occupant_Window_Desire_"
                                                      + std::to_string(id) + "_"
                                                      + std::to_string(windowID));

    std::cerr << "Occupant_Action_Window::setup variableNameWindowDesire " << "Occupant_Window_Desire_"
            + std::to_string(id) + "_"
            + std::to_string(windowID) << std::endl;
}

void Occupant_Action_Window::saveResult() const {
    std::cerr << "Occupant_Action_Window::saveResult  " << variableNameWindowDesire << " " << result << std::endl;
    // DataStore::addValue(variableNameWindowDesire, result);
}

int Occupant_Action_Window::durationOpen() const {
    return m_window.getDurationOpen();
}
