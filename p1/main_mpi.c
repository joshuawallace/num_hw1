//
//  main_mpi.cc
//  MPI parallelized version of Mandelbrot set calculation
//  Created by Joshua Wallace on 2/21/15.
//  Why do we not do simple domain decomposition?  Not all domains are created
//  equal.  Some require more processing time than others, since some domains
//  diverge faster than others. So a worker-slave model provides better
//  load balance across the processors
//
//
//  My master-slave model goes like this: the NxN array I want to produce is separated into
//  N 1-D chunks.  The master tells each slave process which chunk it wants the slave to work 
//  on (denoted by an integer between 0 and N-1), the slave works on it, and then returns a
//  1-D array with the results of its calculation. The 1-D array is length N+1 because it also
//  contains a value letting the master know which chunk's worth of data it is so the master
//  can piece the data back together in the correct order.

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <time.h>
#include "mpi.h"

/*Define master process rank*/
#define MASTER 0

/*Define a tag for communicating the value of i across processors*/
#define I_COMM_TAG 1

/*Define a tag for communicating an array across processors*/
#define ARR_COMM_TAG 2


/*The following function computes z_{n+1} given z_n and the real and
  imaginary component of c*/
double complex next_z(double complex z, double c_real, double c_img)
{
  return z*z + c_real + c_img*I;
}


