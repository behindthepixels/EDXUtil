#pragma once

#include "Char.h"
#include "Template.h"
#include "../Containers/Array.h"
#include "../Containers/AllocationPolicies.h"
#include <stdarg.h>

#define	USE_SECURE_CRT	1
#define	MAX_SPRINTF		1024

#define GET_VARARGS(msg, msgsize, len, lastarg, fmt) \
{ \
	va_list ap; \
	va_start(ap, lastarg); \
	CString::GetVarArgs(msg, msgsize, len, fmt, ap); \
}
#define GET_VARARGS_WIDE(msg, msgsize, len, lastarg, fmt) \
{ \
	va_list ap; \
	va_start(ap, lastarg); \
	CStringWide::GetVarArgs(msg, msgsize, len, fmt, ap); \
}
#define GET_VARARGS_ANSI(msg, msgsize, len, lastarg, fmt) \
{ \
	va_list ap; \
	va_start(ap, lastarg); \
	CStringAnsi::GetVarArgs(msg, msgsize, len, fmt, ap); \
}
#define GET_VARARGS_RESULT(msg, msgsize, len, lastarg, fmt, result) \
{ \
	va_list ap; \
	va_start(ap, lastarg); \
	result = CString::GetVarArgs(msg, msgsize, len, fmt, ap); \
	if (result >= msgsize) \
	{ \
		result = -1; \
	} \
}
#define GET_VARARGS_RESULT_WIDE(msg, msgsize, len, lastarg, fmt, result) \
{ \
	va_list ap; \
	va_start(ap, lastarg); \
	result = CStringWide::GetVarArgs(msg, msgsize, len, fmt, ap); \
	if (result >= msgsize) \
	{ \
		result = -1; \
	} \
}
#define GET_VARARGS_RESULT_ANSI(msg, msgsize, len, lastarg, fmt, result) \
{ \
	va_list ap; \
	va_start(ap, lastarg); \
	result = CStringAnsi::GetVarArgs(msg, msgsize, len, fmt, ap); \
	if (result >= msgsize) \
	{ \
		result = -1; \
	} \
}

#define VARARG_DECL( FuncRet, StaticFuncRet, Return, FuncName, Pure, FmtType, ExtraDecl, ExtraCall )	\
	FuncRet FuncName##__VA( ExtraDecl FmtType Fmt, ... ) Pure;  \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt) {Return FuncName##__VA(ExtraCall (Fmt));} \
	template<class T1> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1) {T1 v1=CheckVA(V1);Return FuncName##__VA(ExtraCall (Fmt),(v1));} \
	template<class T1,class T2> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2));} \
	template<class T1,class T2,class T3> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3));} \
	template<class T1,class T2,class T3,class T4> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4));} \
	template<class T1,class T2,class T3,class T4,class T5> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17,T18 V18) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);T18 v18=CheckVA(V18);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17),(v18));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17,T18 V18,T19 V19) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);T18 v18=CheckVA(V18);T19 v19=CheckVA(V19);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17),(v18),(v19));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17,T18 V18,T19 V19,T20 V20) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);T18 v18=CheckVA(V18);T19 v19=CheckVA(V19);T20 v20=CheckVA(V20);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17),(v18),(v19),(v20));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17,T18 V18,T19 V19,T20 V20, T21 V21) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);T18 v18=CheckVA(V18);T19 v19=CheckVA(V19);T20 v20=CheckVA(V20);T21 v21=CheckVA(V21);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17),(v18),(v19),(v20),(v21));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21,class T22> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17,T18 V18,T19 V19,T20 V20, T21 V21, T22 V22) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);T18 v18=CheckVA(V18);T19 v19=CheckVA(V19);T20 v20=CheckVA(V20);T21 v21=CheckVA(V21);T22 v22=CheckVA(V22);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17),(v18),(v19),(v20),(v21),(v22));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21,class T22,class T23> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17,T18 V18,T19 V19,T20 V20, T21 V21, T22 V22, T23 V23) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);T18 v18=CheckVA(V18);T19 v19=CheckVA(V19);T20 v20=CheckVA(V20);T21 v21=CheckVA(V21);T22 v22=CheckVA(V22);T23 v23=CheckVA(V23);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17),(v18),(v19),(v20),(v21),(v22),(v23));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21,class T22,class T23,class T24> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17,T18 V18,T19 V19,T20 V20, T21 V21, T22 V22, T23 V23, T24 V24) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);T18 v18=CheckVA(V18);T19 v19=CheckVA(V19);T20 v20=CheckVA(V20);T21 v21=CheckVA(V21);T22 v22=CheckVA(V22);T23 v23=CheckVA(V23);T24 v24=CheckVA(V24);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17),(v18),(v19),(v20),(v21),(v22),(v23),(v24));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21,class T22,class T23,class T24,class T25> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17,T18 V18,T19 V19,T20 V20, T21 V21, T22 V22, T23 V23, T24 V24, T25 V25) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);T18 v18=CheckVA(V18);T19 v19=CheckVA(V19);T20 v20=CheckVA(V20);T21 v21=CheckVA(V21);T22 v22=CheckVA(V22);T23 v23=CheckVA(V23);T24 v24=CheckVA(V24);T25 v25=CheckVA(V25);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17),(v18),(v19),(v20),(v21),(v22),(v23),(v24),(v25));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21,class T22,class T23,class T24,class T25,class T26> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17,T18 V18,T19 V19,T20 V20, T21 V21, T22 V22, T23 V23, T24 V24, T25 V25, T26 V26) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);T18 v18=CheckVA(V18);T19 v19=CheckVA(V19);T20 v20=CheckVA(V20);T21 v21=CheckVA(V21);T22 v22=CheckVA(V22);T23 v23=CheckVA(V23);T24 v24=CheckVA(V24);T25 v25=CheckVA(V25);T26 v26=CheckVA(V26);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17),(v18),(v19),(v20),(v21),(v22),(v23),(v24),(v25),(v26));}



#define VARARG_BODY( FuncRet, FuncName, FmtType, ExtraDecl )		\
	FuncRet FuncName##__VA( ExtraDecl  FmtType Fmt, ... )

#define VARARG_EXTRA(A) A,
#define VARARG_NONE

namespace EDX
{

	static inline uint32			CheckVA(uint32 dw) { return dw; }
	static inline uint8				CheckVA(uint8 b) { return b; }
	static inline int32				CheckVA(int32 i) { return i; }
	static inline uint64			CheckVA(uint64 qw) { return qw; }
	static inline int64				CheckVA(int64 sqw) { return sqw; }
	static inline double			CheckVA(double d) { return d; }
	static inline long				CheckVA(long d) { return d; }
	static inline long				CheckVA(unsigned long d) { return d; }
	static inline TCHAR				CheckVA(TCHAR c) { return c; }
	static inline void*				CheckVA(ANSICHAR* s) { return (void*)s; }
	static inline bool				CheckVA(bool b) { return b; }
	template<class T> T*			CheckVA(T* p) { return p; }
	template<class T> const T*		CheckVA(const T* p) { return p; }

	/** Helper class used to convert CString into a boolean value. */
	struct ToBoolHelper
	{
		static bool FromCStringAnsi(const ANSICHAR* String);
		static bool FromCStringWide(const WIDECHAR* String);
	};



#if !USE_SECURE_CRT
#pragma warning(push)
#pragma warning(disable : 4996) // 'function' was declared deprecated  (needed for the secure string functions)
#pragma warning(disable : 4995) // 'function' was declared deprecated  (needed for the secure string functions)
#endif

	struct CStringUtil
	{
		/**
		* Wide character implementation
		**/
		static __forceinline WIDECHAR* Strcpy(WIDECHAR* Dest, SIZE_T DestCount, const WIDECHAR* Src)
		{
#if USE_SECURE_CRT
			_tcscpy_s(Dest, DestCount, Src);
			return Dest;
#else
			return (WIDECHAR*)_tcscpy(Dest, Src);
#endif // USE_SECURE_CRT
		}

