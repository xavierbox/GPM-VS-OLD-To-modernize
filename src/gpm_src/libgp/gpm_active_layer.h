#ifndef GPM_ACTIVE_LAYER_H
#define GPM_ACTIVE_LAYER_H

#include "gpm_sediment_model.h"
#include "gpm_basic_defs.h"
#include "gpm_vbl_array_2d.h"
#include "gpm_vbl_array_3d.h"
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm{

	// The active layer for a given time interval

	class gpm_active_layer{
	public:
		gpm_active_layer();
		void initialize(const int_extent_2d& extents, int sed_flag, int num_sediments);
		void initialize_constants(float layer_thickness, float base_erodability);

		int sed_flag() const;
		float layer_thickness() const;
		float base_erodability() const;

		// The top of the layer
		void set_top_surface_values(const float_2darray_base_type & top_surf);

		void initiate_active_layer( );
		int initialize_active_layer(const struct parm& p, int top_index, const std::vector<int>& sed_indexes, int timestep, int additional, int ns, const Tools::gpm_logger& logger);
		bool find_erosion(float_2darray_base_type* erosion);
		// Top of active layer
		const float_2darray_base_type & get_za()const;
		float_2darray_base_type* get_za_ptr() ;
		// Bottom of active layer
		const float_2darray_base_type & get_zb()const;
		float_2darray_base_type* get_zb_ptr() ;
		// Sediment mix
		const sediment_layer_base_type& get_sl() const;
		sediment_layer_base_type* get_sl_ptr() ;

		// erodability of base
		const float_2darray_base_type& get_serod() const;
		float_2darray_base_type* get_serod_ptr() ;
	private:
		gpm_active_layer(const gpm_active_layer& rhs);
		gpm_active_layer& operator=(const gpm_active_layer& rhs);

		bool find_erosion(const float_2darray_base_type& zb_start, const float_2darray_base_type& base_surf, float_2darray_base_type* erosion_p);
		int initiate_active_layer(int ns, const Tools::gpm_logger& logger);

		int sedflag;
		float athick;
		float baserod;
		float_2darray_type za;
		float_2darray_type zb;
		float_2darray_type zb_start;
		sediment_layer_type sl; // Sediment load over node or in active layer [4]
		float_2darray_type serod; // Sediment erodibility in active layer
		int_extent_2d _extents;
		gpm_sediment_model _active_model;
	};
}}}
#endif
