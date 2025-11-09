# Parallel Computing Project - Complete Explanation

## Project Overview

This is a **Parallel Computing** project that demonstrates Monte Carlo simulation methods using two parallel computing paradigms:
- **MPI (Message Passing Interface)** - For distributed memory parallelization
- **OpenMP (Open Multi-Processing)** - For shared memory parallelization

The project covers two main application domains: **Mathematics** and **Finance**.

---

## 1. Mathematics Domain

### 1.1 Pi Estimation - Dartboard Method (`PiEstimation.cpp`)

**Algorithm**: Monte Carlo Dartboard Method

**How it works**:
- Simulates throwing darts randomly at a square with an inscribed circle
- Counts points that fall inside the unit circle (x² + y² ≤ 1)
- Estimates π using the ratio: π ≈ 4 × (points inside circle) / (total points)

**Parallelization Strategy (MPI)**:
1. **Master Process (Rank 0)**: 
   - Collects number of iterations from user
   - Starts timing
   - Broadcasts iterations to all processes
   
2. **All Processes**:
   - Each process handles `iterations/numprocs` random points
   - Uses independent PRNG seeded with process ID
   - Generates random (x, y) coordinates in range [-1, 1]
   - Counts valid points (inside circle)
   - Calculates local π estimate

3. **Reduction**:
   - All processes send their π estimates to master
   - Master averages all estimates
   - Master calculates error and execution time

**Key Features**:
- Independent random number generation per process
- Work distribution across processes
- MPI_Reduce for result aggregation

---

### 1.2 Pi Estimation - Quadrant Integration Method (`PiEstimation2.cpp`)

**Algorithm**: Monte Carlo Integration of Circle Quadrant

**How it works**:
- Integrates the function √(1 - x²) over [0, 1] (first quadrant of unit circle)
- Estimates π using: π ≈ 4 × (average of function evaluations)

**Mathematical Basis**:
- Area of quarter circle = ∫₀¹ √(1 - x²) dx = π/4
- Therefore: π = 4 × ∫₀¹ √(1 - x²) dx

**Parallelization Strategy (MPI)**:
1. **Master Process**: Collects iterations, broadcasts to all
2. **All Processes**: 
   - Each evaluates function at `iterations/numprocs` random points
   - Uses PRNG seeded with MPI_Wtime() for uniqueness
   - Sums function evaluations
3. **Reduction**: Sum all evaluations, calculate π = 4 × (total_sum / iterations)

---

### 1.3 Integration of High-Power Functions (`Integration_Any.cpp`)

**Algorithm**: Monte Carlo Integration with Reduction Formulas

**Capabilities**:
- Integrates polynomials up to degree 25
- Integrates trigonometric functions (sin, cos, tan) up to power 25
- Uses reduction formulas for exact comparison

**Function Form**:
```
f(x) = Σ(coefficients[i] × x^i) + 
       Σ(sine_coefficients[i] × sin(x)^i) + 
       Σ(cos_coefficients[i] × cos(x)^i) + 
       Σ(tan_coefficients[i] × tan(x)^i)
```

**Parallelization Strategy (MPI)**:
1. **Master Process**: 
   - Collects function coefficients and integration limits
   - Broadcasts all parameters to all processes
   
2. **Domain Decomposition**:
   - Each process gets a sub-interval: [eachllimit, eachrlimit]
   - Interval division: `eachllimit = llimit + ((rlimit-llimit)/numprocs) × myid`
   
3. **Monte Carlo Integration**:
   - Each process samples random points in its sub-interval
   - Evaluates function at each point
   - Sums evaluations
   
4. **Result Aggregation**:
   - MPI_Reduce sums all partial integrations
   - Master calculates: `integral ≈ (sum/iterations) × (rlimit - llimit)`
   - Master also calculates exact integral using reduction formulas for error analysis

**Reduction Formulas**:
- **Sine Reduction**: Recursive formula for ∫ sinⁿ(x) dx
- **Cosine Reduction**: Recursive formula for ∫ cosⁿ(x) dx  
- **Tangent Reduction**: Recursive formula for ∫ tanⁿ(x) dx

---

## 2. Finance Domain - Stock Price Simulation

### 2.1 Overview

Simulates stock price movement using **Geometric Brownian Motion (GBM)** model, which is the basis for the Black-Scholes option pricing model.

