import os

from nomass.NoMASS import NoMASS

current_directory = os.getcwd()
print("Le répertoire de travail actuel est :", current_directory)

result_output = "ExampleResult"
nomassLearn = NoMASS()

nomassLearn.runLocation = os.path.join(current_directory, result_output, "Simulation")
nomassLearn.locationOfNoMASS = os.path.join(current_directory, "FMU", "build")
nomassLearn.configurationDirectory = os.path.join(
    current_directory, "Configuration", "Experiment3"
)
nomassLearn.resultsLocation = os.path.join(
    current_directory, result_output, "ResultsLearning"
)

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
