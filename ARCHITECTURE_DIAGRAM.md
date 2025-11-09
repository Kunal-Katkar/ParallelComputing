# Parallel Computing Project - Architecture Diagram

## System Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        PARALLEL COMPUTING PROJECT                            │
│                     (MPI + OpenMP Hybrid Architecture)                       │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    │                               │
            ┌───────▼────────┐           ┌─────────▼──────────┐
            │  MATHEMATICS   │           │     FINANCE        │
            │  (MPI-Based)   │           │  (OpenMP-Based)    │
            └───────┬────────┘           └─────────┬──────────┘
                    │                               │
        ┌───────────┼───────────┐                   │
        │           │           │                   │
┌───────▼───┐ ┌────▼────┐ ┌────▼─────┐    ┌───────▼──────────────┐
│ Pi Est.   │ │ Pi Est. │ │Integrate │    │ Stock Simulation     │
│ Dartboard │ │ Quadrant│ │ Functions│    │ (3-Stage Pipeline)   │
└───────┬───┘ └────┬────┘ └────┬─────┘    └───────┬──────────────┘
        │           │           │                   │
        └───────────┴───────────┘                   │
                    │                               │
            ┌───────▼───────────────────────────────▼───────┐
            │        Supporting Components                  │
            │  • PRNG Engine (prng_engine.hpp)             │
            │  • Normal Inverse (normsinv.h)               │
            └──────────────────────────────────────────────┘
```

---

## 1. Mathematics Domain Architecture (MPI)

### 1.1 MPI Process Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                      MPI COMMUNICATOR                         │
│                    (MPI_COMM_WORLD)                           │
└──────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
┌───────▼──────┐    ┌───────▼──────┐    ┌──────▼──────┐
│   Process 0  │    │   Process 1  │    │ Process N   │
│   (MASTER)   │    │   (WORKER)   │    │  (WORKER)   │
├──────────────┤    ├──────────────┤    ├─────────────┤
│ • User Input │    │ • Receive    │    │ • Receive   │
│ • Broadcast  │    │   Parameters │    │   Parameters│
│ • Collect    │    │ • Generate   │    │ • Generate  │
│   Results    │    │   RNG        │    │   RNG       │
│ • Reduce     │    │ • Compute    │    │ • Compute   │
│ • Output     │    │   Locally    │    │   Locally   │
│ • Timing     │    │ • Send Result│    │ • Send Result│
└──────┬───────┘    └──────┬───────┘    └─────┬───────┘
       │                   │                   │
       └───────────────────┼───────────────────┘
                           │
                  ┌────────▼────────┐
                  │   MPI_Reduce    │
                  │   (Sum/Average) │
                  └────────┬────────┘
                           │
                  ┌────────▼────────┐
                  │  Final Result   │
                  │  (Master Only)  │
                  └─────────────────┘
```

### 1.2 Pi Estimation - Dartboard Method Flow

```
┌─────────────────────────────────────────────────────────────┐
│                    MASTER PROCESS (Rank 0)                   │
│  • Get iterations from user                                  │
│  • Start timer (MPI_Wtime())                                 │
│  • MPI_Bcast(iterations)                                     │
└───────────────────────┬─────────────────────────────────────┘
                        │
        ┌───────────────┼───────────────┐
        │               │               │
┌───────▼──────┐ ┌──────▼──────┐ ┌─────▼──────┐
│  Process 0   │ │  Process 1   │ │ Process N  │
├──────────────┤ ├──────────────┤ ├────────────┤
│ Seed: 0      │ │ Seed: 1      │ │ Seed: N    │
│ Iterations:  │ │ Iterations:  │ │ Iterations:│
│   N/p        │ │   N/p        │ │   N/p      │
│              │ │              │ │            │
│ for i=1 to   │ │ for i=1 to   │ │ for i=1 to │
│   N/p:       │ │   N/p:       │ │   N/p:     │
│   x = random │ │   x = random │ │   x = random│
│   y = random │ │   y = random │ │   y = random│
│   if x²+y²≤1 │ │   if x²+y²≤1 │ │   if x²+y²≤1│
│     count++  │ │     count++  │ │     count++│
│              │ │              │ │            │
│ π_local =    │ │ π_local =    │ │ π_local =  │
│   4×count/N/p│ │   4×count/N/p│ │   4×count/ │
│              │ │              │ │     N/p    │
└──────┬───────┘ └──────┬───────┘ └─────┬──────┘
       │                │                │
       └────────────────┼────────────────┘
                        │
            ┌───────────▼───────────┐
            │   MPI_Reduce          │
            │   Operation: SUM      │
            │   Root: 0 (Master)    │
            └───────────┬───────────┘
                        │
            ┌───────────▼───────────┐
            │   MASTER PROCESS      │
            │   π = sum / numprocs  │
            │   error = π - π_exact │
            │   time = MPI_Wtime()  │
            │   Print results       │
            └───────────────────────┘
```

