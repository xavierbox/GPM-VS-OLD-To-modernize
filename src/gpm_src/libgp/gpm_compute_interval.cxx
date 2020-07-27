// -- Schlumberger Private --

#include "gpm_compute_interval.h"
#include <boost/icl/continuous_interval.hpp>
#include <boost/icl/split_interval_map.hpp>

namespace Slb { namespace Exploration { namespace Gpm {
//Empty interval with right open policy
gpm_compute_interval::time_interval::time_interval(): lower(0.0), upper(0.0) {
}

gpm_compute_interval::time_interval::time_interval(double low, double up): lower(low), upper(up) {
}

double gpm_compute_interval::time_interval::length() const {
    return upper - lower;
}

namespace { // Anonymous namespace to keep these local

gpm_compute_interval::display_event_interval_descr_type
convert_to_simple_interval_description(boost::icl::split_interval_map<double, gpm_compute_interval::counter>& loc_map) {
    gpm_compute_interval::display_event_interval_descr_type res;
    // Lets find all the display times
    boost::icl::interval_set<double> time_sum;
    gpm_compute_interval::compute_event_interval_descr_type list_sum;
    for (auto tmp = loc_map.begin(); tmp != loc_map.end(); ++tmp) {
        time_sum += (*tmp).first;
        list_sum.push_back(*tmp);
        if (tmp->second.sum[gpm_compute_interval::counter::Events::display_step]) {
            if (tmp->second.sum[gpm_compute_interval::counter::Events::unsteady_start]) {
                // Need to remove the start and add to next list
                // Since start needs to be on the "righthand side of display
                list_sum.back().second.sum[gpm_compute_interval::counter::Events::unsteady_start] = false;
                res.push_back(std::make_pair(*time_sum.begin(), list_sum));
                auto corr = *tmp;
                list_sum.clear();
                time_sum.clear();
                time_sum += corr.first;
                corr.second.sum[gpm_compute_interval::counter::Events::display_step] = false;
                list_sum.push_back(corr);
            }
            else {
                res.push_back(std::make_pair(*time_sum.begin(), list_sum));
                list_sum.clear();
                time_sum.clear();
            }
        }
    }
    return res;
}

} // End anonymous namespace


void gpm_compute_interval::add_timespan(double timebeg, double timeend)
{
    auto span = boost::icl::interval<double>::right_open(timebeg, timeend);
    base_span = span;
    intervals += boost::icl::split_interval_map<double, counter>(std::make_pair(span, counter(counter::steady)));
    auto display = boost::icl::interval<double>::closed(timeend, timeend);
    intervals += boost::icl::split_interval_map<double, counter>(std::make_pair(display, counter(counter::display_step)));
}
void gpm_compute_interval::add_display_steps(double timebeg, double timeend, double tdisplay)
{
    const auto span = boost::icl::interval<double>::right_open(timebeg, timeend);
    if (boost::icl::intersects(span, base_span)) {
        auto new_span = base_span & span;
        auto start = new_span.lower();
        const auto end_t = new_span.upper();
        const auto time_offset = 0.005 * tdisplay;
        while (start + tdisplay < end_t) {
            start += tdisplay;
            if (start >= end_t - time_offset) {
                start = end_t;
            }
            auto display = boost::icl::interval<double>::closed(start, start);
            intervals += boost::icl::split_interval_map<double, counter>(std::make_pair(display, counter(counter::display_step)));
        }
    }
}

void gpm_compute_interval::add_display_steps(const std::vector<double>& display)
{
    for(auto it : display) {
        if (boost::icl::contains(base_span, it)) {
            auto loc_display = boost::icl::interval<double>::closed(it, it);
            intervals += boost::icl::split_interval_map<double, counter>(std::make_pair(loc_display, counter(counter::display_step)));
        }
    }
}

void gpm_compute_interval::add_unsteady_timespans(double timebeg, double unsteady_pulse, double unsteady_duration_time, double unsteady_display_time)
{
    auto start = timebeg;
    while (start + unsteady_duration_time < base_span.upper()) {
        // Add the thing
        auto end_time = start + unsteady_duration_time;
        auto span = boost::icl::interval<double>::right_open(start, end_time);
        intervals += boost::icl::split_interval_map<double, counter>(std::make_pair(span, counter(counter::unsteady)));
        auto start_u = boost::icl::interval<double>::closed(start, start);
        auto end_u = boost::icl::interval<double>::closed(end_time, end_time);
        intervals += boost::icl::split_interval_map<double, counter>(std::make_pair(start_u, counter(counter::unsteady_start)));
        intervals += boost::icl::split_interval_map<double, counter>(std::make_pair(end_u, counter(counter::unsteady_end)));
        if ( unsteady_display_time <= unsteady_duration_time) {
            // Add display times
            auto display_time = start + unsteady_display_time;
            while (display_time < end_time) {
                auto display = boost::icl::interval<double>::closed(display_time, display_time);
                intervals += boost::icl::split_interval_map<double, counter>(std::make_pair(display, counter(counter::display_step)));
                display_time += unsteady_display_time;
            }
            auto display = boost::icl::interval<double>::closed(end_time, end_time);
            intervals += boost::icl::split_interval_map<double, counter>(std::make_pair(display, counter(counter::display_step)));
        }
        start += unsteady_pulse;
    }
}

std::vector<double> gpm_compute_interval::get_display_times() const
{
    std::vector<double> res;
    for (auto it : intervals) {
        if (it.second.sum[gpm_compute_interval::counter::Events::display_step]) {
            res.push_back(it.first.upper());
        }
    }
    return res;
}

gpm_compute_interval::display_event_interval_descr_type gpm_compute_interval::get_two_level_list()
{
    return convert_to_simple_interval_description(intervals);
}

bool operator==(const gpm_compute_interval::counter& lhs, const gpm_compute_interval::counter& rhs)
{
    return rhs.sum == lhs.sum;
}

}}}
