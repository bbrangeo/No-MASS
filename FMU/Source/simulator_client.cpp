//
// Created by Boris on 23/12/2024.
//

#include <iostream>
#include <string>
#include <asio.hpp>
#include <random>


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

int main() {
    try {
        asio::io_context io_context;

        // Se connecter au serveur
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), 12345));
        std::cout << "Connecté au serveur..." << std::endl;

        // Envoyer une donnée au serveur
        float input_value = randomFloat(18.0f, 22.0f);

        std::string message = std::to_string(input_value) + "\n";
        asio::write(socket, asio::buffer(message));
        std::cout << "Envoyé au serveur EnvironmentSiteOutdoorAirDrybulbTemperature: " << input_value << "°C" <<std::endl;

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