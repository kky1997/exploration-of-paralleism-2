Kai Koo

Data was collected by using the three scripts (script_1, script_2, and script_3) which ran the histogram program with constant bin_count, min_measurment, and max_measurment inputs; only the
data_count (data set size) and number of processes is different. Each script handles a different number of processes:

script_1.sh = 1 process
script_2.sh = 2 processes
script_3.sh = 4 processes

The scripts pkill -f "histogram" and also sleep for 1 second between each execution of the histogram program. This is to ensure that there is absolutely no interferences between executions
of the histogram program.
----------------------------------------
Please note:

I am aware that the pkill -f "histogram" script command may not work on the Uni system as you may not have the privilege required to kill a process. The scripts will work either way, they may 
just output an error, but this won't affect functionality. I am leaving the pkill command in just to demonstrate that this would 100% ensure no interference between executions.

Additionally, if you do not have privilege to run the scripts, please run "chmod u+x" in linux command line to give access.
----------------------------------------

The data collected is split into 3 files, one for each of the number of processes run using MPI.

runtimep1.txt - contains the results of running histogram.c with 1 process with data_counts of 1024, 2048, 4096, 8192, and 16384

runtimep2.txt - contains the results of running histogram.c with 2 process with data_counts of 1024, 2048, 4096, 8192, and 16384

runtimep4.txt - contains the results of running histogram.c with 4 process with data_counts of 1024, 2048, 4096, 8192, and 16384


Within each file will be groups of run times.

eg.

Run 1:
Number of processes = 1
Time = 3.647804e-05 seconds
Number of processes = 1
Time = 7.200241e-05 seconds
Number of processes = 1
Time = 1.590252e-04 seconds
Number of processes = 1
Time = 3.659725e-04 seconds
Number of processes = 1
Time = 5.745888e-04 seconds

There are 5 run-times here to show the program was run with the 5 different data set sizes. In each file (runtimep1, runtimep2, and runtimep4) there will be 5 groups of 5 runtimes labelled
run 1, 2, 3, 4 and 5.

The runs indicate the results gathered each run of the script (scripts were run 5 times to flatten variance of runtime due to users/programs on the system).

that means that each file (runtimep1, runtimep2, and runtimep4) will have 5 "runs" and those 5 "runs" will be a group of 5 run-times for each data set size.