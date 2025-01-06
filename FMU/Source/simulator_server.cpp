//
// Created by Boris on 23/12/2024.
//

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <cstring>
#include <cstddef>
#include <unistd.h> // Pour getopt
#include <random>
#include <asio.hpp> // Bibliothèque pour gérer les sockets (utilisez `apt install libasio-dev` ou ajoutez asio à votre projet)


#include <nlohmann/json.hpp> // Inclure la bibliothèque JSON

#include "rapidxml_utils.hpp"
#include "rapidxml.hpp"
#include "Utility.hpp"
#include "Simulation.hpp"
#include "DataStore.hpp"
#include "Configuration.hpp"
#include "Log.hpp"

using json = nlohmann::json;

using asio::ip::tcp;


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
    // Exemple d'utilisation
    float min = 18.0f;
    float max = 22.0f;

    // try {
    // Lire les données
    asio::streambuf buffer;
    asio::read_until(socket, buffer, "\n"); // Lire jusqu'à "\n"
    // Convertir le buffer en chaîne
    std::string input = asio::buffer_cast<const char *>(buffer.data());
    std::cout << "Reçu du client : " << input << std::endl;

    // Tenter de parser la chaîne en JSON
    nlohmann::json json_data;
    try {
        json_data = nlohmann::json::parse(input);
    } catch (const nlohmann::json::exception &e) {
        std::cerr << "Erreur de parsing du JSON : " << e.what() << std::endl;
        return;
    }

    // Vérifier si c'est un tableau JSON
    if (!json_data.is_array()) {
        std::cerr << "Erreur : Données reçues ne sont pas un tableau JSON !" << std::endl;
        return;
    }

    // Parcourir le tableau JSON
    for (const auto &json_entry: json_data) {
        // Vérifier si l'entrée est un objet JSON
        if (!json_entry.is_object()) {
            std::cerr << "Erreur : Entrée du tableau n'est pas un objet JSON !" << std::endl;
            continue;
        }

        // Accéder aux données en utilisant `value()` pour éviter les erreurs
        std::string parameter = json_entry.value("parameter", "Inconnu");
        std::string unit = json_entry.value("unit", "Inconnu");

        double value = json_entry.value("value", 0.0);
        DataStore::addValueS(parameter, value);

        // DataStore::addValueS(parameter, randomFloat(min, max));
        // std::cout << "Paramètre : " << parameter << std::endl;
        // std::cout << "Valeur : " << value << "\n" << unit << std::endl;
    }

    // Affichage d'un float aléatoire borné
    // std::cout << "Nombre aléatoire entre " << min << " et " << max << ": "
    //         << randomFloat(min, max) << std::endl;

    // DataStore::addValueS("Zone1:MasterBedroomZoneAirRelativeHumidity", randomFloat(50.0f, 80.0f));
    // DataStore::addValueS("EnvironmentSiteExteriorHorizontalSkyIlluminance", 200);
    // DataStore::addValueS("EnvironmentSiteRainStatus", randomBool());
    // DataStore::addValueS("EMSwarmUpComplete", randomFloat(min, max));
    // DataStore::addValueS("EMSepTimeStep", randomFloat(min, max));
    // DataStore::addValueS("Zone1:MasterBedroomZoneMeanAirTemperature", randomFloat(18.0f, 22.0f));
    // DataStore::addValueS("Zone1:MasterBedroomZoneMeanRadiantTemperature", randomFloat(18.0f, 22.0f));
    // DataStore::addValueS("MainDaylightingReferencePoint1Illuminance", randomFloat(18.0f, 22.0f));
    // Configuration::info.windows = true;
    // Configuration::info.windows = false;
    // Configuration::info.shading = false;
    // Configuration::info.lights = false;

    sim.preTimeStep();
    std::cout << "SIMU preTimeStep" << std::endl;

    sim.timeStep();
    std::cout << "SIMU timeStep" << std::endl;

    sim.postTimeStep();
    std::cout << "SIMU postTimeStep " << std::endl;

    int AverageGains = DataStore::getValueS("Zone1:MasterBedroomAverageGains");

    // Envoyer la réponse au client
    std::string response = std::to_string(AverageGains) + "\n";
    asio::write(socket, asio::buffer(response));

    json result = DataStore::getJSONVariables();
    std::cout << result.dump(4) << std::endl; // JSON pretty print with 4 spaces

    std::string response2 = result.dump(4) + "\n";
    asio::write(socket, asio::buffer(response2));

    // for (const std::string& name : names) {
    //     std::cout << name << std::endl;
    //     std::string response2 = name + "\n";
    //     asio::write(socket, asio::buffer(response2));
    //
    // }
    // } catch (std::exception &e) {
    //     std::cerr << "Erreur lors du traitement du client : " << e.what() << std::endl;
    // }
}

int main(int argc, char *argv[]) {
    try {
        asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "Serveur en attente sur le port 12345..." << std::endl;

        /* RESET */

        DataStore::clear();
        DataStore::clearValues();

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
            sim.setConfigurationFile(filenameSimulationConfig);
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
        // sim.setupSimulationModel();
        Configuration::setStepCount(0);

        std::cout << "PREPROCESS OK" << "\n" << std::endl;


        int days = Utility::calculateNumberOfDays(Configuration::info.startDay,
                                                  Configuration::info.startMonth,
                                                  Configuration::info.endDay,
                                                  Configuration::info.endMonth);

        std::cout << "NoMASS2Server.exe => calculateNumberOfDays : " << days << std::endl;

        int totoaltimesteps = days * 24 * Configuration::info.timeStepsPerHour;

        if (totoaltimesteps <= 0) {
            totoaltimesteps = 10;
        }
        std::cout << "NoMASS2Server.exe => totoaltimesteps (days * 24 * Configuration::info.timeStepsPerHour) : " <<
                totoaltimesteps << std::endl;

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::thread(handle_client, std::move(socket)).detach();
        }
    } catch (std::exception &e) {
        std::cerr << "Erreur du serveur : " << e.what() << std::endl;
    }

    // WRITE RESULTATS
    sim.postprocess();
    return 0;
}
