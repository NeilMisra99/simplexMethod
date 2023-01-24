#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <string>
#include <time.h>
#include <algorithm>
#include <vector>
#include <limits.h>
#include <mpi.h>

using namespace std;

// Program to implement simplex method in a maximization form, given minimum 2 variables and 2 constraints
int main(int argc, char *argv[])
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    int size, rank;
    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // Variables used for calculating execution time
    double start;
    double end;
    // Variables used to define matrix attributes
    int rows = 3;
    int columns = 6;
    int variableCount = 2;
    int constraints = 0;
    if (rank == 0)
    {
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
        // Start the timer after taking input
        start = MPI_Wtime();
    }
    // Collect the intialization data from the root process to all other processes
    MPI_Bcast(&variableCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&constraints, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&columns, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // declare contiguous memory for the main matrix
    float **mainMatrix = (float **)malloc(rows * sizeof(float *));
    float *temp = (float *)malloc(rows * columns * sizeof(float));
    for (int i = 0; i < rows; i++)
    {
        mainMatrix[i] = &(temp[i * columns]);
    }
    bool isNegative = true;
    int i = 0;

    // division of columns on mutiple core(machine)
    int start1 = rank * columns / size;
    int end1 = (rank + 1) * columns / size;
    if (rank == (size - 1))
    {
        end1 = columns;
    }

    // division of variable count on mutiple core(machine)
    int x = 1, res = 1;
    int start2 = rank * variableCount / size;
    int end2 = (rank + 1) * variableCount / size;
    if (rank == (size - 1))
    {
        end2 = variableCount;
    }

    // division of rows count on mutiple core(machine)
    int start3 = rank * rows / size;
    int end3 = (rank + 1) * rows / size;
    if (rank == (size - 1))
    {
        int end3 = rows - 1;
    }

    // division of rows count on mutiple core(machine)
    int start4 = rank * rows / size;
    int end4 = (rank + 1) * rows / size;
    if (rank == (size - 1))
    {
        int end3 = rows;
    }

    // displacements and recvcounts vector for qllgatherv
    int recvcounts[size];
    int displs[size];
    for (int i = 0; i < size; i++)
    {
        int start = i * rows / size;
        int end = (i + 1) * rows / size;
        if (i == (size - 1))
        {
            end = rows;
        }
        recvcounts[i] = (end - start) * columns;
        displs[i] = start * columns;
    }

    if (rank == 0)
    {
        // Initializes primary matrix with values of 0.0
        for (i = 0; i < rows; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                mainMatrix[i][j] = 0.0;
            }
        }
        // Inputs all the coefficients of slack variables in the designated locations in the main matrix
        int j = variableCount;
        for (int i = 0; i < rows; i++)
        {
            if (j < columns - 1)
            {
                mainMatrix[i][j] = 1.0;
                j++;
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
    }

    // get the intialize matrix from the root process to all other processes
    MPI_Bcast(&mainMatrix[0][0], rows * columns, MPI_FLOAT, 0, MPI_COMM_WORLD);
    int ilter = 0;
    float pivotElement;
    int pivotRow;
    // Performs the simplex method until the maximum value is achieved
    while (isNegative == true)
    {
        // Primary variables
        isNegative = false;
        float maxNegative = 0.0;
        int maxNegativeColumn = -1;

        // Find local maximum negative element in the last row and the column number of the said minimum element
        // Declare local variables for each process
        MPI_Bcast(&mainMatrix[rows - 1][0], columns, MPI_FLOAT, 0, MPI_COMM_WORLD);
        float local_maxNegative = mainMatrix[rows - 1][i];
        int local_maxNegativeColumn = start1;
        for (int i = start1; i < end1; i++)
        {
            if (mainMatrix[rows - 1][i] < local_maxNegative)
            {
                local_maxNegative = mainMatrix[rows - 1][i];
                local_maxNegativeColumn = i;
            }
        }
        MPI_Allreduce(&local_maxNegative, &maxNegative, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
        if (local_maxNegative != maxNegative)
        {
            local_maxNegativeColumn = -1;
        }
        MPI_Allreduce(&local_maxNegativeColumn, &maxNegativeColumn, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

        // Check unbounded solution
        if (rank == 0)
        {
            for (int i = 0; i < variableCount; i++)
            {
                if (mainMatrix[i][maxNegativeColumn] > 0)
                {
                    x = 0;
                    break;
                }
            }
        }
        MPI_Allreduce(&x, &res, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
        if (res == 1)
        {
            if (rank == 0)
            {
                cout << "Unbounded solution reached. Program will now end " << endl;
            }
            MPI_Finalize();
            return 0;
        }

        // Figure out pivot element and pivot row
        if (maxNegative < 0)
        {
            float minimum = 3.402823E+38;
            // Each thread identifies its own local minimum quotient
            // Declare local variables for each thread
            int local_pivotRow = 0;
            float local_min = 3.402823E+38;
            float local_pivotElement = 0.0;
            for (int i = start3; i < end3; i++)
            {
                if ((mainMatrix[i][columns - 1] / mainMatrix[i][maxNegativeColumn]) < local_min && mainMatrix[i][maxNegativeColumn] >= 0)
                {
                    local_pivotRow = i;
                    local_min = mainMatrix[i][columns - 1] / mainMatrix[i][maxNegativeColumn];
                    local_pivotElement = mainMatrix[i][maxNegativeColumn];
                }
            }

            MPI_Allreduce(&local_min, &minimum, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
            if (minimum != local_min)
            {
                local_pivotRow = -1;
                local_pivotElement = -1;
            }
            MPI_Allreduce(&local_pivotRow, &pivotRow, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
            MPI_Allreduce(&local_pivotElement, &pivotElement, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
        }

        // Make the pivot element = 1 by dividing all elements of pivot row by pivot element
        MPI_Bcast(&mainMatrix[pivotRow][0], columns, MPI_FLOAT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < columns; i++)
        {
            mainMatrix[pivotRow][i] = mainMatrix[pivotRow][i] / pivotElement;
        }

        // Adjust every row apart from pivot row accordingly to reach a state where bottom row is non-negative
        for (int i = start4; i < end4; i++)
        {
            if (i == pivotRow)
            {
                continue;
            }
            float temp = (mainMatrix[i][maxNegativeColumn] * -1.0);
            for (int j = 0; j < columns; j++)
            {
                mainMatrix[i][j] = (temp * (mainMatrix[pivotRow][j])) + mainMatrix[i][j];
            }
        }
        // gather all the data from all the processes
        MPI_Gatherv(&mainMatrix[start3][0], (end3 - start3) * columns, MPI_FLOAT, &mainMatrix[0][0], recvcounts, displs, MPI_FLOAT, 0, MPI_COMM_WORLD);

        MPI_Bcast(&mainMatrix[rows - 1][0], columns, MPI_FLOAT, 0, MPI_COMM_WORLD);
        // Runs check to see if last row is non-negative
        for (int j = 0; j < columns; j++)
        {
            if (mainMatrix[rows - 1][j] < 0)
            {
                isNegative = true;
                break;
            }
        }
        if (rank == 0)
        {
            ilter++;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if (rank == 0)
    {
        end = MPI_Wtime();
        // Prints out the maximum value of the objective function
        cout << "The maximum value of the objective function is: " << ceil((mainMatrix[rows - 1][columns - 1]) * 1000.0) / 1000.0 << endl;
        // Final execution time
        cout << "Time taken: " << (end - start) << " s\n"
             << endl;
    }
    free(temp);
    free(mainMatrix);
    MPI_Finalize();
    return 0;
}
