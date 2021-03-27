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
    for(int i = 0; i < 20; i++)
        printPoint(points[i], dim);

	/**********************************************************************************
	 * Work here
	 * *******************************************************************************/
    printf("\n--------------start---------------\n\n");


    permissiblePoints= realloc(permissiblePoints, number*sizeof(Point));

    

    // omp_set_num_threads(4);
    // #pragma omp parallel for ordered//reduction(+:permissiblePointNum )
    
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

            float result[8];

            if (dim >=5){
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
            __m256 bool_table = _mm256_cmp_ps(point1,point2,_CMP_GT_OS);

            __m256 all_t = _mm256_setr_ps(1,1,1,1,1,1,1,1);

            __m256 bool_bits = _mm256_and_ps(bool_table,all_t);
            _mm256_store_ps(result, bool_bits);

            }else{
                __m128 point1 = _mm_load_ps(points[i].values);
                __m128 point2 = _mm_load_ps(points[j].values);
                __m128 bool_table = _mm_cmp_ps(point1,point2,_CMP_GT_OS);
                __m128 all_t = _mm_set1_ps(1);
                __m128 bool_bits = _mm_and_ps(bool_table,all_t);
                
                _mm_store_ps(result, bool_bits);
            }

            // int counter = 0;
            for (int i = 0; i < dim; i++)
            {
                counter += (int)result[i];
            }
            // printf("counter = %d\n",counter);

            if (counter>=dim){
                flag=0;
                break;
            }
            
            // for (int i = 0; i < dim; i++)
            // {
            //     printf("result[%d] = %f  ",i,result[i]);
                
            //     if(i==dim-1) printf("\n");
            // }
            


            // for (int k = 0; k < dim; k++)
            // {   

            //     // printf("threadId = %d, i j k = %d %d %d\n", omp_get_thread_num(),i , j , k);  
                
            //     if (points[i].values[k] < points[j].values[k]){
            //         flag2 = 0;
            //         break;
            //     }
            //     else if (points[i].values[k] > points[j].values[k]){
            //         counter++;
            //         flag2=1;
            //         // printf("ID %d counter = %d\n",i+1, counter);
            //     }
            //     else if(points[i].values[k] == points[j].values[k]){
            //         counter++;
            //         // flag2=1;
            //     }
                            
            // }

            // if (flag2 && counter==dim){
            //     flag=0;
            //     // printf("ID %d prevails ID %d, ID %d is kicked out\n", j+1, i+1, i+1);
            //     break;
            // } 
            
        }

        if(flag){
            // permissiblePointNum++;
            // #pragma omp ordered
            // #pragma omp critical
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