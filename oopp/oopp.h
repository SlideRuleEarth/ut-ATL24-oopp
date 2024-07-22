#pragma once

#include "oopp/precompiled.h"

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

const std::string PI_NAME = std::string ("index_ph");
const std::string X_NAME = std::string ("x_atc");
const std::string Z_NAME = std::string ("geoid_corr_h");
const std::string LABEL_NAME = std::string ("manual_label");
const std::string PREDICTION_NAME = std::string ("prediction");
const std::string SEA_SURFACE_NAME = std::string ("sea_surface_h");
const std::string BATHY_NAME = std::string ("bathy_h");

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

template<typename T>
std::vector<oopp::photon> convert_dataframe (
    const T &df,
    bool &has_manual_label,
    bool &has_predictions,
    bool &has_surface_elevations,
    bool &has_bathy_elevations)
{
    using namespace std;

    // Check invariants
    assert (df.is_valid ());
    assert (df.rows () != 0);
    assert (df.cols () != 0);

    // Get number of photons in this file
    const size_t nrows = df.rows ();

    // Get the columns we are interested in
    const auto headers = df.get_headers ();
    auto pi_it = find (headers.begin(), headers.end(), PI_NAME);
    auto x_it = find (headers.begin(), headers.end(), X_NAME);
    auto z_it = find (headers.begin(), headers.end(), Z_NAME);
    auto cls_it = find (headers.begin(), headers.end(), LABEL_NAME);
    auto prediction_it = find (headers.begin(), headers.end(), PREDICTION_NAME);
    auto surface_elevation_it = find (headers.begin(), headers.end(), SEA_SURFACE_NAME);
    auto bathy_elevation_it = find (headers.begin(), headers.end(), BATHY_NAME);

    if (pi_it == headers.end ())
        throw runtime_error ("Can't find 'ph_index' in dataframe");
    if (x_it == headers.end ())
        throw runtime_error ("Can't find 'along_track_dist' in dataframe");
    if (z_it == headers.end ())
        throw runtime_error ("Can't find 'geoid_corrected_h' in dataframe");

    has_manual_label = cls_it != headers.end ();
    has_predictions = prediction_it != headers.end ();
    has_surface_elevations = surface_elevation_it != headers.end ();
    has_bathy_elevations = bathy_elevation_it != headers.end ();

    // Stuff values into the vector
    std::vector<oopp::photon> dataset (nrows);

    for (size_t i = 0; i < nrows; ++i)
    {
        // Make assignments
        dataset[i].h5_index = df.get_value (PI_NAME, i);
        dataset[i].x = df.get_value (X_NAME, i);
        dataset[i].z = df.get_value (Z_NAME, i);
        if (has_manual_label)
            dataset[i].cls = df.get_value (LABEL_NAME, i);
        if (has_predictions)
            dataset[i].prediction = df.get_value (PREDICTION_NAME, i);
        if (has_surface_elevations)
            dataset[i].surface_elevation = df.get_value (SEA_SURFACE_NAME, i);
        if (has_bathy_elevations)
            dataset[i].bathy_elevation = df.get_value (BATHY_NAME, i);
    }

    return dataset;
}

template<typename T>
std::vector<oopp::photon> convert_dataframe (const T &df)
{
    bool has_manual_label;
    bool has_predictions;
    bool has_surface_elevations;
    bool has_bathy_elevations;

    return convert_dataframe (df,
        has_manual_label,
        has_predictions,
        has_surface_elevations,
        has_bathy_elevations);
}

template<typename T>
std::vector<oopp::photon> convert_dataframe (const T &df,
    bool &has_manual_label,
    bool &has_predictions)
{
    bool has_surface_elevations;
    bool has_bathy_elevations;

    return convert_dataframe (df,
        has_manual_label,
        has_predictions,
        has_surface_elevations,
        has_bathy_elevations);
}

class timer
{
private:
    std::chrono::time_point<std::chrono::system_clock> t1;
    std::chrono::time_point<std::chrono::system_clock> t2;
    bool running;

public:
    timer () : running (false)
    {
        start ();
    }
    void start ()
    {
        t1 = std::chrono::system_clock::now ();
        running = true;
    }
    void stop ()
    {
        t2 = std::chrono::system_clock::now ();
        running = false;
    }
    double elapsed_ns()
    {
        using namespace std::chrono;
        return running
            ? duration_cast<nanoseconds> (system_clock::now () - t1).count ()
            : duration_cast<nanoseconds> (t2 - t1).count ();
    }
};

} // namespace oopp
