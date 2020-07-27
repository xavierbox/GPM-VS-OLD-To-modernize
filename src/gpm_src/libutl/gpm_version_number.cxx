// -- Schlumberger Private --

#include "gpm_version_number.h"
#include "gpm_string_utils.h"
#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive.hpp>

namespace Slb { namespace Exploration { namespace Gpm { namespace Tools {
using namespace boost::xpressive;

namespace {
bool split_into_version_numbers(const std::string& input, int& major, int& minor, int& build, int& revision) {
    sregex rex = (s1 = +_d) >> "." >> (s2 = +_d) >> "." >> (s3 = +_d) >> "." >> (s4 = +_d);
    smatch what;
    bool retval = false;
    if (regex_match(input, what, rex)) {
        major = boost::lexical_cast<int>(what[1]);
        minor = boost::lexical_cast<int>(what[2]);
        build = boost::lexical_cast<int>(what[3]);
        revision = boost::lexical_cast<int>(what[4]);
        retval = true;
    }
    return retval;
}

}
gpm_version_number::gpm_version_number(): _major(1), _minor(0), _revision(0), _build(0), _correct(true) {

}


gpm_version_number::gpm_version_number(const std::string& ver) {
    if (!ver.empty()) {
        _correct = split_into_version_numbers(ver, _major, _minor, _revision, _build);
    }
    else {
        _major = 1;
        _minor = _build = _revision = 0;
        _correct = true;
    }
}

gpm_version_number::gpm_version_number(int major, int minor, int revision, int build): _major(major), _minor(minor), _revision(revision), _build(build) {
    _correct = _major >= 0 && _minor >= 0 && _revision >= 0 && _build >= 0;
}

gpm_version_number::gpm_version_number(int major, int minor): _major(major), _minor(minor), _revision(0), _build(0) {
    _correct = _major >= 0 && _minor >= 0 && _revision >= 0 && _build >= 0;
}

gpm_version_number::~gpm_version_number(){}

int gpm_version_number::get_major() const {
    return _major;
}

int gpm_version_number::get_minor() const {
    return _minor;
}

int gpm_version_number::get_revision() const {
    return _revision;
}

int gpm_version_number::get_build() const {
    return _build;
}

std::string gpm_version_number::to_string() {
    std::string res;
    if (is_number_correct()) {
        res = std::to_string(_major) + "." + std::to_string(_minor) + "." + std::to_string(_revision) + "." + std::to_string(_build);
    }
    return res;
}

bool gpm_version_number::version_equal(const gpm_version_number& rhs) const {
    return _correct && rhs._correct && _major == rhs._major && _minor == rhs._minor;
}

bool gpm_version_number::version_compatible(const gpm_version_number& rhs) const {
    return _correct && rhs._correct && _major == rhs._major;
}

bool operator==(const gpm_version_number& lhs, const gpm_version_number& rhs) {
    return lhs.version_equal(rhs);
}

bool operator!=(const gpm_version_number& lhs, const gpm_version_number& rhs) {
    return !operator==(lhs, rhs);
}

bool operator>=(const gpm_version_number& lhs, const gpm_version_number& rhs) {
    return lhs == rhs || lhs > rhs;
}

bool operator>(const gpm_version_number& lhs, const gpm_version_number& rhs) {
    return lhs.get_major() > rhs.get_major() || lhs.get_minor() > rhs.get_minor() || lhs.get_revision() > rhs.get_revision() || lhs.get_build() > rhs.get_build();
}

bool operator<(const gpm_version_number& lhs, const gpm_version_number& rhs) {
    return lhs.get_major() < rhs.get_major() || lhs.get_minor() < rhs.get_minor() || lhs.get_revision() < rhs.get_revision() || lhs.get_build() < rhs.get_build();
}

bool gpm_version_number::is_number_correct() const {
    return _correct;
}

}}}}
