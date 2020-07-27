#include "gpm_sediment_file_io.h"
#include <string>

std::string old_input = R"###(
{ 
"NUMSEDS":4,
"SEDNAME":["Sand (coarse)","Carbonate","Silt","Clay"],
"ID":["://SedimentDataSource/6b0342f4-465d-4597-8423-d8ecf545ac42","://SedimentDataSource/5b87e73a-d64a-430c-a0ae-65f3a788754b","://SedimentDataSource/ee6154bb-60c8-459d-92e5-4ec0d846b10c","://SedimentDataSource/a480ca18-dc4a-40c2-81e3-d9fff1138a25"],
"DIAMETER":[1,0.1,0.0099999998,0.001],
"GRAINDEN":[2.7,2.7,2.7,2.7],
"TRANSP":[0.80000001,1.6,3.2,6.4000001],
"POR0":[0.30000001,0.34,0.40000001,0.55000001],
"POR1":[0.2,0.15000001,0.1,0.050000001],
"PERM0":[100,10,0.1,0.0099999998],
"PERM1":[10,1,0.1,0.001],
"PERMANIS":[1,1,1,10],
"COMP":[50,50,5,0.5],
"ERODABILITY_COEFF":[1,1,1,1],
"ERODABILITY_FUNC":[
    [
        [1,1],
        [-0,-20000]
    ]
],
"ERODABILITY_PROP":["#0","1.0","1.0","1.0"],
"NO_VAL":-1e+12,
"TRANSFORM_BY_TRANSPORT":["://SedimentDataSource/ee6154bb-60c8-459d-92e5-4ec0d846b10c","","",""],
"DO_INPLACE_TRANSFORM":[7,0,0,0],
"INPLACE_TRANSFORM_COEFF":[0.2,0,0,0],
"VERSION": "1.1.3.0"
}
)###";

std::string new_input = R"###(
{
  "VERSION": "2.0.0.0",
  "SCHEMA_VERSION": "1.0.0.0",
  "ARRAY_2D": [
    {
      "NAME": "Geotime growth scaling (Sediment 1)",
	"VALUES": [
		[
			1.0,
			1.0
		],
			[
				-20000.0,
				0.0
			]
	]
	}
  ],
  "SEDIMENTS": [
  {
	  "Name": "Sand (coarse)",
		  "Id" : "://SedimentDataSource/6b0342f4-465d-4597-8423-d8ecf545ac42",
		  "Diameter" : 1.0,
		  "Density" : 2.7,
		  "InitialPorosity" : 0.3,
		  "InitialPermeability" : 100.0,
		  "CompactedPorosity" : 0.2,
		  "CompactedPermeability" : 10.0,
		  "Compaction" : 50.0,
		  "PermeabilityAnisotropy" : 1.0,
		  "Transportability" : 0.8,
		  "ErodabilityCoefficient" : 1.0,
		  "ErodabilityFunction" : "/ARRAY_2D/0",
		  "InplaceTransformation" : 7,
		  "SedimentTransformation" : "/SEDIMENTS/2",
		  "InplaceTransformationCoefficient" : 0.2
  },
	{
	  "Name": "Carbonate",
	  "Id" : "://SedimentDataSource/5b87e73a-d64a-430c-a0ae-65f3a788754b",
	  "Diameter" : 0.1,
	  "Density" : 2.7,
	  "InitialPorosity" : 0.34,
	  "InitialPermeability" : 10.0,
	  "CompactedPorosity" : 0.15,
	  "CompactedPermeability" : 1.0,
	  "Compaction" : 50.0,
	  "PermeabilityAnisotropy" : 1.0,
	  "Transportability" : 1.6,
	  "ErodabilityCoefficient" : 1.0,
	  "InplaceTransformation" : 0,
	  "InplaceTransformationCoefficient" : 0.0
	},
	{
	  "Name": "Silt",
	  "Id" : "://SedimentDataSource/ee6154bb-60c8-459d-92e5-4ec0d846b10c",
	  "Diameter" : 0.01,
	  "Density" : 2.7,
	  "InitialPorosity" : 0.4,
	  "InitialPermeability" : 0.1,
	  "CompactedPorosity" : 0.1,
	  "CompactedPermeability" : 0.1,
	  "Compaction" : 5.0,
	  "PermeabilityAnisotropy" : 1.0,
	  "Transportability" : 3.2,
	  "ErodabilityCoefficient" : 1.0,
	  "InplaceTransformation" : 0,
	  "InplaceTransformationCoefficient" : 0.0
	},
	{
	  "Name": "Clay",
	  "Id" : "://SedimentDataSource/a480ca18-dc4a-40c2-81e3-d9fff1138a25",
	  "Diameter" : 0.001,
	  "Density" : 2.7,
	  "InitialPorosity" : 0.55,
	  "InitialPermeability" : 0.01,
	  "CompactedPorosity" : 0.05,
	  "CompactedPermeability" : 0.001,
	  "Compaction" : 0.5,
	  "PermeabilityAnisotropy" : 10.0,
	  "Transportability" : 6.4,
	  "ErodabilityCoefficient" : 1.0,
	  "InplaceTransformation" : 0,
	  "InplaceTransformationCoefficient" : 0.0
	}
  ],
	  "PARAMETERS": {
		  "Sediments": [
			  "/SEDIMENTS/0",
				  "/SEDIMENTS/1",
				  "/SEDIMENTS/2",
				  "/SEDIMENTS/3"
		  ]
	  }
}
)###";

using namespace Slb::Exploration::Gpm;

bool check_arrays_equal(const float_2darray_base_type& v1, const float_2darray_base_type& v2)
{
	if (v1.extents() == v2.extents()) {
		const auto res = std::equal(v1.begin(), v1.end(), v2.begin(), [](float a, float b) {return std::abs(a - b) < 1e-5; });
		return res;
	}
	return false;
}
int test_new_format(int argc, char* argv[])
{
	Tools::gpm_default_logger my_logger(Tools::LOG_VERBOSE, true);
	gpm_sediment_file_io old_tester;
	auto iret = old_tester.read_parameters_from_string(old_input, my_logger);
	if (iret == 0) {
		gpm_sediment_file_io new_tester;
		iret = new_tester.read_parameters_from_string(new_input, my_logger);
		if (iret == 0) {
			pars tmp;
			new_tester.get_parms(&tmp);
			return 0;
		}
	}
	return 1;
}

