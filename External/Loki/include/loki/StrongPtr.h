////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2006 Rich Sposato
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
#ifndef LOKI_STRONG_PTR_INC_
#define LOKI_STRONG_PTR_INC_

// $Id$


#include <loki/SmartPtr.h>
#if defined (LOKI_OBJECT_LEVEL_THREADING) || defined (LOKI_CLASS_LEVEL_THREADING)
    #include <loki/Threads.h>
#endif

#if defined( _MSC_VER )
    #pragma warning( push )
    #pragma warning( disable: 4355 )
#endif


////////////////////////////////////////////////////////////////////////////////
///
///  \par Terminology
///   These terms are used within this file's comments.
///   -# StrongPtr : Class used to implement both strong and weak pointers. The
///      second template parameter determines if a StrongPtr is weak or strong.
///   -# Strong pointer : A pointer that claims ownership of a shared object.
///      When the last strong copointer dies, the object is destroyed even if
///      there are weak copointers.
///   -# Weak pointer : A pointer that does not own the shared object it points
///       to.  It only destroys the shared object if there no strong copointers
///       exist when it dies.
///   -# Copointers : All the pointers that refer to the same shared object.
///      The copointers must have the same ownership policy, but the other
///      policies may be different.
///   -# Pointee : The shared object.
///
///  \par OwnershipPolicy
///   The ownership policy has the pointer to the actual object, and it also
///   keeps track of the strong and weak copointers so that it can know if any
///   strong copointers remain.  The plain pointer it maintains is stored as a
///   void pointer, which allows the ownership policy classes to be monolithic
///   classes instead of template classes.  As monolithic classes, they reduce
///   amount of code-bloat.
///
///  \par Writing Your Own OwnershipPolicy
///   If you write your own policy, you must implement these 12 functions:
///   -# explicit YourPolicy( bool strong )
///   -# YourPolicy( void * p, bool strong )
///   -# YourPolicy( const YourPolicy & rhs, bool strong )
///   -# YourPolicy( const YourPolicy & rhs, bool isNull, bool strong )
///   -# ~YourPolicy( void )
///   -# bool Release( bool strong )
///   -# bool HasStrongPointer( void ) const
///   -# void Swap( YourPolicy & rhs )
///   -# void SetPointer( void * p )
///   -# void ZapPointer( void )
///   -# void * GetPointer( void ) const
///   -# void * & GetPointerRef( void ) const
///   It is strongly recommended that all 12 of these functions be protected
///   instead of public.  These two functions are optional for single-threaded
///   policies, but required for multi-threaded policies:
///   -# void Lock( void ) const
///   -# void Unlock( void ) const
///   This function is entirely optional:
///   -# bool Merge( TwoRefLinks & rhs )
///
///  \par DeletePolicy
///   The delete policy provides a mechanism to destroy an object and a default
///   value for an uninitialized pointer.  You can override this policy with
///   your own when using the Singleton, NullObject, or Prototype design
///   patterns.
///
///  \par Writing Your Own DeletePolicy
///   If you write your own policy, you must implement these 3 functions:
///   -# void static Delete( const P * p )
///   -# static P * Default( void )
///   -# Default constructor.
///   -# Copy constructor.
///   -# Templated copy constructor.
///   -# void Swap( YourDeletePolicy & )
///
///  \par ResetPolicy
///   A reset policy tells the ReleaseAll and ResetAll functions whether they
///   should release or reset the StrongPtr copointers.  These functions do
///   not affect just one StrongPtr, but all copointers.  That is unlike
///   SmartPtr where the Release and Reset functions only affect 1 SmartPtr,
///   and leave all copointers untouched.  A useful trick you can do with the
///   ResetPolicy is to not allow reset when a strong pointer exists, and then
///   use the NoCheck policy for all strong pointers.  The reset policy
///   guarantees the strong pointers always have a valid pointee, so checking
///   is not required; but weak pointers may still require checking.
///
///  \par Writing Your Own ResetPolicy
///   If you write your own policy, you must implement these 2 functions:
///   -# bool OnReleaseAll( bool, bool ) const
///   -# bool OnResetAll( bool, bool ) const
///   The first bool parameter is true if the pointer which called the function
///   is strong.  The second parameter is true if any copointer is strong.  The
///   return value means the pointer can be reset or released.
///
///  \defgroup  StrongPointerOwnershipGroup StrongPtr Ownership policies
///  \ingroup   SmartPointerGroup
///  \defgroup  StrongPointerDeleteGroup Delete policies
///  \ingroup   SmartPointerGroup
///  \defgroup  StrongPointerResetGroup Reset policies
///  \ingroup   SmartPointerGroup
////////////////////////////////////////////////////////////////////////////////


namespace Loki
{

static const char * const StrongPtr_Single_Owner_Exception_Message =
    "Object has more than one Owner - which violates the single owner policy for StrongPtr!";


////////////////////////////////////////////////////////////////////////////////
///  \class DeleteUsingFree
///
///  \ingroup  StrongPointerDeleteGroup
///  Implementation of the DeletePolicy used by StrongPtr.  Uses explicit call
///   to T's destructor followed by call to free.  This policy is useful for
///   managing the lifetime of pointers to structs returned by C functions.
////////////////////////////////////////////////////////////////////////////////

template < class P >
class DeleteUsingFree
{
protected:

    inline DeleteUsingFree( void ) {}

    inline DeleteUsingFree( const DeleteUsingFree & ) {}

    template < class P1 >
    inline DeleteUsingFree( const DeleteUsingFree< P1 > & ) {}

    inline void static Delete( const P * p )
    {
        if ( 0 != p )
        {
            p->~P();
            ::free( p );
        }
    }

    /// Provides default value to initialize the pointer
    inline static P * Default( void )
    {
        return 0;
    }

    inline void Swap( DeleteUsingFree & ) {}
};

////////////////////////////////////////////////////////////////////////////////
///  \class DeleteNothing
///
///  \ingroup  StrongPointerDeleteGroup
///  Implementation of the DeletePolicy used by StrongPtr.  This will never
///   delete anything.  You can use this policy with pointers to an undefined
///   type or a pure interface class with a protected destructor.
////////////////////////////////////////////////////////////////////////////////

template < class P >
class DeleteNothing
{
protected:

    inline DeleteNothing( void ) {}

    inline DeleteNothing( const DeleteNothing & ) {}

    template < class P1 >
    inline DeleteNothing( const DeleteNothing< P1 > & ) {}

    inline static void Delete( const P * )
    {
        // Do nothing at all!
    }

    inline static P * Default( void )
    {
        return 0;
    }

    inline void Swap( DeleteNothing & ) {}

};

////////////////////////////////////////////////////////////////////////////////
///  \class DeleteSingle
///
///  \ingroup  StrongPointerDeleteGroup
///  Implementation of the DeletePolicy used by StrongPtr.  This deletes just
///   one shared object.  This is the default class for the DeletePolicy.
////////////////////////////////////////////////////////////////////////////////

template < class P >
class DeleteSingle
{
protected:

    inline DeleteSingle( void ) {}

    inline DeleteSingle( const DeleteSingle & ) {}

    template < class P1 >
    inline DeleteSingle( const DeleteSingle< P1 > & ) {}

    inline static void Delete( const P * p )
    {
        /** @note If you see an error message about a negative subscript, that
         means your are attempting to use Loki to delete an incomplete type.
         Please don't use this policy with incomplete types; you may want to
         use DeleteNothing instead.
         */
        typedef char Type_Must_Be_Defined[ sizeof(P) ? 1 : -1 ];
        delete p;
    }

    inline static P * Default( void )
    {
        return 0;
    }

    inline void Swap( DeleteSingle & ) {}
};

////////////////////////////////////////////////////////////////////////////////
///  \class DeleteArray
///
///  \ingroup  StrongPointerDeleteGroup
///  Implementation of the DeletePolicy used by StrongPtr.  This deletes an
///   array of shared objects.
////////////////////////////////////////////////////////////////////////////////

template < class P >
class DeleteArray : public ::Loki::Private::DeleteArrayBase
{
public:

    DeleteArray( void ) : DeleteArrayBase() {}

    explicit DeleteArray( size_t itemCount ) : DeleteArrayBase( itemCount ) {}

    DeleteArray( const DeleteArray & that ) : DeleteArrayBase( that ) {}

