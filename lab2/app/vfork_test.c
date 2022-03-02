#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(){
    int status;

    pid_t child_pid = fork();

    if(child_pid == 0){
        printf("Child started\n");
        sleep(10);
    }
    else{
        wait(&status);
        printf("parent running!\n");
    }

    return 0;
}
