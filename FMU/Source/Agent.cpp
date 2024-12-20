// Copyright 2016 Jacob Chapman

#include "Agent.hpp"

Agent::Agent() {
}

void Agent::setup() {
}

void Agent::preprocess() {
}

void Agent::step() {
}

void Agent::postprocess() {
}

void Agent::postTimeStep() {
}

void Agent::setBuildingID(const int id) {
    buildingID = id;
}

void Agent::setID(const int id) {
    this->id = id;
}

int Agent::getID() const {
    return id;
}

int Agent::getBuildingID() const {
    return buildingID;
}

void Agent::setIDString(const std::string &idString) {
    this->idString = idString;
}
