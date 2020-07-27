// -- Schlumberger Private --

#include "gpm_logger.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <stdexcept>
#include <cstdarg>
#include <vector>
#include <sstream>
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#endif


namespace Slb { namespace Exploration { namespace Gpm { namespace Tools {
namespace {
void nullterminate_string(char* arr)
{
	arr[sizeof(arr) - 1] = '\0';
}
}

gpm_logger::~gpm_logger(){}


class gpm_default_logger::Impl {
public:
    LogLevel level_;
    FILE* logfile_;
    bool continue_;

    Impl(LogLevel level, const char* filename, bool always_continue)
        : level_(level)
          , logfile_(stderr)
          , continue_(always_continue) {
        if (filename && *filename) {
            FILE* f = fopen(filename, "a");
            if (f == nullptr)
                throw std::runtime_error(std::string("Cannot open log file ") + filename);
            logfile_ = f;
        }
    }

    ~Impl() {
        fflush(logfile_);
        if (logfile_ != stderr) {
            fclose(logfile_);
        }
    }

    void flush() {
        fflush(logfile_);
    }
};

gpm_default_logger::gpm_default_logger(LogLevel level, bool always_continue)
    : pimpl_(new Impl(level, nullptr, always_continue)) {
}

gpm_default_logger::gpm_default_logger(LogLevel level, bool always_continue, const char* filename)
    : pimpl_(new Impl(level, filename, always_continue)) {
}

gpm_default_logger::~gpm_default_logger() {
    delete pimpl_;
}

/**
 * Only message of this importance and higher
 * (i.e. lower numbered) should be displayed.
 */
LogLevel
gpm_default_logger::level() const {
    return pimpl_->level_;
}

/**
 * In a GUI environment, update the progress bar with the specified percentage.
 * In a command line environment, print a single dot. The code will call
 * progress() roughly each time 1% more work has been done.
 */
bool
gpm_default_logger::progress(float /*percent*/) const {
    if (level() >= LOG_NORMAL) {
        fprintf(pimpl_->logfile_, ".");
        fflush(pimpl_->logfile_);
    }
    return true; // continue executing
}

/**
 * Display a message in a log file or on the console,
 * if it is deemed important enough.
 *
 * In Petrel, it is suggested to buffer all LOG_MPORTANT
 * messages until a confirm() or flush() is received,
 * then display it with with message() or isYes().
 * Lower priority messages should print with messageLog().
 *
 * If the user asked that the operation should be done
 * in unattended mode, all output should go to the log.
 */
void
gpm_default_logger::print(LogLevel msglevel, const char* format, ...) const {
    if (level() >= msglevel) {
        va_list ap;
        va_start(ap, format);
        vfprintf(pimpl_->logfile_, format, ap);
        va_end(ap);
    }
}

/**
 * Ask the user for whether it is Ok to continue.
 * What the application does, depends on what mode it is in:
 *
 * If problems: 
 *   - Stop and ask me
 *   - Silently skip this file
 *   - Continue (might crash)
 *
 * If the application has a gui, 'stop and ask' should pop up
 * a question dialog containing all the LOG_IMPORTANT
 * messages received up till now, plus "\nDo you want to continue?".
 * The command line application simply returns true or false
 * depending on options given at startup.
 */
bool
gpm_default_logger::confirm() const {
    return pimpl_->continue_;
}

/**
 * Ensure all messages have been output.
 * If we don't buffer messages, we don't need to do anything here.
 * Applications with a GUI should buffer all LOG_IMPORTANT messages
 * so they can be presented to the user in a single pop-up.
 */
void
gpm_default_logger::flush() const {
    pimpl_->flush();
}

class gpm_strstream_logger::Impl {
public:
	LogLevel level_;
	std::stringstream& logfile_;
	bool continue_;

	Impl(LogLevel level, std::stringstream& logger, bool always_continue)
		: level_(level)
		, logfile_(logger)
		, continue_(always_continue) {
	}

	~Impl() {
	}
};
gpm_strstream_logger::gpm_strstream_logger(LogLevel level, bool always_continue, std::stringstream& stream):pimpl_(new Impl(level, stream, always_continue))
{
}

gpm_strstream_logger::~gpm_strstream_logger()
{
	delete pimpl_;
}

LogLevel gpm_strstream_logger::level() const
{
	return pimpl_->level_;
}

bool gpm_strstream_logger::progress(float percent) const
{
	if (level() >= LOG_NORMAL) {
		pimpl_->logfile_ << '.';
	}
	return true;
}

void gpm_strstream_logger::print(LogLevel level, const char* format, ...) const
{
	// initialize use of the variable argument array
	va_list vaArgs;
	va_start(vaArgs, format);

	// reliably acquire the size from a copy of
	// the variable argument array
	// and a functionally reliable call
	// to mock the formatting
	va_list vaCopy;
	va_copy(vaCopy, vaArgs);
	const int iLen = std::vsnprintf(NULL, 0, format, vaCopy);
	va_end(vaCopy);

	// return a formatted string without
	// risking memory mismanagement
	// and without assuming any compiler
	// or platform specific behavior
	std::vector<char> zc(iLen + 1);
	std::vsnprintf(zc.data(), zc.size(), format, vaArgs);
	va_end(vaArgs);
	std::string res(zc.data(), zc.size());
	pimpl_->logfile_ << res;
}

bool gpm_strstream_logger::confirm() const
{
	return pimpl_->continue_;
}

void gpm_strstream_logger::flush() const
{
}


class gpm_buffered_logger::Impl {
public:
    LogLevel level_;
    Action action_;
    char message_[4096];
    char tmp_message_[4096];
    int max_message_size_;
    bool message_overflow_;

