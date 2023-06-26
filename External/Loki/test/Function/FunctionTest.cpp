// Boost.Function library

//    Copyright Douglas Gregor 2001-2003. Use, modification and
//    distribution is subject to the Boost Software License, Version
//    1.0. (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

// $Id$


#include <functional>
#include <cassert>
#include <string>
#include <utility>

//***********************************************************
//#include <boost/test/minimal.hpp>
#include <iostream>
#define BOOST_CHECK(exp)                            \
    ( (exp)                                            \
        ? static_cast<void>(0)                        \
        : report_error(#exp,__FILE__,__LINE__) )

#define BOOST_ERROR(x) std::cout << x << "\n"
inline void
report_error( const char* msg, const char* file, int line, bool is_msg = false )
{
    std::cout << file << "(" << line << "): ";

    if( is_msg )
        std::cout << msg;
    else
        std::cout << "test " << msg << " failed";

    std::cout << std::endl;
}
//***********************************************************

#define TEST_LOKI_FUNCTION
#ifndef TEST_LOKI_FUNCTION
    #include <boost/function.hpp>
    using namespace boost;
#else
    #define BOOST_FUNCTION_TARGET_FIX(x) x
    #define LOKI_CLASS_LEVEL_THREADING
    // disable to see "static instantiation order fiasco" crash
    #define LOKI_FUNCTOR_IS_NOT_A_SMALLOBJECT
    #include <loki/Function.h>
    using namespace Loki;
    #define function Function
#endif


#ifndef LOKI_FUNCTORS_ARE_COMPARABLE


using namespace std;

int global_int;

struct write_five_obj{void operator()() const {global_int = 5;}};

#ifndef TEST_LOKI_FUNCTION
struct write_three_obj{int operator()() const {global_int = 3; return 7;}};
#else
struct write_three_obj{void operator()() const {global_int = 3;}};
#endif

static void write_five()    {global_int = 5;}
static void write_three()    {global_int = 3;}
struct generate_five_obj    {int operator()() const {return 5;}};
struct generate_three_obj    {int operator()() const {return 3;}};
static int generate_five()    {return 5;}
static int generate_three()    {return 3;}
static string identity_str(const string& s){return s;}
static string string_cat(const string& s1, const string& s2){return s1+s2;}
static int sum_ints(int x, int y){return x+y;}

struct write_const_1_nonconst_2
{
    void operator()(){global_int = 2;}
    void operator()() const {global_int = 1;}
};
struct add_to_obj
{
    add_to_obj(int v) : value(v) {}
    int operator()(int x) const {return value + x;}
    int value;
};

function<void()> static_func(write_five);

static void test_static_function()
{
    static_func();
}

static void test_zero_args()
{
    typedef function<void ()> func_void_type;

    write_five_obj five;
    write_three_obj three;

    // Default construction
    func_void_type v1;
    BOOST_CHECK(v1.empty());

    // Assignment to an empty function
    v1 = five;
    BOOST_CHECK(v1 != 0);

    // Invocation of a function
    global_int = 0;
    v1();
    BOOST_CHECK(global_int == 5);

    // clear() method
    v1.clear();
    BOOST_CHECK(v1 == 0);

    // Assignment to an empty function
    v1 = three;
    BOOST_CHECK(!v1.empty());

    // Invocation and self-assignment
    global_int = 0;
    v1 = v1;
    v1();
    BOOST_CHECK(global_int == 3);

    // Assignment to a non-empty function
    v1 = five;

    // Invocation and self-assignment
    global_int = 0;
    v1 = (v1);
    v1();
    BOOST_CHECK(global_int == 5);

    // clear
    v1 = 0;
    BOOST_CHECK(0 == v1);

    // Assignment to an empty function from a free function
    v1 = BOOST_FUNCTION_TARGET_FIX(&) write_five;
    BOOST_CHECK(0 != v1);

    // Invocation
    global_int = 0;
    v1();
    BOOST_CHECK(global_int == 5);

    // Assignment to a non-empty function from a free function
    v1 = BOOST_FUNCTION_TARGET_FIX(&) write_three;
    BOOST_CHECK(!v1.empty());

    // Invocation
    global_int = 0;
    v1();
    BOOST_CHECK(global_int == 3);

    // Assignment
    v1 = five;
    BOOST_CHECK(!v1.empty());

    // Invocation
    global_int = 0;
    v1();
    BOOST_CHECK(global_int == 5);

    // Assignment to a non-empty function from a free function
    v1 = &write_three;
    BOOST_CHECK(!v1.empty());

    // Invocation
    global_int = 0;
    v1();
    BOOST_CHECK(global_int == 3);

    // Construction from another function (that is empty)
    v1.clear();
    func_void_type v2(v1);
    BOOST_CHECK(!v2? true : false);

    // Assignment to an empty function
    v2 = three;
    BOOST_CHECK(!v2.empty());

    // Invocation
    global_int = 0;
    v2();
    BOOST_CHECK(global_int == 3);

    // Assignment to a non-empty function
    v2 = (five);

    // Invocation
    global_int = 0;
    v2();
    BOOST_CHECK(global_int == 5);

    v2.clear();
    BOOST_CHECK(v2.empty());

    // Assignment to an empty function from a free function
    v2 = (BOOST_FUNCTION_TARGET_FIX(&) write_five);
    BOOST_CHECK(v2? true : false);

    // Invocation
    global_int = 0;
    v2();
    BOOST_CHECK(global_int == 5);

    // Assignment to a non-empty function from a free function
    v2 = BOOST_FUNCTION_TARGET_FIX(&) write_three;
    BOOST_CHECK(!v2.empty());

    // Invocation
    global_int = 0;
    v2();
    BOOST_CHECK(global_int == 3);

    // Swapping
    v1 = five;
    swap(v1, v2);
    v2();
    BOOST_CHECK(global_int == 5);
    v1();
    BOOST_CHECK(global_int == 3);
    swap(v1, v2);
    v1.clear();

    // Assignment
    v2 = five;
    BOOST_CHECK(!v2.empty());

    // Invocation
    global_int = 0;
    v2();
    BOOST_CHECK(global_int == 5);

    // Assignment to a non-empty function from a free function
    v2 = &write_three;
    BOOST_CHECK(!v2.empty());

    // Invocation
    global_int = 0;
    v2();
    BOOST_CHECK(global_int == 3);

    // Assignment to a function from an empty function
    v2 = v1;
    BOOST_CHECK(v2.empty());

    // Assignment to a function from a function with a functor
    v1 = three;
    v2 = v1;
    BOOST_CHECK(!v1.empty());
    BOOST_CHECK(!v2.empty());

    // Invocation
    global_int = 0;
    v1();
    BOOST_CHECK(global_int == 3);
    global_int = 0;
    v2();
    BOOST_CHECK(global_int == 3);

    // Assign to a function from a function with a function
    v2 = BOOST_FUNCTION_TARGET_FIX(&) write_five;
    v1 = v2;
    BOOST_CHECK(!v1.empty());
    BOOST_CHECK(!v2.empty());
    global_int = 0;
    v1();
    BOOST_CHECK(global_int == 5);
    global_int = 0;
    v2();
    BOOST_CHECK(global_int == 5);

    // Construct a function given another function containing a function
    func_void_type v3(v1);

    // Invocation of a function
    global_int = 0;
    v3();
    BOOST_CHECK(global_int == 5);

    // clear() method
    v3.clear();
    BOOST_CHECK(!v3? true : false);

    // Assignment to an empty function
    v3 = three;
    BOOST_CHECK(!v3.empty());

    // Invocation
    global_int = 0;
    v3();
    BOOST_CHECK(global_int == 3);

    // Assignment to a non-empty function
    v3 = five;

    // Invocation
    global_int = 0;
    v3();
    BOOST_CHECK(global_int == 5);

    // clear()
    v3.clear();
    BOOST_CHECK(v3.empty());

    // Assignment to an empty function from a free function
    v3 = &write_five;
    BOOST_CHECK(!v3.empty());

    // Invocation
    global_int = 0;
    v3();
    BOOST_CHECK(global_int == 5);

    // Assignment to a non-empty function from a free function
    v3 = &write_three;
    BOOST_CHECK(!v3.empty());

    // Invocation
    global_int = 0;
    v3();
    BOOST_CHECK(global_int == 3);

    // Assignment
    v3 = five;
    BOOST_CHECK(!v3.empty());

    // Invocation
    global_int = 0;
    v3();
    BOOST_CHECK(global_int == 5);

    // Construction of a function from a function containing a functor
    func_void_type v4(v3);

    // Invocation of a function
    global_int = 0;
    v4();
    BOOST_CHECK(global_int == 5);

    // clear() method
    v4.clear();
    BOOST_CHECK(v4.empty());

    // Assignment to an empty function
    v4 = three;
    BOOST_CHECK(!v4.empty());

    // Invocation
    global_int = 0;
    v4();
    BOOST_CHECK(global_int == 3);

    // Assignment to a non-empty function
    v4 = five;

    // Invocation
    global_int = 0;
    v4();
    BOOST_CHECK(global_int == 5);

    // clear()
    v4.clear();
    BOOST_CHECK(v4.empty());

    // Assignment to an empty function from a free function
    v4 = &write_five;
    BOOST_CHECK(!v4.empty());

    // Invocation
    global_int = 0;
    v4();
    BOOST_CHECK(global_int == 5);

    // Assignment to a non-empty function from a free function
    v4 = &write_three;
    BOOST_CHECK(!v4.empty());

    // Invocation
    global_int = 0;
    v4();
    BOOST_CHECK(global_int == 3);

    // Assignment
    v4 = five;
    BOOST_CHECK(!v4.empty());

    // Invocation
    global_int = 0;
    v4();
    BOOST_CHECK(global_int == 5);

    // Construction of a function from a functor
    func_void_type v5(five);

    // Invocation of a function
    global_int = 0;
    v5();
    BOOST_CHECK(global_int == 5);

    // clear() method
    v5.clear();
    BOOST_CHECK(v5.empty());

    // Assignment to an empty function
    v5 = three;
    BOOST_CHECK(!v5.empty());

    // Invocation
    global_int = 0;
    v5();
    BOOST_CHECK(global_int == 3);

    // Assignment to a non-empty function
    v5 = five;

    // Invocation
    global_int = 0;
    v5();
    BOOST_CHECK(global_int == 5);

    // clear()
    v5.clear();
    BOOST_CHECK(v5.empty());

    // Assignment to an empty function from a free function
    v5 = &write_five;
    BOOST_CHECK(!v5.empty());

    // Invocation
    global_int = 0;
    v5();
    BOOST_CHECK(global_int == 5);

    // Assignment to a non-empty function from a free function
    v5 = &write_three;
    BOOST_CHECK(!v5.empty());

    // Invocation
    global_int = 0;
    v5();
    BOOST_CHECK(global_int == 3);

    // Assignment
    v5 = five;
    BOOST_CHECK(!v5.empty());

    // Invocation
    global_int = 0;
    v5();
    BOOST_CHECK(global_int == 5);

    // Construction of a function from a function
    func_void_type v6(&write_five);

    // Invocation of a function
    global_int = 0;
    v6();
    BOOST_CHECK(global_int == 5);

    // clear() method
    v6.clear();
    BOOST_CHECK(v6.empty());

    // Assignment to an empty function
    v6 = three;
    BOOST_CHECK(!v6.empty());

    // Invocation
    global_int = 0;
    v6();
    BOOST_CHECK(global_int == 3);

    // Assignment to a non-empty function
    v6 = five;

    // Invocation
    global_int = 0;
    v6();
    BOOST_CHECK(global_int == 5);

    // clear()
    v6.clear();
    BOOST_CHECK(v6.empty());

    // Assignment to an empty function from a free function
    v6 = &write_five;
    BOOST_CHECK(!v6.empty());

    // Invocation
    global_int = 0;
    v6();
    BOOST_CHECK(global_int == 5);

    // Assignment to a non-empty function from a free function
    v6 = &write_three;
    BOOST_CHECK(!v6.empty());

    // Invocation
    global_int = 0;
    v6();
    BOOST_CHECK(global_int == 3);

    // Assignment
    v6 = five;
    BOOST_CHECK(!v6.empty());

    // Invocation
    global_int = 0;
    v6();
    BOOST_CHECK(global_int == 5);

    // Const vs. non-const
    write_const_1_nonconst_2 one_or_two;
    const function<void ()> v7(one_or_two);
    function<void ()> v8(one_or_two);

    global_int = 0;
    v7();
    BOOST_CHECK(global_int == 2);

    global_int = 0;
    v8();
    BOOST_CHECK(global_int == 2);

    // Test construction from 0 and comparison to 0
    func_void_type v9(0);
    BOOST_CHECK(v9 == 0);
    BOOST_CHECK(0 == v9);

    // Test return values
    typedef function<int ()> func_int_type;
    generate_five_obj gen_five;
    generate_three_obj gen_three;

    func_int_type i0(gen_five);

    BOOST_CHECK(i0() == 5);
    i0 = gen_three;
    BOOST_CHECK(i0() == 3);
    i0 = &generate_five;
    BOOST_CHECK(i0() == 5);
    i0 = &generate_three;
    BOOST_CHECK(i0() == 3);
    BOOST_CHECK(i0? true : false);
    i0.clear();
    BOOST_CHECK(!i0? true : false);

    // Test return values with compatible types
    typedef function<long ()> func_long_type;
    func_long_type i1(gen_five);

    BOOST_CHECK(i1() == 5);
    i1 = gen_three;
    BOOST_CHECK(i1() == 3);
    i1 = &generate_five;
    BOOST_CHECK(i1() == 5);
    i1 = &generate_three;
    BOOST_CHECK(i1() == 3);
    BOOST_CHECK(i1? true : false);
    i1.clear();
    BOOST_CHECK(!i1? true : false);
}

static void test_one_arg()
{
    negate<int> neg;

    function<int (int)> f1(neg);
    BOOST_CHECK(f1(5) == -5);

    function<string (string)> id(&identity_str);
    BOOST_CHECK(id("str") == "str");

    function<string (const char*)> id2(&identity_str);
    BOOST_CHECK(id2("foo") == "foo");

    add_to_obj add_to(5);
    function<int (int)> f2(add_to);
    BOOST_CHECK(f2(3) == 8);

    const function<int (int)> cf2(add_to);
    BOOST_CHECK(cf2(3) == 8);
}

static void test_two_args()
{
    function<string (const string&, const string&)> cat(&string_cat);
    BOOST_CHECK(cat("str", "ing") == "string");

    function<int (short, short)> sum(&sum_ints);
    BOOST_CHECK(sum(2, 3) == 5);
}

static void test_emptiness()
{
    function<float ()> f1;
    BOOST_CHECK(f1.empty());

    function<float ()> f2;
    f2 = f1;
    BOOST_CHECK(f2.empty());

    function<double ()> f3;
    f3 = f2;
    BOOST_CHECK(f3.empty());
}

struct X
{
    X(int v) : value(v)
    {}

    int twice() const
    {
        return 2*value;
    }
    int plus(int v)
    {
        return value + v;
    }

    int value;
};

static void test_member_functions()
{
#ifndef TEST_LOKI_FUNCTION

    boost::function<int (X*)> f1(&X::twice);

    X one(1);
    X five(5);

    BOOST_CHECK(f1(&one) == 2);
    BOOST_CHECK(f1(&five) == 10);

#else

    X one(1);
    X five(5);

    Loki::Function<int()> f1(&one,&X::twice);
    BOOST_CHECK(f1() == 2);

    f1 = Loki::Function<int()>(&five,&X::twice);
    BOOST_CHECK(f1() == 10);

#endif



#ifndef TEST_LOKI_FUNCTION

    boost::function<int (X*)> f1_2;

    f1_2 = &X::twice;

    BOOST_CHECK(f1_2(&one) == 2);
    BOOST_CHECK(f1_2(&five) == 10);

#else

    Loki::Function<int ()> f1_2(&one,&X::twice);
    BOOST_CHECK(f1_2() == 2);

    f1_2 =    Loki::function<int ()>(&five,&X::twice);
    BOOST_CHECK(f1_2() == 10);

#endif



#ifndef TEST_LOKI_FUNCTION

    boost::function<int (X&, int)> f2(&X::plus);

    BOOST_CHECK(f2(one, 3) == 4);
    BOOST_CHECK(f2(five, 4) == 9);

#else

    Loki::Function<int (int)> f2(&one,&X::plus);
    BOOST_CHECK(f2(3) == 4);

    f2 = Loki::Function<int (int)>(&five,&X::plus);
    BOOST_CHECK(f2(4) == 9);

#endif

}

struct add_with_throw_on_copy
{
    int operator()(int x, int y) const
    {
        return x+y;
    }

    add_with_throw_on_copy()
    {}

    add_with_throw_on_copy(const add_with_throw_on_copy&)
    {
        throw runtime_error("But this CAN'T throw");
    }

    add_with_throw_on_copy& operator=(const add_with_throw_on_copy&)
    {
        throw runtime_error("But this CAN'T throw");
    }
};

static void test_ref()
{
    add_with_throw_on_copy atc;
    try
    {
#ifndef TEST_LOKI_FUNCTION
        boost::function<int (int, int)> f(ref(atc));
        BOOST_CHECK(f(1, 3) == 4);
#else
        //TODO: implement Loki::Ref
        //Loki::Function<int (int, int)> f(Loki::ref(atc));
        //BOOST_CHECK(f(1, 3) == 4);
#endif
        
    }
    catch(runtime_error e)
    {
#ifndef TEST_LOKI_FUNCTION
        BOOST_ERROR("Nonthrowing constructor threw an exception");
#endif
    }
}

static void test_exception()
{
    function<int (int, int)> f;

    try
    {
        f(5, 4);
        BOOST_CHECK(false);
    }
    catch(bad_function_call)
    {
        // okay
    }
}


typedef function< void * (void * reader) > reader_type;
typedef std::pair<int, reader_type> mapped_type;

static void test_implicit()
{
    mapped_type m;
    m = mapped_type();
}

static void test_call_obj(function<int (int, int)> f)
{
    assert(!f.empty());
}

static void test_call_cref(const function<int (int, int)>& f)
{
    assert(!f.empty());
}

static void test_call()
{
    test_call_obj(std::plus<int>());
    test_call_cref(std::plus<int>());
}

int main()
{
    test_static_function();
    test_zero_args();
    test_one_arg();
    test_two_args();
    test_emptiness();
    test_member_functions();
    test_ref();
    test_exception();
    test_implicit();
    test_call();

#if defined(__BORLANDC__) || defined(_MSC_VER)
    system("PAUSE");
#endif

    return 0;
}


#else // #ifndef LOKI_FUNCTORS_ARE_COMPARABLE
int main()
{
    return 0;
}
#endif
