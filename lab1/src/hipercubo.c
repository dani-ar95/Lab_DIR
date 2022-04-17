//compilar: mpicc hipercubo.c -o hipercubo -lm
//run: mpirun --mca btl tcp,self -n 16 --hostfile host hipercubo

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#define FICHERO "datos.dat"
#define N_DATOS 16
#define N_BITS (int)sqrt(N_DATOS)

void procesar_fichero(int rank, float *v_datos);
void checkear_parametros(int rank, int numtasks);
void encontrar_max(float *numero, int rank);
void encontrar_vecino(int *vecino, int i, int rank);

int main(argc, argv)
int argc;
char **argv;
{
    int rank, numtasks;
    float v_datos[N_DATOS];
    float numero;

    MPI_Init( &argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    checkear_parametros(rank, numtasks);
    procesar_fichero(rank, v_datos);

    MPI_Scatter(&v_datos, 1, MPI_FLOAT, &numero, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    encontrar_max(&numero, rank);

    if(rank == 0)
        printf("El número más grande es: %.2f\n", numero);

    MPI_Finalize();
    return 0;
}

void procesar_fichero(int rank, float *v_datos){

    char *linea;
    char *token;
    size_t len = 0;

    if (rank == 0) {
        FILE *fp = fopen(FICHERO, "r");

        if (!fp)
        {
            fprintf(stderr, "Error abriendo el fichero %s: %s.\n", FICHERO, strerror(errno));
            exit(EXIT_FAILURE);
        }

        getline(&linea, &len, fp);
        token = strtok(linea, ",");
        v_datos[0] = atof(token);
        int i = 1;

        while( token != NULL ) 
        {
            token = strtok(NULL, ",");
            if (token ==  NULL) break;
            v_datos[i] = atof(token);
            i++;
        }
        fclose(fp);
    }
}

void checkear_parametros(int rank, int numtasks){
    if (rank == 0 && numtasks != N_DATOS) {
        printf("Numero de tareas = %d\n",numtasks);
        printf("Necesitas lanzar %d tareas\n", N_DATOS);
        MPI_Abort(MPI_COMM_WORLD, 0);
        exit(EXIT_FAILURE);
    }
}

void encontrar_max(float *numero, int rank){
    float max;
    MPI_Status status;
    int vecino;
    for(int i = 0; i < N_BITS; i++){
        vecino = rank^(int)pow(2, i);
        MPI_Send(&(*numero), 1, MPI_FLOAT, vecino, 0, MPI_COMM_WORLD);
        MPI_Recv(&max, 1, MPI_FLOAT, vecino, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(max > (*numero)){
            (*numero) = max;
        }
    }
}