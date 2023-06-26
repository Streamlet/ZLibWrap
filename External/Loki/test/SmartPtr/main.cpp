////////////////////////////////////////////////////////////////////////////////
// Test program for The Loki Library
// Copyright (c) 2006 Richard Sposato
// Permission to use, copy, modify, distribute and sell this software for any
//     purpose is hereby granted without fee, provided that the above copyright
//     notice appear in all copies and that both that copyright notice and this
//     permission notice appear in supporting documentation.
// The authors make no representations about the
//     suitability of this software for any purpose. It is provided "as is"
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id$


// ----------------------------------------------------------------------------

#include <loki/SmartPtr.h>

#include <iostream>
#include <cstring>

#include "base.h"

// ----------------------------------------------------------------------------

using namespace std;
using namespace Loki;

extern void DoWeakLeakTest( void );
extern void DoStrongRefCountTests( void );
extern void DoStrongRefLinkTests( void );
extern void DoStrongReleaseTests( void );
extern void DoWeakCycleTests( void );
extern void DoStrongConstTests( void );
extern void DoStrongForwardReferenceTest( void );
extern void DoStrongCompareTests( void );
extern void DoStrongPtrDynamicCastTests( void );
extern void DoSingleOwnerTests( void );
extern void DoStrongArrayTests( void );

extern void DoLockedPtrTest( void );
extern void DoLockedStorageTest( void );

extern void TryColvinGibbonsTrick( void );

unsigned int BaseClass::s_constructions = 0;
unsigned int BaseClass::s_destructions = 0;

unsigned int MimicCOM::s_constructions = 0;
unsigned int MimicCOM::s_destructions = 0;


// ----------------------------------------------------------------------------

/// Used to check if SmartPtr can be used with a forward-reference.
/// GCC gives out warnings because of it, you can ignore them.
class Thingy;

typedef Loki::SmartPtr< Thingy, RefCounted, DisallowConversion,
    NoCheck, DefaultSPStorage, PropagateConst >
    Thingy_DefaultStorage_ptr;

//typedef Loki::SmartPtr< Thingy, RefCountedMTAdj< >, DisallowConversion,
//    AssertCheck, DefaultSPStorage, PropagateConst >
//    Thingy_Locked_ptr;

typedef Loki::SmartPtr< Thingy, RefCounted, DisallowConversion,
    AssertCheck, HeapStorage, PropagateConst >
    Thingy_HeapStorage_ptr;


// ----------------------------------------------------------------------------

/// @note Used for testing most policies.
typedef Loki::SmartPtr< BaseClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr;

/// @note These 4 are used for testing const policies.

typedef Loki::SmartPtr< BaseClass, RefCounted, DisallowConversion,
    NoCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_RefCount_NoConvert_NoCheck_DontPropagate_ptr;

typedef Loki::SmartPtr< const BaseClass, RefCounted, DisallowConversion,
    NoCheck, DefaultSPStorage, DontPropagateConst >
    ConstBase_RefCount_NoConvert_NoCheck_DontPropagate_ptr;

typedef Loki::SmartPtr< const BaseClass, RefCounted, DisallowConversion,
    NoCheck, DefaultSPStorage, PropagateConst >
    ConstBase_RefCount_NoConvert_NoCheck_Propagate_ptr;

typedef Loki::SmartPtr< BaseClass, RefCounted, DisallowConversion,
    NoCheck, DefaultSPStorage, PropagateConst >
    NonConstBase_RefCount_NoConvert_NoCheck_Propagate_ptr;


// ----------------------------------------------------------------------------

/// @note These 5 are used for testing ownership policies.
typedef Loki::SmartPtr< BaseClass, COMRefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< BaseClass, RefLinked, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< BaseClass, DeepCopy, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< BaseClass, DestructiveCopy, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< const BaseClass, DestructiveCopy, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    ConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< BaseClass, NoCopy, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr;


// ----------------------------------------------------------------------------

/// @note These 2 are used for testing inheritance.
typedef Loki::SmartPtr< PublicSubClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    PublicSub_RefCount_NoConvert_Assert_DontPropagate_ptr;

typedef Loki::SmartPtr< PrivateSubClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    PrivateSub_RefCount_NoConvert_Assert_DontPropagate_ptr;


// ----------------------------------------------------------------------------

/// @note Used for testing how well SmartPtr works with COM objects.
typedef Loki::SmartPtr< MimicCOM, COMRefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst >
    MimicCOM_ptr;


// ----------------------------------------------------------------------------

void DoConstConversionTests( void )
{
    cout << "Starting DoConstConversionTests." << endl;

    NonConstBase_RefCount_NoConvert_NoCheck_DontPropagate_ptr p1;
    ConstBase_RefCount_NoConvert_NoCheck_DontPropagate_ptr p2( p1 );
    ConstBase_RefCount_NoConvert_NoCheck_Propagate_ptr p3( p1 );
    NonConstBase_RefCount_NoConvert_NoCheck_Propagate_ptr p4( p1 );

//  p1 = p2;  // illegal! converts const to non-const.
//  p1 = p3;  // illegal! converts const to non-const.
    p1 = p4;  // legal, but dubious. Changes const-propagation policy.
    p2 = p1;  // legal.  natural const conversion.
    p2 = p3;  // legal, but dubious. Changes const-propagation policy.
    p2 = p4;  // legal, but dubious. Changes const-propagation policy.
    p3 = p1;  // legal, but dubious. Changes const-propagation policy.
    p3 = p2;  // legal, but dubious. Changes const-propagation policy.
    p3 = p4;  // legal.  natural const conversion.
    p4 = p1;  // legal, but dubious. Changes const-propagation policy.
//  p4 = p2;  // illegal! converts const to non-const.
//  p4 = p3;  // illegal! converts const to non-const.

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoConstConversionTests." << endl;
}

// ----------------------------------------------------------------------------

