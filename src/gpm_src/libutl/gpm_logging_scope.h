// -- Schlumberger Private --

#ifndef GPM_LOGGING_SCOPE_H
#define GPM_LOGGING_SCOPE_H

#include "gpm_logger.h"
#include <string>

namespace Slb { namespace Exploration { namespace Gpm { namespace Tools {

class gpm_logging_scope {
public:
    gpm_logging_scope(const gpm_logger& logger, const char* function_scope, LogLevel level = LOG_DEBUG): _logger(logger), _level(level) {
        _report = _logger.level() >= _level;
        if (_report) {
            _text = std::string(function_scope);
            _logger.print(_level, "Entering %s scope\n", _text.c_str());
        }
    }

    gpm_logging_scope(const gpm_logger& logger, const std::string& function_scope, LogLevel level = LOG_DEBUG): _logger(logger), _level(level) {
        _report = _logger.level() >= _level;
        if (_report) {
            _text = function_scope;
            _logger.print(_level, "Entering %s scope\n", _text.c_str());
        }
    }

    ~gpm_logging_scope() {
        if (_report) {
            _logger.print(_level, "Leaving %s scope\n", _text.c_str());
        }
    }

private:
    gpm_logging_scope(const gpm_logging_scope& rhs);
    gpm_logging_scope& operator=(const gpm_logging_scope& rhs);
    const gpm_logger& _logger;
    std::string _text;
    LogLevel _level;
    bool _report; //logger.level() >= msglevel
};

}}}}
#endif