int main(int argc, char **argv)
{
  int rank, numprocess; //store rank and total processor number, respectively

  double value_to_check_against = 4.0; //Value to check against for divergence
                                       //I use 4.0 since this is 2.0 squared
  int i,j,k;                         //indices
  int resolution = 10000;            //Number of resolution elements per axis
  int MAX_ITER = 1000;               //Maximum number of iterations before
                                     //declaring "final" convergence
  double realbounds[2] = {-2.0,0.5}; //Boundaries on real axis
  double imgbounds[2] =  {-2,2};     //Boundaries on imaginary axis
  MPI_Status status;                 //Used to store status of MPI_Recv

  MPI_Init(&argc, &argv);            //Initialize MPI
  double begin = MPI_Wtime();        //Begin timing
  MPI_Comm_rank( MPI_COMM_WORLD, &rank); //Get rank
  MPI_Comm_size( MPI_COMM_WORLD, &numprocess); //Get total number of processors
  if(numprocess==1)
    {
      printf("This program uses master-slave format to solve the problem.\n");
      printf("One processor won't work.  Use at least two.  Now quitting...\n");
      exit(1);
    }
  printf("rank: %d,   numproc: %d\n",rank,numprocess); //Print out rank

  if(rank==MASTER)  //If you are the master processor
    {
      /*An array used to temporarily hold data received from slaves*/
      int *holding_array = malloc((resolution+1)*sizeof(int));
      
      int **divergence_array = (int **)malloc(resolution*sizeof(int *));
                                    //Above is a pointer to pointer, to dynamically 
                                    //allocate an array as follows
      for(i=0;i<resolution;i++)
        {
          divergence_array[i]=malloc(resolution*sizeof(int));
        }                               //An array used to store the number of iterations
                                        //before a specific point on the grid diverges.
                                        //I set the array to be allocated dynamically so
                                        //that the resolution can be changed easily 
                                        //in the code if desired.
      int r;

      /*Send initial work orders to slaves*/
      for (r=0; r<(numprocess-1); r++)
        {
          MPI_Send(&r, 1, MPI_INT, r+1, I_COMM_TAG, MPI_COMM_WORLD);
        }

      i=r; //Initialize i based on work orders already sent

      while(i<resolution)  //For each work order necessary to complete array
        {
          MPI_Recv(holding_array, resolution+1, MPI_INT,MPI_ANY_SOURCE, ARR_COMM_TAG, MPI_COMM_WORLD, &status); //Receive a chunk's results from a slave
          MPI_Send(&i, 1, MPI_INT, status.MPI_SOURCE, I_COMM_TAG, MPI_COMM_WORLD); 
                                        //Give new work order to that same slave
          j = holding_array[0]; //Get chunk number from the array
                                //this let's the master know how to organize these data relative
                                //to those produces by other processes
          for(k=0;k<resolution;k++)
            {
              divergence_array[j][k] = holding_array[k+1]; //Put the data into the master's array
	    }
          i++;  //Update work order number
        }

      for(r=1; r<numprocess; r++) //After all the work orders are sent, receive all outstanding work results
        {
          /*receiving a work result*/
          MPI_Recv(holding_array, resolution+1, MPI_INT,r, ARR_COMM_TAG, MPI_COMM_WORLD, &status); 
          
          /*The next 5 lines do the same thing as the similar lines about 15 lines above,
            they take the received data and place them in the proper spot in the master array*/
          j = holding_array[0];
          for(k=0;k<resolution;k++)
            {
              divergence_array[j][k] = holding_array[k+1];
            }
        }

      /*Tell all the slaves they're done*/
      for(r=1; r<(numprocess); r++)
        {
          int kill=-1; //The -1 value let's the slaves know they're done
          MPI_Send(&kill,1,MPI_INT,r,I_COMM_TAG,MPI_COMM_WORLD);
        }

      if(0) //Allows me to turn output on and off: 0 is off, non-zero on
        {
          FILE *output;  //FILE pointer to output file
          
          //Make 'output.dat' the output file and double check that it opened.
          if(!(output=fopen("output.dat","w")))
            {
              //If file isn't open, let us know
              printf("File was unable to open! Didn't work.  Sorry.\n");
            }
          else //If file did open, write to it.  Real axis is written row-wise, imaginary column-wise
            {
              fprintf(output,"#Mandelbrot set data\n");
              fprintf(output,"#Columns imaginary axis, rows real axis\n");
              fprintf(output,"#Number: one less than number of iterations before diverging.  Max: %d\n",MAX_ITER-1);
              for(i=0;i<resolution;i++) //For all values on real axis
                {
                  for(j=0;j<resolution;j++) //For all values on imaginary axis
                    {
                      fprintf(output,"%d   ",divergence_array[i][j]);
                    }
                  fprintf(output,"\n");
                }
              fclose(output);
            }
        } //closes if() statement

      //Free divergence_array
      for(i=0;i<resolution;i++)
        {
          free(divergence_array[i]);
        }
      free(divergence_array);
      
      //Free holding_array
      free(holding_array);
    }

  /*Slave process work starts here*/
  else  //You are a slave process
    {
      int *array;
      array = (int *)malloc((1+resolution)*sizeof(int)); //Used to store and pass results
      
      while(i!=-1) //While the kill signal isn't given
        {
          MPI_Recv(&i, 1, MPI_INT, MASTER, I_COMM_TAG, MPI_COMM_WORLD, &status); //Receive work order number

          if(i!=-1)  //If the kill signal wasn't received
            {
              array[0]=i; //Make first value of array the work order number so master can put data
                          //in the right spot in master array

              //First, calculate position on real axis
              double realposition = (realbounds[1] - realbounds[0])/resolution * (i+.5) + realbounds[0];
              for(j=0;j<resolution;j++) //For each value on imaginary axis
                {
                  //First, calculate position on imaginary axis
                  double imgposition = (imgbounds[1] - imgbounds[0])/resolution * (j+.5) + imgbounds[0];
                  double complex z=0 + 0*I; //z is our z_n and here we initalize it to 0 + 0*I
                  for(k=0;k<MAX_ITER;k++) 
                    {
                      z = next_z(z,realposition,imgposition);//Calculate z_{n+1}
                      //Then, check if result has diverged
                      if( (creal(z)*creal(z) + cimag(z)*cimag(z)) > value_to_check_against)
                        {
                          break; //If result has diverged, break out of loop.  We have our value.
                        }
                    }
                  if(k==MAX_ITER) //If the value did not diverge after MAX_ITER
                    //iterations, the final k value will be incremented one last
                    //time by the for loop before it terminates.  This corrects this.
                    {
                      k--;
                    }
                  array[j+1]=k;  //Give number of iterations until divergence to 
                                 // the array
                }
              /*Send the results to master*/
              MPI_Send(array, resolution+1, MPI_INT, MASTER, ARR_COMM_TAG, MPI_COMM_WORLD);
            }
          
          else //Kill signal was given
            {
              free(array);
            }  
        }

      //Reaching this point requires the kill signal to have been received
      printf("Processor %d now quitting normally\n",rank);
    }

  //Print how long the code took.
  if(rank==MASTER)
  {
    printf("processors: %d  Time in seconds: %e\n", numprocess, MPI_Wtime()-begin);
  }

  MPI_Finalize();

  return 0;
}
