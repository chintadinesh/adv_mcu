#! /usr/bin/python

import os

def run_tests():
    for i in range(300):
        run_cmd = "../intr_latency_ticks" 
        print(">>> " + run_cmd)
        os.system(run_cmd)

if(not os.path.exists("rpts")):
    os.system("mkdir rpts");

os.chdir("rpts")

run_tests()
