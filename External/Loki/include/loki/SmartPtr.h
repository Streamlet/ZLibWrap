////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Code covered by the MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////
#ifndef LOKI_SMARTPTR_INC_
#define LOKI_SMARTPTR_INC_

// $Id$


///  \defgroup  SmartPointerGroup Smart pointers
///  Policy based implementation of a smart pointer
///  \defgroup  SmartPointerOwnershipGroup Ownership policies
///  \ingroup   SmartPointerGroup
///  \defgroup  SmartPointerStorageGroup Storage policies
///  \ingroup   SmartPointerGroup
///  \defgroup  SmartPointerConversionGroup Conversion policies
///  \ingroup   SmartPointerGroup
///  \defgroup  SmartPointerCheckingGroup Checking policies
///  \ingroup   SmartPointerGroup

#include <loki/LokiExport.h>
#include <loki/SmallObj.h>
#include <loki/TypeManip.h>
#include <loki/static_check.h>
#include <loki/RefToValue.h>
#include <loki/ConstPolicy.h>

#include <functional>
#include <stdexcept>
#include <cassert>
#include <string>

#if !defined(_MSC_VER)
#  if defined(__sparc__)
#    include <inttypes.h>
#  else
#    include <stdint.h>
#  endif
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
// GCC>=4.1 must use -ffriend-injection due to a bug in GCC
#define LOKI_ENABLE_FRIEND_TEMPLATE_TEMPLATE_PARAMETER_WORKAROUND
#endif

#if defined( _MSC_VER )
    #pragma warning( push )
    #pragma warning( disable: 4355 )
#endif


namespace Loki
{

////////////////////////////////////////////////////////////////////////////////
///  \class HeapStorage
///
///  \ingroup  SmartPointerStorageGroup
///  Implementation of the StoragePolicy used by SmartPtr.  Uses explicit call
///   to T's destructor followed by call to free.
////////////////////////////////////////////////////////////////////////////////


    template <class T>
    class HeapStorage
    {
    public:
        typedef T* StoredType;      /// the type of the pointee_ object
        typedef T* InitPointerType; /// type used to declare OwnershipPolicy type.
        typedef T* PointerType;     /// type returned by operator->
        typedef T& ReferenceType;   /// type returned by operator*

    protected:

        HeapStorage() : pointee_(Default())
        {}

        // The storage policy doesn't initialize the stored pointer
        //     which will be initialized by the OwnershipPolicy's Clone fn
        HeapStorage(const HeapStorage&) : pointee_(0)
        {}

        template <class U>
        HeapStorage(const HeapStorage<U>&) : pointee_(0)
        {}

        explicit HeapStorage(const StoredType& p) : pointee_(p) {}

        PointerType operator->() const { return pointee_; }

        ReferenceType operator*() const { return *pointee_; }

        void Swap(HeapStorage& rhs)
        { std::swap(pointee_, rhs.pointee_); }

        // Accessors
        template <class F>
        friend typename HeapStorage<F>::PointerType GetImpl(const HeapStorage<F>& sp);

        template <class F>
        friend const typename HeapStorage<F>::StoredType& GetImplRef(const HeapStorage<F>& sp);

        template <class F>
        friend typename HeapStorage<F>::StoredType& GetImplRef(HeapStorage<F>& sp);

        // Destroys the data stored
        // (Destruction might be taken over by the OwnershipPolicy)
        void Destroy()
        {
            if ( 0 != pointee_ )
            {
                pointee_->~T();
                ::free( pointee_ );
            }
        }

        // Default value to initialize the pointer
        static StoredType Default()
        { return 0; }

    private:
        // Data
        StoredType pointee_;
    };

    template <class T>
    inline typename HeapStorage<T>::PointerType GetImpl(const HeapStorage<T>& sp)
    { return sp.pointee_; }

    template <class T>
    inline const typename HeapStorage<T>::StoredType& GetImplRef(const HeapStorage<T>& sp)
    { return sp.pointee_; }

    template <class T>
    inline typename HeapStorage<T>::StoredType& GetImplRef(HeapStorage<T>& sp)
    { return sp.pointee_; }


////////////////////////////////////////////////////////////////////////////////
///  \class DefaultSPStorage
///
///  \ingroup  SmartPointerStorageGroup
///  Implementation of the StoragePolicy used by SmartPtr
////////////////////////////////////////////////////////////////////////////////


    template <class T>
    class DefaultSPStorage
    {
    public:
        typedef T* StoredType;    // the type of the pointee_ object
        typedef T* InitPointerType; /// type used to declare OwnershipPolicy type.
        typedef T* PointerType;   // type returned by operator->
        typedef T& ReferenceType; // type returned by operator*

    protected:

        DefaultSPStorage() : pointee_(Default())
        {}

        // The storage policy doesn't initialize the stored pointer
        //     which will be initialized by the OwnershipPolicy's Clone fn
        DefaultSPStorage(const DefaultSPStorage&) : pointee_(0)
        {}

        template <class U>
        DefaultSPStorage(const DefaultSPStorage<U>&) : pointee_(0)
        {}

        explicit DefaultSPStorage(const StoredType& p) : pointee_(p) {}

        PointerType operator->() const { return pointee_; }

        ReferenceType operator*() const { return *pointee_; }

        void Swap(DefaultSPStorage& rhs)
        { std::swap(pointee_, rhs.pointee_); }

        // Accessors
        template <class F>
        friend typename DefaultSPStorage<F>::PointerType GetImpl(const DefaultSPStorage<F>& sp);

        template <class F>
        friend const typename DefaultSPStorage<F>::StoredType& GetImplRef(const DefaultSPStorage<F>& sp);

        template <class F>
        friend typename DefaultSPStorage<F>::StoredType& GetImplRef(DefaultSPStorage<F>& sp);

        // Destroys the data stored
        // (Destruction might be taken over by the OwnershipPolicy)
        //
        // If your compiler gives you a warning in this area while
        // compiling the tests, it is on purpose, please ignore it.
        void Destroy()
        {
            delete pointee_;
        }

