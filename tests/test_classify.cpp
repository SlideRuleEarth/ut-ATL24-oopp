#include "oopp/precompiled.h"
#include "oopp/dataframe.h"
#include "oopp/verify.h"

using namespace std;
using namespace oopp;

void test_classify ()
{
    // Random points
    mt19937 rng(12345);
    const size_t total = 1000;
    const double xmin = 100;
    const double xmax = 200;
    const double zmin = -60.0;
    const double zmax  = 20.0;

    uniform_real_distribution<double> dx (xmin, xmax);
    uniform_real_distribution<double> dz (zmin, zmax);

    vector<photon> p (total);
    size_t index = 0;

    for (auto &i : p)
    {
        i.h5_index = index++;
        i.x = dx (rng);
        i.z = dz (rng);
    }

    // Run it once as a reference
    oopp::params oo_params;
    const auto q = classify (p, oo_params);
    const size_t iterations = 10;

    // It should give the same answer every time
    for (size_t i = 0; i < iterations; ++i)
    {
        const auto tmp = classify (p, oo_params);
        VERIFY (tmp == q);
    }
}

int main ()
{
    try
    {
        test_classify ();

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