		static __forceinline WIDECHAR* Strncpy(WIDECHAR* Dest, SIZE_T MaxLen, const WIDECHAR* Src, SIZE_T Count)
		{
#if USE_SECURE_CRT
			_tcsncpy_s(Dest, MaxLen, Src, Count);
#else
			_tcsncpy(Dest, Src, MaxLen - 1);
			Dest[MaxLen - 1] = 0;
#endif // USE_SECURE_CRT
			return Dest;
		}

		static __forceinline WIDECHAR* Strcat(WIDECHAR* Dest, SIZE_T DestCount, const WIDECHAR* Src)
		{
#if USE_SECURE_CRT
			_tcscat_s(Dest, DestCount, Src);
			return Dest;
#else
			return (WIDECHAR*)_tcscat(Dest, Src);
#endif // USE_SECURE_CRT
		}

		static __forceinline WIDECHAR* Strupr(WIDECHAR* Dest, SIZE_T DestCount)
		{
#if USE_SECURE_CRT
			_tcsupr_s(Dest, DestCount);
			return Dest;
#else
			return (WIDECHAR*)_tcsupr(Dest);
#endif // USE_SECURE_CRT
		}

		static __forceinline int32 Strcmp(const WIDECHAR* String1, const WIDECHAR* String2)
		{
			return _tcscmp(String1, String2);
		}

		static __forceinline int32 Strncmp(const WIDECHAR* String1, const WIDECHAR* String2, SIZE_T Count)
		{
			return _tcsncmp(String1, String2, Count);
		}

		static __forceinline int32 Stricmp(const WIDECHAR* String1, const WIDECHAR* String2)
		{
			return _tcsicmp(String1, String2);
		}

		static __forceinline int32 Strnicmp(const WIDECHAR* String1, const WIDECHAR* String2, SIZE_T Count)
		{
			return _tcsnicmp(String1, String2, Count);
		}

		static __forceinline int32 Strlen(const WIDECHAR* String)
		{
			return _tcslen(String);
		}

		static __forceinline const WIDECHAR* Strstr(const WIDECHAR* String, const WIDECHAR* Find)
		{
			return _tcsstr(String, Find);
		}

		static __forceinline const WIDECHAR* Strchr(const WIDECHAR* String, WIDECHAR C)
		{
			return _tcschr(String, C);
		}

		static __forceinline const WIDECHAR* Strrchr(const WIDECHAR* String, WIDECHAR C)
		{
			return _tcsrchr(String, C);
		}

		static __forceinline int32 Atoi(const WIDECHAR* String)
		{
			return _tstoi(String);
		}

		static __forceinline int64 Atoi64(const WIDECHAR* String)
		{
			return _tstoi64(String);
		}

		static __forceinline float Atof(const WIDECHAR* String)
		{
			return _tstof(String);
		}

		static __forceinline double Atod(const WIDECHAR* String)
		{
			return _tcstod(String, NULL);
		}

		static __forceinline int32 Strtoi(const WIDECHAR* Start, WIDECHAR** End, int32 Base)
		{
			return _tcstoul(Start, End, Base);
		}

		static __forceinline uint64 Strtoui64(const WIDECHAR* Start, WIDECHAR** End, int32 Base)
		{
			return _tcstoui64(Start, End, Base);
		}

		static __forceinline WIDECHAR* Strtok(WIDECHAR* StrToken, const WIDECHAR* Delim, WIDECHAR** Context)
		{
			return _tcstok_s(StrToken, Delim, Context);
		}

		static __forceinline int32 GetVarArgs(WIDECHAR* Dest, SIZE_T DestSize, int32 Count, const WIDECHAR*& Fmt, va_list ArgPtr)
		{
#if USE_SECURE_CRT
			int32 Result = _vsntprintf_s(Dest, DestSize, Count, Fmt, ArgPtr);
#else
			int32 Result = _vsntprintf(Dest, Count, Fmt, ArgPtr);
#endif // USE_SECURE_CRT
			va_end(ArgPtr);
			return Result;
		}

		/**
		* Ansi implementation
		**/
		static __forceinline ANSICHAR* Strcpy(ANSICHAR* Dest, SIZE_T DestCount, const ANSICHAR* Src)
		{
#if USE_SECURE_CRT
			strcpy_s(Dest, DestCount, Src);
			return Dest;
#else
			return (ANSICHAR*)strcpy(Dest, Src);
#endif // USE_SECURE_CRT
		}

		static __forceinline void Strncpy(ANSICHAR* Dest, SIZE_T MaxLen, const ANSICHAR* Src, SIZE_T Count)
		{
#if USE_SECURE_CRT
			strncpy_s(Dest, MaxLen, Src, Count);
#else
			strncpy(Dest, Src, MaxLen);
			Dest[MaxLen - 1] = 0;
#endif // USE_SECURE_CRT
		}

		static __forceinline ANSICHAR* Strcat(ANSICHAR* Dest, SIZE_T DestCount, const ANSICHAR* Src)
		{
#if USE_SECURE_CRT
			strcat_s(Dest, DestCount, Src);
			return Dest;
#else
			return (ANSICHAR*)strcat(Dest, Src);
#endif // USE_SECURE_CRT
		}

		static __forceinline ANSICHAR* Strupr(ANSICHAR* Dest, SIZE_T DestCount)
		{
#if USE_SECURE_CRT
			_strupr_s(Dest, DestCount);
			return Dest;
#else
			return (ANSICHAR*)strupr(Dest);
#endif // USE_SECURE_CRT
		}

		static __forceinline int32 Strcmp(const ANSICHAR* String1, const ANSICHAR* String2)
		{
			return strcmp(String1, String2);
		}

		static __forceinline int32 Strncmp(const ANSICHAR* String1, const ANSICHAR* String2, SIZE_T Count)
		{
			return strncmp(String1, String2, Count);
		}

		/**
		* Compares two strings case-insensitive.
		*
		* Specialized version for ANSICHAR types.
		*
		* @param String1 First string to compare.
		* @param String2 Second string to compare.
		*
		* @returns Zero if both strings are equal. Greater than zero if first
		*          string is greater than the second one. Less than zero
		*          otherwise.
		*/
		static __forceinline int32 Stricmp(const ANSICHAR* String1, const ANSICHAR* String2)
		{
			return _stricmp(String1, String2);
		}

		static __forceinline int32 Strnicmp(const ANSICHAR* String1, const ANSICHAR* String2, SIZE_T Count)
		{
			return _strnicmp(String1, String2, Count);
		}

		static __forceinline int32 Strlen(const ANSICHAR* String)
		{
			return strlen(String);
		}

		static __forceinline const ANSICHAR* Strstr(const ANSICHAR* String, const ANSICHAR* Find)
		{
			return strstr(String, Find);
		}

		static __forceinline const ANSICHAR* Strchr(const ANSICHAR* String, ANSICHAR C)
		{
			return strchr(String, C);
		}

		static __forceinline const ANSICHAR* Strrchr(const ANSICHAR* String, ANSICHAR C)
		{
			return strrchr(String, C);
		}

		static __forceinline int32 Atoi(const ANSICHAR* String)
		{
			return atoi(String);
		}

		static __forceinline int64 Atoi64(const ANSICHAR* String)
		{
			return _strtoi64(String, NULL, 10);
		}

		static __forceinline float Atof(const ANSICHAR* String)
		{
			return (float)atof(String);
		}

		static __forceinline double Atod(const ANSICHAR* String)
		{
			return atof(String);
		}

		static __forceinline int32 Strtoi(const ANSICHAR* Start, ANSICHAR** End, int32 Base)
		{
			return strtol(Start, End, Base);
		}

		static __forceinline uint64 Strtoui64(const ANSICHAR* Start, ANSICHAR** End, int32 Base)
		{
			return _strtoui64(Start, End, Base);
		}

		static __forceinline ANSICHAR* Strtok(ANSICHAR* StrToken, const ANSICHAR* Delim, ANSICHAR** Context)
		{
			return strtok_s(StrToken, Delim, Context);
		}

