#include <iostream>
#include <vector>
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

// прямое суммирование
float sum_basic(const std::vector<float>& vals) {
    float s = 0;
    for (float v : vals) s += v;
    return s;
}

// рекурсия
float sum_recursive(const std::vector<float>& vals, int start, int end) {
    if (end - start <= 1) return vals[start];
    int mid = start + (end - start) / 2;
    return sum_recursive(vals, start, mid) + sum_recursive(vals, mid, end);
}

// алгоритм К(п)ахана
float sum_kahan(const std::vector<float>& vals) {
    float sum = 0.0f, c = 0.0f;
    for (float v : vals) {
        float y = v - c;
        float t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}

// суммирование в Double
double sum_double(const std::vector<float>& vals) {
    double sum = 0.0;
    for (float v : vals) sum += (double)v;
    return sum;
}

// интегральчик(вызов методов)
EvalResult integrate(float T, int N, int method, bool squared) {
    float v_max = 5.0f * std::sqrt(T);
    float v_min = -v_max;
    float dv = (v_max - v_min) / N;
    
    std::vector<float> vals(N);
    for (int i = 0; i < N; ++i) {
        float v = v_min + i * dv;
        float pdf = (1.0f / std::sqrt(T * PI)) * std::exp(-(v * v) / T);
        float psi = squared ? (v * v) : std::abs(v);
        vals[i] = psi * pdf * dv;
    }

    double res = 0;
    if (method == 0)      res = sum_basic(vals);
    else if (method == 1) res = sum_recursive(vals, 0, N);
    else if (method == 2) res = sum_kahan(vals);
    else                  res = sum_double(vals);

    double analytical = get_analytical(T, squared);
    double error = std::abs((res / analytical - 1.0) * 100.0);
    return {res, error};
}

int main() {
    std::vector<float> temps = {0.1f, 1.0f, 100.0f};
    std::vector<std::string> methods = {"Basic", "Recursive", "Kahan", "Double"};
    
    std::ofstream out("results.csv");
    out << "T,N,Method,ErrorPct\n";

    for (float T : temps) {
        for (int N = 10; N <= 1000000; N *= 5) { 
            for (int m = 0; m < (int)methods.size(); ++m) {
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
