#include "oopp/precompiled.h"
#include "oopp/dataframe.h"
#include "oopp/timer.h"
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
        timer::timer t0;

        // Read the points
        const auto df = dataframe::read_buffered (cin);

        // Convert it to the correct format
        const auto p = convert_dataframe (df);

        if (args.verbose)
        {
            clog << p.size () << " points read" << endl;
            clog << "Classifying points" << endl;
        }

        // Start a timer
        timer::timer t1;

        // Classify the points
        const auto q = classify (p, args.params, args.use_predictions);

        // Time the classification only
        t1.stop ();

        // Write classified output to stdout
        write_predictions (cout, p, q);

        // Time classification and I/O
        t0.stop ();

        // Write out performance stats
        if (args.verbose)
        {
            const double e0 = t0.elapsed_ns ();
            const double e1 = t1.elapsed_ns ();
            const double s0 = (e0 == 0.0) ? 0.0 : e0 / 1'000'000'000;
            const double s1 = (e1 == 0.0) ? 0.0 : e1 / 1'000'000'000;
            const size_t pps0 = (s0 == 0.0) ? 0.0 : p.size () / s0;
            const size_t pps1 = (s1 == 0.0) ? 0.0 : p.size () / s1;
            clog.imbue (std::locale (""));
            clog << fixed;
            clog << setprecision(3);
            clog << p.size () << " photons" << endl;
            clog << s0 << "/" << s1 << " total/process seconds" << endl;
            clog << pps0 << "/" << pps1 << " total/process photons/second" << endl;
        }

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
