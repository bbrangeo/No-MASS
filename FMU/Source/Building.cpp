// Copyright 2015 Jacob Chapman

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <utility>
#include "SimulationTime.hpp"
#include "Configuration.hpp"
#include "DataStore.hpp"
#include "Occupant.hpp"
#include "Utility.hpp"
#include "Model_Presence.hpp"
#include "Building.hpp"

Building::Building() {}

 /**
  * @brief Set up the building from the configuration struct
  * @details Setup the building, adds the zones, adds the occupants, adds the appliances
  * @param buildingInput The configuration struct which values have been populated form configuration file
  */
void Building::setup(const ConfigStructBuilding &buildingInput) {
    std::cout << "Building setup" << std::endl;
    name = buildingInput.name;
    id = buildingInput.id;
    std::string buildingID = "Building" + std::to_string(id) + "_";
    for (std::pair<std::string, ConfigStructZone> z : buildingInput.zones) {
      zones.push_back(std::make_shared<Building_Zone>(Building_Zone()));
      zones.back()->setName(z.second.name);
      zones.back()->setActive(z.second.active);
      zones.back()->setIDString(buildingID + z.second.name);
      zones.back()->setup(z.second);
    }
    int popSize = buildingInput.agents.size();
    std::vector<int> pop = Utility::randomIntVect(popSize);
    // setup each agent randomly
    for (int a : pop) {
      std::string occid = std::to_string(a);
      population.push_back(Occupant());
      population.back().setBuildingID(id);
      population.back().setBuildingName(name);
      population.back().setIDString(buildingID + "Occupant" + occid + "_");
      population.back().setup(a, buildingInput.agents[a], zones);
    }
    appliances.setup(buildingInput);
}

/**
 * @brief building preprocess
 * @details calls the building appliances preprocess
 */
void Building::preprocess() {
  appliances.preprocess();
}

/**
 * @brief Steps over the appliances locally, then calls the contract negotiate to resolve
 * local demand and supply
 */
void Building::stepAppliancesUse() {
  appliances.stepLocal();
  appliances.stepLocalNegotiation();
}

/**
 * @brief Steps over the appliances on a Neighbourhood level
 */
void Building::stepAppliancesNegotiationNeighbourhood(
                          const Contract_Negotiation & building_negotiation) {
      appliances.stepNeighbourhoodNegotiation(building_negotiation);
}

/**
 * @brief Steps over the appliances on a Grid level
 */
void Building::stepAppliancesNegotiation(
                          const Contract_Negotiation & building_negotiation) {
      appliances.stepGlobalNegotiation(building_negotiation);
}

/**
 * @brief returns the buildings appliance use
 * @return building power useage
 */
double Building::getPower() const {
  return appliances.getTotalPower();
}

/**
 * @brief The buildings timestep function
 * @details calls the timestep function for the appliance, occupants and zones.
 * Also calls the functions to run the social interactions between occupants, and between occupants and appliances.
 */
void Building::step() {
  int popSize = population.size();
  std::vector<int> pop = Utility::randomIntVect(popSize);
  // step each occupant
  for (int a : pop) {
      population[a].step();
      appliances.addCurrentStates(population[a].getStateID());
  }

  //for each active zone
  for (std::shared_ptr<Building_Zone> &zone : zones) {
      if (zone->isActive()) {
        // set number of occupants in the zone
        setOccupantCountForZone(zone);

        if (Configuration::info.windows
          || Configuration::info.windowsLearn) {
            // calculate the window interaction and set for zone
            setOccupantWindowDecisionForZone(zone);
        }
        if (Configuration::info.shading) {
          // calculate the shade interaction and set for zone
            setOccupantBlindDecisionForZone(zone);
        }
        if (Configuration::info.lights) {
            // calculate the light interaction and set for zone
            setOccupantLightDecisionForZone(zone);
        }
        if (Configuration::info.heating) {
            // calculate the heating setpoint interaction and set for zone
            setOccupantHeatDecisionsForZone(zone);
        }
        // calculate the occupant gains and set for zone
        setOccupantGainsForZone(zone);
        // calculate the appliance gains and set for zone
        setAppGainsForZone(zone);
      }
  }
  // Run BDI rules for the building
  buildingInteractions();
  for (std::shared_ptr<Building_Zone> &zone : zones) {
    if (zone->isActive()) {
      // call timestep function on each zone
      zone->step();
    }
  }
}

/**
 * @brief The BDI intractions with the building
 * @details Runs the BDI rule for the building as adapted from
 * \n Chapman, J., Siebers, P., & Robinson, D. (2017). Data Scarce Behavioural Modelling and the Representation of Social Interactions. Unpublished Manuscript, 1–48.
 */
