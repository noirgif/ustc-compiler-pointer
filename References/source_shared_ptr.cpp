template<class _Ty>
class _Ptr_base
{    // base class for shared_ptr and weak_ptr
public:
    typedef _Ptr_base<_Ty> _Myt;
    typedef _Ty _Elem;
    typedef _Elem element_type;

    _Ptr_base()
        : _Ptr(0), _Rep(0)
    {    // construct
    }

    _Ptr_base(_Myt&& _Right)
        : _Ptr(0), _Rep(0)
    {    // construct _Ptr_base object that takes resource from _Right
        _Assign_rv(_STD forward<_Myt>(_Right));
    }

    template<class _Ty2>
    _Ptr_base(_Ptr_base<_Ty2>&& _Right)
        : _Ptr(_Right._Ptr), _Rep(_Right._Rep)
    {    // construct _Ptr_base object that takes resource from _Right
        _Right._Ptr = 0;
        _Right._Rep = 0;
    }

    _Myt& operator=(_Myt&& _Right)
    {    // construct _Ptr_base object that takes resource from _Right
        _Assign_rv(_STD forward<_Myt>(_Right));
        return (*this);
    }

    void _Assign_rv(_Myt&& _Right)
    {    // assign by moving _Right
        if (this != &_Right)
            _Swap(_Right);
    }

    long use_count() const
    {    // return use count
        return (_Rep ? _Rep->_Use_count() : 0);
    }

    void _Swap(_Ptr_base& _Right)
    {    // swap pointers
        _STD swap(_Rep, _Right._Rep);
        _STD swap(_Ptr, _Right._Ptr);
    }

    template<class _Ty2>
    bool owner_before(const _Ptr_base<_Ty2>& _Right) const
    {    // compare addresses of manager objects
        return (_Rep < _Right._Rep);
    }

    void *_Get_deleter(const _XSTD2 type_info& _Type) const
    {    // return pointer to deleter object if its type is _Type
        return (_Rep ? _Rep->_Get_deleter(_Type) : 0);
    }

    _Ty *_Get() const
    {    // return pointer to resource
        return (_Ptr);
    }

    bool _Expired() const
    {    // test if expired
        return (!_Rep || _Rep->_Expired());
    }

    void _Decref()
    {    // decrement reference count
        if (_Rep != 0)
            _Rep->_Decref();
    }

    void _Reset()
    {    // release resource
        _Reset(0, 0);
    }

    template<class _Ty2>
    void _Reset(const _Ptr_base<_Ty2>& _Other)
    {    // release resource and take ownership of _Other._Ptr
        _Reset(_Other._Ptr, _Other._Rep, false);
    }

    template<class _Ty2>
    void _Reset(const _Ptr_base<_Ty2>& _Other, bool _Throw)
    {    // release resource and take ownership from weak_ptr _Other._Ptr
        _Reset(_Other._Ptr, _Other._Rep, _Throw);
    }

    template<class _Ty2>
    void _Reset(const _Ptr_base<_Ty2>& _Other, const _Static_tag&)
    {    // release resource and take ownership of _Other._Ptr
        _Reset(static_cast<_Elem *>(_Other._Ptr), _Other._Rep);
    }

    template<class _Ty2>
    void _Reset(const _Ptr_base<_Ty2>& _Other, const _Const_tag&)
    {    // release resource and take ownership of _Other._Ptr
        _Reset(const_cast<_Elem *>(_Other._Ptr), _Other._Rep);
    }

    template<class _Ty2>
    void _Reset(const _Ptr_base<_Ty2>& _Other, const _Dynamic_tag&)
    {    // release resource and take ownership of _Other._Ptr
        _Elem *_Ptr = dynamic_cast<_Elem *>(_Other._Ptr);
        if (_Ptr)
            _Reset(_Ptr, _Other._Rep);
        else
            _Reset();
    }