### 1.3 Integration Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                    INPUT PARAMETERS                           │
│  • Polynomial coefficients (degree 0-25)                      │
│  • Sine coefficients (power 1-25)                             │
│  • Cosine coefficients (power 1-25)                           │
│  • Tangent coefficients (power 1-25)                          │
│  • Lower limit (a)                                            │
│  • Upper limit (b)                                            │
│  • Number of iterations (N)                                   │
└───────────────────────┬──────────────────────────────────────┘
                        │
            ┌───────────▼───────────┐
            │   MPI_Bcast (All)     │
            └───────────┬───────────┘
                        │
        ┌───────────────┼───────────────┐
        │               │               │
┌───────▼──────┐ ┌──────▼──────┐ ┌─────▼──────┐
│  Process 0   │ │  Process 1   │ │ Process N  │
├──────────────┤ ├──────────────┤ ├────────────┤
│ Domain:      │ │ Domain:      │ │ Domain:    │
│ [a, a+Δ]     │ │ [a+Δ, a+2Δ]  │ │ [a+NΔ, b]  │
│ where        │ │              │ │            │
│ Δ=(b-a)/p    │ │              │ │            │
│              │ │              │ │            │
│ for i=1 to   │ │ for i=1 to   │ │ for i=1 to │
│   N/p:       │ │   N/p:       │ │   N/p:     │
│   x = random │ │   x = random │ │   x = random│
│     in domain│ │     in domain│ │     in domain│
│   sum += f(x)│ │   sum += f(x)│ │   sum += f(x)│
│              │ │              │ │            │
│ integral_local│ │ integral_local│ │ integral_│
│   = sum × Δ  │ │   = sum × Δ  │ │   local   │
└──────┬───────┘ └──────┬───────┘ └─────┬──────┘
       │                │                │
       └────────────────┼────────────────┘
                        │
            ┌───────────▼───────────┐
            │   MPI_Reduce (SUM)    │
            └───────────┬───────────┘
                        │
            ┌───────────▼───────────┐
            │   MASTER PROCESS      │
            │   integral ≈          │
            │     (total_sum/N)×(b-a)│
            │                       │
            │   Calculate exact:    │
            │   • Polynomial:       │
            │     ∫xⁿ = xⁿ⁺¹/(n+1)  │
            │   • Trig: Reduction   │
            │     formulas          │
            │                       │
            │   error = approx-exact│
            │   Print results       │
            └───────────────────────┘
```

---

## 2. Finance Domain Architecture (OpenMP)

### 2.1 Stock Simulation Pipeline Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│              STOCK PRICE SIMULATION PIPELINE                     │
└─────────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
┌───────▼────────┐  ┌───────▼────────┐  ┌──────▼─────────┐
│   STAGE 1      │  │   STAGE 2      │  │   STAGE 3      │
│  RNG Generation│  │  Prefix Sum    │  │ Stock Prices   │
│   (OpenMP)     │  │   (OpenMP)     │  │   (OpenMP)     │
└───────┬────────┘  └───────┬────────┘  └──────┬─────────┘
        │                   │                   │
        │                   │                   │
        ▼                   ▼                   ▼
```

### 2.2 Stage 1: Random Number Generation