    template < class P1 >
    inline DeleteArray( const DeleteArray< P1 > & that ) : DeleteArrayBase( that ) {}

    inline static void Delete( const P * p )
    {
        /** @note If you see an error message about a negative subscript, that
         means your are attempting to use Loki to delete an incomplete type.
         Please don't use this policy with incomplete types; you may want to
         use DeleteNothing instead.
         */
        typedef char Type_Must_Be_Defined[ sizeof(P) ? 1 : -1 ];
        delete [] p;
    }

    inline static P * Default( void )
    {
        return 0;
    }

};

////////////////////////////////////////////////////////////////////////////////
///  \class CantResetWithStrong
///
///  \ingroup  StrongPointerResetGroup
///  Implementation of the ResetPolicy used by StrongPtr.  This is the default
///   ResetPolicy for StrongPtr.  It forbids reset and release only if a strong
///   copointer exists.
////////////////////////////////////////////////////////////////////////////////

template < class P >
struct CantResetWithStrong
{
    inline bool OnReleaseAll( bool isThisStrong, bool isAnyStrong ) const
    {
        (void)isThisStrong;
        return ! isAnyStrong;
    }

    inline bool OnResetAll( bool isThisStrong, bool isAnyStrong ) const
    {
        (void)isThisStrong;
        return ! isAnyStrong;
    }
};

////////////////////////////////////////////////////////////////////////////////
///  \class OnlyStrongMayReset
///
///  \ingroup  StrongPointerResetGroup
///  Implementation of the ResetPolicy used by StrongPtr.  It only allows a
///  a strong co-pointer to reset or release.  This policy was made for use with
///  the single-owner policies.
////////////////////////////////////////////////////////////////////////////////

template < class P >
struct OnlyStrongMayReset
{
    inline bool OnReleaseAll( bool isThisStrong, bool isAnyStrong ) const
    {
        (void)isAnyStrong;
        return isThisStrong;
    }

    inline bool OnResetAll( bool isThisStrong, bool isAnyStrong ) const
    {
        (void)isAnyStrong;
        return isThisStrong;
    }
};

////////////////////////////////////////////////////////////////////////////////
///  \class AllowReset
///
///  \ingroup  StrongPointerResetGroup
///  Implementation of the ResetPolicy used by StrongPtr.  It allows reset and
///   release under any circumstance.
////////////////////////////////////////////////////////////////////////////////

template < class P >
struct AllowReset
{
    inline bool OnReleaseAll( bool isThisStrong, bool isAnyStrong ) const
    {
        (void)isThisStrong;
        (void)isAnyStrong;
        return true;
    }
    inline bool OnResetAll( bool isThisStrong, bool isAnyStrong ) const
    {
        (void)isThisStrong;
        (void)isAnyStrong;
        return true;
    }
};

////////////////////////////////////////////////////////////////////////////////
///  \class NeverReset
///
///  \ingroup  StrongPointerResetGroup
///  Implementation of the ResetPolicy used by StrongPtr.  It forbids reset and
///   release under any circumstance.
////////////////////////////////////////////////////////////////////////////////

template < class P >
struct NeverReset
{
    inline bool OnReleaseAll( bool isThisStrong, bool isAnyStrong ) const
    {
        (void)isThisStrong;
        (void)isAnyStrong;
        return false;
    }
    inline bool OnResetAll( bool isThisStrong, bool isAnyStrong ) const
    {
        (void)isThisStrong;
        (void)isAnyStrong;
        return false;
    }
};

// ----------------------------------------------------------------------------

// Forward declaration needed for pointer to single-owner strong pointer.
class SingleOwnerRefCount;
class Lockable1OwnerRefCount;

namespace Private
{

////////////////////////////////////////////////////////////////////////////////
///  \class TwoRefCountInfo
///
///  \ingroup  StrongPointerOwnershipGroup
///   Implementation detail for reference counting strong and weak pointers.
///   It maintains a void pointer and 2 reference counts.  Since it is just a
///   class for managing implementation details, it is not intended to be used
///   directly - which is why it is in a private namespace.  Each instance is a
///   shared resource for all copointers, and there should be only one of these
///   for each set of copointers.  This class is small, trivial, and inline.
////////////////////////////////////////////////////////////////////////////////

class LOKI_EXPORT TwoRefCountInfo
{
public:

    inline explicit TwoRefCountInfo( bool strong )
        : m_pointer( 0 )
        , m_strongCount( strong ? 1 : 0 )
        , m_weakCount( strong ? 0 : 1 )
    {
    }

    inline TwoRefCountInfo( void * p, bool strong )
        : m_pointer( p )
        , m_strongCount( strong ? 1 : 0 )
        , m_weakCount( strong ? 0 : 1 )
    {
    }

    inline ~TwoRefCountInfo( void )
    {
        assert( 0 == m_strongCount );
        assert( 0 == m_weakCount );
    }

    inline bool HasStrongPointer( void ) const
    {
        return ( 0 < m_strongCount );
    }

    inline bool HasWeakPointer( void ) const
    {
        return ( 0 < m_weakCount );
    }

    inline void IncStrongCount( void )
    {
        ++m_strongCount;
    }

    inline void IncWeakCount( void )
    {
        ++m_weakCount;
    }

    inline bool DecStrongCount( void )
    {
        assert( 0 < m_strongCount );
        --m_strongCount;
        const bool isZero = ( 0 == m_strongCount );
        return isZero;
    }

    inline void DecWeakCount( void )
    {
        assert( 0 < m_weakCount );
        --m_weakCount;
    }

    inline void ZapPointer( void )
    {
        m_pointer = 0;
    }

    void SetPointer( void * p )
    {
        m_pointer = p;
    }

    inline void * GetPointer( void ) const
    {
        return m_pointer;
    }

    inline void * & GetPointerRef( void ) const
    {
        return const_cast< void * & >( m_pointer );
    }

private:
    /// Default constructor not implemented.
    TwoRefCountInfo( void );
    /// Copy-constructor not implemented.
    TwoRefCountInfo( const TwoRefCountInfo & );
    /// Copy-assignment operator not implemented.
    TwoRefCountInfo & operator = ( const TwoRefCountInfo & );

    void * m_pointer;
    unsigned int m_strongCount;
    unsigned int m_weakCount;
};

////////////////////////////////////////////////////////////////////////////////
///  \class OneOwnerRefCountInfo
///
///  \ingroup  StrongPointerOwnershipGroup
///   Implementation detail for reference counting strong and weak pointers.
///   It maintains a void pointer and 2 reference counts.  Since it is just a
///   class for managing implementation details, it is not intended to be used
///   directly - which is why it is in a private namespace.  Each instance is a
///   shared resource for all copointers, and there should be only one of these
///   for each set of copointers.  This class is small, trivial, and inline.
////////////////////////////////////////////////////////////////////////////////

class LOKI_EXPORT OneOwnerRefCountInfo
{
public:

    explicit OneOwnerRefCountInfo( SingleOwnerRefCount * ptr );

    OneOwnerRefCountInfo( const void * p, SingleOwnerRefCount * ptr );

    inline ~OneOwnerRefCountInfo( void )
    {
        assert( NULL == m_strongPtr );
        assert( 0 == m_weakCount );
    }

    inline bool HasStrongPointer( void ) const
    {
        return ( NULL != m_strongPtr );
    }

    inline const SingleOwnerRefCount * GetStrongCoPointer( void ) const
    {
        return m_strongPtr;
    }

    inline SingleOwnerRefCount * GetStrongCoPointer( void )
    {
        return m_strongPtr;
    }

    void SetStrongCoPtr( SingleOwnerRefCount * ptr );

    inline bool HasWeakPointer( void ) const
    {
        return ( 0 < m_weakCount );
    }

    inline unsigned int GetWeakCount( void ) const
    {
        return m_weakCount;
    }

    inline void IncWeakCount( void )
    {
        ++m_weakCount;
    }

    inline void DecWeakCount( void )
    {
        assert( 0 < m_weakCount );
        --m_weakCount;
    }

    inline void ZapPointer( void )
    {
        m_pointer = NULL;
    }

    void SetPointer( void * p )
    {
        m_pointer = p;
    }

    inline void * GetPointer( void ) const
    {
        return const_cast< void * >( m_pointer );
    }

