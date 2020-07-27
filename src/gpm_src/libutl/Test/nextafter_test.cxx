#include <cmath>
#include <cfenv>
#include <limits>

int nextafter_test(int argc, char* argv[])
{
	auto nexta=std::nextafterf(0.0F, 1.0F);
    auto nextt = std::nexttowardf(0.0F, 1.0F);
	auto fmin = std::numeric_limits<float>::min();
	auto nextb = std::fetestexcept(FE_INEXACT);
	auto inv = 1.0F / fmin;
    return 0;
}
