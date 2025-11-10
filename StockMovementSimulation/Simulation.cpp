#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_POINTS 100
#define MARGIN 150

double arr[MAX_POINTS];
int size = 0;
double minv = 1e9, maxv = -1e9;
int currentPoint = 0;
int windowWidth = 1200, windowHeight = 700;
int isPaused = 0;
int showGrid = 1;
int animationSpeed = 200; // milliseconds

// Color structure for better color management
struct Color {
    float r, g, b;
};

// Predefined colors
Color RED = {1.0f, 0.2f, 0.2f};
Color GREEN = {0.2f, 0.7f, 0.2f};
Color BLUE = {0.2f, 0.4f, 0.9f};
Color DARK_BLUE = {0.1f, 0.2f, 0.4f};
Color LIGHT_GRAY = {0.9f, 0.9f, 0.9f};
Color DARK_GRAY = {0.5f, 0.5f, 0.5f};
Color WHITE = {1.0f, 1.0f, 1.0f};
Color BLACK = {0.0f, 0.0f, 0.0f};
Color GOLD = {1.0f, 0.84f, 0.0f};

void setColor(Color c) {
    glColor3f(c.r, c.g, c.b);
}

void drawText(float x, float y, char *text, void* font = GLUT_BITMAP_HELVETICA_12) {
    glRasterPos2f(x, y);
    for (char *c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void drawFilledRect(float x, float y, float width, float height, Color color) {
    setColor(color);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawBorderRect(float x, float y, float width, float height, Color color, float lineWidth = 1.5f) {
    setColor(color);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawGrid(int width, int height, int numCellsX, int numCellsY) {
    if (!showGrid) return;
    
    setColor(LIGHT_GRAY);
    glLineWidth(1.0);
    glBegin(GL_LINES);
    
    // Vertical grid lines
    for (int i = 1; i < numCellsX; i++) {
        float x = (float)i / numCellsX * width;
        glVertex2f(x, 0);
        glVertex2f(x, height);
    }
    
    // Horizontal grid lines
    for (int i = 1; i < numCellsY; i++) {
        float y = (float)i / numCellsY * height;
        glVertex2f(0, y);
        glVertex2f(width, y);
    }
    
    glEnd();
}

void drawAxes(int width, int height) {
    // Draw grid first
    drawGrid(width, height, 10, 8);
    
    // Draw axes
    setColor(DARK_GRAY);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    // Y-axis
    glVertex2f(MARGIN, MARGIN);
    glVertex2f(MARGIN, height - MARGIN);
    // X-axis
    glVertex2f(MARGIN, MARGIN);
    glVertex2f(width - MARGIN, MARGIN);
    glEnd();
    
    // X-axis labels and ticks
    setColor(BLACK);
    for (int i = 0; i <= 10; i++) {
        int xPos = MARGIN + i * (width - 2 * MARGIN) / 10;
        int timeStep = i * size / 10;
        
        // Tick mark
        glLineWidth(1.5);
        glBegin(GL_LINES);
        glVertex2f(xPos, MARGIN - 5);
        glVertex2f(xPos, MARGIN + 5);
        glEnd();
        
        // Label
        char label[20];
        sprintf(label, "%d", timeStep);
        drawText(xPos - 5, MARGIN - 20, label);
    }
    
    // Y-axis labels and ticks
    for (int i = 0; i <= 8; i++) {
        int yPos = MARGIN + i * (height - 2 * MARGIN) / 8;
        double value = minv + (maxv - minv) * i / 8.0;
        
        // Tick mark
        glLineWidth(1.5);
        glBegin(GL_LINES);
        glVertex2f(MARGIN - 5, yPos);
        glVertex2f(MARGIN + 5, yPos);
        glEnd();
        
        // Label
        char label[20];
        sprintf(label, "%.0f", value);
        drawText(MARGIN - 40, yPos - 5, label);
    }
    
    // Axis titles
    drawText(width / 2 - 30, MARGIN - 45, "Time Steps", GLUT_BITMAP_HELVETICA_18);
    drawText(15, height / 2, "Stock Price", GLUT_BITMAP_HELVETICA_18);
    
    // Chart title
    setColor(DARK_BLUE);
    drawText(width / 2 - 80, height - 20, "Stock Price Simulation", GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawInfoPanel(int width, int height) {
    int panelWidth = 250;
    int panelHeight = 120;
    int panelX = width - panelWidth - 20;
    int panelY = height - panelHeight - 20;
    
    // Panel background
    drawFilledRect(panelX, panelY, panelWidth, panelHeight, WHITE);
    drawBorderRect(panelX, panelY, panelWidth, panelHeight, DARK_BLUE, 2.0);
    
    // Panel title
    setColor(DARK_BLUE);
    drawText(panelX + 10, panelY + panelHeight - 20, "Stock Information", GLUT_BITMAP_HELVETICA_18);
    
    // Information
    char info[5][50];
    sprintf(info[0], "Current Price: $%.2f", currentPoint > 0 ? arr[currentPoint-1] : 0);
    sprintf(info[1], "Min: $%.2f", minv);
    sprintf(info[2], "Max: $%.2f", maxv);
    sprintf(info[3], "Data Points: %d/%d", currentPoint, size);
    sprintf(info[4], "Status: %s", isPaused ? "PAUSED" : "RUNNING");
    
    setColor(BLACK);
    for (int i = 0; i < 5; i++) {
        drawText(panelX + 15, panelY + panelHeight - 40 - i*20, info[i]);
    }
}

void drawControls(int width, int height) {
    int controlWidth = 200;
    int controlHeight = 80;
    int controlX = 20;
    int controlY = height - controlHeight - 20;
    
    // Control panel background
    drawFilledRect(controlX, controlY, controlWidth, controlHeight, WHITE);
    drawBorderRect(controlX, controlY, controlWidth, controlHeight, DARK_BLUE, 2.0);
    
    // Control panel title
    setColor(DARK_BLUE);
    drawText(controlX + 10, controlY + controlHeight - 20, "Controls", GLUT_BITMAP_HELVETICA_18);
    
    // Control instructions
    setColor(BLACK);
    drawText(controlX + 15, controlY + controlHeight - 40, "SPACE: Pause/Resume");
    drawText(controlX + 15, controlY + controlHeight - 60, "G: Toggle Grid");
}

void drawStockChart(int width, int height) {
    if (currentPoint < 2) return;
    
    // Calculate scaling factors
    float xScale = (float)(width - 2 * MARGIN) / (size - 1);
    float yScale = (float)(height - 2 * MARGIN) / (maxv - minv);
    
    // Draw the stock price line
    setColor(BLUE);
    glLineWidth(3.0);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < currentPoint; i++) {
        float x = MARGIN + i * xScale;
        float y = MARGIN + (arr[i] - minv) * yScale;
        glVertex2f(x, y);
    }
    glEnd();
    
    // Draw data points
    glPointSize(6.0);
    setColor(RED);
    glBegin(GL_POINTS);
    for (int i = 0; i < currentPoint; i++) {
        float x = MARGIN + i * xScale;
        float y = MARGIN + (arr[i] - minv) * yScale;
        glVertex2f(x, y);
    }
    glEnd();
    
    // Draw current point highlight
    if (currentPoint > 0) {
        float x = MARGIN + (currentPoint - 1) * xScale;
        float y = MARGIN + (arr[currentPoint - 1] - minv) * yScale;
        
        // Highlight circle
        setColor(GOLD);
        glPointSize(12.0);
        glBegin(GL_POINTS);
        glVertex2f(x, y);
        glEnd();
        
        // Current value label
        char currentValue[20];
        sprintf(currentValue, "$%.2f", arr[currentPoint - 1]);
        setColor(DARK_BLUE);
        drawText(x + 10, y + 10, currentValue, GLUT_BITMAP_HELVETICA_18);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Set up coordinate system for easier drawing
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    
    drawAxes(windowWidth, windowHeight);
    drawStockChart(windowWidth, windowHeight);
    drawInfoPanel(windowWidth, windowHeight);
    drawControls(windowWidth, windowHeight);
    
    glutSwapBuffers();
}

void timer(int value) {
    if (!isPaused && currentPoint < size) {
        currentPoint++;
        glutPostRedisplay();
        glutTimerFunc(animationSpeed, timer, 0);
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case ' ':
            isPaused = !isPaused;
            if (!isPaused && currentPoint < size) {
                glutTimerFunc(animationSpeed, timer, 0);
            }
            break;
        case 'g':
        case 'G':
            showGrid = !showGrid;
            break;
        case 'r':
        case 'R':
            currentPoint = 0;
            isPaused = 0;
            glutTimerFunc(animationSpeed, timer, 0);
            break;
        case '+':
            animationSpeed = fmax(50, animationSpeed - 50);
            break;
        case '-':
            animationSpeed = fmin(1000, animationSpeed + 50);
            break;
        case 27: // ESC key
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    glutPostRedisplay();
}

void init() {
    glClearColor(0.95f, 0.95f, 0.98f, 1.0f); // Light blue-gray background
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

int main(int argc, char **argv) {
    FILE *fp = fopen("StockPrice.txt", "r");
    if (!fp) {
        printf("Error: could not open StockPrice.txt\n");
        printf("Creating sample data...\n");
        
        // Create sample data if file doesn't exist
        size = 50;
        arr[0] = 100.0;
        for (int i = 1; i < size; i++) {
            arr[i] = arr[i-1] + (rand() % 20 - 10);
            if (arr[i] < minv) minv = arr[i];
            if (arr[i] > maxv) maxv = arr[i];
        }
    } else {
        while (fscanf(fp, "%lf", &arr[size]) != EOF) {
            if (arr[size] < minv) minv = arr[size];
            if (arr[size] > maxv) maxv = arr[size];
            size++;
        }
        fclose(fp);
    }

    printf("Loaded %d values (min=%.2f, max=%.2f)\n", size, minv, maxv);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Enhanced Stock Price Simulation");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(animationSpeed, timer, 0);
    glutMainLoop();
    return 0;
}