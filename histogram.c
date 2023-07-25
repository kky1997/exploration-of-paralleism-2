/*
   Kai Koo a1739831

   modified histogram.c program using the MPI library to achieve parallelism

   This program has been modified so that it no longer outputs the final histogram to the console, rather it will take runtime measurments of this program
   and append those measurments to various data collection files. The file which this program will append to is determined by whether the program is started with
   1, 2 or 4 processes, this makes it easier to split runtime data into files specific to 1, 2 or 4 processes.

   As specified in the performance analysis specifications: "Your performance analysis is concerned only with the part of the supplied code that carries out the parallel histogram computation."
   As such, since the majority of the parellelisation is done in the Find_bins() method which calls the Which_bins() method (both used carry out histogram computation by determining which bin the randomly generated doubles should be placed in),
   the runtime measurments of this program measure how long the execution of those two functions take.

   MPI_Barrier() is used to synchronize all processes, not allowing any to proceeed until they are all ready to call Find_bins(), at which point a starting time (taken before the Find_bins() call) and
   a finish time (taken after the Find_bin() function is completed), are taken in order to calculate the total elapsed time. Timing is taken using the MPI function MPI_Wtime().

   Each process calculates its own local elapsed time, before MPI_Reduce() is called to get the MPI_MAX of all the local_elapsed times and send that back to process 0 so it can be output. The reason
   only the MAX local_elapsed time of all the processes is taken is because the entire program runs as fast as its slowest process, so we are only interested in the process which takes the longest and hence
   it's runtime is indicative of the entire programs runtime.

   Data is collected by running the program with constant bin_count, Mnimum_measurment, and Maximum_measurment. The only variable that changes is the data_count as this increases the dataset size and is
   the biggest factor in the runtime of the program.

   Constants:
      bin_count = 10
      min_measurment = 1
      maximum_measurment = 100

   Three bash scripts are provided which will run this histogram program with an increasing value for data_count (doubling each time). Each script is responsible for starting the program with either
   1, 2 or 4 processes. 

   The histogram program will need to be compiled before running the scripts, IMPORTANT - the histogram program will need to be compiled into a file called "histogram". This is because that is the name
   of the file called inside the scripts.

   Eg. script_1.sh will start the histogram program with 1 process, while script_2.sh will start the program with 2.

   Each script file will test the same values for data_count, only the number of processes started with MPI differs.

----------------------------------------------------------------------------------
   File:     prog3.1_mpi_histo.c

   Purpose:  Use MPI to implement a program that creates a histogram

   Compile:  mpicc -g -Wall -o histogram histogram.c
   Run:      mpiexec -n <comm_sz> ./histogram

   Input:    Number of bins
            Minimum measurement
            Maximum measurement
            Number of data items

   Output:   Histogram of the data.  If DEBUG flag is set, the data 
            generated.

   Notes:
   1.  The number of data must be evenly divisible by the number of processes
   2.  The program generates random floats x in the range
      
         min measurement <= x < max measurement

   3.  If i >= 1, the ith bin contains measurements x in the
      range

         bin_maxes[i-1] <= x < bin_maxes[i]

      Bin 0 will contain measurements x in the range

         min measurement <= x < bin_maxes[0]


   IPP:     Programming Assignment 3.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h> //include the string.h library so we can access the strlen() function

void Get_input(int* bin_count_p, float* min_meas_p, float* max_meas_p,
      int* data_count_p, int* local_data_count_p, int my_rank, 
      int comm_sz, MPI_Comm comm);
void Check_for_error(int local_ok, char fname[], char message[], 
      MPI_Comm  comm);
void Gen_data(float local_data[], int local_data_count, int data_count, 
      float min_meas, float max_meas, int my_rank, MPI_Comm comm);
void Set_bins(float bin_maxes[], int loc_bin_cts[], float min_meas, 
      float max_meas, int bin_count, int my_rank, MPI_Comm comm);
void Find_bins(int bin_counts[], float local_data[], int loc_bin_cts[], 
      int local_data_count, float bin_maxes[],
      int bin_count, float min_meas, MPI_Comm comm);
int Which_bin(float data, float bin_maxes[], int bin_count, 
      float min_meas);
void Print_histo(float bin_maxes[], int bin_counts[], int bin_count, 
      float min_meas);

/*---------------------------------------------------------------------*/
int main(void) {
   int bin_count;
   
   float min_meas, max_meas;
   float* bin_maxes;
   int* bin_counts;
   int* loc_bin_cts;
   
   int data_count;
   int local_data_count;
   
   float* data;
   float* local_data;

   double local_start, local_finish, local_elapsed, elapsed; //declare doubles for local_start time, local_finish time, local_elapsed time, and the final elapsed time
   
   int my_rank, comm_sz;
   MPI_Comm comm;
   
   MPI_Init(NULL, NULL);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &comm_sz);
   MPI_Comm_rank(comm, &my_rank);
   
   // get user inputs for bin_count, max_meas, min_meas, and data_count
   Get_input(&bin_count, &min_meas, &max_meas, &data_count, 
         &local_data_count, my_rank, comm_sz, comm);
   
   // allocate arrays
   bin_maxes = malloc(bin_count*sizeof(float));
   bin_counts = malloc(bin_count*sizeof(int));
   loc_bin_cts = malloc(bin_count*sizeof(int));
   data = malloc(data_count*sizeof(float));
   local_data = malloc(local_data_count*sizeof(float));

   Set_bins(bin_maxes, loc_bin_cts, min_meas, max_meas, bin_count, 
         my_rank, comm);
   Gen_data(local_data, local_data_count, data_count, min_meas, 
         max_meas, my_rank, comm);

   //use MPI_Barrier() to ensure all processes are at the same stage, about to call the Find_bins() function before we start timing
   MPI_Barrier(comm);

   //initilize the local_start variable to the current time returned by the MPI_Wtime() function. This will give us the starting time for the process as it is about to execute Find_bins().
   local_start = MPI_Wtime();

   //which_bin is called inside Find_bins(), which should be the loop that determines the values in the bin_counts variable, this is where most of the paralleization is focused, hence the runtime 
   //measurments are focused on this specific function.
   Find_bins(bin_counts, local_data, loc_bin_cts, local_data_count, 
         bin_maxes, bin_count, min_meas, comm);

   //initilize the local_finish variable to the current time returned by the MPI_Wtime() function. This will give us the ending time for the process as it has finished executing Find_bins() and come out
   //of that function call.
   local_finish = MPI_Wtime();

   //calculate the local_elapsed time, which is the finish_time - local_start time in order to get the total time which passed from start to finish while the process execution the Find_bins() function.
   local_elapsed = local_finish - local_start;

   //Call MPI_Reduce() to collect all local_elapsed time values, pick the MAX of them all (process with the longest runtime as we care about the runtime of the entire program), and return
   //this local_elapsed time to a variable called "elapsed" local to process 0 (as process 0 will handle all the outputs).
   MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

   //process of rank == 0 will handle all the outputs
   if (my_rank == 0)
   {
      char* output_file; //create a char pointer to a char array called out_putfile which will dynamically allocated memory using malloc().

      //if comm_sz == 1, 2, or 4, we put the correct file name into the output_file char array (runtimep1, runtimep2, or runtimep4). This string held in the output_file array will be used
      //by the fopen() function to open the correct file to append runtime data, depending on whether 1, 2 or 4 processes are started using mpiexec.
      //note that all output files (runtimep1, runtimep2, and runtimep4 exist in the "perfdata" folder, hence the output_file char array always has the path to perfdata)
      if(comm_sz == 1)
      {
         output_file = malloc(sizeof(char) * (strlen("perfdata/runtimep1.txt") + 1)); //initilize output_file char array to size of the desired string
         strcpy(output_file, "perfdata/runtimep1.txt"); //copy the desired string into the output_file char array using the strcpy() function
      }
      else if(comm_sz == 2)
      {
         output_file = malloc(sizeof(char) * (strlen("perfdata/runtimep2.txt") + 1)); 
         strcpy(output_file, "perfdata/runtimep2.txt");
      }
      else
      {
         output_file = malloc(sizeof(char) * (strlen("perfdata/runtimep4.txt") + 1));
         strcpy(output_file, "perfdata/runtimep4.txt");
      }

      //declare a FILE pointer called out_file which will use fopen() to open a file with the name specified by output_file (determined by the if, else-if, else statmenet above).
      //the "a" indicates we are appending to the file
      FILE *out_file = fopen(output_file, "a");  
            
      // test for files not existing. If it does not, output error message and exit program.
      if (out_file == NULL)
      {  
         printf("Error! Could not open file\n");
         exit(-1); 
      }

      //Print_histo(bin_maxes, bin_counts, bin_count, min_meas); No longer call Print_histo() as the results of the histogram program are not important to the analysis of its runtime and parallelization

      fprintf(out_file,"Number of processes = %d\nTime = %e seconds\n", comm_sz, elapsed); //use fprintf() to redirect this output to a file given by the out_file pointer declared above.
      fclose(out_file); //close the file after appending to it
      free(output_file); //free the memory allocated to the output_file char array to avoid memory leaks.
   }
   free(bin_maxes);
   free(bin_counts);
   free(loc_bin_cts);
   free(data);
   free(local_data);
   MPI_Finalize();
   return 0;

} /* main */


