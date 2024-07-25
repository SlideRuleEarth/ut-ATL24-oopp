#pragma once

#include "oopp/precompiled.h"
#include "oopp/oopp.h"

namespace oopp
{

namespace dataframe
{

const std::string PI_NAME = std::string ("index_ph");
const std::string X_NAME = std::string ("x_atc");
const std::string Z_NAME = std::string ("geoid_corr_h");
const std::string LABEL_NAME = std::string ("manual_label");
const std::string PREDICTION_NAME = std::string ("prediction");
const std::string SEA_SURFACE_NAME = std::string ("sea_surface_h");
const std::string BATHY_NAME = std::string ("bathy_h");

class dataframe
{
    private:
    std::vector<std::string> headers;
    std::unordered_map<std::string,size_t> header_column;
    std::vector<std::vector<double>> columns;

    public:
    bool is_valid () const
    {
        // Number of headers match number of columns
        if (headers.size () != columns.size ())
            return false;
        // Header column map matches number of columns
        if (headers.size () != header_column.size ())
            return false;
        // Number of rows are the same in each column
        for (size_t i = 1; i < columns.size (); ++i)
            if (columns[i].size () != columns[0].size ())
                return false;
        return true;
    }
    const std::vector<std::string> get_headers () const
    {
        return headers;
    }
    size_t cols () const
    {
        assert (is_valid ());
        return columns.size ();
    }
    size_t rows () const
    {
        assert (is_valid ());
        return columns.empty () ? 0 : columns[0].size ();
    }
    void add_column (const std::string &name, const std::vector<double> &new_column)
    {
        // Does this column already exist?
        if (header_column.find (name) != header_column.end ())
            throw std::runtime_error ("Column already exists");
        assert (is_valid ());
        // Add the column
        headers.push_back (name);
        columns.push_back (new_column);
        // Update header column map
        header_column[name] = headers.size () - 1;
        assert (is_valid ());
    }
    void add_column (const std::string &name)
    {
        const std::vector<double> zeroes (rows ());
        add_column (name, zeroes);
    }
    void set_rows (const size_t n)
    {
        assert (is_valid ());
        for (size_t i = 0; i < columns.size (); ++i)
            columns[i].resize (n);
        assert (is_valid ());
    }
    double get_value (const size_t col, const size_t row) const
    {
        assert (col < columns.size ());
        assert (row < columns[col].size ());
        return columns[col][row];
    }
    double get_value (const std::string &name, const size_t row) const
    {
        // Make sure column name exists
        assert (header_column.find (name) != header_column.end ());
        const size_t col = header_column.at (name);
        return get_value (col, row);
    }
    void set_value (const std::string &name, const size_t row, const double x)
    {
        // Make sure column name exists
        assert (header_column.find (name) != header_column.end ());
        const size_t col = header_column.at (name);
        assert (col < columns.size ());
        assert (row < columns[col].size ());
        columns[col][row] = x;
    }
    void set_values (std::vector<std::vector<double>> values)
    {
        assert (values.size () == headers.size ());
        assert (values.size () == columns.size ());
        for (size_t i = 0; i < columns.size (); ++i)
            columns[i] = values[i];
        assert (is_valid ());
    }
    friend bool operator ==(const dataframe &a, const dataframe &b)
    {
        if (a.headers != b.headers)
            return false;
        if (a.header_column != b.header_column)
            return false;
        if (a.columns != b.columns)
            return false;
        return true;
    }
};

dataframe read (std::istream &is)
{
    using namespace std;

    // Create the dataframe
    dataframe df;

    // Read the headers
    string line;

    if (!getline (is, line))
        return df;

    // Parse each individual column header
    stringstream ss (line);
    string header;
    while (getline (ss, header, ','))
    {
        // Remove LFs in case the file was created under Windows
        std::erase (header, '\r');

        // Create it
        df.add_column (header);
    }

    // Read the values
    std::vector<std::vector<double>> values (df.cols ());

    // Now get the rows
    while (getline (is, line))
    {
        // Skip empty lines
        if (line.empty ())
            continue;
        char *p = &line[0];
        for (size_t j = 0; j < df.cols (); ++j)
        {
            char *end;
            const double x = strtod (p, &end);
            values[j].push_back (x);
            p = end;
            // Ignore ','
            if (*p == ',')
                ++p;
        }
    }

    // Move the data to the dataframe
    df.set_values (std::move (values));
    assert (values.empty ());
    assert (df.is_valid ());

    return df;
}

dataframe read (const std::string &fn)
{
    using namespace std;

    ifstream ifs (fn);
    if (!ifs)
        throw runtime_error ("Could not open file for reading");

    return oopp::dataframe::read (ifs);
}

std::ostream &write (std::ostream &os, const dataframe &df, const size_t precision = 16)
{
    using namespace std;

    assert (df.is_valid ());

    const size_t ncols = df.cols ();

    // Short-circuit
    if (ncols == 0)
        return os;

    // Print headers
    bool first = true;
    for (auto h : df.get_headers ())
    {
        if (!first)
            os << ",";
        first = false;
        os << h;
    }
    os << endl;

    const size_t nrows = df.rows ();

    // Short-circuit
    if (nrows == 0)
        return os;

    // Save format
    const auto f = os.flags();
    const auto p = os.precision();

    // Set format
    os << std::fixed;
    os << std::setprecision (precision);

    // Write it out
    for (size_t i = 0; i < nrows; ++i)
    {
        for (size_t j = 0; j < ncols; ++j)
        {
            if (j != 0)
                os << ",";
            os << df.get_value (j, i);
        }
        os << endl;
    }

    // Restore format
    os.precision (p);
    os.flags (f);

    return os;
}

std::ostream &write (const std::string &filename, const dataframe &df, const size_t precision = 16)
{
    using namespace std;

    ofstream ofs (filename);
    if (!ofs)
        throw runtime_error ("Can't open file for writing");

    return write (ofs, df, precision);
}

std::ostream &operator<< (std::ostream &os, const dataframe &df)
{
    return write (os , df);
}

std::vector<photon> convert_dataframe (
    const dataframe &df,
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
    std::vector<photon> dataset (nrows);

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

std::vector<photon> convert_dataframe (const dataframe &df)
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

std::vector<photon> convert_dataframe (const dataframe &df,
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

} // namespace dataframe

} // namespace oopp