    template<class _Ty2>
    void _Reset(auto_ptr<_Ty2>& _Other)
    {    // release resource and take _Other.get()
        _Ty2 *_Px = _Other.get();
        _Reset0(_Px, new _Ref_count<_Elem>(_Px));
        _Other.release();
        _Enable_shared(_Px, _Rep);
    }

#if _HAS_CPP0X
    template<class _Ty2>
    void _Reset(_Ty *_Ptr, const _Ptr_base<_Ty2>& _Other)
    {    // release resource and alias _Ptr with _Other_rep
        _Reset(_Ptr, _Other._Rep);
    }
#endif /* _HAS_CPP0X */

    void _Reset(_Ty *_Other_ptr, _Ref_count_base *_Other_rep)
    {    // release resource and take _Other_ptr through _Other_rep
        if (_Other_rep)
            _Other_rep->_Incref();
        _Reset0(_Other_ptr, _Other_rep);
    }

    void _Reset(_Ty *_Other_ptr, _Ref_count_base *_Other_rep, bool _Throw)
    {    // take _Other_ptr through _Other_rep from weak_ptr if not expired
        // otherwise, leave in default state if !_Throw,
        // otherwise throw exception
        if (_Other_rep && _Other_rep->_Incref_nz())
            _Reset0(_Other_ptr, _Other_rep);
        else if (_Throw)
            _THROW_NCEE(bad_weak_ptr, 0);
    }

    void _Reset0(_Ty *_Other_ptr, _Ref_count_base *_Other_rep)
    {    // release resource and take new resource
        if (_Rep != 0)
            _Rep->_Decref();
        _Rep = _Other_rep;
        _Ptr = _Other_ptr;
    }

    void _Decwref()
    {    // decrement weak reference count
        if (_Rep != 0)
            _Rep->_Decwref();
    }

    void _Resetw()
    {    // release weak reference to resource
        _Resetw((_Elem *)0, 0);
    }

    template<class _Ty2>
    void _Resetw(const _Ptr_base<_Ty2>& _Other)
    {    // release weak reference to resource and take _Other._Ptr
        _Resetw(_Other._Ptr, _Other._Rep);
    }

    template<class _Ty2>
    void _Resetw(const _Ty2 *_Other_ptr, _Ref_count_base *_Other_rep)
    {    // point to _Other_ptr through _Other_rep
        _Resetw(const_cast<_Ty2*>(_Other_ptr), _Other_rep);
    }

    template<class _Ty2>
    void _Resetw(_Ty2 *_Other_ptr, _Ref_count_base *_Other_rep)
    {    // point to _Other_ptr through _Other_rep
        if (_Other_rep)
            _Other_rep->_Incwref();
        if (_Rep != 0)
            _Rep->_Decwref();
        _Rep = _Other_rep;
        _Ptr = _Other_ptr;
    }

private:
    _Ty *_Ptr;
    _Ref_count_base *_Rep;
    template<class _Ty0>
    friend class _Ptr_base;
};



