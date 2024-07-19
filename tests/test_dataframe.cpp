#include "oopp/precompiled.h"
#include "oopp/dataframe.h"
#include "oopp/verify.h"

using namespace std;
using namespace oopp::dataframe;

mt19937 rng(12345);

dataframe get_random_dataframe (const size_t cols, const size_t rows)
{
    // Create the dataframe column names
    vector<string> names (cols);
    generate (names.begin(), names.end(), [&](){return to_string(rng ());});

    // Create a dataframe
    dataframe df;
    for (const auto &i : names)
        df.add_column(i);
    df.set_rows (rows);

    // Set the data to random numbers
    std::uniform_real_distribution<> d(1.0, 100.0);
    for (const auto &name : df.get_headers ())
        for (size_t i = 0; i < rows; ++i)
            df.set_value (name, i, d(rng));

    return df;
}

void test_dataframe (const size_t cols, const size_t rows)
{
    const auto df = get_random_dataframe (cols, rows);

    // Write it
    stringstream ss;
    write (ss, df);

    // Read it
    const auto tmp = read (ss);

    // Compare
    VERIFY (df == tmp);
}

int main ()
{
    try
    {
        test_dataframe (1, 1);
        test_dataframe (17, 1);
        test_dataframe (1, 23);
        test_dataframe (19, 111);
        test_dataframe (32, 20'000);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
