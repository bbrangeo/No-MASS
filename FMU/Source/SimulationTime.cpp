// Copyright 2015 Jacob Chapman

#include <vector>
#include <string>
#include "Configuration.hpp"
#include "DataStore.hpp"
#include "SimulationTime.hpp"

#include <iostream>
#include <ostream>

std::vector<int> SimulationTime::monthCount;
int SimulationTime::stepCount = 0;
int SimulationTime::minute = 0;
int SimulationTime::hour = 0;
int SimulationTime::day = 0;
int SimulationTime::month = 0;
int SimulationTime::hourOfDay = 0;
int SimulationTime::minuteOfDay = 0;
int SimulationTime::databaseIdStepCount;
int SimulationTime::databaseIdHour;
int SimulationTime::databaseIdDay;
int SimulationTime::databaseIdMonth;
int SimulationTime::databaseIdHourOfDay;
int SimulationTime::databaseIdMinuteOfDay;

SimulationTime::SimulationTime() {}

void SimulationTime::reset() {
  SimulationTime::stepCount = 0;//-1;
  SimulationTime::minute = 0;
  SimulationTime::hour = 0;
  SimulationTime::day = 0;
  SimulationTime::month = 0;
  SimulationTime::hourOfDay = 0;
  SimulationTime::minuteOfDay = 0;
}

/**
 * @brief Calls the SimulationTime preprocess
 * @details Reads in the configuration file and sends to parser.
 * Sets up the EnergyPlus processor, the AgentModel and the ZoneManager.
 */
void SimulationTime::preprocess() {
  monthCount.push_back(31);
  monthCount.push_back(59);
  monthCount.push_back(90);
  monthCount.push_back(120);
  monthCount.push_back(151);
  monthCount.push_back(181);
  monthCount.push_back(212);
  monthCount.push_back(243);
  monthCount.push_back(273);
  monthCount.push_back(304);
  monthCount.push_back(334);
  monthCount.push_back(365);
  databaseIdStepCount = DataStore::addVariable("TimeStep");
  databaseIdHour = DataStore::addVariable("hour");
  databaseIdDay = DataStore::addVariable("day");
  databaseIdMonth = DataStore::addVariable("month");
  databaseIdHourOfDay = DataStore::addVariable("hourOfDay");
  databaseIdMinuteOfDay = DataStore::addVariable("minuteOfDay");
}

/**
 * @brief processes before timestep
 */
void SimulationTime::trackTime() {
  Configuration::step();
  stepCount = Configuration::getStepCount();
  std::cout << "Step Count: " << stepCount << std::endl;
  std::cout << "\n " << stepCount << std::endl;

  minute = (stepCount * Configuration::lengthOfTimestep()) / 60;
  hour = minute / 60;
  day = hour / 24;

  month = 1;
  for (int mc : monthCount) {
    if (mc > day || month + 1 > 12) {
      break;
    }
    month = month + 1;
  }

  hourOfDay = hour % 24;
  minuteOfDay = minute % 1440;

  DataStore::addValue(databaseIdStepCount, stepCount);
  DataStore::addValue(databaseIdHour, hour);
  DataStore::addValue(databaseIdDay, day);
  DataStore::addValue(databaseIdMonth, month);
  DataStore::addValue(databaseIdHourOfDay, hourOfDay);
  DataStore::addValue(databaseIdMinuteOfDay, minuteOfDay);

}
