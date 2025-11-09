#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <string.h>

int main()
{
	// Use static arrays to avoid stack overflow
	static long double stockprice[100000];
	static long double prefixsum[100000];
	
	long double drift_year, volatility_year, volatility_day, drift_day, drift_mean, stockprice_initial;
	double drift_year_d, volatility_year_d, stockprice_initial_d;  // Temporary double for input
	int i, len = 0;
	
	printf("Enter the yearly drift in percentage : ");
	fflush(stdout);
	if (scanf("%lf", &drift_year_d) != 1) {
		printf("Error: Invalid input for drift\n");
		return 1;
	}
	drift_year = (long double)drift_year_d;
	
	printf("Enter the yearly volatility in percentage : ");
	fflush(stdout);
	if (scanf("%lf", &volatility_year_d) != 1) {
		printf("Error: Invalid input for volatility\n");
		return 1;
	}
	volatility_year = (long double)volatility_year_d;
	
	printf("Enter initial stock price : $");
	fflush(stdout);
	if (scanf("%lf", &stockprice_initial_d) != 1) {
		printf("Error: Invalid input for stock price\n");
		return 1;
	}
	stockprice_initial = (long double)stockprice_initial_d;
	
	// Debug: Verify inputs were read correctly
	printf("\nInput values received:\n");
	printf("  Yearly drift: %.2f%%\n", (double)drift_year);
	printf("  Yearly volatility: %.2f%%\n", (double)volatility_year);
	printf("  Initial stock price: $%.2f\n", (double)stockprice_initial);
	fflush(stdout);
	
	FILE *myFile = fopen("RNG2.txt", "r");
	if (myFile == NULL) {
		printf("Error: Could not open RNG2.txt\n");
		printf("Please run BrownianStockMovement2.exe first to generate RNG2.txt\n");
		return 1;
	}
	
	// Read file properly - check fscanf return value
	double number_double;
	while (fscanf(myFile, "%lf", &number_double) == 1) {
		if (len >= 100000) {
			printf("Error: Too many numbers! Maximum is 100000\n");
			fclose(myFile);
			return 1;
		}
		prefixsum[len++] = (long double)number_double;
	}
	fclose(myFile);
	
	if (len == 0) {
		printf("Error: RNG2.txt is empty or contains invalid data\n");
		return 1;
	}
	
	printf("Read %d prefix sum values from RNG2.txt\n", len);
	fflush(stdout);
	drift_day = drift_year / (100.0 * 252.0);
	volatility_day = volatility_year / (100.0 * sqrt(252.0));
	drift_mean = drift_day - (0.5 * pow(volatility_day, 2));
	
	printf("Daily drift: %.6e\n", (double)drift_day);
	printf("Daily volatility: %.6e\n", (double)volatility_day);
	printf("Adjusted drift mean: %.6e\n", (double)drift_mean);
	fflush(stdout);
	
	// Calculate stock prices using Geometric Brownian Motion
	// Note: On some systems, expl() might not be available, so we use exp() with double conversion
	// The formula: S(t) = S0 * exp((μ - σ²/2) * t + σ * W(t))
#pragma omp parallel for default(none) private(i) shared(drift_mean, volatility_day, stockprice, stockprice_initial, len, prefixsum)
	for(i = 0; i < len; i++) {
		long double time_term = drift_mean * (long double)(i + 1);
		long double wiener_term = volatility_day * prefixsum[i];
		long double exponent = time_term + wiener_term;
		// Use exp() with double conversion for compatibility (expl may not be available on all systems)
		stockprice[i] = stockprice_initial * (long double)exp((double)exponent);
	}
	
	printf("\nStock prices (first 5 and last 5):\n");
	for(i = 0; i < len && i < 5; i++) {
		printf("  Day %d: $%.2f\n", i, (double)stockprice[i]);
	}
	if (len > 10) {
		printf("  ...\n");
	}
	for(i = (len > 5 ? len - 5 : 5); i < len; i++) {
		printf("  Day %d: $%.2f\n", i, (double)stockprice[i]);
	}
	fflush(stdout);
	
	FILE *fp = fopen("StockPrice.txt", "w");
	if (fp == NULL) {
		printf("Error: Could not create StockPrice.txt\n");
		return 1;
	}
	
	for(i = 0; i < len - 1; i++) {
		fprintf(fp, "%.15e\n", (double)stockprice[i]);
	}
	fprintf(fp, "%.15e", (double)stockprice[len - 1]);
	fclose(fp);
	
	printf("\n✓ Stock prices calculated and saved to StockPrice.txt\n");
	printf("  Total days: %d\n", len);
	fflush(stdout);
	
	return 0;
}
