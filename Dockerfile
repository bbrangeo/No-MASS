# Étape 1 : Utiliser une image de base
FROM ubuntu:latest AS base

# Copier uniquement le répertoire /energyplus/build/Products de l'image source
FROM energyplus:latest AS energyplus_image

# Préparer la nouvelle image
FROM ubuntu:latest

# Étape 2 : Installer les outils système et Python 3.10
RUN apt-get update && apt-get install -y \
    software-properties-common \
    python3.12 \
    python3.12-venv \
    python3-pip \
    build-essential \
    libssl-dev \
    libffi-dev \
    python3-dev \
    cmake\
    build-essential \
    && apt-get clean

# Étape 3 : Créer un environnement virtuel
RUN python3 -m venv /app/venv

# Étape 4 : Activer l'environnement virtuel et installer JupyterLab
RUN /app/venv/bin/pip install --no-cache-dir jupyterlab tables pandas numpy matplotlib seaborn

# Étape 5 : Copier les fichiers nécessaires
COPY ./FMU /app/FMU
COPY ./rapidxml /app/rapidxml

# Étape 6 : Créer le répertoire de build
RUN mkdir /app/build

ENV CXX=g++
ENV CXXFLAGS="-std=c++11"

# Étape 7 : Compiler les fichiers FMU
RUN cd /app/build; cmake  ../FMU -DTests=on -DCMAKE_CXX_STANDARD=11 -DCMAKE_BUILD_TYPE=Debug; make -j$(nproc)

# Étape 9 : Ajouter l'environnement virtuel au PATH
ENV PATH="/app/venv/bin:${PATH}"

# Créer un dossier cible pour les fichiers copiés
WORKDIR /energyplus/Products

# Copier les fichiers depuis l'image source
COPY --from=energyplus_image /energyplus/build/Products /energyplus/Products

# Étape 8 : Définir le répertoire de travail
WORKDIR /app/build

# Ajouter les exécutables au PATH
ENV PATH="/energyplus/Products:${PATH}"
ENV LD_LIBRARY_PATH="/energyplus/build/Products:${LD_LIBRARY_PATH}"
RUN echo "/energyplus/build/Products" > /etc/ld.so.conf.d/energyplus.conf && ldconfig

# Étape 11 : Laisser CMD vide pour flexibilité
CMD ["/bin/bash"]

