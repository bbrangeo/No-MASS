// Copyright 2016 Jacob Chapman

#include <string>
#include <list>
#include <algorithm>
#include <vector>
#include "Configuration.hpp"
#include "DataStore.hpp"
#include "Building_Appliances.hpp"

Building_Appliances::Building_Appliances() {}

void Building_Appliances::setup(const ConfigStructBuilding & b) {
  std::cout << "Building_Appliances setup" << std::endl;
  buildingID = b.id;
  buildingString = "Building" + std::to_string(buildingID) + "_Appliance";

  batteriesGrid.setIDString(buildingString + "_BatteryGrid_Sum");
  batteriesGrid.setup(b.AppliancesBatteryGrid, buildingID, buildingString);
  batteries.setIDString(buildingString + "_Battery_Sum");
  batteries.setup(b.AppliancesBattery, buildingID, buildingString);
  csv.setIDString(buildingString + "_CSV_Sum");
  csv.setup(b.AppliancesCSV, buildingID, buildingString);
  large.setIDString(buildingString + "_Large_Sum");
  large.setup(b.AppliancesLarge, buildingID, buildingString);
  largeCSV.setIDString(buildingString + "_Large_CSV_Sum");
  largeCSV.setup(b.AppliancesLargeCSV, buildingID, buildingString);
  largeLearning.setIDString(buildingString + "_LargeLearning_Sum");
  largeLearning.setup(b.AppliancesLargeLearning, buildingID, buildingString);
  largeLearningCSV.setIDString(buildingString + "_LargeLearningCSV_Sum");
  largeLearningCSV.setup(b.AppliancesLargeLearningCSV, buildingID, buildingString);
  small.setIDString(buildingString + "_Small_Sum");
  small.setup(b.AppliancesSmall, buildingID, buildingString);
  fmi.setIDString(buildingString + "_FMI_Sum");
  fmi.setup(b.AppliancesFMI, buildingID, buildingString);

  datastoreIDNonShiftSupplied = DataStore::addVariable(buildingString + "_NonShift_supplied");
  datastoreIDNonShiftSuppliedCost = DataStore::addVariable(buildingString + "_NonShift_suppliedCost");
  datastoreIDNonShiftReceived = DataStore::addVariable(buildingString + "_NonShift_received");
  datastoreIDNonShiftRequested = DataStore::addVariable(buildingString + "_NonShift_requested");
  datastoreIDNonShiftCost = DataStore::addVariable(buildingString + "_NonShift_cost");

}

void Building_Appliances::preprocess() {}

void Building_Appliances::stepLocal() {
  large.hasActivities(currentStates);
  large.step(&app_negotiation);
  largeCSV.hasActivities(currentStates);
  largeCSV.step(&app_negotiation);
  largeLearning.hasActivities(currentStates);
  largeLearning.step(&app_negotiation);
  largeLearningCSV.hasActivities(currentStates);
  largeLearningCSV.step(&app_negotiation);
  small.step(&app_negotiation);
  csv.step(&app_negotiation);
  fmi.step(&app_negotiation);

  double powerShortage = large.getPower()
                  + largeCSV.getPower()
                  + largeLearning.getPower()
                  + largeLearningCSV.getPower()
                  + small.getPower()
                  + csv.getPower()
                  + fmi.getPower()
                  - large.getSupplyLeft()
                  -largeCSV.getSupplyLeft()
                  - largeLearning.getSupplyLeft()
                  - largeLearningCSV.getSupplyLeft()
                  - small.getSupplyLeft()
                  - csv.getSupplyLeft()
                  - fmi.getSupplyLeft();

    batteries.setPowerShortage(powerShortage);
    batteries.step(&app_negotiation);
    powerShortage = batteries.getPowerShortage();
    batteriesGrid.setPowerShortage(powerShortage);
    batteriesGrid.step(&app_negotiation);
}

void Building_Appliances::stepLocalNegotiation() {
  app_negotiation.process();
  small.localNegotiation(app_negotiation);
  large.localNegotiation(app_negotiation);
  largeCSV.localNegotiation(app_negotiation);
  largeLearning.localNegotiation(app_negotiation);
  largeLearningCSV.localNegotiation(app_negotiation);
  fmi.localNegotiation(app_negotiation);
  csv.localNegotiation(app_negotiation);
  batteries.localNegotiation(app_negotiation);
  batteriesGrid.localNegotiation(app_negotiation);
  app_negotiation.clear();
}