```
┌─────────────────────────────────────────────────────────────┐
│              STAGE 1: RNG GENERATION                         │
│  Input: Length (len)                                         │
│  Output: RNG.txt (normal random numbers)                     │
└─────────────────────────────────────────────────────────────┘
                            │
            ┌───────────────▼───────────────┐
            │   OpenMP Parallel Region      │
            │   #pragma omp parallel for    │
            └───────────────┬───────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
┌───────▼──────┐    ┌───────▼──────┐    ┌──────▼──────┐
│   Thread 0   │    │   Thread 1   │    │  Thread N   │
├──────────────┤    ├──────────────┤    ├─────────────┤
│ Iterations:  │    │ Iterations:  │    │ Iterations: │
│   0 to len/p │    │  len/p to    │    │  (N-1)×len/p│
│              │    │   2×len/p    │    │  to len     │
│              │    │              │    │             │
│ for each i:  │    │ for each i:  │    │ for each i: │
│   u = rand_r │    │   u = rand_r │    │   u = rand_r│
│   z =        │    │   z =        │    │   z =       │
│     normsinv │    │     normsinv │    │     normsinv│
│     (u)      │    │     (u)      │    │     (u)     │
│   RNG[i] = z │    │   RNG[i] = z │    │   RNG[i] = z│
└──────┬───────┘    └──────┬───────┘    └─────┬───────┘
       │                   │                   │
       └───────────────────┼───────────────────┘
                           │
            ┌──────────────▼───────────────┐
            │   Write to RNG.txt           │
            │   (Sequential I/O)           │
            └──────────────────────────────┘
```

### 2.3 Stage 2: Prefix Sum (Parallel Scan)

```
┌─────────────────────────────────────────────────────────────┐
│              STAGE 2: PREFIX SUM (PARALLEL SCAN)             │
│  Input: RNG.txt (normal random numbers)                      │
│  Output: RNG2.txt (cumulative sums = Wiener process)         │
└─────────────────────────────────────────────────────────────┘
                            │
            ┌───────────────▼───────────────┐
            │   Read RNG.txt                │
            │   Load into array[0][]        │
            └───────────────┬───────────────┘
                            │
            ┌───────────────▼───────────────┐
            │   Brent-Kung Algorithm        │
            │   Log₂(n) phases              │
            └───────────────┬───────────────┘
                            │
    ┌───────────────────────┼───────────────────────┐
    │                       │                       │
┌───▼────┐            ┌─────▼─────┐          ┌─────▼─────┐
│Phase 0 │            │ Phase 1   │          │Phase log n│
│stride=1│            │stride=2   │          │stride=2^j │
├────────┤            ├───────────┤          ├───────────┤
│OpenMP  │            │ OpenMP    │          │ OpenMP    │
│parallel│            │ parallel  │          │ parallel  │
│for     │            │ for       │          │ for       │
│        │            │           │          │           │
│for i:  │            │ for i:    │          │ for i:    │
│  if i≥1│            │  if i≥2   │          │  if i≥2^j │
│    out │            │    out[i] │          │    out[i] │
│    [i] │            │      =    │          │      =    │
│    =   │            │    in[i]  │          │    in[i]  │
│    in  │            │    +      │          │    +      │
│    [i] │            │    in[i-2]│          │    in     │
│    +   │            │           │          │    [i-2^j]│
│    in  │            │           │          │           │
│    [i-1│            │           │          │           │
│    ]   │            │           │          │           │
└───┬────┘            └─────┬─────┘          └─────┬─────┘
    │                       │                       │
    │    Double Buffering   │                       │
    │    (Alternate arrays) │                       │
    └───────────────────────┼───────────────────────┘
                            │
            ┌───────────────▼───────────────┐
            │   Write to RNG2.txt           │
            │   (Prefix sums = W(t))        │
            └───────────────────────────────┘
```

### 2.4 Stage 3: Stock Price Calculation