        // Default value to initialize the pointer
        static StoredType Default()
        { return 0; }

    private:
        // Data
        StoredType pointee_;
    };

    template <class T>
    inline typename DefaultSPStorage<T>::PointerType GetImpl(const DefaultSPStorage<T>& sp)
    { return sp.pointee_; }

    template <class T>
    inline const typename DefaultSPStorage<T>::StoredType& GetImplRef(const DefaultSPStorage<T>& sp)
    { return sp.pointee_; }

    template <class T>
    inline typename DefaultSPStorage<T>::StoredType& GetImplRef(DefaultSPStorage<T>& sp)
    { return sp.pointee_; }


////////////////////////////////////////////////////////////////////////////////
///  \class LockedStorage
///
///  \ingroup  SmartPointerStorageGroup
///  Implementation of the StoragePolicy used by SmartPtr.
///
///  Each call to operator-> locks the object for the duration of a call to a
///  member function of T.
///
///  \par How It Works
///  LockedStorage has a helper class called Locker, which acts as a smart
///  pointer with limited abilities.  LockedStorage::operator-> returns an
///  unnamed temporary of type Locker<T> that exists for the duration of the
///  call to a member function of T.  The unnamed temporary locks the object
///  when it is constructed by operator-> and unlocks the object when it is
///  destructed.
///
///  \note This storage policy requires class T to have member functions Lock
///  and Unlock.  If your class does not have Lock or Unlock functions, you may
///  either make a child class which does, or make a policy class similar to
///  LockedStorage which calls other functions to lock the object.
////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class Locker
    {
    public:
        explicit Locker( const T * p ) : pointee_( const_cast< T * >( p ) )
        {
            if ( pointee_ != 0 )
                pointee_->Lock();
        }

        ~Locker( void )
        {
            if ( pointee_ != 0 )
                pointee_->Unlock();
        }

        operator T * ()
        {
            return pointee_;
        }

        T * operator->()
        {
            return pointee_;
        }

    private:
        Locker( void );
        Locker & operator = ( const Locker & );
        T * pointee_;
    };

    template <class T>
    class LockedStorage
    {
    public:

        typedef T* StoredType;           /// the type of the pointee_ object
        typedef T* InitPointerType;      /// type used to declare OwnershipPolicy type.
        typedef Locker< T > PointerType; /// type returned by operator->
        typedef T& ReferenceType;        /// type returned by operator*

    protected:

        LockedStorage() : pointee_( Default() ) {}

        ~LockedStorage( void ) {}

        LockedStorage( const LockedStorage&) : pointee_( 0 ) {}

        explicit LockedStorage( const StoredType & p ) : pointee_( p ) {}

        PointerType operator->() const
        {
            return Locker< T >( pointee_ );
        }

        void Swap(LockedStorage& rhs)
        {
            std::swap( pointee_, rhs.pointee_ );
        }

        // Accessors
        template <class F>
        friend typename LockedStorage<F>::InitPointerType GetImpl(const LockedStorage<F>& sp);

        template <class F>
        friend const typename LockedStorage<F>::StoredType& GetImplRef(const LockedStorage<F>& sp);

        template <class F>
        friend typename LockedStorage<F>::StoredType& GetImplRef(LockedStorage<F>& sp);

        // Destroys the data stored
        // (Destruction might be taken over by the OwnershipPolicy)
        void Destroy()
        {
            delete pointee_;
        }

        // Default value to initialize the pointer
        static StoredType Default()
        { return 0; }

    private:
        /// Dereference operator is not implemented.
        ReferenceType operator*();

        // Data
        StoredType pointee_;
    };

    template <class T>
    inline typename LockedStorage<T>::InitPointerType GetImpl(const LockedStorage<T>& sp)
    { return sp.pointee_; }

    template <class T>
    inline const typename LockedStorage<T>::StoredType& GetImplRef(const LockedStorage<T>& sp)
    { return sp.pointee_; }

    template <class T>
    inline typename LockedStorage<T>::StoredType& GetImplRef(LockedStorage<T>& sp)
    { return sp.pointee_; }

    namespace Private
    {

        ////////////////////////////////////////////////////////////////////////////////
        ///  \class DeleteArrayBase
        ///
        ///  \ingroup  StrongPointerDeleteGroup
        ///  Base class used only by the DeleteArray policy class.  This stores the
        ///   number of elements in an array of shared objects.
        ////////////////////////////////////////////////////////////////////////////////

        class DeleteArrayBase
        {
        public:

            inline size_t GetArrayCount( void ) const { return m_itemCount; }

        protected:

            DeleteArrayBase( void ) : m_itemCount( 0 ) {}

            explicit DeleteArrayBase( size_t itemCount ) : m_itemCount( itemCount ) {}

            DeleteArrayBase( const DeleteArrayBase & that ) : m_itemCount( that.m_itemCount ) {}

            void Swap( DeleteArrayBase & rhs );

            void OnInit( const void * p ) const;

            void OnCheckRange( size_t index ) const;

        private:

            size_t m_itemCount;

        };

    }


////////////////////////////////////////////////////////////////////////////////
///  \class ArrayStorage
///
///  \ingroup  SmartPointerStorageGroup
///  Implementation of the ArrayStorage used by SmartPtr
///  This assumes the pointer points to the zeroth element in an array, and uses
///  the array-delete operator to deconstruct and deallocate the array. DeepCopy
///  is not compatible with ArrayStorage DeepCopy::Clone will only copy the first
///  element in the array and won't know the size of the array. Even if it did
///  know the size, it would need to use array new to safely work the array
///  delete operator in ArrayStorage, but array new will not copy the elements
///  in the source array since it calls the default constructor for each element.
////////////////////////////////////////////////////////////////////////////////


    template <class T>
    class ArrayStorage : public ::Loki::Private::DeleteArrayBase
    {
    public:

        typedef T* StoredType;    // the type of the pointee_ object
        typedef T* InitPointerType; /// type used to declare OwnershipPolicy type.
        typedef T* PointerType;   // type returned by operator->
        typedef T& ReferenceType; // type returned by operator*

    protected:

        ArrayStorage() : DeleteArrayBase(), pointee_(Default())
        {}

        // The storage policy doesn't initialize the stored pointer
        //     which will be initialized by the OwnershipPolicy's Clone fn
        ArrayStorage( const ArrayStorage & that ) : DeleteArrayBase( that ), pointee_( 0 )
        {}

        template <class U>
        ArrayStorage( const ArrayStorage< U >& that ) : DeleteArrayBase( that ), pointee_( 0 )
        {}

        ArrayStorage( const StoredType & p, size_t count ) : DeleteArrayBase( count ),
            pointee_( p ) {}

        PointerType operator->() const { return pointee_; }

        ReferenceType operator*() const { return *pointee_; }

        void Swap( ArrayStorage & rhs )
        {
            DeleteArrayBase::Swap( rhs );
            ::std::swap( pointee_, rhs.pointee_ );
        }

        // Accessors
        template <class F>
        friend typename ArrayStorage<F>::PointerType GetImpl(const ArrayStorage<F>& sp);

        template <class F>
        friend const typename ArrayStorage<F>::StoredType& GetImplRef(const ArrayStorage<F>& sp);

        template <class F>
        friend typename ArrayStorage<F>::StoredType& GetImplRef(ArrayStorage<F>& sp);

        // Destroys the data stored
        // (Destruction might be taken over by the OwnershipPolicy)
        void Destroy()
        { delete [] pointee_; }

        // Default value to initialize the pointer
        static StoredType Default()
        { return 0; }

    private:
        // Data
        StoredType pointee_;
    };

    template <class T>
    inline typename ArrayStorage<T>::PointerType GetImpl(const ArrayStorage<T>& sp)
    { return sp.pointee_; }

    template <class T>
    inline const typename ArrayStorage<T>::StoredType& GetImplRef(const ArrayStorage<T>& sp)
    { return sp.pointee_; }

