/************************************/
// auther: WangChuArc               //
// date: 2020-12-31                 //
// license: MIT                     //
// github: github.com/WangChuArc    //
// e-mail: hoho71888675@126.com     //
/************************************/
#ifndef _CURRYING_
#define _CURRYING_

#ifndef _TUPLE_
#include <tuple>
#endif // _TUPLE_

#ifndef _FUNCTIONAL_
#include <functional>
#endif // _FUNCTIONAL_

static class Wildcard {} _;

namespace details
{
	template<typename ...T>
	struct alwaysFalse : std::false_type{};

	template<typename ...T>
	struct tupleCatch
	{
		static_assert(alwaysFalse<T...>::value, "shouldn't use this.");
	};

	template<typename ...TL, typename ...TR, typename ...TTail>
	struct tupleCatch<std::tuple<TL...>, std::tuple<TR...>, TTail...>
	{
		using type = typename tupleCatch<std::tuple<TL..., TR...>, TTail...>::type;
	};

	template<typename ...T>
	struct tupleCatch<std::tuple<T...>>
	{
		using type = std::tuple<T...>;
	};

	template<typename T, T V>
	struct staticInteger
	{
		static const T value = V;
	};

	template<typename T, T...TValues>
	struct integerSequence {};

	template<std::size_t ...TIndices>
	struct indexSequence : integerSequence<std::size_t, TIndices...>{};

	template<typename T, std::size_t U>
	struct add
	{
		static_assert(alwaysFalse<T>::value, "shouldn't use this.");
	};

	template<std::size_t U, std::size_t...TIndices>
	struct add<indexSequence<TIndices...>, U>
	{
		using type = indexSequence<TIndices..., U>;
	};

	template<typename TIndices, int TN>
	struct offset
	{
		static_assert(alwaysFalse<TIndices>::value, "shouldn't use this.");
	};

	template<int TN, std::size_t...TIndices>
	struct offset<indexSequence<TIndices...>, TN>
	{
		using type = indexSequence<(TIndices + TN)...>;
	};

	template<std::size_t T>
	struct makeIndexSequence
	{
		using type = typename add<typename makeIndexSequence<T - 1>::type, T - 1>::type;
	};

	template<>
	struct makeIndexSequence<1>
	{
		using type = indexSequence<0>;
	};

	template<std::size_t TBegin, std::size_t TEnd>
	struct rangeIndexSequence
	{
		static const int length = TEnd - TBegin;
		using segment = typename makeIndexSequence<length>::type;
		using type = typename offset<segment, TBegin>::type;
	};

	template<std::size_t T>
	struct rangeIndexSequence<T, T>
	{
		using type = indexSequence<>;
	};

	template<typename T, typename U>
	struct takeElements_t
	{
		static_assert(alwaysFalse<T>::value, "shouldn't use this.");
	};

	template<std::size_t...TIndices, typename T>
	struct takeElements_t<indexSequence<TIndices...>, T>
	{
		using type = std::tuple<typename std::tuple_element<TIndices, T>::type...>;
	};

	template<typename T>
	struct takeElements_t<indexSequence<>, T>
	{
		using type = std::tuple<>;
	};

	template<std::size_t...TIndices, typename TTuple>
	auto takeElements(indexSequence<TIndices...>, TTuple& _t)->typename takeElements_t<indexSequence<TIndices...>, TTuple>::type
	{
		return std::forward_as_tuple(std::get<TIndices>(_t)...);
	}

	template<std::size_t TPos, typename T>
	struct splitAt_t
	{
		static const int begin = 0;
		static const int end = std::tuple_size<T>::value;

		static_assert(TPos <= end, "where you split the tuple should not beyond the end.");

		using leftType = typename takeElements_t<typename rangeIndexSequence<begin, TPos>::type, T>::type;
		using rightType = typename takeElements_t<typename rangeIndexSequence<TPos, end>::type, T>::type;

		using type = std::tuple<leftType, rightType>;
	};

