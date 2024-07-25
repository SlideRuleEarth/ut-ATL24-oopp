#pragma once

#include "oopp/precompiled.h"
#include "oopp/cmd_utils.h"
#include "oopp/oopp.h"

namespace oopp
{

namespace cmd
{

struct args
{
    bool help = false;
    bool verbose = false;
    bool use_predictions = false;
    oopp::params params;
};

std::ostream &operator<< (std::ostream &os, const args &args)
{
    os << std::boolalpha;
    os << "help: " << args.help << std::endl;
    os << "verbose: " << args.verbose << std::endl;
    os << "use-predictions: " << args.use_predictions << std::endl;
    os << "x-resolution: " << args.params.x_resolution << std::endl;
    os << "z-resolution: " << args.params.z_resolution << std::endl;
    os << "z-min: " << args.params.z_min << std::endl;
    os << "z-max: " << args.params.z_max << std::endl;
    os << "window-overlap: " << args.params.window_overlap << std::endl;
    return os;
}

args get_args (int argc, char **argv, const std::string &usage)
{
    args args;
    while (1)
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"help", no_argument, 0,  'h' },
            {"verbose", no_argument, 0,  'v' },
            {"use-predictions", no_argument, 0,  'p' },
            {"x-resolution", required_argument, 0,  'x' },
            {"z-resolution", required_argument, 0,  'z' },
            {"z-min", required_argument, 0,  'i' },
            {"z-max", required_argument, 0,  'a' },
            {"window-overlap", required_argument, 0,  'w' },
            {0,      0,           0,  0 }
        };

        int c = getopt_long(argc, argv, "hvpx:z:i:a:w:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            default:
            case 0:
            case 'h':
            {
                const size_t noptions = sizeof (long_options) / sizeof (struct option);
                cmd::print_help (std::clog, usage, noptions, long_options);
                if (c != 'h')
                    throw std::runtime_error ("Invalid option");
                args.help = true;
                return args;
            }
            case 'v': args.verbose = true; break;
            case 'p': args.use_predictions = true; break;
            case 'x': args.params.x_resolution = atof (optarg); break;
            case 'z': args.params.z_resolution = atof (optarg); break;
            case 'i': args.params.z_min = atof (optarg); break;
            case 'a': args.params.z_max = atof (optarg); break;
            case 'w': args.params.window_overlap = atof (optarg); break;
        }
    }

    // Check command line
    if (optind != argc)
        throw std::runtime_error ("Too many arguments on command line");

    return args;
}

} // namespace cmd

} // namespace oopp
