#! /usr/bin/python

import os

def run_tests():
    for i in range(3):
        for j in range(3):
            run_cmd = "../test1 --ps_freq " + str(i) + " --pl_freq " + str(j)
            print(">>> " + run_cmd)
            os.system(run_cmd)

if(not os.path.exists("rpts")):
    os.system("mkdir rpts");

os.chdir("rpts")

run_tests()
