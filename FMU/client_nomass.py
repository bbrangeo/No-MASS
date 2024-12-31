import json
import socket
import random

# Utiliser le client pour envoyer des JSON
if __name__ == "__main__":

    # Adresse et port du serveur
    SERVER_HOST = "127.0.0.1"  # Adresse IP du serveur
    SERVER_PORT = 12345  # Port du serveur

    """
    added: EnvironmentSiteExteriorHorizontalSkyIlluminance
    added: EnvironmentSiteRainStatus
    added: EnvironmentSiteOutdoorAirDrybulbTemperature
    added: EMSwarmUpComplete
    added: EMSepTimeStep
    added: MainZoneMeanAirTemperature
    added: MainZoneAirRelativeHumidity
    added: MainZoneMeanRadiantTemperature
    added: MainDaylightingReferencePoint1Illuminance
    """

    # Construire un tableau de JSON
    json_data = [
        {
            "parameter": "EnvironmentSiteOutdoorAirDrybulbTemperature",
            "value": random.uniform(18.0, 35.0),
            "unit": "°C",
        },
        {
            "parameter": "EnvironmentSiteRainStatus",
            "value": random.choice([0, 1]),
            "unit": "°C",
        },
        {
            "parameter": "Block1:MasterBedroomZoneMeanAirTemperature",
            "value": random.uniform(18.0, 35.0),
            "unit": "°C",
        },
        {
            "parameter": "Block1:MasterBedroomZoneAirRelativeHumidity",
            "value": random.uniform(50.0, 80.0),
            "unit": "%",
        },
    ]

    try:
        # Créer un socket TCP/IP
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Se connecter au serveur
        client_socket.connect((SERVER_HOST, SERVER_PORT))
        print(f"Connecté au serveur {SERVER_HOST}:{SERVER_PORT}")

        # Sérialiser les données en une chaîne JSON
        serialized_data = (
            json.dumps(json_data) + "\n"
        )  # Ajouter "\n" pour signaler la fin du message

        # Envoyer les données au serveur
        client_socket.sendall(serialized_data.encode("utf-8"))
        print("JSON envoyé au serveur :")
        print(serialized_data)

        # Lire la réponse du serveur
        response = client_socket.recv(1024).decode(
            "utf-8"
        )  # Taille maximale de la réponse
        print("Réponse reçue du serveur :")
        print(response)

        # Lire la réponse du serveur
        response2 = client_socket.recv(2048).decode(
            "utf-8"
        )  # Taille maximale de la réponse
        print("Réponse2 reçue du serveur :")
        print(response2)

        # Fermer la connexion
        client_socket.close()
        print("Connexion fermée.")
    except Exception as e:
        print(f"Erreur : {e}")
