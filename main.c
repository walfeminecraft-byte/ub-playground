#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

int main() {
    // Simple call
    printf("Just a message: \n");
	printf("Hello, %s\n", "Wal FE");

    sleep(3);

    // The same, but looks strange
    printf("A bit different: \n");
    const char *msg = "Wal FE";
    __asm__(
        "mov %0, %%rsi\n"
        :
        : "r" (msg)
        : "rax"
    );
    printf("Hello, %s\n");

    sleep(3);

    // Now let's use only Assembly
    
    printf("Hardcore: \n");
    const char *m1 = "Hello, %s\n";
    const char *m2 = "Wal FE";
    __asm__ (
        "mov %0, %%rdi\n"
        "mov %1, %%rsi\n"
        "call printf\n"
        :
        : "r" (m1), "r" (m2)
        : "rdi", "rsi"
    );

    sleep(3);

    // Now it will be UB
    
    printf("Ok, let's try to print something from 0xDEADBEEF: \n");

    pid_t pid = fork();

    if (pid < 0) {

        perror("fork failed");
        exit(1);
    }

    if (!pid) {
        printf("The message: %s", (char*)0xDEADBEEF);

        exit(0);
    } else {
        int status;
        pid_t wpid = waitpid(pid, &status, 0);

        if (wpid == -1) {
            perror("waitpid failed");
            exit(1);
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            if (sig == SIGSEGV) {
                printf("Ok, it was Segfault. But fortunately it was a fork so let's keep going\n");
                sleep(3);
            }
        }

        printf("Now we jump into ourselves (it will be forever, so press 'q' to stop): \n");

        pid_t pid2 = fork();

        if (!pid2) {
            __asm__("1: jmp 1b");
        } else {
            struct termios old_t, new_t;
            tcgetattr(STDIN_FILENO, &old_t);
            new_t = old_t;
            new_t.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &new_t);

            char c;
            while (1) {
                if (read(STDIN_FILENO, &c, 1) > 0) {
                    if (c == 'q') {
                        printf("Ok, stopping\n");
                        kill(pid2, SIGTERM);
                        break;
                    }
                }
            }

            sleep(3);

            printf("It gets a bit boring, so let's try something else. For example, we're gonna be printing a string, but string won't have a null terminator (for some reason it work even without it on my PC): ");
            char s[] = {'H', 'e', 'l', 'l', 'o'};

            printf("%s\n", s);

            sleep(3);

            printf("And for the end, let's scan our pretty stack before we crash :)\n");
            char s2[] = "\0";
            for (size_t i = 0;; i++) {
                printf("%c", s2[i]);
            }

        }
    }
    
	return 0;
}
