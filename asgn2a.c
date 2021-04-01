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

    struct __attribute__((aligned (64))) InThreadBuff
    {
        int counter; //4 bytes
        int padding[13]; //52 bytes
        Point * permissiblePoints; //8 bytes  
        //total 64 byte, fit in a cacheline
    };

    // struct InThreadBuff* inTbuff =  malloc(sizeof(struct InThreadBuff)*4);
    // for (int i = 0; i < 4; i++)
    // {
    //     inTbuff[i].counter=0;
    //     inTbuff[i].permissiblePoints = malloc((number/4)*sizeof(Point));
    //     printf("inTbuff[i] = %p\n", &inTbuff[i]);
    // }

    permissiblePointNum =1;
    permissiblePoints[0].ID =1;
    permissiblePoints[0].values = points[0].values;

    // Point ** sub_result_points[4];
    // for (int i = 0; i < 4; i++)
    // {
    //     sub_result_points[i] = malloc(number/4*sizeof(Point));
    //     sub_result_points
    // }
    
    

    // #pragma omp parallel for num_threads(4) schedule(dynamic, number/4) //reduction(+:permissiblePointNum )
    for (int i = 1; i < number; i++)
    {

        int flag = 1;
        int i_inserted = 0;
        int j_wins_index = -1;
        
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
                flag = 0;
                // break;
                // printf("jjjjjjjjjjjjjjjj wins i =%d j =%d\n", i,j);
                memcpy(&tmpPoints[index],&permissiblePoints[j], sizeof(Point));
                index++;
                j_wins_index =j;
                break;
                // can ignore the later ones and directly copy
            } else if(counter==0 ){
                // printf("iiiiiiiiiii wins i =%d j =%d\n", i,j);
                flag = 0;
                if (i_inserted ==0)
                {
                   memcpy(&tmpPoints[index],&points[i], sizeof(Point));
                    index++;
                }
                i_inserted =1;
            } else{
                // printf("no one wins\n");
                memcpy(&tmpPoints[index],&permissiblePoints[j], sizeof(Point));
                index++;
            }
            
        }

        if(j_wins_index != -1){
            for (int j = (j_wins_index+1); j < permissiblePointNum; j++)
            {
                memcpy(&tmpPoints[index],&permissiblePoints[j], sizeof(Point));
                index++;
            }
        }

        if(flag){
            memcpy(&permissiblePoints[permissiblePointNum],&points[i], sizeof(Point));
            permissiblePointNum++;
        } else{
            free(permissiblePoints);
            permissiblePoints = tmpPoints;
            permissiblePointNum=index;
        }

        // for (int i = 0; i < permissiblePointNum; i++)
        // {
        //     printf("id = %d ", permissiblePoints[i].ID);
        // }
        // printf("\n");
        
    
        
    }
    

    // for (int i = 0; i < 4; i++)
    // {
    //     printf("inTbuff[%d].counter = %d\n",i,inTbuff[i].counter); 
    //     permissiblePointNum+=inTbuff[i].counter;
    //     // inTbuff[i].permissiblePoints = malloc((number/4)*sizeof(Point));
    // }

    printf("final permissiblePointNum = %d\n", permissiblePointNum);
    
    

    printf("\n--------------end---------------\n\n");


    
    // pPermissiblePoints -- your computed answer
	*pPermissiblePoints = permissiblePoints;
	return permissiblePointNum;
}