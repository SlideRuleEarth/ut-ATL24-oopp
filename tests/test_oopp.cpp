#include "oopp/precompiled.h"
#include "oopp/oopp.h"
#include "oopp/verify.h"

using namespace std;
using namespace oopp;

void test_get_h_bins ()
{
    vector<photon> p {
        { .x = 0.0 },
        { .x = 0.1 },
        { .x = 1.0 },
        { .x = 2.0 },
        { .x = 3.0 },
        { .x = 4.0 },
    };
    params a { .x_resolution = 1.9 };
    auto indexes = get_h_bins (p, a);

    VERIFY (indexes.size () == 3);
    VERIFY (indexes[0].size () == 3);
    VERIFY (indexes[1].size () == 2);
    VERIFY (indexes[2].size () == 1);

    a.x_resolution = 5;
    indexes = get_h_bins (p, a);
    VERIFY (indexes.size () == 1);
    VERIFY (indexes[0].size () == 6);

    a.x_resolution = 0.9;
    indexes = get_h_bins (p, a);
    VERIFY (indexes.size () == 5);
    VERIFY (indexes[0].size () == 2);
    VERIFY (indexes[1].size () == 1);
    VERIFY (indexes[2].size () == 1);
    VERIFY (indexes[3].size () == 1);
    VERIFY (indexes[4].size () == 1);
}

void test_get_v_bins ()
{
    vector<photon> p {
        { .x = 0.0, .z = -0.9 },
        { .x = 0.1, .z = 0.1 },
        { .x = 0.2, .z = 1.1 },
        { .x = 0.3, .z = 2.1 },
        { .x = 0.4, .z = 3.1 },
    };
    params a {
        .x_resolution = 1.0,
        .z_resolution = 1.0,
        .z_min = -1.0,
        .z_max = 4,
    };
    auto h = get_h_bins (p, a);
    VERIFY (h.size () == 1);
    VERIFY (h[0].size () == 5);
    auto v = get_v_bins (p, h[0], a);
    // bin 0   -> Lowest elevation
    // ...
    // bin N-1 -> Highest elevation
    VERIFY (v.size () == 6);
    VERIFY (v[0].size () == 1);
    VERIFY (v[0][0] == 0);
    VERIFY (v[1].size () == 1);
    VERIFY (v[1][0] == 1);
    VERIFY (v[2].size () == 1);
    VERIFY (v[2][0] == 2);
    VERIFY (v[3].size () == 1);
    VERIFY (v[3][0] == 3);
    VERIFY (v[4].size () == 1);
    VERIFY (v[4][0] == 4);
    VERIFY (v[5].size () == 0);
}

mt19937 rng(12345);

vector<photon> get_random_photons (const size_t total)
{
    vector<photon> p (total);
    uniform_real_distribution<> df (-100.0, 100.0);
    uniform_int_distribution<> di (0, total - 1);

    for (auto &i : p)
    {
        i.h5_index = di (rng);
        i.x = df (rng);
        i.z = df (rng);
    }

    return p;
}

void test_classify (size_t n)
{
    const auto p = get_random_photons (n);
    const params a;
    set<unsigned> classes { 0, 1, 40, 41 };

    // Get each photons classification
    const auto q = classify (p, a);
    // Make sure they are all valid
    for (auto i : q)
    {
        auto it = classes.find (i.prediction);
        VERIFY (it != classes.end ());
    }
    // Make sure nothing changed that shouldn't have
    VERIFY (p.size () == q.size ());
    for (size_t i = 0; i < p.size (); ++i)
    {
        VERIFY (p[i].h5_index == q[i].h5_index);
        VERIFY (p[i].x == q[i].x);
        VERIFY (p[i].z == q[i].z);
        VERIFY (p[i].cls == q[i].cls);
    }
}

int main ()
{
    try
    {
        test_get_h_bins ();
        test_get_v_bins ();
        test_classify (10);
        test_classify (10'000);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
