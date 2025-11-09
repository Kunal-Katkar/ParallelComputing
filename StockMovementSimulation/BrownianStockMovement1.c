#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include "../normsinv.h"

// Windows-compatible thread-safe random number generator
#ifdef _WIN32
// Thread-local storage for random state
static __thread unsigned int thread_seed = 0;

int rand_r_windows(unsigned int *seed) {
    if (*seed == 0) *seed = 1;
    *seed = (*seed * 1103515245U + 12345U) & 0x7FFFFFFF;
    return (int)(*seed >> 16) & 0x7FFF;
}
#define rand_r rand_r_windows
#endif

int main()
{
	int len, i;
	long double randomnum[100000];
	printf("Enter length : ");
	scanf("%d", &len);
	
	if (len <= 0 || len > 100000) {
		printf("Invalid length! Must be between 1 and 100000\n");
		return 1;
	}
	
	// Initialize main seed
	srand((unsigned int)time(NULL));
	
#pragma omp parallel default(none) shared(randomnum, len) private(i)
    {	
		// Each thread gets its own seed based on thread ID
		unsigned int myseed = (unsigned int)(time(NULL) ^ (omp_get_thread_num() << 16) ^ (omp_get_thread_num() * 12345));
		if (myseed == 0) myseed = 1;  // Ensure non-zero seed
		
#pragma omp for
		for(i = 0; i < len; i++) {
			// Generate random number in (0, 1) range for normsinv
			int r = rand_r(&myseed);
			long double temp = ((long double)r) / 32767.0;
			
			// Ensure temp is in valid range for normsinv: (0, 1)
			if (temp <= 0.0) temp = 0.0001;
			if (temp >= 1.0) temp = 0.9999;
			
			randomnum[i] = normsinv(temp);
		}
    }
    
    // Print results (convert to double for MinGW printf compatibility)
    printf("\nGenerated random numbers:\n");
    for(i = 0; i < len && i < 10; i++) {
        printf("  [%d]: %.6f\n", i, (double)randomnum[i]);
    }
    if (len > 10) {
        printf("  ... (%d more)\n", len - 10);
    }
    
    // Write to file (convert to double for file I/O compatibility)
    FILE *fp = fopen("RNG.txt", "w");
    if (fp == NULL) {
		printf("\nError: Could not create RNG.txt\n");
		return 1;
	}
	
    for(i = 0; i < len - 1; i++) {
        fprintf(fp, "%.15e\n", (double)randomnum[i]);
    }
    fprintf(fp, "%.15e", (double)randomnum[len - 1]);
    fclose(fp);
    
    printf("\n✓ Generated %d random numbers and saved to RNG.txt\n", len);
    printf("  File format: One number per line (scientific notation)\n");
    
    return 0;
}
