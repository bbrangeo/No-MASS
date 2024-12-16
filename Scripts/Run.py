import os

from nomass.NoMASS import NoMASS

home = "."

### NoMAss learning
# Have nomass learn for a number of simulations and save the results

nomassLearn = NoMASS()

nomassLearn.runLocation = "./Simulation/"
nomassLearn.locationOfNoMASS = "../FMU/build/"
nomassLearn.configurationDirectory = os.path.join(home, "../Configuration/Experiment3/")
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
nomassLearn.deleteLearningData()  # clear any previsouly learnt data


print(df.head(10))
### Have Nomass simulate useing the learnt data
# Using the learnt data simulate some resutls for comparison
# set epsilon to 0 so no randomness takes place
# set learn to false so no longer updates the learning q-table
# set learntData to the location of the learnt values

# nomass = NoMASS()
# nomass.runLocation = "../FMU/build/Simulation/"
# nomass.locationOfNoMASS = "../FMU/build/"
# nomass.configurationDirectory = home +"/git/No-MASSDSM/Configuration/Experiment3/"
#
# nomass.resultsLocation = "../FMU/build/ResultsLearnt/"
# nomass.PVFile = "PV_single_profile.csv"
# nomass.printInput = True
# nomass.PandasFiles = True
# nomass.numberOfSimulations = 50
# nomassLearn.epsilon = 0.0
# nomass.learn = False
# nomass.learntData = nomassLearn.learntDataLocation()
# nomass.simulate()
