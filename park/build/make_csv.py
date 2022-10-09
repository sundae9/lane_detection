import datetime as dt
import sys

DST_PREFIX = "../result/"
title = dt.datetime.now().strftime("%Y-%m-%d-%H-%M") + ".csv"
content = sys.stdin.read()

with open(DST_PREFIX+title, 'w') as file:
    file.write(content)

print("file {} saved".format(title))