void Building::buildingInteractions() {
  if (Configuration::info.ShadeClosedDuringNight) {
    int hourOfDay = SimulationTime::hourOfDay;
    if (hourOfDay > 19 || hourOfDay < 6) {
      for (std::shared_ptr<Building_Zone> &zone : zones) {
        if (zone->isActive()) {
          zone->setBlindState(0);
        }
      }
    }
  }
}


/**
 * @brief Loops over the values and chooses the value with the highest power
 * @details Loops over the values and calculates if the choice is an increase or
 * decrease on the current state chooses whether to stay the same, increase, or
 * decrease based on the highest value
 * if the power is equal for two or more options then toss a coin.
 * @param  val          Occupant desires
 * @param  power        Occupant power
 * @param  currentState current state of the device
 * @return              The final negotiated choice for the device
 */
double Building::decisionDoubleVec(const std::vector<double> & val,
                                  const std::vector<double> & power,
                                  const double currentState) const {

  double state = currentState;
  if (!val.empty()){
    double totalIncrease = 0;
    double totalDecrease = 0;
    double increase = 0;
    double decrease = 0;
    double increasePower = 0;
    double decreasePower = 0;
    double samePower = 0;
    for (unsigned int i = 0; i < val.size(); i++) {
      double d = val[i];
      double powerValue = power[i];
      if (d < currentState) {
        decrease++;
        totalDecrease = totalDecrease + d;
        decreasePower = decreasePower + powerValue;
      } else if (d > currentState) {
        increase++;
        totalIncrease = totalIncrease + d;
        increasePower = increasePower + powerValue;
      } else {
        samePower = samePower + powerValue;
      }
    }

    if (samePower == increasePower
        && samePower == decreasePower
        && increasePower == decreasePower) {
        // if same toss a coin
        int i = Utility::randomInt(0, 2);
        if (i == 0) {
            state = totalIncrease / increase;
        } else if (i == 1) {
            state = totalDecrease / decrease;
        }
    } else if (samePower < increasePower && increasePower > decreasePower) {
        // choose an increase
        state = totalIncrease / increase;
    } else if (samePower < decreasePower && increasePower < decreasePower) {
        // choose an decrease
        state = totalDecrease / decrease;
    } else if (samePower == increasePower && samePower > decreasePower) {
        // toss a coin if stay the same and increase
        if (Utility::tossACoin()) {
            state = totalIncrease / increase;
        }
    } else if (samePower > increasePower && samePower == decreasePower) {
        // toss a coin if stay the same and decrease
        if (Utility::tossACoin()) {
            state = totalDecrease / decrease;
        }
    } else if (samePower < increasePower
        && samePower < decreasePower
        && increasePower == decreasePower) {
        // toss a coin if stay the increase and decrease
        if (Utility::tossACoin()) {
            state = totalIncrease / increase;
        } else {
            state = totalDecrease / decrease;
        }
    }
  }
  return state;
}

/**
 * @brief collects the heating setpoint states and sets the negotiated outcome
 * @details collects the heating setpoint states of each occupant, calls the negotiate function
 * and set the desired negotiated state to the zone
 * @param zone A pointer to a buldings zone
 */
void Building::setOccupantHeatDecisionsForZone(
      std::shared_ptr<Building_Zone> zone) {
  double state = zone->getHeatingState();
  std::vector<double> desires;
  std::vector<double> powers;
  for (const Occupant &agent : population) {
    double desire = agent.getDesiredHeatState(*zone);
    double power = agent.getPower();
    desires.push_back(desire);
    powers.push_back(power);
  }
  state = decisionDoubleVec(desires, powers, state);
  zone->setHeatingState(state);
}

/**
 * @brief collects the occupants heatgains and sets for the zone
 * @details collects the heatgains of each occupant, calculates an average for per occupant
 * and sets the desired negotiated state to the zone
 * @param zone A pointer to a buldings zone
 */
void Building::setOccupantGainsForZone(std::shared_ptr<Building_Zone> zone) {
    double numberOfOccupants = 0;
    double totalRadientGains = 0;
    double aveRadientGains = 0;

    for (Occupant &agent : population) {
        if  (agent.isActionHeatGains(*zone)) {
            numberOfOccupants++;
            totalRadientGains += agent.getCurrentRadientGains(*zone);
        }
    }
    if (totalRadientGains > 0) {
        aveRadientGains = totalRadientGains / numberOfOccupants;
    }
    zone->setCurrentOccupantGains(aveRadientGains);
}

/**
 * @brief collects the occupants heatgains and sets for the zone
 * @details collects the heatgains of each occupant, calculates an average for per occupant
 * and sets the desired negotiated state to the zone
 * @param zone A pointer to a buldings zone
 */
