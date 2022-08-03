#include <fas/testing.hpp>
#include <wflow/owner.hpp>

class foo
{
public:
  foo() {++ctor;}
  foo(const foo& ) {++cctor;}
  foo(foo&& ) {++mctor;}
public:
  static int ctor ;
  static int cctor ;
  static int mctor ;
};

int foo::ctor = 0;
int foo::cctor = 0;
int foo::mctor = 0;

int inc(int i);
int dec(int i);

int inc(int i)
{
  return i+1;
}


int dec(int i)
{
  return i-1;
}

struct finc{ int operator()(int val) const { return val+1;}};
struct fdec{ int operator()(int dec) const { return dec-1;}};

template<typename Handle>
void rec(Handle&& h, int i)
{
  if ( i == 0 ) h();
  else rec( std::forward<Handle>(h), i-1);
}

template<typename Handle>
void rec2(Handle h, int i)
{
  if ( i == 0 ) h();
  else rec( h, i-1);
}

UNIT(wrap, "")
{
  using namespace fas::testing;

  foo::ctor = 0;
  foo::cctor = 0;
  foo::mctor = 0;

  wflow::owner own;
  auto test1 = own.wrap(&inc, nullptr);
  int val = 0;
  val = test1(1);
  t << equal<expect>(val, 2) << FAS_FL;
  own.reset();
  val = test1(1);
  t << equal<expect>(val, 0) << FAS_FL;
  
  auto test2 = own.wrap(&inc, &dec);
  val = test2(2);
  t << equal<expect>(val, 3) << FAS_FL;
  own.reset();
  val = test2(2);
  t << equal<expect>(val, 1) << FAS_FL;
  
  foo f;
  auto test3 = own.wrap([f](int v)->int { return v + 1;}, nullptr);
  val = test3(1);
  t << equal<expect>(val, 2) << FAS_FL;
  t << equal<expect>(foo::ctor, 1) << FAS_FL;
  t << equal<expect>(foo::cctor, 1) << foo::cctor << FAS_FL;
  t << equal<expect>(foo::mctor, 1) << FAS_FL;
  own.reset();
  val = test3(1);
  t << equal<expect>(val, 0) << FAS_FL;


  auto test4 = own.wrap([f](int v)->int { return v + 1;}, [f](int v)->int { return v - 1;});
  val = test4(2);
  t << equal<expect>(val, 3) << FAS_FL;
  own.reset();
  val = test4(2);
  t << equal<expect>(val, 1) << FAS_FL;

  t << equal<expect>(foo::ctor, 1) << FAS_FL;
  t << equal<expect>(foo::cctor, 3) << foo::cctor << FAS_FL;
  t << equal<expect>(foo::mctor, 3) << foo::mctor << FAS_FL;
  
  rec([f](){}, 10);
  t << equal<expect>(foo::cctor, 4) << foo::cctor << FAS_FL;
  t << equal<expect>(foo::mctor, 3) << foo::mctor << FAS_FL;

  rec2([f](){}, 10);
  t << equal<expect>(foo::cctor, 5) << foo::cctor << FAS_FL;
  t << equal<expect>(foo::mctor, 3) << foo::mctor << FAS_FL;
  
  auto test5 = test4;
  val = test5(2);
  t << equal<expect>(val, 1) << FAS_FL;

  auto test6 = own.wrap([f](int v)->int { return v + 1;}, nullptr);
  auto test7 = test6;
  val = test7(2);
  t << equal<expect>(val, 3) << FAS_FL;
  own.reset();
  val = test7(2);
  t << equal<expect>(val, 0) << FAS_FL;
  
  auto test8 = own.wrap(finc(), fdec());
  //auto test8 = own.wrap(&inc, &dec);
  //auto test8 = own.wrap([f](int val)->int { return val + 1;}, [f](int val)->int { return val - 1;});
  auto test9 = test8;
  val = test9(2);
  t << equal<expect>(val, 3) << val << FAS_FL;
  own.reset();
  val = test9(2);
  t << equal<expect>(val, 1) << val << FAS_FL;
}