void DoOwnershipConversionTests( void )
{
    cout << "Starting DoOwnershipConversionTests." << endl;

    {
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p1( new BaseClass );
        NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p2( new BaseClass );
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p3( new BaseClass );
        NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p4( new BaseClass );
        NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p5( new BaseClass );
        NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p6( new BaseClass );

        // legal constructions.  Each should allow copy with same policies.
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p7( p1 );
        NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p8( p2 );
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p9( p3 );
        NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p10( p4 );
        NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p11( p5 );

        // illegal construction!  Can't copy anything with NoCopy policy.
        // NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p12( p6 );

        // illegal constructions!  Can't convert from one ownership policy to another.
        // NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p13( p2 );
        // NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p14( p3 );
        // NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p15( p4 );
        // NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p16( p5 );
        // NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p17( p6 );

        // illegal constructions!  Can't convert from one ownership policy to another.
        // NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p18( p1 );
        // NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p19( p3 );
        // NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p20( p4 );
        // NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p21( p5 );
        // NonConstBase_ComRef_NoConvert_Assert_DontPropagate_ptr p22( p6 );

        // illegal constructions!  Can't convert from one ownership policy to another.
        // NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p23( p1 );
        // NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p24( p2 );
        // NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p25( p4 );
        // NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p26( p5 );
        // NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p27( p6 );

        // illegal constructions!  Can't convert from one ownership policy to another.
        // NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p28( p1 );
        // NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p29( p2 );
        // NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p30( p3 );
        // NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p31( p5 );
        // NonConstBase_DeepCopy_NoConvert_Assert_DontPropagate_ptr p32( p6 );

        // illegal constructions!  Can't convert from one ownership policy to another.
        // NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p33( p1 );
        // NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p34( p2 );
        // NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p35( p3 );
        // NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p36( p4 );
        // NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p37( p6 );

        // illegal constructions!  Can't convert from one ownership policy to another.
        // NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p38( p1 );
        // NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p39( p2 );
        // NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p40( p3 );
        // NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p41( p4 );
        // NonConstBase_NoCopy_NoConvert_Assert_DontPropagate_ptr p42( p5 );
    }

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );

    {
        // These constructions are legal because the preserve const-ness.
        const BaseClass * rawP = new BaseClass;
        ConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p43( rawP );
        ConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p44( p43 );
        rawP = new BaseClass;
        // These constructions are illegal because the don't preserve constness.
        // NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p45( rawP );
        // NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p46( p43 );
        ConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p47( rawP );

        NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p48;
        NonConstBase_KillCopy_NoConvert_Assert_DontPropagate_ptr p49( new BaseClass );
        // This assignment is legal because it preserves constness.
        p48 = p49;
        // This assignment is illegal because it won't preserve constness.
        // p48 = p43;

        // illegal assignments!  Can't convert from one ownership policy to another.
        // p1 = p2;
        // p1 = p3;
        // p1 = p4;
        // p1 = p5;
        // p1 = p6;
        // p2 = p1;
        // p2 = p3;
        // p2 = p4;
        // p2 = p5;
        // p2 = p6;
        // p3 = p1;
        // p3 = p2;
        // p3 = p4;
        // p3 = p5;
        // p3 = p6;
        // p4 = p1;
        // p4 = p2;
        // p4 = p3;
        // p4 = p5;
        // p4 = p6;
        // p5 = p1;
        // p5 = p2;
        // p5 = p3;
        // p5 = p4;
        // p5 = p6;
        // p6 = p1;
        // p6 = p2;
        // p6 = p3;
        // p6 = p4;
        // p6 = p5;
    }

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoOwnershipConversionTests." << endl;
}

// ----------------------------------------------------------------------------

void DoInheritanceConversionTests( void )
{
    cout << "Starting DoInheritanceConversionTests." << endl;

    {
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p1;
        PublicSub_RefCount_NoConvert_Assert_DontPropagate_ptr p2( new PublicSubClass );
        PrivateSub_RefCount_NoConvert_Assert_DontPropagate_ptr p3;

        p1 = p2;  // legal.  Cast to public base class allowed.
        assert( p1 == p2 );
//      p1 = p3;  // illegal!  Can't assign pointer since base class is private.
//      p2 = p1;  // illegal!  Can't do cast to derived class in pointer assignment.
//      p2 = p3;  // illegal!  Can't assign when types are unrelated.
//      p3 = p1;  // illegal!  Can't do cast to derived class in pointer assignment.
//      p3 = p2;  // illegal!  Can't assign when types are unrelated.

        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p4( p2 );
        assert( p4 == p1 );
        assert( p4 == p2 );
        // These copy-constructions are illegal for reasons shown above.
//      NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p5( p3 );
//      PublicSub_RefCount_NoConvert_Assert_DontPropagate_ptr p6( p1 );
//      PublicSub_RefCount_NoConvert_Assert_DontPropagate_ptr p7( p3 );
//      PrivateSub_RefCount_NoConvert_Assert_DontPropagate_ptr p8( p1 );
//      PrivateSub_RefCount_NoConvert_Assert_DontPropagate_ptr p9( p2 );
    }

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoInheritanceConversionTests." << endl;
}

// ----------------------------------------------------------------------------

void DoRefCountSwapTests( void )
{
    cout << "Starting DoRefCountSwapTests." << endl;

    {
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p1( new BaseClass );
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p2( new BaseClass );

        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p3( p1 );
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p4( p2 );

        // p1 == p3    and    p2 == p4
        assert( p1 == p3 );
        assert( p1 != p2 );
        assert( p1 != p4 );
        assert( p2 == p4 );
        assert( p2 != p3 );
        assert( p2 != p1 );

        // p1 == p4    and    p2 == p3
        p3.Swap( p4 );
        assert( p1 == p4 );
        assert( p1 != p2 );
        assert( p1 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p1 );

        // p1 == p3    and    p2 == p4
        p3.Swap( p4 );
        assert( p1 == p3 );
        assert( p1 != p2 );
        assert( p1 != p4 );
        assert( p2 == p4 );
        assert( p2 != p3 );
        assert( p2 != p1 );

        // p2 == p3    and    p1 == p4
        p1.Swap( p2 );
        assert( p1 == p4 );
        assert( p1 != p2 );
        assert( p1 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p1 );

        // p2 == p3    and    p1 == p4
        p1.Swap( p1 );
        assert( p1 == p4 );
        assert( p1 != p2 );
        assert( p1 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p1 );

        // p2 == p3    and    p4 == p1
        p1.Swap( p4 );
        assert( p1 == p4 );
        assert( p1 != p2 );
        assert( p1 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p1 );
    }

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoRefCountSwapTests." << endl;
}

