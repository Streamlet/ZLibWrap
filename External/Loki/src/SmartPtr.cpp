////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// Copyright (c) 2006 Richard Sposato
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

// $Id$


#include <loki/SmartPtr.h>

#include <cassert>

#include <stdexcept>
#include <string>

//#define DO_EXTRA_LOKI_TESTS
#ifdef DO_EXTRA_LOKI_TESTS
    #include <iostream>
#endif


// ----------------------------------------------------------------------------

namespace Loki
{

namespace Private
{

// ----------------------------------------------------------------------------

void DeleteArrayBase::Swap( DeleteArrayBase & rhs )
{
    assert( NULL != this );

    const size_t temp = m_itemCount;
    m_itemCount = rhs.m_itemCount;
    rhs.m_itemCount = temp;
}

// ----------------------------------------------------------------------------

void DeleteArrayBase::OnInit( const void * p ) const
{
    assert( NULL != this );
    if ( NULL == p )
    {
        assert( 0 == m_itemCount );
    }
    else
    {
        assert( 0 < m_itemCount );
    }
}

// ----------------------------------------------------------------------------

void DeleteArrayBase::OnCheckRange( size_t index ) const
{
    assert( NULL != this );

    if ( index < m_itemCount )
        return;

    const ::std::string message( "index out of range in ::Loki::DeleteArrayBase::OnCheckRange" );
    throw ::std::out_of_range( message );
}

// ----------------------------------------------------------------------------

RefLinkedBase::RefLinkedBase( const RefLinkedBase & rhs ) :
    prev_( &rhs ),
    next_( rhs.next_ )
{
    prev_->next_ = this;
    next_->prev_ = this;

#ifdef DO_EXTRA_LOKI_TESTS
    assert( prev_->HasPrevNode( this ) );
    assert( next_->HasNextNode( this ) );
    assert( CountPrevCycle( this ) == CountNextCycle( this ) );
#endif
}

// ----------------------------------------------------------------------------

bool RefLinkedBase::Release()
{

#ifdef DO_EXTRA_LOKI_TESTS
    assert( prev_->HasPrevNode( this ) );
    assert( next_->HasNextNode( this ) );
    assert( CountPrevCycle( this ) == CountNextCycle( this ) );
#endif

    if ( NULL == next_ )
    {
        assert( NULL == prev_ );
        // Return false so it does not try to destroy shared object
        // more than once.
        return false;
    }
    else if (next_ == this)
    {
        assert(prev_ == this);
        // Set these to NULL to prevent re-entrancy.
        prev_ = NULL;
        next_ = NULL;
        return true;
    }

#ifdef DO_EXTRA_LOKI_TESTS
    assert( this != prev_ );
    assert( NULL != prev_ );
    assert( prev_->HasPrevNode( this ) );
    assert( next_->HasNextNode( this ) );
#endif

    prev_->next_ = next_;
    next_->prev_ = prev_;

#ifdef DO_EXTRA_LOKI_TESTS
    next_ = this;
    prev_ = this;
    assert( 1 == CountNextCycle( this ) );
    assert( 1 == CountPrevCycle( this ) );
#endif

    return false;
}

// ----------------------------------------------------------------------------

void RefLinkedBase::Swap(RefLinkedBase& rhs)
{

#ifdef DO_EXTRA_LOKI_TESTS
    assert( prev_->HasPrevNode( this ) );
    assert( next_->HasNextNode( this ) );
    assert( CountPrevCycle( this ) == CountNextCycle( this ) );
#endif

    if (next_ == this)
    {
        assert(prev_ == this);
        if (rhs.next_ == &rhs)
        {
            assert(rhs.prev_ == &rhs);
            // both lists are empty, nothing 2 do
            return;
        }
        prev_ = rhs.prev_;
        next_ = rhs.next_;
        prev_->next_ = next_->prev_ = this;
        rhs.next_ = rhs.prev_ = &rhs;
        return;
    }
    if (rhs.next_ == &rhs)
    {
        rhs.Swap(*this);
        return;
    }
    if (next_ == &rhs ) // rhs is next neighbour
    {
        if ( prev_ == &rhs )
            return;  // cycle of 2 pointers - no need to swap.
        std::swap(prev_, next_);
        std::swap(rhs.prev_, rhs.next_);
        std::swap(rhs.prev_, next_);
        std::swap(rhs.prev_->next_,next_->prev_);
    }
    else if ( prev_ == &rhs ) // rhs is prev neighbor
    {
        if ( next_ == &rhs )
            return;  // cycle of 2 pointers - no need to swap.
        std::swap( prev_, next_ );
        std::swap( rhs.next_, rhs.prev_ );
        std::swap( rhs.next_, prev_ );
        std::swap( rhs.next_->prev_, prev_->next_ );
    }
    else // not neighhbors
    {
        std::swap(prev_, rhs.prev_);
        std::swap(next_, rhs.next_);
        std::swap(prev_->next_, rhs.prev_->next_);
        std::swap(next_->prev_, rhs.next_->prev_);
    }

#ifdef DO_EXTRA_LOKI_TESTS
    assert( next_ == this ? prev_ == this : prev_ != this);
    assert( prev_ == this ? next_ == this : next_ != this);
    assert( prev_->HasPrevNode( this ) );
    assert( next_->HasNextNode( this ) );
    assert( CountPrevCycle( this ) == CountNextCycle( this ) );
    assert( rhs.prev_->HasPrevNode( &rhs ) );
    assert( rhs.next_->HasNextNode( &rhs ) );
    assert( CountPrevCycle( &rhs ) == CountNextCycle( &rhs ) );
#endif

}

// ----------------------------------------------------------------------------

unsigned int RefLinkedBase::CountPrevCycle( const RefLinkedBase * pThis )
{
    if ( NULL == pThis )
        return 0;
    const RefLinkedBase * p = pThis->prev_;
    if ( NULL == p )
        return 0;
    if ( pThis == p )
        return 1;

    unsigned int count = 1;
    do
    {
        p = p->prev_;
        ++count;
    } while ( p != pThis );

    return count;
}

// ----------------------------------------------------------------------------

unsigned int RefLinkedBase::CountNextCycle( const RefLinkedBase * pThis )
{
    if ( NULL == pThis )
        return 0;
    const RefLinkedBase * p = pThis->next_;
    if ( NULL == p )
        return 0;
    if ( pThis == p )
        return 1;

    unsigned int count = 1;
    while ( p != pThis )
    {
        p = p->next_;
        ++count;
    }

    return count;
}

// ----------------------------------------------------------------------------

bool RefLinkedBase::HasPrevNode( const RefLinkedBase * p ) const
{
    if ( this == p )
        return true;
    const RefLinkedBase * prev = prev_;
    if ( NULL == prev )
        return false;
    while ( prev != this )
    {
        if ( p == prev )
            return true;
        prev = prev->prev_;
    }
    return false;
}

// ----------------------------------------------------------------------------

bool RefLinkedBase::HasNextNode( const RefLinkedBase * p ) const
{
    if ( this == p )
        return true;
    const RefLinkedBase * next = next_;
    if ( NULL == next )
        return false;
    while ( next != this )
    {
        if ( p == next )
            return true;
        next = next->next_;
    }
    return false;
}

// ----------------------------------------------------------------------------

bool RefLinkedBase::Merge( RefLinkedBase & rhs )
{

    if ( NULL == next_ )
    {
        assert( NULL == prev_ );
        return false;
    }
    RefLinkedBase * prhs = &rhs;
    if ( prhs == this )
        return true;
    if ( NULL == prhs->next_ )
    {
        assert( NULL == prhs->prev_ );
        return true;
    }

    assert( CountPrevCycle( this ) == CountNextCycle( this ) );
    assert( CountPrevCycle( prhs ) == CountNextCycle( prhs ) );
    // If rhs node is already in this cycle, then no need to merge.
    if ( HasPrevNode( &rhs ) )
    {
        assert( HasNextNode( &rhs ) );
        return true;
    }

    if ( prhs == prhs->next_ )
    {
        /// rhs is in a cycle with 1 node.
        assert( prhs->prev_ == prhs );
        prhs->prev_ = prev_;
        prhs->next_ = this;
        prev_->next_ = prhs;
        prev_ = prhs;
    }
    else if ( this == next_ )
    {
        /// this is in a cycle with 1 node.
        assert( prev_ == this );
        prev_ = prhs->prev_;
        next_ = prhs;
        prhs->prev_->next_ = this;
        prhs->prev_ = this;
    }
    else
    {
        next_->prev_ = prhs->prev_;
        prhs->prev_->next_ = prev_;
        next_ = prhs;
        prhs->prev_ = this;
    }

    assert( CountPrevCycle( this ) == CountNextCycle( this ) );
    return true;
}

// ----------------------------------------------------------------------------

} // end namespace Private

} // end namespace Loki

