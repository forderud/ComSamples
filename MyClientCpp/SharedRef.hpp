#pragma once
#include <cassert>


/** COM wrapper class that provides support for weak references through the IWeakRef interface. */
template <class Interface, class Class>
class SharedRef : public IUnknown {
    static constexpr ULONG EXPIRED_STRONG = 1;
public:
    SharedRef() : m_child(*this) {
        wprintf(L"SharedRef ctor\n");
        CComAggObject<Class>::CreateInstance(this, &m_ptr);
        assert(m_ptr);
        m_ptr->AddRef();
        m_refs.AddRef(true);
        ++s_obj_count;
    }
    ~SharedRef() {
        wprintf(L"SharedRef dtor\n");
        assert(m_ptr == nullptr);
        --s_obj_count;
    }

    /** QueryInterface doesn't adhere to the COM rules, since the interfaces accessible are dynamic.
        Still, interface disappearance only affect internal IWeakRef clients, so it shouldn't cause problems for external code.
        DOC: https://learn.microsoft.com/en-us/windows/win32/com/rules-for-implementing-queryinterface */
    HRESULT QueryInterface(const IID& iid, void** ptr) override {
        if (!ptr)
            return E_INVALIDARG;

        *ptr = nullptr;
        if (iid == IID_IUnknown) {
            if (!m_ptr)
                return E_NOT_SET;

            // strong reference to this
            *ptr = static_cast<IUnknown*>(this);
            m_refs.AddRef(true);
            return S_OK;
        } else if (iid == __uuidof(IWeakRef)) {
            // weak reference to child oject
            *ptr = static_cast<IWeakRef*>(&m_child);
            m_refs.AddRef(false);
            return S_OK;
        } else if (iid == __uuidof(Interface)) {
            if (!m_ptr)
                return E_NOT_SET;

            // strong reference pointed-to object
            *ptr = static_cast<Interface*>(&m_ptr->m_contained);
            m_refs.AddRef(true);
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG AddRef() override {
        return m_refs.AddRef(true);
    }

    ULONG Release() override {
        RefBlock refs = m_refs.Release(true);

        if (m_ptr && (refs.strong == EXPIRED_STRONG)) {
            // release object handle
            m_ptr->Release(); // will trigger reentrancy
            m_ptr = nullptr;
        }

        if ((refs.strong == EXPIRED_STRONG) && !refs.weak)
            delete this;

        return refs.strong;
    }

    static ULONG ObjectCount() {
        return s_obj_count;
    }

private:
    /** Inner class for managing weak references. */
    class WeakRef : public IWeakRef {
        friend class SharedRef;
    public:
        WeakRef(SharedRef& parent) : m_parent(parent) {
            wprintf(L"WeakRef ctor\n");
        }
        ~WeakRef() {
            wprintf(L"WeakRef dtor\n");
        }

        /** QueryInterface doesn't adhere to the COM aggregation rules for inner objects, but this is a project-internal pointer that's not shared with anyone.
            DOC: https://learn.microsoft.com/en-us/windows/win32/com/aggregation */
        HRESULT QueryInterface(const IID& iid, void** obj) override {
            return m_parent.QueryInterface(iid, obj);
        }

        ULONG AddRef() override {
            return m_parent.m_refs.AddRef(false);
        }

        ULONG Release() override {
            RefBlock refs = m_parent.m_refs.Release(false);
            if ((refs.strong <= EXPIRED_STRONG) && !refs.weak) // delete both if strong is 0 or 1
                delete &m_parent;

            return refs.weak;
        }

        /** Resolve a strong object reference. */
        HRESULT Resolve(IUnknown** ptr) override {
            if (!m_parent.m_refs.AddRefIfStrongValid())
                return E_FAIL;

            *ptr = &m_parent;
            return S_OK;
        }

    private:
        SharedRef& m_parent;
    };

    struct RefBlock {
        uint32_t strong = 0; // strong use-count for m_ptr lifetime
        uint32_t weak = 0;   // weak ref-count for SharedRef lifetime
    };
    /** Thread-safe handling of strong & weak reference-counts. */
    struct AtomicRefBlock {
    public:
        ULONG AddRef(bool _strong) {
            if (_strong)
                return ++strong;
            else
                return ++weak;
        }

        /** Returns a copy of the struct to facillitate a thread safe parent class. */
        RefBlock Release(bool _strong) {
            if (_strong) {
                return { --strong, weak };
            }
            else {
                return { strong, --weak };
            }
        }

        /** Atomic increment of "strong" if the value is already >0. Based on https://devblogs.microsoft.com/oldnewthing/20221209-00/?p=107570 .*/
        bool AddRefIfStrongValid() {
            uint32_t prev = strong;

            if (prev < 1)
                return false; // no strong references

            // if (strong == prev)
            //     strong = prev + 1;
            // else
            //     prev = strong;
            while (!strong.compare_exchange_strong(prev, prev + 1)) {
                // "strong" have changed from a different thread
                if (prev < 1)
                    return false; // no strong references any more
            }

            return true;
        }

    private:
        std::atomic<uint32_t> strong = 0; // strong ref-count for m_ptr lifetime
        std::atomic<uint32_t> weak = 0;   // weak ref-count for SharedRef lifetime
    };

    AtomicRefBlock m_refs; // Reference-counts. Only touched once per method for thread safety.
    CComAggObject<Class>* m_ptr = nullptr;
    WeakRef        m_child;
    static inline ULONG s_obj_count = 0;
};