template<class _Ty>
class shared_ptr
    : public _Ptr_base<_Ty>
{    // class for reference counted resource management
public:
    typedef shared_ptr<_Ty> _Myt;
    typedef _Ptr_base<_Ty> _Mybase;

    shared_ptr()
    {    // construct empty shared_ptr object
    }

    template<class _Ux>
    explicit shared_ptr(_Ux *_Px)
    {    // construct shared_ptr object that owns _Px
        _Resetp(_Px);
    }

    template<class _Ux,
    class _Dx>
        shared_ptr(_Ux *_Px, _Dx _Dt)
    {    // construct with _Px, deleter
        _Resetp(_Px, _Dt);
    }

    //#if _HAS_CPP0X

#if defined(_NATIVE_NULLPTR_SUPPORTED) \
    && !defined(_DO_NOT_USE_NULLPTR_IN_STL)

    shared_ptr(_STD nullptr_t)
    {    // construct with nullptr
        _Resetp((_Ty *)0);
    }

    template<class _Dx>
    shared_ptr(_STD nullptr_t, _Dx _Dt)
    {    // construct with nullptr, deleter
        _Resetp((_Ty *)0, _Dt);
    }

    template<class _Dx,
    class _Alloc>
        shared_ptr(_STD nullptr_t, _Dx _Dt, _Alloc _Ax)
    {    // construct with nullptr, deleter, allocator
        _Resetp((_Ty *)0, _Dt, _Ax);
    }
#endif /* defined(_NATIVE_NULLPTR_SUPPORTED) etc. */

    template<class _Ux,
    class _Dx,
    class _Alloc>
        shared_ptr(_Ux *_Px, _Dx _Dt, _Alloc _Ax)
    {    // construct with _Px, deleter, allocator
        _Resetp(_Px, _Dt, _Ax);
    }
    //#endif /* _HAS_CPP0X */

#if _HAS_CPP0X
    template<class _Ty2>
    shared_ptr(const shared_ptr<_Ty2>& _Right, _Ty *_Px)
    {    // construct shared_ptr object that aliases _Right
        this->_Reset(_Px, _Right);
    }
#endif /* _HAS_CPP0X */

    shared_ptr(const _Myt& _Other)
    {    // construct shared_ptr object that owns same resource as _Other
        this->_Reset(_Other);
    }

    template<class _Ty2>
    shared_ptr(const shared_ptr<_Ty2>& _Other,
        typename enable_if<is_convertible<_Ty2 *, _Ty *>::value,
        void *>::type * = 0)
    {    // construct shared_ptr object that owns same resource as _Other
        this->_Reset(_Other);
    }

    template<class _Ty2>
    explicit shared_ptr(const weak_ptr<_Ty2>& _Other,
        bool _Throw = true)
    {    // construct shared_ptr object that owns resource *_Other
        this->_Reset(_Other, _Throw);
    }

    template<class _Ty2>
    shared_ptr(auto_ptr<_Ty2>& _Other)
    {    // construct shared_ptr object that owns *_Other.get()
        this->_Reset(_Other);
    }

    template<class _Ty2>
    shared_ptr(const shared_ptr<_Ty2>& _Other, const _Static_tag& _Tag)
    {    // construct shared_ptr object for static_pointer_cast
        this->_Reset(_Other, _Tag);
    }

    template<class _Ty2>
    shared_ptr(const shared_ptr<_Ty2>& _Other, const _Const_tag& _Tag)
    {    // construct shared_ptr object for const_pointer_cast
        this->_Reset(_Other, _Tag);
    }

    template<class _Ty2>
    shared_ptr(const shared_ptr<_Ty2>& _Other, const _Dynamic_tag& _Tag)
    {    // construct shared_ptr object for dynamic_pointer_cast
        this->_Reset(_Other, _Tag);
    }

    shared_ptr(_Myt&& _Right)
        : _Mybase(_STD forward<_Myt>(_Right))
    {    // construct shared_ptr object that takes resource from _Right
    }

    template<class _Ty2>
    shared_ptr(shared_ptr<_Ty2>&& _Right,
        typename enable_if<is_convertible<_Ty2 *, _Ty *>::value,
        void *>::type * = 0)
        : _Mybase(_STD forward<shared_ptr<_Ty2> >(_Right))
    {    // construct shared_ptr object that takes resource from _Right
    }

#if _HAS_CPP0X
    template<class _Ux,
    class _Dx>
        shared_ptr(_STD unique_ptr<_Ux, _Dx>&& _Right)
    {    // construct from unique_ptr
        _Resetp(_Right.release(), _Right.get_deleter());
    }

    template<class _Ux,
    class _Dx>
        _Myt& operator=(unique_ptr<_Ux, _Dx>&& _Right)
    {    // move from unique_ptr
        shared_ptr(_STD move(_Right)).swap(*this);
        return (*this);
    }
#endif /* _HAS_CPP0X */

    _Myt& operator=(_Myt&& _Right)
    {    // construct shared_ptr object that takes resource from _Right
        shared_ptr(_STD move(_Right)).swap(*this);
        return (*this);
    }

    template<class _Ty2>
    _Myt& operator=(shared_ptr<_Ty2>&& _Right)
    {    // construct shared_ptr object that takes resource from _Right
        shared_ptr(_STD move(_Right)).swap(*this);
        return (*this);
    }

    void swap(_Myt&& _Right)
    {    // exchange contents with movable _Right
        _Mybase::swap(_STD move(_Right));
    }

    ~shared_ptr()
    {    // release resource
        this->_Decref();
    }

    _Myt& operator=(const _Myt& _Right)
    {    // assign shared ownership of resource owned by _Right
        shared_ptr(_Right).swap(*this);
        return (*this);
    }

    template<class _Ty2>
    _Myt& operator=(const shared_ptr<_Ty2>& _Right)
    {    // assign shared ownership of resource owned by _Right
        shared_ptr(_Right).swap(*this);
        return (*this);
    }

    template<class _Ty2>
    _Myt& operator=(auto_ptr<_Ty2>& _Right)
    {    // assign ownership of resource pointed to by _Right
        shared_ptr(_Right).swap(*this);
        return (*this);
    }

    void reset()
    {    // release resource and convert to empty shared_ptr object
        shared_ptr().swap(*this);
    }

    template<class _Ux>
    void reset(_Ux *_Px)
    {    // release, take ownership of _Px
        shared_ptr(_Px).swap(*this);
    }

    template<class _Ux,
    class _Dx>
        void reset(_Ux *_Px, _Dx _Dt)
    {    // release, take ownership of _Px, with deleter _Dt
        shared_ptr(_Px, _Dt).swap(*this);
    }

    //#if _HAS_CPP0X
    template<class _Ux,
    class _Dx,
    class _Alloc>
        void reset(_Ux *_Px, _Dx _Dt, _Alloc _Ax)
    {    // release, take ownership of _Px, with deleter _Dt, allocator _Ax
        shared_ptr(_Px, _Dt, _Ax).swap(*this);
    }
    //#endif /* _HAS_CPP0X */

    void swap(_Myt& _Other)
    {    // swap pointers
        this->_Swap(_Other);
    }

    _Ty *get() const
    {    // return pointer to resource
        return (this->_Get());
    }

    typename tr1::add_reference<_Ty>::type operator*() const
    {    // return reference to resource
        return (*this->_Get());
    }

    _Ty *operator->() const
    {    // return pointer to resource
        return (this->_Get());
    }

    bool unique() const
    {    // return true if no other shared_ptr object owns this resource
        return (this->use_count() == 1);
    }

    _OPERATOR_BOOL() const
    {    // test if shared_ptr object owns no resource
        return (this->_Get() != 0 ? _CONVERTIBLE_TO_TRUE : 0);
    }

private:
    template<class _Ux>
    void _Resetp(_Ux *_Px)
    {    // release, take ownership of _Px
        _TRY_BEGIN    // allocate control block and reset
            _Resetp0(_Px, new _Ref_count<_Ux>(_Px));
        _CATCH_ALL    // allocation failed, delete resource
            delete _Px;
        _RERAISE;
        _CATCH_END
    }

    template<class _Ux,
    class _Dx>
        void _Resetp(_Ux *_Px, _Dx _Dt)
    {    // release, take ownership of _Px, deleter _Dt
        _TRY_BEGIN    // allocate control block and reset
            _Resetp0(_Px, new _Ref_count_del<_Ux, _Dx>(_Px, _Dt));
        _CATCH_ALL    // allocation failed, delete resource
            _Dt(_Px);
        _RERAISE;
        _CATCH_END
    }

    //#if _HAS_CPP0X
    template<class _Ux,
    class _Dx,
    class _Alloc>
        void _Resetp(_Ux *_Px, _Dx _Dt, _Alloc _Ax)
    {    // release, take ownership of _Px, deleter _Dt, allocator _Ax
        typedef _Ref_count_del_alloc<_Ux, _Dx, _Alloc> _Refd;
        typename _Alloc::template rebind<_Refd>::other _Al = _Ax;

        _TRY_BEGIN    // allocate control block and reset
            _Refd *_Ptr = _Al.allocate(1);
        new (_Ptr) _Refd(_Px, _Dt, _Al);
        _Resetp0(_Px, _Ptr);
        _CATCH_ALL    // allocation failed, delete resource
            _Dt(_Px);
        _RERAISE;
        _CATCH_END
    }
    //#endif /* _HAS_CPP0X */

public:
    template<class _Ux>
    void _Resetp0(_Ux *_Px, _Ref_count_base *_Rx)
    {    // release resource and take ownership of _Px
        this->_Reset0(_Px, _Rx);
        _Enable_shared(_Px, _Rx);
    }
};