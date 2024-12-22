// Copyright 2024 Boris Brangeon

#include <iterator>
#include <vector>
#include <string>
#include <cstring>
#include <cstddef>
#include <unistd.h> // Pour getopt

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
    DataStore::clear();
    DataStore::clearValues();
    Configuration::setStepCount(-1);

    std::string filenameModelDescription;
    std::string filenameSimulationConfig;

    float temperature = 0.0;
    bool help = false;

    int opt;

    // Utiliser getopt pour analyser les options
    while ((opt = getopt(argc, argv, "hc:d:t:")) != -1) {
        switch (opt) {
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

    // Afficher les résultats
    if (!filenameModelDescription.empty()) {
        std::cout << "filenameModelDescription: " << filenameModelDescription << std::endl;
        loadVariables(Configuration::RunLocation + filenameModelDescription);
    } else {
        std::cout << "filenameModelDescription not provided.\n";
    }

    // Afficher les résultats
    if (!filenameSimulationConfig.empty()) {
        std::cout << "filenameSimulationConfig: " << filenameSimulationConfig << "\n" << std::endl;
        sim.setConfigurationFile(Configuration::RunLocation + filenameSimulationConfig);
    } else {
        std::cout << "filenameSimulationConfig not provided.\n";
    }

    std::cout << "Temperature value: " << temperature <<   ".\n"<<std::endl;

    // if (argc > 1) {
    //     sim.setConfigurationFile(argv[1]);
    //     std::string filenameModelDescription = argv[2];
    // } else {
    //     std::string filenameSimulationConfig =
    //             Configuration::RunLocation + "SimulationConfig.xml";
    //
    //     sim.setConfigurationFile(filenameSimulationConfig);
    //     std::string filenameModelDescription = Configuration::RunLocation + "modelDescription.xml";
    //
    //     loadVariables(filenameModelDescription);;
    // }

    if (LOG.getError()) {
        std::cout << "ERROR OK: " << std::endl;
    }

    std::cout << "setConfigurationFile OK: " << std::endl;
    sim.preprocess();
    std::cout << "Preprocess OK: " << std::endl;


    std::cout << "Configuration OK: " << std::endl;
    // Configuration::info.save = true;
    // Configuration::info.windows = true;
    // Configuration::info.windowsLearn = true;
    // Configuration::info.shading = false;
    // Configuration::info.lights = true;
    // Configuration::info.heating = true;

    // Configuration::info.learnep = 0.8;
    // Configuration::info.learnupdate = true;


    std::cout << "Info OK: " << std::endl;

    // DataStore::addVariable("EnvironmentSiteOutdoorAirDrybulbTemperature");
    DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", temperature);

    std::cout << "DataStore OK: " << std::endl;

    for (int i = 0; i < 10; i++) {
        sim.preTimeStep();
        sim.timeStep();
        sim.postTimeStep();
    }

    // int KitchenAverageGains = DataStore::getValueS("Block2:OfficeZoneMeanRadiantTemperature");
    // std::cout << "timeStep KitchenAverageGains: " << KitchenAverageGains << std::endl;
    // DataStore::print();

    // WRITE RESULTATS
    sim.postprocess();

    return 0;
}
