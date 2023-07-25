#!/bin/bash
echo -e "10\n1\n100\n1024" | mpiexec -n 2 ./histogram
pkill -f 'histogram'
sleep 1
echo -e "10\n1\n100\n2048" | mpiexec -n 2 ./histogram
pkill -f 'histogram'
sleep 1
echo -e "10\n1\n100\n4096" | mpiexec -n 2 ./histogram
pkill -f 'histogram'
sleep 1
echo -e "10\n1\n100\n8192" | mpiexec -n 2 ./histogram
pkill -f 'histogram'
sleep 1
echo -e "10\n1\n100\n16384" | mpiexec -n 2 ./histogram