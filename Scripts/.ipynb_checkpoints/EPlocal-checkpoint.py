import subprocess
from shutil import copyfile, copytree, rmtree, move
import random
from time import sleep
import sys, os
import xml.etree.ElementTree as ET
from configuration import *

runLocation = sys.argv[2]
run = int(sys.argv[1])
sims = int(sys.argv[3])

'''
print runLocation
'''
configLocations = configurations()

currentRunSim = -1
currentRun = {}
runnableLocations = []
count = 0;
for x in configLocations:
    if x['run']:
        runnableLocations.append(x)

'''
print runnableLocations
'''
stop = False
bot = 0
top = 0
totalsim = sims * len(runnableLocations)
for x in runnableLocations:
    if stop:
        break
    sim = 0
    for y in x['sims']:
        bot = sims * count
        top = sims * (count +1)
        if (run <= top) and (run > bot):
            currentRun = x
            currentRunSim = sim
            stop = True
            break
        sim = sim + 1
        count = count + 1
'''
print currentRun['name']
print currentRun['sims'][currentRunSim]
'''



location = runLocation +'/' + currentRun['sims'][currentRunSim] + str(run) +'/'

currentResults = resultsFolder + "/" + currentRun['name'] + "/" + currentRun['sims'][currentRunSim]
if 'outFolder' in currentRun:
    currentResults = resultsFolder + "/" + currentRun['name'] + "/" + currentRun['outFolder']

if not os.path.isdir(location):
    copytree(home +'/EnergyPlus/',location)

make_sure_path_exists(resultsFolder)
make_sure_path_exists(currentResults)

configLocation = home + "/ConfigurationFiles/" + currentRun['sims'][currentRunSim]
xmlFile = location+'tmp-fmus/agentFMU.fmu_FMI/SimulationConfig.xml'
copyfile(configLocation + '/in.idf', location+'in.idf' )
copyfile(configLocation + '/in.epw', location+'in.epw' )
copyfile(configLocation + '/SimulationConfig.xml',xmlFile)
if os.path.isfile(configLocation + '/Activity.xml'):
    copyfile(configLocation + '/Activity.xml',location+'tmp-fmus/agentFMU.fmu_FMI/Activity.xml')
copyfile(configLocation + '/modelDescription.xml', location+'tmp-fmus/agentFMU.fmu_FMI/modelDescription.xml' )
make_sure_path_exists(location+'tmp-fmus/agentFMU.fmu_FMI/binaries/linux64/')
copyfile(configLocation + '/FMU/binaries/linux64/FMI.so', location+'tmp-fmus/agentFMU.fmu_FMI/binaries/linux64/FMI.so' )


tree = ET.parse(xmlFile)
root = tree.getroot()
random.seed()
root.find('seed').text = str(run*random.randint(1,999))
if root.find('case') is not None:
    root.find('case').text = currentRun['case']

for buildings in root.findall('buildings'):
    for building in buildings.findall('building'):
        for agents in building.findall('agents'):
            agentcount = 0;
            for agent in agents.findall('agent'):
                agent.find('window').text = currentRun['agents'][agentcount]['window']
                agent.find('shade').text = currentRun['agents'][agentcount]['shade']
                if agent.find('age') is not None :
                    agent.find('age').text = currentRun['agents'][agentcount]['age']
                    agent.find('computer').text = currentRun['agents'][agentcount]['computer']
                    agent.find('civstat').text = currentRun['agents'][agentcount]['civstat']
                    agent.find('unemp').text = currentRun['agents'][agentcount]['unemp']
                    agent.find('retired').text = currentRun['agents'][agentcount]['retired']
                    agent.find('edtry').text = currentRun['agents'][agentcount]['edtry']
                    agent.find('famstat').text = currentRun['agents'][agentcount]['famstat']
                    agent.find('sex').text = currentRun['agents'][agentcount]['sex']

                agent.find('power').text = str(currentRun['agents'][agentcount]['power'])

                agentcount = agentcount +1



tree.write(xmlFile)


p = subprocess.Popen(location+'EnergyPlus', cwd=location)
p.communicate()

readlocation = runLocation + '/ReadVars' + currentRun['sims'][currentRunSim] + str(run) +'/'
if not os.path.isdir(readlocation):
    copytree(home + '/ReadVars/',readlocation)

move(location+'eplusout.eso', readlocation + '/eplusout.eso')

copyfile(xmlFile, currentResults + '/SimulationConfig'+ str(run) +'.xml')

periods = ['runperiod','timestep','monthly']

for j in periods:
    for i in configLocations:
        with open(readlocation + 'myfile.rvi', 'w+') as f:
            f.write(readlocation + 'eplusout.eso\n')
            f.write(currentResults +'/'+ j +  str(run) + '.csv\n')
            for x in i:
                f.write(x+'\n')
            f.write('0\n')

        p = subprocess.Popen([readlocation+'ReadVarsESO', readlocation+'myfile.rvi', j], cwd=readlocation)
        p.communicate()

rmtree(location)
rmtree(readlocation)
