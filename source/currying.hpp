#include<tuple>
#include<vector>
#include<utility>
#include<functional>


namespace FPinCpp
{
    using namespace std;
    namespace CurryTraits
    {
        struct _WildCard_ {};
        const _WildCard_ _;

        template<typename T>
        struct ExtractFuncArgNum {};

        template<typename TR, typename ...TArgs>
        struct ExtractFuncArgNum<TR(TArgs...)> { enum { value = sizeof...(TArgs) }; };

        template<typename T>
        struct ExtractFuncType {};

        template<typename TR, typename ...TArgs>
        struct ExtractFuncType<function<TR(TArgs...)>> { typedef TR(type)(TArgs...); };

        template<typename>
        struct SizeofIndexSequence { enum { value = sizeof...(TI) }; };

        template<size_t ...TI>
        struct SizeofIndexSequence<index_sequence<TI...>> { enum { value = sizeof...(TI) }; };

        template<typename>
        struct getTupleProtoFromFunc{};

        template<typename TR, typename ...TArgs>
        struct getTupleProtoFromFunc<TR(TArgs...)> { typedef tuple<TArgs...> type; };

        template<typename T>
        struct NotWildcard 
        {
            static const bool value = true;
            static const size_t cnt = 1;
        };

        template<>
        struct NotWildcard<_WildCard_>
        {
            static const bool value = false; 
            static const size_t cnt = 0;
        };

        template<typename ...TArgs>
        struct ArgNumWithoutWildcard
        {
            enum { value = argNumWithoutWildcard((TArgs*)(nullptr)...) };
        };

        template<typename ...TArgs>
        struct ArgNumWithoutWildcard<tuple<TArgs...>*>
        {
            enum { value = argNumWithoutWildcard((TArgs*)(nullptr)...) };
        };

        constexpr size_t argNumWithoutWildcard()
        {
            return 0;
        };

        template<typename THead, typename ...TTails, typename = typename enable_if<!is_same<THead, _WildCard_>::value>::type>
        constexpr size_t argNumWithoutWildcard(THead*, TTails* ...args)
        {
            return 1 + argNumWithoutWildcard(args...);
        }

        template<typename ...TTails>
        constexpr size_t argNumWithoutWildcard(_WildCard_*, TTails* ...args)
        {
            return 0 + argNumWithoutWildcard(args...);
        }

        template<typename ...TTupArgs>
        constexpr size_t argNumWithoutWildcard(const tuple<TTupArgs...> &tup)
        {
            return argNumWithoutWildcard_imp(tup, make_index_sequence<sizeof...(TTupArgs)>());
        }

        template<typename ...TTupArgs, size_t ...TSeq>
        constexpr size_t argNumWithoutWildcard_imp(const tuple<TTupArgs...> &tup, index_sequence<TSeq...> seq)
        {
            return argNumWithoutWildcard(&get<TSeq>(tup)...);
        }

        template<size_t T1, size_t T2>
        struct IsLess { static const bool value = T1 < T2; };
    }



    using namespace CurryTraits;

    template<typename TFunc, typename ...TArgs>
    struct Curried;

    /**********************************************************************************************************************************/
    // 构造curried对象首先需要构造std:function对象。对于一般函数，function对象与原函数参数列表相同,在currying函数中构造。
    // 对于成员函数和functor、lambda表达式来说，function对象的参数列表，比原函数多了一个对象参数(与this指针作用类似)，在_makeFuncObj函数中构造。
    //
    // 对于成员函数，currying函数返回的curried对象(本质是functor)的第一个参数是成员所属类的对象指针，由使用者管理。因此参数比原函数多一个。
    // 对于functor、lambda表达式，如果传入currying函数的是对象(值传递),则返回的curried对象会保存传入对象的副本(栈中)；如果传入currying函数的
    //是指针，则返回的curried对象会保存传入的指针，其指向对象由使用者管理。此操作由currying函数完成，因此返回的curried对象参数数量不变。
    // 最后的curried对象会保存的参数是值还是指针，实际上从调用_makeFuncObj函数时就决定了，因此由_makeFuncObj函数的第二个参数的参数类型来决定
    //保存值还是指针。这个写法有点蠢，盖因函数模板没有偏特化，只能用函数重载来实现。
    /**********************************************************************************************************************************/