	template<std::size_t TPos, typename T>
	auto splitAt(T&& _tuple)->typename splitAt_t<TPos, T>::type
	{
		static const int begin = 0;
		static const int end = std::tuple_size<T>::value;

		static_assert(TPos <= end, "where you split the tuple should not beyond the end.");

		auto tpl = _tuple;
		auto left = takeElements(typename rangeIndexSequence<begin, TPos>::type(), tpl);
		auto right = takeElements(typename rangeIndexSequence<TPos, end>::type(), tpl);

		return std::forward_as_tuple(std::move(left), std::move(right));
	}

	template<std::size_t TPos, typename TTuple, typename TElem>
	struct replaceAt_t
	{
		static const int end = std::tuple_size<TTuple>::value;

		static_assert(TPos >= 0, "the replaced element should be atleast the first one.");
		static_assert(TPos < end, "the replaced element should be atmost the last one.");

		using fstCut = typename splitAt_t<TPos, TTuple>::type;
		using fstSeg = typename std::tuple_element<0, fstCut>::type;

		using scdSeg = std::tuple<TElem>;

		using scdCut = typename splitAt_t<1, typename std::tuple_element<1, fstCut>::type>::type;
		using thrSeg = typename std::tuple_element<1, scdCut>::type;

		using type = typename tupleCatch<fstSeg, scdSeg, thrSeg>::type;
	};

	template<std::size_t TPos, typename TTuple, typename TElem>
	auto replaceAt(TTuple&& _t, TElem&& _e)->typename replaceAt_t<TPos, TTuple, TElem>::type
	{
		auto fstCut = splitAt<TPos>(std::forward<TTuple>(_t));
		auto& fstSeg = std::get<0>(fstCut);

		auto scdSeg = std::forward_as_tuple(_e);

		auto scdCut = splitAt<1>(std::move(std::get<1>(fstCut)));
		auto& thrSeg = std::get<1>(scdCut);

		auto rst = std::tuple_cat(fstSeg, scdSeg, thrSeg);

		return rst;
	}

	template<typename TParams, typename TR>
	struct extractParamsAsFunction
	{
		static_assert(alwaysFalse<TParams>::value, "shouldn't use this.");
	};

	template<typename ...TParams, typename TR>
	struct extractParamsAsFunction<std::tuple<TParams...>, TR>
	{
		using type = std::function<TR(TParams...)>;
	};

	template<typename T>
	struct split
	{
		static_assert(alwaysFalse<T>::value, "shouldn't use this.");
	};

	template<typename THead, typename ...TTail>
	struct split<std::tuple<THead, TTail...>>
	{
		using headType = THead;
		using tailType = std::tuple<TTail...>;
	};

	template<std::size_t THead, std::size_t... TTail>
	struct split<indexSequence<THead, TTail...>>
	{
		using headType = staticInteger<std::size_t, THead>;
		using tailType = indexSequence<TTail...>;
	};

	template<typename T>
	auto head(T& _t)->typename std::tuple_element<0, T>::type
	{
		return std::get<0>(_t);
	}

	template<typename ...TArgs>
	auto tail(const std::tuple<TArgs...>& _t)->typename split<std::tuple<TArgs...>>::tailType
	{
		static const size_t end = sizeof...(TArgs);

		static_assert(end > 0, "tuple should not be empty.");

		return takeElements(typename rangeIndexSequence<1, end>::type(), _t);
	}

	template<typename T, std::size_t TN>
	struct repeat
	{
		using type = typename tupleCatch<std::tuple<T>, typename repeat<T, TN - 1>::type>::type;
	};

	template<typename T>
	struct repeat<T, 1>
	{
		using type = std::tuple<T>;
	};

	template<typename T>
	struct forward_t
	{
		using type = T&&;
	};

	template<typename T>
	struct forward_t<T&>
	{
		using type = T&;
	};

	template<typename TL, typename TR>
	struct commonType
	{
		using type = decltype(false ? declval<TL>() : declval<TR>());
	};

	template<typename TL, typename TR>
	struct mergeParam
	{
		using type = typename commonType<TL, TR>::type;
	};

	template<typename TL>
	struct mergeParam<TL, Wildcard>
	{
		using type = TL;
	};

