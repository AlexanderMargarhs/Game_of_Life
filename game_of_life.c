#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

int main(int argc, char* argv[]) 
{
    int process, rank, N, Load, i, Current_Generation, Generation;
    int *A, *A_loc, *temp, ndims = 2, dims[2]= {0, 1}, period[2]= {0, 0}, reorder = 1;
    int *Data_for_Up, *Data_for_Down, *Data_receive_from_Up, *Data_receive_from_Down;
    int Up, Down;
    int Sum, sum_of_overpopulation, sum_of_loneliness, sum_of_birth;
    int alive_cells, overpopulation, loneliness, birth; 
    
    srand(time(NULL));

    MPI_Init(&argc, &argv);
    MPI_Comm New_Comm;
    MPI_Comm_size(MPI_COMM_WORLD, &process); // Get number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get own ID
     
    if(rank == 0)
    {
        printf("Please type the number N.\n");
        scanf("%d", &N);
        if((N * N) % process != 0) // ERROR message.
        { 
            printf("Wrong, arguments.\nN squared must be an integer multiple of the number of processes\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        printf("Please type the number of the generations you want to simulate.\n"); // Number of Generations.
        scanf("%d", &Generation);

        Load = ( N * N ) / process; // Find the Load.
        A = (int *)malloc(sizeof(int) * N * N); // Allocate memory.
        for(i = 0; i < N * N; i ++)
        {
            A[i] = (rand() % 2); // Fill the matrix with 0 and 1 in random places.
        }
        printf("\n\nINTITIAL STATE\n\n"); // Print initial state of the cell.
        for(i = 0; i < N * N; i ++)
        {
            printf(" |%d|", A[i]);
            if(((i + 1) % N) == 0)
            {
                printf("\n");
            }
        }
    }
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD); // Broadcast the Data.
    MPI_Bcast(&Load, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Generation, 1, MPI_INT, 0, MPI_COMM_WORLD);
    A_loc = (int *)malloc(sizeof(int) * Load); // Malloc for each process.
    temp = (int *)calloc(sizeof(int), Load);
    
    MPI_Scatter(A, Load, MPI_INT, A_loc, Load, MPI_INT, 0, MPI_COMM_WORLD); // Send the Data to every process.
    dims[0] = process;
    MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, period, reorder, &New_Comm); // Create the cartesian.
    MPI_Cart_shift(New_Comm, 0, 1, &Up, &Down); // Find the above and below process.
        
    for(i = 0; i < Load; i ++)
    {
        temp[i] = A_loc[i]; // Fill the temp matrix with the data of A_loc of each process.
    }

    Data_for_Up = (int *)calloc(sizeof(int), N); // Malloc for each process.
    Data_for_Down = (int *)calloc(sizeof(int), N);
    Data_receive_from_Up = (int *)calloc(sizeof(int), N);
    Data_receive_from_Down = (int *)calloc(sizeof(int), N);

    for(Current_Generation = 0; Current_Generation < Generation; Current_Generation ++)
    {
        // Initialize the sum of every Generation.
        sum_of_overpopulation = 0;
        sum_of_loneliness = 0;
        sum_of_birth = 0;
        alive_cells = 0;
        overpopulation = 0;
        loneliness = 0;
        birth = 0;
        for(i = 0; i < Load; i ++)
        {
            A_loc[i] = temp[i];
        }

        for(i = 0; i < N; i ++)
        {
            Data_for_Up[i] = A_loc[Load - N + i];
            Data_for_Down[i] = A_loc[i];
        }

        MPI_Send(Data_for_Down, N, MPI_INT, Up, 20, MPI_COMM_WORLD);        
        MPI_Recv(Data_receive_from_Down, N, MPI_INT, Down, 20, MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        MPI_Send(Data_for_Up, N, MPI_INT, Down, 10, MPI_COMM_WORLD);
        MPI_Recv(Data_receive_from_Up, N, MPI_INT, Up, 10, MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        for(i = 0; i < Load; i ++)
        {
            Sum = 0;

            if(i >= N) // UP.
            {
                if(A_loc[i - N] == 1)
                {
                    Sum ++;
                }
                if(i % N != 0) // UP-LEFT.
                {
                    if(A_loc[i - N - 1] == 1)
                    {
                        Sum ++;
                    } 
                }
                if((i + 1) % N != 0) // UP-RIGHT.
                {                                                       
                    if(A_loc[i - N + 1] == 1)
                    {
                        Sum ++;
                    }
                }   
            }
            else
            {
                if(Up >= 0)
                {
                    if(Data_receive_from_Up[i] == 1) // UP from the other process.
                    {
                        Sum ++;
                    }
                    if(i - 1 >= 0)
                    {
                        if(Data_receive_from_Up[i - 1] == 1) // UP-LEFT from the other process.
                        {
                            Sum ++;
                        }
                    }
                    if(i + 1 < N )
                    {
                        if(Data_receive_from_Up[i + 1] == 1) // UP-RIGHT from the other process.
                        {
                            Sum ++;
                        }
                    }  
                }
            }
            
            if(Load - i > N) // DOWN.
            {
                if(A_loc[i + N] == 1)
                {
                    Sum ++;
                }
                if(i % N != 0) // DOWN-LEFT.
                {
                    if(A_loc[i + N - 1] == 1)
                    {
                        Sum ++;
                    }
                }
                if((i + 1) % N != 0) // DOWN-RIGHT.
                {
                    if(A_loc[i + N + 1] == 1)
                    {
                        Sum ++;
                    }
                }   
            }
            else
            {
                if(Down > 0)
                {
                    if(Data_receive_from_Down[i % N] == 1) // DOWN from the other process.
                    {
                        Sum ++;
                    }
                    if((i%N)-1 >= 0)
                    {
                        if(Data_receive_from_Down[(i % N) - 1] == 1) // DOWN-LEFT from the other process.
                        {
                            Sum ++;
                        }
                    }
                    if((i % N) + 1 < N )
                    {
                        if(Data_receive_from_Down[(i % N) + 1] == 1) // DOWN-RIGHT from the other process.
                        {
                            Sum ++;
                        }
                    } 
                }
            }

            if(i % N != 0) // LEFT.
            {
                if(A_loc[i - 1] == 1)
                {
                    Sum ++;
                }
            }

            if((i + 1) % N != 0) // RIGHT.
            {
                if(A_loc[i + 1] == 1)
                {
                    Sum ++;
                }
            }

            if(Sum >= 4) // Overpopulation.
            {
                if(A_loc[i] == 1)
                {
                    temp[i] = 0;
                    overpopulation ++;
                }
            }
            else if(Sum <= 1) // Loneliness.
            {
                if(A_loc[i] == 1)
                {
                    temp[i] = 0;
                    loneliness ++;
                }
            }
            else if(Sum == 3) // Birth.
            {
                if(A_loc[i] == 0)
                {
                    temp[i] = 1;
                    birth ++;
                }
            }
        }

        MPI_Gather(temp, Load, MPI_INT, A, Load, MPI_INT, 0, MPI_COMM_WORLD); // Get the Data.
        MPI_Reduce (&overpopulation, &sum_of_overpopulation, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce (&loneliness, &sum_of_loneliness, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce (&birth, &sum_of_birth, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if(rank == 0)
        {
            printf("\n\nGENERATION %d\n\n", Current_Generation + 1);
            for(i = 0; i < N * N; i ++)
            {
                printf(" |%d|",A[i]); // Print the matrix.
                if(((i + 1) % N) == 0)
                {
                    printf("\n");
                }
                if(A[i] == 1)
                {
                    alive_cells ++;
                }
            }
            printf("\n\n");
            printf("Alive Cells: %d\n", alive_cells); // Print the Data of each generation.
            printf("Cells that died from overpopulation: %d\n", sum_of_overpopulation); 
            printf("Cells that died from loneliness: %d\n", sum_of_loneliness);
            printf("Cells that were born in this generation: %d\n", sum_of_birth);
        } 
    }
    free(A_loc); // Free the matrices for every process.
    free(temp);
    free(Data_for_Up);
    free(Data_for_Down);
    free(Data_receive_from_Up);
    free(Data_receive_from_Down);
    if(rank == 0)
    {
        free(A); // Free the matrix for master process.
    }

    MPI_Finalize();
    return 0;
}