// -- Schlumberger Private --

#ifndef GPM_COMPUTE_INTERVAL_H
#define GPM_COMPUTE_INTERVAL_H


#include <boost/icl/continuous_interval.hpp>
#include <vector>
#include <utility> // For pair
#include <bitset>
#include <boost/icl/split_interval_map.hpp>

namespace Slb { namespace Exploration { namespace Gpm {


class gpm_compute_interval {
public:
    enum COMPUTE_INTERVAL_TYPE { STEADY=1, UNSTEADY=2 };

    struct counter {
        // These are positions in the bitset telling what is on/off
        enum Events { steady = 0, unsteady, display_step, unsteady_start, unsteady_end, num_elems };
        counter()
        = default;

        explicit counter(Events i)
        {
            sum.set(i);
        }

        counter& operator+=(const counter& rhs)
        {
            sum |= rhs.sum;
            return *this;
        }

        std::bitset<num_elems> sum;
    };

    struct time_interval {
        double lower;
        double upper;
        time_interval();
        time_interval(double low, double up);
        double length() const;
    };

    typedef time_interval unit_interval_type;
    typedef std::pair<unit_interval_type, COMPUTE_INTERVAL_TYPE> unit_interval_descr_type;
    // From-to span, interval type
    // The outer vector keeps the number of display steps
    // The innervector keeps tyhe number of processes to run in each display step
    typedef std::vector<unit_interval_descr_type> compute_interval_descr_type;
    typedef std::vector<compute_interval_descr_type> display_interval_descr_type; // 

    // New types
    // Inner loop is for the events that take place in a display loop
    typedef std::pair<boost::icl::continuous_interval<double>, counter>  unit_event_interval_decr_type;
    // These events will be run in a single display loop
    typedef std::vector<unit_event_interval_decr_type> compute_event_interval_descr_type;
    // The vector holding all the display times
    typedef std::vector<std::pair<boost::icl::continuous_interval<double>,compute_event_interval_descr_type>> display_event_interval_descr_type;

    // Our new code as a list of events

    void add_timespan(double timebeg, double timeend);
    void add_display_steps(double timebeg, double timeend, double tdisplay);
    void add_display_steps(const std::vector<double>& display);
    void add_unsteady_timespans(double timebeg, double unsteady_pulse, double unsteady_duration_time, double unsteady_display_time);
    std::vector<double> get_display_times() const;
    display_event_interval_descr_type get_two_level_list();
    boost::icl::split_interval_map<double, counter> intervals; // Here is where we accumulate all
    boost::icl::continuous_interval<double> base_span;
};

bool operator==(const gpm_compute_interval::counter& lhs, const gpm_compute_interval::counter& rhs);
}}}
#endif
