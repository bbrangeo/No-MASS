import os
import subprocess
import xml.etree.ElementTree as ET
from shutil import copyfile

numberOfSimulations = 2
locaitonOfEnergyPlus = "C:\EnergyPlusV8-3-0"

configVariables = []
configLocations = []
configname= []

#Output Variables to save
configVariables.append('Zone Air System Sensible Heating Energy')
configVariables.append('Zone Air System Sensible Cooling Energy')
configVariables.append('Zone People Occupant Count')
configVariables.append('Zone Mean Air Temperature')
configVariables.append('Daylighting Reference Point 1 Illuminance')
configVariables.append('Site Exterior Horizontal Sky Illuminance')
configVariables.append('Zone Exterior Windows Total Transmitted Beam Solar Radiation Rate')
configVariables.append('Window Shading Fraction')
configVariables.append('AFN Surface Venting Window or Door Opening Factor')
configVariables.append('BLOCK1:ZONE1WINDOWSTATE,Schedule Value')
configVariables.append('BLOCK1:ZONE1LIGHTSTATE,Schedule Value')
configLocations.append(configVariables)
configname.append('All')

periods = ['runperiod','timestep','monthly']




def runEP(run):
    print("running simulation" + str(run))
    location = locaitonOfEnergyPlus

    xmlFile = location + 'tmp-fmus/agentFMU.fmu_FMI/SimulationConfig.xml'
    if os.path.exists(xmlFile):
        tree = ET.parse(xmlFile)
        root = tree.getroot()
        root.find('seed').text = str(run * 989)
        tree.write(xmlFile)


    p = subprocess.Popen(location+'/EnergyPlus.exe', cwd=location)
    p.communicate()

    if not os.path.exists(location + "/results"):
        os.makedirs(location + "/results")

    copyfile(location+'/eplusout.eso', location + "/results/eplusout" + str(run)+'.eso')
    copyfile(location+'/eplusout.err', location + "/results/eplusout" + str(run)+'.err')
    copyfile(location+'/eplusout.rdd', location + "/results/eplusout" + str(run)+'.rdd')
    if os.path.exists(location+'/eplusout.edd'):
        copyfile(location+'/eplusout.edd', location + "/results/eplusout" + str(run)+'.edd')


    confignamecount = 0
    for i in configLocations:
        for j in periods:
            name = configname[confignamecount]
            name = name.replace(" ", "")

            f = open(location+'/myfile.rvi','w')
            f.write(location + "/results/eplusout" + str(run)+'.eso\n')

            f.write(location + '/results/'+ name + j +  str(run) + '.csv\n')
            for x in i:
                f.write(x+'\n') # python will convert \n to os.linesep
            f.write('0\n') # python will convert \n to os.linesep
            f.close() # you can omit in most cases as the destructor will call if
            p = subprocess.Popen([location+'/PostProcess/ReadVarsESO.exe', location+'/myfile.rvi', j], cwd=location)
            p.communicate()
        confignamecount = confignamecount +1



def runmulti(run):
    for proc in range(0,run):
        runEP(run)


runmulti(numberOfSimulations)
