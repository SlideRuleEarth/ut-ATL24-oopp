#ifndef __openoceanspp_classifier__
#define __openoceanspp_classifier__

/******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include "LuaObject.h"
#include "OsApi.h"
#include "BathyClassifier.h"
#include "BathyParms.h"

/******************************************************************************
 * BATHY CLASSIFIER
 ******************************************************************************/

class OpenOceansPPClassifier: public BathyClassifier
{
    public:

        /*--------------------------------------------------------------------
         * Constants
         *--------------------------------------------------------------------*/

        static const char* CLASSIFIER_NAME;
        static const char* OPENOCEANSPP_PARMS;
        static const char* DEFAULT_OPENOCEANSPP_MODEL;

        /*--------------------------------------------------------------------
         * Typedefs
         *--------------------------------------------------------------------*/

        struct parms_t {
            bool            set_class;          // whether to update class_ph in extent
            bool            set_surface;        // whether to update surface_h in extent
            bool            use_predictions;    // whether to update surface_h in extent
            bool            verbose;            // verbose settin gin XGBoost library
            double          x_resolution;
            double          z_resolution;
            double          z_min;
            double          z_max;
            double          surface_z_min;
            double          surface_z_max;
            double          bathy_min_depth;
            double          vertical_smoothing_sigma;
            double          surface_smoothing_sigma;
            double          bathy_smoothing_sigma;
            double          min_peak_prominence;
            size_t          min_peak_distance;
            size_t          min_surface_photons_per_window;
            size_t          min_bathy_photons_per_window;

            parms_t(): 
                set_class (false),
                set_surface (false),
                use_predictions (false),
                verbose (true),
                x_resolution (25.0),
                z_resolution (0.2),
                z_min (-50),
                z_max (30),
                surface_z_min (-20),
                surface_z_max (20),
                bathy_min_depth (0.5),
                vertical_smoothing_sigma (0.5),
                surface_smoothing_sigma (100.0),
                bathy_smoothing_sigma (10.0),
                min_peak_prominence (0.01),
                min_peak_distance (2),
                min_surface_photons_per_window (5),
                min_bathy_photons_per_window (5) {};
            ~parms_t() {};
        };

        /*--------------------------------------------------------------------
         * Methods
         *--------------------------------------------------------------------*/

        static int  luaCreate   (lua_State* L);
        static void init        (void);

        bool run (const vector<BathyParms::extent_t*>& extents) override;

    protected:

        /*--------------------------------------------------------------------
         * Methods
         *--------------------------------------------------------------------*/

        OpenOceansPPClassifier (lua_State* L, int index);
        ~OpenOceansPPClassifier (void) override;

        /*--------------------------------------------------------------------
         * Data
         *--------------------------------------------------------------------*/

        parms_t parms;
};

#endif  /* __openoceanspp_classifier__ */