    inline void * & GetPointerRef( void ) const
    {
        return const_cast< void * & >( m_pointer );
    }

private:
    /// Default constructor not implemented.
    OneOwnerRefCountInfo( void );
    /// Copy constructor not implemented.
    OneOwnerRefCountInfo( const OneOwnerRefCountInfo & );
    /// Copy-assignment operator not implemented.
    OneOwnerRefCountInfo & operator = ( const OneOwnerRefCountInfo & );

    const void * m_pointer;
    SingleOwnerRefCount * m_strongPtr;
    unsigned int m_weakCount;
};

#if defined (LOKI_OBJECT_LEVEL_THREADING) || defined (LOKI_CLASS_LEVEL_THREADING)

////////////////////////////////////////////////////////////////////////////////
///  \class LockableTwoRefCountInfo
///
///  \ingroup  StrongPointerOwnershipGroup
///   Implementation detail for thread-safe reference counting for strong and
///   weak pointers.  It uses TwoRefCountInfo to manage the pointer and counts.
///   All this does is provide a thread safety mechanism.  Since it is just a
///   class for managing implementation details, it is not intended to be used
///   directly - which is why it is in a private namespace.  Each instance is a
///   shared resource for all copointers, and there should be only one of these
///   for each set of copointers.  This class is small, trivial, and inline.
///
///  \note This class is not designed for use with a single-threaded model.
///   Tests using a single-threaded model will not run properly, but tests in a
///   multi-threaded model with either class-level-locking or object-level-locking
///   do run properly.
////////////////////////////////////////////////////////////////////////////////

class LOKI_EXPORT LockableTwoRefCountInfo
    : private Loki::Private::TwoRefCountInfo
{
public:

    inline explicit LockableTwoRefCountInfo( bool strong )
        : TwoRefCountInfo( strong )
        , m_Mutex()
    {
    }

    LockableTwoRefCountInfo( void * p, bool strong )
        : TwoRefCountInfo( p, strong )
        , m_Mutex()
    {
    }

    inline ~LockableTwoRefCountInfo( void )
    {
    }

    inline void Lock( void ) const
    {
        m_Mutex.Lock();
    }

    inline void Unlock( void ) const
    {
        m_Mutex.Unlock();
    }

    inline bool HasStrongPointer( void ) const
    {
        m_Mutex.Lock();
        const bool has = TwoRefCountInfo::HasStrongPointer();
        m_Mutex.Unlock();
        return has;
    }

    inline bool HasWeakPointer( void ) const
    {
        m_Mutex.Lock();
        const bool has = TwoRefCountInfo::HasWeakPointer();
        m_Mutex.Unlock();
        return has;
    }

    inline void IncStrongCount( void )
    {
        m_Mutex.Lock();
        TwoRefCountInfo::IncStrongCount();
        m_Mutex.Unlock();
    }

    inline void IncWeakCount( void )
    {
        m_Mutex.Lock();
        TwoRefCountInfo::IncWeakCount();
        m_Mutex.Unlock();
    }

    inline bool DecStrongCount( void )
    {
        m_Mutex.Lock();
        const bool isZero = TwoRefCountInfo::DecStrongCount();
        m_Mutex.Unlock();
        return isZero;
    }

    inline void DecWeakCount( void )
    {
        m_Mutex.Lock();
        TwoRefCountInfo::DecWeakCount();
        m_Mutex.Unlock();
    }

    inline void ZapPointer( void )
    {
        m_Mutex.Lock();
        TwoRefCountInfo::ZapPointer();
        m_Mutex.Unlock();
    }

    void SetPointer( void * p )
    {
        m_Mutex.Lock();
        TwoRefCountInfo::SetPointer( p );
        m_Mutex.Unlock();
    }

    inline void * GetPointer( void ) const
    {
        return TwoRefCountInfo::GetPointer();
    }

    inline void * & GetPointerRef( void ) const
    {
        return TwoRefCountInfo::GetPointerRef();
    }

private:
    /// Default constructor is not available.
    LockableTwoRefCountInfo( void );
    /// Copy constructor is not available.
    LockableTwoRefCountInfo( const LockableTwoRefCountInfo & );
    /// Copy-assignment operator is not available.
    LockableTwoRefCountInfo & operator = ( const LockableTwoRefCountInfo & );

    mutable LOKI_DEFAULT_MUTEX m_Mutex;
};

////////////////////////////////////////////////////////////////////////////////
///  \class Lockable1OwnerRefCountInfo
///
///  \ingroup  StrongPointerOwnershipGroup
///   Implementation detail for thread-safe reference counting for 1 strong and
///   multiple weak pointers.  It uses OneOwnerRefCountInfo to manage the pointers
///   and count.  All this does is provide a thread safety mechanism.  Since it
///   is just a class for managing implementation details, it is not intended to
///   be used directly - which is why it is in a private namespace.  Each instance
///   is a shared resource for all copointers, and there should be only one of
///   these for each set of copointers.  This class is small, trivial, and inline.
///
///  \note This class is not designed for use with a single-threaded model.
///   Tests using a single-threaded model will not run properly, but tests in a
///   multi-threaded model with either class-level-locking or object-level-locking
///   do run properly.
////////////////////////////////////////////////////////////////////////////////

class LOKI_EXPORT Lockable1OwnerRefCountInfo
    : private Loki::Private::OneOwnerRefCountInfo
{
public:

    explicit Lockable1OwnerRefCountInfo( Lockable1OwnerRefCount * ptr )
        : OneOwnerRefCountInfo( reinterpret_cast< SingleOwnerRefCount * >( ptr ) )
        , m_Mutex()
    {
    }

    Lockable1OwnerRefCountInfo( const void * p, Lockable1OwnerRefCount * ptr )
        : OneOwnerRefCountInfo( p,
            reinterpret_cast< SingleOwnerRefCount * >( ptr ) )
        , m_Mutex()
    {
    }

    inline ~Lockable1OwnerRefCountInfo( void )
    {
    }

    inline void Lock( void ) const
    {
        m_Mutex.Lock();
    }

    inline void Unlock( void ) const
    {
        m_Mutex.Unlock();
    }

    inline const Lockable1OwnerRefCount * GetStrongCoPointer( void ) const
    {
        m_Mutex.Lock();
        const SingleOwnerRefCount * ptr =
            OneOwnerRefCountInfo::GetStrongCoPointer();
        m_Mutex.Unlock();
        return reinterpret_cast< const Lockable1OwnerRefCount * >( ptr );
    }

    inline Lockable1OwnerRefCount * GetStrongCoPointer( void )
    {
        m_Mutex.Lock();
        SingleOwnerRefCount * ptr = OneOwnerRefCountInfo::GetStrongCoPointer();
        m_Mutex.Unlock();
        return reinterpret_cast< Lockable1OwnerRefCount * >( ptr );
    }

    inline void SetStrongCoPtr( Lockable1OwnerRefCount * ptr )
    {
        m_Mutex.Lock();
        SingleOwnerRefCount * p = reinterpret_cast< SingleOwnerRefCount * >( ptr );
        OneOwnerRefCountInfo::SetStrongCoPtr( p );
        m_Mutex.Unlock();
    }

    inline bool HasStrongPointer( void ) const
    {
        m_Mutex.Lock();
        const bool has = OneOwnerRefCountInfo::HasStrongPointer();
        m_Mutex.Unlock();
        return has;
    }

    inline unsigned int GetWeakCount( void ) const
    {
        m_Mutex.Lock();
        const unsigned int weakCount = OneOwnerRefCountInfo::HasWeakPointer();
        m_Mutex.Unlock();
        return weakCount;
    }

    inline bool HasWeakPointer( void ) const
    {
        m_Mutex.Lock();
        const bool has = OneOwnerRefCountInfo::HasWeakPointer();
        m_Mutex.Unlock();
        return has;
    }

    inline void IncWeakCount( void )
    {
        m_Mutex.Lock();
        OneOwnerRefCountInfo::IncWeakCount();
        m_Mutex.Unlock();
    }

    inline void DecWeakCount( void )
    {
        m_Mutex.Lock();
        OneOwnerRefCountInfo::DecWeakCount();
        m_Mutex.Unlock();
    }

    inline void ZapPointer( void )
    {
        m_Mutex.Lock();
        OneOwnerRefCountInfo::ZapPointer();
        m_Mutex.Unlock();
    }

    void SetPointer( void * p )
    {
        m_Mutex.Lock();
        OneOwnerRefCountInfo::SetPointer( p );
        m_Mutex.Unlock();
    }

    inline void * GetPointer( void ) const
    {
        return OneOwnerRefCountInfo::GetPointer();
    }

    inline void * & GetPointerRef( void ) const
    {
        return OneOwnerRefCountInfo::GetPointerRef();
    }

private:
    /// Default constructor is not available.
    Lockable1OwnerRefCountInfo( void );
    /// Copy constructor is not available.
    Lockable1OwnerRefCountInfo( const Lockable1OwnerRefCountInfo & );
    /// Copy-assignment operator is not available.
    Lockable1OwnerRefCountInfo & operator = ( const Lockable1OwnerRefCountInfo & );

    mutable LOKI_DEFAULT_MUTEX m_Mutex;
};

#endif // if object-level-locking or class-level-locking

} // end namespace Private

////////////////////////////////////////////////////////////////////////////////
///  \class TwoRefCounts
///
///  \ingroup  StrongPointerOwnershipGroup
///   This implementation of StrongPtr's OwnershipPolicy uses a pointer to a
///   shared instance of TwoRefCountInfo.  This is the default policy for
///   OwnershipPolicy.  Some functions are trivial enough to be inline, while
///   others are implemented elsewhere.  It is not thread safe, and is intended
///   for single-threaded environments.
////////////////////////////////////////////////////////////////////////////////

class LOKI_EXPORT TwoRefCounts
{
protected:

