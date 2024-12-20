// Copyright 2016 Jacob Chapman

#include "Utility.hpp"
#include "Simulation.hpp"

Simulation sim;

int main(int argc, char *argv[]) {
    if (argc > 1) {
        sim.setConfigurationurationFile(argv[1]);
    }

    sim.preprocess();

    int days = Utility::calculateNumberOfDays(Configuration::info.startDay,
                                              Configuration::info.startMonth,
                                              Configuration::info.endDay,
                                              Configuration::info.endMonth);

    std::cout << "NoMASS.exe => days : " << days << std::endl;

    int totoaltimesteps = days * 24 * Configuration::info.timeStepsPerHour;

    std::cout << "NoMASS.exe => totoaltimesteps (days * 24 * Configuration::info.timeStepsPerHour) : " << totoaltimesteps << std::endl;

    for (int i = 0; i < totoaltimesteps; i++) {
        sim.preTimeStep();
        sim.timeStep();
        sim.postTimeStep();
    }
    sim.postprocess();
}
