#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>
#include <omp.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include"util.h"

// If you have referenced to any source code that is not written by you
// You have to cite them here.

union Point_U{
    __m128 m128;
    __m256 m256;
    float v[8];
};


int split_four_v2(Point *points, int number, int dim, Point *permissiblePoints_result ){

    int permissiblePointNum =1;
    Point * permissiblePoints = malloc(sizeof(Point)*number);
    memcpy(&permissiblePoints[0],&points[0], sizeof(Point));

    for (int i = 1; i < number; i++)
    {
        int permissible = 1;
        int i_inserted_index = -1;
        
        Point * tmpPoints = malloc(number*sizeof(Point));
        int index = 0;
        
        // compare candidta with known permissible points
        for (int j = 0; j < permissiblePointNum; j++)
        {

            int counter = 0; //for counting numbers of greater of equal dimensions
            union Point_U result_m; //for storing the bool table when comparing 2 points

            if (dim ==5){
            __m256 point1 = _mm256_setr_ps(
                points[i].values[0],
                points[i].values[1],
                points[i].values[2],
                points[i].values[3],
                points[i].values[4],
                0,0,0); 
            __m256 point2 = _mm256_setr_ps(
                permissiblePoints[j].values[0],
                permissiblePoints[j].values[1],
                permissiblePoints[j].values[2],
                permissiblePoints[j].values[3],
                permissiblePoints[j].values[4],
                0,0,0); 
            __m256 bool_table = _mm256_cmp_ps(point1,point2,_CMP_GE_OS);
            result_m.m256 = _mm256_and_ps(bool_table,_mm256_set1_ps(1));
            }else{
                __m128 point1 = _mm_load_ps(points[i].values);
                __m128 point2 = _mm_load_ps(permissiblePoints[j].values);
                __m128 bool_table = _mm_cmp_ps(point1,point2,_CMP_GE_OS);
                result_m.m128 = _mm_and_ps(bool_table, _mm_set1_ps(1));
            }
            for (int i = 0; i < dim; i++)
            {
                counter += (int)result_m.v[i];
            }

            if (counter>=dim){ //all dimensions of point i are greater than or equal to point j
                permissible = 0;
                break; 
            } else if(counter==0 ){
                if (i_inserted_index == -1)
                {
                    memcpy(&tmpPoints[index],&points[i], sizeof(Point));
                    i_inserted_index = index;
                    index++;
                }
            } else{
                memcpy(&tmpPoints[index],&permissiblePoints[j], sizeof(Point));
                index++;
            }
            
        }


        if(permissible){
            if(i_inserted_index ==-1){
                memcpy(&permissiblePoints[permissiblePointNum],&points[i], sizeof(Point));
                permissiblePointNum++;
                free(tmpPoints);
            }else{
                free(permissiblePoints);
                permissiblePoints = tmpPoints;
                permissiblePointNum = index;
            }
            
        } else{
            free(tmpPoints);
        }

    }
    
    memcpy(permissiblePoints_result,permissiblePoints, permissiblePointNum*sizeof(Point));
    return permissiblePointNum;
}

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

    Point **points_split = malloc(number * sizeof(Point *));
    int counter[4]={0};
    int count_total =0;


    #pragma omp parallel for num_threads(4) //schedule(static,1)
    for (int i =0; i < 4; i++)
    {
        points_split[i] = malloc(number/4*sizeof(Point));
        memcpy(points_split[i],&points[(number/4)*i], number/4*sizeof(Point));
        counter[i]= split_four_v2(points_split[i],number/4,dim,points_split[i]);
        printf("counter[%d] = %d\n",i,counter[i]);
        #pragma omp atomic
        count_total += counter[i];
    }
    
    Point *total = malloc(sizeof(Point)* count_total);
    int tmp=0;
    for (int i = 0; i < 4; i++)
    {
        memcpy(total+tmp,points_split[i], counter[i] * sizeof(Point));
        tmp += counter[i];
    }

    
    permissiblePointNum = split_four_v2(total,count_total,dim,permissiblePoints);

    printf("final permissiblePointNum = %d\n", permissiblePointNum);
    
    
    // for (int i = 0; i < permissiblePointNum; i++)
    // {
    //     printf("ID = %d\n", permissiblePoints[i].ID);
    // }
    

    printf("\n--------------end---------------\n\n");


    
    // pPermissiblePoints -- your computed answer
	*pPermissiblePoints = permissiblePoints;
	return permissiblePointNum;
}