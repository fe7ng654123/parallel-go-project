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

    permissiblePoints= realloc(permissiblePoints, number*sizeof(Point));

    omp_set_num_threads(4);
    // int temp_max = 0;
    int tmp_arrary[number];
    memset (tmp_arrary, -1, sizeof(tmp_arrary));
    #pragma omp parallel for reduction(+:permissiblePointNum )//ordered
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

            if (counter==dim){
                flag=0;
                // printf("ID %d prevails ID %d, ID %d is kicked out\n", j+1, i+1, i+1);
                break;
            } 
            
        }

        if(flag){
            // printf("ID %d survived\n", i+1);
            // #pragma omp ordered
            // #pragma omp atomic update 
            permissiblePointNum++;
            // printf("permissiblePointNum = %d\n", permissiblePointNum);
            tmp_arrary[i]=i;
            
        }
        
    }
    printf("final permissiblePointNum = %d\n", permissiblePointNum);
    
    int j = -1;
    #pragma omp parallel for ordered
    for (int i = 0; i < number; i++)
    {
        
        // j++;
        // printf("i=%d, tmp_arrary[i] = %d \n ",i, tmp_arrary[i]);
        if (tmp_arrary[i] !=-1){
            #pragma omp ordered
            #pragma omp para atomic capture
            j++;
            // printf("i,j = %d , %d, tmp_arrary[i] = %d \n",i,j, tmp_arrary[i]);
            memcpy(&permissiblePoints[j],&points[tmp_arrary[i]], sizeof(Point));
        }
    }
    
    

    printf("\n--------------end---------------\n\n");


    
    // pPermissiblePoints -- your computed answer
	*pPermissiblePoints = permissiblePoints;
	return permissiblePointNum;
}