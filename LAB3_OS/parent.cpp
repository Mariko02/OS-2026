#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string>
using namespace std;


struct Shared {
    int length;
    char text[16];
};

string shm_name;
string free_name;
string ready_name;


void remove_names() {
    shm_unlink(shm_name.c_str());
    sem_unlink(free_name.c_str());
    sem_unlink(ready_name.c_str());
}

void print_error(const string& error){
    write(STDERR_FILENO, error.c_str(), error.size());
    remove_names();
    _exit(1);
}

int main(){

    string name;

        while (true) {
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

    int opened_file = open(name.c_str(), O_RDONLY);

    if (opened_file == -1) {
        print_error("fail to open file\n");
    }

    string id = to_string(getpid());
    shm_name = "/lab3_shm_" + id;
    free_name = "/lab3_free_" + id;
    ready_name = "/lab3_ready_" + id;

    int shm_fd = shm_open(shm_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shm_fd == -1) {
        print_error("error: failed to create shared memory\n");
    }

    if (ftruncate(shm_fd, sizeof(Shared)) == -1) {
        print_error("error: failed to resize shared memory\n");
    }

    Shared* shared = (Shared*)mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        print_error("error: failed to map shared memory\n");
    }

    sem_t* sem_free = sem_open(free_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600, 1);
    sem_t* sem_ready = sem_open(ready_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600, 0);
    if (sem_free == SEM_FAILED || sem_ready == SEM_FAILED) {
        print_error("error: failed to create semaphore\n");
    }

    pid_t pid = fork();

    if (pid == -1) {
        print_error("error: failed to fork\n");
    }
    else if (pid == 0) {
        if (dup2(opened_file, STDIN_FILENO) == -1) {
            print_error("error: failed to redirect stdin\n");
        }

        close(opened_file);

        char* args[] = { (char*)"child", (char*)shm_name.c_str(), (char*)free_name.c_str(), (char*)ready_name.c_str(), nullptr };
        execv("./child", args);
        print_error("error: failed to exec child\n");
    }
    else {
        close(opened_file);

        while (true) {
            if (sem_wait(sem_ready) == -1) {
                print_error("error: failed to wait semaphore\n");
            }

            if (shared->length <= 0) {
                break;
            }

            if (write(STDOUT_FILENO, shared->text, shared->length) == -1) {
                print_error("error: failed to write to stdout\n");
            }

            if (sem_post(sem_free) == -1) {
                print_error("error: failed to post semaphore\n");
            }
        }

        int status;
        if (waitpid(pid, &status, 0) == -1) {
            print_error("error: failed to wait for child\n");
        }

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            print_error("error: child process failed\n");
        }

        munmap(shared, sizeof(Shared));
        close(shm_fd);
        sem_close(sem_free);
        sem_close(sem_ready);
        remove_names();
    }

    return 0;
}