// ----------------------------------------------------------------------------

void DoRefLinkSwapTests( void )
{
    cout << "Starting DoRefLinkSwapTests." << endl;

    {
        BaseClass * pBaseClass = new BaseClass;
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p1( pBaseClass );
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p2( new BaseClass );
        p1->DoThat();
        p2->DoThat();

        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p3( p1 );
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p4( p2 );

        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p5;
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p6( new BaseClass );

        BaseClass * pBare1 = GetImpl( p1 );
        BaseClass * pBare2 = GetImpl( p2 );
        BaseClass * pBare6 = GetImpl( p6 );
        const bool is1LessThan2 = ( pBare1 < pBare2 );
        const bool is1LessThan6 = ( pBare1 < pBare6 );
        const bool is2LessThan6 = ( pBare2 < pBare6 );
        assert( pBare1 != pBare2 );
        assert( pBare1 != pBare6 );
        assert( pBare2 != pBare6 );
        assert( p1 == pBare1 );
        assert( p2 == pBare2 );
        assert( p6 == pBare6 );
        assert( pBare1 == p1 );
        assert( pBare2 == p2 );
        assert( pBare6 == p6 );
        assert( pBare2 != p1 );
        assert( p1 != pBare2 );

        if ( is1LessThan2 )
        {
            assert( p1 < p2 );
            assert( p1 <= p2 );
            assert( p1 != p2 );
            assert( p2 > p1 );
            assert( p2 >= p1 );
            assert( p1 < pBare2 );
            assert( p1 <= pBare2 );
            assert( pBare2 > p1 );
            assert( pBare2 >= p1 );
        }
        else
        {
            assert( p2 < p1 );
            assert( p2 <= p1 );
            assert( p2 != p1 );
            assert( p1 > p2 );
            assert( p1 >= p2 );
            assert( p2 < pBare1 );
            assert( p2 <= pBare1 );
            assert( p2 != pBare1 );
            assert( pBare1 > p2 );
            assert( pBare1 >= p2 );
        }

        if ( is1LessThan6 )
        {
            assert( p1 < p6 );
            assert( p1 <= p6 );
            assert( p1 != p6 );
            assert( p6 > p1 );
            assert( p6 >= p1 );
            assert( p1 < pBare6 );
            assert( p1 <= pBare6 );
            assert( pBare6 > p1 );
            assert( pBare6 >= p1 );
        }
        else
        {
            assert( p6 < p1 );
            assert( p6 <= p1 );
            assert( p6 != p1 );
            assert( p1 > p6 );
            assert( p1 >= p6 );
            assert( p6 < pBare1 );
            assert( p6 <= pBare1 );
            assert( p6 != pBare1 );
            assert( pBare1 > p6 );
            assert( pBare1 >= p6 );
        }

        if ( is2LessThan6 )
        {
            assert( p2 < p6 );
            assert( p2 <= p6 );
            assert( p2 != p6 );
            assert( p6 > p2 );
            assert( p6 >= p2 );
            assert( p2 < pBare6 );
            assert( p2 <= pBare6 );
            assert( pBare6 > p2 );
            assert( pBare6 >= p2 );
        }
        else
        {
            assert( p6 < p2 );
            assert( p6 <= p2 );
            assert( p6 != p2 );
            assert( p2 > p6 );
            assert( p2 >= p6 );
            assert( p6 < pBare2 );
            assert( p6 <= pBare2 );
            assert( p6 != pBare2 );
            assert( pBare2 > p6 );
            assert( pBare2 >= p6 );
        }

        // p1 <---> p3    and    p2 <---> p4   and   p5   and   p6
        assert( p1 == p3 );
        assert( p1 != p2 );
        assert( p1 != p4 );
        assert( p2 == p4 );
        assert( p2 != p3 );
        assert( p2 != p1 );
        assert( p1 != p5 );
        assert( p2 != p5 );
        assert( p3 != p5 );
        assert( p4 != p5 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );

        p3.Swap( p4 );  // p1 <---> p4    and    p2 <---> p3   and   p5   and   p6
        assert( p1 == p4 );
        assert( p1 != p2 );
        assert( p1 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p1 );
        assert( p1 != p5 );
        assert( p2 != p5 );
        assert( p3 != p5 );
        assert( p4 != p5 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );

        p3.Swap( p4 );   // p1 <---> p3    and    p2 <---> p4   and   p5   and   p6
        assert( p1 == p3 );
        assert( p1 != p2 );
        assert( p1 != p4 );
        assert( p2 == p4 );
        assert( p2 != p3 );
        assert( p2 != p1 );
        assert( p1 != p5 );
        assert( p2 != p5 );
        assert( p3 != p5 );
        assert( p4 != p5 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );

        p1.Swap( p2 );  // p2 <---> p3    and    p1 <---> p4   and   p5   and   p6
        assert( p1 != pBaseClass );
        assert( p2 == pBaseClass );
        assert( p1 == p4 );
        assert( p1 != p2 );
        assert( p1 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p1 );
        assert( p1 != p5 );
        assert( p2 != p5 );
        assert( p3 != p5 );
        assert( p4 != p5 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );

        p1.Swap( p1 );  // p2 <---> p3    and    p1 <---> p4   and   p5   and   p6
        assert( p1 == p4 );
        assert( p1 != p2 );
        assert( p1 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p1 );
        assert( p1 != p5 );
        assert( p2 != p5 );
        assert( p3 != p5 );
        assert( p4 != p5 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );

        p1.Swap( p4 );  // p2 <---> p3    and    p4 <---> p1   and   p5   and   p6
        assert( p1 == p4 );
        assert( p1 != p2 );
        assert( p1 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p1 );
        assert( p1 != p5 );
        assert( p2 != p5 );
        assert( p3 != p5 );
        assert( p4 != p5 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );

        p4.Swap( p1 );  // p2 <---> p3    and    p4 <---> p1   and   p5   and   p6
        assert( p1 == p4 );
        assert( p1 != p2 );
        assert( p1 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p1 );
        assert( p1 != p5 );
        assert( p2 != p5 );
        assert( p3 != p5 );
        assert( p4 != p5 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );

        p5.Swap( p5 );  // p2 <---> p3    and    p4 <---> p1   and   p5   and   p6
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p1 != p5 );
        assert( p2 != p5 );
        assert( p3 != p5 );
        assert( p4 != p5 );

        p5.Swap( p1 );  // p2 <---> p3    and    p4 <---> p5   and   p1   and   p6
        assert( p5 == p4 );
        assert( p5 != p2 );
        assert( p5 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p5 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p4 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );

        p6.Swap( p1 );  // p2 <---> p3    and    p4 <---> p5   and   p1   and   p6
        assert( p5 == p4 );
        assert( p5 != p2 );
        assert( p5 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p5 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p4 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );

        p5.Swap( p1 );  // p2 <---> p3    and    p4 <---> p1   and   p5   and   p6
        assert( p1 == p4 );
        assert( p1 != p2 );
        assert( p1 != p3 );
        assert( p2 == p3 );
        assert( p2 != p4 );
        assert( p2 != p1 );
        assert( p1 != p5 );
        assert( p2 != p5 );
        assert( p3 != p5 );
        assert( p4 != p5 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );

        p6 = p2;  // p6 <---> p2 <---> p3    and    p4 <---> p1   and   p5
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p5 != p3 );
        assert( p2 != p4 );
        assert( p2 != p5 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );

        p5 = p3;  // p6 <---> p2 <---> p3 <---> p5   and    p4 <---> p1
        assert( p6 == p5 );
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p5 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p2 != p4 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );

        p5.Swap( p3 );  // p6 <---> p2 <---> p5 <---> p3   and    p4 <---> p1
        assert( p6 == p5 );
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p5 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p2 != p4 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );

        p2.Swap( p3 );  // p6 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
        assert( p6 == p5 );
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p5 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p2 != p4 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );

        bool merged = false;
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p7( pBaseClass );
        assert( p7 == p7 );
        assert( p6 == p7 );
        assert( p1 != p7 );
        merged = p7.Merge( p6 );
        // p7 <---> p6 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
        assert( merged );
        assert( p6 == p7 );
        assert( p1 != p7 );
        assert( p6 == p5 );
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p5 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p2 != p4 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );
        assert( p7 == p7 );

        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p8( pBaseClass );
        assert( p6 == p8 );
        assert( p1 != p8 );
        merged = p6.Merge( p8 );
        // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
        assert( merged );
        assert( p6 == p8 );
        assert( p6 == p7 );
        assert( p1 != p7 );
        assert( p6 == p5 );
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p5 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p2 != p4 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );
        assert( p7 == p7 );
        assert( p8 == p8 );

        merged = p6.Merge( p6 );
        // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
        assert( merged );
        assert( p6 == p8 );
        assert( p6 == p7 );
        assert( p1 != p7 );
        assert( p6 == p5 );
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p5 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p2 != p4 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );
        assert( p7 == p7 );
        assert( p8 == p8 );

        merged = p6.Merge( p3 );
        // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
        assert( merged );
        assert( p6 == p8 );
        assert( p6 == p7 );
        assert( p1 != p7 );
        assert( p6 == p5 );
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p5 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p2 != p4 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );
        assert( p7 == p7 );
        assert( p8 == p8 );

        merged = p5.Merge( p1 );
        // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2   and    p4 <---> p1
        assert( !merged );
        assert( p6 == p8 );
        assert( p6 == p7 );
        assert( p1 != p7 );
        assert( p6 == p5 );
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p5 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p2 != p4 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );
        assert( p7 == p7 );
        assert( p8 == p8 );

        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p9( pBaseClass );
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr pA( p9 );
        assert( p9 == pA );
        assert( p9 == p8 );
        assert( p1 != p8 );
        merged = p9.Merge( p1 );
        // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2
        //   and    p4 <---> p1   and   p9 <---> pA
        assert( !merged );
        assert( p6 == p8 );
        assert( p6 == p7 );
        assert( p1 != p7 );
        assert( p6 == p5 );
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p5 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p2 != p4 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );
        assert( p7 == p7 );
        assert( p8 == p8 );
        assert( p9 == p9 );
        assert( pA == pA );

        merged = p9.Merge( p2 );
        // p7 <---> p6 <---> p8 <---> p3 <---> p5 <---> p2 <---> p9 <---> pA
        //   and    p4 <---> p1
        assert( merged );
        assert( p6 == p8 );
        assert( p6 == p7 );
        assert( p1 != p7 );
        assert( p6 == p5 );
        assert( p6 == p2 );
        assert( p6 == p3 );
        assert( p5 == p3 );
        assert( p2 == p3 );
        assert( p1 == p4 );
        assert( p2 != p4 );
        assert( p1 != p5 );
        assert( p2 != p1 );
        assert( p3 != p1 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p3 == p3 );
        assert( p4 == p4 );
        assert( p5 == p5 );
        assert( p6 == p6 );
        assert( p7 == p7 );
        assert( p8 == p8 );
        assert( p9 == p9 );
        assert( pA == pA );
    }

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoRefLinkSwapTests." << endl;
}

