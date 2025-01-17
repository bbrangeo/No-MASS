// Copyright 2016 Jacob Chapman

#ifndef OCCUPANT_ACTION_WINDOW_H_
#define OCCUPANT_ACTION_WINDOW_H_

#include "Occupant_Action.hpp"
#include "Model_Windows.hpp"
#include "Building_Zone.hpp"

/**
 * @brief Occupant action on windows
 * @details Occupant action on windows
 */

class Occupant_Action_Window : public Occupant_Action {
 public:
  Occupant_Action_Window();
  void setup(int windowID, int id);
  void setDailyMeanTemperature(double dailyMeanTemperature);
  void saveResult() const;
  int durationOpen() const;

 protected:
  Model_Windows m_window;
  int variableNameWindowDesire;
  double dailyMeanTemperature;
};

#endif  // OCCUPANT_ACTION_WINDOW_H_
