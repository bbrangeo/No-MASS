//
// Created by Boris on 23/12/2024.
//

#include <iostream>
#include <string>
#include <asio.hpp>
#include <random>
#include <nlohmann/json.hpp> // Inclure la bibliothèque JSON


using asio::ip::tcp;

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

void sendMultipleJson(asio::ip::tcp::socket& socket, double input_value) {
    try {
        // Créer un tableau JSON
        nlohmann::json json_array = nlohmann::json::array();

        // Ajouter plusieurs objets JSON au tableau
        json_array.push_back({
            {"parameter", "EnvironmentSiteOutdoorAirDrybulbTemperature"},
            {"value", input_value},
            {"unit", "°C"}
        });

        json_array.push_back({
            {"parameter", "EnvironmentSiteOutdoorAirDrybulbTemperature"},
            {"value", input_value},
            {"unit", "°C"}
        });

        // Sérialiser en une chaîne de caractères
        std::string serialized_message = json_array.dump() + "\n"; // Ajoutez "\n" pour marquer la fin du message

        // Envoyer le message JSON
        asio::write(socket, asio::buffer(serialized_message));

        std::cout << "Envoyé au serveur : " << serialized_message << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'envoi du JSON : " << e.what() << std::endl;
    }
}


int main() {
    try {
        asio::io_context io_context;

        // Se connecter au serveur
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), 12345));
        std::cout << "Connxecté au serveur..." << std::endl;


        // Envoyer une donnée au serveur
        float input_value = randomFloat(18.0f, 22.0f);

        sendMultipleJson(socket, input_value);

        // Recevoir la réponse du serveur
        asio::streambuf buffer;
        asio::read_until(socket, buffer, "\n");
        std::string response = asio::buffer_cast<const char*>(buffer.data());
        std::cout << "Réponse du serveur AverageGains : " << response << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Erreur du client : " << e.what() << std::endl;
    }

    return 0;
}