**Mathematical Model**:
```
S(t) = S₀ × exp((μ - σ²/2) × t + σ × W(t))
```
Where:
- S(t) = Stock price at time t
- S₀ = Initial stock price
- μ = Drift (expected return)
- σ = Volatility
- W(t) = Wiener process (Brownian motion)

---

### 2.2 Stage 1: Random Number Generation (`BrownianStockMovement1.c`)

**Purpose**: Generate normally distributed random numbers

**Algorithm**:
1. Generate uniform random numbers [0, 1] using `rand_r()`
2. Transform to standard normal distribution using inverse normal CDF (`normsinv()`)
3. Uses **Box-Muller transformation** equivalent via `normsinv.h`

**Parallelization (OpenMP)**:
- `#pragma omp parallel for` distributes iterations across threads
- Each thread generates independent random numbers
- Shared seed with thread-safe `rand_r()`
- Results written to `RNG.txt`

**Key Features**:
- Parallel random number generation
- Normal distribution transformation
- Thread-safe random number generation

---

### 2.3 Stage 2: Prefix Sum (`BrownianStockMovement2.c`)

**Purpose**: Compute cumulative sum of random numbers (simulates Wiener process)

**Algorithm**: Parallel Prefix Sum (Scan) using OpenMP

**Why Prefix Sum?**:
- Brownian motion W(t) = cumulative sum of independent normal random variables
- W(t) = Z₁ + Z₂ + ... + Zₜ where Zᵢ ~ N(0,1)

**Parallelization Strategy**:
- **Brent-Kung Algorithm** (simplified version):
  - Log₂(n) phases
  - Each phase doubles the stride
  - Phase j: add value from position (i - 2ʲ)
  
**OpenMP Implementation**:
- Parallel for loop in each phase
- Double buffering (alternates between two arrays)
- Reduces race conditions

**Output**: `RNG2.txt` (cumulative sums = Wiener process values)

---

### 2.4 Stage 3: Stock Price Calculation (`BrownianStockMovement3.c`)

**Purpose**: Calculate stock prices using GBM formula

**Process**:
1. Read prefix sums (Wiener process values) from `RNG2.txt`
2. Convert yearly drift and volatility to daily:
   - `drift_day = drift_year / (100 × 252)` (252 trading days/year)
   - `volatility_day = volatility_year / (100 × √252)`
3. Calculate adjusted drift: `drift_mean = drift_day - 0.5 × volatility_day²`
4. For each time step i:
   ```
   stockprice[i] = S₀ × exp(drift_mean × (i+1) + volatility_day × prefixsum[i])
   ```

**Parallelization (OpenMP)**:
- `#pragma omp parallel for` parallelizes stock price calculations
- Each thread computes prices for different time steps
- Independent calculations (no dependencies)

**Output**: `StockPrice.txt` (time series of stock prices)

---

### 2.5 Visualization (`Simulation.c`)

**Purpose**: Visualize stock price movement using OpenGL

**Features**:
- Reads stock prices from `StockPrice.txt`
- Plots line graph with points
- Displays grid and axes
- Shows price movement over time

**Implementation**:
- Uses GLUT (OpenGL Utility Toolkit)
- Calculates scaling based on min/max prices
- Renders line strip and point markers

---

## 3. Supporting Components

### 3.1 PRNG Engine (`prng_engine.hpp`)

**Purpose**: High-quality parallel random number generator

**Features**:
- SITMO PRNG (Stream cipher-based)
- Thread-safe and process-independent
- Each process/thread can have unique seed
- Generates 32-bit random numbers
- Used in MPI-based programs for independent random streams

**Usage**:
- Seeded with process ID or MPI_Wtime()
- Generates uniform random numbers [0, 2³² - 1]
- Converted to [0, 1] by dividing by `sitmo_rand_max`

---

### 3.2 Normal Inverse (`normsinv.h`)

**Purpose**: Inverse standard normal CDF (Cumulative Distribution Function)

**Algorithm**: Acklam's approximation
- Piecewise polynomial approximation
- Three regions: low tail, middle, high tail
- High accuracy for financial simulations

**Usage**: Transforms uniform [0,1] random numbers to standard normal N(0,1)

---

## 4. Project Flow

### Mathematics Programs Flow:
```
User Input (Master)
    ↓
MPI_Broadcast Parameters
    ↓
┌─────────────────┐
│ Process 0       │
│ - Generate RNG  │
│ - Compute       │
│ - Local Result  │
└─────────────────┘
         ↓
┌─────────────────┐
│ Process 1       │
│ - Generate RNG  │
│ - Compute       │
│ - Local Result  │
└─────────────────┘
         ↓
    ... (N processes)
         ↓
MPI_Reduce (Sum/Average)
    ↓
Master: Final Result + Error + Time
```