		static __forceinline int32 GetVarArgs(ANSICHAR* Dest, SIZE_T DestSize, int32 Count, const ANSICHAR*& Fmt, va_list ArgPtr)
		{
#if USE_SECURE_CRT
			int32 Result = _vsnprintf_s(Dest, DestSize, Count, Fmt, ArgPtr);
#else
			int32 Result = _vsnprintf(Dest, Count, Fmt, ArgPtr);
#endif // USE_SECURE_CRT
			va_end(ArgPtr);
			return Result;
		}

		static const ANSICHAR* GetEncodingName()
		{
			return "UTF-16LE";
		}

		static const bool IsUnicodeEncoded = true;


		/**
		* Tests whether a particular character is a valid member of its encoding.
		*
		* @param Ch The character to test.
		* @return True if the character is a valid member of Encoding.
		*/
		template <typename Encoding>
		static bool IsValidChar(Encoding Ch)
		{
			return true;
		}


		/**
		* Tests whether a particular character can be converted to the destination encoding.
		*
		* @param Ch The character to test.
		* @return True if Ch can be encoded as a DestEncoding.
		*/
		template <typename DestEncoding, typename SourceEncoding>
		static bool CanConvertChar(SourceEncoding Ch)
		{
			return IsValidChar(Ch) && (SourceEncoding)(DestEncoding)Ch == Ch && IsValidChar((DestEncoding)Ch);
		}


		/**
		* Returns the string representing the name of the given encoding type.
		*
		* @return The name of the CharType as a TCHAR string.
		*/
		template <typename Encoding>
		static const TCHAR* GetEncodingTypeName();


		/**
		* Metafunction which tests whether a given character type represents a fixed-width encoding.
		*
		* We need to 'forward' the metafunction to a helper because of the C++ requirement that
		* nested structs cannot be fully specialized.  They can be partially specialized however, hence the
		* helper function.
		*/
		template <bool Dummy, typename T>
		struct IsFixedWidthEncoding_Helper
		{
			enum { Value = false };
		};

		template <bool Dummy> struct IsFixedWidthEncoding_Helper<Dummy, ANSICHAR> { enum { Value = true }; };
		template <bool Dummy> struct IsFixedWidthEncoding_Helper<Dummy, WIDECHAR> { enum { Value = true }; };

		template <typename T>
		struct IsFixedWidthEncoding : IsFixedWidthEncoding_Helper<false, T>
		{
		};


		/**
		* Metafunction which tests whether two encodings are compatible.
		*
		* We'll say the encodings are compatible if they're both fixed-width and have the same size.  This
		* should be good enough and catches things like UCS2CHAR and WIDECHAR being equivalent.
		* Specializations of this template can be provided for any other special cases.
		* Same size is a minimum requirement.
		*/
		template <typename EncodingA, typename EncodingB>
		struct AreEncodingsCompatible
		{
			enum { Value = IsFixedWidthEncoding<EncodingA>::Value && IsFixedWidthEncoding<EncodingB>::Value && sizeof(EncodingA) == sizeof(EncodingB) };
		};

		/**
		* Converts the [Src, Src+SrcSize) string range from SourceChar to DestChar and writes it to the [Dest, Dest+DestSize) range.
		* The Src range should contain a null terminator if a null terminator is required in the output.
		* If the Dest range is not big enough to hold the converted output, NULL is returned.  In this case, nothing should be assumed about the contents of Dest.
		*
		* @param Dest      The start of the destination buffer.
		* @param DestSize  The size of the destination buffer.
		* @param Src       The start of the string to convert.
		* @param SrcSize   The number of Src elements to convert.
		* @param BogusChar The char to use when the conversion process encounters a character it cannot convert.
		* @return          A pointer to one past the last-written element.
		*/
		template <typename SourceEncoding, typename DestEncoding>
		static __forceinline typename EnableIf<
			// This overload should be called when SourceEncoding and DestEncoding are 'compatible', i.e. they're the same type or equivalent (e.g. like UCS2CHAR and WIDECHAR are on Windows).
			AreEncodingsCompatible<SourceEncoding, DestEncoding>::Value,
			DestEncoding*
		>::Type Convert(DestEncoding* Dest, int32 DestSize, const SourceEncoding* Src, int32 SrcSize, DestEncoding BogusChar = (DestEncoding)'?')
		{
			if (DestSize < SrcSize)
				return NULL;

			return (DestEncoding*)Memory::Memcpy(Dest, Src, SrcSize * sizeof(SourceEncoding)) + SrcSize;
		}

		/**
		* Converts the [Src, Src+SrcSize) string range from SourceEncoding to DestEncoding and writes it to the [Dest, Dest+DestSize) range.
		* The Src range should contain a null terminator if a null terminator is required in the output.
		* If the Dest range is not big enough to hold the converted output, NULL is returned.  In this case, nothing should be assumed about the contents of Dest.
		*
		* @param Dest      The start of the destination buffer.
		* @param DestSize  The size of the destination buffer.
		* @param Src       The start of the string to convert.
		* @param SrcSize   The number of Src elements to convert.
		* @param BogusChar The char to use when the conversion process encounters a character it cannot convert.
		* @return          A pointer to one past the last-written element.
		*/
		template <typename SourceEncoding, typename DestEncoding>
		static typename EnableIf<
			// This overload should be called when the types are not compatible but the source is fixed-width, e.g. ANSICHAR->WIDECHAR.
			!AreEncodingsCompatible<SourceEncoding, DestEncoding>::Value && IsFixedWidthEncoding<SourceEncoding>::Value,
			DestEncoding*
		>::Type Convert(DestEncoding* Dest, int32 DestSize, const SourceEncoding* Src, int32 SrcSize, DestEncoding BogusChar = (DestEncoding)'?')
		{
			const SourceEncoding* InSrc = Src;
			int32                 InSrcSize = SrcSize;
			bool                  bInvalidChars = false;
			while (SrcSize)
			{
				if (!DestSize)
					return NULL;

				SourceEncoding SrcCh = *Src++;
				if (CanConvertChar<DestEncoding>(SrcCh))
				{
					*Dest++ = (DestEncoding)SrcCh;
				}
				else
				{
					*Dest++ = BogusChar;
					bInvalidChars = true;
				}
				--SrcSize;
				--DestSize;
			}

			if (bInvalidChars)
			{
				//LogBogusChars<DestEncoding>(InSrc, InSrcSize);
			}

			return Dest;
		}

		/**
		* Returns the required buffer length for the [Src, Src+SrcSize) string when converted to the DestChar encoding.
		* The Src range should contain a null terminator if a null terminator is required in the output.
		*
		* @param  Src     The start of the string to convert.
		* @param  SrcSize The number of Src elements to convert.
		* @return         The number of DestChar elements that Src will be converted into.
		*/
		template <typename DestEncoding, typename SourceEncoding>
		static typename EnableIf<IsFixedWidthEncoding<SourceEncoding>::Value && IsFixedWidthEncoding<DestEncoding>::Value, int32>::Type ConvertedLength(const SourceEncoding* Src, int32 SrcSize)
		{
			return SrcSize;
		}
	};

#if !USE_SECURE_CRT
#pragma warning(pop) // 'function' was was declared deprecated  (needed for the secure string functions)
#endif

	/**
	*	Set of basic string utility functions operating on plain C strings. In addition to the
	*	wrapped C string API,this struct also contains a set of widely used utility functions that
	*  operate on c strings.
	*	There is a specialized implementation for ANSICHAR and WIDECHAR strings provided. To access these
	*	functionality, the convenience typedefs CString and CStringAnsi are provided.
	**/
	template <typename T>
	struct TCString
	{
		typedef T CharType;

		/**
		* Returns whether this string contains only pure ansi characters
		* @param Str - string that will be checked
		**/
		static __forceinline bool IsPureAnsi(const CharType* Str);

