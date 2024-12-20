import logging
import os
from shutil import rmtree

from nomass.NoMASS import NoMASS

if __name__ == "__main__":

    current_directory = os.getcwd()
    print("Le répertoire de travail actuel est :", current_directory)

    result_output = "ExampleResult"
    if os.path.exists(result_output):
        rmtree(result_output)

    nomassLearn = NoMASS()
    nomassLearn.NoMASSstr = "NoMASS"
    nomassLearn.setup_logging(level=logging.INFO)

    nomassLearn.runLocation = os.path.join(
        current_directory, result_output, "Simulation"
    )

    nomassLearn.locationOfNoMASS = os.path.join(current_directory, "FMU", "build")

    nomassLearn.configurationDirectory = os.path.join(
        current_directory, "Configuration", "Experiment4"
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
    nomassLearn.clean = True

    # TODO : faire une function bool pour remove files
    # nomassLearn.removeFiles =True

    nomassLearn.numberOfSimulations = 4  # 300
    nomassLearn.learn = True
    nomassLearn.learningXMLFile = "SimulationConfig.xml"

    # MONO PROCESS
    # nomassLearn.simulate()

    # MULTI PROCESS
    nomassLearn.simulate_parallel(num_processes=4)

    # RESULTATS
    df = nomassLearn.getPandasDF()

    # REMOVE
    # nomassLearn.deleteLearningData()

    print(df.head(10))
    df.to_csv(os.path.join(result_output, "results.csv"))
