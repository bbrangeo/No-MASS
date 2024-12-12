from NoMASS import NoMASS

from os.path import expanduser
home = expanduser("~")

### NoMAss learnin
# Have nomass learn for a number of simulations and save the results

nomassLearn = NoMASS()

nomassLearn.runLocation = "../FMU/build/Simulation/"
nomassLearn.locationOfNoMASS = "../FMU/build/"
nomassLearn.configurationDirectory = home +"/git/No-MASSDSM/Configuration/Experiment3/"
nomassLearn.resultsLocation = "../FMU/build/ResultsLearning/"
nomassLearn.PVFile = "PV_single_profile.csv"
nomassLearn.epsilon = 0.1
nomassLearn.alpha = 0.1
nomassLearn.gamma = 0.1
nomassLearn.printInput = True
nomassLearn.PandasFiles = True
nomassLearn.numberOfSimulations = 50#300
nomassLearn.learn = True
nomassLearn.deleteLearningData() # clear any previsouly learnt data
nomassLearn.simulate()

### Have Nomass simulate useing the learnt data
# Using the learnt data simulate some resutls for comparison
# set epsilon to 0 so no randomness takes place
# set learn to false so no longer updates the learning q-table
# set learntData to the location of the learnt values

nomass = NoMASS()
nomass.runLocation = "../FMU/build/Simulation/"
nomass.locationOfNoMASS = "../FMU/build/"
nomass.configurationDirectory = home +"/git/No-MASSDSM/Configuration/Experiment3/"

nomass.resultsLocation = "../FMU/build/ResultsLearnt/"
nomass.PVFile = "PV_single_profile.csv"
nomass.printInput = True
nomass.PandasFiles = True
nomass.numberOfSimulations = 50
nomassLearn.epsilon = 0.0
nomass.learn = False
nomass.learntData = nomassLearn.learntDataLocation()
nomass.simulate()