    Impl(LogLevel level, Action action)
        : level_(level)
          , action_(action) {
        clear();
    }

    ~Impl(){}

    void clear() {
        message_[0] = '\0';
        tmp_message_[0] = '\0';
        // Leave space for "Overflow" and "Continue?" at end.
        max_message_size_ = sizeof(message_) - 200;
        message_overflow_ = false;
    }
};

gpm_buffered_logger::gpm_buffered_logger(LogLevel level, bool popup_ok)
    : pimpl_(new Impl(level, popup_ok ? AskUser : AnswerNo)) {
}

gpm_buffered_logger::gpm_buffered_logger(LogLevel level, Action action)
    : pimpl_(new Impl(level, action)) {
}

gpm_buffered_logger::~gpm_buffered_logger() {
    delete pimpl_;
}

/**
 * Only message of this importance and higher
 * (i.e. lower numbered) should be displayed.
 */
LogLevel
gpm_buffered_logger::level() const {
    return pimpl_->level_;
}

/**
 * In a GUI environment, this ought to be overridden.
 */
bool
gpm_buffered_logger::progress(float /*percent*/) const {
    return true; // continue executing
}

/**
 * Display a message in a log file or on the console,
 * if it is deemed important enough.
 *
 * In Petrel, it is suggested to buffer all LOG_MPORTANT
 * messages until a confirm() or flush() is received,
 * then display it with with message() or isYes().
 * Lower priority messages should print with messageLog().
 *
 * If the user asked that the operation should be done
 * in unattended mode, all output should go to the log.
 */
void
gpm_buffered_logger::print(LogLevel msglevel, const char* format, ...) const {
    // TODO lock to make thread safe, in all the methods.
    if (format[0] == '\0')
        return;
    if (level() >= msglevel) {
        va_list ap;
        va_start(ap, format);
#ifdef WIN32
        if (vsprintf_s(pimpl_->tmp_message_, format, ap) <= 0)
            strncpy(pimpl_->tmp_message_, "(Error formatting message)", sizeof(pimpl_->tmp_message_) - 1);
#else
    if (vsnprintf(pimpl_->tmp_message_, sizeof(pimpl_->tmp_message_), format, ap) <= 0)
      strncpy(pimpl_->tmp_message_, "(Error formatting message)", sizeof(pimpl_->tmp_message_) - 1);
#endif
        nullterminate_string(pimpl_->tmp_message_);
        va_end(ap);

        // Any \n in the message was inserted to make it look good
        // when displayed in a 80-column terminal window. This is
        // not a good idea in pop-ups or in the log window which
        // do their own wrapping.
        char* cp = pimpl_->tmp_message_;
        for (; *cp != '\0'; ++cp)
            if (*cp == '\n')
                *cp = ' ';
        while (cp > pimpl_->tmp_message_ && cp[-1] == ' ')
            *--cp = '\0';
        size_t tmp_message_size = strlen(pimpl_->tmp_message_);
        if (tmp_message_size == 0)
            return;
        if (pimpl_->action_ == AskUser && msglevel == LOG_IMPORTANT) {
            // Buffer the message for later pop-up.
            // Multiple messages separated by a blank line.
            if (pimpl_->message_overflow_) {
                // already warned about overflow.
            }
            else if (tmp_message_size > pimpl_->max_message_size_) {
                pimpl_->message_overflow_ = true;
                strncat(pimpl_->message_, "\n\nRemaining messages not printed.", sizeof(pimpl_->message_)-1-strlen(pimpl_->message_));
				nullterminate_string(pimpl_->message_);
            }
            else {
                if (pimpl_->message_[0] != '\0')
                    strncat(pimpl_->message_, "\n\n", sizeof(pimpl_->message_) - 1 - strlen(pimpl_->message_));
                strncat(pimpl_->message_, pimpl_->tmp_message_, sizeof(pimpl_->message_) - 1 - strlen(pimpl_->message_));
				nullterminate_string(pimpl_->message_);
                pimpl_->max_message_size_ -= static_cast<int>(tmp_message_size + 2U);
            }
        }
        else {
            // Output the message immediately to the log window.
            logit(pimpl_->tmp_message_);
        }
    }
}

/**
 * Ask the user for whether it is Ok to continue.
 * Any buffered LOG_IMPORTANT messages are displayed
 * along with the prompt.
 */
bool
gpm_buffered_logger::confirm() const {
    bool answer = false;
    if (pimpl_->action_ == AskUser) {
        char* cp = pimpl_->message_ + strlen(pimpl_->message_);
        strncpy(cp, "\n\nDo you want to continue?", sizeof(pimpl_->message_)-1);
		nullterminate_string(pimpl_->message_);
        answer = isyes(pimpl_->message_);
        // isyes() will normally call clear(), but just in case it didn't
        // we remove the extra prompt that was added above.
        if (pimpl_->message_[0] != '\0')
            *cp = '\0';
    }
    else {
        // Should not be needed, it should already be flushed.
        if (pimpl_->message_[0] != '\0') {
            logit(pimpl_->message_);
            pimpl_->clear();
        }
        answer = pimpl_->action_ == AnswerYes ? true : false;
    }
    return answer;
}

/**
 * Display any buffered messages now.
 * Note that flush() might be called even if no messages were posted.
 */
void
gpm_buffered_logger::flush() const {
    if (pimpl_->message_[0] != '\0') {
        switch (pimpl_->action_) {
        case AskUser:
            popup(pimpl_->message_);
            break;
        case AnswerNo:
        case AnswerYes:
            // Should never get here, it should already be flushed.
            logit(pimpl_->message_);
            pimpl_->clear();
            break;
        default:
            break;
        }
    }
}

/**
 * Override this method to pop up a message dialog,
 * then call clear() to acknowledge. If this is not
 * done then the message remains buffered and could
 * be retrieved using message() when all operations
 * are completed.
 */
void
gpm_buffered_logger::popup(const char* /*message*/) const {
}

/**
 * Override this method to pop up a question dialog,
 * then call clear() to acknowledge. If this is not
 * done then the message remains buffered and could
 * be retrieved using message() when all operations
 * are completed.
 */
bool
gpm_buffered_logger::isyes(const char* /*message*/) const {
    return false;
}

/**
 * Override this method to log the message.
 * If this is not done, the message is lost.
 */
void
gpm_buffered_logger::logit(const char* /*message*/) const {
}

/**
 * Retrieve the buffered but still not displayed message.
 * Useful if popup() and isyes() were not overridden,
 * so that messages remain in the buffer.
 */
const char*
gpm_buffered_logger::message() const {
    return pimpl_->message_;
}

/**
 * Acknowledge that a message has been retrieved and remove it.
 * popup() and isyes() should call this method after the
 * dialog is closed.
 */
void
gpm_buffered_logger::clear() const {
    pimpl_->clear();
}

void
gpm_buffered_logger::setLogLevel(LogLevel level) {
    pimpl_->level_ = level;
}

void
gpm_buffered_logger::setAction(Action action) {
    pimpl_->action_ = action;
    if (action != AskUser && pimpl_->message_[0] != '\0') {
        // Dump any previously queued-up message to the logger
        // since the user now doesn't want any pop-ups.
        logit(pimpl_->message_);
        clear();
    }
}

class gpm_memory_monitor::Impl {
public:

	Impl(unsigned int c, unsigned int p)
		: B2MB(1024.0*1024.0),
		  m_heapsize_current(c),
		  m_heapsize_peak(p)
		  
	{
		//printf(" gpm_memory_monitor:: Impl RESET\n");
	}

	~Impl() {}

	unsigned int getHeapSizeCurrent() const { return static_cast<unsigned int> (m_heapsize_current/B2MB); }
	unsigned int getHeapSizePeak   () const { return static_cast<unsigned int> (m_heapsize_peak/B2MB) ;  } 
	void sampleMemoryHeap()
	{
#ifdef WIN32
		DWORD processID = GetCurrentProcessId();

		HANDLE hProcess;
		PROCESS_MEMORY_COUNTERS pmc;

		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ,
			FALSE, processID);

		m_heapsize_peak = 0;
		m_heapsize_current = 0;
		if (hProcess != nullptr) {
			// get information about heap memory usage
			if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
			{
				m_heapsize_peak = (long long)pmc.PeakWorkingSetSize;
				m_heapsize_current = (long long)pmc.WorkingSetSize;
			}
		}

		CloseHandle(hProcess);
#else
		m_heapsize_peak = 0;
		m_heapsize_current = 0;

#endif // WIN32


	}

private:
	const double B2MB;
	long long m_heapsize_current;
	long long m_heapsize_peak;

};

gpm_memory_monitor::gpm_memory_monitor(unsigned int c, unsigned int p)
	: pimpl_(new Impl(c,p))
{}
gpm_memory_monitor::~gpm_memory_monitor()
{
	delete pimpl_;
}

void gpm_memory_monitor::sampleMemoryHeap() const {
	pimpl_->sampleMemoryHeap();
}
unsigned int gpm_memory_monitor::getHeapSizeCurrent() const {
	return pimpl_->getHeapSizeCurrent ();
}
unsigned int gpm_memory_monitor::getHeapSizePeak() const {
	return pimpl_->getHeapSizePeak();
}




}}}}