UNIT(wrap_callback, "")
{
  using namespace fas::testing;
  foo::ctor = 0;
  foo::cctor = 0;
  foo::mctor = 0;

  wflow::owner own;
  auto test1 =own.wrap(  own.callback( &inc), nullptr );
  int val = 0;
  val = test1(1);
  t << equal<expect>(val, 2) << FAS_FL;
  own.reset();
  val = test1(1);
  t << equal<expect>(val, 0) << FAS_FL;
  
  auto test2 =own.wrap(  own.callback( &inc), &dec );
  val = test2(2);
  t << equal<expect>(val, 3) << FAS_FL;
  own.reset();
  val = test2(2);
  t << equal<expect>(val, 1) << FAS_FL;
  
  foo f;
  auto test3 = own.wrap( own.callback([f](int v)->int { return v + 1;}), nullptr);
  val = test3(1);
  t << equal<expect>(val, 2) << FAS_FL;
  t << equal<expect>(foo::ctor, 1) << FAS_FL;
  t << equal<expect>(foo::cctor, 1) <<  FAS_FL;
  t << equal<expect>(foo::mctor, 2) << FAS_FL;
  own.reset();
  val = test3(1);
  t << equal<expect>(val, 0) << FAS_FL;


  auto test4 = own.wrap( own.callback([f](int v)->int { return v + 1;}), [f](int v)->int { return v - 1;});
  val = test4(2);
  t << equal<expect>(val, 3) << FAS_FL;
  own.reset();
  val = test4(2);
  t << equal<expect>(val, 1) << FAS_FL;

  t << equal<expect>(foo::ctor, 1) << FAS_FL;
  t << equal<expect>(foo::cctor, 3) << foo::cctor << FAS_FL;
  t << equal<expect>(foo::mctor, 5) << foo::mctor << FAS_FL;
  
  rec([f](){}, 10);
  t << equal<expect>(foo::cctor, 4) << foo::cctor << FAS_FL;
  t << equal<expect>(foo::mctor, 5) << foo::mctor << FAS_FL;

  rec2([f](){}, 10);
  t << equal<expect>(foo::cctor, 5) << foo::cctor << FAS_FL;
  t << equal<expect>(foo::mctor, 5) << foo::mctor << FAS_FL;
  
  auto test5 = test4;
  val = test5(2);
  t << equal<expect>(val, 1) << FAS_FL;

  auto test6 = own.wrap( own.callback([f](int v)->int { return v + 1;}), nullptr);
  auto test7 = test6;
  val = test7(2);
  t << equal<expect>(val, 3) << FAS_FL;
  own.reset();
  val = test7(2);
  t << equal<expect>(val, 0) << FAS_FL;
  
  auto test8 = own.wrap( own.callback(finc()), fdec());
  //auto test8 = own.wrap(&inc, &dec);
  //auto test8 = own.wrap([f](int val)->int { return val + 1;}, [f](int val)->int { return val - 1;});
  auto test9 = test8;
  val = test9(2);
  t << equal<expect>(val, 3) << val << FAS_FL;
  own.reset();
  val = test9(2);
  t << equal<expect>(val, 1) << val << FAS_FL;
}

UNIT(callback1, "")
{
  using namespace fas::testing;
  int count = 0;
  int dcount = 0;
  int ncount = 0;
  
  wflow::owner own;
  auto cb1 = own.callback([&count](){count++;});
  cb1();
  t << equal<expect>(count, 1) << FAS_FL;
  cb1();
  t << equal<expect>(count, 1) << FAS_FL;
  
  own.set_double_call_handler([&dcount]() noexcept {++dcount;});
  own.set_no_call_handler([&ncount]() noexcept {++ncount;});

  auto cb2 = own.callback([&count]() noexcept {count++;});
  cb1();
  t << equal<expect>(count, 1) << FAS_FL;
  cb2();
  t << equal<expect>(count, 2) << FAS_FL;
  cb2();
  t << equal<expect>(count, 2) << FAS_FL;
  t << equal<expect>(dcount, 1) << FAS_FL;
  t << equal<expect>(ncount, 0) << FAS_FL;
  {
    auto cb3 = own.callback([&count](){count++;});
  }
  t << equal<expect>(count, 2) << FAS_FL;
  t << equal<expect>(dcount, 1) << FAS_FL;
  t << equal<expect>(ncount, 1) << FAS_FL;

}

BEGIN_SUITE(owner, "")
  ADD_UNIT(wrap)
  ADD_UNIT(wrap_callback)  
  ADD_UNIT(callback1)  
END_SUITE(owner)


BEGIN_TEST
  RUN_SUITE(owner)
END_TEST
