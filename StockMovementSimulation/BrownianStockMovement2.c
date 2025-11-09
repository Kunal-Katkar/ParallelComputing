#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <limits.h>
#include <string.h>

int main()
{
	printf("Stage 2: Starting prefix sum calculation...\n");
	fflush(stdout);
	
	// Use static arrays to avoid stack overflow (large arrays ~4.8MB)
	static long double randomnum[2][100000];
	static long double prefixsum[100000];
	int i=0,len=0;
	FILE *myFile = fopen("RNG.txt", "r");
	if (myFile == NULL) {
		printf("Error: Could not open RNG.txt\n");
		printf("Please run BrownianStockMovement1.exe first to generate RNG.txt\n");
		return 1;
	}
	
	// Read file properly - check fscanf return value instead of feof
	// Read as double first (Stage 1 writes doubles), then convert to long double
	double number_double;
	while (fscanf(myFile, "%lf", &number_double) == 1) {
		if (len >= 100000) {
			printf("Error: Too many numbers! Maximum is 100000\n");
			fclose(myFile);
			return 1;
		}
		randomnum[0][len++] = (long double)number_double;
	}
	fclose(myFile);
	
	if (len == 0) {
		printf("Error: RNG.txt is empty or contains invalid data\n");
		return 1;
	}
	
	printf("Read %d numbers from RNG.txt\n", len);
	fflush(stdout);
	int n=len, flag, size=0, j;
	long double *t;
  //printf("Enter length:");
  //scanf("%d", &n);
  //printf("Enter numbers:\n", n);
  //for(i = 0; i < n; i++)
  //  scanf("%d", &ar[0][i]);
  /*set up complement to array that holds values*/
  i = len;
  flag = 1;
  /*copy first value, since it is not copied by the code*/
  randomnum[1][0] = randomnum[0][0];
  /*following loop aims to get log2 of size, but can be avoided as in 2nd program*/
  while(i) {
    size++;
    i >>= 1;
  }
  /*following code implements algorithm*/
  for(j = 0; j < size; j++) {
    flag = !flag;
    if(flag) t = randomnum[0];
    else t = randomnum[1];
#pragma omp parallel for default(none) private(i) shared(n, j, t, randomnum, flag)
    for(i = 1; i < n; i++) {
      if(i - (1 << j) >= 0)
    t[i] = randomnum[flag][i] + randomnum[flag][i - (1 << j)];
      else t[i] = randomnum[flag][i];
    }
  }
  flag = !flag;
  
  // Verify we have valid data
  if (n <= 0) {
    printf("Error: No data to process\n");
    return 1;
  }
  
  FILE *fp = fopen("RNG2.txt", "w");
  if (fp == NULL) {
    printf("Error: Could not create RNG2.txt\n");
    return 1;
  }
  
  printf("\nComputed prefix sums (first 5 and last 5):\n");
  for(i = 0; i < n && i < 5; i++)
    printf("  [%d]: %.12e\n", i, (double)randomnum[flag][i]);
  if (n > 10) {
    printf("  ...\n");
  }
  for(i = (n > 5 ? n - 5 : 5); i < n; i++)
    printf("  [%d]: %.12e\n", i, (double)randomnum[flag][i]);
  
  // Write to file
  for(i = 0; i < len - 1; i++) {
    fprintf(fp, "%.15e\n", (double)randomnum[flag][i]);
  }
  fprintf(fp, "%.15e", (double)randomnum[flag][len - 1]);
  fclose(fp);
  
  printf("\n✓ Computed prefix sums and saved to RNG2.txt\n");
  printf("  Total values: %d\n", len);
  fflush(stdout);
  
  return 0;
}