// ----------------------------------------------------------------------------

void DoRefLinkTests( void )
{
    cout << "Starting DoRefLinkTests." << endl;

    const unsigned int ctorCount = BaseClass::GetCtorCount(); (void) ctorCount;
    const unsigned int dtorCount = BaseClass::GetDtorCount(); (void) dtorCount;
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr w0;
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr w1;
    }
    assert( ctorCount == BaseClass::GetCtorCount() );
    assert( dtorCount == BaseClass::GetDtorCount() );

    {
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr w3( new BaseClass );
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr w4( new BaseClass );
        assert( w3 != w4 );
        assert( w3 );
        assert( w4 );
        w3 = w4;
        assert( w3 == w4 );
        assert( w3 );
        assert( w4 );
        assert( dtorCount + 1 == BaseClass::GetDtorCount() );
        w3->DoThat();
    }
    assert( ctorCount + 2 == BaseClass::GetCtorCount() );
    assert( dtorCount + 2 == BaseClass::GetDtorCount() );
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoRefLinkTests." << endl;
}

// ----------------------------------------------------------------------------

void DoRefCountNullPointerTests( void )
{
    cout << "Starting DoRefCountNullPointerTests." << endl;

    BaseClass * pNull = NULL; (void) pNull;
    const unsigned int ctorCount = BaseClass::GetCtorCount(); (void) ctorCount;
    const unsigned int dtorCount = BaseClass::GetDtorCount(); (void) dtorCount;
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p0;
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p1;
        NonConstBase_RefCount_NoConvert_Assert_DontPropagate_ptr p2;

        assert( !p0 );
        assert( !p1 );
        assert( !p2 );
        assert( p1 == pNull );
        assert( p0 == pNull );
        assert( pNull == p0 );
        assert( pNull == p1 );
        assert( pNull == p2 );
        assert( p0 == p0 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p1 == p0 );
        assert( p0 == p1 );
        assert( p2 == p0 );
        assert( p0 == p2 );
        assert( p1 <= p0 );
        assert( p1 >= p0 );
        assert( p0 <= p1 );
        assert( p0 >= p1 );
        assert( p2 <= p0 );
        assert( p2 >= p0 );
        assert( p0 <= p2 );
        assert( p0 >= p2 );
        assert( !( p1 < p0 ) );
        assert( !( p1 > p0 ) );
        assert( !( p0 < p1 ) );
        assert( !( p0 > p1 ) );
        assert( !( p2 < p0 ) );
        assert( !( p2 > p0 ) );
        assert( !( p0 < p2 ) );
        assert( !( p0 > p2 ) );
        assert( !( p0 < pNull ) );
        assert( !( p0 > pNull ) );
        assert( !( pNull < p0 ) );
        assert( !( pNull > p0 ) );
    }
    assert( ctorCount == BaseClass::GetCtorCount() );
    assert( dtorCount == BaseClass::GetDtorCount() );

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoRefCountNullPointerTests." << endl;
}

