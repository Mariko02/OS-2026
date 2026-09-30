#include <unistd.h>
#include <string>
#include <vector>
using namespace std;


void print_error(const string& error){
    write(STDERR_FILENO, error.c_str(), error.size());
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
        int result = nums[0];

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

    if (write(STDOUT_FILENO, digits + pos, 16 - pos) == -1) {
        print_error("error: failed to write result\n");
    }
}



int main() {

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

    return 0;
}