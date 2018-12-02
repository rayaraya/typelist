#include <iostream>
#include "test_runner.h"
#include <string>
#define _15INTS int, int, int, int, int, int, int, int, int, int, int, int, int, int, int
#define LOG(...) cout << #__VA_ARGS__ << endl;

class NullType
{
};
template <typename T = NullType, typename... U>
struct TypeList
{
	using head = T;
	using tail = TypeList<U...>;
};
template <typename T>
struct TypeList<T>
{
	using head = T;
	using tail = NullType;
};
using CharList = TypeList<char, signed char, unsigned char>;
using EmptyList = TypeList<>;

template <typename TList>
struct Length
{
	enum
	{
		value = Length<typename TList::tail>::value + 1
	};
};
template <typename TList>
struct Length;
template <>
struct Length<NullType>
{
	enum
	{
		value = 0
	};
};
template <>
struct Length<TypeList<NullType>>
{
	enum
	{
		value = 0
	};
};

template <class TList, unsigned int pos>
struct TypeAt;
template <class TList, unsigned int pos>
struct TypeAt
{
	using type = typename TypeAt<typename TList::tail, pos - 1>::type;
};

template <unsigned int pos>
struct TypeAt<TypeList<NullType>, pos>
{
	using type = NullType;
};
template <class TList>
struct TypeAt<TList, 0u>
{
	using type = typename TList::head;
};

template <class TList, template <class RealParameter, class InheritParameter> class Unit, class Root = NullType>
class GenLinearHierarchy : public Unit<typename TList::head,
									   GenLinearHierarchy<typename TList::tail, Unit, Root>>
{
};

template <typename T, template <class, class> class Unit, class Root>
class GenLinearHierarchy<TypeList<T>, Unit, Root> : public Unit<T, Root>
{
};

template <template <class> class Unit, typename... Types>
class GenScatterHierarchy;

template <template <class RealParameter> class Unit, class... Types>
class GenScatterHierarchy<Unit, TypeList<Types...>> : public Unit<Types>...
{
};

template <unsigned int N>
struct Fibonacci
{
	enum
	{
		value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value
	};
};
template <>
struct Fibonacci<0>
{
	enum
	{
		value = 1
	};
};
template <>
struct Fibonacci<1>
{
	enum
	{
		value = 1
	};
};

template <typename T, class TList>
struct Append;

template <typename T, class... Types>
struct Append<T, TypeList<Types...>>
{
	using toBegin = TypeList<T, Types...>;
	using toEnd = TypeList<Types..., T>;
};

template <unsigned int pos, class TList>
struct Slice
{
	using front = typename Append<typename TList::head, typename Slice<pos - 1, typename TList::tail>::front>::toBegin;
	using back = typename Slice<pos - 1, typename TList::tail>::back;
};
template <class TList>
struct Slice<0, TList>
{
	using front = TypeList<>;
	using back = TList;
};
template <unsigned int pos>
struct Slice<pos, TypeList<>>
{
	using front = TypeList<>;
	using back = TypeList<>;
};
template <unsigned int pos>
struct Slice<pos, NullType>
{
	using front = TypeList<>;
	using back = TypeList<>;
};

template <class TList, unsigned int pos = 0>
struct SliceForBranches
{
	using sliced = Slice<Fibonacci<pos>::value + 1, TList>;
	using result = typename Append<typename sliced::front, typename SliceForBranches<
															   typename sliced::back, pos + 1>::result>::toBegin;
};
template <unsigned int pos>
struct SliceForBranches<TypeList<>, pos>
{
	using result = EmptyList;
};

template <class TTList, template <class RealParameter, class InheritParameter> class Unit>
struct MakeTypesFromBranchSlices
{
	using result = typename Append<GenLinearHierarchy<typename TTList::head, Unit>,
								   typename MakeTypesFromBranchSlices<typename TTList::tail, Unit>::result>::toBegin;
};
template <template <class, class> class Unit>
struct MakeTypesFromBranchSlices<TypeList<EmptyList>, Unit>
{
	using result = NullType;
};

