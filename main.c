#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

int A[20][20];
int B[20][20];
int C[20][20];
int row[3];
int col[3];
int count = 0;
char file[3][50] = {"a","b","c"};
struct Element {
    int row;
    int col;
};

void readArray(FILE* myFile, int row, int col, int n) {
    if(n == 1)
        for(int i = 0; i < row; i++)
            for(int j = 0; j < col; j++)
                fscanf(myFile,"%d", &A[i][j]);
    else
        for(int i=0;i<row; i++)
            for(int j=0;j<col;j++)
                fscanf(myFile,"%d", &B[i][j]);
}

void readFile(int n) {
    FILE* myFile = fopen(file[n-1], "r");
    if(myFile ==  NULL) {
        printf("Wrong name for file%d\n",n);
        exit(1);
    }
    fscanf(myFile, "row=%d col=%d",&row[n], &col[n]);
    readArray(myFile, row[n], col[n], n);
    fclose(myFile);
}

void readInputFromFiles(int argc, char* argv[]) {
    for(int i = 1; i < argc; i++)
        strcpy(file[i-1], argv[i]);
    strcat(file[0], ".txt");
    strcat(file[1], ".txt");
    readFile(1);
    readFile(2);
    if(col[1] != row[2]) {
        printf("error! can't multiply matrices as column1 != row2\n");
        exit(1);
    }
}

void writeToFile(char nameExtent[], char type[]) {
    char fileName[50];
    strcpy(fileName, file[2]);
    strcat(fileName, nameExtent);
    FILE* myFile = fopen(fileName, "w");
    fprintf(myFile, "Method: A thread per %s\n", type);
    fprintf(myFile, "row=%d col=%d\n", row[1], col[2]);
    for(int i=0;i<row[1];i++) {
        for(int j=0;j<col[2];j++)
            fprintf(myFile, "%d ", C[i][j]);
        fprintf(myFile, "\n");
    }
}

void ThreadPerMatrix() {
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    for(int i = 0;i < row[1]; i++)
        for(int j = 0; j < col[2]; j++)
            for(int k = 0; k < col[1]; k++)
                C[i][j] += A[i][k] * B[k][j];
    count++;
    gettimeofday(&stop, NULL);
    printf("thread_per_matrix method created %d threads\n",count);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);
    writeToFile("_per_matrix.txt", "matrix");
    memset(C, 0, sizeof(C));
    count = 0;
}

void* multiplyThreadPerRow(void* r) {
    int* row = (int*) r;
    for(int i = 0; i < col[2]; i++)
        for(int j = 0; j < col[1]; j++)
            C[*row][i] += A[*row][j] * B[j][i];
    free(row);
    return NULL;
}

void ThreadPerRow() {
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    pthread_t* threadArray = (pthread_t*)malloc(row[1] * sizeof(pthread_t));
    for(int i = 0; i < row[1]; i++) {
        int *row = (int*)malloc(sizeof(int));
        *row = i;
        pthread_create(&threadArray[i], NULL, multiplyThreadPerRow,(void*)row);
        count++;
    }
    for (int i = 0; i < row[1]; i++)
        pthread_join(threadArray[i], NULL);
    free(threadArray);
    gettimeofday(&stop, NULL);
    printf("thread_per_row method created %d threads\n",count);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);
    writeToFile("_per_row.txt", "row");
    memset(C, 0, sizeof(C));
    count = 0;
}

void* multiplyThreadPerElement(void* elem) {
    struct Element* element = (struct Element*) elem;
    for(int i = 0; i < col[1]; i++)
        C[(*element).row][(*element).col] += A[(*element).row][i] * B[i][(*element).col];
    free(elem);
}

void ThreadPerElement() {
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    pthread_t** threadArray = (pthread_t**)malloc(row[1] * sizeof(pthread_t*));
    for(int i = 0; i < row[1]; i++){
        threadArray[i] = (pthread_t*)malloc(col[2] * sizeof(pthread_t));
        for(int j = 0; j < col[2]; j++) {
            struct Element* element = (struct Element*)malloc(sizeof(struct Element));
            (*element).row = i;
            (*element).col = j;
            pthread_create(&threadArray[i][j], NULL, multiplyThreadPerElement,(void*)element);
            count++;
        }
    }
    for(int i = 0; i < row[1]; i++)
        for(int j = 0; j < col[2]; j++) pthread_join(threadArray[i][j], NULL);
    for(int i=0;i<row[1];i++) free(threadArray[i]);
    free(threadArray);
    gettimeofday(&stop, NULL);
    printf("thread_per_element method created %d threads\n",count);
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n\n", stop.tv_usec - start.tv_usec);
    writeToFile("_per_element.txt", "element");
}

int mainThread(int argc, char* argv[]) {
    readInputFromFiles(argc, argv);
    ThreadPerMatrix();
    ThreadPerRow();
    ThreadPerElement();
    return 0;
}

int main(int argc, char* argv[])
{
    return mainThread(argc, argv);
}
