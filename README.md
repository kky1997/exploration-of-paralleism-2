# exploration-of-paralleism-2
Exploration of parallelism - Analysis of a histogram program, modified with the MPI library to achieve parallelism.

This is an exploration of parallelism as well as a performance analysis of a histogram program written in C, parallelised using Foster's PCAM method, using the MPI library. The provided report gives more contexts as well as methodolgy, results, analysis, and conclusion.

Runtime results were gathered using 1, 2, and 4 parallel (simultaneous) processes.

The histogram.c program has been modified to give runtime output using MPI functions for both time collection and synchronization. The internal C code of histogram has been modified to redirect output to a corresponding text file in the perfdata folder.

Overview of findings were that the histogram program was "weakly scalable", meaning both speedup and efficiency of the program could be effectively maintained only if the data set size was increased as number of processes/threads was increased.

To run:

    1. Ensure you have the MPI librarly set up as well as a C compiler capable of compling MPI.
    2. compile the histogram.c

        $ mpicc -g -Wall -o histogram histogram.c

    Now you can run the histogram executable manually and specify what parameters you require for the histogram

        $ mpiexec -n <comm_sz> ./histogram 

        where <comm_sz> is the number of processes/threads started

    Or run the provided bash scripts which will automatically start the histogram executable with predetermined data set sizes as well as parameters for the histogram.

Note that data already exists in the data collection text files in /perfdata, so running the program with either 1, 2, or 4 processes/threads will just append more
runtime data to the respective file.

Running the program with a number of processes/threads that is not 1, 2, or 4, will give an error, however the histogram.c program can be modified from line 160 to change this.




