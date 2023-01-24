#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <string>
#include <time.h>
#include <omp.h>
#include <algorithm>
#include <vector> 
#include <limits.h>

using namespace std;

// Program to implement simplex method in a maximization form, given minimum 2 variables and 2 constraints
int main(int argc, char* argv[])
{
    // Get the number of threads from command line
    int thread_count;
    if (argc > 1) {
        thread_count = strtol(argv[1], NULL, 10);
    } else {
        // Set thread_count to a default value, such as 1
        thread_count = 1;
    }

    // Variables used for calculating execution time
    double start; 
    double end; 

    // Variables used to define matrix attributes
    int rows = 3;
    int columns = 6;
    int variableCount = 2;
    int constraints = 0;

    cout << "Enter the number of variables you will be dealing with: " << endl;
    cin >> variableCount;
    columns += variableCount - 2;

    cout << "Enter the number of constraints you will be dealing with (exlcude the base constraints i.e. x>=0, y>=0,...): " << endl;
    cin >> constraints;
    if (constraints > 2)
    {
        columns += constraints - 2;
        rows += constraints - 2;
    }

    start = omp_get_wtime(); 
    vector<vector<float> > mainMatrix(rows, vector<float>(columns));
    bool isNegative = true;
	int i = 0;

    // Initializes primary matrix with values of 0.0
    #pragma omp parallel for num_threads(thread_count) private(i)
    for (i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            mainMatrix[i][j] = 0.0;
        }
    }

	int j=variableCount;
    // Inputs all the coefficients of slack variables in the designated locations in the main matrix
    #pragma omp parallel for num_threads(thread_count) private(i)
    for(int i=0;i<rows;i++)
    {
        if(j<columns-1)
        {
            mainMatrix[i][j] = 1.0;
            #pragma omp critical
            {
                j++;
            }
        }
    }

    // Inputs random floats to act as coefficients of the objective function that focuses on maximization 
    for (int i = 0; i < variableCount; i++)
    {
        float x = (((float)rand() / RAND_MAX) * 800) + 2;
        (mainMatrix[rows - 1][i]) = x;
        mainMatrix[rows - 1][i] *= -1.0;
    }

    // Inputs random floats to act as coefficients of the inequality functions that act as constraints
    for (int i = 0; i < rows - 1; i++)
    {
        for (int j = 0; j < variableCount; j++)
        {
            float x = (((float)rand() / RAND_MAX) * 800) + 2;
            mainMatrix[i][j] = x;
        }
        float y = (((float)rand() / RAND_MAX) * 800) + 2;
        mainMatrix[i][columns - 1] = y;
    }
    // Performs the simplex method until the maximum value is achieved
    while (isNegative == true)
    {
        // Primary variables 
        isNegative = false;
        float pivotElement;
        int pivotRow;
        float maxNegative = 0.0;
        int maxNegativeColumn = -1;
        bool unboundedCheck = true;

        // Find local maximum negative element in the last row and the column number of the said minimum element
        #pragma omp parallel for num_threads(thread_count)
        for (int i = 0; i < columns; i++)
        {
            // Declare local variables for each thread
            float local_maxNegative = mainMatrix[rows - 1][i];
            int local_maxNegativeColumn = i;

            if (mainMatrix[rows - 1][i] < local_maxNegative)
            {
                local_maxNegative = mainMatrix[rows - 1][i];
                local_maxNegativeColumn = i;
            }

            // Update the global variables with the local values
            #pragma omp critical
            {
                if (local_maxNegative < maxNegative)
                {
                    maxNegative = local_maxNegative;
                    maxNegativeColumn = local_maxNegativeColumn;
                }
            }
        }       
         
        // Check unbounded solution
        for (int i = 0; i < variableCount; i++)
        {
            if (mainMatrix[i][maxNegativeColumn] > 0)
            {
                unboundedCheck = false;
                break;
            }
        }
        if (unboundedCheck == true)
        {
            cout << "Unbounded solution reached. Program will now end" << endl;
            return 1;
        }
        // Figure out pivot element and pivot row 
        if (maxNegative < 0) 
        {
            float minimum = 3.402823E+38;

            // Each thread identifies its own local minimum quotient 
            #pragma omp parallel for num_threads(thread_count)
            for (int i = 0; i < rows - 1; i++)
            {
                // Declare local variables for each thread
                int local_pivotRow = 0;
                float local_min = 3.402823E+38;
                float local_pivotElement = 0.0;

                if ((mainMatrix[i][columns - 1] / mainMatrix[i][maxNegativeColumn]) < local_min && mainMatrix[i][maxNegativeColumn] >= 0)
                {
                    local_pivotRow = i;
                    local_min = mainMatrix[i][columns - 1] / mainMatrix[i][maxNegativeColumn];
                    local_pivotElement = mainMatrix[i][maxNegativeColumn];
                }

                // Update the global variables with the local values
                #pragma omp critical
                {
                    if (local_min < minimum)
                    {
                        pivotRow = local_pivotRow;
                        minimum = local_min;
                        pivotElement = local_pivotElement;
                    }
                }
            }
        }
        /// Make the pivot element = 1 by dividing all elements of pivot row by pivot element
        #pragma omp parallel for num_threads(thread_count)
        for (int i = 0; i < columns; i++) 
        {
            mainMatrix[pivotRow][i] = mainMatrix[pivotRow][i] / pivotElement;
        }
        // Adjust every row apart from pivot row accordingly to reach a state where bottom row is non-negative 
        #pragma omp parallel for num_threads(thread_count)
        for (int i = 0; i < rows; i++) 
        {
            float temp = (mainMatrix[i][maxNegativeColumn] * -1.0);
            for (int j = 0; j < columns; j++)
            {
                if (i == pivotRow)
                {
                    break;
                }
                mainMatrix[i][j] = (temp * (mainMatrix[pivotRow][j])) + mainMatrix[i][j];
            }
        }
        // Runs check to see if last row is non-negative
        for (int j = 0; j < columns; j++) 
        {
            if (mainMatrix[rows - 1][j] < 0)
            {
                isNegative = true;
                break;
            }
        }
        #pragma omp barrier
    }

    // Prints out the maximum value of the objective function
    cout << "The maximum value of the objective function is: " << ceil((mainMatrix[rows - 1][columns - 1]) * 1000.0) / 1000.0 << endl;

    end = omp_get_wtime(); 
    //Final execution time
    cout << "Time taken: " << (end - start) << " s\n"<< endl;
	return 0;
}