    explicit TwoRefCounts( bool strong );

    TwoRefCounts( const void * p, bool strong );

    TwoRefCounts( const TwoRefCounts & rhs, bool strong ) :
        m_counts( rhs.m_counts )
    {
        Increment( strong );
    }

    TwoRefCounts( const TwoRefCounts & rhs, bool isNull, bool strong );

    /** The destructor does not need to do anything since the call to
     ZapPointer inside StrongPtr::~StrongPtr will do the cleanup which
     this dtor would have done.
     */
    inline ~TwoRefCounts( void ) {}

    inline bool Release( bool strong )
    {
        return Decrement( strong );
    }

    inline bool HasStrongPointer( void ) const
    {
        return m_counts->HasStrongPointer();
    }

    void Swap( TwoRefCounts & rhs );

    void SetPointer( void * p )
    {
        m_counts->SetPointer( p );
    }

    void ZapPointer( void );

    inline void * & GetPointerRef( void ) const
    {
        return m_counts->GetPointerRef();
    }

    inline void * GetPointer( void ) const
    {
        return m_counts->GetPointer();
    }

private:

    /// Default constructor is not implemented.
    TwoRefCounts( void );
    /// Copy constructor is not implemented.
    TwoRefCounts( const TwoRefCounts & );
    /// Copy-assignment operator is not implemented.
    TwoRefCounts & operator = ( const TwoRefCounts & );

    void Increment( bool strong );

    bool Decrement( bool strong );

    /// Pointer to all shared data.
    Loki::Private::TwoRefCountInfo * m_counts;
};

////////////////////////////////////////////////////////////////////////////////
///  \class SingleOwnerRefCount
///
///  \ingroup  StrongPointerOwnershipGroup
///   This implementation of StrongPtr's OwnershipPolicy extends the ownership
///   policy class, TwoRefCounts, to enforce that only one StrongPtr may "own" a
///   resource.  The resource is destroyed only when its sole owner dies even if
///   weak pointers access it.  The constructors enforce the single-owner policy
///   by throwing a bad_logic exception if more than one strong pointer claims to
///   own the resource.  Use this policy when you want the code to specify that
///   only one object owns a resource.
///
///  \note This class is not designed for use with a multi-threaded model.
///   Tests using a multi-threaded model may not run properly.
///
///  \note If you use any single-owner class, you should also use the
///   OnlyStrongMayReset class for the ResetPolicy in StrongPtr.
///
///  \note All single-owner policies do not allow programmers to return a
///   a StrongPtr by value from a function, since that would temporarily create
///   an additional strong co-pointer.  Nor can programmers store any strong
///   co-pointers in a container that uses copy-in and copy-out semantics.  Once
///   C++ allows for move constructors, these limitations go away.
////////////////////////////////////////////////////////////////////////////////

class LOKI_EXPORT SingleOwnerRefCount
{
protected:

    explicit SingleOwnerRefCount( bool strong );

    SingleOwnerRefCount( const void * p, bool strong );

    SingleOwnerRefCount( const SingleOwnerRefCount & rhs,
        bool strong );

    SingleOwnerRefCount( const SingleOwnerRefCount & rhs,
        bool isNull, bool strong );

    /** The destructor should not anything since the call to ZapPointer inside
     StrongPtr::~StrongPtr will do the cleanup which this dtor would have done.
     By the time the dtor is called, the underlying pointer, m_info, is NULL.
     */
    inline ~SingleOwnerRefCount( void ) {}

    bool Release( bool strong );

    inline bool HasStrongPointer( void ) const
    {
        return ( NULL != m_info->GetStrongCoPointer() );
    }

    void Swap( SingleOwnerRefCount & rhs );

    void SetPointer( void * p );

    void ZapPointer( void );

    inline void * GetPointer( void ) const
    {
        return m_info->GetPointer();
    }

    inline void * & GetPointerRef( void ) const
    {
        return m_info->GetPointerRef();
    }

private:
    /// Default constructor is not implemented.
    SingleOwnerRefCount( void );
    /// Copy constructor is not implemented.
    SingleOwnerRefCount( const SingleOwnerRefCount & );
    /// Copy-assignment operator is not implemented.
    SingleOwnerRefCount & operator = ( const SingleOwnerRefCount & );

    inline bool IsStrong( void ) const
    {
        return ( this == m_info->GetStrongCoPointer() );
    }

    ::Loki::Private::OneOwnerRefCountInfo * m_info;

};

#if defined (LOKI_OBJECT_LEVEL_THREADING) || defined (LOKI_CLASS_LEVEL_THREADING)

////////////////////////////////////////////////////////////////////////////////
///  \class LockableTwoRefCounts
///
///  \ingroup  StrongPointerOwnershipGroup
///   This implementation of StrongPtr's OwnershipPolicy uses a pointer to a
///   shared instance of LockableTwoRefCountInfo.  It behaves very similarly to
///   TwoRefCounts, except that it provides thread-safety.  Some functions are
///   trivial enough to be inline, while others are implemented elsewhere.
///
///  \note This class is not designed for use with a single-threaded model.
///   Tests using a single-threaded model will not run properly, but tests in a
///   multi-threaded model with either class-level-locking or object-level-locking
///   do run properly.
////////////////////////////////////////////////////////////////////////////////

class LOKI_EXPORT LockableTwoRefCounts
{
    typedef SmallValueObject< ::Loki::ClassLevelLockable > ThreadSafePointerAllocator;

protected:

    explicit LockableTwoRefCounts( bool strong )
        : m_counts( NULL )
    {
        void * temp = ThreadSafePointerAllocator::operator new(
            sizeof(Loki::Private::LockableTwoRefCountInfo) );
#ifdef DO_EXTRA_LOKI_TESTS
        assert( temp != 0 );
#endif
        m_counts = new ( temp ) Loki::Private::LockableTwoRefCountInfo( strong );
    }

    LockableTwoRefCounts( const void * p, bool strong )
        : m_counts( NULL )
    {
        void * temp = ThreadSafePointerAllocator::operator new(
            sizeof(Loki::Private::LockableTwoRefCountInfo) );
#ifdef DO_EXTRA_LOKI_TESTS
        assert( temp != 0 );
#endif
        void * p2 = const_cast< void * >( p );
        m_counts = new ( temp )
            Loki::Private::LockableTwoRefCountInfo( p2, strong );
    }

    LockableTwoRefCounts( const LockableTwoRefCounts & rhs, bool strong ) :
        m_counts( rhs.m_counts )
    {
        Increment( strong );
    }

