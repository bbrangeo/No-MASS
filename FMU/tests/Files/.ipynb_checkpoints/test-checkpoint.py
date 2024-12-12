"""

Created by Ana Sancho (user ezxas5)
on 06 Feb 2017 at 11:43.

"""

import pandas as pd

##### test Appliances
def checkRequestedReceivedRow(row,resultsDF):
    def checkIsEqual(buildingID, appID):
        isequal = (   resultsDF['Building%i_Appliance%i_requested'%(buildingID,appID)]
                   == resultsDF['Building%i_Appliance%i_received'%(buildingID,appID)]).all()
        return isequal
    buildingID = row.Building
    print('Building %i'%buildingID)
    print('\t Large')
    for appID in row.LargeApplianceList: print('\ta%i-'%appID, checkIsEqual(buildingID, appID))
    print('\t Small')
    for appID in row.SmallApplianceList: print('\ta%i-'%appID, checkIsEqual(buildingID, appID))
    print('\t csv')
    if not row.csvList: print('\tNo csv')
    else:
        for appID in row.csvList: print('\ta%i-'%appID, checkIsEqual(buildingID, appID))

def checkDemandRequestedReceived(resultsDF, configInfoDF):

    """
    Test that appliances receive what they request.
    :param resultsDF: Simulation result output. Can be obtained reading pandas dataframe, using NoMASS.py
    :param configInfoDF: Configuration info dataframe. Can be obtained using NoMASS.py
    :return: print(True or False.)
    """
    print('Checking that ApplianceX_requested == ApplianceX_received for all demand apppliances')
    configInfoDF.apply(lambda row: checkRequestedReceivedRow(row,resultsDF), axis=1)



##### Test BATTERY
def checkReceivedSuppliedRow(row,resultsDF):
    def checkIsOpposite(buildingID, appID):
        # check if any of [supplied * received] is different from zero
        isopposite = (resultsDF['Building%i_Appliance%i_received'%(buildingID,appID)] *
                      resultsDF['Building%i_Appliance%i_supplied'%(buildingID,appID)]).any()
        # return true if they are opposite
        return not(isopposite)
    buildingID = row.Building
    print('Building %i'%buildingID)
    print('\t Battery ')
    if not row.BatteryList: print('\tNo battery')
    else:
        for appID in row.BatteryList: print('\ta%i-'%appID, checkIsOpposite(buildingID, appID))

def checkBatteryReceivedSupplied(resultsDF, configInfoDF):
    """
    Check charge and discharge in every battery does not occur at same time.
    :param resultsDF: Simulation result output. Can be obtained reading pandas dataframe, using NoMASS.py
    :param configInfoDF: Configuration info dataframe. Can be obtained using NoMASS.py
    :return:
    """
    print('Checking that ApplianceX_received is opposite (in sign) to ApplianceX_supplied for all ' \
          'batteries. Charging and discharching cannot occur on same timesteps.')

    configInfoDF.apply(lambda row: checkReceivedSuppliedRow(row,resultsDF), axis=1)

def checkBatteryReceived_PVSuppliedRow(row,resultsDF):
    def checkPVIsGreater(buildingID, appID_bat, PVList):
        # check if all pv supply is greater than battery received
        isgreater = (resultsDF[['Building%i_Appliance%i_supplied'%(buildingID, pvID) for pvID in PVList]].sum(axis=1) >=
                    resultsDF['Building%i_Appliance%i_received'%(buildingID, appID_bat)]).all()
        # return true if they are all true
        return isgreater
    buildingID = row.Building
    print('Building %i'%buildingID)
    print('\t Battery ')
    if not row.BatteryList: print('\tNo battery')
    else:
        if not row.PVList: print('\tNo PV in building.')
        else:
            if len(row.PVList) > 1: print('\tWarning: more than 1 PV in building.')
#             appID_pv = row.PVList[0]
            for appID_bat in row.BatteryList:
                print('\tbat%i'%appID_bat, checkPVIsGreater(buildingID, appID_bat,row.PVList))

def checkBatteryReceived_PVSupplied(resultsDF, configInfoDF):
    """
    Test that if battery is charging with PV in same building, total supplied PV should always be greater than power received.
    This may need revision as it is not a general condition (if other PV availble).
    :param resultsDF: Simulation result output. Can be obtained reading pandas dataframe, using NoMASS.py
    :param configInfoDF: Configuration info dataframe. Can be obtained using NoMASS.py
    :return:
    """
    print('Checking that ApplianceX_received is always smaller than the total PV power supplied ' \
          'in the same building: sum(AppliancePV1_PV2_..._supplied), for all batteries.')
    configInfoDF.apply(lambda row: checkBatteryReceived_PVSuppliedRow(row,resultsDF), axis=1)


def checkBatteryReceived_PVSupplied_OthersRow(resultsDF, configInfoDF):
    """
    Test that the battery in buildings with battery and without PV, still charge, i.e. they power from other houses.
    :param resultsDF: Simulation result output. Can be obtained reading pandas dataframe, using NoMASS.py
    :param configInfoDF: Configuration info dataframe. Can be obtained using NoMASS.py
    :return:
    """
    print(" Checking that the battery in buildings with battery and without PV, still charge, " \
          "i.e. they power from other houses.")

    # Identify buildings WITH battery and WITHOUT PV
    configInfoDF['unmatchedBatts'] = [bool(row[1].BatteryList and not row[1].PVList) for row in configInfoDF.iterrows()]
    # For those buildins, check that the battery is still receiving power:
    for buildingID, row in configInfoDF[configInfoDF.unmatchedBatts].iterrows():
        print('Building %i'%buildingID)
        for appID_bat in row.BatteryList:
            isNotNull = (resultsDF['Building%i_Appliance%i_received'%(buildingID, appID_bat)]).any()
            print('\t Battery ')
            print('\tbat%i'%appID_bat, isNotNull)




# call functions
def runTest(resultsDF, configInfoDF):
    print('***APPLIANCES***')
    checkDemandRequestedReceived(resultsDF, configInfoDF)
    print('\n***BATTERIES***\n')
    checkBatteryReceivedSupplied(resultsDF, configInfoDF)
    print('\n\n')
    checkBatteryReceived_PVSupplied(resultsDF, configInfoDF)
    print('\n\n')
    checkBatteryReceived_PVSupplied_OthersRow(resultsDF, configInfoDF)

def main():
    # my code here
    #runTest()
    pass

if __name__ == "__main__":
    main()
