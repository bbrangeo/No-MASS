# Utilisez une image de base avec un environnement de compilation
#FROM ubuntu:latest
FROM debian:12-slim
# Définit la plateforme si nécessaire (AMD64)
ARG TARGETPLATFORM=linux/amd64

# Définir les variables d'environnement pour éviter les invites interactives
ENV DEBIAN_FRONTEND=noninteractive

# Mettre à jour et installer les dépendances nécessaires
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gfortran \
    git \
    python3 \
    python3-pip \
    python3-dev \
    libboost-all-dev \
    libcurl4-openssl-dev \
    libssl-dev \
    zlib1g-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

## Installer CMake si une version plus récente est nécessaire
#RUN pip3 install cmake

# Définir le répertoire de travail
WORKDIR /energyplus

# Cloner le dépôt EnergyPlus (remplacez par l'URL et la branche appropriées)
RUN git clone --recurse-submodules https://github.com/NREL/EnergyPlus.git .

# Configurer la compilation avec CMake
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_CXX_STANDARD=11 -DCMAKE_BUILD_TYPE=DEBUG -DBUILD_FORTRAN=ON && \
    make -j$(nproc)

# Ajouter EnergyPlus au PATH
ENV PATH="/energyplus/build/Products:${PATH}"

# Commande par défaut (affiche l'aide d'EnergyPlus pour vérifier l'installation)
#CMD ["/energyplus/build/Products/energyplus", "--help"]
CMD ["/bin/bash"]