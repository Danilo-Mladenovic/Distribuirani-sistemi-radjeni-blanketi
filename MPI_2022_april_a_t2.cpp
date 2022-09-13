#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define k 3
#define l 4
#define q 2

int main(int argc, char** argv)
{
    int size, rank, root = 0;
    MPI_Request req;
    MPI_Status stat;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int A[k][l], b[l], c[k];
    int local_row_sum[k], final_row_sum[k];
    int local_A[k][q], local_b[q], local_c[k];

    struct {
        int value;
        int rank;
    } in = { INT_MIN, rank }, out;


    if (rank == root)
    {
        for (int i = 0; i < k; i++)
            for (int j = 0; j < l; j++)
                A[i][j] = i + j;

        for (int i = 0; i < l; i++)
            b[i] = i;
    }

    for (int i = 0; i < k; i++)
    {
        local_c[i] = 0;
        local_row_sum[i] = 0;
    }

    if (rank == root)
        for (int i = 0; i < size; i++)
            for (int j = 0; j < k; j++)
                MPI_Isend(&A[j][i * q], q, MPI_INT, i , 0, MPI_COMM_WORLD, &req);
    
    for (int i =0; i < k; i++)
        MPI_Recv(&local_A[j][0], q, MPI_INT, root, 0, MPI_COMM_WORLD, &stat);

    MPI_Scatter(&b[0], q, MPI_INT, &local_b[0], q, MPI_INT, root, MPI_COMM_WORLD);


    for (int i = 0; i < k; i++)
        for (int j = 0; j < q; j++)
        {
            local_c[i] = local_c[i] + local_A[i][j] * local_b[j];
            local_row_sum[i] += local_A[i][j];

            if (local_A[i][j] > in.value)
                in.value = local_A[i][j];
        }


    MPI_Reduce(&in, &out, 1, MPI_2INT, MPI_MAXLOCK, root, MPI_COMM_WORLD);
    MPI_Bcast(&out, 1, MPI_2INT, root, MPI_COMM_WORLD);

    MPI_Reduce(&local_c[0], &local_c[0], q, MPI_INT, out.rank, MPI_COMM_WORLD);
    MPI_Reduce(&local_row_sum, &final_row_sum, 1, MPI_INT, out.rank, MPI_COMM_WORLD);


    if (rank == out.rank)
    {
        for (int i = 0; i < k; i++)
            printf("%d\n", c[i]);
        
        for (int i = 0; i < k; i++)
            printf("%d\n", final_row_sum[i]);
    }

    MPI_Finalize();
    return 0;
}