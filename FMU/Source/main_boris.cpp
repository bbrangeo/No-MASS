// Copyright 2024 Boris Brangeon

#include <iterator>
#include <vector>
#include <string>
#include <cstring>
#include <cstddef>
#include <unistd.h> // Pour getopt
#include <ctime>
#include <random>
#include <iostream>
#include <cstdlib>

#include "rapidxml_utils.hpp"
#include "rapidxml.hpp"
#include "Utility.hpp"
#include "Simulation.hpp"
#include "SimulationTime.hpp"
#include "DataStore.hpp"
#include "Configuration.hpp"
#include "Log.hpp"

std::map<int, std::string> valToRefs;

Simulation sim;


// Fonction pour générer un booléen aléatoire
bool randomBool() {
    std::random_device rd; // Graine pour le générateur
    std::mt19937 gen(rd()); // Générateur Mersenne Twister
    std::uniform_int_distribution<int> dist(0, 1); // Distribution de 0 à 1

    return dist(gen) == 1;
}

// Fonction pour générer un float aléatoire borné
float randomFloat(float min, float max) {
    if (min > max) {
        std::swap(min, max); // S'assurer que min est inférieur ou égal à max
    }

    // Générateur de nombre aléatoire
    std::random_device rd; // Graine pour le générateur
    std::mt19937 gen(rd()); // Générateur Mersenne Twister
    std::uniform_real_distribution<float> dist(min, max);

    return dist(gen);
}

/**
 * @brief Checks the modelDescription file for parameter names
 * @details Checks the modelDescription file so the agent model know which variables in the array are which
 */
void loadVariables(std::string filename) {
    LOG << "\nLoading XML file: -" << filename << "-\n\n";

    namespace rx = rapidxml;
    rx::file<> xmlFile(filename.c_str()); // Default template is char
    rx::xml_document<> doc;
    doc.parse<0>(xmlFile.data()); // 0 means default parse flags
    rx::xml_node<> *root_node = doc.first_node("fmiModelDescription");
    rx::xml_node<> *mv_node = root_node->first_node("ModelVariables");
    rx::xml_node<> *node = mv_node->first_node("ScalarVariable");
    while (node) {
        rx::xml_attribute<> *pAttr;
        pAttr = node->first_attribute("name");
        std::string name = pAttr->value();
        pAttr = node->first_attribute("causality");
        std::string causality = pAttr->value();
        pAttr = node->first_attribute("valueReference");
        int valueReference = std::stoi(pAttr->value());
        Configuration::outputRegexs.push_back(name);
        DataStore::addVariable(name);
        if (causality.compare("input") == 0) {
            std::cout << "added: " << name << std::endl;
        } else {
            rx::xml_node<> *cnode = node->first_node();
            double starValue = 0;
            if (std::strcmp(cnode->name(), "Real") == 0) {
                pAttr = cnode->first_attribute("start");
                starValue = std::stod(pAttr->value());
            }
            DataStore::addValueS(name, starValue);
        }
        valToRefs[valueReference] = name;
        node = node->next_sibling();
    }
    LOG << "\nEND Loaded XML file: -" << filename << "-\n\n";
}

