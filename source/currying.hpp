/************************************/
// auther: WangChu                  //
// date: 2016                       //
// license: MIT                     //
// github: github.com/WangChuArc    //
// e-mail: hoho71888675@126.com     //
/************************************/
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
        struct NormalFunc {};

#define storeCC(x) /* 存储一般函数的calling convention */ \
        template<typename TR, typename ...TArgs>\
        struct NormalFunc<TR x (TArgs...)> { typedef TR(type)(TArgs...); };

        storeCC(__cdecl)
        storeCC(__stdcall)
        storeCC(__fastcall)
        storeCC(__vectorcall)

        template<typename T, typename U>
        struct PtrorValue { typedef U type; };

        template<typename T, typename U>
        struct PtrorValue<T*, U> { typedef U* type; };

        template<typename T, typename U>
        struct handleMemberFunction {};

#define makeHMF(x)\
        template<typename TMarker, typename TR, typename TClass, typename ...TArgs>\
        struct handleMemberFunction<TR(x/*<--*/ TClass::*)(TArgs...), TMarker>\
        {\
            typedef typename PtrorValue<TMarker, TClass>::type CT;\
            typedef TR(x/*<--*/ *type)(CT, TArgs...);\
            template<typename = enable_if<is_pointer<CT>::value>::type>  /* CT是指针类型 */\
            static auto transMF(TR(TClass::*p)(TArgs...)) { return [p](CT cp, TArgs ...args)->TR { (x/*<--*/ cp->*p)(args); }; };\
            template<typename SFINAE = void, typename = enable_if<!is_pointer<CT>::value>::type> /* CT不是指针类型 */\
            static auto transMF(TR(TClass::*p)(TArgs...)) { return [p](CT&& cp, TArgs ...args)->TR { (x/*<--*/ cp.*p)(args); }; }\
        };                                                                   

        makeHMF(__cdecl)
        makeHMF(__fastcall)
        makeHMF(__stdcall)
        makeHMF(__vectorcall)

        template<typename T>
        struct ExtractFuncArgNum {};

        template<typename TR, typename ...TArgs>
        struct ExtractFuncArgNum<TR(TArgs...)> { enum { value = sizeof...(TArgs) }; };

        template<typename T>
        struct ExtractFuncType {};

#define makeEFT(x)\
        template<typename TR, typename ...TArgs>                    \
        struct ExtractFuncType<function<TR x/*<--*/ (TArgs...)>>    \
        {                                                           \
            typedef TR(x/*<--*/ type)(TArgs...);                    \
            typedef tuple<TArgs...> tupleType;                      \
            enum { argNum = sizeof...(TArgs) };                     \
        };

        makeEFT(__cdecl)
        makeEFT(__stdcall)
        makeEFT(__fastcall)
        makeEFT(__vectorcall)

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
    // todo:记得重新写注释
    /**********************************************************************************************************************************/

    template<typename TFunc, typename = typename enable_if<is_function<typename remove_pointer<TFunc>::type>::value>::type>
    auto Currying(TFunc func)// -> Curried<function<typename NormalFunc<typename remove_pointer<TFunc>::type>::type>>
    {
        typedef remove_pointer<TFunc>::type type;
        typedef NormalFunc<type>::type F;
        function<F> f = func;
        return Curried<function<F>>(move(f));
    }

    template<typename TFunc, typename = typename enable_if<is_member_function_pointer<TFunc>::value>::type, typename = void>
    auto Currying(TFunc func)
    {
        //auto f = _makeFuncObj(func, &func);
        typedef handleMemberFunction<TFunc, TFunc> MF;
        typedef MF::type type;
        function<type> f = MF::transMF(func);
        //return Curried<decltype(f)>(move(f));
        return Curried<function<type>>(move(f));
    }

    template<typename TFunctor, typename TFunc = decltype(&remove_pointer<TFunctor>::type::operator()), typename = void, typename = void>
    auto Currying(TFunctor func)
    {
        typedef handleMemberFunction<TFunc, TFunctor> MF;
        typedef MF::type type;
        function<type> f = MF::transMF(func);
        auto g = Curried<function<type>>(move(f));
        return g(func);
    }


    template<typename TFunc, typename ...TArgs>
    struct Curried
    {
        typedef typename ExtractFuncType<TFunc>::type functionType; /* TFunc == function<functionType> */
        typedef tuple<TArgs...> thisTupleType;
        typedef typename ExtractFuncType<TFunc>::tupleType CurriedTupleType;
        enum { funcArgNum = ExtractFuncType<TFunc>::argNum,
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
            //checkElemType(nt, make_index_sequence<tuple_size<NewTupleType>::value>(), (functionType*)(nullptr));
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
