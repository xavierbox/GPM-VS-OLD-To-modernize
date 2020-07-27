#include "gpm_active_layer.h"
#include "gpm_extent_holder.h"
#include "libgp.h"
#include "../libutl/gpm_logger.h"

namespace Slb { namespace Exploration { namespace Gpm{

using namespace Tools;
gpm_active_layer::gpm_active_layer()
{

}



void gpm_active_layer::initialize( const int_extent_2d& extents, int sed_flag, int num_sediments )
{
	gpm_extent_holder extent_defs(extents);
	_extents = extents;
	sedflag = sed_flag;
	za.resize(extent_defs.full_extent_with_boundary_1);
	za.set_core_size(extent_defs.full_extent);
	if(sedflag>1){ // && (c.tranflag>0||c.ssrcflag>0)){
		zb  = float_2darray_type(extent_defs.full_extent_with_boundary_1);	  // Topo b, -1 and nr nc are beyond bounds */
		zb.set_core_size(extent_defs.full_extent);
		sediment_mix_type init_4float_array(num_sediments, 0);
		//init_4float_array.assign(0);
		sl.resize(extent_defs.full_extent, false, init_4float_array);    // Sed load at node
		serod.resize(extent_defs.full_extent);
	}
}

void gpm_active_layer::initialize_constants( float layer_thickness, float base_erodability )
{
	athick = layer_thickness;
	baserod = base_erodability;
}

const float_2darray_base_type & gpm_active_layer::get_za() const
{
	return za;
}

float_2darray_base_type* gpm_active_layer::get_za_ptr() 
{
	return &za;
}

const float_2darray_base_type & gpm_active_layer::get_zb() const
{
	return zb;
}

float_2darray_base_type* gpm_active_layer::get_zb_ptr() 
{
	return &zb;
}

const sediment_layer_base_type& gpm_active_layer::get_sl() const
{
	return sl;
}

sediment_layer_base_type* gpm_active_layer::get_sl_ptr() 
{
	return &sl;
}

const float_2darray_base_type& gpm_active_layer::get_serod() const
{
	return serod;
}

float_2darray_base_type* gpm_active_layer::get_serod_ptr() 
{
	return &serod;
}

int gpm_active_layer::sed_flag() const
{
	return sedflag;
}

float gpm_active_layer::layer_thickness() const
{
	return athick;
}

float gpm_active_layer::base_erodability() const
{
	return baserod;
}

void gpm_active_layer::initiate_active_layer()
{
	zb_start.fill(za);
}

int gpm_active_layer::initiate_active_layer( int ns, const Tools::gpm_logger& logger )
{
	int iret = 0;
	int nr = _extents.row_extent().upper();
	int nc = _extents.col_extent().upper();
	for(int i=0;i<nr;i++){
		for(int j=0;j<nc;j++){
			zb(i,j) = za(i,j)-athick;
			zb_start(i,j) = zb(i,j); // This gets lowered in erosion process;
			// Call erodif to create active layer and set serod 
			iret = erodif(_active_model, i, j, za(i,j), zb(i,j), sl(i,j), baserod, &(serod(i,j)), logger);
			if(iret!=0){
				logger.print(LOG_NORMAL,"Error detected by erodif(1): %d\n",iret);
			}
		}
	}
	return iret;
}

bool gpm_active_layer::find_erosion( float_2darray_base_type* erosion )
{
	bool has_erosion = false;
	if (sedflag > 0){
		float_2darray_base_type* base_surf = sedflag == 1 ? &za : &zb;
		has_erosion = find_erosion(zb_start, *base_surf, erosion);
	}
	return has_erosion;
}

void gpm_active_layer::set_top_surface_values( const float_2darray_base_type & top_surf )
{
	za.fill(top_surf);
}

bool gpm_active_layer::find_erosion(const float_2darray_base_type& zb_start, const float_2darray_base_type& base_surf, float_2darray_base_type* erosion_p)
{
	bool has_erosion = false;
	float_2darray_base_type& erosion(*erosion_p);
	int_extent_2d intersect=erosion.extents() & zb_start.extents() & base_surf.extents();
	int_extent_2d::extent_type row_extent = intersect.row_extent();
	int_extent_2d::extent_type col_extent = intersect.col_extent();		
	for(int i = row_extent.lower();i < row_extent.upper();++i){
		for(int j = col_extent.lower();j < col_extent.upper();++j){
			float dz = zb_start(i,j)-base_surf(i,j);
			if (dz > 0){
				erosion(i,j)=-dz;
				has_erosion = true;
			}
		}
	}
	return has_erosion;
}

int gpm_active_layer::initialize_active_layer( const struct parm& p, int top_index, const std::vector<int>& sed_indexes, int timestep, int additional, int ns, const Tools::gpm_logger& logger )
{
	int iret = _active_model.initialize(p, top_index, sed_indexes, timestep, additional);
	if ( iret != 0){
		logger.print(LOG_NORMAL, "Error detected in base functions preparing for erodif: %d\n", iret);
	}
	iret = initiate_active_layer(ns, logger);
	return iret;
}

}}}
