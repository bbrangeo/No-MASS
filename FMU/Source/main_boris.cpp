// Copyright 2016 Jacob Chapman

#include "Utility.hpp"
#include "Simulation.hpp"
#include "SimulationTime.hpp"
#include "DataStore.hpp"

Simulation sim;

int main(int argc, char *argv[]) {
    if (argc > 1) {
        sim.setConfigurationurationFile(argv[1]);
    } else {
        sim.setConfigurationurationFile(
            "/Users/Boris/Documents/TIPEE/No-MASS/Configuration/HouseWindows-MD/SimulationConfig.xml");
    }
    std::cout << "setConfigurationurationFile OK: " << std::endl;

    sim.preprocess();

    std::cout << "Preprocess OK: " << std::endl;

    DataStore::clear();
    DataStore::clearValues();
    Configuration::setStepCount(-1);

    std::cout << "Configuration OK: " << std::endl;

    Configuration::info.save = true;

    Configuration::info.windows = true;
    Configuration::info.windowsLearn = true;
    Configuration::info.shading = false;
    Configuration::info.lights = true;
    Configuration::info.heating = true;

    Configuration::info.learnep = 0.8;
    Configuration::info.learnupdate = true;

    SimulationTime::preprocess();
    sim.setupSimulationModel();

    std::cout << "Info OK: " << std::endl;

    DataStore::addVariable("EnvironmentSiteOutdoorAirDrybulbTemperature");
    DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", 21);

    std::cout << "DataStore OK: " << std::endl;

    sim.preTimeStep();
    std::cout << "preTimeStep OK: " << std::endl;
    sim.timeStep();
    std::cout << "timeStep OK: " << std::endl;
    sim.postTimeStep();


    DataStore::addVariable("EnvironmentSiteOutdoorAirDrybulbTemperature");
    DataStore::addVariable("EnvironmentSiteRainStatus");

    DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", 18);
    DataStore::addValueS("EnvironmentSiteRainStatus", 0);

    DataStore::addVariable("Block1:Zone1ZoneMeanAirTemperature");
    DataStore::addVariable("Block1:Zone1ZoneAirRelativeHumidity");
    DataStore::addVariable("Block1:Zone1ZoneMeanRadiantTemperature");
    DataStore::addVariable("Block1:Zone1NumberOfOccupants");

    DataStore::addValueS("Block1:Zone1ZoneMeanAirTemperature", 21);
    DataStore::addValueS("Block1:Zone1ZoneAirRelativeHumidity", 21);
    DataStore::addValueS("Block1:Zone1ZoneMeanRadiantTemperature", 21);
    DataStore::addValueS("Block1:Zone1NumberOfOccupants", 0);

    for (int i = 1; i < 100; i++) {
        sim.preTimeStep();
        sim.timeStep();
        sim.postTimeStep();
    }

    DataStore::addValueS("Block1:Zone1ZoneMeanAirTemperature", 128);
    DataStore::addValueS("Block1:Zone1ZoneAirRelativeHumidity", 100);
    DataStore::addValueS("Block1:Zone1ZoneMeanRadiantTemperature", 100);
    DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", 5);
    DataStore::addValueS("EnvironmentSiteRainStatus", 0);

    for (int i = 1; i < 100; i++) {
        sim.preTimeStep();
        sim.timeStep();
        int occs = DataStore::getValueS("Block1:Zone1NumberOfOccupants");
        std::cout << "timeStep occs: " << occs << std::endl;

        if (occs > 0) {
            break;
        }
        sim.postTimeStep();
    }

    sim.postprocess();
}
