#!/bin/bash

for ((var=0; var < $1; var++));
do
	./park > ../result/tmp/test$var.txt
	echo "tc $var completed"
done

echo $1 | python3 make_csv.py
