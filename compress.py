import os
import sys
import string
import subprocess

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("2 arguments are required.")

cmd="make clean; make MACRO_INPUT='-DPREXION_MATCH_PROB=" + sys.argv[2] + "' all"
os.system(cmd)

cwd=os.getcwd()

cmd="./markers_to_brs " + sys.argv[1]
page_size = subprocess.run(cmd.split(' '), stdout=subprocess.PIPE, text=True).stdout.rstrip()

cmd="./gen_prexion_rand_nums " + page_size
print(cmd)
os.system(cmd)

cmd="./minigzip " + sys.argv[1] + ".nomarkers"
print(cmd)
os.system(cmd)