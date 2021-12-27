#include <iostream>
#include <iomanip>

#include "integral_calculation.h"

int main() {
    const function global_function(parameterized_curve({
                    parameterized_part_curve([](double t) { return 2 * std::cos(t); },
                                             [](double t) { return 2 * std::sin(t); },
                                             {M_PI / 4.0, M_PI * 5.0 / 4.0}),
                    parameterized_part_curve([](double t) { return t; },
                                             [](double t) { return t; },
                                             {-std::sqrt(2), std::sqrt(2)})}),
                   function_attributes([](double x, double y) { return x <= y && x * x + y * y <= 4;},
                                      [](double x, double y) -> std::pair<double, double> { return {x + y, 2 * x * y};},
                                      [](double x, double y) { return 2 * y - 1; }));

    double true_result = 1.2592870254769206;
    /// Paragraph 1
    set_delta(0.0001);
    std::clock_t start = std::clock();
    double second_kind_integral = curvilinear_integral_of_second_kind(global_function);
    std::clock_t end = std::clock();
    double diff = static_cast<double>(end - start) / static_cast<double>(CLK_TCK);
    std::cout << "The integral sum of a curve linear integral of the second kind: " << std::setprecision(10) << second_kind_integral << "\n"
              << "deviation from true value: " << std::abs(true_result - second_kind_integral) << "\n"
              << "Time: " << diff << "\n";

    /// Paragraph 2
    set_delta(0.0005);
    start = std::clock();
    double double_int_sum = double_integral_sum(global_function);
    end = std::clock();
    diff = static_cast<double>(end - start) / static_cast<double>(CLK_TCK);
    std::cout << "The integral sum of double integral: " << std::setprecision(10) << double_int_sum << "\n"
              << "deviation from true value: " << std::abs(true_result - double_int_sum) << "\n"
              << "Time: " << diff << "\n";

    /// Paragraph 3
    auto minimum_func = global_function;
    minimum_func.attributes.d = [](double x, double y) {return x <= y && x * x + y * y <= 4 - 0.01;};
    auto maximum_func = global_function;
    maximum_func.attributes.d = [](double x, double y) {return x <= y && x * x + y * y <= 4 + 0.01;};
    start = std::clock();
    double min_double_integral_sum = double_integral_sum(minimum_func);
    double max_double_integral_sum = double_integral_sum(maximum_func);
    end = std::clock();
    diff = static_cast<double>(end - start) / static_cast<double>(CLK_TCK);
    std::cout << "The maximum integral sum of double integral: " << std::setprecision(10) << max_double_integral_sum << "\n"
              << "The minimum integral sum of double integral: " << min_double_integral_sum << "\n"
              << "deviation:  " << std::abs(max_double_integral_sum - min_double_integral_sum) << "\n"
              << "Time: " << diff << "\n";
    return 0;
}
