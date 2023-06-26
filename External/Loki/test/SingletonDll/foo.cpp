////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2006 Peter K�mmel
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author makes no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

// $Id$


#include "foo.h"
#include <iostream>

Foo::Foo()
{
}

void Foo::foo()
{
    std::cout << "\nFoo:foo() called,  this: " << this << "\n";
}
