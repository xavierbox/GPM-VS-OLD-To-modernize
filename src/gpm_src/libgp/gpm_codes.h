#ifndef GPM_CODES_H
#define GPM_CODES_H

namespace Slb { namespace Exploration { namespace Gpm {

enum GPM_USER_ERROR_TYPES {
    GPM_OK = 0,
    GPM_CTL_FILE_MISSING = 2,
    GPM_RUNTIME_LIMIT = 3,
    GPM_NUMERICAL_UNSTABILITY,
    GPM_TIME_BOMB,
    GPM_EXCEPTION_THROWN
};

// These are the ones that the user can do stuff with
// Need the file missing thing to move checking outside
// I hope

}}}
#endif