/*---------------------------------------------------------------------*/
void Print_histo(
      float bin_maxes[] /* in */,
      int bin_counts[]  /* in */, 
      int bin_count     /* in */, 
      float min_meas    /* in */) {
   int i, j;
   float bin_max, bin_min;
   
   for (i = 0; i < bin_count; i++) {
      bin_max = bin_maxes[i];
      bin_min = (i == 0) ? min_meas: bin_maxes[i-1];
      printf("%.3f-%.3f:\t", bin_min, bin_max);
      for (j = 0; j < bin_counts[i]; j++)
         printf("X");
      printf("\n");
   }
}  /* Print_histo */


/*---------------------------------------------------------------------*/
//this function is the one that will get each process to place the floats, in their respctive chunks of the data array, into bins
//hence most of the speedup will be here, as we split the array of floats among processes and they each call find_bins on their own chunk of the array
//to find which bins those floats belong in. Then at the bottom, they all call reduce, and send their bins back to the root process (process 0).
void Find_bins(
      int bin_counts[]      /* out */, 
      float local_data[]    /* in  */,
      int loc_bin_cts[]     /* out */, 
      int local_data_count  /* in  */, 
      float bin_maxes[]     /* in  */,
      int bin_count         /* in  */, 
      float min_meas        /* in  */, 
      MPI_Comm comm){

   int i,bin;
   for (i = 0; i < local_data_count; i++){
      bin = Which_bin(local_data[i], bin_maxes, bin_count, min_meas);
      loc_bin_cts[bin]++;
   }
   
   MPI_Reduce(loc_bin_cts, bin_counts, bin_count, MPI_INT, MPI_SUM, 0, 
         comm); //MPI_reduce is called to combine all the individuall calcuated bins, specifying we want to MPI_SUM them back at the root.
}  /* Find_bins */


