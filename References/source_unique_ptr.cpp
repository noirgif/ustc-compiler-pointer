template<class _Ty,
class _Dx>    // = default_delete<_Ty>
class unique_ptr
    : public _Unique_ptr_base<_Ty, _Dx,
    tr1::is_empty<_Dx>::value
    || tr1::is_same<default_delete<_Ty>, _Dx>::value>
{    // non-copyable pointer to an object
public:
    typedef unique_ptr<_Ty, _Dx> _Myt;
    typedef _Unique_ptr_base<_Ty, _Dx,
        tr1::is_empty<_Dx>::value
        || tr1::is_same<default_delete<_Ty>, _Dx>::value> _Mybase;
    typedef typename _Mybase::pointer pointer;
    typedef _Ty element_type;
    typedef _Dx deleter_type;

    unique_ptr()
        : _Mybase(pointer(), _Dx())
    {    // default construct
        static_assert(!is_pointer<_Dx>::value,
            "unique_ptr constructed with null deleter pointer");
    }

#if defined(_NATIVE_NULLPTR_SUPPORTED) \
    && !defined(_DO_NOT_USE_NULLPTR_IN_STL)
    unique_ptr(_STD nullptr_t)
        : _Mybase(pointer(), _Dx())
    {    // null pointer construct
        static_assert(!is_pointer<_Dx>::value,
            "unique_ptr constructed with null deleter pointer");
    }

    _Myt& operator=(_STD nullptr_t)
    {    // assign a null pointer
        reset();
        return (*this);
    }
#endif /* defined(_NATIVE_NULLPTR_SUPPORTED) etc. */

    explicit unique_ptr(pointer _Ptr)
        : _Mybase(_Ptr, _Dx())
    {    // construct with pointer
        static_assert(!is_pointer<_Dx>::value,
            "unique_ptr constructed with null deleter pointer");
    }

    unique_ptr(pointer _Ptr,
        typename _If<tr1::is_reference<_Dx>::value, _Dx,
        const typename tr1::remove_reference<_Dx>::type&>::_Type _Dt)
        : _Mybase(_Ptr, _Dt)
    {    // construct with pointer and (maybe const) deleter&
    }

    unique_ptr(pointer _Ptr, typename tr1::remove_reference<_Dx>::type&& _Dt)
        : _Mybase(_Ptr, _STD move(_Dt))
    {    // construct by moving deleter
        //        static_assert(!tr1::is_reference<_Dx>::value,
        //            "unique_ptr constructed with reference to rvalue deleter");
    }

    unique_ptr(unique_ptr&& _Right)
        : _Mybase(_Right.release(),
        _STD forward<_Dx>(_Right.get_deleter()))
    {    // construct by moving _Right
    }

    template<class _Ty2,
    class _Dx2>
        unique_ptr(unique_ptr<_Ty2, _Dx2>&& _Right)
        : _Mybase(_Right.release(),
        _STD forward<_Dx2>(_Right.get_deleter()))
    {    // construct by moving _Right
    }

    template<class _Ty2,
    class _Dx2>
        _Myt& operator=(unique_ptr<_Ty2, _Dx2>&& _Right)
    {    // assign by moving _Right
        reset(_Right.release());
        this->get_deleter() = _STD move(_Right.get_deleter());
        return (*this);
    }

    _Myt& operator=(_Myt&& _Right)
    {    // assign by moving _Right
        if (this != &_Right)
        {    // different, do the move
            reset(_Right.release());
            this->get_deleter() = _STD move(_Right.get_deleter());
        }
        return (*this);
    }

    void swap(_Myt&& _Right)
    {    // swap elements
        if (this != &_Right)
        {    // different, do the swap
            _Swap_adl(this->_Myptr, _Right._Myptr);
            _Swap_adl(this->get_deleter(),
                _Right.get_deleter());
        }
    }

    void swap(_Myt& _Right)
    {    // swap elements
        _Swap_adl(this->_Myptr, _Right._Myptr);
        _Swap_adl(this->get_deleter(),
            _Right.get_deleter());
    }

    ~unique_ptr()
    {    // destroy the object
        _Delete();
    }

    typename tr1::add_reference<_Ty>::type operator*() const
    {    // return reference to object
        return (*this->_Myptr);
    }

    pointer operator->() const
    {    // return pointer to class object
        return (&**this);
    }

    pointer get() const
    {    // return pointer to object
        return (this->_Myptr);
    }

    _OPERATOR_BOOL() const
    {    // test for non-null pointer
        return (this->_Myptr != pointer() ? _CONVERTIBLE_TO_TRUE : 0);
    }

    pointer release()
    {    // yield ownership of pointer
        pointer _Ans = this->_Myptr;
        this->_Myptr = pointer();
        return (_Ans);
    }

    void reset(pointer _Ptr = pointer())
    {    // establish new pointer
        if (_Ptr != this->_Myptr)
        {    // different pointer, delete old and reassign
            _Delete();
            this->_Myptr = _Ptr;
        }
    }

private:
    void _Delete()
    {    // delete the pointer
        if (this->_Myptr != pointer())
            this->get_deleter()(this->_Myptr);
    }

    unique_ptr(const _Myt&);    // not defined
    template<class _Ty2,
    class _Dx2>
        unique_ptr(const unique_ptr<_Ty2, _Dx2>&);    // not defined

    _Myt& operator=(const _Myt&);    // not defined
    template<class _Ty2,
    class _Dx2>
        _Myt& operator=(const unique_ptr<_Ty2, _Dx2>&);    // not defined
};