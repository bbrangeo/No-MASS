// Copyright 2015 Jacob Chapman

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>
#include <memory>
#include <cfloat>
#include <cstddef>
#include <utility>


#include "Model_Activity_Survival.hpp"
#include "Model_Presence.hpp"
#include "State.hpp"
#include "DataStore.hpp"
#include "Configuration.hpp"
#include "StateMachine.hpp"
#include "Utility.hpp"
#include "Occupant.hpp"

Occupant::Occupant() {
}

/**
 * @brief Initialises the occupant
 * @details Set occupant parameters, preprocesses the activities or states and
 * sets up the inital state of the occupant
 * @param id    The occupants id
 * @param agent The configuration struct, built from config file
 * @param zones Zones that the agent can inhabit
 */
void Occupant::setup(int id, const ConfigStructAgent &agent,
                     const std::vector<std::shared_ptr<Building_Zone> > &zones) {
  std::cout << "Occupant setup" << std::endl;
  this->id = id;
  datastoreIdActivity = DataStore::addVariable(idString + "Activity");
  datastoreIdHeatGains = DataStore::addVariable(idString + "HeatGains");
  bedroom = buildingName + agent.bedroom;
  office = buildingName + agent.office;
  power = agent.power;
  if (power == 0) {
    // no power causes problems with agent not present allso having no power
    // so set smallest possible
    power = std::numeric_limits<double>::epsilon();
  }

  for (const std::shared_ptr<Building_Zone> &buldingZone: zones) {
    if (Configuration::info.presencePage &&
        buldingZone->hasActivity(3) &&
        !buldingZone->isNamed(office)) {
      continue;
    }
    if (!Configuration::info.presencePage &&
        buldingZone->hasActivity(0) &&
        !buldingZone->isNamed(bedroom) &&
        buldingZone->getNumberOfActivities() == 1) {
      continue;
    }
    agentZones.push_back(Occupant_Zone());
    agentZones.back().setup(buildingID, *buldingZone, id, agent);
  }
  if (Configuration::info.presencePage) {
    model_presenceFromPage(agent);
  } else {
    model_activity(agent);
  }
  initialiseStates(zones);
}

/**
 * @brief Initialises the states and assigns them to a zone
 * @details builds the states an occupant can be in, and then matches the states
 * to their corresponding zones
 * @param zones Zones that the occupant can inhabit
 */
void Occupant::initialiseStates(
  const std::vector<std::shared_ptr<Building_Zone> > &zones) {
  State present(-100, -1, -1, "");
  if (Configuration::info.presencePage) {
    State it(3, 70, 1, "IT");
    matchStateToZone(&it, zones);
    present.addState(it);
  } else {
    State sleep(0, 46, 2.55, "Sleep");
    State passive(1, 58, 1, "Passive");
    State audioVisual(2, 70, 1, "AudioVisual");
    State it(3, 70, 1, "IT");
    State cooking(4, 116, 1, "Cooking");
    State cleaning(5, 116, 1, "Cleaning");
    State washing(6, 116, 0, "Washing");
    State metabolic(7, 93, 1, "Metabolic");
    State washingAppliance(8, 116, 1, "WashingAppliance");
    matchStateToZone(&sleep, zones);
    matchStateToZone(&passive, zones);
    matchStateToZone(&washingAppliance, zones);
    matchStateToZone(&washing, zones);
    matchStateToZone(&audioVisual, zones);
    matchStateToZone(&cleaning, zones);
    matchStateToZone(&cooking, zones);
    matchStateToZone(&metabolic, zones);
    matchStateToZone(&it, zones);
    present.addState(sleep);
    present.addState(passive);
    present.addState(washingAppliance);
    present.addState(washing);
    present.addState(audioVisual);
    present.addState(cleaning);
    present.addState(cooking);
    present.addState(metabolic);
    present.addState(it);
  }
  stateMachine.addState(present);
  State out(9, 0, 1, "Out");
  matchStateToZone(&out, zones);
  stateMachine.addState(out);
  setState(out);
}