/*---------------------------------------------------------------------*/
int Which_bin(float data, float bin_maxes[], int bin_count, 
      float min_meas) {
   int bottom = 0, top =  bin_count-1;
   int mid;
   float bin_max, bin_min;
   
   while (bottom <= top) {
      mid = (bottom + top)/2;
      bin_max = bin_maxes[mid];
      bin_min = (mid == 0) ? min_meas: bin_maxes[mid-1];
      if (data >= bin_max) 
         bottom = mid+1;
      else if (data < bin_min)
         top = mid-1;
      else
         return mid;
   }
   printf("Uh oh . . .\n");
   return 0;
}  /* Which_bin */


/*---------------------------------------------------------------------*/
void Set_bins(
      float bin_maxes[]  /* out */,
      int loc_bin_cts[]  /* out */,
      float min_meas     /* in  */,
      float max_meas     /* in  */, 
      int bin_count      /* in  */, 
      int my_rank        /* in  */, 
      MPI_Comm comm      /* in  */) {

   if (my_rank == 0) {
      int i;
      float bin_width;
      bin_width = (max_meas - min_meas) / bin_count;
      
      for (i = 0; i < bin_count; i++) {
         loc_bin_cts[i] = 0;
         bin_maxes[i] = min_meas + (i+1)*bin_width;
      }
      
   }

   // set bin_maxes for each proc
   MPI_Bcast(bin_maxes, bin_count, MPI_FLOAT, 0, comm);
   
   // reset loc_bin_cts of each proc
   MPI_Bcast(loc_bin_cts, bin_count, MPI_INT, 0, comm);
}  /* Set_bins */


