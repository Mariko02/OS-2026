#include <unistd.h>
#include <pthread.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <cstdio>

using std::vector;

const size_t MIN_WORK_PER_THREAD = 50000;

bool g_pause = false;


void sum_range(const vector<int64_t>& data, vector<int64_t>& res,
               size_t k, size_t n, size_t j_begin, size_t j_end) {
    for (size_t j = j_begin; j < j_end; j++) {
        res[j] = 0;
    }

    for (size_t i = 0; i < k; i++) {
        for (size_t j = j_begin; j < j_end; j++) {
            res[j] += data[i * n + j];
        }
    }
}


double elapsed_ms(timespec start, timespec end) {
    double sec  = static_cast<double>(end.tv_sec)  - static_cast<double>(start.tv_sec);
    double nsec = static_cast<double>(end.tv_nsec) - static_cast<double>(start.tv_nsec);
    return sec * 1000.0 + nsec / 1000000.0;
}


struct ThreadArgs {
    const vector<int64_t>* data;
    vector<int64_t>* res;
    size_t k;
    size_t n;
    size_t j_begin;
    size_t j_end;
};


void* worker(void* args) {
    ThreadArgs* p = static_cast<ThreadArgs*>(args);
    if (g_pause) {
        sleep(15);
    }
    sum_range(*p->data, *p->res, p->k, p->n, p->j_begin, p->j_end);
    return nullptr;
}


size_t choose_threads(size_t k, size_t n, size_t t) {
    size_t threads = k * n / MIN_WORK_PER_THREAD;
    if (threads > t) {
        threads = t;
    }
    if (threads > n) {
        threads = n;
    }
    if (threads < 1) {
        threads = 1;
    }
    return threads;
}


void sum_parallel(const vector<int64_t>& data, vector<int64_t>& res,
                  size_t k, size_t n, size_t t) {
    if (t == 1) {
        sum_range(data, res, k, n, 0, n);
        return;
    }

    vector<pthread_t> threads(t);
    vector<ThreadArgs> thread_args(t);

    for (size_t p = 0; p < t; p++) {
        thread_args[p].data = &data;
        thread_args[p].res = &res;
        thread_args[p].k = k;
        thread_args[p].n = n;
        thread_args[p].j_begin = p * n / t;
        thread_args[p].j_end = (p + 1) * n / t;

        pthread_create(&threads[p], nullptr, worker, &thread_args[p]);
    }

    for (size_t p = 0; p < t; p++) {
        pthread_join(threads[p], nullptr);
    }
}


int main(int argc, char** argv) {

    size_t k = 0;
    size_t n = 0;
    size_t t = 0;

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            k = strtoull(argv[i + 1], nullptr, 10);
        }

        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            n = strtoull(argv[i + 1], nullptr, 10);
        }

        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            t = strtoull(argv[i + 1], nullptr, 10);
        }

        if (strcmp(argv[i], "-p") == 0) {
            g_pause = true;
        }
    }

    if (t == 0) {
        t = static_cast<size_t>(sysconf(_SC_NPROCESSORS_ONLN));
    }

    srand(time(nullptr));

    vector<int64_t> data(k * n);
    vector<int64_t> res_seq(n);
    vector<int64_t> res_par(n);

    for (size_t i = 0; i < data.size(); i++) {
        data[i] = rand() % 100;
    }

    size_t used = choose_threads(k, n, t);

    char buf[128];
    int len;

    if (g_pause) {
        len = snprintf(buf, sizeof(buf), "PID: %d, threads: %zu\n", static_cast<int>(getpid()), used);
        write(1, buf, len);
        sum_parallel(data, res_par, k, n, used);
        return 0;
    }

    timespec start;
    timespec end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    sum_range(data, res_seq, k, n, 0, n);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_seq = elapsed_ms(start, end);

    clock_gettime(CLOCK_MONOTONIC, &start);
    sum_parallel(data, res_par, k, n, used);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_par = elapsed_ms(start, end);

    double speedup = time_seq / time_par;
    double efficiency = speedup / static_cast<double>(used);

    len = snprintf(buf, sizeof(buf), "K=%zu N=%zu T(max)=%zu threads used=%zu\n", k, n, t, used);
    write(1, buf, len);

    len = snprintf(buf, sizeof(buf), "Sequential: %.3f ms\n", time_seq);
    write(1, buf, len);

    len = snprintf(buf, sizeof(buf), "Parallel:   %.3f ms\n", time_par);
    write(1, buf, len);

    len = snprintf(buf, sizeof(buf), "Speedup:    %.3f\n", speedup);
    write(1, buf, len);

    len = snprintf(buf, sizeof(buf), "Efficiency: %.3f\n", efficiency);
    write(1, buf, len);

    return 0;
}