### Stock Simulation Flow:
```
Stage 1: Random Number Generation
    ↓ (OpenMP parallel)
Uniform RNG → Normal RNG
    ↓
RNG.txt
    ↓
Stage 2: Prefix Sum
    ↓ (OpenMP parallel)
Cumulative Sum (Wiener Process)
    ↓
RNG2.txt
    ↓
Stage 3: Stock Price Calculation
    ↓ (OpenMP parallel)
GBM Formula → Stock Prices
    ↓
StockPrice.txt
    ↓
Visualization (OpenGL)
    ↓
Graphical Display
```

---

## 5. Parallel Computing Paradigms Used

### MPI (Message Passing Interface)
- **Used in**: Pi estimation, Integration
- **Model**: Distributed memory
- **Communication**: MPI_Bcast, MPI_Reduce
- **Advantages**: Scalable across multiple machines
- **Use Case**: Embarrassingly parallel Monte Carlo simulations

### OpenMP (Open Multi-Processing)
- **Used in**: Stock simulation
- **Model**: Shared memory
- **Directives**: `#pragma omp parallel for`
- **Advantages**: Easy to parallelize loops, fine-grained parallelism
- **Use Case**: Data-parallel operations on arrays

---

## 6. Key Design Decisions

1. **Monte Carlo Methods**: Chosen for their natural parallelism - each sample is independent
2. **Domain Decomposition**: Integration splits domain across processes
3. **Independent PRNGs**: Each process/thread has unique seed to avoid correlation
4. **Pipeline Architecture**: Stock simulation uses 3-stage pipeline for modularity
5. **Exact Comparison**: Integration program calculates exact results for error analysis
6. **Visualization**: OpenGL provides real-time visualization of results

---

## 7. Performance Considerations

1. **Load Balancing**: Work evenly distributed across processes/threads
2. **Minimal Communication**: MPI programs minimize communication (only broadcast and reduce)
3. **Cache Efficiency**: OpenMP programs access contiguous memory
4. **Random Number Quality**: High-quality PRNG ensures statistical correctness
5. **Scalability**: Both MPI and OpenMP programs scale with number of processors

---

## 8. File Structure

```
ParallelComputing/
├── Mathematics/
│   ├── PiEstimation.cpp          # Dartboard method
│   ├── PiEstimation2.cpp         # Quadrant integration
│   └── Integration_Any.cpp       # General integration
├── Finance/
│   └── StockMovementSimulation/
│       ├── BrownianStockMovement1.c  # RNG generation
│       ├── BrownianStockMovement2.c  # Prefix sum
│       ├── BrownianStockMovement3.c  # Stock prices
│       ├── Simulation.c              # Visualization
│       └── Run.sh                    # Build & run script
├── Utilities/
│   ├── prng_engine.hpp          # Parallel RNG
│   └── normsinv.h               # Normal inverse CDF
└── README.md
```

---

## 9. Compilation and Execution

### Mathematics Programs (MPI):
```bash
mpic++ PiEstimation.cpp -o pi_estimation
mpirun -np 4 ./pi_estimation
```

### Stock Simulation (OpenMP):
```bash
cd StockMovementSimulation
bash Run.sh
# Or individually:
gcc -fopenmp BrownianStockMovement1.c -o stage1 -lm
gcc -fopenmp BrownianStockMovement2.c -o stage2 -lm
gcc -fopenmp BrownianStockMovement3.c -o stage3 -lm
gcc Simulation.c -o vis -lglut -lGL -lGLU -lm
```

---

## 10. Applications

1. **Pi Estimation**: Demonstrates Monte Carlo methods, statistical estimation
2. **Integration**: Solves complex integrals numerically, useful for functions without closed-form solutions
3. **Stock Simulation**: Financial modeling, risk analysis, option pricing, portfolio optimization

---

## Summary

This project demonstrates:
- **Parallel Computing** concepts using MPI and OpenMP
- **Monte Carlo Methods** for numerical computation
- **Financial Modeling** using stochastic processes
- **Performance Optimization** through parallelization
- **Real-world Applications** in mathematics and finance

The architecture is designed for scalability, correctness, and educational value, making it an excellent example of parallel computing principles in practice.

