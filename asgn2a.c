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
	// printf("points->ID = %d, length = %ld \n",
    //     points->ID, sizeof(points->values));
    // for (int i = 0; i < sizeof(points->values); i++)
    // {
    //     printf("value[%d] = %f\n", i, points->values[i]);
    // }

    omp_set_num_threads(4);

    #pragma omp parallel for ordered
    for (int i = 0; i < number; i++)
    {
        int flag = 1;
        int counter = 0;
        for (int j = 0; j < number; j++)
        {
            if(i==j)
                continue;
            counter = 0;
            for (int k = 0; k < dim; k++)
            {   
                // printf("threadId = %d, i j k = %d %d %d\n", omp_get_thread_num(),i , j , k);  
                if (points[i].values[k] > points[j].values[k]){
                    counter++;
                    // printf("ID %d counter = %d\n",i+1, counter);
                }
            }

            if (counter>=dim){
                flag=0;
                // printf("ID %d prevails ID %d, ID %d is kicked out\n", j+1, i+1, i+1);
            } 
            if (flag ==0)
            {
                break;
            }
            
        }

        if(flag){
            // printf("ID %d survived\n", i+1);
            #pragma omp ordered
            #pragma omp atomic update 
            permissiblePointNum++;
            // printf("permissiblePointNum = %d\n", permissiblePointNum);
            permissiblePoints= realloc(permissiblePoints, permissiblePointNum*sizeof(Point));
            memcpy(&permissiblePoints[permissiblePointNum-1],&points[i], sizeof(Point));
        }
        
    }
    

    printf("\n--------------end---------------\n\n");


    
    // pPermissiblePoints -- your computed answer
	*pPermissiblePoints = permissiblePoints;
	return permissiblePointNum;
}

