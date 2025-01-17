// Copyright 2016 Jacob Chapman

#ifndef OCCUPANT_ACTION_H_
#define OCCUPANT_ACTION_H_

#include <vector>
#include "Building_Zone.hpp"

/**ww
 * @brief Occupant action super class
 * @details Occupant action super class
 */

class Occupant_Action {
 public:
  Occupant_Action();

  void setAvailableActivities(const std::vector<int> &availableActivities);
  void setZoneId(const double zoneId);
  void setReward(const double reward);
  double getResult() const;
  double getFutureDurationOfAbsenceState(
                    const std::vector<double> &activities) const;
  double getPreviousDurationOfAbsenceState(
                    const std::vector<double> &activities) const;
  double getCurrentDurationOfPresenceState(
                    const std::vector<double> &activities) const;

 protected:
  bool activityAvailable(const int act) const;
  double result;
  double reward;
  std::vector<int> availableActivities;
  int zoneId;
};
#endif  // OCCUPANT_ACTION_H_
