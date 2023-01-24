# Simplex Method
Implements the simplex method in C++ serially, and in parallel using OpenMPIand MPI:

Compilation/Execution instructions for OpenMP:
To compile the program:

g++ -fopenmp -o executable_name cpp_file_name
Example compilation:
g++ -fopenmp -o mp mp_imp.cpp

To execute:
./executable_name thread_count

Once executed, you will be prompted to enter the variable count and constraint
count, respectively, for the current instance of the program
The program will then generate that number of variables and constraints with
randomized float coefficients and output a max Z value and time taken during
execution

Compilation/Execution instructions for OpenMPI
To Compile the program:

mpic++ -g -Wall -o executable_name cpp_file_name
Example compilation:
mpic++ -g -Wall -o mpi mpi_imp.cpp

To execute:
mpiexec -n process_count ./executable_name
Once again, you will be prompted to enter variable and constraint count and
output will be the same as the OpenMP version
