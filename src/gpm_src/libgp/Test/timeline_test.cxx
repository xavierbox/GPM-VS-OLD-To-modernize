// -- Schlumberger Private --

#include "libgp.h"
#include "gpm_compute_interval.h"
#include "gpm_find_sediment_intervals.h"
#include <boost/icl/separate_interval_set.hpp>
#include <boost/icl/split_interval_map.hpp>
#include <boost/math/special_functions/next.hpp>
#include <limits>
#include <bitset>
//#include "na_discrete_events.h"

using namespace Slb::Exploration::Gpm;


int erodif_test(int argc, char* argv[]) {
    float ztop1 = 195.10641F;
    float zbot1 = 194.65889F;
    float tops1[] = {188.26730f,191.00844f,192.92064f,194.19421f,196.10641f};
    float sed11[] = {0.25000000f,0.45380813f,0.15582958f,0.13605416f,0.046718560f};
    float sed21[] = {0.25000000f,0.41254240f,0.14165966f,0.15509057f,0.053255320f};
    float sed31[] = {0.25000000f,0.066824742f,0.67956436f,0.49464884f,0.82647151f};

    float ztop2 = 206.73109F;
    float zbot2 = 205.84111F;
    float tops2[] = {204.61810F,204.71527f,206.44856f,209.31981f,211.05310F};
    float sed12[] = {0.25000000f,0.35185745f,0.12873004f,0.19277850f,0.070529655f};
    float sed22[] = {0.25000000f,0.21604753f,0.079042822f,0.29015982f,0.10615744f};
    float sed32[] = {0.25000000f,0.21604753f,0.71318430f,0.27156475f,0.73349577f};

    float ztop3 = 261.40137F;
    float zbot3 = 261.40067F;
    float tops3[] = {252.86038f,267.71188f,267.75482f,252.86038f,262.40137f};

    boost::icl::split_interval_map<float, int> intervals1 = find_sediment_intervals(zbot1, ztop1, std::vector<float>(tops1, tops1 + 5));
    boost::icl::split_interval_map<float, int> intervals2 = find_sediment_intervals(zbot2, ztop2, std::vector<float>(tops2, tops2 + 5));
    boost::icl::split_interval_map<float, int> intervals3 = find_sediment_intervals(zbot2, 212, std::vector<float>(tops2, tops2 + 5));
    boost::icl::split_interval_map<float, int> intervals4 = find_sediment_intervals(zbot3, ztop3, std::vector<float>(tops3, tops3 + 5));
    return 0;
}

int timeline_test(int argc, char* argv[]) {

    double timebeg = 0;
    double timeend = 10000;
    double tdisplay = 3000;
    float pulse = 1000;
    double pulse_duration = 1e-5F;
    double pulse_display = 2e-5F;
    gpm_compute_interval test;
    test.add_timespan(timebeg, timeend);
    test.add_display_steps(timebeg, timeend, tdisplay);
    test.add_unsteady_timespans(timebeg, pulse, pulse_duration, pulse_display);
    auto t = test.get_two_level_list();
    int sum = 0;
    std::vector<double> times;
    for( auto it : test.intervals) {
        if ( it.second.sum[gpm_compute_interval::counter::Events::display_step] ) {
            times.push_back(it.first.upper());
            sum += 1;
        }
    }

    gpm_compute_interval test1;
    test1.add_timespan(timebeg, timeend);
    test1.add_display_steps(timebeg, timeend, tdisplay);
    test1.add_unsteady_timespans(timebeg, pulse, pulse_duration, pulse_duration);
    auto t1 = test1.get_two_level_list();
    int sum1 = 0;
    std::vector<double> times1;
    for (auto it : test1.intervals) {
        if (it.second.sum[gpm_compute_interval::counter::Events::display_step]) {
            times1.push_back(it.first.upper());
            sum1 += 1;
        }
    }
    gpm_compute_interval test2;
    test2.add_timespan(timebeg, timeend);
    test2.add_display_steps(timebeg, timeend, tdisplay);
    test2.add_unsteady_timespans(timebeg, pulse, pulse_duration, pulse_duration*0.45);
    auto t2 = test2.get_two_level_list();
    int sum2 = 0;
    std::vector<double> times2;
    for (auto it : test2.intervals) {
        if (it.second.sum[gpm_compute_interval::counter::Events::display_step]) {
            times2.push_back(it.first.upper());
            sum2 += 1;
        }
    }
    return 0;
}


int timeline_extremal_test(int argc, char* argv[])
{
    //double timebeg = 0;
    //double timeend = 10000;
    //std::vector<float> x= { 0,1000,4000,5000,5600,7000,9000 };
    //std::vector<float> y= { 100,150,110, 130, 120, 180, 179 };
    //float pulse = 1000;
    //float pulse_duration = 1e-5F;
    //float pulse_display = 2e-5F;
    //gpm_compute_interval test;
    //test.add_timespan(timebeg, timeend);
    //std::vector<float> extremes;
    //na_DiscreteEventFindAllExtremes(x, y, true, &extremes);
    //na_DiscreteEventFindAllExtremes(x, y, false, &extremes);
    //if (!extremes.empty()) {
    //    std::vector<double> exts(extremes.begin(), extremes.end());
    //    test.add_display_steps(exts);
    //}
    return 0;
}

int timeline_infinite_test(int argc, char* argv[]) {
    double timebeg = -std::numeric_limits<float>::infinity();
    double timeend = std::numeric_limits<float>::infinity();
    auto inf = boost::icl::interval<double>::right_open(timebeg, timeend);
    auto test = inf & boost::icl::interval<double>::right_open(0, 100);
    return 0;
}

int time_event_tests(int argc, char* argv[])
{
    std::bitset<5> sum, sum1;
    sum.set(0);
    sum1.set(1);
    auto res = sum | sum1;
	std::bitset<3> test(4);
	std::bitset<3> other = test | std::bitset<3>(2);
    return 0;

}