#include <stdio.h>
#include <iostream>
#include <cmath>
#include <string>

using namespace std;

//Program to implement simplex method in a maximization form, given minimum 2 variables and 2 constraints
int main()
{
    int rows=3;
    int columns=6;
    int variableCount=2;
    int constraints = 2;
    int resultColumn [10];
    int resultRow [10];
    int temp=0;

    cout<<"Enter the number of variables you will be dealing with: "<<endl;
    cin>>variableCount;
    columns+=variableCount-2;

    cout<<"Enter the number of constraints you will be dealing with (exlcude the base constraints i.e. x>=0, y>=0,...): "<<endl;
    cin>>constraints;
    if(constraints>2)
    {
        for(int i=2;i<constraints;i++)
        {
            rows++;
            columns++;
        }
    }
    
    float mainMatrix[rows][columns];
    bool isNegative = true;

    //Initializes primary matrix with values of 0.0
    for(int i=0;i<rows;i++)
    {
        for(int j=0;j<columns;j++)
        {
            mainMatrix[i][j] = 0.0;
        }
    }

    //Inputs all the coefficients of slack variables in the designated locations in the main matrix
    for(int i=0;i<rows;i++)
    {
        for(int j=0;j<variableCount;j++)
        {
            mainMatrix[i][variableCount+i]=1.0;
        }
    }

    //Inputs the coefficients of the objective function that focuses on maximization
    cout<<"Enter the coefficients for the objective function. Separate the coefficients using a space: "<<endl;
    for(int i=0;i<variableCount;i++)
    {
        cin>>(mainMatrix[rows-1][i]);
        mainMatrix[rows-1][i]*=-1.0;
    }

    //Inputs the coefficients of the inequality functions that act as constraints
    cout<<"Enter the coefficients for the inequalities. Separate the coefficients using a space: "<<endl;
    for(int i=0;i<rows-1;i++)
    {
        for(int j=0;j<variableCount;j++)
        {
            cin>>mainMatrix[i][j];
        }
        cin>>mainMatrix[i][columns-1];
    }

    //Prints out initial array
    cout<<"The initial arrray, before performing Simplex method, is: "<<endl;
    for(int i=0;i<rows;i++) //print out final array
    {
        for(int j=0;j<columns;j++)
        {
            cout<<"|\t"<<ceil(mainMatrix[i][j] * 1000.0)/1000.0<<"\t|";
        }
        cout<<endl;
    }
    cout<<endl;

    //Primary computational part. Runs until no negatives exist in the last row
    while(isNegative == true)
    {
        isNegative = false;
        float pivotElement;
        int pivotRow;
        float maxNegative = 0;
        int maxNegativeColumn;
        bool unboundedCheck = true;

        for(int i=0;i<columns;i++) //finds max negative element and pivot column
        {
            if(mainMatrix[rows-1][i]<maxNegative)
            {
                maxNegative = mainMatrix[rows-1][i];
                maxNegativeColumn = i;
            }
        }
        //check unbounded solution
        for(int i=0;i<variableCount;i++)
        {
            if(mainMatrix[i][maxNegativeColumn]>0)
            {
                unboundedCheck = false;
                break;
            }
        }
        if(unboundedCheck == true)
        {
            cout<<"Unbounded solution reached. Program will now end"<<endl;
            return 1;
        }
        if(maxNegative<0) //figures out pivot element and pivot row
        {
            float min = 999;
            for(int i=0;i<rows-1;i++)
            {
                if((mainMatrix[i][columns-1]/mainMatrix[i][maxNegativeColumn])<min && mainMatrix[i][maxNegativeColumn]>=0)
                {
                    min = mainMatrix[i][columns-1]/mainMatrix[i][maxNegativeColumn];
                    pivotElement = mainMatrix[i][maxNegativeColumn];
                    pivotRow = i;
                }
            }
            for(int i=0;i<variableCount;i++) //array to track the order of solving columns for printing out appropriately at the end
            {
                if(maxNegativeColumn == i)
                {
                    resultColumn[temp] = maxNegativeColumn;
                    resultRow[temp] = pivotRow;
                    temp++;
                }
            }
        }
        for(int i=0;i<columns;i++) //make the pivot element = 1 by dividing all elements of pivot row by pivot element
        {
            mainMatrix[pivotRow][i] = mainMatrix[pivotRow][i]/pivotElement;
        }
        for(int i=0;i<rows;i++) //adjust every row apart from pivot row accordingly to reach a state where bottom row is non-negative
        {
            float temp = (mainMatrix[i][maxNegativeColumn]*-1.0);
            for(int j=0;j<columns;j++)
            {
                if(i==pivotRow)
                {
                    break;
                }
                mainMatrix[i][j] = (temp*(mainMatrix[pivotRow][j]))+mainMatrix[i][j];
            }
        }
        for(int j=0;j<columns;j++) //runs check to see if last row is non-negative
        {
            if(mainMatrix[rows-1][j]<0)
            {
                isNegative = true;
                break;
            }
        }
    }

    //Prints out final array after simplex implemented
    cout<<"The final arrray, after performing Simplex method, is: "<<endl;
    for(int i=0;i<rows;i++) //print out final array
    {
        for(int j=0;j<columns;j++)
        {
            cout<<"|\t"<<ceil(mainMatrix[i][j] * 100.0)/100.0<<"\t|";
        }
        cout<<endl;
    }
    cout<<endl;

    cout<<"The results of the maximization are: "<<endl;
    
    int count = 0;
    int temp2 = 0;

    //Prints out the variable values in appropriate order
    while(temp2<variableCount)
    {
        if(resultColumn[count] == temp2)
        {
            cout<<"X"<<(temp2+1)<<": "<<mainMatrix[resultRow[count]][columns-1]<<endl;
            temp2++;
        }
        count++;
    }
    //Prints out our Z value
    cout<<"Z: "<<mainMatrix[rows-1][columns-1]<<endl;
}