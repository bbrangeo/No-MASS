import csv
import os
directory = "/Volumes/Disk2/Dropbox/HPC/results/Office/learn/"

all = []

for file in os.listdir(directory):
    if file.endswith(".csv"):
        with open(directory+file, 'rb') as csvfile:
          spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
          count = 0
          Learn_state1 = -1
          Learn_previous_state1 = -1
          Learn_state0 = -1
          Learn_action0 = -1
          Learn_reward1 = -1
          Learn_reward0 = -1
          Learn_previous_state0 = -1
          Learn_action1 = -1
          for row in spamreader:
            if count == 0:
                count = count + 1
                for x,y in enumerate(row):
                    if y == "Learn_state1":
                        Learn_state1 = x
                    if y == "Learn_previous_state1":
                        Learn_previous_state1 = x
                    if y == "Learn_state0":
                        Learn_state0 = x
                    if y == "Learn_action0":
                        Learn_action0 = x
                    if y == "Learn_reward1":
                        Learn_reward1 = x
                    if y == "Learn_reward0":
                        Learn_reward0 = x
                    if y == "Learn_previous_state0":
                        Learn_previous_state0 = x
                    if y == "Learn_action1":
                        Learn_action1 = x
                continue
            r1 = row[Learn_reward0]
            r2 = row[Learn_reward1]
            if r1 == '1.9735e-316':
                r1 = '0'
            if r2 == '1.9735e-316':
                r2 = '0'
            if not r1 == "":
                a1 = [row[Learn_previous_state0],row[Learn_state0],row[Learn_action0],r1]
                all.append(a1)
            if not r2 == "":
                a2 = [row[Learn_previous_state1],row[Learn_state1],row[Learn_action1],r2]
                all.append(a2)
            #print "action: " + str(row[2]) + " reward: " + str(r1)



with open('test.csv', 'w') as fp:
    a = csv.writer(fp, delimiter=',')
    a.writerows(all)
