// Copyright 2015 Jacob Chapman

#include <vector>
#include <iostream>
#include <fstream>
#include <utility>
#include <iomanip>
#include <string>
#include <regex>

#include "Log.hpp"
#include "Configuration.hpp"
#include "DataStore.hpp"

using json = nlohmann::json;
std::unordered_map<std::string, int> DataStore::variableMap;
std::vector<std::vector<float> > DataStore::intMap = {};

int DataStore::variableCount = 0;

DataStore::DataStore() {
}

int DataStore::getID(const std::string &name) {
  // std::cout << "getID: " << name << std::endl;
  return variableMap.at(name);
}

int DataStore::addVariable(const std::string &name) {
  int ret = -1;
  if (name != "") {
    for (std::string reg: Configuration::outputRegexs) {
      std::regex rgx(reg);
      std::smatch match;
      if (std::regex_match(name, match, rgx)) {
        if (variableMap.find(name) == variableMap.end()) {
          // std::cout << "addVariable: " << ret << ": " << name << std::endl;
          ret = DataStore::variableCount;
          // std::cout << "addVariable: " << ret << ": " << name << std::endl;
          variableMap.insert(std::make_pair(name, ret));
          intMap.push_back(std::vector<float>());
          DataStore::variableCount++;
        } else {
          ret = getID(name);
        }
        break;
      }
    }
  }
  return ret;
}

void DataStore::addValueS(const std::string &name, const float value) {
  if (name != "") {
    // std::cout << "addValueS: " << name << ": " << value << std::endl;
    // Vérifie si la clé existe dans variableMap
    if (variableMap.count(name) > 0) {
      int val = variableMap.at(name);
      // std::cout << "addValueS: " << val;
      // std::cout << ": " << name << ": " << value << std::endl;
      addValue(val, value);
    } else {
      std::cerr << "Error: Key '" << name << "' not found in variableMap" << std::endl;
    }
  }
}

void DataStore::addValue(const int &id, const float val) {
  if (id > -1) {
    intMap.at(id).push_back(val);
  }
}

json DataStore::getJSONVariables() {
  json result;
  try {
    for (const auto &pair: variableMap) {
      std::string name = pair.first;

      // Vérifier si la clé existe
      if (variableMap.find(name) == variableMap.end()) {
        std::cerr << "Clé non trouvée : " << name << std::endl;
        continue; // Passer à la prochaine clé
      }
      try {
        // int id = variableMap.at(name);
        // std::cout << "get: " << name << std::endl;
        // Afficher le type de la variable
        // std::cout << "Type de 'value' : " << typeid(getValueS(name)).name() << std::endl;
        // std::cout << "get: " << getValueS(name) << std::endl;

        // Récupération de la valeur float
        // const float value = getValue(id);
        // std::cout << "get: " << value << std::endl;
        float value = getValueS(name);

        result[name] = value;
      } catch (const std::exception &e) {
        std::cerr << "Erreur générale : " << e.what() << std::endl;
      }
    }
  } catch (const std::exception &e) {
    // Gestion générique des autres exceptions
    std::cerr << "Erreur générale : " << e.what() << std::endl;
  }

  return result;
}


float DataStore::getValueForZone(const std::string &name,
                                 const std::string &zoneName) {
  return getValueS(zoneName + name);
}

float DataStore::getValueS(const std::string &name) {
  std::cout << "getValueS: " << name << std::endl;
  if (variableMap.find(name) == variableMap.end()) {
    LOG << "Cannot find the variable: " << name;
    LOG << "\nThis could happen for a number of reasons:\n";
    LOG << " - Check the Zone Name is correct in ";
    LOG << "the NoMass simulation configuration file\n";
    LOG << " - Check that all variable are defined ";
    LOG << "in the model description file\n";
    LOG.error();
    exit(-1);
  }
  try {
    int id = variableMap.at(name);
    // Vérification de la validité de l'ID
    if (id < 0) {
      LOG << "Invalid ID: " << id;
      LOG.error();
      exit(-1);
    }

    // Appel sécurisé à getValue
    try {
      float value = getValue(id);

      return value;
    } catch (const std::exception &e) {
      LOG << "Erreur DataStore::getValueS: Exception during getValue: " << e.what();
      LOG.error();
      exit(-1);
    }
  } catch (const std::exception &e) {
    std::cerr << "Erreur DataStore::getValueS: Erreur générale : " << e.what() << std::endl;
    exit(-1);
  }
}

float DataStore::getValue(const int &id) {
  try {
    // Vérification si l'ID existe
    if (id < 0 || id >= static_cast<int>(intMap.size())) {
      std::cerr << "Erreur DataStore::getValue: ID invalide ou hors des limites : " << id <<  std::endl;
      exit(-1);
    }

    // Vérification si le vecteur est vide
    if (intMap[id].empty()) {
      std::cerr << "Erreur DataStore::getValue: vecteur vide pour l'ID : " << id << std::endl;
      exit(-1);
    }

    float ret = intMap.at(id).back();

    return ret;
  } catch (const std::exception &e) {
    std::cerr << "Erreur générale : " << e.what() << std::endl;
    exit(-1);
  }
}

void DataStore::clearValues() {
  std::cout << "clear values" << std::endl;
  std::vector<std::vector<float> >::iterator it;
  for (it = intMap.begin(); it != intMap.end(); ++it) {
    it->clear();
  }
}

void DataStore::clear() {
  std::cout << "DataStore clear" << std::endl;
  variableMap.clear();
  intMap.clear();
  DataStore::variableCount = 0;
}

void DataStore::print() {
  if (Configuration::info.save) {
    // std::cout << "print " << std::endl;
    std::ofstream myfile;
    myfile.open("NoMASS.out");
    myfile << std::fixed << std::setprecision(Configuration::info.precision);
    myfile << "stepCount";
    std::vector<int> ids;
    unsigned int maxSize = 0;
    std::unordered_map<std::string, int>::const_iterator it;
    for (it = variableMap.cbegin(); it != variableMap.cend(); ++it) {
      myfile << "," << it->first;
      int val = it->second;
      ids.push_back(val);
      if (maxSize < intMap.at(val).size()) {
        maxSize = intMap.at(val).size();
      }
    }
    myfile << "\n";

    for (unsigned int i = 0; i < maxSize; i++) {
      myfile << i;
      for (unsigned int j: ids) {
        myfile << ",";
        if (intMap.at(j).size() > i) {
          myfile << intMap.at(j).at(i);
        }
      }
      myfile << "\n";
    }
    myfile.close();
  }

  std::ofstream myfileOreni;
  myfileOreni.open("OreniNoMASS.csv");
  myfileOreni << std::fixed << std::setprecision(Configuration::info.precision);
  myfileOreni << "stepCount";
  std::vector<int> ids;
  unsigned int maxSize = 0;
  std::unordered_map<std::string, int>::const_iterator it;
  for (it = variableMap.cbegin(); it != variableMap.cend(); ++it) {
    myfileOreni << "," << it->first;
    int val = it->second;
    ids.push_back(val);
    if (maxSize < intMap.at(val).size()) {
      maxSize = intMap.at(val).size();
    }
  }
  myfileOreni << "\n";

  for (unsigned int i = 0; i < maxSize; i++) {
    myfileOreni << i;
    for (unsigned int j: ids) {
      myfileOreni << ",";
      if (intMap.at(j).size() > i) {
        myfileOreni << intMap.at(j).at(i);
      }
    }
    myfileOreni << "\n";
  }
  myfileOreni.close();
}
