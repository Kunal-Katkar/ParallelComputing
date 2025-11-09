@echo off
echo ========================================
echo Parallel Computing Project - Build Script
echo ========================================
echo.

REM Check for g++
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: g++ not found! Please install MinGW or add it to PATH.
    pause
    exit /b 1
)

echo [1/4] Building Stock Simulation - Stage 1 (RNG Generation)...
cd StockMovementSimulation
gcc -fopenmp BrownianStockMovement1.c -o BrownianStockMovement1.exe -lm
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile Stage 1
    pause
    exit /b 1
)

echo [2/4] Building Stock Simulation - Stage 2 (Prefix Sum)...
gcc -fopenmp BrownianStockMovement2.c -o BrownianStockMovement2.exe -lm
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile Stage 2
    pause
    exit /b 1
)

echo [3/4] Building Stock Simulation - Stage 3 (Stock Prices)...
gcc -fopenmp BrownianStockMovement3.c -o BrownianStockMovement3.exe -lm
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile Stage 3
    pause
    exit /b 1
)

echo [4/4] Building Visualization (Note: Requires OpenGL/GLUT)...
gcc Simulation.c -o Simulation.exe -lglut -lGL -lGLU -lm 2>nul
if %errorlevel% neq 0 (
    echo WARNING: Visualization requires OpenGL/GLUT libraries
    echo You can skip visualization and just run the simulation stages
)

cd ..
echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo To run the stock simulation:
echo   1. cd StockMovementSimulation
echo   2. Run: BrownianStockMovement1.exe
echo   3. Run: BrownianStockMovement2.exe
echo   4. Run: BrownianStockMovement3.exe
echo   5. (Optional) Run: Simulation.exe
echo.
pause

