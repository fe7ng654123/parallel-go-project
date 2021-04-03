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

struct  InThreadPara
{
    int counter; //4 bytes
    int padding[13]; //52 bytes
    Point * points; //8 bytes  
    //total 64 byte, fit in a cacheline
};


int split_four_v2(Point *points, int number, int dim, Point *permissiblePoints_result ){

    int permissiblePointNum =1;
    Point * permissiblePoints = malloc(sizeof(Point)*number);
    memcpy(&permissiblePoints[0],&points[0], sizeof(Point));

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
            permissiblePointNum = index;
        }

    }
    memcpy(permissiblePoints_result,permissiblePoints, permissiblePointNum*sizeof(Point));
    // for (int i = 0; i < permissiblePointNum; i++)
    // {
    //     memcpy(&permissiblePoints_result[i],&permissiblePoints[i], sizeof(Point));
    //     // printf("permissiblePoints[] = %d\n", permissiblePoints[i].ID);
    // }
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

    int split_num = 16;
    Point **points_split = malloc(split_num * sizeof(Point *));
    int counter[16]={0};
    int count_total =0;

    // for (int i = 0; i < 4; i++)
    // {
    //     points_split[i] = malloc(number/4*sizeof(Point));
    //     memcpy(points_split[i],&points[(number/4)*i], number/4*sizeof(Point));
    // }

    #pragma omp parallel for num_threads(4) schedule(static)
    for (int i =0; i < split_num; i++)
    {
        points_split[i] = malloc(number/split_num*sizeof(Point));
        memcpy(points_split[i],&points[(number/split_num)*i], number/split_num*sizeof(Point));
        counter[i]= split_four_v2(points_split[i],number/split_num,dim,points_split[i]);
        printf("counter[%d] = %d\n",i,counter[i]);
        // #pragma omp atomic
        // count_total += counter[i];
    }

    int split_num_second = 4;

    Point **points_split_second = malloc(split_num_second * sizeof(Point *));
    int counter_second[4]={0};

    #pragma omp parallel for num_threads(4) schedule(static)
    for (int i =0; i < split_num_second; i++)
    {
        points_split_second[i] = malloc(number/4*sizeof(Point));
        int tmp =0;
        for (int j = 0; j < 4; j++)
        {
            Point* tmp_pointer = points_split_second[i];
            memcpy(tmp_pointer+tmp,points_split[i*4+j], counter[i*4+j]*sizeof(Point));
            tmp+=counter[i*4+j];
        }
        counter_second[i]= split_four_v2(points_split_second[i],tmp,dim,points_split_second[i]);
        printf("counter_second[%d] = %d\n",i,counter_second[i]);
        #pragma omp atomics
        count_total += counter_second[i];
    }
    
    Point *total = malloc(sizeof(Point)* count_total);
    int tmp=0;
    for (int i = 0; i < split_num_second; i++)
    {
        memcpy(total+tmp,points_split_second[i], counter_second[i] * sizeof(Point));
        tmp += counter_second[i];
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