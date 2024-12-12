import os
import errno
import datetime
from time import sleep
import random

civstat = ["civstat1", "civstat2"]
sex = ["sex1", "sex2"]
unemp = ["unemp1", "unemp0"]
computer =["computer0", "computer1"]
age = ["age1", "age2", "age3"]
famstat = ["famstat0", "famstat1", "famstat2", "famstat3"]
edtry = ["edtry1", "edtry2", "edtry3"]
retired = ["retired0", "retired1"]


home = '/home/jacob/git'
home = '/home/jake/git'

resultsFolder = "/media/jake/2tb/results"
resultsFolder = home + "/results"

home = '/home/psxjc5/Dropbox/HPC/HPCFiles'
resultsFolder = home + "/../results"

def make_sure_path_exists(path):
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise


def outputs():
    configVariables = []
    configLocations = []
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
    return configLocations

def checkfile(file):
    while not os.path.isfile(file):
        sleep(20)
    while datetime.datetime.fromtimestamp(os.path.getctime(file)) > datetime.datetime.now()-datetime.timedelta(seconds=30):
        sleep(30)


def configurations():
    configLocations = []

    configLocations.append({
        'name': 'GENOffice',
        'run':True,
        'outFolder':'power1',
        'sims':[
                'BaseCaseGENOfficeAll1'
                ],
        'agents':[
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            }
        ]

        })




    configLocations.append({
        'name': 'GENOffice',
        'run':True,
        'outFolder':'power11',
        'sims':[
                'BaseCaseGENOfficeAll'
                ],
        'agents':[
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            }
        ]

        })

    configLocations.append({
        'name': 'GENOffice',
        'run':True,
        'outFolder':'power10',
        'sims':[
                'BaseCaseGENOfficeAll'
		],
        'agents':[
            {
            'power':1.0,
            'window': "3",#str(random.randint(1,28)),
            'shade': "3",#str(random.randint(1,23))
            },
            {
            'power':0.0,
            'window': "3",#str(random.randint(1,28)),
            'shade': "3",#str(random.randint(1,23))
            }
        ]

        })



    configLocations.append({
        'name': 'GENOffice',
        'run':True,
        'outFolder':'power100',
        'sims':[
                'BaseCaseGENOfficeAll3'
                ],
        'agents':[
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':0.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':0.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            }
        ]

        })


    configLocations.append({
        'name': 'GENOffice',
        'run':True,
        'outFolder':'power111',
        'sims':[
                'BaseCaseGENOfficeAll3'
                ],
        'agents':[
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            }
        ]

        })

    configLocations.append({
        'name': 'GENOffice',
        'run':True,
        'outFolder':'power166',
        'sims':[
                'BaseCaseGENOfficeAll3'
                ],
        'agents':[
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':0.6,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':0.6,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            }
        ]

        })

    configLocations.append({
        'name': 'GENOffice',
        'run':True,
        'outFolder':'power1000',
        'sims':[
                'BaseCaseGENOfficeAll4'
                ],
        'agents':[
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':0.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':0.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':0.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            }
        ]

        })


    configLocations.append({
        'name': 'GENOffice',
        'run':True,
        'outFolder':'power1444',
        'sims':[
                'BaseCaseGENOfficeAll4'
                ],
        'agents':[
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':0.4,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':0.4,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':0.4,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            }
        ]

        })

    configLocations.append({
        'name': 'GENOffice',
        'run':True,
        'outFolder':'power1111',
        'sims':[
                'BaseCaseGENOfficeAll4'
                ],
        'agents':[
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            },
            {
            'power':1.0,
            'window': str(random.randint(1,28)),
            'shade': str(random.randint(1,23))
            }
        ]

        })
    '''
    configLocations = []
    for i in range(1,28):
        configLocations.append({
            'name': 'GENOffice',
            'run':True,
            'outFolder':'power11y'+str(i),
            'sims':[
                    'BaseCaseGENOfficeAll'
                    ],
            'agents':[
                {
                'power':1.0,
                'window': str(i),
                'shade': str(random.randint(1,23))
                },
                {
                'power':1.0,
                'window': str(i),
                'shade': str(random.randint(1,23))
                }
            ]

            })
    '''
    return configLocations
