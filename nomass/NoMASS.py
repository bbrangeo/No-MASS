import glob
import logging
import multiprocessing
import os
import platform
import random
import stat
import subprocess
import time
import xml.etree.ElementTree as ET
from multiprocessing import Lock
from shutil import copyfile, copytree, rmtree

import colorlog
import pandas as pd

lock = Lock()


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
        self.xmlFiles = False
        self.seed = -1
        self.printInput = False
        self.epsilon = 0.1
        self.alpha = 0.1
        self.gamma = 0.1
        self.start = time.time()

    # Configuration du logger
    @staticmethod
    def setup_logging(level: int = logging.DEBUG):
        """
        Configures the logging system to display messages with specific formats and
        colors defined for different logging levels. This method sets up a colored
        console handler using `colorlog` with a format that includes the timestamp,
        log level, and the message. Optionally, a file handler can be uncommented
        to log messages to a file.

        Attributes:
            level (int): Optional logging level; defaults to `logging.DEBUG`.

        Args:
            level (int): The logging level to set. Common values include:
                - `logging.DEBUG` for detailed debugging information.
                - `logging.INFO` for general informational messages.
                - `logging.WARNING` for warnings.
                - `logging.ERROR` for error messages.
                - `logging.CRITICAL` for critical issues.

        Raises:
            None
        """

        if len(logging.getLogger().handlers) == 0:  # Évite les duplications de handlers
            handler = colorlog.StreamHandler()
            handler.setFormatter(
                colorlog.ColoredFormatter(
                    "%(log_color)s%(asctime)s - %(levelname)s - %(message)s",
                    log_colors={
                        "DEBUG": "cyan",
                        "INFO": "green",
                        "WARNING": "yellow",
                        "ERROR": "red",
                        "CRITICAL": "bold_red",
                    },
                )
            )
            logging.basicConfig(
                level=level,  # Niveau de détail
                format="%(asctime)s - %(levelname)s - %(message)s",
                handlers=[
                    handler,
                    # logging.FileHandler("simulation.log"),  # Enregistre dans un fichier
                ],
            )

    def deleteLearningData(self):
        """ """
        ll = self.learntDataLocation()
        if os.path.exists(ll):
            rmtree(ll)
            # self.resultsLocation + "NoMASS.out.hdf"

    def printConfiguration(self):
        """ """
        logging.debug("Run location: {}".format(self.runLocation))
        logging.debug("locationOfNoMASS: {}".format(self.locationOfNoMASS))
        logging.debug("configurationDirectory: {}".format(self.configurationDirectory))
        logging.debug("resultsLocation: {}".format(self.resultsLocation))
        logging.debug("printInput: {}".format(self.printInput))
        logging.debug("numberOfSimulations: {}".format(self.numberOfSimulations))
        logging.debug("Learning: {}".format(self.learn))

        con = self.configurationDirectory
        tree = ET.parse(os.path.join(con, self.simulationFile))
        root = tree.getroot()
        for buildings in root.findall("buildings"):
            for building in buildings.findall("building"):
                for agents in building.findall("agents"):
                    agentcount = 0
                    for agent in agents.findall("agent"):
                        agentcount = agentcount + 1
                    logging.debug("Number of agents {}".format(agentcount))
                for apps in building.findall("Appliances"):
                    logging.debug(
                        "Number of large appliances {}".format(
                            len(apps.findall("Large"))
                        )
                    )
                    logging.debug(
                        "Number of small appliances {}".format(
                            len(apps.findall("Small"))
                        )
                    )
                    logging.debug(
                        "Number of pv appliances {}".format(len(apps.findall("pv")))
                    )
                    if apps.findall("Grid"):
                        logging.debug("Grid enabled")

    def simulate(self):
        """ """
        self.start = time.time()

        ll = os.path.join(self.resultsLocation, f"{self.outFile }.hdf")
        if os.path.exists(ll):
            os.remove(ll)

        if self.printInput:
            self.printConfiguration()

        for x in range(0, self.numberOfSimulations):
            if x % 25 == 1:
                elapsed = time.time() - self.start
                logging.debug(f"Simulation: {x} Time: {elapsed:.2f} seconds")

            self.copyToRunLocation(x)
            self.makeExecutable(x)
            self.configuration(x)
            self.run(x)
            self.copyToResultsLocation(x)
            if self.clean:
                rmtree(self.runLoc(x))

        elapsed = time.time() - self.start
        logging.debug(f"Total Simulation Time: {elapsed:.2f}seconds")

    def simulation_task(self, x):
        """Fonction exécutée pour chaque simulation."""
        if x % 25 == 1:
            elapsed = time.time() - self.start
            logging.debug(f"Simulation: {x} Time: {elapsed:.2f} seconds")
        self.copyToRunLocation(x)
        self.makeExecutable(x)
        self.configuration(x)
        self.run(x)
        self.copyToResultsLocation(x)

        if self.clean:
            rmtree(self.runLoc(x))

    def simulate_parallel(self, num_processes=4):
        """Lance les simulations en parallèle."""
        self.setup_logging()  # Configurer le logger

        if self.printInput:
            self.printConfiguration()

        logging.info(
            f"Starting simulations: {self.numberOfSimulations} tasks with {num_processes} processes."
        )

        self.start = time.time()  # Démarrage du chronomètre
        with multiprocessing.Pool(processes=num_processes) as pool:
            # Préparer les arguments pour chaque simulation
            args = [x for x in range(0, self.numberOfSimulations)]
            # Lancer les simulations en parallèle
            pool.map(self.simulation_task, args)

        self.createHDFFiles()

        elapsed = time.time() - self.start
        logging.info(f"All simulations completed. Total time: {elapsed:.2f} seconds.")

    def learning(self):
        """
        This method retrieves the value of the 'learn' attribute associated with the
        object. The purpose is to provide external access to the learning data stored
        within the class instance.

        Returns:
            Any: The current value of the 'learn' attribute.
        """
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

        outfileStr = "NoMASS-" + str(x).zfill(5) + ".out"
        copyfile(
            os.path.join(self.runLoc(x), self.outFile), os.path.join(rl, outfileStr)
        )

        # self.createPandasFiles(x, os.path.join(self.runLoc(x), self.outFile))

        if self.xmlFiles:
            outfileStr = "SimulationConfig-" + str(x).zfill(5) + ".xml"
            copyfile(
                os.path.join(self.runLoc(x), self.simulationFile),
                os.path.join(rl, outfileStr),
            )

        if self.learning():
            for f in glob.glob(os.path.join(self.runLoc(x), self.appLearningFile)):
                path = os.path.dirname(f)
                filename = os.path.basename(f)
                copyfile(f, os.path.join(rl, filename + "." + str(x).zfill(5)))
                ll = self.learntDataLocation()
                copyfile(f, os.path.join(ll, filename))

    def runLoc(self, x):
        logging.debug(
            "runLoc =>",
            os.path.join(self.runLocation, self.simulationLocation + str(x)),
        )
        return os.path.join(self.runLocation, self.simulationLocation + str(x)) + "/"

    def learntDataLocation(self):
        if self.learntData == "":
            self.learntData = os.path.join(self.resultsLocation, "learningdata")
            try:
                if not os.path.exists(self.learntData):
                    os.makedirs(self.learntData)
            except FileExistsError as e:
                print(e)
        return self.learntData

    def copyToRunLocation(self, x):
        rl = self.runLoc(x)
        if not os.path.exists(rl):
            os.makedirs(rl)

        logging.debug(
            "copyToRunLocation =>",
            os.path.join(self.locationOfNoMASS, self.NoMASSstr),
            os.path.join(rl, self.NoMASSstr),
        )
        copyfile(
            os.path.join(self.locationOfNoMASS, self.NoMASSstr),
            os.path.join(rl, self.NoMASSstr),
        )
        con = self.configurationDirectory
        copyfile(
            os.path.join(con, self.simulationFile),
            os.path.join(rl + self.simulationFile),
        )
        copyfile(
            os.path.join(con, self.activityFile), os.path.join(rl + self.activityFile)
        )
        copyfile(
            os.path.join(con, self.learningXMLFile),
            os.path.join(rl + self.learningXMLFile),
        )
        copyfile(
            os.path.join(con, self.largeApplianceFile),
            os.path.join(rl + self.largeApplianceFile),
        )
        copyfile(
            os.path.join(con, self.HeatingPowerFile),
            os.path.join(rl + self.HeatingPowerFile),
        )
        copyfile(os.path.join(con, self.PVFile), os.path.join(rl, self.PVFile))

        for x in self.appFiles:
            copyfile(os.path.join(con, x), os.path.join(rl, x))

        if os.path.isdir(os.path.join(rl, self.smallApplianceFolder)):
            rmtree(os.path.join(rl, self.smallApplianceFolder))

        copytree(
            os.path.join(con, self.smallApplianceFolder),
            os.path.join(rl, self.smallApplianceFolder),
        )

        ll = self.learntDataLocation()
        if self.learning():
            if not os.path.exists(ll):
                os.makedirs(ll)

        for f in glob.glob(os.path.join(ll, self.appLearningFile)):
            path = os.path.dirname(f)
            filename = os.path.basename(f)
            copyfile(f, os.path.join(rl, filename))

    def makeExecutable(self, x: int) -> None:
        rl = self.runLoc(x)
        nomassexe = os.path.join(rl, self.NoMASSstr)
        st = os.stat(nomassexe)
        os.chmod(nomassexe, st.st_mode | stat.S_IEXEC)

    def run(self, x: int) -> None:
        rl = self.runLoc(x)
        if platform.system() == "Windows":
            self.NoMASSstr = self.NoMASSstr + ".exe"
            p = subprocess.Popen(self.NoMASSstr, cwd=rl)
        else:
            p = subprocess.Popen("./" + self.NoMASSstr, cwd=rl)
        p.communicate()

    def createPandasFiles(self, x, filename):
        if self.pandasFiles:
            if not os.path.exists(filename):
                raise FileNotFoundError(f"The file {filename} does not exist.")
            print("createPandasFiles =>", filename)
            a = pd.read_csv(filename)
            a["nsim"] = x
            with lock:  # Verrouiller l'accès au fichier
                a.to_hdf(
                    path_or_buf=os.path.join(
                        self.resultsLocation, f"{self.outFile }.hdf"
                    ),
                    key="file%i" % x,
                    mode="a",
                )

    def createHDFFiles(self):
        # Motif pour trouver tous les fichiers NoMASS*
        pattern = os.path.join(self.resultsLocation, "NoMASS-*.out")

        # Récupérer tous les fichiers correspondant au motif
        files = glob.glob(pattern)
        for x, file in enumerate(files):
            a = pd.read_csv(file)
            a["nsim"] = x
            a.to_hdf(
                path_or_buf=os.path.join(self.resultsLocation, f"{self.outFile}.hdf"),
                key="file%i" % x,
                mode="a",
            )
            os.remove(file)  # Supprimer le fichier temporaire après fusion

    def getPandasDF(self, nrows=None) -> pd.DataFrame:
        ad = pd.DataFrame()
        with pd.HDFStore(
            path=os.path.join(self.resultsLocation, "NoMASS.out.hdf"), mode="r"
        ) as store:
            for j in range(0, self.numberOfSimulations):
                a = store.get("file%i" % j)
                ad = pd.concat([ad, a], ignore_index=True)

        return ad

    def simulationConfigInfoDF(self):
        """
        Generates a DataFrame containing configuration information for a simulation.

        The function builds a DataFrame that summarizes the appliance configurations
        for each building in the XML structure provided in `self.root`. This includes
        various types of appliances such as large appliances, small appliances, shiftable
        appliances, photovoltaic (PV) systems, battery systems, and CSV-based appliances.
        The resulting DataFrame `self.simulationConfigInfo` has detailed information regarding
        each building's appliance setup.

        Parameters:
            None

        Returns:
            pandas.DataFrame: A DataFrame containing columns for each type of appliance
            configuration, where each row corresponds to a building.

        Raises:
            None
        """

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


if __name__ == "__main__":
    toot = NoMASS()