/**
 * @brief matches a state to a zone
 * @param s     The state
 * @param zones zones to be searched for which the state will belong
 */
void Occupant::matchStateToZone(State *s,
                                const std::vector<std::shared_ptr<Building_Zone> > &zones) {
  for (unsigned int i = 0; i < zones.size(); i++) {

    if (Configuration::info.presencePage &&
        zones[i]->hasActivity(3) &&
        !zones[i]->isNamed(office)) {
      std::cout << "\n\nOccupant::matchStateToZone 1" << std::endl;

      continue;
    }

    if (!Configuration::info.presencePage &&
        zones[i]->hasActivity(0) &&
        !zones[i]->isNamed(bedroom) &&
        zones[i]->getNumberOfActivities() == 1) {
      std::cout << "\n\nOccupant::matchStateToZone 2" << std::endl;

      continue;
    }

    if (zones[i]->hasActivity(s->getId())) {
      std::cout << "\n\n Occupant::matchStateToZone 3" << std::endl;
      std::cout << "Occupant::matchStateToZone s->getId() " << s->getId() <<std::endl;
      std::cout << "Occupant::matchStateToZone zones[i]->hasActivity(s->getId()) " << zones[i]->hasActivity(s->getId())<<std::endl;

      s->setZonePtr(zones[i]);
      break;
    }
  }
}

/**
 * @brief the occupant timestep function
 * @detail moves the agent to their new state, calls the agents understanding
 * of zone that an agent will be interacting with.
 */
void Occupant::step() {
  int stepCount = Configuration::getStepCount();
  int newStateID = activities.at(stepCount);
  zonePtrPrevious = state.getZonePtr();

  state = stateMachine.transistionTo(newStateID);
  zonePtr = state.getZonePtr();

  metabolicRate = state.getMetabolicRate();
  clo = state.getClo();
  for (Occupant_Zone &agentZone: agentZones) {
    // BORIS
    if (zonePtr == nullptr || zonePtrPrevious == nullptr) {
      continue;
    }
    agentZone.setClo(clo);
    agentZone.setMetabolicRate(metabolicRate);
    agentZone.step(*zonePtr, *zonePtrPrevious, activities);
    agentZone.stepPre(*zonePtr, *zonePtrPrevious, activities);
  }
  // BORIS
  if (zonePtr == nullptr || zonePtrPrevious == nullptr) {
    std::cerr << "Erreur Occupant : zonePtr est null après transition à l'état " << newStateID << std::endl;
    return;
  }
  DataStore::addValue(datastoreIdHeatGains, getCurrentRadientGains(*zonePtr));
  DataStore::addValue(datastoreIdActivity, newStateID);
}

/**
 * @brief Calls the activity model
 * @param agent the config struct with the info needed for the activity model,
 * saves the result to the activity array for recall later
 */
void Occupant::model_activity(const ConfigStructAgent &agent) {
  Model_Activity_Survival ma;
  ma.setAge(agent.age);
  ma.setComputer(agent.computer);
  ma.setCivstat(agent.civstat);
  ma.setUnemp(agent.unemp);
  ma.setRetired(agent.retired);
  ma.setEdtry(agent.edtry);
  ma.setFamstat(agent.famstat);
  ma.setSex(agent.sex);
  ma.setProbMap(agent.profile);
  activities = ma.preProcessActivities();
}

/**
 * @brief Calls the resence model
 * @param agent the config struct with the info needed for the presence model,
 * saves the result to the activity array for recall later. Here there are two states
 * IT and Out
 */
void Occupant::model_presenceFromPage(const ConfigStructAgent &agent) {
  Model_Presence presence;
  presence.setProbMap(agent.profile);
  presence.calculatePresenceFromPage();
  for (unsigned int i = 0; i < presence.size(); ++i) {
    if (presence.at(i)) {
      activities.push_back(3);
    } else {
      activities.push_back(9);
    }
  }
}