```
┌─────────────────────────────────────────────────────────────┐
│              STAGE 3: STOCK PRICE CALCULATION                │
│  Input: RNG2.txt (Wiener process values)                     │
│  Input: drift_year, volatility_year, initial_price           │
│  Output: StockPrice.txt (stock prices over time)             │
└─────────────────────────────────────────────────────────────┘
                            │
            ┌───────────────▼───────────────┐
            │   Read Parameters             │
            │   • drift_year (%)            │
            │   • volatility_year (%)       │
            │   • stockprice_initial ($)    │
            └───────────────┬───────────────┘
                            │
            ┌───────────────▼───────────────┐
            │   Convert to Daily            │
            │   drift_day =                 │
            │     drift_year/(100×252)      │
            │   volatility_day =            │
            │     volatility_year/(100×√252)│
            │   drift_mean =                │
            │     drift_day - 0.5×σ²        │
            └───────────────┬───────────────┘
                            │
            ┌───────────────▼───────────────┐
            │   Read RNG2.txt               │
            │   Load prefixsum[]            │
            └───────────────┬───────────────┘
                            │
            ┌───────────────▼───────────────┐
            │   OpenMP Parallel Region      │
            │   #pragma omp parallel for    │
            └───────────────┬───────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
┌───────▼──────┐    ┌───────▼──────┐    ┌──────▼──────┐
│   Thread 0   │    │   Thread 1   │    │  Thread N   │
├──────────────┤    ├──────────────┤    ├─────────────┤
│ for i in     │    │ for i in     │    │ for i in    │
│   [0, len/p) │    │   [len/p,    │    │   [(N-1)×   │
│              │    │    2×len/p)  │    │   len/p,    │
│              │    │              │    │   len)      │
│ stockprice   │    │ stockprice   │    │ stockprice  │
│ [i] = S₀ ×   │    │ [i] = S₀ ×   │    │ [i] = S₀ ×  │
│   exp(       │    │   exp(       │    │   exp(      │
│     drift_   │    │     drift_   │    │     drift_  │
│     mean ×   │    │     mean ×   │    │     mean ×  │
│     (i+1) +  │    │     (i+1) +  │    │     (i+1) + │
│     σ ×      │    │     σ ×      │    │     σ ×     │
│     prefix   │    │     prefix   │    │     prefix  │
│     sum[i]   │    │     sum[i]   │    │     sum[i]  │
│   )          │    │   )          │    │   )         │
└──────┬───────┘    └──────┬───────┘    └─────┬───────┘
       │                   │                   │
       └───────────────────┼───────────────────┘
                           │
            ┌──────────────▼───────────────┐
            │   Write to StockPrice.txt    │
            └──────────────┬───────────────┘
                           │
            ┌──────────────▼───────────────┐
            │   Visualization (OpenGL)     │
            │   • Read StockPrice.txt      │
            │   • Plot line graph          │
            │   • Display axes & grid      │
            └──────────────────────────────┘
```

---

## 3. Data Flow Diagrams

### 3.1 Mathematics Programs Data Flow

```
USER INPUT
    │
    ▼
┌──────────────┐
│  Master      │
│  Process     │
│  (Rank 0)    │
└──────┬───────┘
       │
       │ MPI_Bcast (parameters)
       │
       ▼
┌──────────────────────────────────┐
│  All Processes Receive           │
│  • iterations                    │
│  • function parameters           │
│  • limits                        │
└──────┬───────────────────────────┘
       │
       │ Parallel Execution
       │
       ▼
┌──────────────────────────────────┐
│  Each Process                    │
│  • Generate independent RNG      │
│  • Compute partial results       │
│  • Store local sum/count         │
└──────┬───────────────────────────┘
       │
       │ MPI_Reduce
       │
       ▼
┌──────────────┐
│  Master      │
│  Aggregates  │
│  Results     │
└──────┬───────┘
       │
       ▼
   OUTPUT
   (Result + Error + Time)
```

### 3.2 Stock Simulation Data Flow

```
USER INPUT
(drift, volatility, initial price, length)
    │
    ▼
┌─────────────────────┐
│  Stage 1: RNG       │
│  • Generate uniform │
│  • Transform normal │
│  Output: RNG.txt    │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Stage 2: Prefix Sum│
│  • Read RNG.txt     │
│  • Compute scan     │
│  Output: RNG2.txt   │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Stage 3: Prices    │
│  • Read RNG2.txt    │
│  • Apply GBM        │
│  Output:            │
│  StockPrice.txt     │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Visualization      │
│  • Read prices      │
│  • Plot graph       │
└─────────────────────┘
```

---

## 4. Component Interaction Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                          │
│  • PiEstimation.cpp                                           │
│  • PiEstimation2.cpp                                          │
│  • Integration_Any.cpp                                        │
│  • BrownianStockMovement*.c                                   │
└───────────────────────┬──────────────────────────────────────┘
                        │
        ┌───────────────┼───────────────┐
        │               │               │
┌───────▼──────┐ ┌──────▼──────┐ ┌─────▼──────┐
│   MPI API    │ │  OpenMP API │ │  OpenGL API│
│  • Bcast     │ │  • parallel │ │  • glut    │
│  • Reduce    │ │  • for      │ │  • gl      │
│  • Init      │ │  • shared   │ │  • glu     │
└──────┬───────┘ └──────┬──────┘ └─────┬──────┘
       │                │               │
