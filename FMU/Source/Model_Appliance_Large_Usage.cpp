// Copyright 2015 Jacob Chapman

#include <cstring>
#include <string>
#include <vector>
#include "Utility.h"
#include "SimulationConfig.h"
#include "Model_Appliance_Large_Usage.h"


Model_Appliance_Large_Usage::Model_Appliance_Large_Usage() {
  state = Utility::randomInt(0.0, 1.0);
}

void Model_Appliance_Large_Usage::setCountry(const std::string & country) {
  this->country = country;
}

void Model_Appliance_Large_Usage::setID(const int id) {
  this->id = id;
}

std::string Model_Appliance_Large_Usage::getCountry() {
  return country;
}

double Model_Appliance_Large_Usage::probOn(int timestep) const {
  double prob = 0.0;
  int leninsec = SimulationConfig::lengthOfTimestep();
  int ten = static_cast<int>(((timestep * leninsec) / 360.0)) % 144;
  prob = onProbabilities[ten];
  return prob;
}

double Model_Appliance_Large_Usage::consumption(const int timeStep) {
  double result = 0.0;
  int leninsec = SimulationConfig::lengthOfTimestep();
  int now = timeStep * leninsec;
  int end = timeStep * leninsec + leninsec;
  while (now < end) {
    int ten = static_cast<int>(now / 600) % 144;
    if (onAt(ten)) {
      result += getPower() / 10;
    }
    now += 60;
  }
  return result;
}

double Model_Appliance_Large_Usage::getTransitionAt(const int state,
                                                              const int i) {
        return  transitions[state][i];
}

double Model_Appliance_Large_Usage::getPower() {
        return getMeanFraction() * maxPower;
}

double Model_Appliance_Large_Usage::getMeanFraction() {
  double random = Utility::randomDouble(0.0, 1.0);
  double currSum = 0;
  for (int i =0; i < 10; i++) {
    currSum = currSum + getTransitionAt(state, i);
    if (random < currSum) {
      state = i;
      break;
    }
  }
  return meanF[state];
}

bool Model_Appliance_Large_Usage::isOn() const {
  return on;
}

bool Model_Appliance_Large_Usage::onAt(const int timeStep) {
  double probability = probOn(timeStep);
  double random = Utility::randomDouble(0.0, 1.0);
  on = random < probability;
  return on;
}

template <typename T>
std::vector<T> Model_Appliance_Large_Usage::as_vector(
                                                  rapidxml::xml_node<> *node) {
    std::vector<T> r;
    std::string value = node->value();

    std::vector<std::string> tokens = Utility::splitCSV(node->value());
    for (std::string& item : tokens) {
        r.push_back(std::stod(item));
    }
    return r;
}

template <typename T>
std::vector<std::vector<T>> Model_Appliance_Large_Usage::as_vector_vector(
  rapidxml::xml_node<> *node) {
    std::vector<std::vector<T>> r;
    rapidxml::xml_node<> *cnode = node->first_node();
    while (cnode) {
      r.push_back(as_vector<T>(cnode));
      cnode = cnode->next_sibling();
    }
    return r;
}

void Model_Appliance_Large_Usage::parseConfiguration(
                                                  const std::string filename) {
  namespace rx = rapidxml;
  rx::file<> xmlFile(filename.c_str());  // Default template is char
  rx::xml_document<> doc;
  doc.parse<0>(xmlFile.data());    // 0 means default parse flags
  rx::xml_node<> *root_node = doc.first_node("Appliances");
  rx::xml_node<> *node = root_node->first_node("Appliance");
  while (node) {
    rx::xml_node<> *cnode = node->first_node();
    bool found = false;
    while (cnode) {
      if (std::strcmp(cnode->name(), "id") == 0) {
          if (id == std::stoi(cnode->value())) {
            found = true;
          }
      }
      cnode = cnode->next_sibling();
    }
    if (found) {
      cnode = node->first_node();
      while (cnode) {
        if (std::strcmp(cnode->name(), "country") == 0) {
            country = cnode->value();
        } else if (std::strcmp(cnode->name(), "name") == 0) {
            name = cnode->value();
        } else if (std::strcmp(cnode->name(), "maxPower") == 0) {
            maxPower = std::stod(cnode->value());
        } else if (std::strcmp(cnode->name(), "meanF") == 0) {
            meanF = as_vector<double>(cnode);
        } else if (std::strcmp(cnode->name(), "transitions") == 0) {
            transitions = as_vector_vector<double>(cnode);
        } else if (std::strcmp(cnode->name(), "on") == 0) {
            onProbabilities = as_vector<double>(cnode);
        } else {
          parseShapeScale(cnode);
        }
        cnode = cnode->next_sibling();
      }
      break;
    }
    node = node->next_sibling();
  }
}

void Model_Appliance_Large_Usage::parseShapeScale(rapidxml::xml_node<> *node) {
  node->value();
}
