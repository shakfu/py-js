// alternative run_cmd functions for ztp.c

#define PYTHON_EXE "/absolute-path-to/python3"


void run_cmd(char *cmd)
{
    pid_t pid;
    char *argv[] = {PYTHON_EXE, cmd, NULL};
    if(posix_spawn(&pid, argv[0], NULL, NULL, argv, environ) != 0) {
        error("spawn failed");
        return;
    } else {
        post("process %d started", pid);
    }
    waitpid(pid, NULL, WNOHANG);
}


int run_cmd(char *cmd) {
    pid_t pid;
    char *argv[] = {PYTHON_EXE, cmd, NULL};
    int status;
    posix_spawnattr_t attr;

    // Initialize spawn attributes
    posix_spawnattr_init(&attr);

    // Set flags if needed, for example, to specify the scheduling policy
    // posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSCHEDULER);

    // Spawn a new process
    if (posix_spawn(&pid, PYTHON_EXE, NULL, &attr, argv, environ) != 0) {
        error("spawn failed");
        exit(EXIT_FAILURE);
    } else {
        post("process %d started", pid);
    }

    // Wait for the spawned process to terminate
    // without blocking due to WNOHANG
    if (waitpid(pid, &status, WNOHANG) == -1) {
        error("waitpid failed");
        exit(EXIT_FAILURE);
    }

    if (WIFEXITED(status)) {
        post("Spawned process exited with status %d\n", WEXITSTATUS(status));
    }

    // Destroy spawn attributes
    posix_spawnattr_destroy(&attr);

    return EXIT_SUCCESS;
}

void run_cmd(char *cmd)
{
    pid_t pid;
    char *argv[] = {"python3", cmd, NULL};
    int status;

    post("Run command: %s\n", cmd);
    status = posix_spawn(&pid, PYTHON_EXE, NULL, NULL, argv, environ);
    if (status == 0) {
        post("Child pid: %i\n", pid);
        do {
          if (waitpid(pid, &status, WNOHANG) != -1) {
            post("Child status %d\n", WEXITSTATUS(status));
          } else {
            perror("waitpid");
            exit(1);
          }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
        post("posix_spawn: %s\n", strerror(status));
    }
}