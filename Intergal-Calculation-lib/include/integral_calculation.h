#pragma once

#include <cmath>
#include <functional>
#include <limits>
#include <thread>
#include <utility>

struct function_attributes {
    function_attributes(std::function<bool(double, double)> d,
                        std::function<std::pair<double, double>(double, double)> pot,
                        std::function<double(double, double)> inte) : d(std::move(d)), potential(std::move(pot)), integrand(std::move(inte)) {}

    std::function<bool(double, double)> d;
    std::function<std::pair<double, double>(double, double)> potential;
    std::function<double(double, double)> integrand;
};

struct parameterized_part_curve {
    parameterized_part_curve(std::function<double(double)> x,
                             std::function<double(double)> y,
                             std::pair<double, double> range) : x(std::move(x)), y(std::move(y)), range(std::move(range)) {}

    std::function<double(double)> x;
    std::function<double(double)> y;
    std::pair<double, double> range;
};

struct parameterized_curve {
    parameterized_curve() = default;

    parameterized_curve(const std::vector<parameterized_part_curve> &other) : parameterized_parts(other) {}

    parameterized_curve(std::vector<parameterized_part_curve> &&other) : parameterized_parts(std::move(other)) {}

    void add_part(const parameterized_part_curve &part) {
        parameterized_parts.push_back(part);
    }

    void add_part(parameterized_part_curve &&part) {
        parameterized_parts.push_back(part);
    }

    std::vector<parameterized_part_curve> parameterized_parts;
};

struct function {
    function(parameterized_curve par_curve, function_attributes func_attr) : curve(std::move(par_curve)), attributes(std::move(func_attr)) {}

    parameterized_curve curve;
    function_attributes attributes;
};

inline double delta = 0.01;

inline size_t thread_count_for_splitting = 12;

inline constexpr void set_thread_count_for_splitting(size_t new_count) {
    thread_count_for_splitting = new_count;
}

inline constexpr void set_delta(double new_delta) {
    delta = new_delta;
}

namespace integral_calculation_details {
    /// \param x - parameterized x from t
    /// \param y - parameterized y from t
    /// \param range - range of t values
    /// \return curve splitting
    std::pair<std::vector<double>, std::vector<double>> thread_generate_curve_splitting(const std::function<double(double)> &x,
                                                                                        const std::function<double(double)> &y,
                                                                                        std::pair<double, double> range);

    /// \param part_curve - parameterized part of curve
    /// Function create threads_count of threads to calculate splitting
    /// \result splitting of curve
    std::pair<std::vector<double>, std::vector<double>> generate_curve_splitting(const parameterized_part_curve &part_curve);

    /// \param func - integration function
    /// \param vector_x - x coordinates of splitting of curve
    /// \param vector_y - y coordinates of splitting of curve
    /// \return calculated part of integral
    double calculate_curve_second_integral(const function &func, const std::vector<double> &vector_x, const std::vector<double> &vector_y);

    /// \param min_x - the minimum value of the coordinate of the point along the X-axis
    /// \param max_x -the maximum value of the coordinate of the point along the X-axis
    /// \param min_y - the minimum value of the coordinate of the point along the Y-axis
    /// \param max_y -the maximum value of the coordinate of the point along the Y-axis
    /// \result a pair of lists matching all possible X and Y coordinates for grid points
    std::pair<std::vector<double>, std::vector<double>> generate_square_plane_splitting(double min_x, double max_x, double min_y, double max_y);

    /// \param vector_x - the x coordinates of the split points
    /// \param vector_y - the y coordinates of the split points
    /// \return calculated sum
    double evaluate_double_integral(const function &func, const std::vector<double> &vector_x, const std::vector<double> &vector_y);
}// namespace integral_calculation_details

/// \param func - integration function
/// Calculates the curvilinear integral of the second kind given by constants. Function create thread for every parameterized part
/// @return integral meaning
double curvilinear_integral_of_second_kind(const function &func);

/// \param func - integration function
/// \return integral meaning
double double_integral_sum(const function &func);
