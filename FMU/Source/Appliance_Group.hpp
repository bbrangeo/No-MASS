// Copyright 2016 Jacob Chapman

#ifndef APPLIANCE_GROUP_H_
#define APPLIANCE_GROUP_H_

#include <vector>
#include <algorithm>
#include "Appliance.hpp"
#include "DataStore.hpp"
#include "Utility.hpp"
#include "Contract.hpp"
#include "Contract_Negotiation.hpp"

/**
 * @brief Groups the types appliances for easy handling
 * @details
 */
template <class T>
class Appliance_Group {

 public:
    Appliance_Group(){};

    void setup(const std::vector<ConfigStructAppliance> & app,
                                    const int & buildingID,
                                    const std::string & buildingString) {
      //for (const ConfigStructAppliance s : app) {
      for (const ConfigStructAppliance& s : app) {
	appliances.push_back(T());
        appliances.back().setBuildingID(buildingID);
        appliances.back().setIDString(buildingString + std::to_string(s.id));
        appliances.back().setup(s);
        appliances.back().setupSave();
      }
      setupSave();
    }

    void hasActivities(const std::vector<int> & Activities) {
      for (T & a : appliances) {
        a.hasActivities(Activities);
      }
    }

    void shuffleAppliances() {
      std::shuffle(appliances.begin(), appliances.end(), Utility::engine);
    }

    virtual void stepApp(T & a, Contract_Negotiation * app_negotiation) {
      a.step();
      bool local = sendContractLocal(a, app_negotiation);
      a.setLocal(local);
      a.setGlobal(false);
    }

    void step(Contract_Negotiation * app_negotiation) {
      shuffleAppliances();
      for (T & a : appliances) {
        stepApp(a, app_negotiation);
      }
    }

    bool sendContractLocal(const T & a, Contract_Negotiation * app_negotiation) {
      bool ret = (a.getPower() > 0 || a.getSupply() > 0);
      if (ret) {
        Contract c;
        c.id = a.getID();
        c.buildingID = a.getBuildingID();
        c.requested = a.getPower();
        c.priority = a.getPriority();
        c.supplied = a.getSupply();
        c.suppliedLeft = a.getSupplyLeft();
        c.suppliedCost = a.getSupplyCost();
        c.receivedCost = a.getReceivedCost();
        c.received = a.getReceived();
        app_negotiation->submit(c);
      }
      return ret;
    }

    virtual void negotiationApp(const Contract_Negotiation & app_negotiation,
                                      T & app, const bool negotiate) {
        double received = app.getReceived();
        double receivedCost = app.getReceivedCost();
        double suppliedLeft = app.getSupplyLeft();
        if (negotiate) {
          int appid = app.getID();
          int buildingID = app.getBuildingID();
          Contract c = app_negotiation.getContract(buildingID, appid);
          app.setGlobal(sendContractGlobal(c));
          received = c.received;
          receivedCost = c.receivedCost;
          suppliedLeft = c.suppliedLeft;
        }
        app.setReceived(received);
        app.setReceivedCost(receivedCost);
        app.setSupplyLeft(suppliedLeft);
    }

    void localNegotiation(const Contract_Negotiation & app_negotiation) {
      for (T & app : appliances) {
        negotiationApp(app_negotiation, app, app.isLocal());
        app.saveLocal();
      }
    }

    void neighbourhoodNegotiation(const Contract_Negotiation & building_negotiation) {
      for (T & app : appliances) {
        negotiationApp(building_negotiation, app, app.isGlobal());
        app.saveNeighbourhood();
      }
    }

    virtual void negotiationAppGlobal(const Contract_Negotiation & app_negotiation,
                                      T & app, const bool negotiate) {
        negotiationApp(app_negotiation, app, negotiate);
    }

    void globalNegotiation(const Contract_Negotiation & building_negotiation) {
      for (T & app : appliances) {
        negotiationAppGlobal(building_negotiation, app, app.isGlobal());
        app.saveGlobal();
        app.save();
      }
      if (!appliances.empty()) {
        DataStore::addValue(datastoreIDSupplied, getSupply());
        DataStore::addValue(datastoreIDSuppliedCost, getSupplyCost());
        DataStore::addValue(datastoreIDReceived, getReceived());
        DataStore::addValue(datastoreIDRequested, getPower());
        DataStore::addValue(datastoreIDCost, getReceivedCost());
      }
    }

    void clear() {
      for (T & app : appliances) {
        app.clear();
      }
    }

    virtual bool sendCondition(const Contract & c){
      return (c.requested > c.received || c.suppliedLeft > 0);
    }

    bool sendContractGlobal(const Contract & c) {
      bool send = sendCondition(c);
      if (send) {
        globalContracts.push_back(c);
      }
      return send;
    }

    void postprocess() {
      for (T & l : appliances) {
        l.postprocess();
      }
    }

    void addGlobalContactsTo(Contract_Negotiation * building_negotiation) {
      for (const Contract & c : globalContracts) {

        building_negotiation->submit(c);
      }
      globalContracts.clear();
    }


    double getSupply() const {
      double sum = 0;
      for (const T & l : appliances) {
        sum += l.getSupply();
      }
      return sum;
    }

    double getSupplyLeft() const {
      double sum = 0;
      for (const T & l : appliances) {
        sum += l.getSupplyLeft();
      }
      return sum;
    }

    double getSupplyCost() const {
      double sum = 0;
      for (const T & l : appliances) {
        sum += l.getSupplyCost();
      }
      return sum;
    }

    double getPower() const {
      double sum = 0;
      for (const T & l : appliances) {
        sum += l.getPower();
      }
      return sum;
    }

    double getReceived() const {
      double sum = 0;
      for (const T & l : appliances) {
        sum += l.getReceived();
      }
      return sum;
    }

    double getReceivedCost() const {
      double sum = 0;
      for (const T & l : appliances) {
        sum += l.getReceivedCost();
      }
      return sum;
    }

    T getApplianceAt(int BuildingID, int id){
      T app;
      for (T a : appliances) {
        if (a.getID() == id && a.getBuildingID() == BuildingID){
          app = a;
        }
      }
      return app;
    }

    void setIDString(const std::string id){
      idString = id;
    }

 protected:
   std::vector<T> appliances;
   std::vector<Contract> globalContracts;
   std::string idString;

   void setupSave() {
     if (!appliances.empty()) {
       datastoreIDSupplied = DataStore::addVariable(idString + "_supplied");
       datastoreIDSuppliedCost = DataStore::addVariable(idString + "_suppliedCost");
       datastoreIDReceived = DataStore::addVariable(idString + "_received");
       datastoreIDRequested = DataStore::addVariable(idString + "_requested");
       datastoreIDCost = DataStore::addVariable(idString + "_cost");
     }
   }

   int datastoreIDSupplied;
   int datastoreIDSuppliedCost;
   int datastoreIDReceived;
   int datastoreIDRequested;
   int datastoreIDCost;
};

#endif  // APPLIANCE_GROUP_H_
