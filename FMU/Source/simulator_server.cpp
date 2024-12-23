//
// Created by Boris on 23/12/2024.
//

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <asio.hpp> // Bibliothèque pour gérer les sockets (utilisez `apt install libasio-dev` ou ajoutez asio à votre projet)



using asio::ip::tcp;



#include <cstring>
#include <cstddef>
#include <unistd.h> // Pour getopt
#include <random>


#include "rapidxml_utils.hpp"
#include "rapidxml.hpp"
#include "Utility.hpp"
#include "Simulation.hpp"
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

void handle_client(tcp::socket socket) {
    try {
        asio::streambuf buffer;
        asio::read_until(socket, buffer, "\n");
        std::string input = asio::buffer_cast<const char *>(buffer.data());
        std::cout << "Reçu du client : " << input << std::endl;

        // Simulation : multiplier l'entrée par 2
        double value = std::stod(input);


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
        DataStore::addValueS("EnvironmentSiteOutdoorAirDrybulbTemperature", value);


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
        std::cout << "postTimeStep : " << std::endl;

        // WRITE RESULTATS
        // sim.postprocess();

        double result = value * 2;
        int AverageGains = DataStore::getValueS("Block1:MasterBedroomAverageGains");

        // Envoyer la réponse au client
        std::string response = std::to_string(AverageGains) + "\n";
        asio::write(socket, asio::buffer(response));
    } catch (std::exception &e) {
        std::cerr << "Erreur lors du traitement du client : " << e.what() << std::endl;
    }
}

int main(int argc, char *argv[]) {
    try {
        asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "Serveur en attente sur le port 12345..." << std::endl;

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

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::thread(handle_client, std::move(socket)).detach();
        }
    } catch (std::exception &e) {
        std::cerr << "Erreur du serveur : " << e.what() << std::endl;
    }

    return 0;
}