// ----------------------------------------------------------------------------

void DoRefLinkNullPointerTests( void )
{
    cout << "Starting DoRefLinkNullPointerTests." << endl;

    BaseClass * pNull = NULL; (void) pNull;
    const unsigned int ctorCount = BaseClass::GetCtorCount(); (void) ctorCount;
    const unsigned int dtorCount = BaseClass::GetDtorCount(); (void) dtorCount;
    assert( BaseClass::GetCtorCount() == BaseClass::GetDtorCount() );

    {
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p0;
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p1;
        NonConstBase_RefLink_NoConvert_Assert_DontPropagate_ptr p2;

        assert( !p0 );
        assert( !p1 );
        assert( !p2 );
        assert( p1 == pNull );
        assert( p0 == pNull );
        assert( pNull == p0 );
        assert( pNull == p1 );
        assert( pNull == p2 );
        assert( p0 == p0 );
        assert( p1 == p1 );
        assert( p2 == p2 );
        assert( p1 == p0 );
        assert( p0 == p1 );
        assert( p2 == p0 );
        assert( p0 == p2 );
        assert( p1 <= p0 );
        assert( p1 >= p0 );
        assert( p0 <= p1 );
        assert( p0 >= p1 );
        assert( p2 <= p0 );
        assert( p2 >= p0 );
        assert( p0 <= p2 );
        assert( p0 >= p2 );
        assert( !( p1 < p0 ) );
        assert( !( p1 > p0 ) );
        assert( !( p0 < p1 ) );
        assert( !( p0 > p1 ) );
        assert( !( p2 < p0 ) );
        assert( !( p2 > p0 ) );
        assert( !( p0 < p2 ) );
        assert( !( p0 > p2 ) );
        assert( !( p0 < pNull ) );
        assert( !( p0 > pNull ) );
        assert( !( pNull < p0 ) );
        assert( !( pNull > p0 ) );
    }
    assert( ctorCount == BaseClass::GetCtorCount() );
    assert( dtorCount == BaseClass::GetDtorCount() );

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoRefLinkNullPointerTests." << endl;
}

// ----------------------------------------------------------------------------

void DoComRefTest( void )
{
    cout << "Starting DoComRefTest." << endl;

    const unsigned int ctorCount = MimicCOM::GetCtorCount(); (void) ctorCount;
    const unsigned int dtorCount = MimicCOM::GetDtorCount(); (void) dtorCount;
    assert( MimicCOM::AllDestroyed() );
    {
        MimicCOM_ptr p1;
    }
    assert( MimicCOM::AllDestroyed() );
    assert( ctorCount == MimicCOM::GetCtorCount() );
    assert( dtorCount == MimicCOM::GetDtorCount() );

    {
        MimicCOM_ptr p1( MimicCOM::QueryInterface() );
    }
    assert( ctorCount+1 == MimicCOM::GetCtorCount() );
    assert( dtorCount+1 == MimicCOM::GetDtorCount() );

    {
        MimicCOM_ptr p2( MimicCOM::QueryInterface() );
        MimicCOM_ptr p3( p2 );
        MimicCOM_ptr p4;
        p4 = p2;
    }
    assert( ctorCount+2 == MimicCOM::GetCtorCount() );
    assert( dtorCount+2 == MimicCOM::GetDtorCount() );

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoComRefTest." << endl;
}

// ----------------------------------------------------------------------------

