#include "includes.h"

#define MONITOR_PID_FILE ".monitor_pid"
#define MONITOR_PID_FILE_PERMS 0644

static volatile sig_atomic_t got_sigusr1 = 0;
static volatile sig_atomic_t got_sigint  = 0;

static void handler_sigusr1(int signo) {
    (void)signo;
    got_sigusr1 = 1;
}
static void handler_sigint(int signo) {
    (void)signo;
    got_sigint = 1;
}
static int write_pid_file(pid_t pid) {
    int fd = open(MONITOR_PID_FILE,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  MONITOR_PID_FILE_PERMS);
    if (fd == -1) {
        perror("Monitor write error");
        return -1;
    }

    char buf[32];
    int  len = snprintf(buf, sizeof buf, "%d\n", (int)pid);
    if (write(fd, buf, len) != len) {
        perror("Other monitor write error");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}
static void delete_pid_file(void) {
    if (unlink(MONITOR_PID_FILE) == -1 && errno != ENOENT)
        perror("Monitor unlink error");
}

static int setup_signal_handlers(void) {
    struct sigaction sa_usr1, sa_int;
    sa_usr1.sa_handler = handler_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = SA_RESTART; 
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("Monitor sigaction error");
        return -1;
    }

    sa_int.sa_handler = handler_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0; 
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("Another sigaction error");
        return -1;
    }

    return 0;
}

int main() {
    pid_t my_pid = getpid();
    if (write_pid_file(my_pid) == -1)
        return 1;
        
    printf("[monitor_reports] started (PID %d). Waiting for signals...\n",(int)my_pid);
    
    fflush(stdout);

    if (setup_signal_handlers() == -1) {
        delete_pid_file();
        return 1;
    }

    while (!got_sigint) {
        pause(); 
        if (got_sigusr1) {
            got_sigusr1 = 0;
            printf("[monitor_reports] SIGUSR1 received: a new report has been added.\n");
            fflush(stdout);
        }
    }

    printf("[monitor_reports] SIGINT received: shutting down (PID %d).\n",
           (int)my_pid);
    fflush(stdout);
    delete_pid_file();

    return 0;
}