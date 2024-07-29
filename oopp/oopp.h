#pragma once

#include "oopp/precompiled.h"
#include "oopp/utils.h"

namespace oopp
{

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
    double window_overlap = 2.0; // meters
    double vertical_smoothing_sigma = 0.5; // meters
};

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
std::vector<std::vector<size_t>> get_v_bins (const T &p, U indexes, const V &params)
{
    using namespace std;

    assert (params.z_max > params.z_min);
    const size_t total_bins = (params.z_max - params.z_min) / params.z_resolution + 1;
    vector<vector<size_t>> bins (total_bins);
    for (const auto i : indexes)
    {
        assert (i < p.size ());
        assert (p[i].z >= params.z_min);
        size_t bin = (p[i].z - params.z_min) / params.z_resolution;
        assert (bin < bins.size ());
        bins[bin].push_back (i);
    }

    return bins;
}

template<typename T,typename U>
std::vector<size_t> get_surface_indexes (const T &p, const U &v_bins)
{
    std::vector<size_t> indexes;
    return indexes;
}

template<typename T,typename U>
std::vector<size_t> get_bathy_indexes (const T &p, const U &v_bins)
{
    std::vector<size_t> indexes;
    return indexes;
}

template<typename T,typename U>
std::vector<unsigned> classify (const T &p, const U &params, const bool use_predictions)
{
    using namespace std;
    using namespace oopp::utils;

    // Get indexes of photons in each along-track bin
    const auto h_bins = get_h_bins (p, params);

    // Set default prediction to '1' = unknown
    vector<unsigned> q (p.size (), 1);

    // Assign predictions
#pragma omp parallel for
    for (size_t i = 0; i < h_bins.size (); ++i)
    {
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
        auto pmf = convert_to_pmf<float> (h);

        // Smooth it
        pmf = gaussian_1D_filter (pmf, params.vertical_smoothing_sigma);

        // Assign surface
        const auto s = get_surface_indexes (p, pmf);
        for (auto j : s)
        {
            if (!use_predictions)
                q[j] = p[j].cls;
            else if (p[j].prediction != 0)
                q[j] = p[j].prediction;
        }

        // Assign bathy
        const auto b = get_bathy_indexes (p, v_bins);
        for (auto j : b)
        {
            if (!use_predictions)
                q[j] = p[j].cls;
            else if (p[j].prediction != 0)
                q[j] = p[j].prediction;
        }
    }

    return q;
}

} // namespace oopp
