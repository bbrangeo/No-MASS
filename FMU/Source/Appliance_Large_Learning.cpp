// Copyright 2016 Jacob Chapman

#include <string>
#include <vector>
#include "Configuration.hpp"
#include "DataStore.hpp"
#include "Utility.hpp"
#include "Appliance_Large_Learning.hpp"

Appliance_Large_Learning::Appliance_Large_Learning() {
}

void Appliance_Large_Learning::setup(ConfigStructAppliance a) {
  setID(a.id);
  setHoulyPriority(a.priority);
  setHoulyTimeRequired(a.timeRequired);
  setActivities(a.activities);
  setFile(a.fileProfile);

  std::string buildingString = "Building" + std::to_string(buildingID);
  std::string s_fullname = buildingString + "_Appliance";
  s_fullname = s_fullname + std::to_string(id) + "_";
  databaseIDactual = DataStore::addVariable(s_fullname + "actual");
  qLearning.setEpsilon(a.epsilon);
  qLearning.setAlpha(a.alpha);
  qLearning.setGamma(a.gamma);
  qLearning.setUpdate(a.update);
  qLearning.setFilename(s_fullname);
  qLearning.setStates(24);
  qLearning.setActions(24);
  qLearning.setId(id);
  qLearning.setup();
  setupModel();
}

/**
 * @brief using Q-learning calculate the learnt start time
 * @details set the current state for Q-learning
 * get the corresponding best action (start time)
 */
void Appliance_Large_Learning::calculateLearntStartTime() {
  qLearning.setState(powerProfile.front().requestedTime);
  powerProfile.front().startTime = qLearning.getAction();
}

double Appliance_Large_Learning::getPowerAt(const int timestep) {
  return model.consumption(timestep);
}

/**
 * @brief Check large appliance model for a turn on, then generate the profile
 * @details Calculate if the applaince is predicted a turn on
 * if so increment model and save teh power demand until turn off
 */
void Appliance_Large_Learning::calculateProfile() {
  int stepCount = Configuration::getStepCount();
  double p = getPowerAt(stepCount);
  profileStruct profile;
  while (model.onAt(stepCount)) {
    profile.power.push_back(p);
    stepCount++;
    profile.nonLearningStep++;
    model.decreaseDuration();
    p = getPowerAt(stepCount);
  }
  if (profile.power.size()) {
    powerProfile.push(profile);
  }
}


void Appliance_Large_Learning::startLearningPeriod(const int hourOfTheDay) {
  if (hourOfTheDay == powerProfile.front().startTime &&
      powerProfile.front().isLearningPeriod == false) {
    powerProfile.front().isLearningPeriod = true;
  }
}

/**
 * @brief The reward function
 * @details calculate the reward function
 * @return the reward
 */
double Appliance_Large_Learning::calculateReward() {
  // return -(cost / powerProfile.size() * hoursWaited * 0.1);
  double costOfProfile = powerProfile.front().cost;
  double sizeOfProfile = powerProfile.front().power.size();
  double highestTimeRequired = powerProfile.front().maxTimeRequired;
  return -(costOfProfile / sizeOfProfile * highestTimeRequired);
}

bool Appliance_Large_Learning::learnStepLessThanProfile() const {
  auto it = powerProfile.front();
  int s = it.power.size();
  int l = it.learningStep;
  return (l >= s);
}

void Appliance_Large_Learning::eraseFirstPowerProfile() {
  powerProfile.pop();
}

void Appliance_Large_Learning::stopLearningPeriod(const int hourOfTheDay) {
  if (learnStepLessThanProfile()) {
    double reward = calculateReward();
    qLearning.setReward(reward);
    qLearning.setState(hourOfTheDay);
    qLearning.learn();
    eraseFirstPowerProfile();
  }
}

double Appliance_Large_Learning::getRequiredTime(int hourOfDay) const {
  return houlyTimeRequired[hourOfDay];
}

void Appliance_Large_Learning::step() {
  setPower(0.0);

  int hourOfTheDay = calculateHourOfDay();
  if (powerProfile.empty() == false) {
    if (powerProfile.front().startTime < 0) {
      calculateLearntStartTime();
    }
    double requiredTime = getRequiredTime(hourOfTheDay);
    if (powerProfile.front().maxTimeRequired < requiredTime) {
      powerProfile.front().maxTimeRequired = requiredTime;
    }
    startLearningPeriod(hourOfTheDay);
    if (powerProfile.front().isLearningPeriod) {
      setPower(powerProfile.front().power[powerProfile.front().learningStep]);
      powerProfile.front().learningStep++;
      stopLearningPeriod(hourOfTheDay);
    }
  }

  stepApplianceOffAndNotLearning(hourOfTheDay);
  saveActualProfile();
}


void Appliance_Large_Learning::beforeClear() {
  if (powerProfile.empty() == false && powerProfile.front().isLearningPeriod) {
    addToCost(getReceivedCost());
  }
}

void Appliance_Large_Learning::saveActualProfile() {
  double p = 0.0;
  if (powerProfile.empty() == false &&
      powerProfile.front().nonLearningStep > 0) {
    int i = powerProfile.front().power.size() -
            powerProfile.front().nonLearningStep;
    p = powerProfile.front().power[i];
    powerProfile.front().nonLearningStep--;
  }
  DataStore::addValue(databaseIDactual, p);
}

bool Appliance_Large_Learning::isModelOn() {
  int timeStep = Configuration::getStepCount();
  return model.onAt(timeStep) && match;
}

/**
 * @brief Appliance is off and we are not learning
 * @details check if the appliance model should checked for a turn on
 * get the profile
 * if there is data in the profile get the learn start time
 */
void Appliance_Large_Learning::stepApplianceOffAndNotLearning(const int hourOfTheDay) {
  if (isModelOn()) {
    calculateProfile();
    powerProfile.back().requestedTime = hourOfTheDay;
  }
}

/**
 * @brief Appliance cost is added to for calculating the reward function
 * @param cost cost of using the appliance
 */
void Appliance_Large_Learning::addToCost(const double cost) {
  if (!powerProfile.empty() && powerProfile.front().isLearningPeriod) {
    powerProfile.front().cost += cost;
  }
}

/**
 * @brief Appliance prostprocess, calls Q-Learning save data
 */
void Appliance_Large_Learning::postprocess() {
  qLearning.printQ();
}

void Appliance_Large_Learning::setHoulyTimeRequired(
  const std::vector<double> &houlyTimeRequired) {
  this->houlyTimeRequired = houlyTimeRequired;
}
