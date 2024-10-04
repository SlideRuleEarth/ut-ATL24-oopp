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

const double icesat_2_sampling_rate = 0.7;

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
    double surface_smoothing_sigma = 200.0; // meters
    double bathy_smoothing_sigma = 100.0; // meters
    double min_peak_prominence = 0.01; // probability
    size_t min_peak_distance = 2; // z bins
    size_t min_surface_photons_per_window = 0.25 * (x_resolution / icesat_2_sampling_rate); // photons
    size_t min_bathy_photons_per_window = 0.25 * (x_resolution / icesat_2_sampling_rate); // photons
    double surface_n_stddev = 3.5;
    double bathy_n_stddev = 3.0;
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
    os << "min-surface-photons-per-window: " << params.min_surface_photons_per_window << " photons" << std::endl;
    os << "min-bathy-photons-per-window: " << params.min_bathy_photons_per_window << " photons" << std::endl;
    os << "surface-n-stddev: " << params.surface_n_stddev << "m" << std::endl;
    os << "bathy-n-stddev: " << params.bathy_n_stddev << "m" << std::endl;

    return os;
}

template<typename T>
void write_predictions (std::ostream &os, const T &p)
{
    using namespace std;

    // Save precision
    const auto pr = os.precision ();

    // Print along-track meters
    os << "index_ph,x_atc,ortho_h,manual_label,prediction,sea_surface_h,bathy_h" << endl;
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

struct surface_estimate
{
    double mean;
    double variance;
};

template<typename T,typename U>
surface_estimate get_surface_estimate (const T &p, const U &params, const bool use_predictions)
{
    using namespace std;
    using namespace oopp::utils;

    // Potential surface photon elevations
    vector<double> z;
    z.reserve (p.size ());

    // Get photon elevations
    if (use_predictions)
    {
        // Only use those marked as sea surface
        for (auto i : p)
            if (i.prediction == sea_surface_class)
                z.push_back (i.z);
    }
    else
    {
        // Only use those in range
        for (auto i : p)
            if (i.z > params.surface_z_min && i.z < params.surface_z_max)
                z.push_back (i.z);
    }

    // Free memory
    z.shrink_to_fit ();

    if (use_predictions)
    {
        // Trust the current predictions
        surface_estimate e;
        e.mean = mean (z);
        e.variance = variance (z);

        return e;
    }

    // Iterate
    const auto m = median (z);

    // Select only the photons near the median
    z.clear ();
    for (auto i : p)
    {
        const double max_distance = 1.0; // meters
        if (fabs (i.z - m) < max_distance)
            z.push_back (i.z);
    }

    surface_estimate e;
    e.mean = mean (z);
    e.variance = variance (z);

    return e;
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

template<typename T,typename U,typename V,typename W,typename X>
std::vector<size_t> get_surface_indexes (const T &p,
    const U &se,
    const V &v_bins,
    const W &v_bin_elevations,
    const X &params)
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

    // Determine range from surface estimate
    //
    // The range is += N standard deviations from the mean
    const double surface_z_min = se.mean - params.surface_n_stddev * sqrt (se.variance);
    const double surface_z_max = se.mean + params.surface_n_stddev * sqrt (se.variance);
    for (auto i : tmp)
    {
        assert (i < v_bin_elevations.size ());
        if (v_bin_elevations[i] < surface_z_min)
            continue;
        if (v_bin_elevations[i] > surface_z_max)
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

    // Get the local elevation of the surface
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
        if (d < sqrt (v) * params.surface_n_stddev)
            indexes.push_back (i);
    }

    // Check to make sure we have enough
    if (indexes.size () < params.min_surface_photons_per_window)
        indexes.clear ();

    return indexes;
}

template<typename T,typename U,typename V,typename W,typename X>
std::vector<size_t> get_bathy_indexes (const T &p,
    const U &se,
    V v_bins,
    const W &v_bin_elevations,
    const X &params)
{
    using namespace oopp::utils;
    using namespace std;

    // Check invariants
    assert (v_bins.size () == v_bin_elevations.size ());

    // Remove indexes of photons below the surface
    V tmp_v_bins (v_bins.size ());

    size_t total_subsurface_photons = 0;

    for (size_t i = 0; i < v_bins.size (); ++i)
    {
        for (size_t j = 0; j < v_bins[i].size (); ++j)
        {
            const size_t index = v_bins[i][j];
            assert (index < p.size ());

            // If it's too close to the surface, skip it
            const double z_min = se.mean - params.bathy_n_stddev * sqrt (se.variance);
            if (p[index].z >= z_min)
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

    // Get the indexes of all photons in this bin within some standard
    // deviations of the bathy estimate
    for (auto bin : v_bins) for (auto i : bin)
    {
        assert (i < p.size ());
        const double d = fabs (p[i].z - u);
        if (d < sqrt (v) * params.bathy_n_stddev)
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

template<typename T,typename U,typename V,typename W,typename X>
estimates get_estimates (const T &p,
    const U &se,
    const V &v_bins,
    const W &v_bin_elevations,
    const X &params,
    const bool use_predictions)
{
    estimates e;

    if (use_predictions)
    {
        // Get surface estimate from existing predictions
        for (auto bin : v_bins) for (auto i : bin)
        {
            assert (i < p.size ());
            if (p[i].prediction == sea_surface_class)
                e.surface_indexes.push_back (i);
        }
    }
    else
    {
        // Get surface indexes from the vertical histogram
        e.surface_indexes = get_surface_indexes (p, se, v_bins, v_bin_elevations, params);
    }

    // If there is no surface, there is no bathy
    if (e.surface_indexes.empty ())
        return e;

    // Compute surface elevation for this window
    e.surface_elevation = get_mean_elevation (p, e.surface_indexes);

    // Get bathy
    e.bathy_indexes = get_bathy_indexes (p, se, v_bins, v_bin_elevations, params);
    e.bathy_elevation = get_mean_elevation (p, e.bathy_indexes);

    return e;
}

template<typename T,typename U,typename V,typename W>
std::vector<double> get_smooth_estimates (const T &p, const U &h_bins, const V &e, const double sigma, W op)
{
    using namespace std;
    using namespace oopp::utils;

    // Check invariants
    assert (!p.empty ());
    assert (h_bins.size () == e.size ());

    // Get the bounds
    const double x_min = min_element (p.begin (), p.end (),
            [](const auto &a, const auto &b) { return a.x < b.x; })->x;
    const double x_max = max_element (p.begin (), p.end (),
            [](const auto &a, const auto &b) { return a.x < b.x; })->x;

    // Get estimates at 'resolution' m intervals
    const double resolution = 5.0; // meters
    const size_t total = (x_max - x_min) / resolution + 1;
    vector<double> z (total, NAN);

#pragma omp parallel for
    for (size_t i = 0; i < h_bins.size (); ++i)
    {
        for (auto j : h_bins[i])
        {
            assert (j < p.size ());
            assert (p[j].x >= x_min);
            const size_t index = (p[j].x - x_min) / resolution;

            // The photon gets the elevation associated with the window
            assert (!std::isnan (op (e[i])));
            assert (index < z.size ());
            z[index] = op (e[i]);
        }
    }

    // Fill in NANs from left to right
    double last = 0.0;
    auto zl (z);
    for (size_t i = 0; i < z.size (); ++i)
    {
        if (isnan (zl[i]))
            zl[i] = last;
        else
        {
            last = zl[i];
        }

    }

    // Fill in NANs from right to left
    last = 0.0;
    auto zr (z);
    for (size_t i = z.size (); i != 0; --i)
    {
        assert (i - 1 < zr.size ());
        if (isnan (zr[i - 1]))
            zr[i - 1] = last;
        else
        {
            last = zr[i - 1];
        }
    }

    // Average them together
    for (size_t i = 0; i < z.size (); ++i)
    {
        z[i] = (zl[i] + zr[i]) / 2.0;
        assert (!isnan (z[i]));
    }

    // Smooth it
    z = gaussian_1D_filter (z, sigma / resolution);

    // Associate smooth estimates with photons
    vector<double> s (p.size (), 0.0);

#pragma omp parallel for
    for (size_t i = 0; i < p.size (); ++i)
    {
        assert (p[i].x >= x_min);
        const size_t index = (p[i].x - x_min) / resolution;
        assert (index < z.size ());
        s[i] = z[index];
    }

    return s;
}

template<typename T,typename U>
T classify (T p, const U &params, const bool use_predictions)
{
    using namespace std;
    using namespace oopp::utils;

    // Assume that there is a single sea surface in the track. If
    // there is a case that a land mass separates two water bodies,
    // and the surface of the water bodies is significantly different,
    // then this will likely cause this function to fail. However, for
    // the on-demand product, when this occurs, you should change your
    // AOIs so that the two water bodies are separated.
    const auto se = get_surface_estimate (p, params, use_predictions);

    // Get indexes of photons in each along-track bin
    const auto h_bins = get_h_bins (p, params);

    // Get each vertical bin's elevation
    const auto v_bin_elevations = get_v_bin_elevations (params);

    // Save estimates for each horizontal window here
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
        e[i] = get_estimates (p, se, v_bins, v_bin_elevations, params, use_predictions);
    }

    // Smooth the surface and bathy elevation estimates
    const auto ss = get_smooth_estimates (p, h_bins, e, params.surface_smoothing_sigma,
        [](const estimates &a) { return a.surface_elevation; });
    const auto sb = get_smooth_estimates (p, h_bins, e, params.bathy_smoothing_sigma,
        [](const estimates &a) { return a.bathy_elevation; });

    assert (ss.size () == p.size ());
    assert (sb.size () == p.size ());

    // Assign surface and bathy elevations
#pragma omp parallel for
    for (size_t i = 0; i < h_bins.size (); ++i)
    {
        for (auto j : h_bins[i])
        {
            assert (j < p.size ());
            p[j].surface_elevation = ss[j];
            p[j].bathy_elevation = sb[j];
        }
    }

    // Assign estimates
#pragma omp parallel for
    for (size_t i = 0; i < h_bins.size (); ++i)
    {
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
    }

    return p;
}

} // namespace oopp
