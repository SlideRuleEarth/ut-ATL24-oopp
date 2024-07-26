#pragma once

#include "oopp/precompiled.h"

namespace oopp
{

namespace utils
{

/// @brief Normalize a container to 0.0, 1.0
/// @tparam T Container type
/// @param x Container
/// @return Normalized values
template <typename T>
T normalize (const T &x)
{
    assert (!x.empty ());
    const auto xmin = *std::min_element (x.begin (), x.end ());
    const auto xmax = *std::max_element (x.begin (), x.end ());
    T y (x);

    // Do the scaling
    const double d = xmax - xmin;
    for (size_t i = 0; i < y.size (); ++i)
        y[i] = (y[i] - xmin) / d;

    return y;
}

/// @brief Get the mean of the values in a container
/// @tparam T Container type
/// @param x Container
/// @return Mean
template <typename T>
double mean (const T &x)
{
    size_t total = 0;
    double sum = 0.0;
    for (size_t i = 0; i < x.size (); ++i)
    {
        ++total;
        sum += x[i];
    }

    // Degenerate case
    if (total == 0)
        return 0.0;

    return sum / total;
}

/// @brief Get the variance of the values in a container
/// @tparam T Container type
/// @param x Container
/// @return Variance
template <typename T>
double variance (const T &x)
{
    size_t total = 0;
    double sum = 0.0;
    double sum2 = 0.0;
    for (size_t i = 0; i < x.size (); ++i)
    {
        ++total;
        sum += x[i];
        sum2 += x[i] * x[i];
    }

    // Degenerate case
    if (total == 0)
        return 0.0;

    // Variance = E[x^2] - E[x]^2
    const double mean = sum / total;
    const double var = sum2 / total - mean * mean;

    // E[x^2] >= E[x]^2
    assert (var >= 0.0);

    return var;
}


/// @brief Get the z-scores of the values in a container
/// @tparam T Container type
/// @param x Container
/// @return Z-scored values
template <typename T>
T z_score (const T &x)
{
    const double u = mean (x);
    const double s = std::sqrt (variance (x));

    // Copy it
    T y (x);

#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < y.size (); ++i)
        y[i] = (y[i] - u) / s;

    return y;
}


/// @brief Get the median value from an unsorted container
/// @tparam T Container type
/// @param x Unsorted container
/// @return The median
template <typename T>
typename T::value_type median (T x)
{
    // Check args
    assert (!x.empty ());

    // Partial sort is O(n) vs O(n lg (n)) for sort()
    std::nth_element (x.begin (), x.begin () + x.size () / 2, x.end ());

    // Get the median
    return x[x.size () / 2];
}

namespace detail
{

/// @brief Box 1D filter helper function
template<typename T, typename U>
typename T::value_type get_row_average (const T &sums, const U &totals, const size_t sz, const size_t len, const size_t i)
{
    // Check args
    assert (sums.size () == totals.size ());
    assert (i < sums.size ());

    // Get sums and totals on the edges of the filter window
    const int i1 = i - sz / 2 - 1;
    const int i2 = i + sz / 2;

    // Clip the index values to the edges of the row
    const double sum1 = (i1 < 0) ? 0 : sums[i1];
    const size_t total1 = (i1 < 0) ? 0 : totals[i1];
    const double sum2 = (i2 >= static_cast<int>(len)) ? sums[len - 1] : sums[i2];
    const size_t total2 = (i2 >= static_cast<int>(len)) ? totals[len - 1] : totals[i2];

    // Compute sum and total at pixel 'i'.
    //
    // The sums array contains the cumulative sums, so we can just subtract the first sum from the second sum to get the
    // sum across the part of the row that we are interested in. The same goes for the totals array.
    //
    const double sum = sum2 - sum1;
    const int total = total2 - total1;
    assert (total > 0);

    // Return the average over the window of size 'sz' at pixel 'i'
    return sum / total;
}

} // namespace detail

/// @brief Box filter a 1D array of pixels
/// @tparam T Pixel iterator type
/// @param p_begin Pixel iterator
/// @param p_end Pixel iterator
/// @param sz Kernel size
///
/// The filtering is done in-place because it is a support routine that operates on an image row.
template<typename T>
void box_1D_filter (T p_begin, const T p_end, const size_t sz)
{
    T p = p_begin;
    const size_t len = p_end - p_begin;

    // Keep cumulative sums and totals across the array
    std::vector<double> sums (len);
    std::vector<size_t> totals (len);
    double cumulative_sum = 0.0;
    size_t cumulative_total = 0;

    // For each pixel get cumulative sums and counts
    for (size_t i = 0; i < len; ++i)
    {
        // Update counts
        cumulative_sum += p[i];
        ++cumulative_total;
        // Remember them
        sums[i] = cumulative_sum;
        totals[i] = cumulative_total;
    }

    // Now go back and fill in filter values based upon sums and totals
    for (size_t i = 0; i < len; ++i)
    {
        // Get the box filter average
        p[i] = detail::get_row_average (sums, totals, sz, len, i);
    }
}

/// @brief Get the ideal filter width of a box filter that
///        approximates a Gaussian filter
/// @param sigma Standard deviation of the Gaussian filter
/// @param n The number of iterations that the box filter is applied
/// @return Ideal box filter width
///
/// When approximating a Gaussian filter, this function calculates the
/// box filter width that should be used.
inline double ideal_filter_width (const double sigma, const size_t n)
{
    return std::sqrt ((12.0 * sigma * sigma) / n + 1.0);
}

/// @brief Filter a container with Gaussian kernel
/// @tparam T Container type
/// @param x Container
/// @param sigma Standard deviation of kernel, see note below
/// @param n Number of iterations used in the approximation
/// @return Filtered image
/// @cite "Kovesi, Peter. "Fast almost-gaussian filtering." Digital
///       Image Computing: Techniques and Applications (DICTA), 2010
///       International Conference on. IEEE, 2010."
template<typename T>
T gaussian_1D_filter (T x, const double sigma, const size_t n = 5)
{
    // Get the ideal box filter kernel size
    const double w = ideal_filter_width (sigma, n);
    assert (w >= 1.0);

    // Get the smaller integer kernel size
    int wl = std::floor (w);

    // Make it odd
    if ((wl & 1) == 0)
        --wl;

    assert (wl >= 1.0);

    // Get the larger integer kernel size
    const int wu = wl + 2;

    // Get the number of iteration of size wl
    const size_t m = round (
        (12 * sigma * sigma
         - n * wl * wl
         - 4 * n * wl - 3 * n)
        / (-4 * wl - 4));

    // Approximate a Gaussian filter by iteratively applying a box filter
    //
    // First apply small kernel
    for (size_t i = 0; i < m; ++i)
        box_1D_filter (x.begin (), x.end (), wl);

    // Then apply large kernel
    assert (n >= m);
    for (size_t i = 0; i < (n - m); ++i)
        box_1D_filter (x.begin (), x.end (), wu);

    return x;
}

/// @brief Return a list of indices above a given peak value
/// @tparam T Container type
/// @param x Container
/// @return Peak container
template <typename T>
std::vector<size_t> find_peaks(const T &x)
{
    std::vector<size_t> peaks;

    if (x.size() < 3)
        return peaks;

    // If the center value is greater than value on the left and right...
    for (size_t index = 1; index + 1 < x.size(); ++index)
        if (x[index-1] < x[index] && x[index+1] < x[index])
            peaks.push_back(index);

    return peaks;
}

} // namespace utils

} // namespace oopp
