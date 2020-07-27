#include "mann.h"
using namespace Slb::Exploration::Gpm;

int mannings_test(int argc, char* argv[])
{
	manning_1d mann(1.0F, 1.0F);
	float vel;

	mann.calculate_use_dx(-0.68999F, 0.446F, -0.01F, 0.031F, &vel, 0.0F);
	mann.calculate_use_dx(-0.68999F, 0.446F, 0.0F, 0.0F, &vel, 0.0F);
	mann.calculate_use_dx(-0.68999F, 1e-10F, -0.001F, 0.031F, &vel, 0.0F);
    return 0;
}
