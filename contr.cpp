#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <string>

const float PI = 3.1415926535f;

// хранилище
struct EvalResult {
    double value;
    double error_pct;
};

// эталон
double get_analytical(float T, bool squared) {
    return squared ? (T / 2.0) : std::sqrt(T / PI);
}

// сумма
float sum_basic(const float* vals, int n) {
    float s = 0;
    for (int i = 0; i < n; ++i) s += vals[i];
    return s;
}

// рекурсия
float sum_recursive(const float* vals, int start, int end) {
    if (end - start <= 1) return vals[start];
    int mid = start + (end - start) / 2;
    return sum_recursive(vals, start, mid) + sum_recursive(vals, mid, end);
}

// то же но цикл
float sum_pairwise_iter(const float* source_vals, int n) {
    if (n == 0) return 0.0f;
    float* vals = new float[n];
    for (int i = 0; i < n; ++i) vals[i] = source_vals[i];

    int cur_n = n;
    while (cur_n > 1) {
        for (int i = 0; i < cur_n / 2; ++i) {
            vals[i] = vals[2 * i] + vals[2 * i + 1];
        }
        if (cur_n % 2 != 0) {
            vals[cur_n / 2] = vals[cur_n - 1];
            cur_n = cur_n / 2 + 1;
        } else {
            cur_n = cur_n / 2;
        }
    }
    float result = vals[0];
    delete[] vals;
    return result;
}

// алгоритм кахана
float sum_kahan(const float* vals, int n) {
    float sum = 0.0f, c = 0.0f;
    for (int i = 0; i < n; ++i) {
        float y = vals[i] - c;
        float t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}

//суммирование в Double
double sum_double(const float* vals, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; ++i) sum += (double)vals[i];
    return sum;
}

//метод для выбора интеграла
EvalResult integrate(float T, int N, int method, bool squared) {
    float v_max = 5.0f * std::sqrt(T);
    float v_min = -v_max;
    float dv = (v_max - v_min) / N;
    
    float* vals = new float[N];
    for (int i = 0; i < N; ++i) {
        float v = v_min + i * dv;
        float pdf = (1.0f / std::sqrt(T * PI)) * std::exp(-(v * v) / T);
        float psi = squared ? (v * v) : std::abs(v);
        vals[i] = psi * pdf * dv;
    }

    double res = 0;
    if (method == 0)      res = sum_basic(vals, N);
    else if (method == 1) res = sum_recursive(vals, 0, N);
    else if (method == 2) res = sum_pairwise_iter(vals, N);
    else if (method == 3) res = sum_kahan(vals, N);
    else                  res = sum_double(vals, N);

    double analytical = get_analytical(T, squared);
    double error = std::abs((res / analytical - 1.0) * 100.0);
    
    delete[] vals;
    return {res, error};
}

int main() {
    float temps[] = {0.1f, 1.0f, 100.0f};
    std::string methods[] = {"Basic", "Recursive", "Pairwise_Iter", "Kahan", "Double"};
    
    std::ofstream out("results.csv");
    out << "T,N,Method,ErrorPct\n";

    for (float T : temps) {
        for (int N = 10; N <= 1000000; N *= 10) { 
            for (int m = 0; m < 5; ++m) {
                EvalResult r = integrate(T, N, m, false); 
                out << std::fixed << std::setprecision(1) << T << "," 
                    << std::fixed << std::setprecision(0) << N << "," 
                    << methods[m] << "," 
                    << std::scientific << std::setprecision(12) << r.error_pct << "\n";
            }
        }
    }
    out.close();
    return 0;
}
