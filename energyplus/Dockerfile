# Utilisez une image de base avec un environnement de compilation
#FROM ubuntu:latest
#FROM debian:12-slim
FROM arm64v8/debian:latest
# Définit la plateforme si nécessaire (AMD64)
#ARG TARGETPLATFORM=linux/amd64

# Définir les variables d'environnement pour éviter les invites interactives
ENV DEBIAN_FRONTEND=noninteractive

# Mettre à jour et installer les dépendances nécessaires
RUN apt-get update && apt-get install -y \
    build-essential \
    software-properties-common \
    cmake \
    gfortran \
    git \
    python3-dev \
    python3-pip \
    libboost-all-dev \
    libcurl4-openssl-dev \
    libssl-dev \
    libffi-dev \
    zlib1g-dev \
    wget \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

ENV CXX=g++
ENV CC=gcc
ENV CFLAGS="-fPIC"

## Installer CMake si une version plus récente est nécessaire
#RUN pip3 install cmake
RUN wget https://www.python.org/ftp/python/3.12.0/Python-3.12.0.tgz \
    && tar -xf Python-3.12.0.tgz \
    && cd Python-3.12.0 \
    && ./configure --enable-optimizations \
    && make -j$(nproc) \
    && make install

# Définir le répertoire de travail
WORKDIR /energyplus

# Cloner le dépôt EnergyPlus (remplacez par l'URL et la branche appropriées)
RUN git clone --recurse-submodules https://github.com/NREL/EnergyPlus.git .

ENV CXXFLAGS="-std=c++17 -fPIC"

# Configurer la compilation avec CMake
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_CXX_STANDARD=17  -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_FORTRAN=ON DENABLE_GTEST_DEBUG_MODE:BOOL=OFF \
     -DLINK_WITH_PYTHON=ON -DPYTHON_CLI:BOOL=OFF \
    && make -j$(nproc)

# Ajouter EnergyPlus au PATH
ENV PATH="/energyplus/build/Products:${PATH}"

# Commande par défaut (affiche l'aide d'EnergyPlus pour vérifier l'installation)
#CMD ["/energyplus/build/Products/energyplus", "--help"]
CMD ["/bin/bash"]