void Building_Appliances::stepNeighbourhoodNegotiation(
                          const Contract_Negotiation & building_negotiation) {

    large.neighbourhoodNegotiation(building_negotiation);
    largeCSV.neighbourhoodNegotiation(building_negotiation);
    largeLearning.neighbourhoodNegotiation(building_negotiation);
    largeLearningCSV.neighbourhoodNegotiation(building_negotiation);
    small.neighbourhoodNegotiation(building_negotiation);
    fmi.neighbourhoodNegotiation(building_negotiation);
    csv.neighbourhoodNegotiation(building_negotiation);
    batteries.neighbourhoodNegotiation(building_negotiation);
    batteriesGrid.neighbourhoodNegotiation(building_negotiation);
}

void Building_Appliances::stepGlobalNegotiation(
                          const Contract_Negotiation & building_negotiation) {

  large.globalNegotiation(building_negotiation);
  largeCSV.globalNegotiation(building_negotiation);
  largeLearning.globalNegotiation(building_negotiation);
  largeLearningCSV.globalNegotiation(building_negotiation);
  small.globalNegotiation(building_negotiation);
  fmi.globalNegotiation(building_negotiation);
  csv.globalNegotiation(building_negotiation);

  double cost = large.getReceivedCost() +
  largeCSV.getReceivedCost() +
                largeLearning.getReceivedCost() +
                largeLearningCSV.getReceivedCost() +
                small.getReceivedCost() +
                fmi.getReceivedCost() +
                csv.getReceivedCost();

  batteries.globalCost(cost);
  batteriesGrid.globalCost(cost);
  batteries.globalNegotiation(building_negotiation);
  batteriesGrid.globalNegotiation(building_negotiation);

  double supply = large.getSupply() +
  largeCSV.getSupply() +
          small.getSupply() +
          fmi.getSupply() +
          csv.getSupply();
  double supplyCost = large.getSupplyCost() +
  largeCSV.getSupplyCost() +
          small.getSupplyCost() +
          fmi.getSupplyCost() +
          csv.getSupplyCost();
  double received = large.getReceived() +
  largeCSV.getReceived() +
          small.getReceived() +
          fmi.getReceived() +
          csv.getReceived();
  double power = large.getPower() +
  largeCSV.getPower() +
          small.getPower() +
          fmi.getPower() +
          csv.getPower();
  double receivedCost = large.getReceivedCost() +
  largeCSV.getReceivedCost() +
          small.getReceivedCost() +
          fmi.getReceivedCost() +
          csv.getReceivedCost();

  DataStore::addValue(datastoreIDNonShiftSupplied, supply);
  DataStore::addValue(datastoreIDNonShiftSuppliedCost, supplyCost);
  DataStore::addValue(datastoreIDNonShiftReceived, received);
  DataStore::addValue(datastoreIDNonShiftRequested, power);
  DataStore::addValue(datastoreIDNonShiftCost, receivedCost);

  currentStates.clear();
  app_negotiation.clear();

  large.clear();
  largeCSV.clear();
  largeLearning.clear();
  largeLearningCSV.clear();
  small.clear();
  fmi.clear();
  csv.clear();
  batteries.clear();
  batteriesGrid.clear();
}

void Building_Appliances::postprocess() {
  largeLearning.postprocess();
  largeLearningCSV.postprocess();
  batteries.postprocess();
  batteriesGrid.postprocess();
}

void Building_Appliances::postTimeStep() {
}

double Building_Appliances::getTotalPower() const {
  return totalPower;
}

void Building_Appliances::addCurrentStates(const int stateid) {
    currentStates.push_back(stateid);
}

void Building_Appliances::addContactsTo(
                                Contract_Negotiation * building_negotiation,
                              const bool battery) {
  large.addGlobalContactsTo(building_negotiation);
  largeCSV.addGlobalContactsTo(building_negotiation);
  largeLearning.addGlobalContactsTo(building_negotiation);
  largeLearningCSV.addGlobalContactsTo(building_negotiation);
  small.addGlobalContactsTo(building_negotiation);
  csv.addGlobalContactsTo(building_negotiation);
  fmi.addGlobalContactsTo(building_negotiation);
  if (battery) {
    batteries.addGlobalContactsTo(building_negotiation);
    batteriesGrid.addGlobalContactsTo(building_negotiation);
  }
}

void Building_Appliances::stepAppliancesUseBatteries(
                    Contract_Negotiation * building_negotiation){
  batteries.neighbourhoodNegotiationBattery(building_negotiation);
  batteriesGrid.neighbourhoodNegotiationBattery(building_negotiation);
}
