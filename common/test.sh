#!/bin/bash

for ((var=0; var < $1; var++));
do
	./main > ../result/tmp/test$var.txt
	echo "tc $var completed"
done

python3 make_csv.py