    LockableTwoRefCounts( const LockableTwoRefCounts & rhs, bool isNull, bool strong ) :
        m_counts( ( isNull ) ? NULL : rhs.m_counts )
    {
        if ( isNull )
        {
            void * temp = ThreadSafePointerAllocator::operator new(
                sizeof(Loki::Private::LockableTwoRefCountInfo) );
#ifdef DO_EXTRA_LOKI_TESTS
            assert( temp != 0 );
#endif
            m_counts = new ( temp ) Loki::Private::LockableTwoRefCountInfo( strong );
        }
        else
        {
            Increment( strong );
        }
    }

    /** The destructor does not need to do anything since the call to
     ZapPointer inside StrongPtr::~StrongPtr will do the cleanup which
     this dtor would have done.
     */
    inline ~LockableTwoRefCounts( void ) {}

    inline void Lock( void ) const
    {
        m_counts->Lock();
    }

    inline void Unlock( void ) const
    {
        m_counts->Unlock();
    }

    inline bool Release( bool strong )
    {
        return Decrement( strong );
    }

    void Increment( bool strong )
    {
        if ( strong )
        {
            m_counts->IncStrongCount();
        }
        else
        {
            m_counts->IncWeakCount();
        }
    }

    bool Decrement( bool strong )
    {
        bool noStrongPointers = false;
        if ( strong )
        {
            noStrongPointers = m_counts->DecStrongCount();
        }
        else
        {
            m_counts->DecWeakCount();
            noStrongPointers = !m_counts->HasStrongPointer();
        }
        return noStrongPointers;
    }

    bool HasStrongPointer( void ) const
    {
        return m_counts->HasStrongPointer();
    }

    void Swap( LockableTwoRefCounts & rhs )
    {
        ::std::swap( m_counts, rhs.m_counts );
    }

    void SetPointer( void * p )
    {
        m_counts->SetPointer( p );
    }

    void ZapPointer( void )
    {
#ifdef DO_EXTRA_LOKI_TESTS
        assert( !m_counts->HasStrongPointer() );
#endif
        if ( m_counts->HasWeakPointer() )
        {
            m_counts->ZapPointer();
        }
        else
        {
            ThreadSafePointerAllocator::operator delete ( m_counts,
                sizeof(Loki::Private::LockableTwoRefCountInfo) );
            m_counts = NULL;
        }
    }

    inline void * GetPointer( void ) const
    {
        return m_counts->GetPointer();
    }

    inline void * & GetPointerRef( void ) const
    {
        return m_counts->GetPointerRef();
    }

private:
    /// Default constructor is not implemented.
    LockableTwoRefCounts( void );
    /// Copy constructor is not implemented.
    LockableTwoRefCounts( const LockableTwoRefCounts & );
    /// Copy-assignment operator is not implemented.
    LockableTwoRefCounts & operator = ( const LockableTwoRefCounts & );

    /// Pointer to all shared data.
    Loki::Private::LockableTwoRefCountInfo * m_counts;
};

////////////////////////////////////////////////////////////////////////////////
///  \class Lockable1OwnerRefCount
///
///  \ingroup  StrongPointerOwnershipGroup
///   This implementation of StrongPtr's OwnershipPolicy extends the ownership
///   policy class, LockableTwoRefCounts, to enforce that only one StrongPtr
///   may "own" a resource.  The resource is destroyed only when its sole owner
///   dies regardless of how many weak pointers access it.  The constructors
///   enforce the single-owner policy by throwing a bad_logic exception if more
///   than one strong pointer claims to own the resource.  Use this policy when
///   you want the code to specify that only one object owns a resource.
///
///  \note This class is not designed for use with a single-threaded model.
///   Tests using a single-threaded model will not run properly, but tests in a
///   multi-threaded model with either class-level-locking or object-level-locking
///   do run properly.
///
///  \note If you use any single-owner class, you should also use the
///   OnlyStrongMayReset class for the ResetPolicy in StrongPtr.
///
///  \note All single-owner policies do not allow programmers to return a
///   a StrongPtr by value from a function, since that would temporarily create
///   an additional strong co-pointer.  Nor can programmers store any strong
///   co-pointers in a container that uses copy-in and copy-out semantics.  Once
///   C++ allows for move constructors, these limitations go away.
////////////////////////////////////////////////////////////////////////////////

class LOKI_EXPORT Lockable1OwnerRefCount
{
    typedef SmallValueObject< ::Loki::ClassLevelLockable > ThreadSafePointerAllocator;

protected:

    explicit Lockable1OwnerRefCount( bool strong )
        : m_info( NULL )
    {
        assert( NULL != this );

        void * temp = SmallObject<>::operator new(
            sizeof(Loki::Private::Lockable1OwnerRefCountInfo) );
#ifdef DO_EXTRA_LOKI_TESTS
        assert( temp != 0 );
#endif

        Lockable1OwnerRefCount * ptr = ( strong ) ? this : NULL;
        m_info = new ( temp )
            ::Loki::Private::Lockable1OwnerRefCountInfo( ptr );
    }

    Lockable1OwnerRefCount( const void * p, bool strong )
        : m_info( NULL )
    {
        assert( NULL != this );

        void * temp = SmallObject<>::operator new(
            sizeof(Loki::Private::Lockable1OwnerRefCountInfo) );
#ifdef DO_EXTRA_LOKI_TESTS
        assert( temp != 0 );
#endif

        Lockable1OwnerRefCount * ptr = ( strong ) ? this : NULL;
        m_info = new ( temp )
            ::Loki::Private::Lockable1OwnerRefCountInfo( p, ptr );
    }

    Lockable1OwnerRefCount( const Lockable1OwnerRefCount & rhs, bool strong ) :
        m_info( rhs.m_info )
    {
        assert( NULL != this );

        if ( strong && rhs.HasStrongPointer() )
        {
            throw ::std::logic_error( StrongPtr_Single_Owner_Exception_Message );
        }

        m_info = rhs.m_info;
        if ( strong )
        {
            m_info->SetStrongCoPtr( this );
        }
        else
        {
            m_info->IncWeakCount();
        }
    }

    Lockable1OwnerRefCount( const Lockable1OwnerRefCount & rhs,
        bool isNull, bool strong ) :
        m_info( ( isNull ) ? NULL : rhs.m_info )
    {
        assert( NULL != this );

        if ( isNull )
        {
            void * temp = SmallObject<>::operator new(
                sizeof(Loki::Private::Lockable1OwnerRefCountInfo) );
#ifdef DO_EXTRA_LOKI_TESTS
            assert( temp != 0 );
#endif

            Lockable1OwnerRefCount * ptr = ( strong ) ? this : NULL;
            m_info = new ( temp )
                ::Loki::Private::Lockable1OwnerRefCountInfo( ptr );
            return;
        }

        if ( strong && rhs.HasStrongPointer() )
        {
            throw ::std::logic_error( StrongPtr_Single_Owner_Exception_Message );
        }

        m_info = rhs.m_info;
        if ( strong )
        {
            m_info->SetStrongCoPtr( this );
        }
        else
        {
            m_info->IncWeakCount();
        }
    }

    /** The destructor does not need to do anything since the call to
     ZapPointer inside StrongPtr::~StrongPtr will do the cleanup which
     this dtor would have done.
     */
    inline ~Lockable1OwnerRefCount( void ) {}

    inline void Lock( void ) const
    {
        m_info->Lock();
    }

    inline void Unlock( void ) const
    {
        m_info->Unlock();
    }

    inline bool Release( bool strong )
    {
        assert( NULL != this );
        assert( strong == IsStrong() );

        if ( strong )
        {
            m_info->SetStrongCoPtr( NULL );
            return true;
        }

        m_info->Lock();
        assert( 0 < m_info->GetWeakCount() );
        m_info->DecWeakCount();
        const bool noOwner = ( !m_info->HasStrongPointer() );
        const bool doRelease = ( ( 0 == m_info->GetWeakCount() ) && noOwner );
        m_info->Unlock();
        return doRelease;
    }

    bool HasStrongPointer( void ) const
    {
        return m_info->HasStrongPointer();
    }

    void Swap( Lockable1OwnerRefCount & rhs )
    {
        assert( NULL != this );
        m_info->Lock();
        rhs.m_info->Lock();

        if ( IsStrong() && rhs.IsStrong() )
        {
            // These two strong pointers are trading resources.
            rhs.m_info->SetStrongCoPtr( this );
            m_info->SetStrongCoPtr( &rhs );
        }
        ::std::swap( m_info, rhs.m_info );
        m_info->Unlock();
        rhs.m_info->Unlock();
    }