	template<>
	struct mergeParam<Wildcard, Wildcard>
	{
		using type = Wildcard;
	};

	template<typename T>
	void validFunc(T&&){}

	template<typename T, typename U>
	void valid(U&& _arg)
	{
		validFunc<T>(std::forward<U>(_arg));
	}

	template<typename T>
	void valid(Wildcard){}

	template<typename TParams, typename THead, typename ...TTail>
	void allValid(THead&& _head, TTail&&... _tail)
	{
		using tpl = split<TParams>;
		using headType = typename tpl::headType;
		valid<headType>(std::forward<THead>(_head));

		allValid<typename tpl::tailsType>(std::forward<TTail>(_tail)...);
	}

	template<typename TParams>
	void allValid() {}

	template<typename T>
	struct whetherLack : std::is_same<typename std::decay<T>::type, Wildcard>{};

	template<bool TLack, typename TLackList, std::size_t TIdx>
	struct appendLack
	{
		using type = TLackList;
	};

	template<typename TLackList, std::size_t TIdx>
	struct appendLack<true, TLackList, TIdx>
	{
		using type = typename add<TLackList, TIdx>::type;
	};

	template<typename TLackList, typename TArgs, std::size_t TIdx>
	struct reduceToGetLackList_impl
	{
		using tupleArgs = split<TArgs>;
		using head = typename std::decay<typename tupleArgs::headType>::type;

		using args = typename tupleArgs::tailType;

		using lackList = typename appendLack<whetherLack<head>::value, TLackList, TIdx>::type;

		using type = typename reduceToGetLackList_impl<lackList, args, TIdx + 1>::type;
	};

	template<typename TLackList, std::size_t TIdx>
	struct reduceToGetLackList_impl<TLackList, std::tuple<>, TIdx>
	{
		using type = TLackList;
	};

	template<typename TArgs>
	struct reduceToGetLackList
	{
		using type = typename reduceToGetLackList_impl<indexSequence<>, TArgs, 0>::type;
	};

	template<bool TLack, typename TSaved, typename TElem, std::size_t TIdx>
	struct mergeElem_t
	{
		using type = TSaved;
	};

	template<typename TSaved, typename TElem, std::size_t TIdx>
	struct mergeElem_t<false, TSaved, TElem, TIdx>
	{
		using type = typename replaceAt_t<TIdx, TSaved, TElem>::type;
	};

	template<typename TSaved, typename TElem, std::size_t TIdx>
	auto mergeElem(std::false_type, TSaved&& _saved, TElem&& _elem, staticInteger<std::size_t, TIdx>)->typename mergeElem_t<false, TSaved, TElem, TIdx>::type
	{
		return replaceAt<TIdx>(std::forward<TSaved>(_saved), std::forward<TElem>(_elem));
	}

	template<typename TSaved, typename TElem, std::size_t TIdx>
	auto mergeElem(std::true_type, TSaved&& _saved, TElem&& _elem, staticInteger<std::size_t, TIdx>)->typename mergeElem_t<true, TSaved, TElem, TIdx>::type
	{
		return _saved;
	}

	template<typename TSaved, typename TArgs, typename TLacks>
	struct merge_t
	{
		static const int idx = split<TLacks>::headType::value;
		using elem = typename split<TArgs>::headType;
		using saved = typename mergeElem_t<whetherLack<elem>::value, TSaved, elem, idx>::type;
		using args = typename split<TArgs>::tailType;
		using lacks = typename split<TLacks>::tailType;

		using type = typename merge_t<saved, args, lacks>::type;
	};

	template<typename TSaved, typename TLacks>
	struct merge_t<TSaved, std::tuple<>, TLacks>
	{
		using type = TSaved;
	};

	template<typename TSaved, typename TArgs, typename TLacks>
	auto merge(TSaved&& _saved, TArgs&& _args, TLacks&& _lacks)->typename merge_t<TSaved, TArgs, TLacks>::type
	{
		static const int idx = split<TLacks>::headType::value;
		auto&& elem = head(_args);
		auto saved = mergeElem(typename whetherLack<decltype(elem)>::type(), std::forward<TSaved>(_saved), std::forward<decltype(elem)>(elem), staticInteger<std::size_t, idx>());
		auto args = tail(_args);
		using lacks = typename split<TLacks>::tailType;

		return merge(std::move(saved), std::move(args), lacks());
	}

