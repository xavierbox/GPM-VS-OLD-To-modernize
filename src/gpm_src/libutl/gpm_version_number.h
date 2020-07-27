// -- Schlumberger Private --

#ifndef GPM_VERSION_NUMBER_H
#define GPM_VERSION_NUMBER_H

#include <string>

namespace Slb { namespace Exploration { namespace Gpm { namespace Tools {
class gpm_version_number {
public:
    gpm_version_number();
    explicit gpm_version_number(const std::string& ver);
    gpm_version_number(int major, int minor, int revision, int build);
    gpm_version_number(int major, int minor);
    ~gpm_version_number();
    int get_major() const;
    int get_minor() const;
    int get_revision() const;
    int get_build() const;
    bool is_number_correct() const;
    std::string to_string();
    bool version_equal(const gpm_version_number& rhs) const;
    bool version_compatible(const gpm_version_number& rhs) const;
private:
    std::string version_string;
    int _major;
    int _minor;
    int _revision;
    int _build;
    bool _correct;
};

bool operator==(const gpm_version_number& lhs, const gpm_version_number& rhs);
bool operator!=(const gpm_version_number& lhs, const gpm_version_number& rhs);
bool operator>=(const gpm_version_number& lhs, const gpm_version_number& rhs);
bool operator>(const gpm_version_number& lhs, const gpm_version_number& rhs);
bool operator<(const gpm_version_number& lhs, const gpm_version_number& rhs);

}}}}
#endif
