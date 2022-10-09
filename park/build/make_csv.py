import datetime as dt
from glob import glob
import sys

DST_PREFIX = "../result/"
SRC = "../result/tmp/*"
subject = ["min (ms)", "max (ms)", "avg", "min-frame-idx", "max-frame-idx"]
tasks=["thresholding", "roi", "canny", "hough line", "draw line", "total"]

title = dt.datetime.now().strftime("%Y-%m-%d-%H-%M-%S") + ".csv"
file_list = glob(SRC)
tc = len(file_list)
content = []

for file in file_list:
    f = open(file, 'r')
    tmp = f.read().split('\n')[:-1]
    tmp2 = []
    for i in tmp:
        tmp2.append(i.split(', '))
    content.append(list(zip(*tmp2)))
    f.close()

with open(DST_PREFIX + title, 'w') as file:
    file.write(", ," + ','.join(subject) + "\n")

    for k in range(len(tasks)):
        file.write(tasks[k])
        for j in range(tc):
            file.write(",#{}, ".format(j+1))
            for i in range(len(subject)):
                file.write(content[j][i][k]+",")
            file.write("\n")
        file.write("\n")

print("file {} saved".format(title))
