/******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include "oopp/precompiled.h"
#include "oopp/dataframe.h"
#include "oopp/timer.h"
#include "classify_cmd.h"
#include "oopp/oopp.h"

#include "OpenOceansPPClassifier.h"
#include "bathy/BathyParms.h"

/******************************************************************************
 * DATA
 ******************************************************************************/

const char* OpenOceansPPClassifier::CLASSIFIER_NAME = "openoceanspp";
const char* OpenOceansPPClassifier::OPENOCEANSPP_PARMS = "openoceanspp";

static const char* OPENOCEANSPP_PARM_SET_CLASS = "set_class";
static const char* OPENOCEANSPP_PARM_SET_SURFACE = "set_surface";
static const char* OPENOCEANSPP_PARM_USE_PREDICTIONS = "use_predictions";
static const char* OPENOCEANSPP_PARM_VERBOSE = "verbose";
static const char* OPENOCEANSPP_PARM_X_RESOLUTION = "x_resolution";
static const char* OPENOCEANSPP_PARM_Z_RESOLUTION = "z_resolution";
static const char* OPENOCEANSPP_PARM_Z_MIN = "z_min";
static const char* OPENOCEANSPP_PARM_Z_MAX = "z_max";
static const char* OPENOCEANSPP_PARM_SURFACE_Z_MIN = "surface_z_min";
static const char* OPENOCEANSPP_PARM_SURFACE_Z_MAX = "surface_z_max";
static const char* OPENOCEANSPP_PARM_BATHY_MIN_DEPTH = "bathy_min_depth";
static const char* OPENOCEANSPP_PARM_VERTICAL_SMOOTHING_SIGMA = "vertical_smoothing_sigma";
static const char* OPENOCEANSPP_PARM_SURFACE_SMOOTHING_SIGMA = "surface_smoothing_sigma";
static const char* OPENOCEANSPP_PARM_BATHY_SMOOTHING_SIGMA = "bathy_smoothing_sigma";
static const char* OPENOCEANSPP_PARM_MIN_PEAK_PROMINENCE = "min_peak_prominence";
static const char* OPENOCEANSPP_PARM_MIN_PEAK_DISTANCE = "min_peak_distance";
static const char* OPENOCEANSPP_PARM_MIN_SURFACE_PHOTONS_PER_WINDOW = "min_surface_photons_per_window";
static const char* OPENOCEANSPP_PARM_MIN_BATHY_PHOTONS_PER_WINDOW = "min_bathy_photons_per_window";

/******************************************************************************
 * METHODS
 ******************************************************************************/

/*----------------------------------------------------------------------------
 * luaCreate - create(parms)
 *----------------------------------------------------------------------------*/
int OpenOceansPPClassifier::luaCreate (lua_State* L)
{
    try
    {
        return createLuaObject(L, new OpenOceansPPClassifier(L, 1));
    }
    catch(const RunTimeException& e)
    {
        mlog(e.level(), "Error creating OpenOceansPPClassifier: %s", e.what());
        return returnLuaStatus(L, false);
    }
}

/*----------------------------------------------------------------------------
 * init
 *----------------------------------------------------------------------------*/
void OpenOceansPPClassifier::init (void)
{
}

/*----------------------------------------------------------------------------
 * Constructor
 *----------------------------------------------------------------------------*/