	template<typename TSaved, typename TLacks>
	auto merge(TSaved&& _saved, std::tuple<>, TLacks&& _lacks)->TSaved
	{
		return _saved;
	}

	template<typename T>
	struct empty : std::false_type{};

	template<>
	struct empty<indexSequence<>> : std::true_type{};

	template<typename TR, typename ...TParams, std::size_t...TIndices, typename...TArgs>
	auto invoke(std::function<TR(TParams...)>&& _f, indexSequence<TIndices...>, std::tuple<TArgs...>&& _args)->TR
	{
		return _f(std::get<TIndices>(_args)...);
	}

	template<typename T>
	struct functionProto
	{
		static_assert(alwaysFalse<T>::value, "don't use this.");
	};

	template<typename TR, typename TClass, typename...TParams>
	struct functionProto<TR(TClass::*)(TParams...) const>
	{
		using type = std::function<TR(TParams...)>;
	};
}

template<bool lazy, typename TR, typename TParams, typename TArgs, typename TLacks>
class CurriedFunction;

template<typename T, typename ...TParams>
struct currying_t
{
	using type = CurriedFunction<false, T, std::tuple<TParams...>, typename details::repeat<Wildcard, sizeof...(TParams)>::type, typename details::makeIndexSequence<sizeof...(TParams)>::type>;
};

template<typename TR, typename ...TParams>
struct currying_t<std::function<TR(TParams...)>>
{
	using type = typename currying_t<TR, TParams...>::type;
};

template<typename TR, typename ...TParams>
auto currying(const std::function<TR(TParams...)>& _f)->typename currying_t<TR, TParams...>::type;

template<typename TR, typename ...TParams>
auto currying(std::function<TR(TParams...)>&& _f)->typename currying_t<TR, TParams...>::type;

template<bool lazy, typename TR, typename TParams, typename TArgs, typename TLacks>
class CurriedFunction
{
private:
	template<typename U, typename ...Us>
	friend auto currying(const std::function<U(Us...)>&)->typename currying_t<U, Us...>::type;

	template<typename U, typename ...Us>
	friend auto currying(std::function<U(Us...)>&&)->typename currying_t<U, Us...>::type;

	template<bool U1, typename U2, typename U3, typename U4, typename U5>
	friend class CurriedFunction;

public:
	using funcType = typename details::extractParamsAsFunction<TParams, TR>::type;

private:
	CurriedFunction(funcType&& _f, TArgs&& _args) : m_func(_f), m_args(_args){}

	template<typename UEmpty, typename UArgs, typename ULackList>
	struct invoke_t_impl
	{
		using type = CurriedFunction<lazy, TR, TParams, UArgs, ULackList>;
	};

	template<typename UArgs, typename ULackList>
	struct invoke_t_impl<std::true_type, UArgs, ULackList>
	{
		using type = TR;
	};

	template<typename ...UArgs>
	struct invoke_t
	{
		//using args = std::tuple<typename details::forward_t<UArgs>::type...>;
		using args = std::tuple<UArgs...>;
		using merged = typename details::merge_t<TArgs, args, TLacks>::type;
		using lackList = typename details::reduceToGetLackList<merged>::type;
		using type = typename invoke_t_impl<typename details::empty<lackList>::type, merged, lackList>::type;
	};

	template<typename UArgs, typename ULacks>
	auto invoke(std::false_type, funcType&& _f, UArgs&& _args, ULacks)->CurriedFunction<lazy, TR, TParams, UArgs, ULacks>
	{
		return CurriedFunction<lazy, TR, TParams, UArgs, ULacks>{ std::move(_f), std::move(_args) };
	}

