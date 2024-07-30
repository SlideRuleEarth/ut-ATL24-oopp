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
    double vertical_smoothing_sigma = 0.5; // meters
    double min_peak_prominence = 0.01; // probability
    size_t min_peak_distance = 0; // bins
};

std::ostream &operator<< (std::ostream &os, const params &params)
{
    os << "x-resolution: " << params.x_resolution << "m" << std::endl;
    os << "z-resolution: " << params.z_resolution << "m" << std::endl;
    os << "z-min: " << params.z_min << "m" << std::endl;
    os << "z-max: " << params.z_max << "m" << std::endl;
    os << "vertical-smoothing-sigma: " << "m" << params.vertical_smoothing_sigma << std::endl;
    os << "min-peak-prominence: " << params.min_peak_prominence << std::endl;
    os << "min-peak-distance: " << params.min_peak_distance << " bins" << std::endl;

    return os;
}

template<typename T,typename U>
void write_predictions (std::ostream &os, const T &p, const U &q)
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
        os << "," << q[i];
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

template<typename T,typename U,typename V,typename W>
std::vector<size_t> get_surface_indexes (const T &p,
    const U &v_bins,
    const V &peaks,
    const W &params)
{
    using namespace oopp::utils;
    using namespace std;

    // Return value
    vector<size_t> indexes;

    if (peaks.empty ())
        return indexes;

    // What is the elevation of the highest elevation peak?
    //
    // Peaks are stored from lowest to highest elevation, so the last
    // peak is the one we want.
    //
    // The indexes in the peaks vector are indexes into the vertical bins
    assert (peaks.back () < v_bins.size ());
    const auto photon_indexes = v_bins[peaks.back ()];

    // Check our logic
    assert (!photon_indexes.empty ());
    const double surface_elevation = p[photon_indexes[0]].z;

    // Get all photons within a certain range of the surface elevation
    const double max_distance = 2.0; // meters
    vector<double> surface_photons;
    surface_photons.reserve (v_bins.size ());

    for (auto i : v_bins)
    {
        for (auto j : i)
        {
            assert (j < p.size ());
            const double d = fabs (p[j].z - surface_elevation);
            if (d < max_distance)
                surface_photons.push_back (p[j].z);
        }
    }

    // Short circuit if needed
    if (surface_photons.empty ())
        return indexes;

    // Get shape of photon distribution near the surface
    const double u = mean (surface_photons);
    const double v = variance (surface_photons);

    // Get the indexes of all photons in this bin within 2 standard deviations of the surface estimate
    for (auto i : v_bins)
    for (auto j : i)
    {
        assert (j < p.size ());
        const double d = fabs (p[j].z - u);
        if (d < sqrt (v) * 2.0)
            indexes.push_back (j);
    }

    return indexes;
}

template<typename T,typename U,typename V,typename W>
std::vector<size_t> get_bathy_indexes (const T &p,
    const U &v_bins,
    const V &peaks,
    const W &params)
{
    using namespace oopp::utils;
    using namespace std;

    // Return value
    vector<size_t> indexes;

    // We need at least two peaks
    if (peaks.size () < 2)
        return indexes;

    // What is the elevation of the lowest elevation peak?
    //
    // Peaks are stored from lowest to highest elevation, so the first
    // peak is the one we want.
    //
    // The indexes in the peaks vector are indexes into the vertical bins
    assert (peaks.front () < v_bins.size ());
    const auto photon_indexes = v_bins[peaks.front ()];

    // Check our logic
    assert (!photon_indexes.empty ());
    const double bathy_elevation = p[photon_indexes[0]].z;

    // Get all photons within a certain range of the surface elevation
    const double max_distance = 2.0; // meters
    vector<double> bathy_photons;
    bathy_photons.reserve (v_bins.size ());

    for (auto i : v_bins)
    {
        for (auto j : i)
        {
            assert (j < p.size ());
            const double d = fabs (p[j].z - bathy_elevation);
            if (d < max_distance)
                bathy_photons.push_back (p[j].z);
        }
    }

    // Short circuit if needed
    if (bathy_photons.empty ())
        return indexes;

    // Get shape of photon distribution near the surface
    const double u = mean (bathy_photons);
    const double v = variance (bathy_photons);

    // Get the indexes of all photons in this bin within 2 standard deviations of the surface estimate
    for (auto i : v_bins)
    for (auto j : i)
    {
        assert (j < p.size ());
        const double d = fabs (p[j].z - u);
        if (d < sqrt (v) * 2.0)
            indexes.push_back (j);
    }

    return indexes;
}

template<typename T,typename U>
std::vector<unsigned> classify (const T &p, const U &params, const bool use_predictions)
{
    using namespace std;
    using namespace oopp::utils;

    // Get indexes of photons in each along-track bin
    const auto h_bins = get_h_bins (p, params);

    // Set default prediction to '0'
    vector<unsigned> q (p.size (), 0);

    // Assign predictions
#pragma omp parallel for
    for (size_t i = 0; i < h_bins.size (); ++i)
    {
        // If there are no photons in the h_bin, there is nothing to do
        if (h_bins[i].empty ())
            continue;

        // Construct distribution at each bin
        //
        // std::move() the bins, so 'h_bins[i]' will be valid but
        // unspecified after the call
        const auto v_bins = get_v_bins (p, move (h_bins[i]), params);

        // Get a histogram from the bin indexes
        vector<size_t> h (v_bins.size ());

        transform (v_bins.begin (), v_bins.end (), h.begin (),
            [&](const auto &b) { return b.size (); });

        // Convert the histogram to a probability mass function
        auto pmf = convert_to_pmf<double> (h);

        // Smooth it
        pmf = gaussian_1D_filter (pmf, params.vertical_smoothing_sigma);

        // Get peaks from the PMF
        const auto peaks = find_peaks (pmf,
            params.min_peak_prominence,
            params.min_peak_distance);

        // Assign surface
        const auto s = get_surface_indexes (p, v_bins, peaks, params);
        for (auto j : s)
            q[j] = sea_surface_class;

        // Assign bathy
        const auto b = get_bathy_indexes (p, v_bins, peaks, params);
        for (auto j : b)
            q[j] = bathy_class;

        // Overwrite previous predictions if specified
        if (use_predictions)
        {
            for (auto j : v_bins)
            {
                for (auto k : j)
                {
                    if (p[k].prediction != 0)
                        q[k] = p[k].prediction;
                }
            }
        }
    }

    return q;
}

} // namespace oopp
