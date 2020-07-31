/*-------------------------------------------------------
File: user.c

Name: Dmitry Kutin
Student Number: 300015920

Description: This program is designed to test the severs
             program using pipes.
             Please follow the instructions in the comments
--------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <signal.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// Utilities

// Time Information Structure
typedef struct TIME_INFO
{
    int hour;
    int minute;
    int second;
}TimeInfo;

// This function would close the pipe
void closePipe(int *pipe)
{
    close(pipe[0]);
    close(pipe[1]);
}

// This function will get current time and store it in TimeInfo struct
TimeInfo getTimeInfo()
{
    time_t now;
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now);
    
    TimeInfo timeInfo = {tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec};
    return timeInfo;
}

// This function is used for print the time information storing in the TimeInfo
void printTimeInfo(TimeInfo timeInfo)
{
    printf("%d@%d:%d:%d\n", getpid(), timeInfo.hour, timeInfo.minute, timeInfo.second);
}

// This function is to print the current time
void printTime()
{
    TimeInfo timeInfo = getTimeInfo();
    printTimeInfo(timeInfo);
}

// read function in block mode, same usage as read(int fd, const void *buf, size_t count)
int blockRead(int fd, const void *buf, size_t count)
{
    int readNum = 0;
    for(;readNum < count;)
    {
        char data = 0;
        int retVal = read(fd, &data, sizeof(data));
        if(retVal > 0)
        {
            readNum += sizeof(data);
            *(char *)buf = data;
            (char *)buf++;
        }
    }
    return readNum;
}

// Used for thread to print time atleast twice. 
void* backgroundPrint() {
    for (int i = 0; i < 3; i++) {
        printTime();
    }
    return NULL;
}


// define your parameters here
int queryTimesMax = 0;
int queryInterval = 0;
int serverPrintTimesMax = 0;
int serverPrintInterval = 0;

#define QueryCommand 't' // Used for Querying Time
#define QuitCommand 'q'

/*---------------------------------------------------------------
Function: main

Description: Complete the functions listed in the comments.
             Each function is 10 points.

Assignment: Complete this function to ensure proper calls to 
            the other functions.
---------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    pid_t       p, rp; 
    pthread_t   ptid;
    int         UR[2], RU[2], SR[2], RS[2], stat;
    char        buf[1024];
    // You may liek to use the utility functions provided above //
    // Create pipes
	if(pipe(SR) == -1) {
        fprintf(stderr, "Pipe UR Failed" ); 
        return 1; 
    }

    if(pipe(RS) == -1) {
        fprintf(stderr, "Pipe RS Failed" ); 
        return 1; 
    }

    if(pipe(UR) == -1) {
        fprintf(stderr, "Pipe UR Failed" ); 
        return 1; 
    }

    if(pipe(RU) == -1) {
        fprintf(stderr, "Pipe RU Failed" ); 
        return 1; 
    }
    
    // Create router process
    rp = fork();

    if (rp == 0) {
        dup2(SR[0], STDIN_FILENO); // Server to Router, Read
        dup2(RS[1], STDOUT_FILENO); // Router to Server, Write
        dup2(UR[0], STDIN_FILENO); // User to Router, Read
        dup2(RU[1], STDOUT_FILENO); // Router to User, Write
        closePipe(UR);
        closePipe(RU);
        closePipe(RS);
        closePipe(SR); // Close all pipes
        
        TimeInfo time = getTimeInfo(); // Get current time info
        sprintf(buf, "%c", QueryCommand); // Write QueryCommand to STDOUT
        write(STDOUT_FILENO, buf, sizeof(buf));
        execlp("./router", "./router", time.second, time.minute, time.hour, getpid(), (char*) NULL); // Execlp router, pass pipes
        printf("execlp() Failed to start router");
    }
    
    // Create Server process
    p = fork();

    // Server process
    if (p == 0) {
        close(RS[1]);
        close(SR[0]);
        // 1. Server process should create a thread to display current time for at least 2 times
        if (pthread_create(&ptid, NULL, backgroundPrint, NULL)) { // Create thread to run backgroundPrint (Runs print 3 times)
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }

        dup2(RS[0], STDIN_FILENO); // Route Router to Server read to STDIN
        close(RS[0]); // Close pipe after routing to STDIN
        dup2(SR[1], STDOUT_FILENO); // Route Server to Router write to STDOUT
        close(SR[1]); // Close pipe after routing to STDOUT

        while((read(STDIN_FILENO, buf, sizeof(buf) + 1)) != 0) { // Wait for something to be written to buffer, and read it 
            // 2. Server should respond the following command from Router
            //     -"t": send current time to the router
            //     -"q": stop waiting for the command, kill thread and exit.
            if (strcmp(buf, "t")) { // if 't' has been passed
                printTime();
            } else if (strcmp(buf, "q")) { // if 'q' has been passed
                // 3. Server should end the process by itself instead of User process
                printf("Server Quits");
                break;
            } else {
                // Error
                printf("Error has occured");
            } 
        }
        pthread_join(ptid, NULL); // Join pthread
    }
    // Create User process
    else if (p > 0) {
        close(RU[1]); // Close unnecessary pipes
        close(UR[0]);

        dup2(UR[1], STDOUT_FILENO); // Route User to Router write to STDOUT
        close(UR[1]); // Close pipe

        dup2(RU[0], STDin_FILENO); // Route Router to User read to STDIN
        close(RU[0]); // Close pipe

        // 4. User queries time for at least 2 times by sending 't' command to Router
        for (int i; i < 3; i++) {
            write(STDOUT_FILENO, "t", 1); // Write t to STDOUT
            blockRead(STDIN_FILENO, buf, sizeof(buf) + 1); // Read any response 
            printf("%s", buf); // Print the response
        }
        // 5. Then User sends 'q' command to Router
        write(STDOUT_FILENO, "q", 1); // Write to STDOUT, quit
        // 6. User waits for the Server ending itself
        wait(NULL); // Waits for the server process to end 
        // 7. User kills the Router process
        kill(rp, SIGKILL); // Kill the router process
    }
    
    
    exit(0); 
    return 0;
}
