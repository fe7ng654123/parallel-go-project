#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>
#include <omp.h>
#include"util.h"

// If you have referenced to any source code that is not written by you
// You have to cite them here.

int asgn2a(Point * points, Point ** pPermissiblePoints, int number, int dim, int thread_number)
{
    // points -- input data
    // pPermissiblePoints -- your computed answer
    // number -- number of points in dataset
    // dim -- the dimension of the dataset
    // thread_number -- the number of threads (cores) to use
    
	int permissiblePointNum = 0;
	Point * permissiblePoints = NULL;

	//the following for-loop iterates the first 20 points that will be inputted by runtest.c
    // for(int i = 0; i < 20; i++)
    //     printPoint(points[i], dim);

	/**********************************************************************************
	 * Work here
	 * *******************************************************************************/
    printf("\n--------------start---------------\n\n");


    permissiblePoints= realloc(permissiblePoints, number*sizeof(Point));

    

    omp_set_num_threads(4);
    #pragma omp parallel for ordered//reduction(+:permissiblePointNum )
    
    for (int i = 0; i < number; i++)
    {
        int flag = 1;
        // int flag2 =0;
        // int counter = 0;
        for (int j = 0; j < number; j++)
        {
            if(i==j)
                continue;
            int counter = 0;
            int flag2 = 0 ;
            for (int k = 0; k < dim; k++)
            {   
                // printf("threadId = %d, i j k = %d %d %d\n", omp_get_thread_num(),i , j , k);  
                if (points[i].values[k] > points[j].values[k]){
                    counter++;
                    flag2=1;
                    // printf("ID %d counter = %d\n",i+1, counter);
                } else if(points[i].values[k] == points[j].values[k]){
                    counter++;
                    // flag2=1;
                }
            // printf("%d\n", counter);
                
              
            }

            if (flag2 && counter==dim){
                flag=0;
                // printf("ID %d prevails ID %d, ID %d is kicked out\n", j+1, i+1, i+1);
                break;
            } 
            
        }

        if(flag){
            // permissiblePointNum++;
            // #pragma omp ordered
            #pragma omp critical
            memcpy(&permissiblePoints[permissiblePointNum++],&points[i], sizeof(Point));
            // printf("%d\n", points[i].ID);
            // printf("permissiblePointNum = %d\n", permissiblePointNum);
        }
        
    }
    printf("final permissiblePointNum = %d\n", permissiblePointNum);
    
    

    printf("\n--------------end---------------\n\n");


    
    // pPermissiblePoints -- your computed answer
	*pPermissiblePoints = permissiblePoints;
	return permissiblePointNum;
}