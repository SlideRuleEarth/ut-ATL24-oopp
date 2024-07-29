#include "oopp/precompiled.h"
#include "oopp/utils.h"
#include "oopp/verify.h"

using namespace std;
using namespace oopp::utils;

void test_normalize ()
{
    vector<double> x1 { 1.0, 2.0, 3.0, 4.0, 5.0 };
    auto y = normalize (x1);
    VERIFY (y[0] == 0.0);
    VERIFY (y[4] == 1.0);
    vector<double> x2 { 1.0, 100.0, -3.0, 4.0, 5.0 };
    y = normalize (x2);
    VERIFY (y[2] == 0.0);
    VERIFY (y[1] == 1.0);
}

void test_mean ()
{
    auto y = mean (vector<double> { 1.0, 2.0, 3.0, 4.0, 5.0 });
    VERIFY (round(y) == 3);
}

void test_variance ()
{
    vector<double> x1 { 1.0, 2.0, 3.0, 4.0, 5.0 };
    auto y = variance (x1);
    VERIFY (round(y) == 2);
}

void test_z_score ()
{
    mt19937 r;
    uniform_real_distribution<> d (-10.0, 100.0);

    vector<double> x (1000);
    for (auto &i : x)
        i = d (r);

    auto u = mean (x);
    auto s = std::sqrt (variance (x));

    VERIFY (round (u) > 30);
    VERIFY (round (u) < 60);
    VERIFY (round (s) > 10);
    VERIFY (round (s) < 50);

    auto y = z_score (x);
    u = mean (y);
    s = std::sqrt (variance (y));

    VERIFY (round (u) == 0);
    VERIFY (round (s) == 1);
}

void test_median ()
{
    vector<int> x { 7, 4, 2, 9, 5 };
    int y = median (x);
    VERIFY (y == 5);

    x.push_back (1);
    x.push_back (-1);
    y = median (x);
    VERIFY (y == 4);

    x.push_back (100);
    x.push_back (101);
    x.push_back (-1);
    y = median (x);
    VERIFY (y == 5);
}

void test_pmf ()
{
    {
    vector<size_t> h (10, 1.0);
    const auto p = convert_to_pmf (h);
    VERIFY (round (p[0] * 100) == 10);
    }

    {
    vector<size_t> h (1, 1000.0);
    const auto p = convert_to_pmf (h);
    VERIFY (round (p[0] * 100) == 100);
    }

    {
    vector<size_t> h { 20, 80, 0, 100 };
    const auto p = convert_to_pmf (h);
    VERIFY (round (p[0] * 100) == 10);
    VERIFY (round (p[1] * 100) == 40);
    VERIFY (round (p[2] * 100) == 0);
    VERIFY (round (p[3] * 100) == 50);
    }
}

void test_gaussian_filter ()
{
    {
    vector<double> x (21);
    x[10] = 1.0;
    x = gaussian_1D_filter (x, 3);
    VERIFY (x[ 9] != 0.0);
    VERIFY (x[10] < 1.0);
    VERIFY (x[11] != 0.0);
    VERIFY (x[ 9] < x[10]);
    VERIFY (x[11] < x[10]);
    VERIFY (round (x[9] * 100) == round (x[11] * 100));
    }

    {
    vector<double> x (20);
    for (size_t i = 10; i < 20; ++i)
        x[i] = 1.0;
    x = gaussian_1D_filter (x, 2.3);
    VERIFY (x[ 8] != 0.0);
    VERIFY (x[ 9] < 0.5);
    VERIFY (x[10] > 0.5);
    VERIFY (x[11] != 1.0);
    }
}

void test_find_peaks ()
{
    vector<double> x (20);
    x[0] = 1.0;
    x[4] = 1.0;
    x[5] = 0.98;
    x[6] = 0.99;
    x[18] = 1.0;
    auto y = find_peaks (x);
    VERIFY (y.size () == 3);
    VERIFY (y[0] = 5);
    VERIFY (y[1] = 6);
    VERIFY (y[2] = 8);
    x = gaussian_1D_filter (x, 1.0);
    y = find_peaks (x);
    VERIFY (y.size () == 1);
    VERIFY (y[0] == 5);
}

int main ()
{
    try
    {
        test_normalize ();
        test_mean ();
        test_variance ();
        test_z_score ();
        test_median ();
        test_pmf ();
        test_gaussian_filter ();
        test_find_peaks ();

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
