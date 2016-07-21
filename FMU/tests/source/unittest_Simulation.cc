// Copyright 2015 Jacob Chapman

#include <limits.h>

#include "Gen.h"
#include "Simulation.h"
#include "SimulationConfig.h"
#include "DataStore.h"

#include "gtest/gtest.h"

TEST(Simulation, HeatGainsOnly) {
  DataStore::clear();
  SimulationConfig::reset();
  SimulationConfig::RunLocation  = testFiles;
  Simulation s;
  s.parseConfiguration(SimulationConfig::RunLocation  + "/SimulationConfig.xml");
  SimulationConfig::info.windows = false;
  SimulationConfig::info.shading = false;
  SimulationConfig::info.lights = false;

  s.setupSimulationModel();

  s.preTimeStep();
  DataStore::addVariable("EnvironmentSiteOutdoorAirDrybulbTemperature");
  DataStore::addValue("EnvironmentSiteOutdoorAirDrybulbTemperature", 21);
  s.timeStep();
  EXPECT_EQ(SimulationConfig::getStepCount(), 0);
  int activity = DataStore::getValue("Occupant_Activity_1");
  EXPECT_EQ(activity, 9);
  s.postTimeStep();
  s.postprocess();

  DataStore::addValue("EnvironmentSiteOutdoorAirDrybulbTemperature", 21);
  DataStore::addVariable("Block1:Zone1ZoneMeanAirTemperature");
  DataStore::addVariable("Block1:Zone1ZoneAirRelativeHumidity");
  DataStore::addVariable("Block1:Zone1ZoneMeanRadiantTemperature");
  DataStore::addValue("Block1:Zone1ZoneMeanAirTemperature", 21);
  DataStore::addValue("Block1:Zone1ZoneAirRelativeHumidity", 21);
  DataStore::addValue("Block1:Zone1ZoneMeanRadiantTemperature", 21);
  for (int i =1; i< 10000; i++) {
    s.preTimeStep();
    s.timeStep();
    EXPECT_EQ(SimulationConfig::getStepCount(), i);
    int activity = DataStore::getValue("Occupant_Activity_1");

    s.postTimeStep();
    if (activity < 9) {
      EXPECT_EQ(activity, 3);
      ASSERT_NEAR(DataStore::getValue("AgentGains1"), 77.046341167, 0.001);
      ASSERT_NEAR(DataStore::getValue("Occupant_Metabolic_Rate_1"), 70, 0.001);
      ASSERT_NEAR(DataStore::getValue("Occupant_clo_1"), 1, 0.001);
      ASSERT_NEAR(DataStore::getValue("Occupant_ppd_1"), 7.835778219685593, 0.001);
      ASSERT_NEAR(DataStore::getValue("Occupant_pmv_1"), -0.36908214203982, 0.001);
    }
  }
  s.postprocess();
}


TEST(Simulation, HeatGainsWindowsOnly) {
  DataStore::clear();
  SimulationConfig::reset();
  SimulationConfig::RunLocation  = testFiles;
  Simulation s;

  s.parseConfiguration(SimulationConfig::RunLocation  + "/SimulationConfig.xml");
  SimulationConfig::info.shading = false;
  SimulationConfig::info.lights = false;
  SimulationConfig::info.windows = true;
  s.setupSimulationModel();


  DataStore::addVariable("Block1:Zone1ZoneMeanAirTemperature");
  DataStore::addVariable("Block1:Zone1ZoneAirRelativeHumidity");
  DataStore::addVariable("Block1:Zone1ZoneMeanRadiantTemperature");
  DataStore::addValue("Block1:Zone1ZoneMeanAirTemperature", 21);
  DataStore::addValue("Block1:Zone1ZoneAirRelativeHumidity", 21);
  DataStore::addValue("Block1:Zone1ZoneMeanRadiantTemperature", 21);
  DataStore::addValue("EnvironmentSiteOutdoorAirDrybulbTemperature", 18);
  DataStore::addValue("EnvironmentSiteRainStatus", 0);

  s.preTimeStep();
  s.timeStep();
  EXPECT_EQ(SimulationConfig::getStepCount(), 0.0);
  int WindowState = DataStore::getValue("Block1:Zone1WindowState0");
  EXPECT_EQ(WindowState, 0);
  s.postTimeStep();
  s.postprocess();

  DataStore::addVariable("Block1:Zone1ZoneMeanAirTemperature");
  DataStore::addVariable("Block1:Zone1ZoneAirRelativeHumidity");
  DataStore::addVariable("Block1:Zone1ZoneMeanRadiantTemperature");
  DataStore::addVariable("EnvironmentSiteOutdoorAirDrybulbTemperature");
  DataStore::addVariable("EnvironmentSiteRainStatus");
  DataStore::addValue("Block1:Zone1ZoneMeanAirTemperature", 21);
  DataStore::addValue("Block1:Zone1ZoneAirRelativeHumidity", 21);
  DataStore::addValue("Block1:Zone1ZoneMeanRadiantTemperature", 21);
  DataStore::addValue("EnvironmentSiteOutdoorAirDrybulbTemperature", 18);
  DataStore::addValue("EnvironmentSiteRainStatus", 0);

  int i = 1;
  for ( ; i < 100; i++) {
    s.preTimeStep();
    s.timeStep();
    EXPECT_EQ(SimulationConfig::getStepCount(), i);
    WindowState = DataStore::getValue("Block1:Zone1WindowState0");
    // EXPECT_EQ(WindowState, 0);
    s.postTimeStep();
  }

  DataStore::addValue("Block1:Zone1ZoneMeanAirTemperature", 28);
  DataStore::addValue("Block1:Zone1ZoneAirRelativeHumidity", 100);
  DataStore::addValue("Block1:Zone1ZoneMeanRadiantTemperature", 100);
  DataStore::addValue("EnvironmentSiteOutdoorAirDrybulbTemperature", 15);
  DataStore::addValue("EnvironmentSiteRainStatus", 0);

  for (i =100;; i++) {
    s.preTimeStep();
    s.timeStep();
    EXPECT_EQ(SimulationConfig::getStepCount(), i);
    WindowState = DataStore::getValue("Block1:Zone1WindowState0");
    int occs = DataStore::getValue("Block1:Zone1NumberOfOccupants");
    if (occs > 0) {
      //  EXPECT_EQ(WindowState, 1);
      break;
    }
    s.postTimeStep();
  }

  s.postprocess();
}
