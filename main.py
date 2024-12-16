import os

from nomass.NoMASS import NoMASS

home = "."

nomassLearn = NoMASS()

nomassLearn.runLocation = "./Simulation/"
nomassLearn.locationOfNoMASS = "./FMU/build/"
nomassLearn.configurationDirectory = os.path.join(home, "./Configuration/Experiment3/")
nomassLearn.resultsLocation = "./ResultsLearning/"
nomassLearn.PVFile = "PV_single_profile.csv"
nomassLearn.epsilon = 0.1
nomassLearn.alpha = 0.1
nomassLearn.gamma = 0.1
nomassLearn.printInput = True
nomassLearn.pandasFiles = True
nomassLearn.numberOfSimulations = 2  # 300
nomassLearn.learn = True
nomassLearn.learningXMLFile = "SimulationConfig.xml"
nomassLearn.simulate()
df = nomassLearn.getPandasDF()
nomassLearn.deleteLearningData()


print(df.head(10))