		/**
		* Returns whether this string contains only numeric characters
		* @param Str - string that will be checked
		**/
		static bool IsNumeric(const CharType* Str)
		{
			if (*Str == '-' || *Str == '+')
			{
				Str++;
			}

			bool bHasDot = false;
			while (*Str != '\0')
			{
				if (*Str == '.')
				{
					if (bHasDot)
					{
						return false;
					}
					bHasDot = true;
				}
				else if (!FChar::IsDigit(*Str))
				{
					return false;
				}

				++Str;
			}

			return true;
		}

		/**
		* strcpy wrapper
		*
		* @param Dest - destination string to copy to
		* @param Destcount - size of Dest in characters
		* @param Src - source string
		* @return destination string
		*/
		static __forceinline CharType* Strcpy(CharType* Dest, SIZE_T DestCount, const CharType* Src);

		/**
		* Copy a string with length checking. Behavior differs from strncpy in that last character is zeroed.
		*
		* @param Dest - destination buffer to copy to
		* @param Src - source buffer to copy from
		* @param MaxLen - max length of the buffer (including null-terminator)
		* @return pointer to resulting string buffer
		*/
		static __forceinline CharType* Strncpy(CharType* Dest, SIZE_T MaxLen, const CharType* Src, SIZE_T Count);

		/**
		* strcpy wrapper
		* (templated version to automatically handle static destination array case)
		*
		* @param Dest - destination string to copy to
		* @param Src - source string
		* @return destination string
		*/
		template<SIZE_T DestCount>
		static __forceinline CharType* Strcpy(CharType(&Dest)[DestCount], const CharType* Src)
		{
			return Strcpy(Dest, DestCount, Src);
		}

		/**
		* strcat wrapper
		*
		* @param Dest - destination string to copy to
		* @param Destcount - size of Dest in characters
		* @param Src - source string
		* @return destination string
		*/
		static __forceinline CharType* Strcat(CharType* Dest, SIZE_T DestCount, const CharType* Src);

		/**
		* strcat wrapper
		* (templated version to automatically handle static destination array case)
		*
		* @param Dest - destination string to copy to
		* @param Src - source string
		* @return destination string
		*/
		template<SIZE_T DestCount>
		static __forceinline CharType* Strcat(CharType(&Dest)[DestCount], const CharType* Src)
		{
			return Strcat(Dest, DestCount, Src);
		}

		/**
		* Concatenate a string with length checking.
		*
		* @param Dest - destination buffer to append to
		* @param Src - source buffer to copy from
		* @param MaxLen - max length of the buffer
		* @return pointer to resulting string buffer
		*/
		static inline CharType* Strncat(CharType* Dest, const CharType* Src, int32 MaxLen)
		{
			int32 Len = Strlen(Dest);
			CharType* NewDest = Dest + Len;
			if ((MaxLen -= Len) > 0)
			{
				Strncpy(NewDest, MaxLen, Src, MaxLen - 1);
			}
			return Dest;
		}

		/**
		* strupr wrapper
		*
		* @param Dest - destination string to convert
		* @param Destcount - size of Dest in characters
		* @return destination string
		*/
		static __forceinline CharType* Strupr(CharType* Dest, SIZE_T DestCount);


		/**
		* strupr wrapper
		* (templated version to automatically handle static destination array case)
		*
		* @param Dest - destination string to convert
		* @return destination string
		*/
		template<SIZE_T DestCount>
		static __forceinline CharType* Strupr(CharType(&Dest)[DestCount])
		{
			return Strupr(Dest, DestCount);
		}

		/**
		* strcmp wrapper
		**/
		static __forceinline int32 Strcmp(const CharType* String1, const CharType* String2);

		/**
		* strncmp wrapper
		*/
		static __forceinline int32 Strncmp(const CharType* String1, const CharType* String2, SIZE_T Count);

		/**
		* stricmp wrapper
		*/
		static __forceinline int32 Stricmp(const CharType* String1, const CharType* String2);

		/**
		* strnicmp wrapper
		*/
		static __forceinline int32 Strnicmp(const CharType* String1, const CharType* String2, SIZE_T Count);

		/**
		* Returns a static string that is filled with a variable number of spaces
		*
		* @param NumSpaces Number of spaces to put into the string, max of 255
		*
		* @return The string of NumSpaces spaces.
		*/
		static const CharType* Spc(int32 NumSpaces);

		/**
		* Returns a static string that is filled with a variable number of tabs
		*
		* @param NumTabs Number of tabs to put into the string, max of 255
		*
		* @return The string of NumTabs tabs.
		*/
		static const CharType* Tab(int32 NumTabs);

		/**
		* Find string in string, case insensitive, requires non-alphanumeric lead-in.
		*/
		static const CharType* Strfind(const CharType* Str, const CharType* Find);

		/**
		* Finds string in string, case insensitive, requires the string be surrounded by one the specified
		* delimiters, or the start or end of the string.
		*/
		static const CharType* StrfindDelim(const CharType* Str, const CharType* Find, const CharType* Delim = LITERAL(CharType, " \t,"));

		/**
		* Finds string in string, case insensitive
		* @param Str The string to look through
		* @param Find The string to find inside Str
		* @return Position in Str if Find was found, otherwise, NULL
		*/
		static const CharType* Stristr(const CharType* Str, const CharType* Find);

		/**
		* Finds string in string, case insensitive (non-const version)
		* @param Str The string to look through
		* @param Find The string to find inside Str
		* @return Position in Str if Find was found, otherwise, NULL
		*/
		static CharType* Stristr(CharType* Str, const CharType* Find) { return (CharType*)Stristr((const CharType*)Str, Find); }

		/**
		* strlen wrapper
		*/
		static __forceinline int32 Strlen(const CharType* String);

		/**
		* strstr wrapper
		*/
		static __forceinline const CharType* Strstr(const CharType* String, const CharType* Find);
		static __forceinline CharType* Strstr(CharType* String, const CharType* Find);

		/**
		* strchr wrapper
		*/
		static __forceinline const CharType* Strchr(const CharType* String, CharType c);
		static __forceinline CharType* Strchr(CharType* String, CharType c);

		/**
		* strrchr wrapper
		*/
		static __forceinline const CharType* Strrchr(const CharType* String, CharType c);
		static __forceinline CharType* Strrchr(CharType* String, CharType c);

		/**
		* strrstr wrapper
		*/
		static __forceinline const CharType* Strrstr(const CharType* String, const CharType* Find);
		static __forceinline CharType* Strrstr(CharType* String, const CharType* Find);

		/**
		* strspn wrapper
		*/
		static __forceinline int32 Strspn(const CharType* String, const CharType* Mask);

		/**
		* strcspn wrapper
		*/
		static __forceinline int32 Strcspn(const CharType* String, const CharType* Mask);

		/**
		* atoi wrapper
		*/
		static __forceinline int32 Atoi(const CharType* String);

		/**
		* atoi64 wrapper
		*/
		static __forceinline int64 Atoi64(const CharType* String);

		/**
		* atof wrapper
		*/
		static __forceinline float Atof(const CharType* String);

		/**
		* atod wrapper
		*/
		static __forceinline double Atod(const CharType* String);

		/**
		* Converts a string into a boolean value
		*   1, "True", "Yes", GTrue, GYes, and non-zero integers become true
		*   0, "False", "No", GFalse, GNo, and unparsable values become false
		*
		* @return The boolean value
		*/
		static __forceinline bool ToBool(const CharType* String);

		/**
		* strtoi wrapper
		*/
		static __forceinline int32 Strtoi(const CharType* Start, CharType** End, int32 Base);

		/**
		* strtoui wrapper
		*/
		static __forceinline uint64 Strtoui64(const CharType* Start, CharType** End, int32 Base);

		/**
		* strtok wrapper
		*/
		static __forceinline CharType* Strtok(CharType* TokenString, const CharType* Delim, CharType** Context);

		/**
		* Standard string formatted print.
		* @warning: make sure code using CString::Sprintf allocates enough (>= MAX_SPRINTF) memory for the destination buffer
		*/
		VARARG_DECL(static inline int32, static inline int32, return, Sprintf, VARARG_NONE, const CharType*, VARARG_EXTRA(CharType* Dest), VARARG_EXTRA(Dest));