OpenOceansPPClassifier::OpenOceansPPClassifier (lua_State* L, int index):
    BathyClassifier(L, CLASSIFIER_NAME)
{
    /* Build Parameters */
    if(lua_istable(L, index))
    {
        /* set class */
        lua_getfield(L, index, OPENOCEANSPP_PARM_SET_CLASS);
        parms.set_class = LuaObject::getLuaBoolean(L, -1, true, parms.set_class);
        lua_pop(L, 1);

        /* set surface */
        lua_getfield(L, index, OPENOCEANSPP_PARM_SET_SURFACE);
        parms.set_surface = LuaObject::getLuaBoolean(L, -1, true, parms.set_surface);
        lua_pop(L, 1);

        /* use predictions */
        lua_getfield(L, index, OPENOCEANSPP_PARM_USE_PREDICTIONS);
        parms.use_predictions = LuaObject::getLuaBoolean(L, -1, true, parms.use_predictions);
        lua_pop(L, 1);

        /* verbose */
        lua_getfield(L, index, OPENOCEANSPP_PARM_VERBOSE);
        parms.verbose = LuaObject::getLuaBoolean(L, -1, true, parms.verbose);
        lua_pop(L, 1);

        /* x_resolution */
        lua_getfield(L, index, OPENOCEANSPP_PARM_X_RESOLUTION);
        parms.x_resolution = LuaObject::getLuaFloat(L, -1, true, parms.x_resolution);
        lua_pop(L, 1);

        /* y_resolution */
        lua_getfield(L, index, OPENOCEANSPP_PARM_Z_RESOLUTION);
        parms.z_resolution = LuaObject::getLuaFloat(L, -1, true, parms.z_resolution);
        lua_pop(L, 1);

        /* z_min */
        lua_getfield(L, index, OPENOCEANSPP_PARM_Z_MIN);
        parms.z_min = LuaObject::getLuaFloat(L, -1, true, parms.z_min);
        lua_pop(L, 1);

        /* z_max */
        lua_getfield(L, index, OPENOCEANSPP_PARM_Z_MAX);
        parms.z_max = LuaObject::getLuaFloat(L, -1, true, parms.z_max);
        lua_pop(L, 1);

        /* surface_z_min */
        lua_getfield(L, index, OPENOCEANSPP_PARM_SURFACE_Z_MIN);
        parms.surface_z_min = LuaObject::getLuaFloat(L, -1, true, parms.surface_z_min);
        lua_pop(L, 1);

        /* surface_z_max */
        lua_getfield(L, index, OPENOCEANSPP_PARM_SURFACE_Z_MAX);
        parms.surface_z_max = LuaObject::getLuaFloat(L, -1, true, parms.surface_z_max);
        lua_pop(L, 1);

        /* bathy_min_depth */
        lua_getfield(L, index, OPENOCEANSPP_PARM_BATHY_MIN_DEPTH);
        parms.bathy_min_depth = LuaObject::getLuaFloat(L, -1, true, parms.bathy_min_depth);
        lua_pop(L, 1);

        /* vertical_smoothing_sigma */
        lua_getfield(L, index, OPENOCEANSPP_PARM_VERTICAL_SMOOTHING_SIGMA);
        parms.vertical_smoothing_sigma = LuaObject::getLuaFloat(L, -1, true, parms.vertical_smoothing_sigma);
        lua_pop(L, 1);

        /* surface_smoothing_sigma */
        lua_getfield(L, index, OPENOCEANSPP_PARM_SURFACE_SMOOTHING_SIGMA);
        parms.surface_smoothing_sigma = LuaObject::getLuaFloat(L, -1, true, parms.surface_smoothing_sigma);
        lua_pop(L, 1);

        /* surface_smoothing_sigma */
        lua_getfield(L, index, OPENOCEANSPP_PARM_BATHY_SMOOTHING_SIGMA);
        parms.bathy_smoothing_sigma = LuaObject::getLuaFloat(L, -1, true, parms.bathy_smoothing_sigma);
        lua_pop(L, 1);

        /* min_peak_prominence */
        lua_getfield(L, index, OPENOCEANSPP_PARM_MIN_PEAK_PROMINENCE);
        parms.min_peak_prominence = LuaObject::getLuaFloat(L, -1, true, parms.min_peak_prominence);
        lua_pop(L, 1);

        /* min_peak_distance */
        lua_getfield(L, index, OPENOCEANSPP_PARM_MIN_PEAK_DISTANCE);
        parms.min_peak_distance = LuaObject::getLuaInteger(L, -1, true, parms.min_peak_distance);
        lua_pop(L, 1);

        /* min_surface_photons_per_window */
        lua_getfield(L, index, OPENOCEANSPP_PARM_MIN_SURFACE_PHOTONS_PER_WINDOW);
        parms.min_surface_photons_per_window = LuaObject::getLuaInteger(L, -1, true, parms.min_surface_photons_per_window);
        lua_pop(L, 1);

        /* min_bathy_photons_per_window */
        lua_getfield(L, index, OPENOCEANSPP_PARM_MIN_BATHY_PHOTONS_PER_WINDOW);
        parms.min_bathy_photons_per_window = LuaObject::getLuaInteger(L, -1, true, parms.min_bathy_photons_per_window);
        lua_pop(L, 1);
    }
}