void Building::setAppGainsForZone(std::shared_ptr<Building_Zone> zone) {
    double totalRadientGainsAppliance = 0;
    double aveRadientGains = 0;
    double numberOfOccupantsAppliance = 0;

    for (Occupant &agent : population) {
        numberOfOccupantsAppliance++;
        if  (agent.isActionAppliance(*zone)) {
            totalRadientGainsAppliance += agent.getDesiredAppliance(*zone);
        }
    }
    if (totalRadientGainsAppliance > 0) {
        aveRadientGains = totalRadientGainsAppliance /
                                    numberOfOccupantsAppliance;
    }
    zone->setAppFraction(aveRadientGains);
}

/**
 * @brief collects the occupants window opening choices and sets for the zone
 * @details collects the window opening choice of each occupant, calls the negotiate function
 * on the collected values and sets the desired negotiated state to the zone
 * @param zone A pointer to a buldings zone
 */
void Building::setOccupantWindowDecisionForZone(
        std::shared_ptr<Building_Zone> zone) {
    double open = 0;
    double close = 0;
    int numberOfActiveOccupants = 0;

    for (const Occupant &agent : population) {
        if (agent.isActionWindow(*zone)) {
            double power = agent.getPower();
            numberOfActiveOccupants++;
            if (agent.getDesiredWindowState(*zone)) {
                open = open + power;
            } else {
                close = close + power;
            }
        }
    }
    if (numberOfActiveOccupants > 0) {
        zone->setWindowState(decisionBoolean(close, open));
    }
}

/**
 * @brief collects the occupants window opening choices and sets for the zone
 * @details collects the window opening choice of each occupant, calls the negotiate function
 * on the collected values and sets the desired negotiated state to the zone
 * @param zone A pointer to a buldings zone
 */
void Building::setOccupantBlindDecisionForZone(
        std::shared_ptr<Building_Zone> zone) {
    double state = zone->getBlindState();
    std::vector<double> desires;
    std::vector<double> powers;
    for (const Occupant &agent : population) {
        if (agent.isActionShades(*zone)) {
            double desire = agent.getDesiredShadeState(*zone);
            double power = agent.getPower();
            desires.push_back(desire);
            powers.push_back(power);
        }
    }
    state = decisionDoubleVec(desires, powers, state);
    zone->setBlindState(state);
}

/**
 * @brief takes two values and return true if val1 is less than val2
 * @details If they are equal toss a coin
 * @param  val1 The first value
 * @param  val2 The second value
 * @return      true if val1 < val2, or random if equal
 */
bool Building::decisionBoolean(const double val1, const double val2) const {
  bool ret = val1 < val2;
  if (val1 == val2) {
      ret = Utility::tossACoin();
  }
  return ret;
}

/**
 * @brief collects the occupants light choices and sets for the zone
 * @details collects the light choice of each occupant, calls the negotiate function
 * on the collected values and sets the desired negotiated state to the zone
 * @param zone A pointer to a buldings zone
 */
void Building::setOccupantLightDecisionForZone(
        std::shared_ptr<Building_Zone> zone) {
    double open = 0;
    double close = 0;
    double numberOfActiveOccupants = 0;
    for (const Occupant &agent : population) {
        if (agent.isActionLights(*zone)) {
            numberOfActiveOccupants++;
            double power = agent.getPower();
            if (agent.getDesiredLightState(*zone)) {
                open = open + power;
            } else {
                close = close + power;
            }
        }
    }
    if (numberOfActiveOccupants > 0.0) {
        zone->setLightState(decisionBoolean(close, open));
    }
}

/**
 * @brief sets the number of occupants active in a zone
 * @details sets the number of occupants active in a zone
 * @param zone A pointer to a buldings zone
 */
void Building::setOccupantCountForZone(std::shared_ptr<Building_Zone> zone) {
    double numberOfOccupants = 0;
    for (const Occupant &agent : population) {
        if (agent.currentlyInZone(*zone)) {
            numberOfOccupants++;
        }
    }
    zone->setOccupantFraction(numberOfOccupants);
}

/**
 * @brief calls the posttime functions on the occupants and appliances
 */
void Building::postTimeStep() {
  for (Occupant &agent : population) {
      agent.postTimeStep();
  }
  appliances.postTimeStep();
}

/**
 * @brief calls the postprocess functions on the occupants and appliances
 */
void Building::postprocess() {
    for (Occupant &agent : population) {
        agent.postprocess();
    }
    appliances.postprocess();
}

/**
 * @brief checks if a building has a given zone
 * @param  zoneName name of the zone
 * @return          true if the zone is in the building
 */
bool Building::hasZone(const std::string& zoneName) const {
  bool has = false;
  for (const std::shared_ptr<Building_Zone> & z : zones) {
    if (z->isNamed(zoneName)) {
      has = true;
    }
  }
  return has;
}

int Building::getID() const {
    return id;
}

void Building::addContactsTo(Contract_Negotiation * building_negotiation,const bool battery) {
  appliances.addContactsTo(building_negotiation, battery);
}

void Building::stepAppliancesUseBatteries(Contract_Negotiation * building_negotiation){
  appliances.stepAppliancesUseBatteries(building_negotiation);
}