template <class TList, template <class, class> class LinearUnit, template <class> class ScatterUnit>
class GenerateFibonacciHierarchy
{
	using result = GenScatterHierarchy<ScatterUnit, MakeTypesFromBranchSlices<SliceForBranches<TList>, LinearUnit>>;
};

template <typename T, class Inherit>
class IHolder : public Inherit
{
  public:
	IHolder()
	{
	}
	T data;
};

template <typename T>
class Holder
{
  public:
	Holder()
	{
	}
	T data;
};

void TestTL()
{
	//length
	ASSERT_EQUAL(0, Length<EmptyList>::value);
	ASSERT_EQUAL(3, Length<CharList>::value);
	ASSERT_EQUAL(1, Length<TypeList<double>>::value);

	//type at
	TypeAt<CharList, 0u>::type c = 'h';
	ASSERT_EQUAL('h', c);
	TypeAt<TypeList<char, int>, 1u>::type i = 3.14;
	ASSERT_EQUAL(3, i);

	//lin hieararchy
	using CivH = GenLinearHierarchy<TypeList<char, int, vector<int>>, IHolder>;
	CivH civH;

	//Fib
	ASSERT_EQUAL(8, Fibonacci<5>::value);

	//Append
	using CharIntVecList = TypeList<char, int, vector<int>>;
	using appendedF = typename Append<double, CharIntVecList>::toBegin;
	using appendedB = typename Append<double, CharIntVecList>::toEnd;
	ASSERT_EQUAL(4, Length<appendedF>::value);
	ASSERT_EQUAL(4, Length<appendedB>::value);
	TypeAt<appendedF, 0u>::type df = 3.14;
	ASSERT_EQUAL(3.14, df);
	TypeAt<appendedB, 3u>::type db = 3.14;
	ASSERT_EQUAL(3.14, db);

	//Slice

	using slicedM = Slice<2, appendedF>;
	ASSERT_EQUAL(2, Length<slicedM::front>::value);
	ASSERT_EQUAL(2, Length<slicedM::back>::value);

	using slicedFull = Slice<13, appendedF>;
	ASSERT_EQUAL(4, Length<slicedFull::front>::value);
	ASSERT_EQUAL(0, Length<slicedFull::back>::value);
	TypeAt<slicedFull::front, 0u>::type df2 = 3.14;
	ASSERT_EQUAL(3.14, df2);
	TypeAt<slicedFull::front, 3u>::type v = {1, 2};
	ASSERT_EQUAL(vector<int>({1, 2}), v);

	//Branches
	using BranchTL = SliceForBranches<TypeList<_15INTS>>::result;
	ASSERT_EQUAL(5, Length<BranchTL>::value);

	cout << Length<TypeAt<BranchTL, 0>::type>::value << endl;
	cout << Length<TypeAt<BranchTL, 1>::type>::value << endl;
	cout << Length<TypeAt<BranchTL, 2>::type>::value << endl;
	cout << Length<TypeAt<BranchTL, 3>::type>::value << endl;
	cout << Length<TypeAt<BranchTL, 4>::type>::value << endl;
	/* 
        root_          
 					|\ \_
				/|| \  \
			 / || |  |
			/  ||	 \	\
		 /  / |   |  \
    /  |  |   |   |
   /	/   |   |   |
	#		#		#		#		#
	#		#		#		#		#
					#		#		#
							#		#
*/
	//Finally

	using FibInh = GenerateFibonacciHierarchy<TypeList<_15INTS>, IHolder, Holder>;
	FibInh finally;

	//cout << static_cast<Holder<char, NullType>&>(cid).data;
}

int main()
{
	//using WidgetInfo = GenScatterHierarchy<Holder, TypeList<int, string>>;
	//WidgetInfo w;
	//cout << GenScatterHierarchy<TypeList<char, int, int>, Perviy>::value << endl;
	TestRunner tr;
	RUN_TEST(tr, TestTL);
	return 0;
}