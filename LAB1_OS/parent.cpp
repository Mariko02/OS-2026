#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string>
using namespace std;


void print_error(const string& error){
    write(STDERR_FILENO, error.c_str(), error.size());
    _exit(1); 
}

int main(){

    string name;

        while (true) { //read and save name of file
        char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);

        if (n == -1) {
            print_error("error: failed to read file name\n");
        }
        if (n == 0) {
            break;
        }
        if (c == '\n') {
            break;
        }
        name += c;
    }
    if (name.empty()){
        print_error("file name is empty\n");
    }

    int opened_file = open(name.c_str(), O_RDONLY); //open file

    if (opened_file == -1) {
        print_error("fail to open file\n");
    }

    int pipe1[2]; // create pipe (pipe for convversation child -> parent)
    if (pipe(pipe1) == -1) {
        print_error("failed to create pipe\n");
    }

    pid_t pid = fork();

    if (pid == -1) {
        print_error("error: failed to fork\n");
    }
    else if (pid == 0) { //child branch
        close(pipe1[0]);

        if (dup2(opened_file, STDIN_FILENO) == -1) {
            print_error("error: failed to redirect stdin\n");
        }
        if (dup2(pipe1[1], STDOUT_FILENO) == -1) {
            print_error("error: failed to redirect stdout\n");
        }

        close(opened_file);
        close(pipe1[1]);

        char* args[] = { (char*)"child", nullptr };
        execv("./child", args);
        print_error("error: failed to exec child\n");
    }
    else { //parent branch
        close(opened_file);
        close(pipe1[1]);

        char buf[4096];
        while (true) {
            ssize_t n = read(pipe1[0], buf, sizeof(buf));

            if (n == -1) {
                print_error("error: failed to read from pipe\n");
            }
            if (n == 0) {
                break;
            }

            if (write(STDOUT_FILENO, buf, n) == -1) {
                print_error("error: failed to write to stdout\n");
            }
        }
        close(pipe1[0]);


        int status;
        if (waitpid(pid, &status, 0) == -1) { // check status of children
            print_error("error: failed to wait for child\n");
        }

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            print_error("error: child process failed\n");
        }
    }

    return 0;
}

