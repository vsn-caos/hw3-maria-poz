#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Программе передаётся два аргумента: CMD1 и CMD2.
// Необходимо запустить два процесса, выполняющих эти команды,
// и перенаправить стандартный поток вывода CMD1 на стандартный поток ввода CMD2.
// Эквивалентно: CMD1 | CMD2
// Родительский процесс должен завершаться самым последним.

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <CMD1> <CMD2>\n", argv[0]);
        return 1;
    }

    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork 1");
        return 1;
    }

    if (pid1 == 0) {
        dup2(fd[1], STDOUT_FILENO);

        close(fd[0]);
        close(fd[1]);

        execlp(argv[1], argv[1], NULL);

        perror("execlp CMD1");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork 2");
        return 1;
    }

    if (pid2 == 0) {
        dup2(fd[0], STDIN_FILENO);

        close(fd[0]);
        close(fd[1]);

        execlp(argv[2], argv[2], NULL);
        
        perror("execlp CMD2");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}
