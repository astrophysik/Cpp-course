#include "integral_calculation.h"

std::pair<std::vector<double>, std::vector<double>> thread_generate_curve_splitting(const std::function<double(double)> &x,
                                                                                    const std::function<double(double)> &y,
                                                                                    std::pair<double, double> range) {
    double step = range.first < range.second ? delta : -1 * delta;
    while (true) {
        auto n = static_cast<size_t>(((range.second - range.first) / step + 1.0));
        std::vector<double> vector_x(n);
        std::vector<double> vector_y(n);
        vector_x[0] = x(range.first);
        vector_y[0] = y(range.first);
        double max_delta_x = std::numeric_limits<double>::min();
        double max_delta_y = std::numeric_limits<double>::min();
        for (size_t i = 1; i < n; ++i) {
            double next_x = x(range.first + i * step);
            double next_y = y(range.first + i * step);
            vector_x[i] = next_x;
            vector_y[i] = next_y;
            if (std::abs(vector_x[i] - vector_x[i - 1]) > max_delta_x) {
                max_delta_x = std::abs(vector_x[i] - vector_x[i - 1]);
            }
            if (std::abs(vector_y[i] - vector_y[i - 1]) > max_delta_y) {
                max_delta_y = std::abs(vector_y[i] - vector_y[i - 1]);
            }
        }
        if (std::sqrt(std::pow(max_delta_x, 2) + std::pow(max_delta_y, 2)) <= delta) {
            return {vector_x, vector_y};
        }
        step /= 2;
    }
}

std::pair<std::vector<double>, std::vector<double>> generate_curve_splitting(const parameterized_part_curve &part_curve) {
    size_t threads_count = thread_count_for_splitting;
    std::vector<std::thread> threads;
    std::vector<std::pair<std::vector<double>, std::vector<double>>> vectors(threads_count);
    std::vector<std::pair<double, double>> ranges;
    auto range_element = (part_curve.range.second - part_curve.range.first) / threads_count;
    for (size_t i = 0; i < threads_count; ++i) {
        ranges.emplace_back(part_curve.range.first + range_element * i, part_curve.range.first + range_element * (i + 1));
        threads.emplace_back([&vectors, &ranges, i](const parameterized_part_curve &part) {
            auto [vector_x, vector_y] = thread_generate_curve_splitting(part.x, part.y, ranges[i]);
            vectors[i].first = vector_x;
            vectors[i].second = vector_y;
        }, part_curve);
    }
    for (size_t i = 0; i < threads_count; ++i) {
        threads[i].join();
    }
    std::vector<double> result_vector_x;
    std::vector<double> result_vector_y;
    for (size_t i = 0; i < threads_count; ++i) {
        result_vector_x.insert(result_vector_x.end(), vectors[i].first.begin(), vectors[i].first.end());
        result_vector_y.insert(result_vector_y.end(), vectors[i].second.begin(), vectors[i].second.end());
    }
    return {result_vector_x, result_vector_y};
}

double calculate_curve_second_integral(const function & func, const std::vector<double> &vector_x, const std::vector<double> &vector_y) {
    double ans = 0;
    for (size_t i = 1; i < vector_x.size(); ++i) {
        auto [dp, dq] = func.attributes.potential(vector_x[i], vector_y[i]);
        ans += dp * (vector_x[i] - vector_x[i - 1]);
        ans += dq * (vector_y[i] - vector_y[i - 1]);
    }
    return ans;
}

double curvilinear_integral_of_second_kind(const function & func) {
    double sum = 0;
    size_t parts_count = 2;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < parts_count; ++i) {
        threads.emplace_back([&sum, &func, i]() {
            auto [vector_x, vector_y] = generate_curve_splitting(func.curve.parameterized_parts[i]);
            sum += calculate_curve_second_integral(func, vector_x, vector_y);
        });
    }
    for (size_t i = 0; i < parts_count; ++i) {
        threads[i].join();
    }
    return sum;
}

std::pair<std::vector<double>, std::vector<double>> generate_square_plane_splitting(double min_x, double max_x, double min_y, double max_y) {
    std::vector<double> vector_x((max_x - min_x) / delta);
    std::vector<double> vector_y((max_y - min_y) / delta);
    std::thread thread_for_x([&vector_x, min_x]() {
        for (size_t i = 0; i < vector_x.size(); ++i) {
            vector_x[i] = min_x + delta / 2 + i * delta;
        }
    });
    std::thread thread_for_y([&vector_y, min_y]() {
        for (size_t i = 0; i < vector_y.size(); ++i) {
            vector_y[i] = min_y + delta / 2 + i * delta;
        }
    });
    thread_for_x.join();
    thread_for_y.join();
    return {vector_x, vector_y};
}

double evaluate_double_integral(const function & func, const std::vector<double> &vector_x, const std::vector<double> &vector_y) {
    double sum = 0;
    size_t thread_counts = 12;
    std::vector<std::thread> threads_for_evaluate_integral;
    size_t range_step = (vector_x.size()) / thread_counts;
    for (size_t i = 0; i < thread_counts; ++i) {
        threads_for_evaluate_integral.emplace_back([&sum, &vector_x, &vector_y, &func, i, range_step]() {
            double current_sum = 0;
            for (size_t j = i * range_step; j < (i + 1) * range_step; ++j) {
                for (auto k : vector_y) {
                    if (func.attributes.d(vector_x[j], k)) {
                        current_sum += func.attributes.integrand(vector_x[j], k) * delta * delta;
                    }
                }
            }
            sum += current_sum;
        });
    }
    for (size_t i = 0; i < thread_counts; ++i) {
        threads_for_evaluate_integral[i].join();
    }
    return sum;
}

double double_integral_sum(const function & func) {
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::min();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::min();
    std::vector<std::pair<std::vector<double>, std::vector<double>>> vectors(func.curve.parameterized_parts.size());
    std::vector<std::thread> threads_for_calc_parts;
    for (size_t i = 0; i < func.curve.parameterized_parts.size(); ++i) {
        threads_for_calc_parts.emplace_back([&vectors, &func, i]() {
            vectors[i] = generate_curve_splitting(func.curve.parameterized_parts[i]);
        });
    }
    for (size_t i = 0; i < func.curve.parameterized_parts.size(); ++i) {
        threads_for_calc_parts[i].join();
    }
    std::thread thread_for_x([&min_x, &max_x, &vectors]() {
        for (auto &vector : vectors) {
            for (double i : vector.first) {
                if (i < min_x) {
                    min_x = i;
                } else if (i > max_x) {
                    max_x = i;
                }
            }
        }
    });
    std::thread thread_for_y([&min_y, &max_y, &vectors]() {
        for (auto &vector : vectors) {
            for (double i : vector.second) {
                if (i < min_y) {
                    min_y = i;
                } else if (i > max_y) {
                    max_y = i;
                }
            }
        }
    });
    thread_for_x.join();
    thread_for_y.join();
    auto [vector_x, vector_y] = generate_square_plane_splitting(min_x, max_x, min_y, max_y);
    return evaluate_double_integral(func, vector_x, vector_y);
}