    template<typename TPtr, typename = typename enable_if<is_pointer<TPtr>::value>::type, typename TClass, typename TR, typename ...TArgs>
    auto _makeFuncObj(TR(TClass::*func)(TArgs...), TPtr*)
    {
        typedef TR(type)(TClass*, TArgs...);
        function<type> f = [func](TClass* pObj, TArgs ...args) { return (pObj->*func)(args...); };
        return move(f);
    }

    template<typename TPtr, typename = typename enable_if<is_pointer<TPtr>::value>::type, typename TClass, typename TR, typename ...TArgs>
    auto _makeFuncObj(TR(TClass::*func)(TArgs...) const, TPtr*)
    {
        typedef TR(type)(TClass*, TArgs...);
        function<type> f = [func](TClass* pObj, TArgs ...args) { return (pObj->*func)(args...); };
        return move(f);
    }

    template<typename SFINAE = void, typename TPtr, typename = typename enable_if<!is_pointer<TPtr>::value>::type, typename TClass, typename TR, typename ...TArgs>
    auto _makeFuncObj(TR(TClass::*func)(TArgs...), TPtr*)
    {
        typedef TR(type)(TClass&&, TArgs...);
        function<type> f = [func](TClass&& pObj, TArgs ...args) { return (pObj.*func)(args...); };
        return move(f);
    }

    template<typename SFINAE = void, typename TPtr, typename = typename enable_if<!is_pointer<TPtr>::value>::type, typename TClass, typename TR, typename ...TArgs>
    auto _makeFuncObj(TR(TClass::*func)(TArgs...) const, TPtr*)
    {
        typedef TR(type)(TClass&&, TArgs...);
        function<type> f = [func](TClass&& pObj, TArgs ...args) { return (pObj.*func)(args...); };
        return move(f);
    }

    template<typename TFunc, typename = typename enable_if<is_function<typename remove_pointer<TFunc>::type>::value>::type>
    auto Currying(TFunc func) -> Curried<function<typename remove_pointer<TFunc>::type>>
    {
        typedef remove_pointer<TFunc>::type F;
        function<F> f = func;
        return Curried<function<F>>(move(f));
    }

    template<typename TFunc, typename = typename enable_if<is_member_function_pointer<TFunc>::value>::type>
    auto Currying(TFunc func)
    {
        void** ptr_of_ptr = nullptr;
        auto f = _makeFuncObj(func, ptr_of_ptr);
        return Curried<decltype(f)>(move(f));
    }

    template<typename TFunctor, typename TFunc = decltype(&remove_pointer<TFunctor>::type::operator()), typename avoidRedefine = void>
    auto Currying(TFunctor func)
    {
        auto f = _makeFuncObj(&remove_pointer<TFunctor>::type::operator(), &func);
        auto g = Curried<decltype(f)>(move(f));
        return g(func);
    }


    template<typename TFunc, typename ...TArgs>
    struct Curried
    {
        typedef typename ExtractFuncType<TFunc>::type functionType; /* TFunc == function<functionType> */
        typedef tuple<TArgs...> thisTupleType;
        typedef typename getTupleProtoFromFunc<functionType>::type CurriedTupleType;
        enum { funcArgNum = ExtractFuncArgNum<functionType>::value, 
               tupleArgNum = sizeof...(TArgs), 
               tupleArgNumWithoutWildcard = ArgNumWithoutWildcard<TArgs...>::value };
        static_assert(tupleArgNum <= funcArgNum, "Curry, Too Many Args Passed"); 

        tuple<TArgs...> m_argsTuple;
        TFunc m_func; /* TFunc == function<functionType> */

        template<size_t idx>
        struct Idx { static const size_t value = idx; };

        template<typename ...TArgs_>
        Curried(TFunc func, tuple<TArgs_...>&& tup) :m_func(func), m_argsTuple(tup) {};
        Curried(TFunc&& func) :m_func(func) {};
        Curried() :m_func(nullptr) {};

    public:

        template<size_t TIdx, typename TEle, typename TArg>
        struct checkElemType_imp
        {
            static_assert(is_same<TEle, TArg>::value, "An Argument You Passed Has Wrong Type!!");
        };

