#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>

int THREADS_COUNT;
int ARRAY_SIZE;

int * arrayToSort;
int * resultingArray;
int swapped = 1;
pthread_mutex_t lock;

struct ThreadArguments {
	int offset;
	int size;
};

void * insertionSort(void * arg) {
	struct ThreadArguments * thr_arg = arg;
	int offset = (int) thr_arg->offset;
	int size = (int) thr_arg->size;
	int limit = offset + size;
	
	int i, key, j;
	
	for (i = offset; i < limit; i++) {
		key = arrayToSort[i];
		j = i-1;

		while (j >= offset && arrayToSort[j] > key) {
			//pthread_mutex_lock(&lock);
			arrayToSort[j+1] = arrayToSort[j];
			j = j-1;
			swapped = 1;
			//pthread_mutex_unlock(&lock);
		}
		arrayToSort[j+1] = key;
	}
}
 
int main(int argc, char *argv[]) {
	int i, resultArrayIndex, SHOW_ARRAYS = 0, maxValue, maxSubArrayIndex, j;
	int * THREADS_INDEXES;
	struct timeval startT, endT;
	double elapsed;
	pthread_t threads[THREADS_COUNT];
	struct ThreadArguments *args;
	
	if(argc < 3) {
		printf("%s <THREADS_COUNT> <ARRAY_SIZE> [<SHOW_ARRAYS>]\n", argv[0]);
		exit(1);
	}
	if(argc == 4) SHOW_ARRAYS = 1;
	 
	THREADS_COUNT = atoi(argv[1]);
	ARRAY_SIZE = atoi(argv[2]);
	resultArrayIndex = ARRAY_SIZE - 1;
	
	arrayToSort = malloc(sizeof(int)*ARRAY_SIZE);
	resultingArray = malloc(sizeof(int)*ARRAY_SIZE);
	THREADS_INDEXES = malloc(sizeof(int)*THREADS_COUNT);
	
	int partSize = floor((double) ARRAY_SIZE / THREADS_COUNT);
	int lastPartSize = ARRAY_SIZE - partSize * THREADS_COUNT;

	for (i = 0; i < ARRAY_SIZE; ++i)
		arrayToSort[i] = i;
	
    srand(time(NULL));
	
    for(i = ARRAY_SIZE - 1; i > 0; i--) {
        int j = rand() % (i+1);
		int temp = arrayToSort[i];
		arrayToSort[i] = arrayToSort[j];
		arrayToSort[j] = temp;
    }

	if (SHOW_ARRAYS) {
		printf("Unsorted array: \n");
		for (i = 0; i < ARRAY_SIZE; i++)
			printf ("%d ", arrayToSort[i]);
		printf("\n\n");
	}

	gettimeofday(&startT, NULL);
	printf("Start sorting\n");
	
	for(i = 0; i < THREADS_COUNT; i++) {
		args = malloc(sizeof(struct ThreadArguments));
		(*args).offset = i * partSize;
		(*args).size = partSize;
		if (i == THREADS_COUNT - 1) {
			(*args).size += lastPartSize;
			pthread_create(&threads[i], NULL, insertionSort, args);
		} else {
			pthread_create(&threads[i], NULL, insertionSort, args);
		}
		printf("%d - %d (%d)\n", (*args).offset, (*args).offset + (*args).size, (*args).size);
		THREADS_INDEXES[i] = (*args).size;
	}
	
	for (i = 0; i < THREADS_COUNT; i++)
		pthread_join(threads[i], NULL);
	
	//pthread_mutex_destroy(&lock);
	
	gettimeofday(&endT, NULL);
	elapsed = (double)(endT.tv_usec - startT.tv_usec) / 1000000 + (double)(endT.tv_sec - startT.tv_sec);
	printf("Done in %f ms\n", elapsed);

	if (SHOW_ARRAYS) {
		printf("\nSorted array: \n");
		for (i = 0; i < ARRAY_SIZE; i++)
			printf ("%d ", arrayToSort[i]);
		printf("\n");
	}
	
	//arrayToSort[(int) floor(ARRAY_SIZE/2)] = 0;
	
	gettimeofday(&startT, NULL);
	
	if (THREADS_COUNT > 1) {
		printf("\nMerging results\n");
		
		for(; resultArrayIndex >= 0; resultArrayIndex--) {
			maxSubArrayIndex = maxValue = -1;
			for(i = THREADS_COUNT - 1; i >= 0; i--) {
				if (THREADS_INDEXES[i]) {
					j = THREADS_INDEXES[i] - 1 + partSize * i;
					if(arrayToSort[j] > maxValue) {
						maxValue = arrayToSort[j];
						maxSubArrayIndex = i;
					}
				}
			}
			THREADS_INDEXES[maxSubArrayIndex]--;
			resultingArray[resultArrayIndex] = maxValue;
		}
		
		gettimeofday(&endT, NULL);
		elapsed = (double)(endT.tv_usec - startT.tv_usec) / 1000000 + (double)(endT.tv_sec - startT.tv_sec);
		printf("Done in %f ms\n", elapsed);

		arrayToSort = resultingArray;
	} else {
		printf("\nOnly 1 thread, no merge needed\n");
	}
	
	if (SHOW_ARRAYS) {
		printf("\nMerged array: \n");
		for (i = 0; i < ARRAY_SIZE; i++)
			printf ("%d ", resultingArray[i]);
		printf("\n");
	}
	
	printf("\nStart checking\n");
	swapped = 0;
	args = malloc(sizeof(struct ThreadArguments));
	(*args).offset = 0;
	(*args).size = ARRAY_SIZE;
	insertionSort(args);
	
	printf("Checking %s!\n\n", swapped ? "failed" : "passed");

	return 0;
}