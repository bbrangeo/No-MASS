// Copyright 2016 Jacob Chapman

#include <deque>
#include "DataStore.hpp"
#include "Configuration.hpp"
#include "Environment.hpp"

#include <iostream>
#include <ostream>

double Environment::dailyMeanTemperature = 0;
std::deque<double> Environment::outDoorTemperatures;

Environment::Environment() {
}

double Environment::getDailyMeanTemperature() {
  return dailyMeanTemperature;
}

void Environment::calculateDailyMeanTemperature() {
  if (Configuration::info.windows) {
    double outdoorTemperature = getOutdoorAirDrybulbTemperature();
    std::cout << "outdoorTemperature " << outdoorTemperature << "°C" << std::endl;

    outDoorTemperatures.push_back(outdoorTemperature);

    if (outDoorTemperatures.size() >
        (Configuration::info.timeStepsPerHour * 24)) {
      outDoorTemperatures.pop_front();
    }

    dailyMeanTemperature = 0;
    for (const double temp: outDoorTemperatures) {
      dailyMeanTemperature += temp;
      // std::cout << "outdoorTemperature " << dailyMeanTemperature << "°C" << std::endl;

    }

    dailyMeanTemperature =
        dailyMeanTemperature /
        static_cast<double>(outDoorTemperatures.size());
  }
  std::cout << "Calculating daily mean temperature " << dailyMeanTemperature << std::endl;
}


double Environment::getEVG() {
  return DataStore::getValueS("EnvironmentSiteExteriorHorizontalSkyIlluminance");
}


double Environment::getOutdoorAirDrybulbTemperature() {
  return DataStore::getValueS("EnvironmentSiteOutdoorAirDrybulbTemperature");
}
