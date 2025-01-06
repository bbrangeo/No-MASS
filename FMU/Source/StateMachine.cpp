// Copyright 2016 Jacob Chapman

#include <iostream>
#include "StateMachine.hpp"

StateMachine::StateMachine() {}

unsigned int StateMachine::numberOfStates() const {
  return states.size();
}

void StateMachine::addState(const State & s) {
  states.insert({s.getId(), s});
}

bool StateMachine::hasState(const int stateID) const {
  bool found = states.end() != states.find(stateID) ;
  if (!found) {
    for (const auto & s : states) {
      found = s.second.hasState(stateID);
      if (found) {
        break;
      }
    }
  }
  return found;
}

State StateMachine::transistionTo(const int stateID) const {
  std::unordered_map<int, State>::const_iterator si = states.find(stateID) ;
  State x = State();  // Assurez-vous que le constructeur par défaut produit un état cohérent
  // State x;

  if (states.end() == si && hasState(stateID)) {

    for (const auto & s : states) {
      if (s.second.hasState(stateID)) {
        x = s.second.getState(stateID);
        std::cout << "   ==> StateID trouvé via hasState : " << stateID << std::endl;
        break;
      }
    }
  }else{
    x = si->second;
    // x = states.at(stateID);
  }
  return x;
}
