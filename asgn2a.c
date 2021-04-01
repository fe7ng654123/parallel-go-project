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

int split_four(Point *points, int number, int dim, Point *points_subset ){

    struct InThreadPara* inTPara =  malloc(sizeof(struct InThreadPara)*4);

    for (int i = 0; i < 4; i++)
    {
        inTPara[i].points = malloc(number/4*sizeof(Point));
        memcpy(&inTPara[i].points[0],&points[number/4*i], sizeof(Point));
        inTPara[i].counter =1;
    }

    #pragma omp parallel for num_threads(4) schedule(static, number/4) //reduction(+:permissiblePointNum )
    for (int i = 0; i < number; i++)
    {
        if( (i%(number/4)) == 0) continue;

        int flag = 1;
        int i_inserted = 0;
        int j_wins_index = -1;
        
        Point * tmpPoints = malloc(number*sizeof(Point));
        int index = 0;
        
        int tid = i/(number/4);
        // printf("i,tid = %d %d\n",i,tid);
        // compare candidta with known permissible points
        for (int j = 0; j < inTPara[tid].counter; j++)
        {
            // printf("waaaaaaaaaaaaaaaaaa");

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
                inTPara[tid].points[j].values[0],
                inTPara[tid].points[j].values[1],
                inTPara[tid].points[j].values[2],
                inTPara[tid].points[j].values[3],
                inTPara[tid].points[j].values[4],
                0,0,0); 
            __m256 bool_table = _mm256_cmp_ps(point1,point2,_CMP_GE_OS);
            result_m.m256 = _mm256_and_ps(bool_table,_mm256_set1_ps(1));
            }else{
                __m128 point1 = _mm_load_ps(points[i].values);
                __m128 point2 = _mm_load_ps(inTPara[tid].points[j].values);
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
                memcpy(&tmpPoints[index],&inTPara[tid].points[j], sizeof(Point));
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
                memcpy(&tmpPoints[index],&inTPara[tid].points[j], sizeof(Point));
                index++;
            }
            
        }

        if(j_wins_index != -1){
            for (int j = (j_wins_index+1); j < inTPara[tid].counter; j++)
            {
                memcpy(&tmpPoints[index],&inTPara[tid].points[j], sizeof(Point));
                index++;
            }
        }

        if(flag){
            memcpy(&inTPara[tid].points[inTPara[tid].counter],&points[i], sizeof(Point));
            inTPara[tid].counter++;
        } else{
            free(inTPara[tid].points);
            inTPara[tid].points = tmpPoints;
            inTPara[tid].counter=index;
        }

        // for (int i = 0; i < permissiblePointNum; i++)
        // {
        //     printf("id = %d ", permissiblePoints[i].ID);
        // }
        // printf("\n");
        
        
        
    }

    // Point * points_second = malloc(number*sizeof(Point));
    int points_subset_num = 0;
    
    for (int i = 0; i < 4; i++)
    {
        printf("inTPara[%d].counter = %d\n",i,inTPara[i].counter); 
        // permissiblePointNum+=inTPara[i].counter;

        for (int j = 0; j < inTPara[i].counter; j++)
        {
            memcpy(&points_subset[points_subset_num],&inTPara[i].points[j], sizeof(Point));
            points_subset_num++;
        }
        
    }

    printf("points_subset_num = %d \n", points_subset_num);
    return points_subset_num;

};

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

    Point * points_mid = malloc(number*sizeof(Point));
    int points_mid_num = split_four(points,number,dim,points_mid);
    printf("points_mid_num = %d \n", points_mid_num);


    Point * points_second = malloc(number*sizeof(Point));
    int points_second_num = 0;
    points_second_num = split_four(points_mid,points_mid_num,dim,points_second);
    printf("points_second_num = %d \n", points_second_num);

    
    

    permissiblePointNum =1;
    memcpy(&permissiblePoints[0],&points_second[0], sizeof(Point));

    for (int i = 1; i < points_second_num; i++)
    {
        int flag = 1;
        int i_inserted = 0;
        int j_wins_index = -1;
        
        Point * tmpPoints = malloc(points_second_num*sizeof(Point));
        int index = 0;
        
        // compare candidta with known permissible points
        for (int j = 0; j < permissiblePointNum; j++)
        {

            int counter = 0; //for counting numbers of greater of equal dimensions
            union Point_U result_m; //for storing the bool table when comparing 2 points

            if (dim ==5){
            __m256 point1 = _mm256_setr_ps(
                points_second[i].values[0],
                points_second[i].values[1],
                points_second[i].values[2],
                points_second[i].values[3],
                points_second[i].values[4],
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
                __m128 point1 = _mm_load_ps(points_second[i].values);
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
                   memcpy(&tmpPoints[index],&points_second[i], sizeof(Point));
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
            memcpy(&permissiblePoints[permissiblePointNum],&points_second[i], sizeof(Point));
            permissiblePointNum++;
        } else{
            free(permissiblePoints);
            permissiblePoints = tmpPoints;
            permissiblePointNum = index;
        }

        // for (int i = 0; i < permissiblePointNum; i++)
        // {
        //     printf("id = %d ", permissiblePoints[i].ID);
        // }
        // printf("\n");
        

    }

    printf("final permissiblePointNum = %d\n", permissiblePointNum);
    
    

    printf("\n--------------end---------------\n\n");


    
    // pPermissiblePoints -- your computed answer
	*pPermissiblePoints = permissiblePoints;
	return permissiblePointNum;
}