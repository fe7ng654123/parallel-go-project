#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
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
    //	for(int i = 0; i < 20; i++)
    //		printPoint(points[i], dim);

	/**********************************************************************************
	 * Work here
	 * *******************************************************************************/
	
    
    // pPermissiblePoints -- your computed answer
	*pPermissiblePoints = permissiblePoints;
	return permissiblePointNum;
}

