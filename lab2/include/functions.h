#pragma once

#include <stdbool.h>
#include <math.h>

static inline bool isPrime(int n) {
    for (int i = 2; i <= sqrt(n); i++)
        if (n % i == 0)
            return false;
    return true;
}

static inline double average_first_n_primes(int n) {
    int sum_primes = 0, prime_count = 0;

    for (int i = 2; i < n; i++)
        if (isPrime(i)) {
            prime_count++;
            sum_primes += i;
        }

    return (double)sum_primes/prime_count;
}
