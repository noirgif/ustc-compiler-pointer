template<class _Ty>
class weak_ptr
    : public _Ptr_base<_Ty>
{    // class for pointer to reference counted resource
    typedef typename _Ptr_base<_Ty>::_Elem _Elem;

public:
    weak_ptr()
    {    // construct empty weak_ptr object
    }

    template<class _Ty2>
    weak_ptr(const shared_ptr<_Ty2>& _Other,
        typename enable_if<is_convertible<_Ty2 *, _Ty *>::value,
        void *>::type * = 0)
    {    // construct weak_ptr object for resource owned by _Other
        this->_Resetw(_Other);
    }

    weak_ptr(const weak_ptr& _Other)
    {    // construct weak_ptr object for resource pointed to by _Other
        this->_Resetw(_Other);
    }

    template<class _Ty2>
    weak_ptr(const weak_ptr<_Ty2>& _Other,
        typename enable_if<is_convertible<_Ty2 *, _Ty *>::value,
        void *>::type * = 0)
    {    // construct weak_ptr object for resource pointed to by _Other
        this->_Resetw(_Other);
    }

    ~weak_ptr()
    {    // release resource
        this->_Decwref();
    }

    weak_ptr& operator=(const weak_ptr& _Right)
    {    // assign from _Right
        this->_Resetw(_Right);
        return (*this);
    }

    template<class _Ty2>
    weak_ptr& operator=(const weak_ptr<_Ty2>& _Right)
    {    // assign from _Right
        this->_Resetw(_Right);
        return (*this);
    }

    template<class _Ty2>
    weak_ptr& operator=(shared_ptr<_Ty2>& _Right)
    {    // assign from _Right
        this->_Resetw(_Right);
        return (*this);
    }

    void reset()
    {    // release resource, convert to null weak_ptr object
        this->_Resetw();
    }

    void swap(weak_ptr& _Other)
    {    // swap pointers
        this->_Swap(_Other);
    }

    bool expired() const
    {    // return true if resource no longer exists
        return (this->_Expired());
    }

    shared_ptr<_Ty> lock() const
    {    // convert to shared_ptr
        return (shared_ptr<_Elem>(*this, false));
    }
};