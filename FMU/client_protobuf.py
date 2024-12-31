import socket
import message_pb2  # Importer les classes Protobuf générées


def send_protobuf(host, port):
    try:
        # Créer un socket TCP/IP
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Se connecter au serveur
        client_socket.connect((host, port))
        print(f"Connecté au serveur {host}:{port}")

        # Créer des données Protobuf
        sensor_data_list = message_pb2.SensorDataList()

        # Ajouter des données
        sensor1 = sensor_data_list.data.add()
        sensor1.parameter = "Temperature"
        sensor1.value = 25.0
        sensor1.unit = "°C"

        sensor2 = sensor_data_list.data.add()
        sensor2.parameter = "Humidity"
        sensor2.value = 60.0
        sensor2.unit = "%"

        sensor3 = sensor_data_list.data.add()
        sensor3.parameter = "Pressure"
        sensor3.value = 1013.25
        sensor3.unit = "hPa"

        # Sérialiser les données
        serialized_data = sensor_data_list.SerializeToString()

        # Envoyer la longueur des données suivie des données
        client_socket.sendall(len(serialized_data).to_bytes(4, byteorder="big"))
        client_socket.sendall(serialized_data)
        print("Données Protobuf envoyées.")

        # Recevoir la réponse
        response_length = int.from_bytes(client_socket.recv(4), byteorder="big")
        response_data = client_socket.recv(response_length)
        print("Réponse reçue du serveur :", response_data.decode("utf-8"))

        # Fermer la connexion
        client_socket.close()
        print("Connexion fermée.")
    except Exception as e:
        print(f"Erreur : {e}")


# Utiliser le client
if __name__ == "__main__":
    send_protobuf("127.0.0.1", 12345)
