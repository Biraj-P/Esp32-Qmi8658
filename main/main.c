#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "QMI8658.h"  
#include "CST816S.h"    
#include "GC91A01.h"   

#define LOG_FILE "user_activity_log.csv"

// Function to read user data from file
void read_user_data(char *name) {
    FILE *file = fopen("user_data.txt", "r");
    if (file == NULL) {
        printf("User data file not found.\n");
        return;
    }
    fscanf(file, "Name: %s", name);
    fclose(file);
}

// Function to initialize log file
void initialize_log_file() {
    FILE *file = fopen(LOG_FILE, "r");
    if (file == NULL) {
        file = fopen(LOG_FILE, "w");
        if (file) {
            fprintf(file, "count,timestamp,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z,activity\n");
            fclose(file);
        }
    } else {
        fclose(file);
    }
}

// Function to get the last count from the log file
int read_last_count() {
    FILE *file = fopen(LOG_FILE, "r");
    if (!file) return 0;
    
    char line[256];
    int last_count = 0;
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d", &last_count);
    }
    fclose(file);
    return last_count;
}

int main() {
    char name[50];
    read_user_data(name);
    initialize_log_file();
    int count = read_last_count();
    
    // Initialize display
    gc9a01_init();
    gc9a01_clear();
    
    // Initialize touch sensor
    if (cst816_init()) {
        printf("CST816 detected.\n");
    } else {
        printf("CST816 not detected.\n");
    }
    
    // Initialize motion sensor
    qmi8658a_init();
    
    FILE *log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        printf("Failed to open log file.\n");
        return 1;
    }
    
    while (1) {
        int anting = 0;
        int touch_gesture = cst816_get_gesture();
        
        if (touch_gesture >= 1 && touch_gesture <= 4) {
            gc9a01_display_text("Annotating", 10, 10, GREEN);
            count++;
            
            char activity[20];
            switch (touch_gesture) {
                case 1: strcpy(activity, "Walking"); break;
                case 2: strcpy(activity, "Sitting"); break;
                case 3: strcpy(activity, "Standing"); break;
                case 4: strcpy(activity, "Running"); break;
                default: strcpy(activity, "Unknown");
            }
            
            gc9a01_display_text(activity, 20, 40, YELLOW);
        } else {
            gc9a01_display_text("Not Annotating", 10, 10, RED);
        }
        
        // Get timestamp
        long timestamp = time(NULL);
        
        // Read sensor data
        float acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z;
        QMI8658_read_acc_xyz(&acc_x, &acc_y, &acc_z);
        QMI8658_read_gyro_xyz(&gyro_x, &gyro_y, &gyro_z);
        
        // Log data
        fprintf(log_file, "%d,%ld,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%s\n",
                count, timestamp, acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z, "Activity");
        fflush(log_file);
        
        // Sleep for 10ms (simulated delay)
        usleep(10000);
    }
    
    fclose(log_file);
    return 0;
}
