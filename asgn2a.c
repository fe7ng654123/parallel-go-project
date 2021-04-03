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
    float v[5];
};


int get_p_points(Point *points, int number, int dim, Point *permissiblePoints_result ){

    int permissiblePointNum =1;
    Point * permissiblePoints = malloc(sizeof(Point)*number);
    memcpy(&permissiblePoints[0],&points[0], sizeof(Point));

    for (int i = 1; i < number; i++)
    {
        int permissible = 1; //assume new point is permissible
        int i_inserted_index = 0; //insert i if i preveils a know permissible point
        
        Point * tmpPoints = malloc(number*sizeof(Point)); //for store updated P points
        int index = 0; // index for tmpPoints
        
        // compare candidta with known permissible points
        for (int j = 0; j < permissiblePointNum; j++)
        {

            int counter = 0; //for counting numbers of greater of equal dimensions
            union Point_U result_m; //for storing the bool table when comparing 2 points

            __m128 point1 = _mm_load_ps(points[i].values);
            __m128 point2 = _mm_load_ps(permissiblePoints[j].values);
            __m128 bool_table = _mm_cmp_ps(point1,point2,_CMP_GE_OS);
            result_m.m128 = _mm_and_ps(bool_table, _mm_set1_ps(1));

            if(dim == 5 && points[i].values[4]>=permissiblePoints[j].values[4]){
                result_m.v[4] = 1.0;
            }else{
                result_m.v[4] =0;
            }
            
            for (int i = 0; i < dim; i++)
            {
                counter += (int)result_m.v[i];
            }

            if (counter>=dim){ //all dimensions of point i are greater than or equal to point j
                permissible = 0;
                break; 
            } else if(counter==0 ){ //i preveils j, replace j with i
                if (i_inserted_index == 0)
                {
                    memcpy(&tmpPoints[index],&points[i], sizeof(Point));
                    i_inserted_index = 1;
                    index++;
                }
            } else{
                memcpy(&tmpPoints[index],&permissiblePoints[j], sizeof(Point));
                index++;
            }
            
        }


        if(permissible){
            if(!i_inserted_index){
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


    #pragma omp parallel for num_threads(thread_number) //schedule(static,1)
    for (int i =0; i < 4; i++)
    {
        points_split[i] = malloc(number/4*sizeof(Point));
        memcpy(points_split[i],&points[(number/4)*i], number/4*sizeof(Point));
        counter[i]= get_p_points(points_split[i],number/4,dim,points_split[i]);
        printf("counter[%d] = %d\n",i,counter[i]);
        #pragma omp atomic
        count_total += counter[i];
    }
    
    Point *merged_p_points = malloc(sizeof(Point)* count_total);
    int tmp=0;
    for (int i = 0; i < 4; i++)
    {
        memcpy(merged_p_points+tmp,points_split[i], counter[i] * sizeof(Point));
        tmp += counter[i];
    }

    
    permissiblePointNum = get_p_points(merged_p_points,count_total,dim,permissiblePoints);

    printf("final permissiblePointNum = %d\n", permissiblePointNum);
    
    

    printf("\n--------------end---------------\n\n");


    
    // pPermissiblePoints -- your computed answer
	*pPermissiblePoints = permissiblePoints;
	return permissiblePointNum;
}