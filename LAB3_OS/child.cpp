#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string>
#include <vector>
using namespace std;


struct Shared {
    int length;
    char text[16];
};

Shared* shared;
sem_t* sem_free;
sem_t* sem_ready;
bool connected = false;


void send_message(int length, const char* text) {
    if (sem_wait(sem_free) == -1) {
        _exit(1);
    }

    shared->length = length;
    if (length > 0) {
        memcpy(shared->text, text, length);
    }

    if (sem_post(sem_ready) == -1) {
        _exit(1);
    }
}

void print_error(const string& error){
    write(STDERR_FILENO, error.c_str(), error.size());
    if (connected) {
        send_message(-1, nullptr);
    }
    _exit(1);
}


void process_line(const string& line) {
    vector<int> nums;
    size_t i = 0;

    while (i < line.size()) {
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')) {
            i++;
        }

        if (i == line.size()) {
            break;
        }

        bool negative = false;
        if (line[i] == '-') {
            negative = true;
            i++;
        }

        if (i == line.size() || line[i] < '0' || line[i] > '9') {
            print_error("error: invalid number\n");
        }

        long long value = 0;
        while (i < line.size() && line[i] >= '0' && line[i] <= '9') {
            value = value * 10 + (line[i] - '0');
            if (value > 2147483648) {
                print_error("error: number out of range\n");
            }
            i++;
        }

        if (negative) {
            value = -value;
        }
        if (value > 2147483647) {
            print_error("error: number out of range\n");
        }

        nums.push_back((int)value);
    }

    if (nums.empty()) {
        return;
    }
        long long result = nums[0];

    for (size_t j = 1; j < nums.size(); j++) {
        if (nums[j] == 0) {
            print_error("error: division by zero\n");
        }
        result = result / nums[j];
    }

    char digits[16];
    int pos = 16;

    digits[--pos] = '\n';

    long long x = result;
    bool negative = x < 0;
    if (negative) {
        x = -x;
    }

    do {
        digits[--pos] = (char)('0' + x % 10);
        x = x / 10;
    } while (x > 0);

    if (negative) {
        digits[--pos] = '-';
    }

    send_message(16 - pos, digits + pos);
}



int main(int argc, char* argv[]) {

    (void)argc;

    int shm_fd = shm_open(argv[1], O_RDWR, 0);
    if (shm_fd == -1) {
        print_error("error: failed to open shared memory\n");
    }

    shared = (Shared*)mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        print_error("error: failed to map shared memory\n");
    }

    sem_free = sem_open(argv[2], O_RDWR);
    sem_ready = sem_open(argv[3], O_RDWR);
    if (sem_free == SEM_FAILED || sem_ready == SEM_FAILED) {
        print_error("error: failed to open semaphore\n");
    }
    connected = true;

    char buf[4096];
    string line;

    while (true) {
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));

        if (n == -1) {
            print_error("error: failed to read input\n");
        }
        if (n == 0) {
            break;
        }

    for(int i = 0; i < n; ++i) {
        if (buf[i] == '\n') {
            process_line(line);
            line.clear();
        }
        else {
            line += buf[i];
        }
    }
    }

if (!line.empty()) {
        process_line(line);
    }

    send_message(0, nullptr);

    munmap(shared, sizeof(Shared));
    close(shm_fd);
    sem_close(sem_free);
    sem_close(sem_ready);

    return 0;}