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

    union Point_U{
        __m128 m128;
        __m256 m256;
        float v[8];
    };


    permissiblePoints= realloc(permissiblePoints, number*sizeof(Point));

    // #pragma omp end distribute parallel for simd
// #pragma simd_level(10)
// for (i=1; i<1000; i++) { 
// /* program code */

// } 

    #pragma omp parallel for num_threads(4) //schedule(dynamic, 1024) //reduction(+:permissiblePointNum )
    for (int i = 0; i < number; i++)
    {
        int flag = 1;

        // compare candidta with known permissible points first, point i highly likely to be beaten by them.
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
            // #pragma execution_frequency(very_high)
            for (int i = 0; i < dim; i++)
            {
                counter += (int)result_m.v[i];
            }

            if (counter>=dim){ //all dimensions of point i are greater than or equal to point j, kick i out of permissiblePoint.
                flag=0;
                // break;
                goto come_here;
            } 

        }
        
        //if point i not beaten by know permissible points, then compare with all other points
        for (int j = 0; j < number; j++)
        {
            if(i==j) continue;

            int counter = 0; //for counting numbers of greater of equal dimensions
            union Point_U result_m; //for storing the bool table when comparing 2 points

            // #pragma execution_frequency(very_high)
            if (dim ==5){
            __m256 point1 = _mm256_setr_ps(
                points[i].values[0],
                points[i].values[1],
                points[i].values[2],
                points[i].values[3],
                points[i].values[4],
                0,0,0); 
            __m256 point2 = _mm256_setr_ps(
                points[j].values[0],
                points[j].values[1],
                points[j].values[2],
                points[j].values[3],
                points[j].values[4],
                0,0,0); 
            __m256 bool_table = _mm256_cmp_ps(point1,point2,_CMP_GE_OS);
            result_m.m256 = _mm256_and_ps(bool_table,_mm256_set1_ps(1));
            }else{
                __m128 point1 = _mm_load_ps(points[i].values);
                __m128 point2 = _mm_load_ps(points[j].values);
                __m128 bool_table = _mm_cmp_ps(point1,point2,_CMP_GE_OS);
                result_m.m128 = _mm_and_ps(bool_table, _mm_set1_ps(1));
            }
            // #pragma execution_frequency(very_high)
            for (int i = 0; i < dim; i++)
            {
                counter += (int)result_m.v[i];
            }

            if (counter>=dim){ //all dimensions of point i are greater than or equal to point j, kick i out of permissiblePoint.
                flag=0;
                // break;
                goto come_here;
            } 
            
        }
        
        if(flag){
            // permissiblePointNum++;
            // #pragma omp ordered
            #pragma omp critical
            memcpy(&permissiblePoints[permissiblePointNum++],&points[i], sizeof(Point));
            // printf(" points[i].ID = %d  ", points[i].ID);
            // printf("permissiblePointNum = %d\n", permissiblePointNum);
        }
        come_here:
        continue;
        
    }
    printf("final permissiblePointNum = %d\n", permissiblePointNum);
    
    

    printf("\n--------------end---------------\n\n");


    
    // pPermissiblePoints -- your computed answer
	*pPermissiblePoints = permissiblePoints;
	return permissiblePointNum;
}