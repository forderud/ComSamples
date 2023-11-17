#pragma once
#include <mutex>
#include <atlbase.h>


/** Marshal a COM interface pointer into a stream.
    Intended for transport of COM interface pointers between threads in different apartments within the same process. */
template <typename T>
class ComMarshal {
public:
    ComMarshal() {
    }

    ~ComMarshal() {
        Clear_internal();
    }

    /** Marshal ptr for access by a thread in a difference COM apartment.
        ptr must be accessible by the current thread/apartment or nullptr to clear the object. */
    ComMarshal& operator = (T * ptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        Clear_internal();
        if (ptr) {
            // marshal pointer to be ready for threads that call Get()
            if (FAILED(RoGetAgileReference(AGILEREFERENCE_DEFAULT, __uuidof(T), ptr, &m_ptr)))
                abort(); // crash since failure is a sign of a bug and not a recoverable error
        }
        return *this;
    }

    /** Get COM interface ptr marshaled for the current thread. */
    CComPtr<T> Get() {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!m_ptr)
            return CComPtr<T>();

        // unmarshal pointer back to apartment for current thread
        CComPtr<T> ptr;
        if (FAILED(m_ptr->Resolve(&ptr)))
            abort(); // crash since failure is a sign of a bug and not a recoverable error
        return ptr;
    }

    // non-copyable
    ComMarshal(ComMarshal&) = delete;
    ComMarshal& operator = (ComMarshal&) = delete;

private:
    void Clear_internal() {
        m_ptr.Release();
    }

    std::mutex                  m_mutex;
    CComPtr<IAgileReference>    m_ptr;  ///< multi-apartment pointer
};