		/**
		* Safe string formatted print.
		*/
		VARARG_DECL(static inline int32, static inline int32, return, Snprintf, VARARG_NONE, const CharType*, VARARG_EXTRA(CharType* Dest) VARARG_EXTRA(int32 DestSize), VARARG_EXTRA(Dest) VARARG_EXTRA(DestSize));

		/**
		* Helper function to write formatted output using an argument list
		*
		* @param Dest - destination string buffer
		* @param DestSize - size of destination buffer
		* @param Count - number of characters to write (not including null terminating character)
		* @param Fmt - string to print
		* @param Args - argument list
		* @return number of characters written or -1 if truncated
		*/
		static __forceinline int32 GetVarArgs(CharType* Dest, SIZE_T DestSize, int32 Count, const CharType*& Fmt, va_list ArgPtr);
	};

	typedef TCString<TCHAR>    CString;
	typedef TCString<ANSICHAR> CStringAnsi;
	typedef TCString<WIDECHAR> CStringWide;

	/*-----------------------------------------------------------------------------
	generic TCString implementations
	-----------------------------------------------------------------------------*/

	template <typename CharType>
	struct CStringSpcHelper
	{
		/** Number of characters to be stored in string. */
		static const int32 MAX_SPACES = 255;

		/** Number of tabs to be stored in string. */
		static const int32 MAX_TABS = 255;

		static const CharType SpcArray[MAX_SPACES + 1];
		static const CharType TabArray[MAX_TABS + 1];
	};

	template <typename T>
	const typename TCString<T>::CharType* TCString<T>::Spc(int32 NumSpaces)
	{
		Assert(NumSpaces >= 0 && NumSpaces <= CStringSpcHelper<T>::MAX_SPACES);
		return CStringSpcHelper<T>::SpcArray + CStringSpcHelper<T>::MAX_SPACES - NumSpaces;
	}

	template <typename T>
	const typename TCString<T>::CharType* TCString<T>::Tab(int32 NumTabs)
	{
		Assert(NumTabs >= 0 && NumTabs <= CStringSpcHelper<T>::MAX_TABS);
		return CStringSpcHelper<T>::TabArray + CStringSpcHelper<T>::MAX_TABS - NumTabs;
	}

	//
	// Find string in string, case insensitive, requires non-alphanumeric lead-in.
	//
	template <typename T>
	const typename TCString<T>::CharType* TCString<T>::Strfind(const CharType* Str, const CharType* Find)
	{
		if (Find == NULL || Str == NULL)
		{
			return NULL;
		}

		bool Alnum = 0;
		CharType f = (*Find < LITERAL(CharType, 'a') || *Find > LITERAL(CharType, 'z')) ? (*Find) : (*Find + LITERAL(CharType, 'A') - LITERAL(CharType, 'a'));
		int32 Length = Strlen(Find++) - 1;
		CharType c = *Str++;
		while (c)
		{
			if (c >= LITERAL(CharType, 'a') && c <= LITERAL(CharType, 'z'))
			{
				c += LITERAL(CharType, 'A') - LITERAL(CharType, 'a');
			}
			if (!Alnum && c == f && !Strnicmp(Str, Find, Length))
			{
				return Str - 1;
			}
			Alnum = (c >= LITERAL(CharType, 'A') && c <= LITERAL(CharType, 'Z')) ||
				(c >= LITERAL(CharType, '0') && c <= LITERAL(CharType, '9'));
			c = *Str++;
		}
		return NULL;
	}

	//
	// Finds string in string, case insensitive, requires the string be surrounded by one the specified
	// delimiters, or the start or end of the string.
	//
	template <typename T>
	const typename TCString<T>::CharType* TCString<T>::StrfindDelim(const CharType* Str, const CharType* Find, const CharType* Delim)
	{
		if (Find == NULL || Str == NULL)
		{
			return NULL;
		}

		int32 Length = Strlen(Find);
		const T* Found = Stristr(Str, Find);
		if (Found)
		{
			// check if this occurrence is delimited correctly
			if ((Found == Str || Strchr(Delim, Found[-1]) != NULL) &&								// either first char, or following a delim
				(Found[Length] == LITERAL(CharType, '\0') || Strchr(Delim, Found[Length]) != NULL))	// either last or with a delim following
			{
				return Found;
			}

			// start searching again after the first matched character
			for (;;)
			{
				Str = Found + 1;
				Found = Stristr(Str, Find);

				if (Found == NULL)
				{
					return NULL;
				}

				// check if the next occurrence is delimited correctly
				if ((Strchr(Delim, Found[-1]) != NULL) &&												// match is following a delim
					(Found[Length] == LITERAL(CharType, '\0') || Strchr(Delim, Found[Length]) != NULL))	// either last or with a delim following
				{
					return Found;
				}
			}
		}

		return NULL;
	}

	/**
	* Finds string in string, case insensitive
	* @param Str The string to look through
	* @param Find The string to find inside Str
	* @return Position in Str if Find was found, otherwise, NULL
	*/
	template <typename T>
	const typename TCString<T>::CharType* TCString<T>::Stristr(const CharType* Str, const CharType* Find)
	{
		// both strings must be valid
		if (Find == NULL || Str == NULL)
		{
			return NULL;
		}

		// get upper-case first letter of the find string (to reduce the number of full strnicmps)
		CharType FindInitial = TChar<CharType>::ToUpper(*Find);
		// get length of find string, and increment past first letter
		int32   Length = Strlen(Find++) - 1;
		// get the first letter of the search string, and increment past it
		CharType StrChar = *Str++;
		// while we aren't at end of string...
		while (StrChar)
		{
			// make sure it's upper-case
			StrChar = TChar<CharType>::ToUpper(StrChar);
			// if it matches the first letter of the find string, do a case-insensitive string compare for the length of the find string
			if (StrChar == FindInitial && !Strnicmp(Str, Find, Length))
			{
				// if we found the string, then return a pointer to the beginning of it in the search string
				return Str - 1;
			}
			// go to next letter
			StrChar = *Str++;
		}

		// if nothing was found, return NULL
		return NULL;
	}

	template <typename T> __forceinline
	typename TCString<T>::CharType* TCString<T>::Strcpy(CharType* Dest, SIZE_T DestCount, const CharType* Src)
	{
		return CStringUtil::Strcpy(Dest, DestCount, Src);
	}

	template <typename T> __forceinline
	typename TCString<T>::CharType* TCString<T>::Strncpy(CharType* Dest, SIZE_T MaxLen, const CharType* Src, SIZE_T Count)
	{
		Assert(MaxLen > 0);
		CStringUtil::Strncpy(Dest, MaxLen, Src, Count);
		return Dest;
	}

	template <typename T> __forceinline
	typename TCString<T>::CharType* TCString<T>::Strcat(CharType* Dest, SIZE_T DestCount, const CharType* Src)
	{
		return CStringUtil::Strcat(Dest, DestCount, Src);
	}

	template <typename T> __forceinline
	typename TCString<T>::CharType* TCString<T>::Strupr(CharType* Dest, SIZE_T DestCount)
	{
		return CStringUtil::Strupr(Dest, DestCount);
	}

	template <typename T> __forceinline
	int32 TCString<T>::Strcmp(const CharType* String1, const CharType* String2)
	{
		return CStringUtil::Strcmp(String1, String2);
	}

	template <typename T> __forceinline
	int32 TCString<T>::Strncmp(const CharType* String1, const CharType* String2, SIZE_T Count)
	{
		return CStringUtil::Strncmp(String1, String2, Count);
	}

	template <typename T> __forceinline
	int32 TCString<T>::Stricmp(const CharType* String1, const CharType* String2)
	{
		return CStringUtil::Stricmp(String1, String2);
	}

	template <typename T> __forceinline
	int32 TCString<T>::Strnicmp(const CharType* String1, const CharType* String2, SIZE_T Count)
	{
		return CStringUtil::Strnicmp(String1, String2, Count);
	}

