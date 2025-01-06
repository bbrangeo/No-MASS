// Copyright 2016 Jacob Chapman

#ifndef OCCUPANT_H_
#define OCCUPANT_H_

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include "StateMachine.hpp"
#include "State.hpp"
#include "Agent.hpp"
#include "Occupant_Zone.hpp"

/**
* @brief The occupant agent
* @details The occupant agent
*/

class Occupant : public Agent {
 public:
    Occupant();
    void setup(int id, const ConfigStructAgent &agent,
                const std::vector<std::shared_ptr<Building_Zone>> &zones);

    void step();
    void setState(const State &state);
    void setBuildingName(const std::string & buildingName);
    void postTimeStep();
    void zoneInteractions();
    void postprocess();
    bool currentlyInZone(const Building_Zone &zone) const;
    bool InteractionOnZone(const Building_Zone &zone) const;
    bool getDesiredLightState(const Building_Zone &zone) const;
    bool getDesiredWindowState(const Building_Zone &zone) const;
    bool isActionWindow(const Building_Zone &zone) const;
    bool isActionLights(const Building_Zone &zone) const;
    bool isActionShades(const Building_Zone &zone) const;
    bool isActionHeatGains(const Building_Zone &zone) const;
    bool isActionLearning(const Building_Zone &zone) const;
    bool isActionAppliance(const Building_Zone &zone) const;
    bool previouslyInZone(const Building_Zone &zone) const;
    int getStateID() const;
    double getDesiredShadeState(const Building_Zone &zone) const;
    double getDesiredAppliance(const Building_Zone &zone) const;
    double getDesiredHeatState(const Building_Zone &zone) const;
    double getCurrentRadientGains(const Building_Zone &zone) const;
    double getPower() const;

 private:
    void model_presenceFromPage(const ConfigStructAgent &agent);
    void model_pastAndFutureDurations();
    void model_activity(const ConfigStructAgent &agent);
    void matchStateToZone(State *s,
                   const std::vector<std::shared_ptr<Building_Zone>> &zones);

    void initialiseStates(
                     const std::vector<std::shared_ptr<Building_Zone>> &zones);
    bool calculateLightInteractionsOnZone(const Building_Zone &zone);
    bool calculateWindowInteractionsOnZone(const Building_Zone &zone);
    double calculateExternalShadeInteractionsOnZone(const Building_Zone &zone);
    double calculateMetabolicHeatGainsOnZone(const Building_Zone &zone);
    double getPMV(const Building_Zone &zone) const;

    bool heatState; //!< the desired heating state
    double power; //!< power of occupant, used in negotiations
    double metabolicRate; //!< Metabolic Rate of the occupant
    double clo; //!< clothing value of the occupant
    std::string buildingName; //!< building agent belongs to
    std::string bedroom; //!< Which bedroom the occupant sleeps in
    std::string office; //!< Which Office the occupant works in
    std::vector<Occupant_Zone> agentZones;
    std::vector<double> activities;
    std::vector<int> availableActions;
    std::shared_ptr<Building_Zone> zonePtrPrevious;
    std::shared_ptr<Building_Zone> zonePtr;
    State state; //!< Occupants current state
    StateMachine stateMachine; //!< transitions occupant between states

    int datastoreIdActivity;
    int datastoreIdHeatGains;
};

#endif  // OCCUPANT_H_
