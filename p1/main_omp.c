//
//  main_serial.cc
//  Non-parallelized version of Mandelbrot set calculation
//  Created by Joshua Wallace on 2/12/15.
//

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <omp.h>

/*The following function computes z_{n+1} given z_n and the real and
  imaginary component of c*/
double complex next_z(double complex z, double c_real, double c_img)
{
  return z*z + c_real + c_img*I;
}

/*The following function is called if the wrong number of command
  line arguments is given*/
void intro_message(char* name)
{
  printf("Wrong number of command line arguments given.\n");
  printf("usage: %s  number_of_threads\n",name);
  exit(1);
}

int main(int argc, char **argv)
{
  if(argc!=2) //If not the right number of command line arguments given
    {
      intro_message(argv[0]);
    }

  if(atoi(argv[1])<1) //If the number of threads is set to be less than 1
    {
      printf("You set the number of threads to be %d, which is less than 1\n",atoi(argv[1]));
      printf("Now quitting...\n");
      exit(1);
    }

  double start_time = omp_get_wtime();                   //Begin timing
  double realbounds[2] = {-2.0,0.5}; //Boundaries on real axis
  double imgbounds[2] =  {-2,2};     //Boundaries on imaginary axis
  int resolution = 10000;              //Number of resolution elements per axis
  int MAX_ITER = 1000;               //Maximum number of iterations before
                                     //declaring "final" convergence
  double value_to_check_against = 4.0; //Value to check against for divergence
                                       //I use 4.0 since this is 2.0 squared
  int i,j,k;                         //indices
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

  omp_set_num_threads(atoi(argv[1])); //Set the number of threads
#pragma omp parallel for private(i,j,k)
  for(i=0;i<resolution;i++) //For each value on real axis
    {
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
          //printf("Thread: %d, i: %d, j: %d\n",omp_get_thread_num(),i,j);
          divergence_array[i][j]=k;  //Give number of iterations until divergence to 
                                     // divergence_array
        }
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
    }//closes if() statement  


  
  //Free divergence_array
  for(i=0;i<resolution;i++)
    {
      free(divergence_array[i]);
    }
  free(divergence_array);

  //Print how long the code took.
  printf("threads: %d   time in seconds: %e\n",atoi(argv[1]),omp_get_wtime()-start_time);
  
  return 0;
}
