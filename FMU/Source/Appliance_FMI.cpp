// Copyright 2016 Jacob Chapman

#include <string>
#include "DataStore.hpp"
#include "Appliance_FMI.hpp"

Appliance_FMI::Appliance_FMI() {
}

void Appliance_FMI::setup(ConfigStructAppliance a) {
  setID(a.id);
  setHoulyPriority(a.priority);
  setFMIVariableName(a.variableName);
}

/**
 * @brief Set the variable name of the variable we want to retrieve at run time
 */
void Appliance_FMI::setFMIVariableName(const std::string &FMIVariableName) {
  this->FMIVariableName = DataStore::addVariable(FMIVariableName);
}

/**
 * @brief retrieves the value from the given variable name in the DataStore
 * @details At each simulation timestep with FMI all input variables are written
 * to the data store, this simply returns the variable
 */
void Appliance_FMI::step() {
  setPower(DataStore::getValue(FMIVariableName));
}
