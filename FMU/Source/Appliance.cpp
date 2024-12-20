// Copyright 2016 Jacob Chapman

#include <string>
#include <vector>
#include "Utility.hpp"
#include "DataStore.hpp"
#include "Configuration.hpp"
#include "Appliance.hpp"

Appliance::Appliance() {
}

double Appliance::getPriority() const {
  int timeStep = Configuration::getStepCount();
  int leninsec = Configuration::lengthOfTimestep();
  int numberOfSeconds = timeStep * leninsec;
  int hour = numberOfSeconds / 3600;
  int hourOfDay = hour % 24;
  return hourlyPriority[hourOfDay];
}

void Appliance::setReceived(const double r) {
  parameters.received = r;
}

void Appliance::setReceivedCost(const double c) {
  parameters.receivedCost = c;
}

void Appliance::setupSave() {
  datastoreIDSupplied = DataStore::addVariable(idString + "_supplied");
  datastoreIDSuppliedCost = DataStore::addVariable(idString + "_suppliedCost");
  datastoreIDReceived = DataStore::addVariable(idString + "_received");
  datastoreIDRequested = DataStore::addVariable(idString + "_requested");
  datastoreIDCost = DataStore::addVariable(idString + "_cost");

  datastoreLocalIDSupplied = DataStore::addVariable(idString + "_supplied_local");
  datastoreLocalIDReceived = DataStore::addVariable(idString + "_received_local");
  datastoreLocalIDRequested = DataStore::addVariable(idString + "_requested_local");
  datastoreLocalIDCost = DataStore::addVariable(idString + "_cost_local");

  datastoreNeighbourhoodIDSupplied = DataStore::addVariable(idString + "_supplied_neighbourhood");
  datastoreNeighbourhoodIDReceived = DataStore::addVariable(idString + "_received_neighbourhood");
  datastoreNeighbourhoodIDRequested = DataStore::addVariable(idString + "_requested_neighbourhood");
  datastoreNeighbourhoodIDCost = DataStore::addVariable(idString + "_cost_neighbourhood");

  datastoreGridIDSupplied = DataStore::addVariable(idString + "_supplied_grid");
  datastoreGridIDReceived = DataStore::addVariable(idString + "_received_grid");
  datastoreGridIDRequested = DataStore::addVariable(idString + "_requested_grid");
  datastoreGridIDCost = DataStore::addVariable(idString + "_cost_grid");
}

void Appliance::save() {
  DataStore::addValue(datastoreIDSupplied, parameters.supply);
  DataStore::addValue(datastoreIDReceived, parameters.received);
  DataStore::addValue(datastoreIDRequested, parameters.power);
  DataStore::addValue(datastoreIDCost, parameters.receivedCost);
}

void Appliance::saveLocalCalculate() {
  parametersLocal.supply = parameters.supply - parameters.suppliedLeft;
  parametersLocal.suppliedLeft = parameters.suppliedLeft;
  parametersLocal.received = parameters.received;
  parametersLocal.power = parameters.power;
  parametersLocal.receivedCost = parameters.receivedCost;
}

void Appliance::saveLocal() {
  saveLocalCalculate();
  DataStore::addValue(datastoreLocalIDSupplied, parametersLocal.supply);
  DataStore::addValue(datastoreLocalIDReceived, parametersLocal.received);
  DataStore::addValue(datastoreLocalIDRequested, parametersLocal.power);
  DataStore::addValue(datastoreLocalIDCost, parametersLocal.receivedCost);
}

void Appliance::saveNeighbourhoodCalculate() {
  parametersNeighbourhood.supply = parametersLocal.suppliedLeft - parameters.suppliedLeft;
  parametersNeighbourhood.suppliedLeft = parameters.suppliedLeft;
  parametersNeighbourhood.received = parameters.received - parametersLocal.received;
  parametersNeighbourhood.power = parameters.power - parametersLocal.received;
  parametersNeighbourhood.receivedCost = parameters.receivedCost - parametersLocal.receivedCost;
}

void Appliance::saveNeighbourhood() {
  saveNeighbourhoodCalculate();
  DataStore::addValue(datastoreNeighbourhoodIDSupplied, parametersNeighbourhood.supply);
  DataStore::addValue(datastoreNeighbourhoodIDReceived, parametersNeighbourhood.received);
  DataStore::addValue(datastoreNeighbourhoodIDRequested, parametersNeighbourhood.power);
  DataStore::addValue(datastoreNeighbourhoodIDCost, parametersNeighbourhood.receivedCost);
}