void DoForwardReferenceTest( void )
{
    cout << "Starting DoForwardReferenceTest." << endl;

    /** @note These lines should cause the compiler to make a warning message
     about attempting to delete an undefined type.  But it should not produce
     any error messages.
     */
    Thingy_DefaultStorage_ptr p1;
    Thingy_DefaultStorage_ptr p2( p1 );
    Thingy_DefaultStorage_ptr p3;
    p3 = p2;

    /** @note These lines should cause the compiler to make an error message
     about attempting to call the destructor for an undefined type.
     */
    //Thingy_HeapStorage_ptr p4;
    //Thingy_HeapStorage_ptr p5( p4 );
    //Thingy_HeapStorage_ptr p6;
    //p6 = p5;

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoForwardReferenceTest." << endl;
}

// ----------------------------------------------------------------------------

void DoSmartPtrDynamicCastTests( void )
{
    cout << "Starting DoSmartPtrDynamicCastTests." << endl;

    typedef ::Loki::SmartPtr< Feline, RefCounted, DisallowConversion, NoCheck > FelinePtr;
    typedef ::Loki::SmartPtr< Tiger,  RefCounted, DisallowConversion, NoCheck > TigerPtr;
    typedef ::Loki::SmartPtr< Lion,   RefCounted, DisallowConversion, NoCheck > LionPtr;
    typedef ::Loki::SmartPtr< Dog,    RefCounted, DisallowConversion, NoCheck > DogPtr;

    {
        Feline * feline = new Lion;
        Lion * lion = new Lion;
        Tiger * tiger = new Tiger;
        Dog * dog = new Dog;

        FelinePtr pFeline( feline );
        LionPtr pLion( lion );
        TigerPtr pTiger( tiger );
        DogPtr pDog( dog );

        // This is legal because C++ allows an automatic down-cast to public base class.
        pFeline = pLion;

#ifdef CHECK_TYPE_CAST
        pLion = pFeline; // Fails as the compiler cannot convert pointers in SmartPtr
#endif // CHECK_TYPE_CAST

        assert( pFeline );
        // Can up-cast from feline to lion only if the feline is a lion.
        pLion.DynamicCastFrom( pFeline );
        assert( pLion );
        assert( pLion == pFeline );

        // Can't cast from lion to tiger since although these are both types of felines,
        // they are not related to one another.
        pTiger.DynamicCastFrom( pLion );
        assert( !pTiger );

        // Can't cast from dog to lion since a dog is not a type of feline.
        pLion.DynamicCastFrom( pDog );
        assert( !pLion );

        pLion.DynamicCastFrom( pFeline );
        assert( pLion );
        assert( pLion == pFeline );

        // Can't cast from lion to dog since these animal types are not related.
        pDog.DynamicCastFrom( pLion );
        assert( !pDog );

        feline = new Lion;
        lion = new Lion;
        tiger = new Tiger;
        dog = new Dog;

        // Now do tests when converting from const pointers.
        const FelinePtr pcFeline( feline );
        const LionPtr pcLion( lion );
        const TigerPtr pcTiger( tiger );
        const DogPtr pcDog( dog );

        assert( pcFeline );
        // Can up-cast from feline to lion only if the feline is a lion.
        pLion.DynamicCastFrom( pcFeline );
        assert( pLion );
        assert( pLion == pcFeline );

        // Can't cast from lion to tiger since although these are both types of felines,
        // they are not related to one another.
        pTiger.DynamicCastFrom( pcLion );
        assert( !pTiger );

        // Can't cast from dog to lion since a dog is not a type of feline.
        pLion.DynamicCastFrom( pcDog );
        assert( !pLion );

        pLion.DynamicCastFrom( pcFeline );
        assert( pLion );
        assert( pLion == pcFeline );

        // Can't cast from lion to dog since these animal types are not related.
        pDog.DynamicCastFrom( pcLion );
        assert( !pDog );
    }

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoSmartPtrDynamicCastTests." << endl;
}

// ----------------------------------------------------------------------------

/// @note Used for testing exception-throwing policy, and for checking for null-pointer dereference.
typedef ::Loki::SmartPtr< Feline, DeepCopy, DisallowConversion, RejectNullStatic, DefaultSPStorage, DontPropagateConst > Feline_DeepCopy_Ptr;
typedef ::Loki::SmartPtr< Tiger,  DeepCopy, DisallowConversion, RejectNullStatic, DefaultSPStorage, DontPropagateConst > Tiger_DeepCopy_Ptr;
typedef ::Loki::SmartPtr< Lion,   DeepCopy, DisallowConversion, RejectNullStatic, DefaultSPStorage, DontPropagateConst > Lion_DeepCopy_Ptr;
typedef ::Loki::SmartPtr< Dog,    DeepCopy, DisallowConversion, RejectNullStatic, DefaultSPStorage, DontPropagateConst > Dog_DeepCopy_Ptr;


// ----------------------------------------------------------------------------

void DoDeepCopyTests( void )
{
    cout << "Starting DoDeepCopyTests." << endl;

    try
    {
        const Dog_DeepCopy_Ptr p1( NULL );
        Dog_DeepCopy_Ptr p2( p1 );
        assert( false );
    }
    catch ( ... )
    {
        assert( true );
    }

    try
    {
        Dog_DeepCopy_Ptr p1( NULL );
        Dog_DeepCopy_Ptr p2( p1 );
        assert( false );
    }
    catch ( ... )
    {
        assert( true );
    }

    try
    {
        const Dog_DeepCopy_Ptr p1( NULL );
        Dog_DeepCopy_Ptr p2( NULL );
        p2 = p1;
        assert( false );
    }
    catch ( ... )
    {
        assert( true );
    }

    try
    {
        Dog_DeepCopy_Ptr p1( NULL );
        Dog_DeepCopy_Ptr p2( NULL );
        p2 = p1;
        assert( false );
    }
    catch ( ... )
    {
        assert( true );
    }

    try
    {
        Tiger_DeepCopy_Ptr p1( new Tiger );
        Lion_DeepCopy_Ptr p2( NULL );
        p2.DynamicCastFrom( p1 );
        assert( false );
    }
    catch ( ... )
    {
        assert( true );
    }

    try
    {
        const Tiger_DeepCopy_Ptr p1( new Tiger );
        Lion_DeepCopy_Ptr p2( NULL );
        p2.DynamicCastFrom( p1 );
        assert( false );
    }
    catch ( ... )
    {
        assert( true );
    }

    assert( Feline::AllDestroyed() );
    assert( !Feline::ExtraConstructions() );
    assert( !Feline::ExtraDestructions() );
    cout << "Finished DoDeepCopyTests." << endl;
}

