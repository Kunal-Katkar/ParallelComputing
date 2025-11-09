# Stock Price Simulation - Geometric Brownian Motion

## Overview
Simulates stock price movements using **Geometric Brownian Motion (GBM)** with parallel computing (OpenMP). Used in finance for option pricing, risk analysis, and Monte Carlo simulations.

## How It Works

**3-Stage Pipeline:**

1. **Stage 1 - Random Generation**: Generates normally distributed random numbers (parallel processing)
2. **Stage 2 - Prefix Sum**: Computes cumulative sums to simulate Wiener process (Brownian motion)
3. **Stage 3 - Price Calculation**: Applies GBM formula to calculate stock prices over time

**Formula**: `S(t) = S₀ × exp((μ - σ²/2) × t + σ × W(t))`
- S₀ = Initial price, μ = Drift, σ = Volatility, W(t) = Wiener process

## Execution

```bash
# Stage 1: Generate random numbers
BrownianStockMovement1.exe
# Input: length (e.g., 100)

# Stage 2: Compute prefix sums  
BrownianStockMovement2.exe
# Reads: RNG.txt → Outputs: RNG2.txt

# Stage 3: Calculate stock prices
BrownianStockMovement3.exe  
# Input: drift (%), volatility (%), initial price ($)
# Example: 10, 20, 100
```

## Output Files
- `RNG.txt` - Normal random numbers
- `RNG2.txt` - Wiener process (cumulative sums)
- `StockPrice.txt` - Simulated stock prices

## Technical Stack
- **Language**: C with OpenMP
- **Method**: Monte Carlo simulation
- **Algorithm**: Parallel prefix sum (Brent-Kung)
- **Capacity**: Up to 100,000 time steps

## Test Cases

### Test Case 1: Small Simulation (10 time steps)

#### Stage 1: BrownianStockMovement1.exe
```
Input:
  Enter length : 10

Expected Output:
  Generated random numbers:
    [0]: -0.079374
    [1]: -2.487103
    [2]: -1.430860
    ...
  ✓ Generated 10 random numbers and saved to RNG.txt
```

#### Stage 2: BrownianStockMovement2.exe
```
Input:
  (No input required - automatically reads RNG.txt)

Expected Output:
  Stage 2: Starting prefix sum calculation...
  Read 10 numbers from RNG.txt
  
  Computed prefix sums (first 5 and last 5):
    [0]: -7.937402e-02
    [1]: -2.566477e+00
    [2]: -3.997337e+00
    ...
  ✓ Computed prefix sums and saved to RNG2.txt
```

#### Stage 3: BrownianStockMovement3.exe
```
Input:
  Enter the yearly drift in percentage : 10.5
  Enter the yearly volatility in percentage : 25.75
  Enter initial stock price : $125.50

Expected Output:
  Input values received:
    Yearly drift: 10.50%
    Yearly volatility: 25.75%
    Initial stock price: $125.50
  Read 10 prefix sum values from RNG2.txt
  Daily drift: 4.166667e-04
  Daily volatility: 1.622146e-02
  Adjusted drift mean: 3.331545e-04
  
  Stock prices (first 5 and last 5):
    Day 0: $125.43
    Day 1: $121.85
    Day 2: $119.55
    ...
  ✓ Stock prices calculated and saved to StockPrice.txt
```

### Test Case 2: Medium Simulation (100 time steps)

#### Stage 1: BrownianStockMovement1.exe
```
Input:
  Enter length : 100

Expected Output:
  Generated 100 random numbers and saved to RNG.txt
```

#### Stage 2: BrownianStockMovement2.exe
```
Input:
  (No input required)

Expected Output:
  Read 100 numbers from RNG.txt
  ✓ Computed prefix sums and saved to RNG2.txt
  Total values: 100
```

#### Stage 3: BrownianStockMovement3.exe
```
Input:
  Enter the yearly drift in percentage : 8.25
  Enter the yearly volatility in percentage : 18.50
  Enter initial stock price : $50.75

Expected Output:
  Input values received:
    Yearly drift: 8.25%
    Yearly volatility: 18.50%
    Initial stock price: $50.75
  Read 100 prefix sum values from RNG2.txt
  Daily drift: 3.273810e-04
  Daily volatility: 1.165948e-02
  Adjusted drift mean: 2.653797e-04
  
  Stock prices (first 5 and last 5):
    Day 0: $50.73
    Day 1: $50.45
    ...
    Day 99: $51.25
  ✓ Stock prices calculated and saved to StockPrice.txt
  Total days: 100
```

### Test Case 3: Large Simulation (1000 time steps)

#### Stage 1: BrownianStockMovement1.exe
```
Input:
  Enter length : 1000

Expected Output:
  Generated 1000 random numbers and saved to RNG.txt
```

#### Stage 2: BrownianStockMovement2.exe
```
Input:
  (No input required)

Expected Output:
  Read 1000 numbers from RNG.txt
  ✓ Computed prefix sums and saved to RNG2.txt
  Total values: 1000
```

#### Stage 3: BrownianStockMovement3.exe
```
Input:
  Enter the yearly drift in percentage : 12.75
  Enter the yearly volatility in percentage : 30.25
  Enter initial stock price : $200.00

Expected Output:
  Input values received:
    Yearly drift: 12.75%
    Yearly volatility: 30.25%
    Initial stock price: $200.00
  Read 1000 prefix sum values from RNG2.txt
  Daily drift: 5.059524e-04
  Daily volatility: 1.906250e-02
  Adjusted drift mean: 3.742188e-04
  
  Stock prices (first 5 and last 5):
    Day 0: $199.92
    Day 1: $198.45
    ...
    Day 999: $205.78
  ✓ Stock prices calculated and saved to StockPrice.txt
  Total days: 1000
```

### Test Case 4: High Volatility Scenario

#### Stage 1: BrownianStockMovement1.exe
```
Input:
  Enter length : 50
```

#### Stage 2: BrownianStockMovement2.exe
```
Input:
  (No input required)
```

#### Stage 3: BrownianStockMovement3.exe
```
Input:
  Enter the yearly drift in percentage : 15.0
  Enter the yearly volatility in percentage : 45.5
  Enter initial stock price : $75.25

Expected Output:
  Input values received:
    Yearly drift: 15.00%
    Yearly volatility: 45.50%
    Initial stock price: $75.25
  Read 50 prefix sum values from RNG2.txt
  Daily drift: 5.952381e-04
  Daily volatility: 2.867725e-02
  Adjusted drift mean: 1.842262e-04
  
  ✓ Stock prices calculated and saved to StockPrice.txt
  Total days: 50
```

### Notes on Test Cases
- **Floating Point Inputs**: All inputs accept floating point numbers (e.g., 10.5, 25.75, 125.50)
- **Stage 1**: Accepts integer for length (1-100000)
- **Stage 2**: No input required, automatically processes RNG.txt
- **Stage 3**: Accepts floating point for drift (%), volatility (%), and initial price ($)
- **Output Files**: Each stage generates files that are used by the next stage
- **Verification**: Check output files (RNG.txt, RNG2.txt, StockPrice.txt) to verify results