	template <typename T> __forceinline
	int32 TCString<T>::Strlen(const CharType* String)
	{
		return CStringUtil::Strlen(String);
	}

	template <typename T> __forceinline
	const typename TCString<T>::CharType* TCString<T>::Strstr(const CharType* String, const CharType* Find)
	{
		return CStringUtil::Strstr(String, Find);
	}

	template <typename T> __forceinline
	typename TCString<T>::CharType* TCString<T>::Strstr(CharType* String, const CharType* Find)
	{
		return (CharType*)CStringUtil::Strstr(String, Find);
	}

	template <typename T> __forceinline
	const typename TCString<T>::CharType* TCString<T>::Strchr(const CharType* String, CharType c)
	{
		return CStringUtil::Strchr(String, c);
	}

	template <typename T> __forceinline
	typename TCString<T>::CharType* TCString<T>::Strchr(CharType* String, CharType c)
	{
		return (CharType*)CStringUtil::Strchr(String, c);
	}

	template <typename T> __forceinline
	const typename TCString<T>::CharType* TCString<T>::Strrchr(const CharType* String, CharType c)
	{
		return CStringUtil::Strrchr(String, c);
	}

	template <typename T> __forceinline
	typename TCString<T>::CharType* TCString<T>::Strrchr(CharType* String, CharType c)
	{
		return (CharType*)CStringUtil::Strrchr(String, c);
	}

	template <typename T> __forceinline
	const typename TCString<T>::CharType* TCString<T>::Strrstr(const CharType* String, const CharType* Find)
	{
		return Strrstr((CharType*)String, Find);
	}

	template <typename T> __forceinline
	typename TCString<T>::CharType* TCString<T>::Strrstr(CharType* String, const CharType* Find)
	{
		if (*Find == (CharType)0)
		{
			return String + Strlen(String);
		}

		CharType* Result = nullptr;
		for (;;)
		{
			CharType* Found = Strstr(String, Find);
			if (!Found)
			{
				return Result;
			}

			Result = Found;
			String = Found + 1;
		}
	}

	template <typename T> __forceinline
	int32 TCString<T>::Strspn(const CharType* String, const CharType* Mask)
	{
		const TCHAR* StringIt = String;
		while (*StringIt)
		{
			for (const TCHAR* MaskIt = Mask; *MaskIt; ++MaskIt)
			{
				if (*StringIt != *MaskIt)
				{
					return StringIt - String;
				}
			}

			++StringIt;
		}

		return StringIt - String;
	}

	template <typename T> __forceinline
	int32 TCString<T>::Strcspn(const CharType* String, const CharType* Mask)
	{
		const TCHAR* StringIt = String;
		while (*StringIt)
		{
			for (const TCHAR* MaskIt = Mask; *MaskIt; ++MaskIt)
			{
				if (*StringIt == *MaskIt)
				{
					return StringIt - String;
				}
			}

			++StringIt;
		}

		return StringIt - String;
	}

	template <typename T> __forceinline
	int32 TCString<T>::Atoi(const CharType* String)
	{
		return CStringUtil::Atoi(String);
	}

	template <typename T> __forceinline
	int64 TCString<T>::Atoi64(const CharType* String)
	{
		return CStringUtil::Atoi64(String);
	}

	template <typename T> __forceinline
	float TCString<T>::Atof(const CharType* String)
	{
		return CStringUtil::Atof(String);
	}

	template <typename T> __forceinline
	double TCString<T>::Atod(const CharType* String)
	{
		return CStringUtil::Atod(String);
	}

	template <typename T> __forceinline
	int32 TCString<T>::Strtoi(const CharType* Start, CharType** End, int32 Base)
	{
		return CStringUtil::Strtoi(Start, End, Base);
	}

	template <typename T> __forceinline
	uint64 TCString<T>::Strtoui64(const CharType* Start, CharType** End, int32 Base)
	{
		return CStringUtil::Strtoui64(Start, End, Base);
	}


	template <typename T> __forceinline
	typename TCString<T>::CharType* TCString<T>::Strtok(CharType* TokenString, const CharType* Delim, CharType** Context)
	{
		return CStringUtil::Strtok(TokenString, Delim, Context);
	}

	template <typename T> __forceinline
	int32 TCString<T>::GetVarArgs(CharType* Dest, SIZE_T DestSize, int32 Count, const CharType*& Fmt, va_list ArgPtr)
	{
		return CStringUtil::GetVarArgs(Dest, DestSize, Count, Fmt, ArgPtr);
	}


	/*-----------------------------------------------------------------------------
	TCString<WIDECHAR> specializations
	-----------------------------------------------------------------------------*/
	template <> __forceinline
	bool TCString<WIDECHAR>::IsPureAnsi(const WIDECHAR* Str)
	{
		for (; *Str; Str++)
		{
			if (*Str > 0x7f)
			{
				return 0;
			}
		}
		return 1;
	}


	template <> inline
	VARARG_BODY(int32, TCString<WIDECHAR>::Sprintf, const CharType*, VARARG_EXTRA(CharType* Dest))
	{
		int32	Result = -1;
		GET_VARARGS_RESULT_WIDE(Dest, MAX_SPRINTF, MAX_SPRINTF - 1, Fmt, Fmt, Result);
		return Result;
	}

	template <> inline
	VARARG_BODY(int32, TCString<WIDECHAR>::Snprintf, const CharType*, VARARG_EXTRA(CharType* Dest) VARARG_EXTRA(int32 DestSize))
	{
		int32	Result = -1;
		GET_VARARGS_RESULT_WIDE(Dest, DestSize, DestSize - 1, Fmt, Fmt, Result);
		return Result;
	}

	template <>
	__forceinline bool TCString<WIDECHAR>::ToBool(const WIDECHAR* Str)
	{
		return ToBoolHelper::FromCStringWide(Str);
	}

	/*-----------------------------------------------------------------------------
	TCString<ANSICHAR> specializations
	-----------------------------------------------------------------------------*/
	template <> __forceinline bool TCString<ANSICHAR>::IsPureAnsi(const CharType* Str)
	{
		return true;
	}

	template <> inline
	VARARG_BODY(int32, TCString<ANSICHAR>::Sprintf, const CharType*, VARARG_EXTRA(CharType* Dest))
	{
		int32	Result = -1;
		GET_VARARGS_RESULT_ANSI(Dest, MAX_SPRINTF, MAX_SPRINTF - 1, Fmt, Fmt, Result);
		return Result;
	}

	template <> inline
	VARARG_BODY(int32, TCString<ANSICHAR>::Snprintf, const CharType*, VARARG_EXTRA(CharType* Dest) VARARG_EXTRA(int32 DestSize))
	{
		int32	Result = -1;
		GET_VARARGS_RESULT_ANSI(Dest, DestSize, DestSize - 1, Fmt, Fmt, Result);
		return Result;
	}

	template <>
	__forceinline bool TCString<ANSICHAR>::ToBool(const ANSICHAR* Str)
	{
		return ToBoolHelper::FromCStringAnsi(Str);
	}


#define DEFAULT_STRING_CONVERSION_SIZE 128u
#define UNICODE_BOGUS_CHAR_CODEPOINT   '?'


	template <typename From, typename To>
	class StringConvert
	{
	public:
		typedef From FromType;
		typedef To   ToType;

		__forceinline static void Convert(To* Dest, int32 DestLen, const From* Source, int32 SourceLen)
		{
			To* Result = CStringUtil::Convert(Dest, DestLen, Source, SourceLen, (To)UNICODE_BOGUS_CHAR_CODEPOINT);
			Assert(Result);
		}

		static int32 ConvertedLength(const From* Source, int32 SourceLen)
		{
			return CStringUtil::ConvertedLength<To>(Source, SourceLen);
		}
	};

	/**
	* This is a basic ANSICHAR* wrapper which swallows all output written through it.
	*/
	struct NulPointerIterator
	{
		NulPointerIterator()
			: Ptr_(NULL)
		{
		}

		const NulPointerIterator& operator* () const { return *this; }
		const NulPointerIterator& operator++() { ++Ptr_; return *this; }
		const NulPointerIterator& operator++(int) { ++Ptr_; return *this; }