double Occupant::getCurrentRadientGains(const Building_Zone &zone) const {
  double state = 0.0;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      state = agentZone.getHeatgains();
      break;
    }
  }
  return state;
}

double Occupant::getPower() const {
  return power;
}

bool Occupant::getDesiredLightState(const Building_Zone &zone) const {
  bool state = false;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      state = agentZone.getDesiredLightState();
      break;
    }
  }
  return state;
}

bool Occupant::getDesiredWindowState(const Building_Zone &zone) const {
  bool state = false;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      state = agentZone.getDesiredWindowState();
      break;
    }
  }
  return state;
}

double Occupant::getDesiredShadeState(const Building_Zone &zone) const {
  double state = 1.0;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      state = agentZone.getDesiredShadeState();
      break;
    }
  }
  return state;
}

double Occupant::getDesiredAppliance(const Building_Zone &zone) const {
  double state = 1.0;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      state = agentZone.getDesiredAppliance();
      break;
    }
  }
  return state;
}

double Occupant::getPMV(const Building_Zone &zone) const {
  double state = 0.0;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      state = agentZone.getPMV();
      break;
    }
  }
  return state;
}

double Occupant::getDesiredHeatState(const Building_Zone &zone) const {
  double state = 0.0;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      state = agentZone.getDesiredHeatingSetPoint();
      break;
    }
  }
  return state;
}

bool Occupant::currentlyInZone(const Building_Zone &zone) const {
  if (state.getZonePtr() == nullptr) {
    std::cerr << "Erreur : zonePtr est null currentlyInZone" << std::endl;
    return false;
  }

  return zone.getId() == state.getZonePtr()->getId();
}

bool Occupant::previouslyInZone(const Building_Zone &zone) const {
  bool inZone = false;
  if (Configuration::getStepCount() > 0) {
    inZone = zone.getId() == zonePtrPrevious->getId();
  }
  return inZone;
}

bool Occupant::InteractionOnZone(const Building_Zone &zone) const {
  return currentlyInZone(zone) || previouslyInZone(zone);
}

void Occupant::postprocess() {
  for (Occupant_Zone &agentZone: agentZones) {
    agentZone.postprocess();
  }
}

void Occupant::setState(const State &state) {
  this->state = state;
}

bool Occupant::isActionWindow(const Building_Zone &zone) const {
  bool act = false;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      act = agentZone.isActionWindow();
      break;
    }
  }
  return act;
}

bool Occupant::isActionLights(const Building_Zone &zone) const {
  bool act = false;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      act = agentZone.isActionLights();
      break;
    }
  }
  return act;
}

bool Occupant::isActionShades(const Building_Zone &zone) const {
  bool act = false;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      act = agentZone.isActionShades();
      break;
    }
  }
  return act;
}

bool Occupant::isActionHeatGains(const Building_Zone &zone) const {
  bool act = false;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      act = agentZone.isActionHeatGains();
      break;
    }
  }
  return act;
}

bool Occupant::isActionLearning(const Building_Zone &zone) const {
  bool act = false;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      act = agentZone.isActionLearning();
      break;
    }
  }
  return act;
}

bool Occupant::isActionAppliance(const Building_Zone &zone) const {
  bool act = false;
  for (const Occupant_Zone &agentZone: agentZones) {
    if (agentZone.getId() == zone.getId()) {
      act = agentZone.isActionAppliance();
      break;
    }
  }
  return act;
}

void Occupant::postTimeStep() {
  for (Occupant_Zone &agentZone: agentZones) {
    agentZone.postTimeStep();
  }
}

int Occupant::getStateID() const {
  return state.getId();
}

void Occupant::setBuildingName(const std::string &buildingName) {
  this->buildingName = buildingName;
}
