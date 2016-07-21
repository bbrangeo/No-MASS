// Copyright 2016 Jacob Chapman
#ifndef FMU_SOURCE_Occupant_ACTION_LIGHTS_H_
#define FMU_SOURCE_Occupant_ACTION_LIGHTS_H_

#include <vector>
#include "Occupant_Action.h"

class Occupant_Action_Lights : public Occupant_Action {
 public:
    Occupant_Action_Lights();
    void step(const Building_Zone& zone, const bool inZone,
              const bool previouslyInZone,
              const std::vector<double> &activities);

};

#endif  // FMU_SOURCE_Occupant_ACTION_LIGHTS_H_