	template<typename UArgs, typename ULacks>
	auto invoke(std::true_type, funcType&& _f, UArgs&& _args, ULacks)->TR
	{
		static const int size = std::tuple_size<UArgs>::value;
		using indices = typename details::makeIndexSequence<size>::type;

		return details::invoke(std::move(_f), indices(), std::move(_args));
	}

	template<typename ULacks>
	auto invoke(std::true_type, funcType&& _f, std::tuple<>&&, ULacks)->TR
	{
		return _f();
	}

public:
	template<typename ...U>
	auto operator()(U&&... _args)->typename invoke_t<U...>::type
	{
		//auto args = std::forward_as_tuple(std::forward<U>(_args)...);
		auto args = std::tuple<U...>(std::forward<U>(_args)...);
		auto merged = details::merge(std::move(m_args), std::move(args), TLacks());

		using mergedType = decltype(merged);
		using lackList = typename details::reduceToGetLackList<mergedType>::type;

		return invoke(typename details::empty<lackList>::type(), std::move(m_func), std::move(merged), lackList());
	}

private:
	funcType m_func;
	TArgs m_args;
};

template<typename TR, typename ...TParams>
auto currying(std::function<TR(TParams...)>&& _f)->typename currying_t<TR, TParams...>::type
{
	static const std::size_t size = sizeof...(TParams);
	using repeat = typename details::repeat<Wildcard, size>::type;
	using indices = typename details::makeIndexSequence<size>::type;

	return CurriedFunction<false, TR, std::tuple<TParams...>, repeat, indices>(std::move(_f), repeat());
}

template<typename TR, typename ...TParams>
auto currying(const std::function<TR(TParams...)>& _f)->typename currying_t<TR, TParams...>::type
{
	static const std::size_t size = sizeof...(TParams);
	using repeat = typename details::repeat<Wildcard, size>::type;
	using indices = typename details::makeIndexSequence<size>::type;

	std::function<TR(TParams...)> f = _f;	

	return CurriedFunction<false, TR, std::tuple<TParams...>, repeat, indices>(std::move(f), repeat());
}

template<typename TR, typename ...TParams>
auto makeFunction(TR(*_f)(TParams...))->std::function<TR(TParams...)>
{
	return std::function<TR(TParams...)>(_f);
}

template<typename TR, typename ...TParams>
auto makeFunction(TR(&_f)(TParams...))->std::function<TR(TParams...)>
{
	return std::function<TR(TParams...)>(_f);
}

template<typename TR, typename TClass, typename ...TParams>
auto makeFunction(TR(TClass::*_f)(TParams...))->std::function<TR(TClass*, TParams...)>
{
	return std::function<TR(TParams...)>(_f);
}

template<typename TFunctor, typename TF = decltype(&std::decay<TFunctor>::type::operator())>
auto makeFunction(TFunctor&& _f)->typename details::functionProto<TF>::type
{
	TFunctor f = _f;
	typename details::functionProto<TF>::type rst(f);
	return rst;
}

template<typename TR, typename ...TParams>
auto currying(TR*(*_f)(TParams...))->typename currying_t<TR, TParams...>::type
{
	auto f = makeFunction(_f);
	auto rst = currying(forward<decltype(f)>(f));

	return rst;
}

template<typename TR, typename ...TParams>
auto currying(TR*(&_f)(TParams...))->typename currying_t<TR, TParams...>::type
{
	auto f = makeFunction(_f);
	auto rst = currying(forward<decltype(f)>(f));

	return rst;
}

template<typename TR, typename TClass, typename ...TParams>
auto currying(TR*(TClass::*_f)(TParams...))->typename currying_t<TR, TClass, TParams...>::type
{
	auto f = makeFunction(_f);
	auto rst = currying(forward<decltype(f)>(f));

	return rst;
}

template<typename TFunctor, typename TF = decltype(&std::decay<TFunctor>::type::operator())>
auto currying(TFunctor&& _f)->typename currying_t<typename details::functionProto<TF>::type>::type
{
	auto f = makeFunction(std::forward<TFunctor>(_f));
	auto rst = currying(std::forward<decltype(f)>(f));

	return rst;
}

#endif // _CURRYING_