void Appliance::saveGlobalCalculate() {
  parametersGrid.supply = parametersNeighbourhood.suppliedLeft - parameters.suppliedLeft;
  parametersGrid.suppliedLeft = parameters.suppliedLeft;
  parametersGrid.received = parameters.received - parametersNeighbourhood.received - parametersLocal.received;
  parametersGrid.power = parameters.power - parametersNeighbourhood.received - parametersLocal.received;
  parametersGrid.receivedCost = parameters.receivedCost - parametersNeighbourhood.receivedCost - parametersLocal.
                                receivedCost;
}

void Appliance::saveGlobal() {
  saveGlobalCalculate();
  DataStore::addValue(datastoreGridIDSupplied, parametersGrid.supply);
  DataStore::addValue(datastoreGridIDReceived, parametersGrid.received);
  DataStore::addValue(datastoreGridIDRequested, parametersGrid.power);
  DataStore::addValue(datastoreGridIDCost, parametersGrid.receivedCost);
}

void Appliance::beforeClear() {
}

void Appliance::clear() {
  beforeClear();
  local = false;
  global = false;
  parameters.power = 0.0;
  parameters.supply = 0.0;
  parameters.supplyCost = 0.0;
  parameters.received = 0.0;
  parameters.receivedCost = 0.0;
  parameters.suppliedLeft = 0.0;
}

bool Appliance::isLocal() const {
  return local;
}

void Appliance::setLocal(bool local) {
  this->local = local;
}

bool Appliance::isGlobal() const {
  return global;
}

void Appliance::setGlobal(bool global) {
  this->global = global;
}

void Appliance::setPower(const double power) {
  this->parameters.power = power;
}

void Appliance::setSupply(const double supply) {
  this->parameters.supply = supply;
}

void Appliance::setSupplyCost(const double supplyCost) {
  this->parameters.supplyCost = supplyCost;
}

void Appliance::setHourlyCost(const std::vector<double> &cost) {
  hourlyCost = cost;
}

void Appliance::setHoulyPriority(const std::vector<double> &priority) {
  this->hourlyPriority = priority;
}

void Appliance::setSupplyLeft(const double supplyLeft) {
  this->parameters.suppliedLeft = supplyLeft;
}

int Appliance::calculateHourOfDay() const {
  int stepCount = Configuration::getStepCount();
  int hour = (stepCount * Configuration::lengthOfTimestep()) / 3600;
  return hour % 24;
}

/**
 * @brief Check the activities the appliance can be turned on during.
 * @return If there is a corresponding activity to turn the appliance on in.
 * @details For some appliances we may wish that they only turn on for some
 * applainces, here we check if the activities match.
 */
bool Appliance::hasActivities(const std::vector<int> &Activities) {
  match = this->Activities.empty();
  for (int i: this->Activities) {
    for (int j: Activities) {
      if (i == j) {
        match = true;
        break;
      }
    }
  }
  return match;
}

/**
 * @brief Set the activity for which the appliance can be turned on during.
 * @param Activities: A set of activities
 */
void Appliance::setActivities(const std::vector<int> &Activities) {
  this->Activities = Activities;
}

double Appliance::getSupply() const {
  return parameters.supply;
}

double Appliance::getSupplyLeft() const {
  return parameters.suppliedLeft;
}

double Appliance::getSupplyCost() const {
  return parameters.supplyCost;
}

double Appliance::getPower() const {
  return parameters.power;
}

double Appliance::getReceived() const {
  return parameters.received;
}

double Appliance::getReceivedCost() const {
  return parameters.receivedCost;
}

double Appliance::getLocalSupply() const {
  return parametersLocal.supply;
}

double Appliance::getLocalSupplyLeft() const {
  return parametersLocal.suppliedLeft;
}

double Appliance::getLocalReceived() const {
  return parametersLocal.received;
}

double Appliance::getLocalReceivedCost() const {
  return parametersLocal.receivedCost;
}

double Appliance::getLocalPower() const {
  return parametersLocal.power;
}

double Appliance::getNeighbourhoodSupply() const {
  return parametersNeighbourhood.supply;
}

double Appliance::getNeighbourhoodSupplyLeft() const {
  return parametersNeighbourhood.suppliedLeft;
}

double Appliance::getNeighbourhoodReceived() const {
  return parametersNeighbourhood.received;
}

double Appliance::getNeighbourhoodReceivedCost() const {
  return parametersNeighbourhood.receivedCost;
}

double Appliance::getNeighbourhoodPower() const {
  return parametersNeighbourhood.power;
}

double Appliance::getGridSupply() const {
  return parametersGrid.supply;
}

double Appliance::getGridSupplyLeft() const {
  return parametersGrid.suppliedLeft;
}

double Appliance::getGridReceived() const {
  return parametersGrid.received;
}

double Appliance::getGridReceivedCost() const {
  return parametersGrid.receivedCost;
}

double Appliance::getGridPower() const {
  return parametersGrid.power;
}
