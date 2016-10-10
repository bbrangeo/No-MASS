// Copyright 2015 Jacob Chapman

#include <vector>
#include <iostream>
#include "SimulationConfig.h"
#include "Utility.h"
#include "DataStore.h"
#include "Occupant_Action_Window.h"

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
}

void Occupant_Action_Window::saveResult() {
  DataStore::addValue(variableNameWindowDesire, result);
}

int Occupant_Action_Window::durationOpen() const {
    return m_window.getDurationOpen();
}
