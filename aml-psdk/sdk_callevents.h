#ifndef __AML_PSDK_CALLEVENTS_H
#define __AML_PSDK_CALLEVENTS_H

#include "sdk_base.h"
#include <type_traits>
#include <vector>

#define DECL_EVENT_BASE(_name) \
    struct CallEvent_##_name {

#define DECL_EVENT_SYM(_ret, _sym, ...) \
        typedef _ret (MyType)(__VA_ARGS__); \
        static inline MyType* m_pOriginalFunc = NULL; \
        struct { \
            std::vector<MyType*> m_list; \
            void operator+=(MyType* newFn) { \
                m_list.push_back(newFn); StartEvent(); \
            } \
            void operator-=(MyType* newFn) { \
                for(auto it = m_list.begin(); it != m_list.end(); ++it) { if(*it == newFn) { m_list.erase(it); break; } } \
            } \
        } before, after; \
        void operator+=(MyType* newFn) { after += newFn; } \
        static inline void StartEvent() { \
            if(!m_pOriginalFunc) { \
                aml->Hook(GetMainLibrarySymbol(#_sym), (void*)(&EventExecuted), (void**)(&m_pOriginalFunc)); \
            } \
        }

#define DECL_EVENT_BASE_END(_name) \
    }; static inline CallEvent_##_name _name

#define DECL_EVENT_SYM_ARG0(_ret, _name, _sym) \
    DECL_EVENT_BASE(_name) \
        DECL_EVENT_SYM(_ret, _sym) \
        inline void CallBefore() { \
            for(auto fn : before.m_list) fn(); \
        } \
        inline void CallAfter() { \
            for(auto fn : after.m_list) fn(); \
        } \
        template<typename RetType = _ret> \
        static inline RetType EventExecutedImpl() { \
            _name.CallBefore(); \
            if constexpr (std::is_same_v<RetType, void>) { \
                _name.m_pOriginalFunc(); \
                _name.CallAfter(); \
            } else { \
                RetType ret = _name.m_pOriginalFunc(); \
                _name.CallAfter(); \
                return ret; \
            } \
        } \
        static inline _ret EventExecuted() { \
            return EventExecutedImpl(); \
        } \
    DECL_EVENT_BASE_END(_name)

#define DECL_EVENT_SYM_ARG1(_ret, _name, _sym, _t1, _v1) \
    DECL_EVENT_BASE(_name) \
        DECL_EVENT_SYM(_ret, _sym, _t1 _v1) \
    DECL_EVENT_BASE_END(_name)

#endif // __AML_PSDK_CALLEVENTS_H