    void SetPointer( void * p )
    {
        assert( NULL != this );
        if ( IsStrong() || ( 1 == m_info->GetWeakCount() ) )
        {
            // Only a strong pointer or the last weak pointer may change a resource.
            m_info->SetPointer( p );
        }
    }

    void ZapPointer( void )
    {
        assert( !m_info->HasStrongPointer() );
        if ( m_info->HasWeakPointer() )
        {
            m_info->ZapPointer();
        }
        else
        {
            SmallObject<>::operator delete ( m_info,
                sizeof(Loki::Private::Lockable1OwnerRefCountInfo) );
            m_info = NULL;
        }
    }

    inline void * GetPointer( void ) const
    {
        return m_info->GetPointer();
    }

    inline void * & GetPointerRef( void ) const
    {
        return m_info->GetPointerRef();
    }

private:
    /// Default constructor is not implemented.
    Lockable1OwnerRefCount( void );
    /// Copy constructor is not implemented.
    Lockable1OwnerRefCount( const Lockable1OwnerRefCount & );
    /// Copy-assignment operator is not implemented.
    Lockable1OwnerRefCount & operator = ( const Lockable1OwnerRefCount & );

    inline bool IsStrong( void ) const
    {
        return ( this == m_info->GetStrongCoPointer() );
    }

    /// Pointer to shared info about resource.
    ::Loki::Private::Lockable1OwnerRefCountInfo * m_info;

};

#endif // if object-level-locking or class-level-locking

////////////////////////////////////////////////////////////////////////////////
///  \class TwoRefLinks
///
///  \ingroup  StrongPointerOwnershipGroup
///   This implementation of StrongPtr's OwnershipPolicy uses a doubly-linked
///   cycle of copointers to a shared object. Some functions are trivial enough
///   to be inline, while others are implemented in elsewhere.  It is not thread
///   safe, and is intended for single-threaded environments.
////////////////////////////////////////////////////////////////////////////////

class LOKI_EXPORT TwoRefLinks
{
protected:

    inline explicit TwoRefLinks( bool strong )
        : m_pointer( 0 )
        , m_prev( this )
        , m_next( this )
        , m_strong( strong )
    {
    }

    TwoRefLinks( const void * p, bool strong );

    TwoRefLinks( const TwoRefLinks & rhs, bool strong );

    TwoRefLinks( const TwoRefLinks & rhs, bool isNull, bool strong );

    ~TwoRefLinks( void );

    bool Release( bool strong );

    void Swap( TwoRefLinks & rhs );

    bool Merge( TwoRefLinks & rhs );

    /// Returns pointer to next link in cycle is a strong pointer.
    const TwoRefLinks * GetNextStrongPointer( void ) const;

    bool HasStrongPointer( void ) const;

    unsigned int GetStrongPointerCount( void ) const;

    inline void ZapPointer( void )
    {
        ZapAllNodes();
    }

    void SetPointer( void * p );

    inline void * GetPointer( void ) const
    {
        return m_pointer;
    }

    inline void * & GetPointerRef( void ) const
    {
        return const_cast< void * & >( m_pointer );
    }

    inline bool IsStrong( void ) const
    {
        return m_strong;
    }

private:
    static unsigned int CountPrevCycle( const TwoRefLinks * pThis );
    static unsigned int CountNextCycle( const TwoRefLinks * pThis );

    /// Default constructor is not implemented.
    TwoRefLinks( void );
    /// Copy constructor is not implemented.
    TwoRefLinks( const TwoRefLinks & );
    /// Copy-assignment operator is not implemented.
    TwoRefLinks & operator = ( const TwoRefLinks & );

    bool HasPrevNode( const TwoRefLinks * p ) const;
    bool HasNextNode( const TwoRefLinks * p ) const;
    bool AllNodesHaveSamePointer( void ) const;
    void ZapAllNodes( void );

    bool IsValid( void ) const;

    void * m_pointer;
    mutable TwoRefLinks * m_prev;
    mutable TwoRefLinks * m_next;
    const bool m_strong;
};

////////////////////////////////////////////////////////////////////////////////
///  \class SingleOwnerRefLinks
///
///  \ingroup  StrongPointerOwnershipGroup
///   This implementation of StrongPtr's OwnershipPolicy extends the ownership
///   policy class, TwoRefLinks, to enforce that only one StrongPtr may "own" a
///   resource.  The resource is destroyed only when its sole owner dies even if
///   weak pointers access it.  The constructors enforce the single-owner policy
///   by throwing a bad_logic exception if more than one strong pointer claims to
///   own the resource.  Use this policy when you want the code to specify that
///   only one object owns a resource.
///
///  \note This class is not designed for use with a multi-threaded model.
///   Tests using a multi-threaded model may not run properly.
///
///  \note If you use any single-owner class, you should also use the
///   OnlyStrongMayReset class for the ResetPolicy in StrongPtr.
///
///  \note All single-owner policies do not allow programmers to return a
///   a StrongPtr by value from a function, since that would temporarily create
///   an additional strong co-pointer.  Nor can programmers store any strong
///   co-pointers in a container that uses copy-in and copy-out semantics.  Once
///   C++ allows for move constructors, these limitations go away.
////////////////////////////////////////////////////////////////////////////////

class LOKI_EXPORT SingleOwnerRefLinks : public TwoRefLinks
{
protected:

    explicit SingleOwnerRefLinks( bool strong );

    SingleOwnerRefLinks( const void * p, bool strong );

    SingleOwnerRefLinks( const SingleOwnerRefLinks & rhs, bool strong );

    SingleOwnerRefLinks( const SingleOwnerRefLinks & rhs, bool isNull, bool strong );

    ~SingleOwnerRefLinks( void );

    void Swap( SingleOwnerRefLinks & rhs );

    bool Merge( SingleOwnerRefLinks & rhs );

    bool Release( bool strong );

private:

    /// Default constructor is not implemented.
    SingleOwnerRefLinks( void );
    /// Copy constructor is not implemented.
    SingleOwnerRefLinks( const SingleOwnerRefLinks & );
    /// Copy-assignment operator is not implemented.
    SingleOwnerRefLinks & operator = ( const SingleOwnerRefLinks & );
};

////////////////////////////////////////////////////////////////////////////////
///  \class StrongPtr
///
///  \ingroup SmartPointerGroup
///
///  \param Strong           default = true,
///  \param OwnershipPolicy  default = TwoRefCounts,
///  \param ConversionPolicy default = DisallowConversion,
///  \param CheckingPolicy   default = AssertCheck,
///  \param ResetPolicy      default = CantResetWithStrong,
///  \param DeletePolicy     default = DeleteSingle
///  \param ConstnessPolicy  default = LOKI_DEFAULT_CONSTNESS
////////////////////////////////////////////////////////////////////////////////

template
<
    typename T,
    bool Strong = true,
    class OwnershipPolicy = ::Loki::TwoRefCounts,
    class ConversionPolicy = ::Loki::DisallowConversion,
    template < class > class CheckingPolicy = ::Loki::AssertCheck,
    template < class > class ResetPolicy = ::Loki::CantResetWithStrong,
    template < class > class DeletePolicy = ::Loki::DeleteSingle,
    template < class > class ConstnessPolicy = LOKI_DEFAULT_CONSTNESS
>
class StrongPtr
    : public OwnershipPolicy
    , public ConversionPolicy
    , public CheckingPolicy< T * >
    , public ResetPolicy< T >
    , public DeletePolicy< T >
{
    typedef ConversionPolicy CP;
    typedef CheckingPolicy< T * > KP;
    typedef ResetPolicy< T > RP;
    typedef DeletePolicy< T > DP;

public:

    typedef OwnershipPolicy OP;

    typedef T * StoredType;    // the type of the pointer
    typedef T * PointerType;   // type returned by operator->
    typedef T & ReferenceType; // type returned by operator*

    typedef typename ConstnessPolicy< T >::Type * ConstPointerType;
    typedef typename ConstnessPolicy< T >::Type & ConstReferenceType;

private:
    struct NeverMatched {};

#ifdef LOKI_SMARTPTR_CONVERSION_CONSTRUCTOR_POLICY
    typedef typename Select< CP::allow, const StoredType&, NeverMatched>::Result ImplicitArg;
    typedef typename Select<!CP::allow, const StoredType&, NeverMatched>::Result ExplicitArg;
#else
    typedef const StoredType& ImplicitArg;
    typedef typename Select<false, const StoredType&, NeverMatched>::Result ExplicitArg;
#endif

    /// StrongPtr uses this helper class to specify the dynamic-caster constructor.
    class DynamicCastHelper {};

    /// Private constructor is only used for dynamic-casting.
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    StrongPtr( const StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs,
        bool isNull, const DynamicCastHelper & helper )
        // Dynamic casting from T1 to T and saving result in ownership policy.
        : OP( rhs, isNull, Strong )
    {
        (void)helper; // do void cast to remove compiler warning.
    }

    /// Private constructor is only used for dynamic-casting.
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    StrongPtr( StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs,
        bool isNull, const DynamicCastHelper & helper )
        // Dynamic casting from T1 to T and saving result in ownership policy.
        : OP( rhs, isNull, Strong )
    {
        (void)helper; // do void cast to remove compiler warning.
    }

public:

    StrongPtr( void ) : OP( Strong ), DP()
    {
        KP::OnDefault( GetPointer() );
    }

    explicit StrongPtr( ExplicitArg p ) : OP( p, Strong ), DP()
    {
        KP::OnInit( GetPointer() );
    }

    StrongPtr( ImplicitArg p ) : OP( p, Strong ), DP()
    {
        KP::OnInit( GetPointer() );
    }

    /** This constructor was designed to only work with the DeleteArray policy. Using it with any
     other Delete policies will cause compiler errors. Call it with this syntax:
     "ThingyPtr sp2( new Thingy[ 4 ], 4 );" so the StrongPtr knows how many elements are in the
     array for range checking.
     */
    StrongPtr( ImplicitArg p, size_t itemCount ) : OP( p, Strong ), DP( itemCount )
    {
        KP::OnInit( GetPointer() );
        DP::OnInit( GetPointer() );
    }

    StrongPtr( const StrongPtr & rhs )
        : OP( rhs, Strong ), CP( rhs ), KP( rhs ), DP( rhs )
    {
    }

    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    StrongPtr(
        const StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs )
        : OP( rhs, Strong ), CP( rhs ), DP( rhs )
    {
    }

    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    StrongPtr(
        StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs )
        : OP( rhs, Strong ), CP( rhs ), DP( rhs )
    {
    }

    StrongPtr( RefToValue< StrongPtr > rhs )
        : OP( rhs, Strong ), KP( rhs ), CP( rhs ), DP( rhs )
    {
    }

    operator RefToValue< StrongPtr >( void )
    {
        return RefToValue< StrongPtr >( *this );
    }

    StrongPtr & operator = ( const StrongPtr & rhs )
    {
        if ( GetPointer() != rhs.GetPointer() )
        {
            StrongPtr temp( rhs );
            temp.Swap( *this );
        }
        return *this;
    }

    StrongPtr & operator = ( T * p )
    {
        if ( GetPointer() != p )
        {
            StrongPtr temp( p );
            Swap( temp );
        }
        return *this;
    }

    /** This function is equivalent to an assignment operator for StrongPtr's that use the
     DeleteArray policy where the programmer needs to write the equivalent of "sp = new P;".
     With DeleteArray, the programmer should write "sp.Assign( new [5] Thingy, 5 );" so the
     StrongPtr knows how many elements are in the array.
     */
    StrongPtr & Assign( T * p, size_t itemCount )
    {
        if ( GetPointer() != p )
        {
            StrongPtr temp( p, itemCount );
            Swap( temp );
        }
        return *this;
    }

    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    StrongPtr & operator = (
        const StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs )
    {
        if ( !rhs.Equals( GetPointer() ) )
        {
            StrongPtr temp( rhs );
            temp.Swap( *this );
        }
        return *this;
    }

    inline bool IsStrong( void ) const
    {
        return Strong;
    }

    void Swap( StrongPtr & rhs )
    {
        OP::Swap( rhs );
        CP::Swap( rhs );
        KP::Swap( rhs );
        DP::Swap( rhs );
    }

    ~StrongPtr()
    {
        if ( OP::Release( Strong ) )
        {
            // Must zap the pointer before deleteing the object. Otherwise a
            // cycle of weak pointers will lead to recursion, which leads to
            // to deleting the shared object multiple times, which leads to
            // undefined behavior.  Therefore, this must get pointer before
            // zapping it, and then delete the temp pointer.
            T * p = GetPointer();
            OP::ZapPointer();
            if ( p != 0 )
            {
                DP::Delete( p );
            }
        }
    }

    /// Dynamically-casts parameter pointer to the type specified by this SmartPtr type.
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    StrongPtr & DynamicCastFrom( const StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs )
    {
        typedef typename StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 >::PointerType RightPointerType;
        const StrongPtr & sp = reinterpret_cast< const StrongPtr & >( rhs );
        PointerType p = sp.GetPointer();
        const RightPointerType rp = reinterpret_cast< const RightPointerType >( p );
        p = dynamic_cast< const PointerType >( rp );
        const bool isNull = ( NULL == p );
        StrongPtr temp( rhs, isNull, DynamicCastHelper() );
        Swap( temp );
        return *this;
    }

    /// Dynamically-casts parameter pointer to the type specified by this SmartPtr type.
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    StrongPtr & DynamicCastFrom( StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs )
    {
        typedef typename StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 >::PointerType RightPointerType;
        StrongPtr & sp = reinterpret_cast< StrongPtr & >( rhs );
        PointerType p = sp.GetPointer();
        RightPointerType rp = reinterpret_cast< RightPointerType >( p );
        p = dynamic_cast< PointerType >( rp );
        const bool isNull = ( NULL == p );
        StrongPtr temp( rhs, isNull, DynamicCastHelper() );
        Swap( temp );
        return *this;
    }

#ifdef LOKI_ENABLE_FRIEND_TEMPLATE_TEMPLATE_PARAMETER_WORKAROUND

    // old non standard in class definition of friends
    friend bool ReleaseAll( StrongPtr & sp,
        typename StrongPtr::StoredType & p )
    {
        if ( !sp.RP::OnReleaseAll( sp.IsStrong(), sp.OP::HasStrongPointer() ) )
        {
            return false;
        }
        p = sp.GetPointer();
        sp.OP::SetPointer( sp.DP::Default() );
        return true;
    }

    friend bool ResetAll( StrongPtr & sp,
        typename StrongPtr::StoredType p )
    {
        if ( sp.OP::GetPointer() == p )
        {
            return true;
        }
        if ( !sp.RP::OnResetAll( sp.IsStrong(), sp.OP::HasStrongPointer() ) )
        {
            return false;
        }
        sp.DP::Delete( sp.GetPointer() );
        sp.OP::SetPointer( p );
        return true;
    }

#else

    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    friend bool ReleaseAll( StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & sp,
        typename StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 >::StoredType & p );


    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    friend bool ResetAll( StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & sp,
        typename StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 >::StoredType p );

#endif


    /** Merges ownership of two StrongPtr's that point to same shared object
      but are not copointers.  Requires Merge function in OwnershipPolicy.
      \return True for success, false if not pointer to same object.
     */
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    bool Merge( StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs )
    {
        if ( OP::GetPointer() != rhs.OP::GetPointer() )
        {
            return false;
        }
        return OP::Merge( rhs );
    }

    /** Locks StrongPtr so other threads can't affect pointer.  Requires the
     OwnershipPolicy to have Lock function.
     */
    void Lock( void )
    {
        OP::Lock();
    }

    /** Unlocks StrongPtr so other threads can affect pointer.  Requires the
     OwnershipPolicy to have Unlock function.
     */
    void Unlock( void )
    {
        OP::Unlock();
    }

    PointerType operator -> ()
    {
        KP::OnDereference( GetPointer() );
        return GetPointer();
    }

    ConstPointerType operator -> () const
    {
        KP::OnDereference( GetPointer() );
        return GetPointer();
    }

    ReferenceType operator * ()
    {
        KP::OnDereference( GetPointer() );
        return * GetPointer();
    }

    ConstReferenceType operator * () const
    {
        KP::OnDereference( GetPointer() );
        return * GetPointer();
    }

    /** operator[] returns a reference to an modifiable object. If the index is greater than or
     equal to the number of elements, the function will throw a std::out_of_range exception.
     This only works with DeleteArray policy. Any other policy will cause a compiler error.
     */
    ReferenceType operator [] ( size_t index )
    {
        PointerType p = GetPointer();
        KP::OnDereference( p );
        DP::OnCheckRange( index );
        return p[ index ];
    }

    /** operator[] returns a reference to a const object. If the index is greater than or
     equal to the number of elements, the function will throw a std::out_of_range exception.
     This only works with DeleteArray policy. Any other policy will cause a compiler error.
     */
    ConstReferenceType operator [] ( size_t index ) const
    {
        ConstPointerType p = GetPointer();
        KP::OnDereference( p );
        DP::OnCheckRange( index );
        return p[ index ];
    }

    /// Helper function which can be called to avoid exposing GetPointer function.
    template < class T1 >
    bool Equals( const T1 * p ) const
    {
        return ( GetPointer() == p );
    }

    /// Helper function which can be called to avoid exposing GetPointer function.
    template < class T1 >
    bool LessThan( const T1 * p ) const
    {
        return ( GetPointer() < p );
    }

    /// Helper function which can be called to avoid exposing GetPointer function.
    template < class T1 >
    bool GreaterThan( const T1 * p ) const
    {
        return ( GetPointer() > p );
    }

    /// Equality comparison operator is templated to handle ambiguity.
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    bool operator == (
        const StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs ) const
    {
        return ( rhs.Equals( GetPointer() ) );
    }

    /// Inequality comparison operator is templated to handle ambiguity.
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    bool operator != (
        const StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs ) const
    {
        return !( rhs.Equals( GetPointer() ) );
    }

    /// Less-than comparison operator is templated to handle ambiguity.
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    bool operator < (
        const StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs ) const
    {
        return ( rhs.GreaterThan( GetPointer() ) );
    }

    /// Greater-than comparison operator is templated to handle ambiguity.
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    inline bool operator > (
        const StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs ) const
    {
        return ( rhs.LessThan( GetPointer() ) );
    }

    /// Less-than-or-equal-to operator is templated to handle ambiguity.
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    inline bool operator <= (
        const StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs ) const
    {
        return !( rhs.LessThan( GetPointer() ) );
    }

    /// Greater-than-or-equal-to operator is templated to handle ambiguity.
    template
    <
        typename T1,
        bool S1,
        class OP1,
        class CP1,
        template < class > class KP1,
        template < class > class RP1,
        template < class > class DP1,
        template < class > class CNP1
    >
    inline bool operator >= (
        const StrongPtr< T1, S1, OP1, CP1, KP1, RP1, DP1, CNP1 > & rhs ) const
    {
        return !( rhs.GreaterThan( GetPointer() ) );
    }

    inline bool operator ! () const // Enables "if ( !sp ) ..."
    {
        return ( 0 == OP::GetPointer() );
    }

protected:

    inline PointerType GetPointer( void )
    {
        return reinterpret_cast< PointerType >( OP::GetPointer() );
    }

    inline ConstPointerType GetPointer( void ) const
    {
        return reinterpret_cast< ConstPointerType >( OP::GetPointer() );
    }

private:

    inline ReferenceType GetPointerRef( void )
    {
        return reinterpret_cast< ReferenceType >( OP::GetPointerRef() );
    }

    inline ConstReferenceType GetPointerRef( void ) const
    {
        return reinterpret_cast< ConstReferenceType >( OP::GetPointerRef() );
    }

    // Helper for enabling 'if (sp)'
    struct Tester
    {
        Tester(int) {}
        void dummy() {}
    };

    typedef void (Tester::*unspecified_boolean_type_)();

    typedef typename Select< CP::allow, Tester, unspecified_boolean_type_ >::Result
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

    typedef typename Select< CP::allow, PointerType, Insipid >::Result
        AutomaticConversionResult;

public:
    operator AutomaticConversionResult() const
    {
        return GetPointer();
    }

};

// ----------------------------------------------------------------------------

// friend functions

#ifndef LOKI_ENABLE_FRIEND_TEMPLATE_TEMPLATE_PARAMETER_WORKAROUND

template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
bool ReleaseAll( StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & sp,
                 typename StrongPtr< T, S, OP, CP, KP, RP, DP, CNP >::StoredType & p )
{
  if ( !sp.RP<T>::OnReleaseAll( sp.IsStrong(), sp.OP::HasStrongPointer() ) )
  {
    return false;
  }
  p = sp.GetPointer();
  sp.OP::SetPointer( sp.DP<T>::Default() );
  return true;
}

template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
bool ResetAll( StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & sp,
               typename StrongPtr< T, S, OP, CP, KP, RP, DP, CNP >::StoredType p )
{
  if ( sp.OP::GetPointer() == p )
  {
    return true;
  }
  if ( !sp.RP<T>::OnResetAll( sp.IsStrong(), sp.OP::HasStrongPointer() ) )
  {
    return false;
  }
  sp.DP<T>::Delete( sp.GetPointer() );
  sp.OP::SetPointer( p );
  return true;
}
#endif


// free comparison operators for class template StrongPtr

///  operator== for lhs = StrongPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator == (
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & lhs, U * rhs )
{
    return ( lhs.Equals( rhs ) );
}

