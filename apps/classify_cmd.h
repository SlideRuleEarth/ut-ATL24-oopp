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
    oopp::params oo_params;
};

std::ostream &operator<< (std::ostream &os, const args &args)
{
    os << std::boolalpha;
    os << "help: " << args.help << std::endl;
    os << "verbose: " << args.verbose << std::endl;
    os << "use-predictions: " << args.use_predictions << std::endl;
    os << args.oo_params;
    return os;
}

const int OO_X_RESOLUTION_ID = 1001;
const int OO_Z_RESOLUTION_ID = 1002;
const int OO_Z_MIN_ID = 1003;
const int OO_Z_MAX_ID = 1004;
const int OO_SURFACE_Z_MIN_ID = 1005;
const int OO_SURFACE_Z_MAX_ID = 1006;
const int OO_BATHY_MIN_DEPTH_ID = 1007;
const int OO_VERTICAL_SMOOTHING_SIGMA_ID = 1008;
const int OO_SURFACE_SMOOTHING_SIGMA_ID = 1009;
const int OO_BATHY_SMOOTHING_SIGMA_ID = 1010;
const int OO_MIN_PEAK_PROMINENCE_ID = 1011;
const int OO_MIN_PEAK_DISTANCE_ID = 1012;
const int OO_MIN_SURFACE_PHOTONS_PER_WINDOW_ID = 1013;
const int OO_MIN_BATHY_PHOTONS_PER_WINDOW_ID = 1014;

args get_args (int argc, char **argv, const std::string &usage)
{
    args args;
    while (1)
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"help", no_argument, 0,  'h'},
            {"verbose", no_argument, 0,  'v'},
            {"use-predictions", no_argument, 0,  'p'},
            {"oo-x-resolution", required_argument, 0, OO_X_RESOLUTION_ID},
            {"oo-z-resolution", required_argument, 0, OO_Z_RESOLUTION_ID},
            {"oo-z-min", required_argument, 0, OO_Z_MIN_ID},
            {"oo-z-max", required_argument, 0, OO_Z_MAX_ID},
            {"oo-surface-z-min-id", required_argument, 0, OO_SURFACE_Z_MIN_ID},
            {"oo-surface-z-max-id", required_argument, 0, OO_SURFACE_Z_MAX_ID},
            {"oo-bathy-min-depth-id", required_argument, 0, OO_BATHY_MIN_DEPTH_ID},
            {"oo-vertical-smoothing-sigma-id", required_argument, 0, OO_VERTICAL_SMOOTHING_SIGMA_ID},
            {"oo-surface-smoothing-sigma-id", required_argument, 0, OO_SURFACE_SMOOTHING_SIGMA_ID},
            {"oo-bathy-smoothing-sigma-id", required_argument, 0, OO_BATHY_SMOOTHING_SIGMA_ID},
            {"oo-min-peak-prominence-id", required_argument, 0, OO_MIN_PEAK_PROMINENCE_ID},
            {"oo-min-peak-distance-id", required_argument, 0, OO_MIN_PEAK_DISTANCE_ID},
            {"oo-min-surface-photons-per-window-id", required_argument, 0, OO_MIN_SURFACE_PHOTONS_PER_WINDOW_ID},
            {"oo-min-bathy-photons-per-window-id", required_argument, 0, OO_MIN_BATHY_PHOTONS_PER_WINDOW_ID},
            {0,      0,           0,  0 }
        };

        int c = getopt_long(argc, argv, "hvpx:z:i:a:", long_options, &option_index);
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
            case OO_X_RESOLUTION_ID: args.oo_params.x_resolution = atof (optarg); break;
            case OO_Z_RESOLUTION_ID: args.oo_params.z_resolution = atof (optarg); break;
            case OO_Z_MIN_ID: args.oo_params.z_min = atof (optarg); break;
            case OO_Z_MAX_ID: args.oo_params.z_max = atof (optarg); break;
            case OO_SURFACE_Z_MIN_ID: args.oo_params.surface_z_min = atof (optarg); break;
            case OO_SURFACE_Z_MAX_ID: args.oo_params.surface_z_max = atof (optarg); break;
            case OO_BATHY_MIN_DEPTH_ID: args.oo_params.bathy_min_depth = atof (optarg); break;
            case OO_VERTICAL_SMOOTHING_SIGMA_ID: args.oo_params.vertical_smoothing_sigma = atof (optarg); break;
            case OO_SURFACE_SMOOTHING_SIGMA_ID: args.oo_params.surface_smoothing_sigma = atof (optarg); break;
            case OO_BATHY_SMOOTHING_SIGMA_ID: args.oo_params.bathy_smoothing_sigma = atof (optarg); break;
            case OO_MIN_PEAK_PROMINENCE_ID: args.oo_params.min_peak_prominence = atof (optarg); break;
            case OO_MIN_PEAK_DISTANCE_ID: args.oo_params.min_peak_distance = atol (optarg); break;
            case OO_MIN_SURFACE_PHOTONS_PER_WINDOW_ID: args.oo_params.min_surface_photons_per_window = atol (optarg); break;
            case OO_MIN_BATHY_PHOTONS_PER_WINDOW_ID: args.oo_params.min_bathy_photons_per_window = atol (optarg); break;
        }
    }

    // Check command line
    if (optind != argc)
        throw std::runtime_error ("Too many arguments on command line");

    return args;
}

} // namespace cmd

} // namespace oopp