// ----------------------------------------------------------------------------

typedef Loki::SmartPtr< BaseClass, DestructiveCopy > DestructiveCopyPtr;

DestructiveCopyPtr MakePointer( void )
{
    DestructiveCopyPtr p( new BaseClass );
    return p;
}

// ----------------------------------------------------------------------------

void DoDestructiveCopyTest( void )
{
    cout << "Starting DoDestructiveCopyTest." << endl;

    {
        DestructiveCopyPtr p1( new BaseClass );
        assert( p1 );
        DestructiveCopyPtr p2;
        assert( !p2 );
        p2 = p1;
        assert( !p1 );
        assert( p2 );
        DestructiveCopyPtr p3( p2 );
        assert( p3 );
        assert( !p2 );
    }

    {
        const DestructiveCopyPtr p1( new BaseClass );
        assert( p1 );
        /** @note The following code won't compile because p1 is declared const. You can test
         if Loki's SmartPtr DestructiveCopy policy was designed correctly by uncommenting these
         lines and seeing if any errors occur when compiling. If you see errors about converting
         from const to non-const, or about assigning a read-only reference, then DestructiveCopy
         was designed correctly.
         */
        //DestructiveCopyPtr p2;
        //assert( !p2 );
        //p2 = p1;
        //assert( !p1 );
        //assert( p2 );
        //DestructiveCopyPtr p3( p2 );
        //assert( p3 );
        //assert( !p2 );
    }

    {
        /// @todo The following lines need to be uncommented when bug 3224572 gets fixed.
        DestructiveCopyPtr p1( MakePointer() );
        assert( p1 );
        DestructiveCopyPtr p2;
        p2 = MakePointer();
        assert( p2 );
    }

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoDestructiveCopyTest." << endl;
}

// ----------------------------------------------------------------------------

/// Use these typedefs to test DeleteArray policy.

typedef Loki::SmartPtr< Tiger, RefCounted, DisallowConversion,
    AssertCheck, ArrayStorage, DontPropagateConst >
    TigerArray_RefCounted_ptr;

typedef Loki::SmartPtr< Tiger, RefLinked, DisallowConversion,
    AssertCheck, ArrayStorage, DontPropagateConst >
    TigerArray_2RefLinks_ptr;

// ----------------------------------------------------------------------------

void DoSmartArrayTests( void )
{
    cout << "Starting DoSmartArrayTests." << endl;

    {
        // test default construction.
        TigerArray_RefCounted_ptr sp1;
        assert( !sp1 );
        assert( 0 == sp1.GetArrayCount() );

        // test assignment.
        sp1.Assign( new Tiger[ 8 ], 8 );
        assert( sp1 );
        assert( 8 == sp1.GetArrayCount() );
        sp1[ 0 ].SetStripes( 8 );
        sp1[ 1 ].SetStripes( 16 );
        sp1[ 2 ].SetStripes( 24 );
        sp1[ 3 ].SetStripes( 32 );
        sp1[ 4 ].SetStripes( 40);
        sp1[ 5 ].SetStripes( 48 );
        sp1[ 6 ].SetStripes( 56 );
        sp1[ 7 ].SetStripes( 64 );

        // test initialization construction.
        TigerArray_RefCounted_ptr sp2( new Tiger[ 4 ], 4 );
        assert( sp2 );
        assert( 4 == sp2.GetArrayCount() );
        sp2[ 0 ].SetStripes( 5 );
        sp2[ 1 ].SetStripes( 10 );
        sp2[ 2 ].SetStripes( 15 );
        sp2[ 3 ].SetStripes( 20 );

        // test range checking.
        try
        {
            Tiger & p4 = sp2[ 4 ];
            (void)p4;
            assert( false );
        }
        catch ( const ::std::out_of_range & ex )
        {
            (void)ex;
            assert( true );
        }

        // test range checking.
        try
        {
            Tiger & p8 = sp1[ 8 ];
            (void)p8;
            assert( false );
        }
        catch ( const ::std::out_of_range & ex )
        {
            (void)ex;
            assert( true );
        }

        // test swap.
        sp2.Swap( sp1 );
        assert( sp1 );
        assert( sp2 );
        // test checking of item count.
        assert( 4 == sp1.GetArrayCount() );
        assert( 8 == sp2.GetArrayCount() );

        // test that operator[] returns reference to
        assert(  5 == sp1[ 0 ].GetStripes() );
        assert( 10 == sp1[ 1 ].GetStripes() );
        assert( 15 == sp1[ 2 ].GetStripes() );
        assert( 20 == sp1[ 3 ].GetStripes() );
        assert(  8 == sp2[ 0 ].GetStripes() );
        assert( 16 == sp2[ 1 ].GetStripes() );
        assert( 24 == sp2[ 2 ].GetStripes() );
        assert( 32 == sp2[ 3 ].GetStripes() );
        assert( 40 == sp2[ 4 ].GetStripes() );
        assert( 48 == sp2[ 5 ].GetStripes() );
        assert( 56 == sp2[ 6 ].GetStripes() );
        assert( 64 == sp2[ 7 ].GetStripes() );

        try
        {
            Tiger & p4 = sp1[ 4 ];
            (void)p4;
            assert( false );
        }
        catch ( const ::std::out_of_range & ex )
        {
            (void)ex;
            assert( true );
        }

        try
        {
            Tiger & p8 = sp2[ 8 ];
            (void)p8;
            assert( false );
        }
        catch ( const ::std::out_of_range & ex )
        {
            (void)ex;
            assert( true );
        }

        const TigerArray_RefCounted_ptr sp3( sp1 );
        assert( sp3 == sp1 );
        assert( sp3.GetArrayCount() == sp1.GetArrayCount() );
        try
        {
            const Tiger & p4 = sp3[ 4 ];
            (void)p4;
            assert( false );
        }
        catch ( const ::std::out_of_range & ex )
        {
            (void)ex;
            assert( true );
        }

        const TigerArray_RefCounted_ptr sp5( sp2 );
        assert( sp5 == sp2 );
        assert( sp5.GetArrayCount() == sp2.GetArrayCount() );
        try
        {
            const Tiger & p8 = sp5[ 8 ];
            (void)p8;
            assert( false );
        }
        catch ( const ::std::out_of_range & ex )
        {
            (void)ex;
            assert( true );
        }

        sp2 = sp1;
        assert( sp1 == sp2 );
        assert( sp3 == sp2 );
        assert( sp2.GetArrayCount() == sp1.GetArrayCount() );
        assert( sp2.GetArrayCount() == sp1.GetArrayCount() );
        assert( sp1 != sp5 );
        assert( sp2 != sp5 );
        assert( sp3 != sp5 );
        assert( sp1.GetArrayCount() != sp5.GetArrayCount() );
        assert( sp2.GetArrayCount() != sp5.GetArrayCount() );
        assert( sp3.GetArrayCount() != sp5.GetArrayCount() );
    }

    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );
    assert( !BaseClass::ExtraDestructions() );
    cout << "Finished DoSmartArrayTests." << endl;
}

