#!/bin/bash

output_file1="cache_ipc.dat"
#output_file2="cache_misses.dat"
echo "----hits---" >> $output_file1
#echo "----misses---" >> $output_file2

echo "lru     plru   ipv"  >> $output_file1
echo " -------------------------------" >> $output_file1

#echo "lru     plru   ipv"  >> $output_file2
#echo " -------------------------------" >> $output_file2
for file in ./benchmarks/*.eio.gz 

#for file in ./benchmarks/gcc.eio.gz 
do
	echo "" >> $output_file1
#	echo "" >> $output_file2
 # by a.k: 'l'for LRU, 'i' for ipv, p for PLRU
	for conf_file in "conf_lru" "conf_plru"  "conf_ipv" 
	do
		./sim-outorder -config $conf_file $file
 #result of the command is saved by default to a file named "output"

#		echo "   $pred      $result" >> $output_file 
		result1=$(cat output | grep IPC  |awk '{print $2}') 
#		result2=$(cat output | grep ul2.misses |awk '{print $2}') 
		echo -n "$result1     " >> $output_file1
		#echo -n "$result2     " >> $output_file2
	done
	echo -n "    benchmark: $file " >> $output_file1
 #	echo -n "    benchmark: $file " >> $output_file2
done

