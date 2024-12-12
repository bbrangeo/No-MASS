// Copyright 2015 Jacob Chapman

#include <string>
#include <cstring>
#include <algorithm>
#include <cstddef>
#include "rapidxml_utils.hpp"
#include "rapidxml.hpp"

#include "Utility.hpp"
#include "DataStore.hpp"
#include "SimulationTime.hpp"
#include "Occupant_Action_HeatingSetPoints_Learning.hpp"

Occupant_Action_HeatingSetPoints_Learning::
                  Occupant_Action_HeatingSetPoints_Learning() :
                  file(Configuration::RunLocation + "learning.xml") {
    setPoint = 20;
    pmv = -1.0;
    result = 20;
}

void Occupant_Action_HeatingSetPoints_Learning::setFile(std::string file){
  this->file = file;
}

void Occupant_Action_HeatingSetPoints_Learning::print() {
  qlWeekDay.printQ();
  qlWeekEnd.printQ();
}
void Occupant_Action_HeatingSetPoints_Learning::reset() {
  qlWeekDay.reset();
  qlWeekEnd.reset();
}

void Occupant_Action_HeatingSetPoints_Learning::setup(const int id,
                                                      const int learn) {
  learnId = learn;
  agentId = id;
  std::string zoneIdStr = std::to_string(zoneId);

  qlWeekDay.setFilename("Weekday-" + zoneIdStr + "-");
  qlWeekEnd.setFilename("Weekend-" + zoneIdStr + "-");
  qlWeekDay.setStates(24 * 12);
  qlWeekEnd.setStates(24 * 12);
  qlWeekDay.setActions(20);
  qlWeekEnd.setActions(20);
  qlWeekDay.setId(id);
  qlWeekDay.setup();
  qlWeekEnd.setId(id);
  parseConfiguration(file);
  qlWeekEnd.setup();
  pmv_name =
  DataStore::addVariable("Weekday-" + zoneIdStr + "-_pmv" + std::to_string(id));
  step_name =
  DataStore::addVariable("Weekday-" + zoneIdStr
    + "-_steps" + std::to_string(id));
}

void Occupant_Action_HeatingSetPoints_Learning::step(const Building_Zone& zone,
                                                      const bool inZone) {
    int hour = SimulationTime::hour;

    if (inZone) {
      hasBeenInZone = true;
      setPoint = zone.getHeatingState();
      pmv = reward;
    }

    if (hour != previousHour) {
      reward = 0;
      steps = steps + 1;
      if (hasBeenInZone) {
        steps = 0;
      }

      bool x = (pmv > 0 && setPoint == 10);
      bool w = (pmv > 0 && setPoint > 10);
      bool y = (pmv < -0.5);
      bool z = (pmv >= -0.5 && pmv <= 0);
      bool a = setPoint == 10 && steps > 1;

      reward = hasBeenInZone * (1 * z + 0.1 * x - 0.1 * y - 0.1 * w) +
                (1-hasBeenInZone) * (a * 0.1 + (1-a) * -0.1);

      int day = SimulationTime::day + 1;
      int dayOfTheWeek = (day - 1) % 7;
      int state = getState();

      if (dayOfTheWeek < 5) {
        qlWeekDay.setState(state);
        qlWeekDay.setAction(setPoint-10);
        qlWeekDay.setReward(reward);
        qlWeekDay.learn();
        result = qlWeekDay.getAction() + 10;
      } else {
        qlWeekEnd.setState(state);
        qlWeekEnd.setAction(setPoint-10);
        qlWeekEnd.setReward(reward);
        qlWeekEnd.learn();
        result = qlWeekEnd.getAction() + 10;
      }

      DataStore::addValue(pmv_name, pmv);
      DataStore::addValue(step_name, steps);
      hasBeenInZone = false;
      previousHour = hour;
      setPoint = result;
    }
}

int Occupant_Action_HeatingSetPoints_Learning::getState() const {
  int month = SimulationTime::month - 1;
  int count = 0;
// MODIF BORIS
//  for (const auto v : stateMappings){
for (const auto& v : stateMappings) {
    if(std::find(v.begin(), v.end(), month) != v.end()){
      break;
    }
    count++;
  }
  int hourOfDay = SimulationTime::hourOfDay;
  int state = (count * 24) + hourOfDay;
  return state;
}



void Occupant_Action_HeatingSetPoints_Learning::parseConfiguration(
                                                  const std::string &filename) {
  namespace rx = rapidxml;
  rx::file<> xmlFile(filename.c_str());  // Default template is char
  rx::xml_document<> doc;
  doc.parse<0>(xmlFile.data());    // 0 means default parse flags
  rx::xml_node<> *node = doc.first_node("learning");
  while (node) {
    rx::xml_node<> *cnode = node->first_node();
    cnode = node->first_node();
    while (cnode) {
      if (std::strcmp(cnode->name(), "states") == 0) {
        rx::xml_node<> *snode = cnode->first_node();
        while (snode) {
          std::vector<int> x = Utility::csvToInt(snode->value());
          stateMappings.push_back(x);
          snode = snode->next_sibling();
        }
      }
      cnode = cnode->next_sibling();
    }
    node = node->next_sibling();
  }

  qlWeekDay.setStates(24 * stateMappings.size());
  qlWeekEnd.setStates(24 * stateMappings.size());
}