// ----------------------------------------------------------------------------

int main( int argc, const char * argv[] )
{
    bool doThreadTest = false;
    for ( int ii = 1; ii < argc; ++ii )
    {
        if ( ::strcmp( argv[ii], "-t" ) == 0 )
            doThreadTest = true;
    }

    DoDeepCopyTests();
    DoDestructiveCopyTest();
    DoRefLinkTests();
    DoWeakLeakTest();
    DoStrongRefCountTests();
    DoStrongReleaseTests();
    DoStrongReleaseTests();
    DoWeakCycleTests();
    DoStrongCompareTests();
    DoSingleOwnerTests();

    DoForwardReferenceTest();
    DoStrongForwardReferenceTest();

    DoRefCountNullPointerTests();
    DoRefLinkNullPointerTests();

    DoRefCountSwapTests();
    DoRefLinkSwapTests();

    DoComRefTest();
//    TryColvinGibbonsTrick();

    DoStrongConstTests();
    DoConstConversionTests();
    DoOwnershipConversionTests();
    DoInheritanceConversionTests();
    DoSmartPtrDynamicCastTests();
    DoStrongPtrDynamicCastTests();
    DoStrongArrayTests();
    DoSmartArrayTests();

#if defined (LOKI_OBJECT_LEVEL_THREADING) || defined (LOKI_CLASS_LEVEL_THREADING)
    if ( doThreadTest )
    {
        DoLockedStorageTest();
        DoLockedPtrTest();
    }
#endif

    // Check that nothing was leaked.
    assert( BaseClass::AllDestroyed() );
    assert( !BaseClass::ExtraConstructions() );

    // Check that no destructor called too often.
    assert( !BaseClass::ExtraDestructions() );

    cout << "All SmartPtr tests passed!" << endl;
    return 0;
}


// ----------------------------------------------------------------------------

#include <algorithm>

struct Foo
{
};

typedef Loki::SmartPtr
<
    BaseClass, RefCounted, DisallowConversion,
    AssertCheck, DefaultSPStorage, DontPropagateConst
>
Ptr;

bool Compare( const Ptr&, const Ptr&)
{
    return true;
}

void friend_handling()
{
    // http://sourceforge.net/tracker/index.php?func=detail&aid=1570582&group_id=29557&atid=396644

    // friend injection
    // see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=28597
    std::vector<Ptr> vec;
    std::sort( vec.begin(), vec.end(), Compare );
    std::nth_element( vec.begin(), vec.begin(), vec.end(), Compare );
    std::search( vec.begin(), vec.end(),
        vec.begin(), vec.end(), Compare );
    Ptr a, b;
    Compare( a, b );

    // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=29486
    BaseClass * pNull ;
    Ptr w1( new BaseClass );
    Release( w1, pNull );

 }

// ----------------------------------------------------------------------------

// $Log$
// Revision 1.14  2006/10/17 10:36:08  syntheticpp
// change line ending
//
// Revision 1.13  2006/10/17 10:09:37  syntheticpp
// add test code for template friends with template template parameters
//
// Revision 1.12  2006/10/16 11:48:13  syntheticpp
// by default Loki is compiled without thread support, so we must disable the dependency on thread classes (StrongPtr) to avaoid linker errors when compiling with the default build process. Should  we change the default threading of Loki?
//
// Revision 1.11  2006/10/13 23:59:42  rich_sposato
// Added check for -t command line parameter to do lock-thread test.
// Changed ending chars of some lines from LF to CR-LF to be consistent.
//
// Revision 1.10  2006/10/11 11:17:53  syntheticpp
// test injected friends. Thanks to Sigoure Benoit
//
// Revision 1.9  2006/05/30 14:17:05  syntheticpp
// don't confuse with warnings
//
// Revision 1.8  2006/05/18 05:05:21  rich_sposato
// Added QueryInterface function to MimicCOM class.
//
// Revision 1.7  2006/04/28 00:34:21  rich_sposato
// Added test for thread-safe StrongPtr policy.
//
// Revision 1.6  2006/04/16 14:05:39  syntheticpp
// remove warnings
//
// Revision 1.5  2006/04/05 22:53:12  rich_sposato
// Added StrongPtr class to Loki along with tests for StrongPtr.
//
// Revision 1.4  2006/03/21 20:50:22  syntheticpp
// fix include error
//
// Revision 1.3  2006/03/17 22:52:56  rich_sposato
// Fixed bugs 1452805 and 1451835.  Added Merge ability for RefLink policy.
// Added more tests for SmartPtr.
//
// Revision 1.2  2006/03/01 02:08:11  rich_sposato
// Fixed bug 1440694 by adding check if rhs is previous neighbor.
//
// Revision 1.1  2006/02/25 01:53:20  rich_sposato
// Added test project for Loki::SmartPtr class.
//