		ANSICHAR operator=(ANSICHAR Val) const
		{
			return Val;
		}

		friend int32 operator-(NulPointerIterator Lhs, NulPointerIterator Rhs)
		{
			return Lhs.Ptr_ - Rhs.Ptr_;
		}

		ANSICHAR* Ptr_;
	};

	// This should be replaced with Platform stuff when FPlatformString starts to know about UTF-8.
	class TCHARToUTF8_Convert
	{
	public:
		typedef TCHAR    FromType;
		typedef ANSICHAR ToType;

		// I wrote this function for originally for PhysicsFS. --ryan.
		// !!! FIXME: Maybe this shouldn't be inline...
		template <typename OutputIterator>
		static void utf8fromcodepoint(uint32 cp, OutputIterator* _dst, int32 *_len)
		{
			OutputIterator dst = *_dst;
			int32          len = *_len;

			if (len == 0)
				return;

			if (cp > 0x10FFFF)   // No Unicode codepoints above 10FFFFh, (for now!)
			{
				cp = UNICODE_BOGUS_CHAR_CODEPOINT;
			}
			else if ((cp == 0xFFFE) || (cp == 0xFFFF))  // illegal values.
			{
				cp = UNICODE_BOGUS_CHAR_CODEPOINT;
			}
			else
			{
				// There are seven "UTF-16 surrogates" that are illegal in UTF-8.
				switch (cp)
				{
				case 0xD800:
				case 0xDB7F:
				case 0xDB80:
				case 0xDBFF:
				case 0xDC00:
				case 0xDF80:
				case 0xDFFF:
					cp = UNICODE_BOGUS_CHAR_CODEPOINT;
				}
			}

			// Do the encoding...
			if (cp < 0x80)
			{
				*(dst++) = (char)cp;
				len--;
			}

			else if (cp < 0x800)
			{
				if (len < 2)
				{
					len = 0;
				}
				else
				{
					*(dst++) = (char)((cp >> 6) | 128 | 64);
					*(dst++) = (char)(cp & 0x3F) | 128;
					len -= 2;
				}
			}

			else if (cp < 0x10000)
			{
				if (len < 3)
				{
					len = 0;
				}
				else
				{
					*(dst++) = (char)((cp >> 12) | 128 | 64 | 32);
					*(dst++) = (char)((cp >> 6) & 0x3F) | 128;
					*(dst++) = (char)(cp & 0x3F) | 128;
					len -= 3;
				}
			}

			else
			{
				if (len < 4)
				{
					len = 0;
				}
				else
				{
					*(dst++) = (char)((cp >> 18) | 128 | 64 | 32 | 16);
					*(dst++) = (char)((cp >> 12) & 0x3F) | 128;
					*(dst++) = (char)((cp >> 6) & 0x3F) | 128;
					*(dst++) = (char)(cp & 0x3F) | 128;
					len -= 4;
				}
			}

			*_dst = dst;
			*_len = len;
		}

		/**
		* Converts the string to the desired format.
		*
		* @param Dest      The destination buffer of the converted string.
		* @param DestLen   The length of the destination buffer.
		* @param Source    The source string to convert.
		* @param SourceLen The length of the source string.
		*/
		static __forceinline void Convert(ANSICHAR* Dest, int32 DestLen, const TCHAR* Source, int32 SourceLen)
		{
			// Now do the conversion
			// You have to do this even if !UNICODE, since high-ASCII chars
			//  become multibyte. If you aren't using UNICODE and aren't using
			//  a Latin1 charset, you are just screwed, since we don't handle
			//  codepages, etc.
			while (SourceLen--)
			{
				utf8fromcodepoint((uint32)*Source++, &Dest, &DestLen);
			}
		}

		/**
		* Determines the length of the converted string.
		*
		* @return The length of the string in UTF-8 code units.
		*/
		static int32 ConvertedLength(const TCHAR* Source, int32 SourceLen)
		{
			NulPointerIterator DestStart;
			NulPointerIterator Dest;
			int32               DestLen = SourceLen * 4;
			while (SourceLen--)
			{
				utf8fromcodepoint((uint32)*Source++, &Dest, &DestLen);
			}
			return Dest - DestStart;
		}
	};

	struct ENullTerminatedString
	{
		enum Type
		{
			No = 0,
			Yes = 1
		};
	};

	/**
	* Class takes one type of string and converts it to another. The class includes a
	* chunk of presized memory of the destination type. If the presized array is
	* too small, it mallocs the memory needed and frees on the class going out of
	* scope.
	*/
	template<typename Converter, int32 DefaultConversionSize = DEFAULT_STRING_CONVERSION_SIZE>
	class StringConversion : private Converter, private InlineAllocator<DefaultConversionSize>::template ForElementType<typename Converter::ToType>
	{
	private:
		typedef typename InlineAllocator<DefaultConversionSize>::template ForElementType<typename Converter::ToType> AllocatorType;

		typedef typename Converter::FromType FromType;
		typedef typename Converter::ToType   ToType;

		ToType* Ptr;
		int32   StringLength;

		// Non-copyable
		StringConversion(const StringConversion&) = delete;
		StringConversion& operator=(const StringConversion&) = delete;

		/**
		* Converts the data by using the Convert() method on the base class
		*/
		void Init(const FromType* Source, int32 SourceLen, ENullTerminatedString::Type NullTerminated)
		{
			StringLength = Converter::ConvertedLength(Source, SourceLen);

			int32 BufferSize = StringLength + NullTerminated;

			AllocatorType::ResizeAllocation(0, BufferSize, sizeof(ToType));

			Ptr = (ToType*)AllocatorType::GetAllocation();
			Converter::Convert(Ptr, BufferSize, Source, SourceLen + NullTerminated);
		}

	public:
		explicit StringConversion(const FromType* Source)
		{
			if (Source)
			{
				Init(Source, TCString<FromType>::Strlen(Source), ENullTerminatedString::Yes);
			}
			else
			{
				Ptr = NULL;
				StringLength = 0;
			}
		}

		StringConversion(const FromType* Source, int32 SourceLen)
		{
			if (Source)
			{
				Init(Source, SourceLen, ENullTerminatedString::No);
			}
			else
			{
				Ptr = NULL;
				StringLength = 0;
			}
		}

		/**
		* Move constructor
		*/
		StringConversion(StringConversion&& Other)
			: Converter(Move((Converter&&)Other))
		{
			AllocatorType::MoveToEmpty(Other);
		}

		/**
		* Accessor for the converted string.
		*
		* @return A const pointer to the null-terminated converted string.
		*/
		__forceinline const ToType* Get() const
		{
			return Ptr;
		}

		/**
		* Length of the converted string.
		*
		* @return The number of characters in the converted string, excluding any null terminator.
		*/
		__forceinline int32 Length() const
		{
			return StringLength;
		}
	};


	/**
	* NOTE: The objects these macros declare have very short lifetimes. They are
	* meant to be used as parameters to functions. You cannot assign a variable
	* to the contents of the converted string as the object will go out of
	* scope and the string released.
	*
	* NOTE: The parameter you pass in MUST be a proper string, as the parameter
	* is typecast to a pointer. If you pass in a char, not char* it will compile
	* and then crash at runtime.
	*
	* Usage:
	*
	*		SomeApi(TCHAR_TO_ANSI(SomeUnicodeString));
	*
	*		const char* SomePointer = TCHAR_TO_ANSI(SomeUnicodeString); <--- Bad!!!
	*/

	// Usage of these should be replaced with StringCasts.
#define TCHAR_TO_ANSI(str) (ANSICHAR*)StringCast<ANSICHAR>(static_cast<const TCHAR*>(str)).Get()
#define ANSI_TO_TCHAR(str) (TCHAR*)StringCast<TCHAR>(static_cast<const ANSICHAR*>(str)).Get()

