#pragma once

#include "oopp/precompiled.h"
#include "oopp/utils.h"

namespace oopp
{

// ASPRS Definitions
constexpr unsigned unprocessed_class = 0;
constexpr unsigned unclassified_class = 1;
constexpr unsigned bathy_class = 40;
constexpr unsigned sea_surface_class = 41;
constexpr unsigned water_column_class = 45;

struct photon
{
    size_t h5_index;
    double x;
    double z;
    unsigned cls;
    unsigned prediction;
    double surface_elevation;
    double bathy_elevation;
};

std::ostream &operator<< (std::ostream &os, const photon &p)
{
    os << "index=" << p.h5_index
        << ",x=" << p.x
        << ",z=" << p.z
        << ",cls=" << p.cls
        << ",prediction=" << p.prediction
        << ",surface_elevation=" << p.surface_elevation
        << ",bathy_elevation=" << p.bathy_elevation
        << std::endl;
    return os;
}

struct params
{
    double x_resolution = 10.0; // meters
    double z_resolution = 0.2; // meters
    double z_min = -50; // meters
    double z_max = 30; // meters
    double surface_z_min = -20; // meters
    double surface_z_max = 20; // meters
    double bathy_min_depth = 0.5; // meters
    double vertical_smoothing_sigma = 0.5; // meters
    double surface_smoothing_sigma = 100.0; // meters
    double bathy_smoothing_sigma = 10.0; // meters
    double min_peak_prominence = 0.01; // probability
    size_t min_peak_distance = 2; // z bins
    size_t min_surface_photons_per_window = (x_resolution / 0.7) / 2.0; // count
    size_t min_bathy_photons_per_window = (x_resolution / 0.7) / 2.0; // count
};

std::ostream &operator<< (std::ostream &os, const params &params)
{
    os << "x-resolution: " << params.x_resolution << "m" << std::endl;
    os << "z-resolution: " << params.z_resolution << "m" << std::endl;
    os << "z-min: " << params.z_min << "m" << std::endl;
    os << "z-max: " << params.z_max << "m" << std::endl;
    os << "surface-z-min: " << params.surface_z_min << "m" << std::endl;
    os << "surface-z-max: " << params.surface_z_max << "m" << std::endl;
    os << "bathy-min-depth: " << params.bathy_min_depth << "m" << std::endl;
    os << "vertical-smoothing-sigma: " << params.vertical_smoothing_sigma << "m" << std::endl;
    os << "surface-smoothing-sigma: " << params.surface_smoothing_sigma << "m" << std::endl;
    os << "bathy-smoothing-sigma: " << params.bathy_smoothing_sigma << "m" << std::endl;
    os << "min-peak-prominence: " << params.min_peak_prominence << std::endl;
    os << "min-peak-distance: " << params.min_peak_distance << " bins" << std::endl;

    return os;
}

template<typename T>
void write_predictions (std::ostream &os, const T &p)
{
    using namespace std;

    // Save precision
    const auto pr = os.precision ();

    // Print along-track meters
    os << "index_ph,x_atc,geoid_corr_h,manual_label,prediction,sea_surface_h,bathy_h" << endl;
    for (size_t i = 0; i < p.size (); ++i)
    {
        // Write the index
        os << fixed;
        os << p[i].h5_index;
        os << setprecision (4) << fixed;;
        os << "," << p[i].x;
        os << setprecision (4) << fixed;;
        os << "," << p[i].z;
        // Write the class
        os << setprecision (0) << fixed;;
        os << "," << p[i].cls;
        // Write the prediction
        os << setprecision (0) << fixed;
        os << "," << p[i].prediction;
        // Write the surface estimate
        os << setprecision (4) << fixed;
        os << "," << p[i].surface_elevation;
        // Write the bathy estimate
        os << setprecision (4) << fixed;
        os << "," << p[i].bathy_elevation;
        os << endl;
    }

    // Restore precision
    os.precision (pr);
}

// Get a vector in which each entry corresponds to a (e.g. 10 meter) bin.
//
// Each bin will contain a vector the photon indexes that belong to that bin.
template<typename T,typename U>
std::vector<std::vector<size_t>> get_h_bins (const T &p, const U &params)
{
    using namespace std;
    vector<vector<size_t>> bins;

    if (p.empty ())
        return bins;

    // Get the bounds
    const double x_min = min_element (p.begin (), p.end (),
            [](const auto &a, const auto &b) { return a.x < b.x; })->x;
    const double x_max = max_element (p.begin (), p.end (),
            [](const auto &a, const auto &b) { return a.x < b.x; })->x;

    // Allocate vector of indexes
    const size_t total_bins = (x_max - x_min) / params.x_resolution + 1;
    bins.resize (total_bins);

    // Add indexes
    for (size_t i = 0; i < p.size (); ++i)
    {
        // Don't add ones that are out of range
        const double z = p[i].z;
        if (z > params.z_max || z < params.z_min)
            continue;

        assert (p[i].x >= x_min);
        size_t bin = (p[i].x - x_min) / params.x_resolution;
        assert (bin < bins.size ());
        bins[bin].push_back (i);
    }

    return bins;
}

template<typename T,typename U,typename V>
std::vector<std::vector<size_t>> get_v_bins (const T &p, const U &h_bins, const V &params)
{
    using namespace std;

    assert (params.z_max > params.z_min);
    const size_t total_bins = (params.z_max - params.z_min) / params.z_resolution + 1;
    vector<vector<size_t>> bins (total_bins);
    for (const auto i : h_bins)
    {
        assert (i < p.size ());
        assert (p[i].z >= params.z_min);
        size_t bin = (p[i].z - params.z_min) / params.z_resolution;
        assert (bin < bins.size ());
        bins[bin].push_back (i);
    }

    return bins;
}

template<typename T>
std::vector<double> get_v_bin_elevations (const T &params)
{
    using namespace std;

    assert (params.z_max > params.z_min);
    const size_t total_bins = (params.z_max - params.z_min) / params.z_resolution + 1;
    vector<double> e (total_bins);
    for (size_t i = 0; i < e.size (); ++i)
    {
        // Get the elevation of the middle of the bin
        const double bin_z_min = i * params.z_resolution + params.z_min;
        const double bin_z_max = (i + 1) * params.z_resolution + params.z_min;
        e[i] = (bin_z_min + bin_z_max) / 2.0;
    }

    return e;
}

template<typename T,typename U,typename V,typename W>
std::vector<size_t> get_surface_indexes (const T &p,
    const U &v_bins,
    const V &v_bin_elevations,
    const W &params)
{
    using namespace oopp::utils;
    using namespace std;

    // Check invariants
    assert (v_bins.size () == v_bin_elevations.size ());

    // Get a histogram from the bin indexes
    vector<size_t> h (v_bins.size ());

    transform (v_bins.begin (), v_bins.end (), h.begin (),
        [&](const auto &b) { return b.size (); });

    // Convert the histogram to a probability mass function
    auto pmf = convert_to_pmf<double> (h);

    // Smooth it
    pmf = gaussian_1D_filter (pmf, params.vertical_smoothing_sigma);

    // Get peak bin indexes from the PMF
    const auto tmp = find_peaks (pmf,
        params.min_peak_prominence,
        params.min_peak_distance);

    // Eliminate peaks that can't be surface
    vector<size_t> peak_v_bin_indexes;

    for (auto i : tmp)
    {
        assert (i < v_bin_elevations.size ());
        if (v_bin_elevations[i] < params.surface_z_min)
            continue;
        if (v_bin_elevations[i] > params.surface_z_max)
            continue;
        peak_v_bin_indexes.push_back (i);
    }

    // Return value
    vector<size_t> indexes;

    // If there are no peaks, there is nothing to do
    if (peak_v_bin_indexes.empty ())
        return indexes;

    // Set a sentinel
    size_t surface_bin_index = v_bins.size ();

    // If there is only one peak, it is the surface estimate
    if (peak_v_bin_indexes.size () == 1)
    {
        surface_bin_index = peak_v_bin_indexes[0];
    }
    else
    {
        // Get the two highest peaks
        //
        // Height is determined by the number of photons in the bin
        assert (peak_v_bin_indexes.size () >= 2);
        nth_element (peak_v_bin_indexes.begin (),
            peak_v_bin_indexes.begin () + 1,
            peak_v_bin_indexes.end (),
            [&](auto a, auto b) {
                assert (a < v_bins.size ());
                assert (b < v_bins.size ());
                return v_bins[a].size () > v_bins[b].size (); });

        // Get the sizes of the two bins
        assert (peak_v_bin_indexes[0] < v_bins.size ());
        assert (peak_v_bin_indexes[1] < v_bins.size ());
        const size_t size0 = v_bins[peak_v_bin_indexes[0]].size ();
        const size_t size1 = v_bins[peak_v_bin_indexes[1]].size ();

        // If they are close in height...
        if (std::min (size0, size1) > std::max (size0, size1) / 3)
        {
            // ... use the one at the highest elevation
            assert (peak_v_bin_indexes[0] < v_bin_elevations.size ());
            assert (peak_v_bin_indexes[1] < v_bin_elevations.size ());
            if (v_bin_elevations[peak_v_bin_indexes[0]] > v_bin_elevations[peak_v_bin_indexes[1]])
                surface_bin_index = peak_v_bin_indexes[0];
            else
                surface_bin_index = peak_v_bin_indexes[1];
        }
        else
        {
            // ... otherwise, use largest one
            if (size0 > size1)
                surface_bin_index = peak_v_bin_indexes[0];
            else
                surface_bin_index = peak_v_bin_indexes[1];
        }
    }

    // Get the elevation of the surface
    assert (surface_bin_index < v_bin_elevations.size ());
    const double surface_elevation = v_bin_elevations[surface_bin_index];

    // Get all photons within a certain range of the surface elevation
    const double max_distance = 1.0; // meters
    vector<double> surface_elevations;

    for (auto bin : v_bins) for (auto i : bin)
    {
        assert (i < p.size ());
        const double d = fabs (p[i].z - surface_elevation);
        if (d < max_distance)
            surface_elevations.push_back (p[i].z);
    }

    // Short circuit if needed
    if (surface_elevations.empty ())
        return indexes;

    // Get shape of photon distribution near the surface
    const double u = mean (surface_elevations);
    const double v = variance (surface_elevations);

    // Get the indexes of all photons in this bin within N standard
    // deviations of the surface estimate
    for (auto bin : v_bins) for (auto i : bin)
    {
        assert (i < p.size ());
        const double d = fabs (p[i].z - u);
        const double n_stddev = 3.0;
        if (d < sqrt (v) * n_stddev)
            indexes.push_back (i);
    }

    // Check to make sure we have enough
    if (indexes.size () < params.min_surface_photons_per_window)
        indexes.clear ();

    return indexes;
}

template<typename T,typename U,typename V,typename W>
std::vector<size_t> get_bathy_indexes (const T &p,
    U v_bins,
    const V &v_bin_elevations,
    const W &params,
    const double surface_lowest_elevation)
{
    using namespace oopp::utils;
    using namespace std;

    // Check invariants
    assert (v_bins.size () == v_bin_elevations.size ());

    // Remove indexes of photons below the surface
    U tmp_v_bins (v_bins.size ());

    size_t total_subsurface_photons = 0;

    for (size_t i = 0; i < v_bins.size (); ++i)
    {
        for (size_t j = 0; j < v_bins[i].size (); ++j)
        {
            const size_t index = v_bins[i][j];
            assert (index < p.size ());

            // If it's to high, skip it
            if (p[index].z >= surface_lowest_elevation)
                continue;

            // Save the photon index
            tmp_v_bins[i].push_back (index);

            // Count it
            ++total_subsurface_photons;
        }
    }

    // Save new one
    v_bins = tmp_v_bins;

    // Return value
    vector<size_t> indexes;

    // If there are none, there is nothing to do
    if (total_subsurface_photons == 0)
        return indexes;

    // Get a histogram from the bin indexes
    vector<size_t> h (v_bins.size ());

    transform (v_bins.begin (), v_bins.end (), h.begin (),
        [&](const auto &b) { return b.size (); });

    // Convert the histogram to a probability mass function
    auto pmf = convert_to_pmf<double> (h);

    // Smooth it
    pmf = gaussian_1D_filter (pmf, params.vertical_smoothing_sigma);

    // Get peak bin indexes from the PMF
    const auto peak_v_bin_indexes = find_peaks (pmf,
        params.min_peak_prominence,
        params.min_peak_distance);

    // We need at least one peak
    if (peak_v_bin_indexes.empty ())
        return indexes;

    // Use the highest peak
    //
    // Peak heights are determined by the number of photons in each bin
    const size_t bathy_bin_index = *max_element (peak_v_bin_indexes.begin (),
        peak_v_bin_indexes.end (),
        [&](auto a, auto b) {
            assert (a < v_bins.size ());
            assert (b < v_bins.size ());
            return v_bins[a].size () < v_bins[b].size (); });

    // Get the elevation of the bathy estimate
    assert (bathy_bin_index < v_bins.size ());
    assert (!v_bins[bathy_bin_index].empty ());
    assert (bathy_bin_index < v_bin_elevations.size ());
    const double bathy_elevation = v_bin_elevations[bathy_bin_index];

    // Get all photons within a certain range of the bathy elevation
    const double max_distance = 1.0; // meters
    vector<double> bathy_photons;

    for (auto bin : v_bins) for (auto i : bin)
    {
        assert (i < p.size ());
        const double d = fabs (p[i].z - bathy_elevation);
        if (d < max_distance)
            bathy_photons.push_back (p[i].z);
    }

    // Short circuit if needed
    if (bathy_photons.empty ())
        return indexes;

    // Get shape of photon distribution near the bathy estimate
    const double u = mean (bathy_photons);
    const double v = variance (bathy_photons);

    // Get the indexes of all photons in this bin within 2 standard deviations of the surface estimate
    for (auto bin : v_bins) for (auto i : bin)
    {
        assert (i < p.size ());
        const double d = fabs (p[i].z - u);
        const double n_stddev = 3.0;
        if (d < sqrt (v) * n_stddev)
            indexes.push_back (i);
    }

    // Check to make sure we have enough
    if (indexes.size () < params.min_bathy_photons_per_window)
        indexes.clear ();

    return indexes;
}

struct estimates
{
    double surface_elevation = 0.0;
    std::vector<size_t> surface_indexes;
    double bathy_elevation = 0.0;
    std::vector<size_t> bathy_indexes;
};

template<typename T,typename U>
double get_mean_elevation (const T &p, const U &indexes)
{
    if (indexes.empty ())
        return 0.0;

    double sum = 0.0;
    for (auto i : indexes)
    {
        assert (i < p.size ());
        sum += p[i].z;
    };

    return sum / indexes.size ();
}

template<typename T,typename U>
double get_lowest_elevation (const T &p, const U &indexes)
{
    if (indexes.empty ())
        return 0.0;

    double lowest = std::numeric_limits<float>::max ();
    for (auto i : indexes)
    {
        assert (i < p.size ());
        lowest = std::min (lowest, p[i].z);
    };

    return lowest;
}

template<typename T,typename U,typename V,typename W>
estimates get_estimates (const T &p,
    const U &v_bins,
    const V &v_bin_elevations,
    const W &params,
    const bool use_predictions)
{
    estimates e;

    // Get surface estimate from existing predictions, if specified
    if (use_predictions)
    {
        for (auto bin : v_bins) for (auto i : bin)
        {
            assert (i < p.size ());
            if (p[i].prediction == sea_surface_class)
                e.surface_indexes.push_back (i);
        }
    }
    else
    {
        e.surface_indexes = get_surface_indexes (p, v_bins, v_bin_elevations, params);
    }

    // If there is no surface, there is no bathy
    if (e.surface_indexes.empty ())
        return e;

    // Get the elevation
    e.surface_elevation = get_mean_elevation (p, e.surface_indexes);

    // Get bathy
    const double surface_lowest_elevation = get_lowest_elevation (p, e.surface_indexes);
    e.bathy_indexes = get_bathy_indexes (p, v_bins, v_bin_elevations, params, surface_lowest_elevation);
    e.bathy_elevation = get_mean_elevation (p, e.bathy_indexes);

    return e;
}

template<typename T>
T smooth_surface (T e, const double sigma)
{
    using namespace std;
    using namespace oopp::utils;

    // Extract surface
    vector<double> x (e.size ());

#pragma omp parallel for
    for (size_t i = 0; i < x.size (); ++i)
    {
        assert (!std::isnan (e[i].surface_elevation));
        x[i] = e[i].surface_elevation;
    }

    // Smooth it
    x = gaussian_1D_filter (x, sigma);

    // Put the smoothed values back into the estimates
#pragma omp parallel for
    for (size_t i = 0; i < x.size (); ++i)
    {
        assert (!std::isnan (x[i]));
        e[i].surface_elevation = x[i];
    }

    return e;
}

template<typename T>
T smooth_bathy (T e, const double sigma)
{
    using namespace std;
    using namespace oopp::utils;

    // Extract bathy
    vector<double> x (e.size ());

#pragma omp parallel for
    for (size_t i = 0; i < x.size (); ++i)
    {
        assert (!std::isnan (e[i].bathy_elevation));
        x[i] = e[i].bathy_elevation;
    }

    // Smooth it
    x = gaussian_1D_filter (x, sigma);

    // Put the smoothed values back into the estimates
#pragma omp parallel for
    for (size_t i = 0; i < x.size (); ++i)
    {
        assert (!std::isnan (x[i]));
        e[i].bathy_elevation = x[i];
    }

    return e;
}

template<typename T,typename U>
T classify (T p, const U &params, const bool use_predictions)
{
    using namespace std;
    using namespace oopp::utils;

    // Get indexes of photons in each along-track bin
    const auto h_bins = get_h_bins (p, params);

    // Get each vertical bin's elevation
    const auto v_bin_elevations = get_v_bin_elevations (params);

    // Save estimates for each horizontal window
    vector<estimates> e (h_bins.size ());

    // Get estimates for each horizontal window
#pragma omp parallel for
    for (size_t i = 0; i < h_bins.size (); ++i)
    {
        // If there are no photons in the h_bin, there is nothing to do
        if (h_bins[i].empty ())
            continue;

        // Construct vertical distribution at each horizontal bin
        const auto v_bins = get_v_bins (p, h_bins[i], params);

        // Get surface and bathy estimates
        e[i] = get_estimates (p, v_bins, v_bin_elevations, params, use_predictions);
    }

    // Smooth the surface and bathy elevation estimates
    e = smooth_surface (e, params.surface_smoothing_sigma / params.x_resolution);
    e = smooth_bathy (e, params.bathy_smoothing_sigma / params.x_resolution);

    // Assign estimates
#pragma omp parallel for
    for (size_t i = 0; i < h_bins.size (); ++i)
    {
        // If there are no photons in the h_bin, there is nothing to do
        if (h_bins[i].empty ())
            continue;

        // Save surface predictions
        for (auto j : e[i].surface_indexes)
        {
            assert (j < p.size ());
            p[j].prediction = sea_surface_class;
        }

        // Save bathy predictions
        for (auto j : e[i].bathy_indexes)
        {
            assert (j < p.size ());
            p[j].prediction = bathy_class;
        }

        // Save surface and bathy elevations for all photons in this
        // horizontal window
        for (auto j : h_bins[i])
        {
            assert (j < p.size ());
            p[j].surface_elevation = e[i].surface_elevation;
            p[j].bathy_elevation = e[i].bathy_elevation;
        }
    }

    return p;
}

} // namespace oopp
