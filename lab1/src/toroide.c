// compilar: mpicc toroide.c -o toroide
// run: mpirun --mca btl tcp,self -n 16 --hostfile host toroide

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#define FICHERO "datos.dat"
#define N_DATOS 16
#define L (int)sqrt(N_DATOS)

void procesar_fichero(int rank, float *v_datos);
void comprobar_parametros(int rank, int numtasks);
void encontrar_vecinos(int *norte, int *sur, int *este, int *oeste, int rank);
void encontrar_min(float *numero, int norte, int sur, int este, int oeste);

int main(argc, argv)
int argc;
char **argv;
{
    int rank, numtasks;
    float v_datos[N_DATOS];
    float numero;
    int norte, sur, este, oeste;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    comprobar_parametros(rank, numtasks);
    procesar_fichero(rank, v_datos);

    MPI_Scatter(&v_datos, 1, MPI_FLOAT, &numero, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    encontrar_vecinos(&norte, &sur, &este, &oeste, rank);
    encontrar_min(&numero, norte, sur, este, oeste);

    if (rank == 0)
        printf("El número más pequeño es: %.2f\n", numero);

    MPI_Finalize();
    return 0;
}

void procesar_fichero(int rank, float *v_datos)
{

    char *linea;
    char *token;
    size_t len = 0;

    if (rank == 0)
    {
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

        while (token != NULL)
        {
            token = strtok(NULL, ",");
            if (token == NULL)
                break;
            v_datos[i] = atof(token);
            i++;
        }
        fclose(fp);
    }
}

void comprobar_parametros(int rank, int numtasks)
{
    if (rank == 0 && numtasks != N_DATOS)
    {
        printf("Numero de tareas = %d\n", numtasks);
        printf("Necesitas lanzar %d tareas\n", N_DATOS);
        MPI_Abort(MPI_COMM_WORLD, 0);
        exit(EXIT_FAILURE);
    }
}

void encontrar_vecinos(int *norte, int *sur, int *este, int *oeste, int rank)
{

    (*norte) = rank + L;
    (*sur) = rank - L;
    (*este) = rank + 1;
    (*oeste) = rank - 1;

    if ((*norte) >= N_DATOS) // Primera fila
        (*norte) -= N_DATOS;
    if ((*sur) < 0) //Última fila
        (*sur) = rank - (L - N_DATOS);
    if (rank % L == 0) // Primera columna
        (*oeste) = rank + (L - 1);
    if (rank % L == (L - 1)) //Última columna
        (*este) = rank - (L - 1);
}

void encontrar_min(float *numero, int norte, int sur, int este, int oeste)
{
    float min;
    MPI_Status status;
    for (int i = 0; i < L; i++)
    {
        MPI_Send(&(*numero), 1, MPI_FLOAT, este, 0, MPI_COMM_WORLD);
        MPI_Recv(&min, 1, MPI_FLOAT, oeste, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (min < (*numero))
        {
            (*numero) = min;
        }
    }
    for (int i = 0; i < L; i++)
    {
        MPI_Send(&(*numero), 1, MPI_FLOAT, norte, 0, MPI_COMM_WORLD);
        MPI_Recv(&min, 1, MPI_FLOAT, sur, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (min < (*numero))
        {
            (*numero) = min;
        }
    }
}