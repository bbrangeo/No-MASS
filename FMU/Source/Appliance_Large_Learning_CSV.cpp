// Copyright 2016 Jacob Chapman

#include <string>
#include <vector>
#include "Configuration.hpp"
#include "Utility.hpp"
#include "Appliance_Large_Learning_CSV.hpp"

Appliance_Large_Learning_CSV::Appliance_Large_Learning_CSV() {
}

/**
 * @brief Check large appliance model for a turn on, then generate the profile
 * @details Calculate if the applaince is predicted a turn on
 * if so increment model and save teh power demand until turn off
 */
void Appliance_Large_Learning_CSV::calculateProfile() {
  profileStruct profile;
  profile.power = profileCSV;
  profile.nonLearningStep = profileCSV.size();
  powerProfile.push(profile);
  model.setDuration(0.0);
}

/**
 * @brief Set up the large appliance model, reading in the large applaince
 * configuration file
 * @details Sets the large appliance configuration file and gives the model
 * the id of the appliance in the file
 */
void Appliance_Large_Learning_CSV::setupModel() {
  model.setID(id);
  model.parseConfiguration(Configuration::FileLargeAppliance);

  for (auto x: Utility::csvToTable<double>(file, false)) {
    profileCSV.push_back(x[0]);
  }
}
