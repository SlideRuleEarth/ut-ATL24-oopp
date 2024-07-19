#include "oopp/precompiled.h"
#include "oopp/dataframe.h"
#include "classify_cmd.h"
#include "oopp.h"

const std::string usage {"classify [options] < fn.csv"};

int main (int argc, char **argv)
{
    using namespace std;
    using namespace oopp;

    try
    {
        // Parse the args
        const auto args = cmd::get_args (argc, argv, usage);

        // If you are getting help, exit without an error
        if (args.help)
            return 0;

        if (args.verbose)
        {
            // Show the args
            clog << "cmd_line_parameters:" << endl;
            clog << args;
            clog << "Reading dataframe from stdin" << endl;
        }

        // Start a timer
        timer t;

        // Read the points
        const auto df = dataframe::read (cin);

        // Convert it to the correct format
        const auto p = convert_dataframe (df);

        if (args.verbose)
        {
            clog << p.size () << " points read" << endl;
            clog << "Classifying points" << endl;
        }

        // Classify the points
        auto q (p);

        // Temp
        for (size_t i = 0; i < p.size (); ++i)
        {
            if (!args.use_predictions)
                q[i].prediction = p[i].cls;
            else if (q[i].prediction != 0)
                q[i].prediction = p[i].prediction;
        }

        // Write classified output to stdout
        write_photons (cout, q);

        // Get the elapsed time
        t.stop ();

        if (args.verbose)
        {
            const double e = t.elapsed_ns ();
            const double s = (e == 0.0) ? 0.0 : e / 1'000'000'000;
            const size_t pps = (s == 0.0) ? 0.0 : p.size () / s;
            clog << fixed;
            clog << p.size () << " photons" << endl;
            clog << s << " seconds" << endl;
            clog << pps << " photons/sec" << endl;
        }

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