        template<size_t TIdx, typename TEle>
        struct checkElemType_imp<TIdx, _WildCard_, TEle>
        {
            /* Always right */
        };

        template<typename ...T>
        void doNothing(T...) {};

        template<size_t ...TSeq, typename TTup, typename TR, typename ...TArgs>
        void checkElemType(TTup& tup, index_sequence<TSeq...>, TR(*)(TArgs...))
        {
            typedef tuple<TArgs...> FuncArgTypes;
            //doNothing(checkElemType_imp<TSeq, typename tuple_element<TSeq, TTup>::type, typename tuple_element<TSeq, FuncArgTypes>::type>()...);
        }

        template<typename ...TArgs, size_t ...seq>
        auto execute(tuple<TArgs...>& tuple, index_sequence<seq...>)
        {
            return m_func(move(get<seq>(tuple))...);
        }

        template<typename ...TArgs_>
        auto operator()(TArgs_&& ...args)
        {
            static_assert(tupleArgNumWithoutWildcard + sizeof...(TArgs_) <= funcArgNum, "Passed too many arguments!!");
            auto nt = buildTuple(tuple<>(), Idx<0>(), forward<TArgs_>(args)...);
            typedef decltype(nt) NewTupleType;
            checkElemType(nt, make_index_sequence<tuple_size<NewTupleType>::value>(), (functionType*)(nullptr));
            return handleReturn(nt, conditional<ArgNumWithoutWildcard<NewTupleType*>::value == funcArgNum, true_type, false_type>::type());
        }
    
        template<typename ...TTupArgs, typename THead, typename ...TTails, size_t TIdx, typename = typename enable_if<IsLess<TIdx , tupleArgNum>::value>::type>
        auto buildTuple(tuple<TTupArgs...>&& nt, Idx<TIdx>&&, THead&& head, TTails&& ...tails)
        {
            typedef tuple<TTupArgs...> tpType;
            auto nt_ = catchTupleAndCall(nt, Idx<TIdx>(), get<TIdx>(m_argsTuple), forward<THead>(head), forward<TTails>(tails)...);
            return nt_;
        }

        template<typename ...TTupArgs, typename THead, typename ...TTails>
        auto buildTuple(tuple<TTupArgs...>&& nt, Idx<tupleArgNum>&&, THead&& head, TTails&& ...tails)
        {
            return tuple_cat(move(nt), make_tuple(move(head), move(tails)...));
        }

        template<typename ...TTupArgs, size_t TIdx, typename = typename enable_if<IsLess<TIdx , tupleArgNum>::value>::type>
        auto buildTuple(tuple<TTupArgs...>&& nt, Idx<TIdx>)
        {
            auto nt_ = tuple_cat(nt, make_tuple(get<TIdx>(m_argsTuple)));
            return buildTuple(move(nt_), Idx<TIdx + 1>());
        }
        template<typename ...TTupArgs>
        auto buildTuple(tuple<TTupArgs...>&& nt, Idx<tupleArgNum>)
        {
            return move(nt);
        }

        template<typename TTup, typename TG, typename TH, typename ...TTails, size_t TIdx>
        auto catchTupleAndCall(TTup&& tup, Idx<TIdx>, TG&& t1, TH&& t2, TTails ...tails)
        {
            auto nt = tuple_cat(move(tup), make_tuple(move(t1)));
            return buildTuple(move(nt), Idx<TIdx + 1>(), forward<TH>(t2), forward<TTails>(tails)...);
        }

        template<typename TTup, typename TH, typename ...TTails, size_t TIdx>
        auto catchTupleAndCall(TTup&& tup, Idx<TIdx>, _WildCard_& wc, TH&& t2, TTails ...tails)
        {
            auto nt = tuple_cat(move(tup), make_tuple(move(t2)));
            return buildTuple(move(nt), Idx<TIdx + 1>(), forward<TTails>(tails)...);
        }

        template<typename ...TTupArgs>
        auto handleReturn(tuple<TTupArgs...>& tup, true_type)
        {
            return execute(tup, make_index_sequence<sizeof...(TTupArgs)>());
        }

        template<typename ...TTupArgs>
        auto handleReturn(tuple<TTupArgs...>& tup, false_type)
        {
            return Curried<TFunc, TTupArgs...>(m_func, move(tup));
        }
    }; 
}
