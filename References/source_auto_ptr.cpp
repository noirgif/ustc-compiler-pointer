template<class _Ty>
class auto_ptr
{    // wrap an object pointer to ensure destruction
public:
    typedef auto_ptr<_Ty> _Myt;
    typedef _Ty element_type;

    explicit auto_ptr(_Ty *_Ptr = 0) _THROW0()
        : _Myptr(_Ptr)
    {    // construct from object pointer
    }

    auto_ptr(_Myt& _Right) _THROW0()
        : _Myptr(_Right.release())
    {    // construct by assuming pointer from _Right auto_ptr
    }

    auto_ptr(auto_ptr_ref<_Ty> _Right) _THROW0()
    {    // construct by assuming pointer from _Right auto_ptr_ref
        _Ty *_Ptr = _Right._Ref;
        _Right._Ref = 0;    // release old
        _Myptr = _Ptr;    // reset this
    }

    template<class _Other>
    operator auto_ptr<_Other>() _THROW0()
    {    // convert to compatible auto_ptr
        return (auto_ptr<_Other>(*this));
    }

    template<class _Other>
    operator auto_ptr_ref<_Other>() _THROW0()
    {    // convert to compatible auto_ptr_ref
        _Other *_Cvtptr = _Myptr;    // test implicit conversion
        auto_ptr_ref<_Other> _Ans(_Cvtptr);
        _Myptr = 0;    // pass ownership to auto_ptr_ref
        return (_Ans);
    }

    template<class _Other>
    _Myt& operator=(auto_ptr<_Other>& _Right) _THROW0()
    {    // assign compatible _Right (assume pointer)
        reset(_Right.release());
        return (*this);
    }

    template<class _Other>
    auto_ptr(auto_ptr<_Other>& _Right) _THROW0()
        : _Myptr(_Right.release())
    {    // construct by assuming pointer from _Right
    }

    _Myt& operator=(_Myt& _Right) _THROW0()
    {    // assign compatible _Right (assume pointer)
        reset(_Right.release());
        return (*this);
    }

    _Myt& operator=(auto_ptr_ref<_Ty> _Right) _THROW0()
    {    // assign compatible _Right._Ref (assume pointer)
        _Ty *_Ptr = _Right._Ref;
        _Right._Ref = 0;    // release old
        reset(_Ptr);    // set new
        return (*this);
    }

    ~auto_ptr()
    {    // destroy the object
        delete _Myptr;
    }

    _Ty& operator*() const _THROW0()
    {    // return designated value
#if _ITERATOR_DEBUG_LEVEL == 2
        if (_Myptr == 0)
            _DEBUG_ERROR("auto_ptr not dereferencable");
#endif /* _ITERATOR_DEBUG_LEVEL == 2 */

        return (*get());
    }

    _Ty *operator->() const _THROW0()
    {    // return pointer to class object
#if _ITERATOR_DEBUG_LEVEL == 2
        if (_Myptr == 0)
            _DEBUG_ERROR("auto_ptr not dereferencable");
#endif /* _ITERATOR_DEBUG_LEVEL == 2 */

        return (get());
    }

    _Ty *get() const _THROW0()
    {    // return wrapped pointer
        return (_Myptr);
    }

    _Ty *release() _THROW0()
    {    // return wrapped pointer and give up ownership
        _Ty *_Tmp = _Myptr;
        _Myptr = 0;
        return (_Tmp);
    }

    void reset(_Ty *_Ptr = 0)
    {    // destroy designated object and store new pointer
        if (_Ptr != _Myptr)
            delete _Myptr;
        _Myptr = _Ptr;
    }

private:
    _Ty *_Myptr;    // the wrapped object pointer
};