/*----------------------------------------------------------------------------
 * Destructor
 *----------------------------------------------------------------------------*/
OpenOceansPPClassifier::~OpenOceansPPClassifier (void)
{
}

/*----------------------------------------------------------------------------
 * run
 *----------------------------------------------------------------------------*/
bool OpenOceansPPClassifier::run (const vector<BathyParms::extent_t*>& extents)
{
    try
    {
        // Get number of samples
        size_t number_of_samples = 0;
        for(size_t i = 0; i < extents.size(); i++)
        {
            number_of_samples += extents[i]->photon_count;
        }

        // Preallocate samples vector
        std::vector<oopp::photon> samples;
        samples.reserve(number_of_samples);
        mlog(INFO, "Building %ld photon samples", number_of_samples);

        // Build and add samples
        for(size_t i = 0; i < extents.size(); i++)
        {
            BathyParms::photon_t* photons = extents[i]->photons;
            for(size_t j = 0; j < extents[i]->photon_count; j++)
            {
                // Populate photon
                oopp::photon photon = {
                    .h5_index = (i << 32) | j, // TEMPORARY HACK to get results below
                    .x = photons[j].x_atc,
                    .z = photons[j].ortho_h,
                    .prediction = static_cast<unsigned>(photons[j].class_ph)
                };
                samples.push_back(photon);

                // Clear classification (if necessary)
                if(parms.set_class)
                {
                    photons[j].class_ph = BathyParms::UNCLASSIFIED;
                }
            }
        }

        // Initialize Parameters
        oopp::params params = {
            .x_resolution = parms.x_resolution,
            .z_resolution = parms.z_resolution,
            .z_min = parms.z_min,
            .z_max = parms.z_max,
            .surface_z_min = parms.surface_z_min,
            .surface_z_max = parms.surface_z_max,
            .bathy_min_depth = parms.bathy_min_depth,
            .vertical_smoothing_sigma = parms.vertical_smoothing_sigma,
            .surface_smoothing_sigma = parms.surface_smoothing_sigma,
            .bathy_smoothing_sigma = parms.bathy_smoothing_sigma,
            .min_peak_prominence = parms.min_peak_prominence,
            .min_peak_distance = parms.min_peak_distance,
            .min_surface_photons_per_window = parms.min_surface_photons_per_window,
            .min_bathy_photons_per_window = parms.min_bathy_photons_per_window,
            .surface_n_stddev = 3.0,
            .bathy_n_stddev = 3.0
        }; 

        // Run classification
        samples = classify(samples, params, parms.use_predictions);

        // Update extents
        for(auto sample: samples)
        {
            size_t i = (sample.h5_index >> 32) & 0xFFFFFFFF;
            size_t j = sample.h5_index & 0xFFFFFFFF;
            if(parms.set_surface) extents[i]->photons[j].surface_h = sample.surface_elevation;
            if(parms.set_class) extents[i]->photons[j].class_ph = sample.prediction;
            extents[i]->photons[j].predictions[classifier] = sample.prediction;
        }
    }
    catch (const std::exception &e)
    {
        mlog(CRITICAL, "Failed to run openoceanspp classifier: %s", e.what());
        return false;
    }

    return true;
}