    template <class T>
    inline typename ArrayStorage<T>::StoredType& GetImplRef(ArrayStorage<T>& sp)
    { return sp.pointee_; }


////////////////////////////////////////////////////////////////////////////////
///  \class RefCounted
///
///  \ingroup  SmartPointerOwnershipGroup
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Provides a classic external reference counting implementation
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    class RefCounted
    {
    protected:
        RefCounted()
            : pCount_(static_cast<uintptr_t*>(
                SmallObject<>::operator new(sizeof(uintptr_t))))
        {
            assert(pCount_!=0);
            *pCount_ = 1;
        }

        RefCounted(const RefCounted& rhs)
        : pCount_(rhs.pCount_)
        {}

        // MWCW lacks template friends, hence the following kludge
        template <typename P1>
        RefCounted(const RefCounted<P1>& rhs)
        : pCount_(reinterpret_cast<const RefCounted&>(rhs).pCount_)
        {}

        P Clone(const P& val)
        {
            ++*pCount_;
            return val;
        }

        bool Release(const P&)
        {
            if (!--*pCount_)
            {
                SmallObject<>::operator delete(pCount_, sizeof(uintptr_t));
                pCount_ = NULL;
                return true;
            }
            return false;
        }

        void Swap(RefCounted& rhs)
        { std::swap(pCount_, rhs.pCount_); }

        enum { destructiveCopy = false };

    private:
        // Data
        uintptr_t* pCount_;
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct RefCountedMT
///
///  \ingroup  SmartPointerOwnershipGroup
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Implements external reference counting for multithreaded programs
///  Policy Usage: RefCountedMTAdj<ThreadingModel>::RefCountedMT
///
///  \par Warning
///  There could be a race condition, see bug "Race condition in RefCountedMTAdj::Release"
///  http://sourceforge.net/tracker/index.php?func=detail&aid=1408845&group_id=29557&atid=396644
///  As stated in bug 1408845, the Release function is not thread safe if a
///  SmartPtr copy-constructor tries to copy the last pointer to an object in
///  one thread, while the destructor is acting on the last pointer in another
///  thread.  The existence of a race between a copy-constructor and destructor
///  implies a design flaw at a higher level.  That race condition must be
///  fixed at a higher design level, and no change to this class could fix it.
////////////////////////////////////////////////////////////////////////////////

    template <template <class, class> class ThreadingModel,
              class MX = LOKI_DEFAULT_MUTEX >
    struct RefCountedMTAdj
    {
        template <class P>
        class RefCountedMT : public ThreadingModel< RefCountedMT<P>, MX >
        {
        public:

            typedef ThreadingModel< RefCountedMT<P>, MX > base_type;
            typedef typename base_type::IntType       CountType;
            typedef volatile CountType               *CountPtrType;

        protected:
            RefCountedMT()
            {
                pCount_ = static_cast<CountPtrType>(
                    SmallObject<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::operator new(
                        sizeof(*pCount_)));
                assert(pCount_);
                //*pCount_ = 1;
                ThreadingModel<RefCountedMT, MX>::AtomicAssign(*pCount_, 1);
            }

            RefCountedMT(const RefCountedMT& rhs)
            : pCount_(rhs.pCount_)
            {}

            //MWCW lacks template friends, hence the following kludge
            template <typename P1>
            RefCountedMT(const RefCountedMT<P1>& rhs)
            : pCount_(reinterpret_cast<const RefCountedMT<P>&>(rhs).pCount_)
            {}

            P Clone(const P& val)
            {
                ThreadingModel<RefCountedMT, MX>::AtomicIncrement(*pCount_);
                return val;
            }

            bool Release(const P&)
            {
                bool isZero = false;
                ThreadingModel< RefCountedMT, MX >::AtomicDecrement( *pCount_, 0, isZero );
                if ( isZero )
                {
                    SmallObject<LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL>::operator delete(
                        const_cast<CountType *>(pCount_),
                        sizeof(*pCount_));
                    return true;
                }
                return false;
            }

            void Swap(RefCountedMT& rhs)
            { std::swap(pCount_, rhs.pCount_); }

            enum { destructiveCopy = false };

        private:
            // Data
            CountPtrType pCount_;
        };
    };

////////////////////////////////////////////////////////////////////////////////
///  \class COMRefCounted
///
///  \ingroup  SmartPointerOwnershipGroup
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Adapts COM intrusive reference counting to OwnershipPolicy-specific syntax
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    class COMRefCounted
    {
    protected:
        COMRefCounted()
        {}

        template <class U>
        COMRefCounted(const COMRefCounted<U>&)
        {}

        static P Clone(const P& val)
        {
            if(val!=0)
               val->AddRef();
            return val;
        }

        static bool Release(const P& val)
        {
            if(val!=0)
                val->Release();
            return false;
        }

        enum { destructiveCopy = false };

        static void Swap(COMRefCounted&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct DeepCopy
///
///  \ingroup  SmartPointerOwnershipGroup
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Implements deep copy semantics, assumes existence of a Clone() member
///  function of the pointee type. DeepCopy is not compatible with ArrayStorage
///  DeepCopy::Clone will only copy the first element in the array and won't
///  know the size of the array. Even if it did know the size, it would need to
///  use array new to safely work the array delete operator in ArrayStorage, but
///  array new will not copy the elements in the source array since it calls the
///  default constructor for each array element.
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct DeepCopy
    {
    protected:
        DeepCopy()
        {}

        template <class P1>
        DeepCopy(const DeepCopy<P1>&)
        {}

        static P Clone(const P& val)
        { return val->Clone(); }

        static bool Release(const P&)
        { return true; }

        static void Swap(DeepCopy&)
        {}

        enum { destructiveCopy = false };
    };

////////////////////////////////////////////////////////////////////////////////
///  \class RefLinked
///
///  \ingroup  SmartPointerOwnershipGroup
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Implements reference linking
////////////////////////////////////////////////////////////////////////////////

    namespace Private
    {
        class LOKI_EXPORT RefLinkedBase
        {
        protected:
            RefLinkedBase( void ) :
                prev_( this ), next_( this )
            {}

            RefLinkedBase(const RefLinkedBase& rhs);

            bool Release();

            void Swap(RefLinkedBase& rhs);

            bool Merge( RefLinkedBase & rhs );

            enum { destructiveCopy = false };

        private:
            static unsigned int CountPrevCycle( const RefLinkedBase * pThis );
            static unsigned int CountNextCycle( const RefLinkedBase * pThis );
            bool HasPrevNode( const RefLinkedBase * p ) const;
            bool HasNextNode( const RefLinkedBase * p ) const;

            mutable const RefLinkedBase* prev_;
            mutable const RefLinkedBase* next_;
        };
    }

    template <class P>
    class RefLinked : public Private::RefLinkedBase
    {
    protected:
        RefLinked()
        {}

        template <class P1>
        RefLinked(const RefLinked<P1>& rhs)
        : Private::RefLinkedBase(rhs)
        {}

        static P Clone(const P& val)
        { return val; }

        bool Release(const P&)
        { return Private::RefLinkedBase::Release(); }

        template < class P1 >
        bool Merge( RefLinked< P1 > & rhs )
        {
            return Private::RefLinkedBase::Merge( rhs );
        }
    };

////////////////////////////////////////////////////////////////////////////////
///  \class DestructiveCopy
///
///  \ingroup  SmartPointerOwnershipGroup
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Implements destructive copy semantics (a la std::auto_ptr)
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    class DestructiveCopy
    {
    protected:
        DestructiveCopy()
        {}

        template <class P1>
        DestructiveCopy(const DestructiveCopy<P1>&)
        {}

        template <class P1>
        static P Clone( const P1 & val )
        {
            P result(val);
            const_cast< P1 & >( val ) = P1();
            return result;
        }

        template <class P1>
        static P Clone(P1& val)
        {
            P result(val);
            val = P1();
            return result;
        }

        static bool Release(const P&)
        { return true; }

        static void Swap(DestructiveCopy&)
        {}

        enum { destructiveCopy = true };
    };

////////////////////////////////////////////////////////////////////////////////
///  \class NoCopy
///
///  \ingroup  SmartPointerOwnershipGroup
///  Implementation of the OwnershipPolicy used by SmartPtr
///  Implements a policy that doesn't allow copying objects
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    class NoCopy
    {
    protected:
        NoCopy()
        {}

        template <class P1>
        NoCopy(const NoCopy<P1>&)
        {}

        static P Clone(const P&)
        {
            // Make it depended on template parameter
            static const bool DependedFalse = sizeof(P*) == 0;

            LOKI_STATIC_CHECK(DependedFalse, This_Policy_Disallows_Value_Copying);
        }

        static bool Release(const P&)
        { return true; }

        static void Swap(NoCopy&)
        {}

        enum { destructiveCopy = false };
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct AllowConversion
///
///  \ingroup  SmartPointerConversionGroup
///  Implementation of the ConversionPolicy used by SmartPtr
///  Allows implicit conversion from SmartPtr to the pointee type
////////////////////////////////////////////////////////////////////////////////

    struct AllowConversion
    {
        enum { allow = true };

        void Swap(AllowConversion&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct DisallowConversion
///
///  \ingroup  SmartPointerConversionGroup
///  Implementation of the ConversionPolicy used by SmartPtr
///  Does not allow implicit conversion from SmartPtr to the pointee type
///  You can initialize a DisallowConversion with an AllowConversion
////////////////////////////////////////////////////////////////////////////////

    struct DisallowConversion
    {
        DisallowConversion()
        {}

        DisallowConversion(const AllowConversion&)
        {}

        enum { allow = false };

        void Swap(DisallowConversion&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct NoCheck
///
///  \ingroup  SmartPointerCheckingGroup
///  Implementation of the CheckingPolicy used by SmartPtr
///  Well, it's clear what it does :o)
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct NoCheck
    {
    protected:

        NoCheck()
        {}

        NoCheck( const NoCheck & ) {}

        template <class P1>
        NoCheck(const NoCheck<P1>&)
        {}

        static void OnDefault(const P&)
        {}

        static void OnInit(const P&)
        {}

        static void OnDereference(const P&)
        {}

        static void Swap(NoCheck&)
        {}
    };


////////////////////////////////////////////////////////////////////////////////
///  \struct AssertCheck
///
///  \ingroup  SmartPointerCheckingGroup
///  Implementation of the CheckingPolicy used by SmartPtr
///  Checks the pointer before dereference
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct AssertCheck
    {
    protected:

        AssertCheck()
        {}

        AssertCheck( const AssertCheck & ) {}

        template <class P1>
        AssertCheck(const AssertCheck<P1>&)
        {}

        template <class P1>
        AssertCheck(const NoCheck<P1>&)
        {}

        static void OnDefault(const P&)
        {}

        static void OnInit(const P&)
        {}

        static void OnDereference(P val)
        { assert(val); (void)val; }

        static void Swap(AssertCheck&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct AssertCheckStrict
///
///  \ingroup  SmartPointerCheckingGroup
///  Implementation of the CheckingPolicy used by SmartPtr
///  Checks the pointer against zero upon initialization and before dereference
///  You can initialize an AssertCheckStrict with an AssertCheck
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct AssertCheckStrict
    {
    protected:

        AssertCheckStrict()
        {}

        AssertCheckStrict( const AssertCheckStrict & ) {}

        template <class U>
        AssertCheckStrict(const AssertCheckStrict<U>&)
        {}

        template <class U>
        AssertCheckStrict(const AssertCheck<U>&)
        {}

        template <class P1>
        AssertCheckStrict(const NoCheck<P1>&)
        {}

        static void OnDefault(P val)
        { assert(val); }

        static void OnInit(P val)
        { assert(val); }

        static void OnDereference(P val)
        { assert(val); }

        static void Swap(AssertCheckStrict&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct NullPointerException
///
///  \ingroup SmartPointerGroup
///  Used by some implementations of the CheckingPolicy used by SmartPtr
////////////////////////////////////////////////////////////////////////////////

    struct NullPointerException : public std::runtime_error
    {
        NullPointerException() : std::runtime_error(std::string(""))
        { }
        const char* what() const throw()
        { return "Null Pointer Exception"; }
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct RejectNullStatic
///
///  \ingroup  SmartPointerCheckingGroup
///  Implementation of the CheckingPolicy used by SmartPtr
///  Checks the pointer upon initialization and before dereference
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct RejectNullStatic
    {
    protected:

        RejectNullStatic()
        {}

        RejectNullStatic( const RejectNullStatic & ) {}

        template <class P1>
        RejectNullStatic(const RejectNullStatic<P1>&)
        {}

        template <class P1>
        RejectNullStatic(const NoCheck<P1>&)
        {}

        template <class P1>
        RejectNullStatic(const AssertCheck<P1>&)
        {}

        template <class P1>
        RejectNullStatic(const AssertCheckStrict<P1>&)
        {}

        static void OnDefault(const P&)
        {
            // Make it depended on template parameter
            static const bool DependedFalse = ( sizeof(P*) == 0 );

            LOKI_STATIC_CHECK(DependedFalse, ERROR_This_Policy_Does_Not_Allow_Default_Initialization);
        }

        static void OnInit(const P& val)
        { if (!val) throw NullPointerException(); }

        static void OnDereference(const P& val)
        { if (!val) throw NullPointerException(); }

        static void Swap(RejectNullStatic&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct RejectNull
///
///  \ingroup  SmartPointerCheckingGroup
///  Implementation of the CheckingPolicy used by SmartPtr
///  Checks the pointer before dereference
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct RejectNull
    {
    protected:

        RejectNull()
        {}

        RejectNull( const RejectNull & ) {}

        template <class P1>
        RejectNull(const RejectNull<P1>&)
        {}

        static void OnInit(P)
        {}

        static void OnDefault(P)
        {}

        void OnDereference(P val)
        { if (!val) throw NullPointerException(); }

        void OnDereference(P val) const
        { if (!val) throw NullPointerException(); }

        void Swap(RejectNull&)
        {}
    };

////////////////////////////////////////////////////////////////////////////////
///  \struct RejectNullStrict
///
///  \ingroup  SmartPointerCheckingGroup
///  Implementation of the CheckingPolicy used by SmartPtr
///  Checks the pointer upon initialization and before dereference
////////////////////////////////////////////////////////////////////////////////

    template <class P>
    struct RejectNullStrict
    {
    protected:

        RejectNullStrict()
        {}

        RejectNullStrict( const RejectNullStrict & ) {}

        template <class P1>
        RejectNullStrict(const RejectNullStrict<P1>&)
        {}

        template <class P1>
        RejectNullStrict(const RejectNull<P1>&)
        {}

        static void OnInit(P val)
        { if (!val) throw NullPointerException(); }

        void OnDereference(P val)
        { OnInit(val); }

        void OnDereference(P val) const
        { OnInit(val); }

        void Swap(RejectNullStrict&)
        {}
    };


////////////////////////////////////////////////////////////////////////////////
// class template SmartPtr (declaration)
// The reason for all the fuss above
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OwnershipPolicy = RefCounted,
        class ConversionPolicy = DisallowConversion,
        template <class> class CheckingPolicy = AssertCheck,
        template <class> class StoragePolicy = DefaultSPStorage,
        template<class> class ConstnessPolicy = LOKI_DEFAULT_CONSTNESS
     >
     class SmartPtr;

////////////////////////////////////////////////////////////////////////////////
// class template SmartPtrDef (definition)
// this class added to unify the usage of SmartPtr
// instead of writing SmartPtr<T,OP,CP,KP,SP> write SmartPtrDef<T,OP,CP,KP,SP>::type
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OwnershipPolicy = RefCounted,
        class ConversionPolicy = DisallowConversion,
        template <class> class CheckingPolicy = AssertCheck,
        template <class> class StoragePolicy = DefaultSPStorage,
        template<class> class ConstnessPolicy = LOKI_DEFAULT_CONSTNESS
    >
    struct SmartPtrDef
    {
        typedef SmartPtr
        <
            T,
            OwnershipPolicy,
            ConversionPolicy,
            CheckingPolicy,
            StoragePolicy,
            ConstnessPolicy
        >
        type;
    };

////////////////////////////////////////////////////////////////////////////////
///  \class SmartPtr
///
///  \ingroup SmartPointerGroup
///
///  \param OwnershipPolicy  default =  RefCounted,
///  \param ConversionPolicy default = DisallowConversion,
///  \param CheckingPolicy default = AssertCheck,
///  \param StoragePolicy default = DefaultSPStorage
///  \param ConstnessPolicy default = LOKI_DEFAULT_CONSTNESS
///
///  \par IMPORTANT NOTE
///  Due to threading issues, the OwnershipPolicy has been changed as follows:
///
///     - Release() returns a boolean saying if that was the last release
///        so the pointer can be deleted by the StoragePolicy
///     - IsUnique() was removed
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OwnershipPolicy,
        class ConversionPolicy,
        template <class> class CheckingPolicy,
        template <class> class StoragePolicy,
        template <class> class ConstnessPolicy
    >
    class SmartPtr
        : public StoragePolicy<T>
        , public OwnershipPolicy<typename StoragePolicy<T>::InitPointerType>
        , public CheckingPolicy<typename StoragePolicy<T>::StoredType>
        , public ConversionPolicy
    {
        typedef StoragePolicy<T> SP;
        typedef OwnershipPolicy<typename StoragePolicy<T>::InitPointerType> OP;
        typedef CheckingPolicy<typename StoragePolicy<T>::StoredType> KP;
        typedef ConversionPolicy CP;

    public:
        typedef typename ConstnessPolicy<T>::Type* ConstPointerType;
        typedef typename ConstnessPolicy<T>::Type& ConstReferenceType;

        typedef typename SP::PointerType PointerType;
        typedef typename SP::StoredType StoredType;
        typedef typename SP::ReferenceType ReferenceType;

        typedef typename Select<OP::destructiveCopy,SmartPtr, const SmartPtr>::Result
                CopyArg;

    private:
        struct NeverMatched {};

#ifdef LOKI_SMARTPTR_CONVERSION_CONSTRUCTOR_POLICY
        typedef typename Select< CP::allow, const StoredType&, NeverMatched>::Result ImplicitArg;
        typedef typename Select<!CP::allow, const StoredType&, NeverMatched>::Result ExplicitArg;
#else
        typedef const StoredType& ImplicitArg;
        typedef typename Select<false, const StoredType&, NeverMatched>::Result ExplicitArg;
#endif

        /// SmartPtr uses this helper class to specify the dynamic-caster constructor.
        class DynamicCastHelper {};

        /// Private constructor is only used for dynamic-casting.
        template
        <
            typename T1,
            template < class > class OP1,
            class CP1,
            template < class > class KP1,
            template < class > class SP1,
            template < class > class CNP1
        >
        SmartPtr( const SmartPtr< T1, OP1, CP1, KP1, SP1, CNP1 > & rhs, const DynamicCastHelper & helper )
        {
            (void)helper; // do void cast to remove compiler warning.
            // Dynamic casting from T1 to T and saving result in `this''s pointer
            PointerType p = dynamic_cast< PointerType >( GetImplRef( rhs ) );
            KP::OnDereference( p );
            GetImplRef( *this ) = OP::Clone( p );
        }

        /// Private constructor is only used for dynamic-casting.
        template
        <
            typename T1,
            template < class > class OP1,
            class CP1,
            template < class > class KP1,
            template < class > class SP1,
            template < class > class CNP1
        >
        SmartPtr( SmartPtr< T1, OP1, CP1, KP1, SP1, CNP1 > & rhs, const DynamicCastHelper & helper )
        {
            (void)helper; // do void cast to remove compiler warning.
            // Dynamic casting from T1 to T and saving result in `this''s pointer
            PointerType p = dynamic_cast< PointerType >( GetImplRef( rhs ) );
            KP::OnDereference( p );
            GetImplRef( *this ) = OP::Clone( p );
        }

    public:

        SmartPtr()
        {
            KP::OnDefault(GetImpl(*this));
        }

        explicit
        SmartPtr(ExplicitArg p) : SP(p)
        {
            KP::OnInit(GetImpl(*this));
        }

        SmartPtr(ImplicitArg p) : SP(p)
        {
            KP::OnInit(GetImpl(*this));
        }

        /** This constructor was designed to only work with the ArrayStorage policy. Using it with
         any other Delete policies will cause compiler errors. Call it with this syntax:
         "ThingyPtr sp2( new Thingy[ 4 ], 4 );" so SmartPtr can do range checking on the number of elements.
         */
        SmartPtr( ImplicitArg p, size_t itemCount ) : SP( p, itemCount )
        {
            KP::OnInit( GetImpl( *this ) );
            SP::OnInit( GetImpl( *this ) );
        }

        SmartPtr(CopyArg& rhs) : SP(rhs), OP(rhs), KP(rhs), CP(rhs)
        {
            KP::OnDereference( GetImpl( rhs ) );
            GetImplRef(*this) = OP::Clone(GetImplRef(rhs));
        }

        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        SmartPtr( const SmartPtr< T1, OP1, CP1, KP1, SP1, CNP1 >& rhs )
        : SP(rhs), OP(rhs), KP(rhs), CP(rhs)
        {
            KP::OnDereference( GetImpl( rhs ) );
            GetImplRef(*this) = OP::Clone(GetImplRef(rhs));
        }

        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        SmartPtr( SmartPtr< T1, OP1, CP1, KP1, SP1, CNP1 >& rhs )
        : SP(rhs), OP(rhs), KP(rhs), CP(rhs)
        {
            KP::OnDereference( GetImpl( rhs ) );
            GetImplRef(*this) = OP::Clone(GetImplRef(rhs));
        }

        SmartPtr(RefToValue<SmartPtr> rhs)
        : SP(rhs), OP(rhs), KP(rhs), CP(rhs)
        {
            SmartPtr & ref = rhs;
            KP::OnDereference( GetImpl( ref ) );
            GetImplRef( *this ) = OP::Clone( GetImplRef( ref ) );
        }

        operator RefToValue<SmartPtr>()
        { return RefToValue<SmartPtr>(*this); }

        SmartPtr& operator=(CopyArg& rhs)
        {
            SmartPtr temp(rhs);
            temp.Swap(*this);
            return *this;
        }

        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        SmartPtr& operator=(const SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs)
        {
            SmartPtr temp(rhs);
            temp.Swap(*this);
            return *this;
        }

        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        SmartPtr& operator=(SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs)
        {
            SmartPtr temp(rhs);
            temp.Swap(*this);
            return *this;
        }

        /** This function is equivalent to an assignment operator for SmartPtr's that use the
         DeleteArray policy where the programmer needs to write the equivalent of "sp = new P;".
         With DeleteArray, the programmer should write "sp.Assign( new [5] Thingy, 5 );" so the
         SmartPtr knows how many elements are in the array.
         */
        SmartPtr & Assign( T * p, size_t itemCount )
        {
            if ( GetImpl( *this ) != p )
            {
                SmartPtr temp( p, itemCount );
                Swap( temp );
            }
            return *this;
        }

        void Swap(SmartPtr& rhs)
        {
            OP::Swap(rhs);
            CP::Swap(rhs);
            KP::Swap(rhs);
            SP::Swap(rhs);
        }

        ~SmartPtr()
        {
            if (OP::Release(GetImpl(*static_cast<SP*>(this))))
            {
                SP::Destroy();
            }
        }

        /// Dynamically-casts parameter pointer to the type specified by this SmartPtr type.
        template
        <
            typename T1,
            template < class > class OP1,
            class CP1,
            template < class > class KP1,
            template < class > class SP1,
            template < class > class CNP1
        >
        SmartPtr & DynamicCastFrom( const SmartPtr< T1, OP1, CP1, KP1, SP1, CNP1 > & rhs )
        {
            SmartPtr temp( rhs, DynamicCastHelper() );
            temp.Swap( *this );
            return *this;
        }

        /// Dynamically-casts parameter pointer to the type specified by this SmartPtr type.
        template
        <
            typename T1,
            template < class > class OP1,
            class CP1,
            template < class > class KP1,
            template < class > class SP1,
            template < class > class CNP1
        >
        SmartPtr & DynamicCastFrom( SmartPtr< T1, OP1, CP1, KP1, SP1, CNP1 > & rhs )
        {
            SmartPtr temp( rhs, DynamicCastHelper() );
            temp.Swap( *this );
            return *this;
        }

#ifdef LOKI_ENABLE_FRIEND_TEMPLATE_TEMPLATE_PARAMETER_WORKAROUND

        // old non standard in class definition of friends
        friend inline void Release(SmartPtr& sp, typename SP::StoredType& p)
        {
            p = GetImplRef(sp);
            GetImplRef(sp) = SP::Default();
        }

        friend inline void Reset(SmartPtr& sp, typename SP::StoredType p)
        { SmartPtr(p).Swap(sp); }

#else

        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        friend void Release(SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1>& sp,
                            typename SP1<T1>::StoredType& p);

        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        friend void Reset(SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1>& sp,
                          typename SP1<T1>::StoredType p);
#endif


        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        bool Merge( SmartPtr< T1, OP1, CP1, KP1, SP1, CNP1 > & rhs )
        {
            if ( GetImpl( *this ) != GetImpl( rhs ) )
            {
                return false;
            }
            return OP::template Merge( rhs );
        }

        PointerType operator->()
        {
            KP::OnDereference(GetImplRef(*this));
            return SP::operator->();
        }

        ConstPointerType operator->() const
        {
            KP::OnDereference(GetImplRef(*this));
            return SP::operator->();
        }

        ReferenceType operator*()
        {
            KP::OnDereference(GetImplRef(*this));
            return SP::operator*();
        }

        ConstReferenceType operator*() const
        {
            KP::OnDereference(GetImplRef(*this));
            return SP::operator*();
        }

        /** operator[] returns a reference to an modifiable object. If the index is greater than or
         equal to the number of elements, the function will throw a std::out_of_range exception.
         This only works with DeleteArray policy. Any other policy will cause a compiler error.
         */
        ReferenceType operator [] ( size_t index )
        {
            PointerType p = SP::operator->();
            KP::OnDereference( p );
            SP::OnCheckRange( index );
            return p[ index ];
        }

        /** operator[] returns a reference to a const object. If the index is greater than or
         equal to the number of elements, the function will throw a std::out_of_range exception.
         This only works with DeleteArray policy. Any other policy will cause a compiler error.
         */
        ConstReferenceType operator [] ( size_t index ) const
        {
            ConstPointerType p = SP::operator->();
            KP::OnDereference( p );
            SP::OnCheckRange( index );
            return p[ index ];
        }

        bool operator!() const // Enables "if (!sp) ..."
        { return GetImpl(*this) == 0; }

        static inline T * GetPointer( const SmartPtr & sp )
        { return GetImpl( sp ); }

        // Ambiguity buster
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        bool operator==(const SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs) const
        { return GetImpl(*this) == GetImpl(rhs); }

        // Ambiguity buster
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        bool operator!=(const SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs) const
        { return !(*this == rhs); }

        // Ambiguity buster
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        bool operator<(const SmartPtr<T1, OP1, CP1, KP1, SP1, CNP1 >& rhs) const
        { return GetImpl(*this) < GetImpl(rhs); }

        // Ambiguity buster
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        inline bool operator > ( const SmartPtr< T1, OP1, CP1, KP1, SP1, CNP1 > & rhs )
        {
            return ( GetImpl( rhs ) < GetImpl( *this ) );
        }

        // Ambiguity buster
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        inline bool operator <= ( const SmartPtr< T1, OP1, CP1, KP1, SP1, CNP1 > & rhs )
        {
            return !( GetImpl( rhs ) < GetImpl( *this ) );
        }

        // Ambiguity buster
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            template <class> class CNP1
        >
        inline bool operator >= ( const SmartPtr< T1, OP1, CP1, KP1, SP1, CNP1 > & rhs )
        {
            return !( GetImpl( *this ) < GetImpl( rhs ) );
        }

    private:
        // Helper for enabling 'if (sp)'
        struct Tester
        {
            Tester(int) {}
            void dummy() {}
        };

        typedef void (Tester::*unspecified_boolean_type_)();

        typedef typename Select<CP::allow, Tester, unspecified_boolean_type_>::Result
            unspecified_boolean_type;

    public:
        // enable 'if (sp)'
        operator unspecified_boolean_type() const
        {
            return !*this ? 0 : &Tester::dummy;
        }

    private:
        // Helper for disallowing automatic conversion
        struct Insipid
        {
            Insipid(PointerType) {}
        };

        typedef typename Select<CP::allow, PointerType, Insipid>::Result
            AutomaticConversionResult;

    public:
        operator AutomaticConversionResult() const
        { return GetImpl(*this); }
    };


////////////////////////////////////////////////////////////////////////////////
// friends
////////////////////////////////////////////////////////////////////////////////

#ifndef LOKI_ENABLE_FRIEND_TEMPLATE_TEMPLATE_PARAMETER_WORKAROUND

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP
    >
    inline void Release(SmartPtr<T, OP, CP, KP, SP, CNP>& sp,
                        typename SP<T>::StoredType& p)
    {
      p = GetImplRef(sp);
      GetImplRef(sp) = SP<T>::Default();
    }

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP
    >
    inline void Reset(SmartPtr<T, OP, CP, KP, SP, CNP>& sp,
                      typename SP<T>::StoredType p)
    { SmartPtr<T, OP, CP, KP, SP, CNP>(p).Swap(sp); }

#endif

////////////////////////////////////////////////////////////////////////////////
// free comparison operators for class template SmartPtr
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
///  operator== for lhs = SmartPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP1,
        typename U
    >
    inline bool operator==(const SmartPtr<T, OP, CP, KP, SP, CNP1 >& lhs,
        U* rhs)
    { return ( GetImpl( lhs ) == rhs ); }

////////////////////////////////////////////////////////////////////////////////
///  operator== for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP1,
        typename U
    >
    inline bool operator==(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP1 >& rhs)
    { return ( GetImpl( rhs ) == lhs ); }

////////////////////////////////////////////////////////////////////////////////
///  operator!= for lhs = SmartPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator!=(const SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
        U* rhs)
    { return ( GetImpl( lhs ) != rhs ); }

////////////////////////////////////////////////////////////////////////////////
///  operator!= for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator!=(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP >& rhs)
    { return ( GetImpl( rhs ) != lhs ); }

////////////////////////////////////////////////////////////////////////////////
///  operator< for lhs = SmartPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator<(const SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
        U* rhs)
    {
        return ( GetImpl( lhs ) < rhs );
    }

////////////////////////////////////////////////////////////////////////////////
///  operator< for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator<(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP >& rhs)
    {
        return ( lhs < GetImpl( rhs ) );
    }

////////////////////////////////////////////////////////////////////////////////
//  operator> for lhs = SmartPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator>(const SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
        U* rhs)
    { return rhs < GetImpl( lhs ); }

////////////////////////////////////////////////////////////////////////////////
///  operator> for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator>(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP >& rhs)
    { return rhs < lhs; }

////////////////////////////////////////////////////////////////////////////////
///  operator<= for lhs = SmartPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator<=(const SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
        U* rhs)
    { return !(rhs < lhs); }

////////////////////////////////////////////////////////////////////////////////
///  operator<= for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator<=(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP >& rhs)
    { return !(rhs < lhs); }

////////////////////////////////////////////////////////////////////////////////
///  operator>= for lhs = SmartPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator>=(const SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
        U* rhs)
    { return !(lhs < rhs); }

////////////////////////////////////////////////////////////////////////////////
///  operator>= for lhs = raw pointer, rhs = SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP,
        typename U
    >
    inline bool operator>=(U* lhs,
        const SmartPtr<T, OP, CP, KP, SP, CNP >& rhs)
    { return !(lhs < rhs); }

} // namespace Loki

////////////////////////////////////////////////////////////////////////////////
///  specialization of std::less for SmartPtr
///  \ingroup SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////

namespace std
{
    template
    <
        typename T,
        template <class> class OP,
        class CP,
        template <class> class KP,
        template <class> class SP,
        template <class> class CNP
    >
    struct less< Loki::SmartPtr<T, OP, CP, KP, SP, CNP > >
        : public binary_function<Loki::SmartPtr<T, OP, CP, KP, SP, CNP >,
            Loki::SmartPtr<T, OP, CP, KP, SP, CNP >, bool>
    {
        bool operator()(const Loki::SmartPtr<T, OP, CP, KP, SP, CNP >& lhs,
            const Loki::SmartPtr<T, OP, CP, KP, SP, CNP >& rhs) const
        { return less<T*>()(GetImpl(lhs), GetImpl(rhs)); }
    };
}

// ----------------------------------------------------------------------------

#if defined( _MSC_VER )
    #pragma warning( pop )
#endif

#endif // end file guardian