/*---------------------------------------------------------------------*/
void Gen_data(
      float local_data[]    /* out */,
      int local_data_count  /* in  */, 
      int data_count        /* in  */, 
      float min_meas        /* in  */, 
      float max_meas        /* in  */, 
      int my_rank           /* in  */, 
      MPI_Comm comm         /* in  */) {
   int i;
   float* data = NULL;
   
   if (my_rank ==0) {
      data = malloc(data_count*sizeof(float));
      srandom(1);
      for (i = 0; i < data_count; i++)
         data[i] = 
            min_meas + (max_meas - min_meas)*random()/((double) RAND_MAX);
#     ifdef DEBUG
      printf("Generated data:\n   ");
      for (i = 0; i < data_count; i++)
         printf("%.3f ", data[i]);
      printf("\n\n");
#     endif
      MPI_Scatter(data, local_data_count, MPI_FLOAT, local_data, 
            local_data_count, MPI_FLOAT, 0, comm);
      free(data);
   } else {
      MPI_Scatter(data, local_data_count, MPI_FLOAT, local_data, 
            local_data_count, MPI_FLOAT, 0, comm);
   }
   
}  /* Gen_data */

/*---------------------------------------------------------------------*/
void Get_input(int* bin_count_p, float* min_meas_p, float* max_meas_p, 
      int* data_count_p, int* local_data_count_p, int my_rank, 
      int comm_sz, MPI_Comm comm) {
   
   int local_ok = 1;
   
   if (my_rank == 0) {
      printf("Enter the number of bins\n");
      scanf("%d", bin_count_p);
      printf("Enter the minimum measurement\n");
      scanf("%f", min_meas_p);
      printf("Enter the maximum measurement\n");
      scanf("%f", max_meas_p);
      printf("Enter the number of data\n");
      scanf("%d", data_count_p);
   }
   
   MPI_Bcast(bin_count_p, 1, MPI_INT, 0, comm);
   MPI_Bcast(min_meas_p, 1, MPI_INT, 0, comm);
   MPI_Bcast(max_meas_p, 1, MPI_INT, 0, comm);
   MPI_Bcast(data_count_p, 1, MPI_INT, 0, comm);
   
   if(*data_count_p % comm_sz != 0) local_ok = 0;
   
   Check_for_error(local_ok, "Get_input",
               "data_count must be evenly divisible by comm_sz", 
               comm);
   *local_data_count_p = *data_count_p / comm_sz;

}  /* Get_input */
   
      
/*---------------------------------------------------------------------*/
void Check_for_error(
     int       local_ok   /* in */, 
     char      fname[]    /* in */,
     char      message[]  /* in */, 
     MPI_Comm  comm       /* in */) {
   int ok;
   
   MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
   if (ok == 0) {
      int my_rank;
      MPI_Comm_rank(comm, &my_rank);
      if (my_rank == 0) {
         fprintf(stderr, "Proc %d > In %s, %s\n", my_rank, fname, 
               message);
         fflush(stderr);
      }
      MPI_Finalize();
      exit(-1);
   }
}  /* Check_for_error */
