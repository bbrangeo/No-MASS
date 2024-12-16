import glob
import os
import random
import stat
import subprocess
import time
import xml.etree.ElementTree as ET
from shutil import copyfile, copytree, rmtree

import pandas as pd


class NoMASS(object):
    """ """

    def __init__(self):
        self.runLocation = ""
        self.simulationLocation = "simulation"
        self.locationOfNoMASS = ""
        self.NoMASSstr = "NoMASS"
        self.configurationDirectory = ""
        self.simulationFile = "SimulationConfig.xml"
        self.activityFile = "Activity.xml"
        self.largeApplianceFile = "AppliancesLarge.xml"
        self.learningXMLFile = "learning.xml"
        self.HeatingPowerFile = "HeatingPower.csv"
        self.PVFile = "PVBowler2013_365.csv"
        self.appFiles = []
        self.smallApplianceFolder = "SmallAppliances"
        self.resultsLocation = "Results"
        self.outFile = "NoMASS.out"
        self.appLearningFile = "*.dat"
        self.numberOfSimulations = 1
        self.learn = False
        self.learntData = ""
        self.clean = True
        self.pandasFiles = False
        self.outFiles = False
        self.xmlFiles = False
        self.seed = -1
        self.printInput = False
        self.epsilon = 0.1
        self.alpha = 0.1
        self.gamma = 0.1
        self.start = time.time()

    def deleteLearningData(self):
        """ """
        ll = self.learntDataLocation()
        if os.path.exists(ll):
            rmtree(ll)
            # self.resultsLocation + "NoMASS.out.hdf"

    def printConfiguration(self):
        """ """
        print("Run location: {}".format(self.runLocation))
        print("locationOfNoMASS: {}".format(self.locationOfNoMASS))
        print("configurationDirectory: {}".format(self.configurationDirectory))
        print("resultsLocation: {}".format(self.resultsLocation))
        print("printInput: {}".format(self.printInput))
        print("numberOfSimulations: {}".format(self.numberOfSimulations))
        print("Learning: {}".format(self.learn))
        con = self.configurationDirectory
        tree = ET.parse(con + self.simulationFile)
        root = tree.getroot()
        for buildings in root.findall("buildings"):
            for building in buildings.findall("building"):
                for agents in building.findall("agents"):
                    agentcount = 0
                    for agent in agents.findall("agent"):
                        agentcount = agentcount + 1
                    print("Number of agents {}".format(agentcount))
                for apps in building.findall("Appliances"):
                    print(
                        "Number of large appliances {}".format(
                            len(apps.findall("Large"))
                        )
                    )
                    print(
                        "Number of small appliances {}".format(
                            len(apps.findall("Small"))
                        )
                    )
                    print("Number of pv appliances {}".format(len(apps.findall("pv"))))
                    if apps.findall("Grid"):
                        print("Grid enabled")

    def simulate(self):
        """ """
        self.start = time.time()
        ll = os.path.join(self.resultsLocation, "NoMASS.out.hdf")
        if os.path.exists(ll):
            os.remove(ll)
        if self.printInput:
            self.printConfiguration()
        for x in range(0, self.numberOfSimulations):
            if x % 25 == 1:
                elapsed = time.time() - self.start
                print("Simulation: %i Time: %02d seconds" % (x, elapsed))
            self.copyToRunLocation(x)
            self.makeExecutable(x)
            self.configuration(x)
            self.run(x)
            self.copyToResultsLocation(x)
            if self.clean:
                rmtree(self.runLoc(x))
        elapsed = time.time() - self.start
        print("Total Simulation Time: %02d seconds" % elapsed)

    def learning(self):
        """ """
        return self.learn

    def configuration(self, x):
        """ """
        rl = self.runLoc(x)
        tree = ET.parse(rl + self.simulationFile)
        root = tree.getroot()
        if self.seed < 0:
            random.seed()
        else:
            random.seed(self.seed)
        s = str(random.randint(1, 99999))
        root.find("seed").text = s

        for buildings in root.findall("buildings"):
            for building in buildings.findall("building"):
                for apps in building.findall("Appliances"):
                    for ll in apps.findall("battery"):
                        if ll.find("updateQTable") is not None:
                            if self.learning():
                                ll.find("updateQTable").text = str(1)
                            else:
                                ll.find("updateQTable").text = str(0)
                        else:
                            newNode = ET.Element("updateQTable")
                            newNode.text = "1"
                            ll.append(newNode)
                        if ll.find("epsilon") is not None:
                            ll.find("epsilon").text = str(self.epsilon)
                        else:
                            newNode = ET.Element("epsilon")
                            newNode.text = str(self.epsilon)
                            ll.append(newNode)
                        if ll.find("alpha") is not None:
                            ll.find("alpha").text = str(self.alpha)
                        else:
                            newNode = ET.Element("alpha")
                            newNode.text = str(self.alpha)
                            ll.append(newNode)
                        if ll.find("gamma") is not None:
                            ll.find("gamma").text = str(self.gamma)
                        else:
                            newNode = ET.Element("gamma")
                            newNode.text = str(self.gamma)
                            ll.append(newNode)
                    for ll in apps.findall("LargeLearning"):
                        if ll.find("updateQTable") is not None:
                            if self.learning():
                                ll.find("updateQTable").text = str(1)
                            else:
                                ll.find("updateQTable").text = str(0)
                        else:
                            newNode = ET.Element("updateQTable")
                            newNode.text = "1"
                            ll.append(newNode)
                        if ll.find("epsilon") is not None:
                            ll.find("epsilon").text = str(self.epsilon)
                        else:
                            newNode = ET.Element("epsilon")
                            newNode.text = str(self.epsilon)
                            ll.append(newNode)
                        if ll.find("alpha") is not None:
                            ll.find("alpha").text = str(self.alpha)
                        else:
                            newNode = ET.Element("alpha")
                            newNode.text = str(self.alpha)
                            ll.append(newNode)
                        if ll.find("gamma") is not None:
                            ll.find("gamma").text = str(self.gamma)
                        else:
                            newNode = ET.Element("gamma")
                            newNode.text = str(self.gamma)
                            ll.append(newNode)
        tree.write(rl + self.simulationFile)
        self.root = root

    def copyToResultsLocation(self, x):
        rl = self.resultsLocation
        if not os.path.exists(rl):
            os.makedirs(rl)
        if self.outFiles:
            outfileStr = "NoMASS-" + str(x).zfill(5) + ".out"
            copyfile(self.runLoc(x) + self.outFile, rl + outfileStr)
        self.createPandasFiles(x, self.runLoc(x) + self.outFile)
        if self.xmlFiles:
            outfileStr = "SimulationConfig-" + str(x).zfill(5) + ".xml"
            copyfile(self.runLoc(x) + self.simulationFile, rl + outfileStr)
        if self.learning():
            for f in glob.glob(self.runLoc(x) + self.appLearningFile):
                path = os.path.dirname(f)
                filename = os.path.basename(f)
                copyfile(f, rl + filename + "." + str(x).zfill(5))
                ll = self.learntDataLocation()
                copyfile(f, ll + filename)

    def runLoc(self, x):
        return self.runLocation + self.simulationLocation + str(x) + "/"

    def learntDataLocation(self):
        if self.learntData == "":
            self.learntData = self.resultsLocation + "learningdata/"
            if not os.path.exists(self.learntData):
                os.makedirs(self.learntData)
        return self.learntData

    def copyToRunLocation(self, x):
        rl = self.runLoc(x)
        if not os.path.exists(rl):
            os.makedirs(rl)
        copyfile(
            os.path.join(self.locationOfNoMASS, self.NoMASSstr), rl + self.NoMASSstr
        )
        con = self.configurationDirectory
        copyfile(os.path.join(con, self.simulationFile), rl + self.simulationFile)
        copyfile(os.path.join(con, self.activityFile), rl + self.activityFile)
        copyfile(os.path.join(con, self.learningXMLFile), rl + self.learningXMLFile)
        copyfile(
            os.path.join(con, self.largeApplianceFile), rl + self.largeApplianceFile
        )
        copyfile(os.path.join(con, self.HeatingPowerFile), rl + self.HeatingPowerFile)
        copyfile(os.path.join(con, self.PVFile), rl + self.PVFile)
        for x in self.appFiles:
            copyfile(con + x, rl + x)
        if os.path.isdir(rl + self.smallApplianceFolder):
            rmtree(rl + self.smallApplianceFolder)
        copytree(con + self.smallApplianceFolder, rl + self.smallApplianceFolder)
        ll = self.learntDataLocation()
        if self.learning():
            if not os.path.exists(ll):
                os.makedirs(ll)
        for f in glob.glob(ll + self.appLearningFile):
            path = os.path.dirname(f)
            filename = os.path.basename(f)
            copyfile(f, rl + filename)

    def makeExecutable(self, x):
        rl = self.runLoc(x)
        nomassexe = rl + self.NoMASSstr
        st = os.stat(nomassexe)
        os.chmod(nomassexe, st.st_mode | stat.S_IEXEC)

    def run(self, x):
        rl = self.runLoc(x)
        p = subprocess.Popen("./" + self.NoMASSstr, cwd=rl)
        p.communicate()

    def createPandasFiles(self, x, filename):
        if self.pandasFiles:
            if not os.path.exists(filename):
                raise FileNotFoundError(f"The file {filename} does not exist.")
            a = pd.read_csv(filename)
            a["nsim"] = x
            a.to_hdf(
                path_or_buf=os.path.join(self.resultsLocation, "NoMASS.out.hdf"),
                key="file%i" % x,
                mode="a",
            )

    def getPandasDF(self, nrows=None) -> pd.DataFrame:
        ad = pd.DataFrame()
        with pd.HDFStore(
            path=os.path.join(self.resultsLocation, "NoMASS.out.hdf")
        ) as store:
            for j in range(0, self.numberOfSimulations):
                a = store.get("file%i" % j)
                ad = pd.concat([ad, a], ignore_index=True)

        return ad

    def simulationConfigInfoDF(self):
        """ """

        def getIDList(ApplianceElement, keyword):
            theList = []
            for element in ApplianceElement.iter(keyword):
                theList.append(int(element.find("id").text))
            return theList

        self.simulationConfigInfo = pd.DataFrame(
            columns=[
                "Building",
                "LargeApplianceList",
                "SmallApplianceList",
                "ShiftApplianceList",
                "PVList",
                "BatteryList",
                "csvList",
            ]
        )
        buildingNumber = 0
        for build in self.root.iter("building"):
            for ap in build.iter("Appliances"):
                largeApplianceList = getIDList(ap, "Large") + getIDList(ap, "LargeCSV")
                smallApplianceList = getIDList(ap, "Small")
                shiftApplianceList = getIDList(ap, "LargeLearning") + getIDList(
                    ap, "LargeLearningCSV"
                )
                pvList = getIDList(ap, "pv")
                batteryList = getIDList(ap, "battery")
                batteryList_cont = getIDList(ap, "batteryGridCostReward")
                csvApplianceList = getIDList(ap, "csv")

            self.simulationConfigInfo.loc[buildingNumber] = [
                buildingNumber,
                largeApplianceList,
                smallApplianceList,
                shiftApplianceList,
                pvList,
                batteryList + batteryList_cont,
                csvApplianceList,
            ]
            buildingNumber += 1
        return self.simulationConfigInfo

        #     i=0
        # for b in root.iter('building'):
        #     print(b.tag , i)
        #     i+=1
        #     for ap in b.iter('Appliances'):
        #         for a in ap.getchildren():
        #             print('\t'+ a.tag + " - id: " + a.find('id').text)

    def simulationConfigInfoDF_occupancy(self):
        """
        Only used for information on occupancy profiles. Gives a lit of buildings and the number of occupants in them.
        :return:
        """
        self.simulationConfigInfo = pd.DataFrame(
            columns=["Building", "numberOccupants"]
        )
        buildingNumber = 0
        for build in self.root.iter("building"):
            number_agents = len(list(build.find("agents")))
            self.simulationConfigInfo.loc[buildingNumber] = [
                buildingNumber,
                number_agents,
            ]
            buildingNumber += 1
        return self.simulationConfigInfo
