#ifndef acf_hdf5_handler_h
#define acf_hdf5_handler_h

#include "hdf5.h"
#include <memory>
#include <string>
namespace Slb { namespace Exploration { namespace Gpm {

struct hdf5_unique_handler {
    typedef herr_t (*close_type)(hid_t handle); // Asssume the call type
    hdf5_unique_handler()=default;
    hdf5_unique_handler(hid_t handle, close_type close_func) : tester(handle, handle_deleter(close_func))
    {
    }

    hdf5_unique_handler(const hdf5_unique_handler& rhs) = delete;
    hdf5_unique_handler& operator=(const hdf5_unique_handler& rhs) = delete;

    hdf5_unique_handler& operator=(hdf5_unique_handler&& r) noexcept
    {
        this->tester = std::move(r.tester);
        return *this;
    }
     hdf5_unique_handler(hdf5_unique_handler&& rhs) = default;
    ~hdf5_unique_handler() = default;
    explicit operator bool() const { return static_cast<bool>(tester); }
    hid_t operator*() const { return tester.get(); }
    hid_t release() {return tester.release();} // Now we get the ownership back
    void reset(hid_t id) noexcept {tester.reset(id);} // If we need to destruct and look at return value
    herr_t close_state() const {return tester.get_deleter().return_val;}


private:
    struct handle_deleter {
        class pointer {
            hid_t t;
        public:
            pointer(hid_t t) : t(t)
            {
            }
            pointer(std::nullptr_t = nullptr) : t(hid_t())
            {
            }

            explicit operator bool() const { return t != hid_t(); }
            friend bool operator ==(pointer lhs, pointer rhs) { return lhs.t == rhs.t; }
            friend bool operator !=(pointer lhs, pointer rhs) { return lhs.t != rhs.t; }
            operator hid_t() const { return t; }
        };
        handle_deleter(close_type cb):close_handle(cb), return_val(0){}
        handle_deleter():close_handle(nullptr), return_val(0){}
        void operator()(pointer handle)
        {
            if (handle && close_handle != nullptr) {
                return_val = close_handle(static_cast<hid_t>(handle));
            }
        }

        close_type close_handle;
        herr_t return_val;
    };

    std::unique_ptr<hid_t, handle_deleter> tester;
};

class hdf5_shared_handler
{
public:
	typedef herr_t(*close_type)(hid_t handle); // Asssume the call type
private:
	std::shared_ptr<void> _d;
	hid_t value() const
	{
		return static_cast<hid_t>(reinterpret_cast<intptr_t>(_d.get()));
	}
	struct handle_deleter {
		handle_deleter(close_type cb) :close_handle(cb), return_val(0) {}
		handle_deleter() :close_handle(nullptr), return_val(0) {}
		void operator()(void* handle)
		{
		    const auto val = (reinterpret_cast<intptr_t>(handle));
			if (val > 0 && close_handle != nullptr) {
				return_val = close_handle(static_cast<hid_t>(val));
			}
		}

		close_type close_handle;
		herr_t return_val;
	};
public:
	hdf5_shared_handler() = default;
	hdf5_shared_handler(hid_t handle, close_type close_func)
	{
		_d = std::shared_ptr<void>(reinterpret_cast<void*>(static_cast<intptr_t>(handle)), handle_deleter(close_func)); 
	}
	hid_t operator*() const { return value(); }
	explicit operator bool() const { return value() > 0; }

    herr_t close_state() const
    {
        const auto del_p = std::get_deleter<void(*)(void*)>(_d);
        if (del_p != nullptr) {
            return reinterpret_cast<handle_deleter*>(del_p)->return_val;
        }
		return herr_t();
    }
};

class H5F_handler:public hdf5_shared_handler {
public:
	H5F_handler(hid_t handle):hdf5_shared_handler(handle, &H5Fclose){}
};

class H5S_handler :public hdf5_shared_handler {
public:
	H5S_handler(hid_t handle) :hdf5_shared_handler(handle, &H5Sclose) {}
};
class H5T_handler :public hdf5_shared_handler {
public:
	H5T_handler(hid_t handle) :hdf5_shared_handler(handle, &H5Tclose) {}
};
class H5D_handler :public hdf5_shared_handler {
public:
	H5D_handler(hid_t handle) :hdf5_shared_handler(handle, &H5Dclose) {}
};
class H5A_handler :public hdf5_shared_handler {
public:
	H5A_handler(hid_t handle) :hdf5_shared_handler(handle, &H5Aclose) {}
};
class H5O_handler :public hdf5_shared_handler {
public:
	H5O_handler(hid_t handle) :hdf5_shared_handler(handle, &H5Oclose) {}
};
}}}
#endif