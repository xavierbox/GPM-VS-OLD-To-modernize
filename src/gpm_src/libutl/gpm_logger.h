// -- Schlumberger Private --

#ifndef GPM_LOGGER_H
#define GPM_LOGGER_H
#include <iosfwd>

namespace Slb { namespace Exploration { namespace Gpm { namespace Tools {

enum LogLevel { LOG_IMPORTANT=0, LOG_NORMAL, LOG_VERBOSE, LOG_DEBUG, LOG_TRACE };

/**
 * Interface to handle logging and progress reporting.
 */
class gpm_logger {
public:
    virtual ~gpm_logger();
    virtual LogLevel level() const = 0;
    virtual bool progress(float percent) const = 0;
    virtual void print(LogLevel level, const char* format, ...) const = 0;
    virtual bool confirm() const = 0;
    virtual void flush() const = 0;
};

/**
 * Default logger, prints messages to stdout.
 */
class gpm_default_logger : public gpm_logger {
    class Impl;
    Impl* pimpl_;
    gpm_default_logger(const gpm_default_logger& rhs) ;
    gpm_default_logger& operator=(const gpm_default_logger& rhs) ;
public:
    gpm_default_logger(LogLevel level, bool always_continue);
    gpm_default_logger(LogLevel level, bool always_continue, const char* filename);
    virtual ~gpm_default_logger();
    LogLevel level() const override;
    bool progress(float percent) const override;
    void print(LogLevel level, const char* format, ...) const override;
    bool confirm() const override;
    void flush() const override;
};

/**
 * Default logger, prints messages to stringstream.
 */
class gpm_strstream_logger : public gpm_logger {
	class Impl;
	Impl* pimpl_;
	gpm_strstream_logger(const gpm_strstream_logger& rhs);
	gpm_strstream_logger& operator=(const gpm_strstream_logger& rhs);
public:
	gpm_strstream_logger(LogLevel level, bool always_continue, std::stringstream& stream);
	virtual ~gpm_strstream_logger();
	LogLevel level() const override;
	bool progress(float percent) const override;
	void print(LogLevel level, const char* format, ...) const override;
	bool confirm() const override;
	void flush() const override;
};
/**
 * Buffer messages. Specialize this for GUI pop-up dialogs.
 * All messages declared as const, even though the instance
 * is technically changed when buffering data.
 */
class gpm_buffered_logger : public gpm_logger {
private:
    class Impl;
    Impl* pimpl_;
    gpm_buffered_logger(const gpm_buffered_logger& rhs) ;
    gpm_buffered_logger& operator=(const gpm_buffered_logger& rhs) ;
public:
    enum Action { AskUser, AnswerNo, AnswerYes };

    gpm_buffered_logger(LogLevel level, bool popup_ok); // deprecated
    gpm_buffered_logger(LogLevel level, Action action = AskUser);
    virtual ~gpm_buffered_logger();
    // Methods from SegyLogger
    LogLevel level() const override;
    bool progress(float percent) const override;
    void print(LogLevel level, const char* format, ...) const override;
    bool confirm() const override;
    void flush() const override;
    // New virtual methods
    virtual void popup(const char* message) const;
    virtual bool isyes(const char* message) const;
    virtual void logit(const char* message) const;
    // New non-virtual methods
    const char* message() const;
    void clear() const;
    void setLogLevel(LogLevel level);
    void setAction(Action action);
};

class gpm_memory_monitor {
	class Impl;
	Impl* pimpl_;
	gpm_memory_monitor(const gpm_memory_monitor& rhs);
	gpm_memory_monitor& operator=(const gpm_memory_monitor& rhs);
public:
	gpm_memory_monitor(unsigned int c, unsigned int p);
	~gpm_memory_monitor();

	void sampleMemoryHeap() const;
	unsigned int getHeapSizeCurrent() const;
	unsigned int getHeapSizePeak() const;
};

}}}}

#endif