	// This seemingly-pointless class is intended to be API-compatible with StringConversion
	// and is returned by StringCast when no string conversion is necessary.
	template <typename T>
	class StringPointer
	{
	public:
		__forceinline explicit StringPointer(const T* InPtr)
			: Ptr(InPtr)
		{
		}

		__forceinline const T* Get() const
		{
			return Ptr;
		}

		__forceinline int32 Length() const
		{
			return Ptr ? TCString<T>::Strlen(Ptr) : 0;
		}

	private:
		const T* Ptr;
	};

	/**
	* StringCast example usage:
	*
	* void Func(const String& Str)
	* {
	*     auto Src = StringCast<ANSICHAR>();
	*     const ANSICHAR* Ptr = Src.Get(); // Ptr is a pointer to an ANSICHAR representing the potentially-converted string data.
	* }
	*
	*/

	/**
	* Creates an object which acts as a source of a given string type.  See example above.
	*
	* @param Str The null-terminated source string to convert.
	*/
	template <typename To, typename From>
	__forceinline typename EnableIf<CStringUtil::AreEncodingsCompatible<To, From>::Value, StringPointer<To>>::Type StringCast(const From* Str)
	{
		return StringPointer<To>((const To*)Str);
	}

	/**
	* Creates an object which acts as a source of a given string type.  See example above.
	*
	* @param Str The null-terminated source string to convert.
	*/
	template <typename To, typename From>
	__forceinline typename EnableIf<!CStringUtil::AreEncodingsCompatible<To, From>::Value, StringConversion<StringConvert<From, To>>>::Type StringCast(const From* Str)
	{
		return StringConversion<StringConvert<From, To>>(Str);
	}

	/**
	* Creates an object which acts as a source of a given string type.  See example above.
	*
	* @param Str The source string to convert, not necessarily null-terminated.
	* @param Len The number of From elements in Str.
	*/
	template <typename To, typename From>
	__forceinline typename EnableIf<CStringUtil::AreEncodingsCompatible<To, From>::Value, StringPointer<To>>::Type StringCast(const From* Str, int32 Len)
	{
		return StringPointer<To>((const To*)Str);
	}

	/**
	* Creates an object which acts as a source of a given string type.  See example above.
	*
	* @param Str The source string to convert, not necessarily null-terminated.
	* @param Len The number of From elements in Str.
	*/
	template <typename To, typename From>
	__forceinline typename EnableIf<!CStringUtil::AreEncodingsCompatible<To, From>::Value, StringConversion<StringConvert<From, To>>>::Type StringCast(const From* Str, int32 Len)
	{
		return StringConversion<StringConvert<From, To>>(Str, Len);
	}


	/**
	* Casts one fixed-width char type into another.
	*
	* @param Ch The character to convert.
	* @return The converted character.
	*/
	template <typename To, typename From>
	__forceinline To CharCast(From Ch)
	{
		To Result;
		CStringUtil::Convert(&Result, 1, &Ch, 1, (To)UNICODE_BOGUS_CHAR_CODEPOINT);
		return Result;
	}

	/**
	* This class is returned by StringPassthru and is not intended to be used directly.
	*/
	template <typename ToType, typename FromType>
	class StringPassthru : private InlineAllocator<DEFAULT_STRING_CONVERSION_SIZE>::template ForElementType<FromType>
	{
	private:
		typedef typename InlineAllocator<DEFAULT_STRING_CONVERSION_SIZE>::template ForElementType<FromType> AllocatorType;

		ToType* Dest;
		int32   DestLen;
		int32   SrcLen;

	public:
		__forceinline StringPassthru(ToType* InDest, int32 InDestLen, int32 InSrcLen)
			: Dest(InDest)
			, DestLen(InDestLen)
			, SrcLen(InSrcLen)
		{
			AllocatorType::ResizeAllocation(0, SrcLen, sizeof(FromType));
		}

		__forceinline StringPassthru(StringPassthru&& Other)
		{
			AllocatorType::MoveToEmpty(Other);
		}

		// Non-copyable
		StringPassthru(const StringPassthru&) = delete;
		StringPassthru& operator=(const StringPassthru&) = delete;

		__forceinline void Apply() const
		{
			const FromType* Source = (const FromType*)AllocatorType::GetAllocation();
			Assert(CStringUtil::ConvertedLength<ToType>(Source, SrcLen) <= DestLen);
			CStringUtil::Convert(Dest, DestLen, Source, SrcLen);
		}

		__forceinline FromType* Get()
		{
			return (FromType*)AllocatorType::GetAllocation();
		}
	};

	// This seemingly-pointless class is intended to be API-compatible with StringPassthru
	// and is returned by StringPassthru when no string conversion is necessary.
	template <typename T>
	class PassthruPointer
	{
	public:
		__forceinline explicit PassthruPointer(T* InPtr)
			: Ptr(InPtr)
		{
		}

		__forceinline T* Get() const
		{
			return Ptr;
		}

		__forceinline void Apply() const
		{
		}

	private:
		T* Ptr;
	};

	/**
	* Allows the efficient conversion of strings by means of a temporary memory buffer only when necessary.  Intended to be used
	* when you have an API which populates a buffer with some string representation which is ultimately going to be stored in another
	* representation, but where you don't want to do a conversion or create a temporary buffer for that string if it's not necessary.
	*
	* Intended use:
	*
	* // Populates the buffer Str with StrLen characters.
	* void SomeAPI(APICharType* Str, int32 StrLen);
	*
	* void Func(DestChar* Buffer, int32 BufferSize)
	* {
	*     // Create a passthru.  This takes the buffer (and its size) which will ultimately hold the string, as well as the length of the
	*     // string that's being converted, which must be known in advance.
	*     // An explicit template argument is also passed to indicate the character type of the source string.
	*     // Buffer must be correctly typed for the destination string type.
	*     auto Passthru = StringMemoryPassthru<APICharType>(Buffer, BufferSize, SourceLength);
	*
	*     // Passthru.Get() returns an APICharType buffer pointer which is guaranteed to be SourceLength characters in size.
	*     // It's possible, and in fact intended, for Get() to return the same pointer as Buffer if DestChar and APICharType are
	*     // compatible string types.  If this is the case, SomeAPI will write directly into Buffer.  If the string types are not
	*     // compatible, Get() will return a pointer to some temporary memory which allocated by and owned by the passthru.
	*     SomeAPI(Passthru.Get(), SourceLength);
	*
	*     // If the string types were not compatible, then the passthru used temporary storage, and we need to write that back to Buffer.
	*     // We do that with the Apply call.  If the string types were compatible, then the data was already written to Buffer directly
	*     // and so Apply is a no-op.
	*     Passthru.Apply();
	*
	*     // Now Buffer holds the data output by SomeAPI, already converted if necessary.
	* }
	*/
	template <typename From, typename To>
	__forceinline typename EnableIf<CStringUtil::AreEncodingsCompatible<To, From>::Value, PassthruPointer<From>>::Type StringMemoryPassthru(To* Buffer, int32 BufferSize, int32 SourceLength)
	{
		Assert(SourceLength <= BufferSize);
		return PassthruPointer<From>((From*)Buffer);
	}

	template <typename From, typename To>
	__forceinline typename EnableIf<!CStringUtil::AreEncodingsCompatible<To, From>::Value, StringPassthru<To, From>>::Type StringMemoryPassthru(To* Buffer, int32 BufferSize, int32 SourceLength)
	{
		return StringPassthru<To, From>(Buffer, BufferSize, SourceLength);
	}

	template <typename ToType, typename FromType>
	__forceinline Array<ToType> StringToArray(const FromType* Src, int32 SrcLen)
	{
		int32 DestLen = CStringUtil::ConvertedLength<TCHAR>(Src, SrcLen);

		Array<ToType> Result;
		Result.AddUninitialized(DestLen);
		CStringUtil::Convert(Result.Data(), DestLen, Src, SrcLen);

		return Result;
	}

	template <typename ToType, typename FromType>
	__forceinline Array<ToType> StringToArray(const FromType* Str)
	{
		return ToArray(Str, TCString<FromType>::Strlen(Str) + 1);
	}

}