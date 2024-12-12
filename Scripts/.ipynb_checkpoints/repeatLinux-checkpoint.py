import subprocess
from shutil import copyfile, copytree, rmtree
import random
from time import sleep
import sys, os
import multiprocessing
from multiprocessing import Process, Lock

import xml.etree.ElementTree as ET

run = 0

sims = 1

configVariables = []
configLocations = []
configname= []
#Output:Reports, Schedules, Timestep;
#Output:Variable,*,Schedule Value,Timestep;

configVariables.append('Zone Air System Sensible Heating Energy')
configVariables.append('Zone Air System Sensible Cooling Energy')
configVariables.append('Zone People Occupant Count')
configVariables.append('Zone Mean Air Temperature')
configVariables.append('Daylighting Reference Point 1 Illuminance')
configVariables.append('Site Exterior Horizontal Sky Illuminance')
configVariables.append('Zone Exterior Windows Total Transmitted Beam Solar Radiation Rate')
configVariables.append('Window Shading Fraction')
configVariables.append('AFN Surface Venting Window or Door Opening Factor')
configVariables.append('Zone Thermal Comfort Fanger Model PMV')
configVariables.append('Zone Thermal Comfort Fanger Model PPD')
configVariables.append('BLOCK1:ZONE1WINDOWSTATE,Schedule Value')
configVariables.append('BLOCK1:ZONE1LIGHTSTATE,Schedule Value')
configVariables.append('BLOCK1:ZONE1HEATING,Schedule Value')
configLocations.append(configVariables)
configname.append('')


#configLocations.append(['BLOCK1:ZONE1BLINDSTATE,Schedule Value'])
#configname.append('shadestate')

periods = ['runperiod','timestep','monthly']

def makeEP():
    location = home + '/git/masplatform/EnergyPlusBuild/'
    p = subprocess.Popen(['/usr/bin/make','-j4'], cwd=location)
    p.communicate()
    location2 = home + '/git/EnergyPlus/EnergyPlus'
    copyfile(location+'Products/EnergyPlus', location2)

    location = home + '/git/masplatform/AgentSimulationFMI/'
    p = subprocess.Popen(['/usr/bin/make','-j4'], cwd=location)
    p.communicate()


def runEP(run):
    location = home + '/git/EnergyPlus' + str(run) +'/'
    if os.path.isdir(location):
        rmtree(location)

    copytree(home + '/git/EnergyPlus/',location)

    xmlFile = location + 'tmp-fmus/agentFMU.fmu_FMI/SimulationConfig.xml'
    if os.path.exists(xmlFile):
        tree = ET.parse(xmlFile)
        root = tree.getroot()
        if root.find('seed') is not None :
            root.find('seed').text = str(run * random.randint(0,10000))
        if root.find('learnep') is not None :
            root.find('learnep').text = str(ep)
        tree.write(xmlFile)


    p = subprocess.Popen(location+'EnergyPlus', cwd=location)
    p.communicate()

    results = "/media/sf_git/results/GENOfficeLearn2"
    if not os.path.exists(home + "/git/results"):
        os.makedirs(home + "/git/results")

    copyfile(location+'eplusout.eso', results + "/eplusout" + str(run)+'.eso')
    #copyfile(location+'eplusout.err', results + "/git/results/eplusout" + str(run)+'.err')
    #copyfile(location+'eplusout.rdd', results + "/git/results/eplusout" + str(run)+'.rdd')
    copyfile(location+'rlearning0.dat', results + "/rlearning0-" + str(run)+'.dat')
    copyfile(location+'rlearning1.dat', results + "/rlearning1-" + str(run)+'.dat')
    copyfile(location+'rlearning0.dat', home + "/git/EnergyPlus/rlearning0.dat")
    copyfile(location+'rlearning1.dat', home + "/git/EnergyPlus/rlearning1.dat")
    #if os.path.exists(location+'eplusout.edd'):
    #    copyfile(location+'eplusout.edd', home + "/git/results/eplusout" + str(run)+'.edd')

    copyfile(location+'agent.csv', results + "/agent" + str(run)+'.csv')


    confignamecount = 0
    ''''''
    for i in configLocations:
        for j in periods:
            name = configname[confignamecount]
            name = name.replace(" ", "")

            f = open(location+'myfile.rvi','w')
            f.write(location+'eplusout.eso\n')

            f.write(results + '/'+ name + j +  str(run) + '.csv\n') # python will convert \n to os.linesep

            for x in i:
                f.write(x+'\n') # python will convert \n to os.linesep
            f.write('0\n') # python will convert \n to os.linesep
            f.close() # you can omit in most cases as the destructor will call if
            p = subprocess.Popen([location+'ReadVarsESO', location+'myfile.rvi', j], cwd=location)
            p.communicate()
        confignamecount = confignamecount +1
    ''''''
    rmtree(location)


def runmulti(tragetMethod, cpus, count):
    threads = []
    for proc in range(0,cpus):
        p = Process(target = tragetMethod, args = ([count]))
        p.start()
        threads.append(p)
        count = count + 1
    for t in threads:
        t.join()

ep = 1

home = "/Volumes/Home"
home = "/home/jake"
cpus = 1
count = 1

runs = 1
make = False

if make:
    if EnergyPlus:
        makeEP()

for j in range(0,10):

    for i in range(0,runs):
        runmulti(runEP, cpus, count)
        count = count + cpus
    break
