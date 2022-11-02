#!/bin/bash

for ((vara=0;vara <= $2;vara++));
do
	mkdir ../result/tmp/proc$vara
done

for ((var=0; var < $1; var++));
do
	
	./main $var > ../result/tmp/test$var.txt
	echo "tc $var completed"
done

echo $1 | python3 make_csv.py
