// Copyright 2015 Jacob Chapman

#ifndef DATASTORE_H_
#define DATASTORE_H_

#include <unordered_map>
#include <vector>
#include <string>

class DataStore {
 public:
    static int addVariable(const std::string &name);
    static int getID(const std::string &name);
    static void addValueS(const std::string &name, const double val);
    static void addValue(const int & id, const double val);
    static double getValueS(const std::string &name);
    static double getValue(const int & id);
    static double getValueForZone(const std::string &name, const std::string &zoneName);
    static void print();
    static void clear();
 private:
    DataStore();
    static std::unordered_map<std::string, int> variableMap;
    static std::unordered_map<int, std::vector<float>> intMap;
    static int variableCount;
};

#endif  // DATASTORE_H_