int main(int argc, char *argv[]) {
    /* RESET */

    DataStore::clear();
    DataStore::clearValues();
    Configuration::setStepCount(-1);

    /* DECLARATION */

    std::string filenameModelDescription;
    std::string filenameSimulationConfig;

    float temperature = 0.0;
    bool help = false;
    int option;

    /* OPTIONS LINE COMMAND */

    // Utiliser getopt pour analyser les options
    while ((option = getopt(argc, argv, "hc:d:t:")) != -1) {
        switch (option) {
            case 'h': // Option -h pour afficher l'aide
                help = true;
                break;
            case 'c': // Option -c pour un fichier
                filenameSimulationConfig = optarg; // `optarg` contient l'argument suivant l'option
                break;
            case 'd': // Option -d pour un fichier
                filenameModelDescription = optarg; // `optarg` contient l'argument suivant l'option
                break;
            case 't': // Option -t pour un nombre
                try {
                    temperature = std::stof(optarg); // Convertir en float
                } catch (const std::invalid_argument &) {
                    std::cerr << "Error: Invalid number format for -t option.\n";
                    return 1;
                }
                break;
            case '?': // Option inconnue
                std::cerr << "Unknown option: " << static_cast<char>(optopt) << std::endl;
                return 1;
            default:
                std::cerr << "Usage: " << argv[0] <<
                        " [-c SimulationConfig.xml] [-d modelDescription.xml] [-t temperature] [-h]" << std::endl;
                return 1;
        }
    }

    // Afficher l'aide si demandé
    if (help) {
        std::cout << "Usage: " << argv[0] << " [-f filename] [-n number] [-h]\n"
                << "Options:\n"
                << "  -h           Display this help message\n"
                << "  -c filename  Specify a file name SimulationConfig.xml\n"
                << "  -d filename  Specify a file name modelDescription.xml\n"
                << "  -t temperature    Specify a numeric value\n";
        return 0;
    }

    /* READ FILES XML */

    if (!filenameModelDescription.empty()) {
        loadVariables(filenameModelDescription);
    } else {
        filenameModelDescription = Configuration::RunLocation + "modelDescription.xml";
        loadVariables(filenameModelDescription);
    }

    if (!filenameSimulationConfig.empty()) {
        sim.setConfigurationFile(filenameSimulationConfig);
    } else {
        filenameSimulationConfig = Configuration::RunLocation + "SimulationConfig.xml";
        sim.setConfigurationFile(Configuration::RunLocation + filenameSimulationConfig);
    }

    DataStore::addVariable("EnvironmentSiteOutdoorAirDrybulbTemperature");

    if (!filenameModelDescription.empty()) {
        DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", temperature);
    } else {
        DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", 21);
    }


    /* AFFICHAGE FILES XML */

    std::cout << ".\n" << "filenameSimulationConfig : " << filenameSimulationConfig << ".\n" << std::endl;
    std::cout << "filenameModelDescription : " << filenameModelDescription << ".\n" << std::endl;
    std::cout << "Temperature value : " << temperature << ".\n" << std::endl;

    if (LOG.getError()) {
        std::cout << "ERROR OK " << "\n" << std::endl;
    }

    /* PREPROCESS : Reads in the configuration file and sends to parser.*/

    sim.preprocess();
    std::cout << "PREPROCESS OK" << "\n" << std::endl;

    // Configuration::info.save = true;
    // Configuration::info.windows = true;
    // Configuration::info.windowsLearn = true;
    // Configuration::info.shading = false;
    // Configuration::info.lights = true;
    // Configuration::info.heating = true;

    // Configuration::info.learnep = 0.8;
    // Configuration::info.learnupdate = true;

    // Exemple d'utilisation
    float min = 18.0f;
    float max = 22.0f;

    // Affichage d'un float aléatoire borné
    std::cout << "Nombre aléatoire entre " << min << " et " << max << ": "
            << randomFloat(min, max) << std::endl;

    /*
    added: EnvironmentSiteExteriorHorizontalSkyIlluminance
    added: EnvironmentSiteRainStatus
    added: EnvironmentSiteOutdoorAirDrybulbTemperature
    added: EMSwarmUpComplete
    added: EMSepTimeStep
    added: MainZoneMeanAirTemperature
    added: MainZoneAirRelativeHumidity
    added: MainZoneMeanRadiantTemperature
    added: MainDaylightingReferencePoint1Illuminance
     */
    int days = Utility::calculateNumberOfDays(Configuration::info.startDay,
                                              Configuration::info.startMonth,
                                              Configuration::info.endDay,
                                              Configuration::info.endMonth);

    std::cout << "NoMASS2.exe => days : " << days << std::endl;

    int totoaltimesteps = days * 24 * Configuration::info.timeStepsPerHour;

    if (totoaltimesteps <= 0) {
        totoaltimesteps = 10;
    }
    std::cout << "NoMASS.exe => totoaltimesteps (days * 24 * Configuration::info.timeStepsPerHour) : " <<
            totoaltimesteps << std::endl;

    DataStore::addValueS("Block1:MasterBedroomZoneAirRelativeHumidity", randomFloat(50.0f, 80.0f));
    DataStore::addValueS("EnvironmentSiteExteriorHorizontalSkyIlluminance", 200);
    DataStore::addValueS("EnvironmentSiteRainStatus", randomBool());
    DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", randomFloat(min, max));

    for (int i = 0; i < 70; i++) {

        // DataStore::addValueS("EMSwarmUpComplete", randomFloat(min, max));
        // DataStore::addValueS("EMSepTimeStep", randomFloat(min, max));
        DataStore::addValueS("Block1:MasterBedroomZoneMeanAirTemperature", randomFloat(18.0f, 22.0f));
        DataStore::addValueS("Block1:MasterBedroomZoneMeanRadiantTemperature", randomFloat(18.0f, 22.0f));
        // DataStore::addValueS("MainDaylightingReferencePoint1Illuminance", randomFloat(18.0f, 22.0f));

        sim.preTimeStep();
        std::cout << "preTimeStep : " << std::endl;

        sim.timeStep();
        std::cout << "timeStep: " << std::endl;

        sim.postTimeStep();
        std::cout << "postTimeStep : " <<  std::endl;

    }

    // // int KitchenAverageGains = DataStore::getValueS("Block2:OfficeZoneMeanRadiantTemperature");
    // // std::cout << "timeStep KitchenAverageGains: " << KitchenAverageGains << std::endl;
    // // DataStore::print();
    //
    // // DataStore::clear();
    // // DataStore::addVariable("Block1:MasterBedroomZoneMeanAirTemperature");
    // // DataStore::addVariable("Block1:MasterBedroomZoneAirRelativeHumidity");
    // // DataStore::addVariable("Block1:MasterBedroomZoneMeanRadiantTemperature");
    // DataStore::addValueS("Block1:MasterBedroomZoneMeanAirTemperature", 21);
    // DataStore::addValueS("Block1:MasterBedroomZoneAirRelativeHumidity", 21);
    // DataStore::addValueS("Block1:MasterBedroomZoneMeanRadiantTemperature", 21);
    // // DataStore::addVariable("EnvironmentSiteOutdoorAirDrybulbTemperature");
    // // DataStore::addVariable("EnvironmentSiteRainStatus");
    // DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", 18);
    // DataStore::addValueS("EnvironmentSiteRainStatus", 0);
    //
    // sim.preTimeStep();
    // sim.timeStep();
    // int WindowState = DataStore::getValueS("Block1:MasterBedroomZoneWindowState");
    // sim.postTimeStep();
    //
    // // DataStore::addVariable("Block1:MasterBedroomZoneMeanAirTemperature");
    // // DataStore::addVariable("Block1:MasterBedroomZoneAirRelativeHumidity");
    // // DataStore::addVariable("Block1:MasterBedroomZoneMeanRadiantTemperature");
    // // DataStore::addVariable("EnvironmentSiteOutdoorAirDrybulbTemperature");
    // // DataStore::addVariable("EnvironmentSiteRainStatus");
    // DataStore::addValueS("Block1:MasterBedroomZoneMeanAirTemperature", 21);
    // DataStore::addValueS("Block1:MasterBedroomZoneAirRelativeHumidity", 21);
    // DataStore::addValueS("Block1:MasterBedroomZoneMeanRadiantTemperature", 21);
    // DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", 18);
    // DataStore::addValueS("EnvironmentSiteRainStatus", 0);
    //
    // for (int i = 1; i < 10; i++) {
    //     sim.preTimeStep();
    //     sim.timeStep();
    //     WindowState = DataStore::getValueS("Block1:MasterBedroomZoneWindowState");
    //     // EXPECT_EQ(WindowState, 0);
    //     sim.postTimeStep();
    // }
    //
    // DataStore::addValueS("Block1:MasterBedroomZoneMeanAirTemperature", 28);
    // DataStore::addValueS("Block1:MasterBedroomZoneAirRelativeHumidity", 100);
    // DataStore::addValueS("Block1:MasterBedroomZoneMeanRadiantTemperature", 100);
    // DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", 15);
    // DataStore::addValueS("EnvironmentSiteRainStatus", 0);
    //
    // // for (i = 100;; i++) {
    // //     sim.preTimeStep();
    // //     sim.timeStep();
    // //     //WindowState = DataStore::getValueS("Block1:MasterBedroomWindowState0");
    // //     // int occs = DataStore::getValueS("Block1:MasterBedroomNumberOfOccupants");
    // //     // if (occs > 0) {
    // //     // EXPECT_EQ(WindowState, 1);
    // //     // break;
    // //     // }
    // //     sim.postTimeStep();
    // // }

    // WRITE RESULTATS
    sim.postprocess();

    return 0;
}