┌──────▼───────────────▼───────────────▼──────┐
│            UTILITY LIBRARIES                 │
│  • prng_engine.hpp (SITMO PRNG)              │
│  • normsinv.h (Inverse Normal CDF)           │
│  • math.h (Standard Math Functions)          │
└──────────────────────────────────────────────┘
                        │
┌───────────────────────▼───────────────────────┐
│            SYSTEM LAYER                       │
│  • MPI Runtime (mpirun)                       │
│  • OpenMP Runtime (libgomp)                   │
│  • Operating System                           │
│  • Hardware (CPU Cores, Memory)               │
└───────────────────────────────────────────────┘
```

---

## 5. Parallelization Strategy Comparison

```
┌──────────────────────────────────────────────────────────────┐
│              MPI vs OpenMP Usage                              │
└──────────────────────────────────────────────────────────────┘

MATHEMATICS (MPI - Distributed Memory)
┌──────────────────────────────────────────────────────────────┐
│  Advantages:                                                  │
│  • Scalable across multiple machines                         │
│  • Large problem sizes                                       │
│  • Independent processes                                     │
│  • Minimal shared state                                      │
│                                                              │
│  Communication Pattern:                                       │
│  • Broadcast: O(log p)                                       │
│  • Reduce: O(log p)                                          │
│  • Total: Minimal communication overhead                     │
└──────────────────────────────────────────────────────────────┘

FINANCE (OpenMP - Shared Memory)
┌──────────────────────────────────────────────────────────────┐
│  Advantages:                                                  │
│  • Easy to parallelize loops                                 │
│  • Fine-grained parallelism                                  │
│  • Shared memory access                                      │
│  • Lower overhead for small problems                         │
│                                                              │
│  Parallelization Pattern:                                     │
│  • Data parallelism                                          │
│  • Loop-level parallelism                                    │
│  • Shared arrays                                             │
└──────────────────────────────────────────────────────────────┘
```

---

## 6. Memory Model

### MPI Programs (Distributed Memory)
```
Process 0          Process 1          Process N
┌─────────┐       ┌─────────┐       ┌─────────┐
│ Memory  │       │ Memory  │       │ Memory  │
│         │       │         │       │         │
│ Local   │       │ Local   │       │ Local   │
│ Variables│      │ Variables│      │ Variables│
│         │       │         │       │         │
│ RNG     │       │ RNG     │       │ RNG     │
│ State   │       │ State   │       │ State   │
└────┬────┘       └────┬────┘       └────┬────┘
     │                 │                 │
     └─────────────────┼─────────────────┘
                       │
              ┌────────▼────────┐
              │   Network       │
              │   Communication │
              │   (MPI)         │
              └─────────────────┘
```

### OpenMP Programs (Shared Memory)
```
┌─────────────────────────────────────┐
│         Shared Memory                │
│  ┌─────────┐  ┌─────────┐  ┌───────┐│
│  │ Array[] │  │ Array[] │  │ Array ││
│  │ (Shared)│  │ (Shared)│  │ (Sh.) ││
│  └────┬────┘  └────┬────┘  └───┬───┘│
└───────┼────────────┼────────────┼────┘
        │            │            │
  ┌─────▼────┐  ┌───▼────┐  ┌───▼────┐
  │ Thread 0 │  │Thread 1│  │Thread N│
  │          │  │        │  │        │
  │ Private  │  │Private │  │Private │
  │ Variables│  │Variables│ │Variables│
  └──────────┘  └────────┘  └────────┘
```

---

## Summary

This architecture demonstrates:

1. **Hybrid Parallelism**: Combining MPI and OpenMP for different problem types
2. **Scalability**: MPI for distributed systems, OpenMP for shared memory
3. **Modularity**: Separate stages in pipeline architecture
4. **Efficiency**: Minimal communication in MPI, fine-grained parallelism in OpenMP
5. **Correctness**: Independent RNG streams, proper synchronization
6. **Usability**: Clear separation of concerns, reusable components

The architecture is designed to be educational, demonstrating best practices in parallel computing while solving real-world problems in mathematics and finance.