///  operator== for lhs = raw pointer, rhs = StrongPtr
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator == ( U * lhs,
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & rhs )
{
    return ( rhs.Equals( lhs ) );
}

///  operator!= for lhs = StrongPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator != (
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & lhs, U * rhs )
{
    return !( lhs.Equals( rhs ) );
}

///  operator!= for lhs = raw pointer, rhs = StrongPtr
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator != ( U * lhs,
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & rhs )
{
    return !( rhs.Equals( lhs ) );
}

///  operator< for lhs = StrongPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator < (
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & lhs, U * rhs )
{
    return ( lhs.LessThan( rhs ) );
}

///  operator< for lhs = raw pointer, rhs = StrongPtr
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator < ( U * lhs,
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & rhs )
{
    return ( rhs.GreaterThan( lhs ) );
}

//  operator> for lhs = StrongPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator > (
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & lhs, U * rhs )
{
    return ( lhs.GreaterThan( rhs ) );
}

///  operator> for lhs = raw pointer, rhs = StrongPtr
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator > ( U * lhs,
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & rhs )
{
    return ( rhs.LessThan( lhs ) );
}

///  operator<= for lhs = StrongPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator <= (
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & lhs, U * rhs )
{
    return !( lhs.GreaterThan( rhs ) );
}

///  operator<= for lhs = raw pointer, rhs = StrongPtr
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator <= ( U * lhs,
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & rhs )
{
    return !( rhs.LessThan( lhs ) );
}

///  operator>= for lhs = StrongPtr, rhs = raw pointer
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator >= (
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & lhs, U * rhs )
{
    return !( lhs.LessThan( rhs ) );
}

///  operator>= for lhs = raw pointer, rhs = StrongPtr
///  \ingroup SmartPointerGroup
template
<
    typename U,
    typename T,
    bool S,
    class OP,
    class CP,
    template < class > class KP,
    template < class > class RP,
    template < class > class DP,
    template < class > class CNP
>
inline bool operator >= ( U * lhs,
    const StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & rhs )
{
    return !( rhs.GreaterThan( lhs ) );
}

} // namespace Loki

namespace std
{
    ////////////////////////////////////////////////////////////////////////////////
    ///  specialization of std::less for StrongPtr
    ///  \ingroup SmartPointerGroup
    ////////////////////////////////////////////////////////////////////////////////
    template
    <
        typename T,
        bool S,
        class OP,
        class CP,
        template < class > class KP,
        template < class > class RP,
        template < class > class DP,
        template < class > class CNP
    >
    struct less< Loki::StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > >
        : public binary_function<
            Loki::StrongPtr< T, S, OP, CP, KP, RP, DP, CNP >,
            Loki::StrongPtr< T, S, OP, CP, KP, RP, DP, CNP >, bool >
    {
        bool operator () (
            const Loki::StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & lhs,
            const Loki::StrongPtr< T, S, OP, CP, KP, RP, DP, CNP > & rhs ) const
        {
            return ( lhs < rhs );
        }
    };
}

////////////////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER )
    #pragma warning( pop )
#endif

#endif // end file guardian
