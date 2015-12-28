//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//
//	File: Cry_PS3_Math.h
//	Description: declaration of fast trigonometric funcs for PS3
//
//	History:
//	-May 29,2008: Created by MichaelG
//
//////////////////////////////////////////////////////////////////////
#if defined(PS3)

#ifndef CRY_PS3_MATH
#define CRY_PS3_MATH

#ifndef VOID
	#define VOID void
#endif
#ifndef CONST
	#define CONST const
#endif

#ifdef __SPU__
	#undef __fsel
	#define __fsel(x,y,z) (((x)>=0)?(y):(z))
	#undef __fres
	#define __fres(a) (1.f/(a))
#endif

#undef __fself
#undef __fsels
#define __fself(x,y,z) __fsel(x,y,z)
#define __fsels(x,y,z) __fsel(x,y,z)
#define __fsqrts(x) (sqrt_tpl(x))
#define __frsqrte(x) (isqrt_tpl(x))

//copies an vector from unaligned source to unaligned destination
ILINE void CopyVecUnaligned(const uint8* const __restrict ptrSrc, uint8* const __restrict ptrDst)
{
#ifdef __SPU__
	const uint32 shiftSrc		= (uint32)ptrSrc & 15;
	const uint32 shiftDst		= (uint32)ptrDst & 15;
	vec_uchar16				qw		= (vec_uchar16)spu_or(spu_slqwbyte(*(vec_uchar16*)ptrSrc, shiftSrc), spu_rlmaskqwbyte(((vec_uchar16*)ptrSrc)[1], (signed)(shiftSrc-16)));
	
	const vec_uchar16	qw0		= *(vec_uchar16*)ptrDst;
	const vec_uchar16	qw1		= ((vec_uchar16*)ptrDst)[1];

	vec_uint4 mask = (vec_uint4) spu_rlmaskqwbyte((vec_uchar16){0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}, -shiftDst);
	qw = spu_rlqwbyte(qw, -shiftDst);
	*(vec_uchar16*)ptrDst = (vec_uchar16)spu_sel( (vec_uint4)qw0, (vec_uint4)qw, mask);
	((vec_uchar16*)ptrDst)[1] = (vec_uchar16)spu_sel( (vec_uint4)qw, (vec_uint4)qw1, mask);
#else
	vec_uchar16 VL = (vec_uchar16)vec_lvlx(0,ptrSrc);
	vec_uchar16 VR = (vec_uchar16)vec_lvrx(16, ptrSrc);
	vec_uchar16 qw = vec_or(VL, VR);
	vec_stvlx(qw,0,ptrDst);
	vec_stvrx(qw,16,ptrDst);
#endif
}

#ifdef __SPU__
//copies an vector from unaligned source to unaligned destination
ILINE void StoreVecUnaligned(qword qw, uint8* const __restrict ptrDst)
{
	const uint32 shiftDst		= (uint32)ptrDst & 15;
	const qword	qw0		= *(qword*)ptrDst;
	const qword	qw1		= ((qword*)ptrDst)[1];
	
	vec_uint4 mask = (vec_uint4) spu_rlmaskqwbyte((qword){0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}, -shiftDst);
	qw = spu_rlqwbyte(qw, -shiftDst);
	*(qword*)ptrDst = (qword)spu_sel( (vec_uint4)qw0, (vec_uint4)qw, mask);
	((qword*)ptrDst)[1] = (qword)spu_sel( (vec_uint4)qw, (vec_uint4)qw1, mask);
}
//copies an vector from unaligned source to unaligned destination
ILINE const qword LoadVecUnaligned(const uint8* const __restrict ptrSrc)
{
	const uint32 shiftSrc		= (uint32)ptrSrc & 15;
	return (qword)spu_or(spu_slqwbyte(*(qword*)ptrSrc, shiftSrc), spu_rlmaskqwbyte(((qword*)ptrSrc)[1], (signed)(shiftSrc-16)));
}
#else
#define StoreVecUnaligned(qw, ptrDst)\
	vec_stvlx((qword)(qw),0,(uint8*)(ptrDst));vec_stvrx((qword)(qw),16,(uint8*)(ptrDst))
#define LoadVecUnaligned(ptrSrc)\
	(qword)vec_or((qword)vec_lvlx(0,(uint8*)(ptrSrc)), (qword)vec_lvrx(16, (uint8*)(ptrSrc)))
#endif

// Compatibility with Xenon SDK unaligned vector load/store macros
#define __loadunalignedvector(ptr)						(XMVECTOR)LoadVecUnaligned((const uint8*)ptr)
#define __storeunalignedvector(vector, ptr)		StoreVecUnaligned(vector, (const uint8*)ptr)
#if !defined(__SPU__) 
#define __loadunalignedvector_volatile(ptr)		(__vor(__lvlx_volatile(ptr, 0), __lvrx_volatile(ptr,16)))
#else
#define __loadunalignedvector_volatile(ptr)		(XMVECTOR)LoadVecUnaligned((const uint8*)ptr)
#endif
ILINE vec_float4 __lvlx_volatile(const volatile void * base, int offset)
{
	// Volatile memory fence to stop compiler re-ordering the load
	MEMORY_RW_REORDERING_BARRIER;
	return vec_lvlx(offset, (const volatile float*)base);
}

ILINE vec_float4 __lvrx_volatile(const volatile void * base, int offset)
{
	// Volatile memory fence to stop compiler re-ordering the load
	MEMORY_RW_REORDERING_BARRIER;
	return vec_lvrx(offset, (const volatile float*)base);
}


#if defined(PS3OPT)
#if !defined(__SPU__) && !defined(SUPP_FP_EXC)
	//--------------------------------fast triangular functions----------------------------------------------
	extern void   ps3_sincos(double, double*, double*);
	extern void   ps3_sincos(float, float*, float*);
	extern float	ps3_sin(float x);
	extern float	ps3_cos(float x);
	extern float	ps3_tan(float x);
	extern float	ps3_asin(float x);
	extern float	ps3_acos(float x);
	extern float	ps3_atan(float x);
	extern float	ps3_atan2(float x, float y);
	extern double	ps3_sin(double x);
	extern double	ps3_cos(double x);
	extern double	ps3_tan(double x);
	extern double	ps3_asin(double x);
	extern double	ps3_acos(double x);
	extern double	ps3_atan(double	x);
	extern double	ps3_atan2(double x, double y);
	ILINE void		cry_sincosf (float angle, float* pSin, float* pCos){ps3_sincos(angle, pSin, pCos);}
	ILINE void		cry_sincos  (double angle, double* pSin, double* pCos){ps3_sincos(angle, pSin, pCos);}
	ILINE float		cry_sinf(float x)		{return ps3_sin(x);}
	ILINE float		cry_cosf(float x)		{return ps3_cos(x);}
	ILINE float		cry_tanf(float x)		{return ps3_tan(x);}
	ILINE float		cry_asinf(float x)	{return ps3_asin((float)__fsel(x+1.0f, __fsel(x-1.0f,1.0f,x), -1.0f));}
	ILINE float		cry_acosf(float x)	{return ps3_acos((float)__fsel(x+1.0f, __fsel(x-1.0f,1.0f,x), -1.0f));}
	ILINE float		cry_atanf(float x)	{return ps3_atan(x);}
	ILINE float		cry_atan2f(float y, float x){return ps3_atan2(y, x);}
	ILINE double	cos_tpl(double op)	{return ps3_cos(op);}
	ILINE double	sin_tpl(double op)	{return ps3_sin(op);}
	ILINE double	acos_tpl(double op)	{return ps3_acos(op);}
	ILINE double	asin_tpl(double op)	{return ps3_asin(op);}
	ILINE double	tan_tpl(double op)	{return ps3_tan(op);}
	ILINE double	atan_tpl(double op) {return ps3_atan(op); }
	ILINE double	atan2_tpl(double op1,double op2) {return ps3_atan2(op1, op2);}
#endif

#if 1
	//--------------------------------XMVECTOR def----------------------------------------------

	//enable this define to use scalar impl. of vector funcs
//	#define EMULATE_XENON_INTRINSICS
  #if !defined(ALIGN16)
	#  define ALIGN16 _ALIGN(16)
  #endif
	//		#define XINLINE static
	#define XINLINE ILINE		
	//		#define VERIFY_XMVECTOR(V,num) for(uint32 _u=0;_u<(num);++_u)if(!IsXenonNumberValid((V).v[_u]))snPause();
	#define VERIFY_XMVECTOR(V,num)
	//#define IsXenonNumberValid NumberValid
	#define XMDUMMY_INITIALIZE_UINT(a) (a)=0
	#define IsXenonNumberValid(a) true
	#if !defined(IsXenonNumberValid)
		extern bool IsXenonNumberValid(const float& x);
	#endif

	typedef vec_float4 XMVECTOR;
	typedef vec_float4 XMVECTORF32;
	typedef vec_uint4 XMVECTORI;

	union XMVECTOR_CONV
	{
		XMVECTOR vec;
		struct  
		{
			float x,y,z,w;
		};
		float v[4];
		unsigned int u[4];
		ILINE XMVECTOR_CONV(){}
		ILINE XMVECTOR_CONV(XMVECTOR v)
		{
			vec = v;
		}
		ILINE XMVECTOR_CONV& operator=(const XMVECTOR v)
		{
			vec = v;
			return *this;
		}
		ILINE operator XMVECTOR() const
		{
			return vec;
		}
	} __attribute__((d64_abi));

	#define CHECK_XMVECTOR_RES(name,xv,xvc,c){XMVECTOR_CONV _c(xv);for(uint32 u=0;u<c;++u)if(fabs(_c.v[u]-xvc.v[u])>(0.05f*fabs(xvc.v[u])+0.001f)){printf("%s: xv[%d]=%f  xvc[%d]=%f\n",name,u,_c.v[u],u,xvc.v[u]);snPause();}}

	//--------------------------------Power PC intrinsic mappings----------------------------------------------

	//defines: #define __vpkd3d(ValueV, ValueV, VPACK_FLOAT16_4, VPACK_64LO, 2)
	//#define __vpkd3d(a, b, c, d, e)\

	#define XM_CRMASK_CR6TRUE   0x00000080
	#define XM_CRMASK_CR6FALSE  0x00000020
	#define XMComparisonAllTrue(CR)  (((CR) & XM_CRMASK_CR6TRUE)  == XM_CRMASK_CR6TRUE)
	#define XMComparisonAllFalse(CR) (((CR) & XM_CRMASK_CR6FALSE) == XM_CRMASK_CR6FALSE)
	#define XMComparisonAnyFalse(CR) (((CR) & XM_CRMASK_CR6TRUE) != XM_CRMASK_CR6TRUE)

	#define __lvlx(a,b) vec_lvlx(b,(float*)a)
	#define __lvrx(a,b) vec_lvrx(b,(float*)a)
	#define __lvx(a,b) vec_lvx(b,(float*)a)
//	#define __vsplth vec_vsplth
//	#define __stvehx vec_stvehx
	#define __stvx(a,b,c) vec_stvx(a,c,(float*)b)
	#define __stvrx(a,b,c) vec_stvrx(a,c,(float*)b)
	#define __stvlx(a,b,c) vec_stvlx(a,c,(float*)b)
	#define __lvsr(a,b) vec_lvsr(b,a)
	#define __vor vec_or
	#define vec_vsel vec_sel

#ifdef __SPU__
	#include <spu_intrinsics.h>
	#include <vmx2spu.h>
  #define __vecreg
  #define vec_sld(_a, _b, _c)	spu_shuffle(_a, _b, ((vec_uchar16){ 0+(_c),  1+(_c),  2+(_c),  3+(_c), \
        4+(_c),  5+(_c),  6+(_c),  7+(_c),                              \
        8+(_c),  9+(_c), 10+(_c), 11+(_c),                              \
        12+(_c), 13+(_c), 14+(_c), 15+(_c)}))
  #define vec_splat_s32 spu_splats
	#define __vrefp vec_re
  #define __vsldoi vec_sld
	#define __vspltw vec_splat
	#define __vaddfp vec_add
	#define __vsubfp vec_sub
	#define __vmaddfp vec_madd
	#define __vminfp fminf4
	#define __vmaxfp fmaxf4
	#define __vcmpeqfp(a,b) (XMVECTOR)vec_cmpeq(a,b)
	#define __vandc(a,b) (XMVECTOR)(qword)vec_andc(qword)a, (qword)b)
	#define __vcmpequw(a,b) (XMVECTOR)(qword)vec_cmpeq((vec_uint4)(qword)a,(vec_uint4)(qword)b)
	#define __vrsqrtefp vec_rsqrte
	#define __vmrghw vec_mergeh
	#define __vmrglw vec_mergel
	#define	__vexptefp(a) ( (XMVECTOR) vec_vexptefp ( (XMVECTOR)(a) ) )
	#define __vsel(a,b,c) (XMVECTOR)vec_vsel((vec_uint4)(qword)a, (vec_uint4)(qword)b, (vec_uint4)(qword)c)
	#define __vcmpgtfp(a, b) vec_cmpgt(a, b)
	#define __vcmpgefp(a, b) vec_cmpge(a, b)
	#define __vcmpgtsw(a, b) vec_vcmpgtsw((vec_int4)(a), (vec_int4)(b))
	#define __vcmpgtuw(a, b) vec_vcmpgtuw((vec_uint4)(a), (vec_uint4)(b))
	#define __vperm(a,b,c) (qword)vec_perm((vec_uchar16)a,(vec_uchar16)b,(vec_uchar16)c)
	#define __vcfux(a,b) spu_convtf((vec_uint4)(qword)a, (unsigned int)b)
	ILINE void __stvewx(const vec_float4 a, float* const __restrict b, const unsigned int cIndex)
	{
		XMVECTOR_CONV conv(a);
		b[cIndex/4] = conv.v[cIndex/4];
	}
#else
	#define __vrefp vec_re
	#define __vsldoi vec_vsldoi
	#define __vspltw vec_vspltw
	#define __vaddfp vec_vaddfp
	#define __vsubfp vec_vsubfp
	#define __vmaddfp vec_vmaddfp
	#define __vminfp vec_vminfp
	#define __vmaxfp vec_vmaxfp
	#define __vcmpeqfp(a,b) (XMVECTOR)vec_vcmpeqfp(a,b)
	#define __vandc(a,b) (XMVECTOR)(qword)vec_vandc((qword)a, (qword)b)
	#define __vcmpequw(a,b) (XMVECTOR)(qword)vec_vcmpequw((vec_uint4)(qword)a,(vec_uint4)(qword)b)
	#define __vrsqrtefp vec_vrsqrtefp
	#define __vmrghw vec_vmrghw
	#define __vmrglw vec_vmrglw
	#define __vcfsx(a,b) (XMVECTOR)(qword)vec_vcfsx((vec_int4)(qword)a, (unsigned char)b)
	#define __vmulfp(a,b) (vec_madd(a, b, (XMVECTOR){0.0f, 0.f, 0.f, 0.f}))
	#define	__vexptefp(a) ( (XMVECTOR) vec_vexptefp ( (XMVECTOR)(a) ) )
	#define __vsel(a,b,c) (XMVECTOR)vec_vsel((qword)a, (qword)b, (qword)c)
	#define __vperm(a,b,c) (qword)vec_perm((vec_float4)(qword)a,(vec_float4)(qword)b,(qword)c)
	#define __vcfux(a,b) vec_vcfux((vec_uint4)(qword)a, (int)b)
	#define __stvewx(a,b,c) vec_stvewx(a,c,(float*)b)
	#define __vcmpgtfp(a, b) vec_cmpgt(a, b)
	#define __vcmpgefp(a, b) vec_cmpge(a, b)
	#define __vcmpgtsw(a, b) vec_vcmpgtsw((vec_int4)(a), (vec_int4)(b))
	#define __vcmpgtuw(a, b) vec_vcmpgtuw((vec_uint4)(a), (vec_uint4)(b))

#endif
	
	#define	__vmrghh(a,b) (qword)vec_mergeh((vec_ushort8)(qword)a, (vec_ushort8)(qword)b)
	#define __vand(a, b) (XMVECTOR)vec_and((qword)a, (qword)b)
	#define __vxor(a,b) (qword)vec_xor((qword)a,(qword)b)
	#define __vspltisw (qword)vec_splat_s32
	#define __vslw(a,b) (qword)vec_vslw((vec_uint4)(qword)a,(vec_uint4)(qword)b)
	#define __vmsum3fp(a,b) (XMVector3Dot(a,b))
	#define __vmsum4fp(a,b) (XMVector4Dot(a,b))
	#define __vmaddcfp __vmaddfp
	#define __vrfin vec_round
	#define __vupklsh(a) (XMVECTOR)vec_vupklsh((vec_short8)a)
	#define __vupkhsh(a) (XMVECTOR)vec_vupkhsh((vec_short8)a)
	#define __vsubuwm(a, b) (XMVECTOR)vec_vsubuwm((vec_int4)a, (vec_int4)b)
	#define __vsraw(a, b) (XMVECTOR)vec_vsraw((vec_int4)a, (vec_uint4)b)
	#define __vsrw(a, b) (XMVECTOR)vec_vsrw((vec_uint4)a, (vec_uint4)b)
	#define __vnmsubfp vec_nmsub

	//PERMUATATION CONSTANTS
	#define _VECTORMATH_PERM_X 0x00010203
	#define _VECTORMATH_PERM_Y 0x04050607
	#define _VECTORMATH_PERM_Z 0x08090a0b
	#define _VECTORMATH_PERM_W 0x0c0d0e0f
	#define _VECTORMATH_PERM_A 0x10111213
	#define _VECTORMATH_PERM_B 0x14151617
	#define _VECTORMATH_PERM_C 0x18191a1b
	#define _VECTORMATH_PERM_D 0x1c1d1e1f

	#define XM_PERMUTE_0X _VECTORMATH_PERM_X
	#define XM_PERMUTE_0Y _VECTORMATH_PERM_Y
	#define XM_PERMUTE_0Z _VECTORMATH_PERM_Z
	#define XM_PERMUTE_0W _VECTORMATH_PERM_W
	#define XM_PERMUTE_1X _VECTORMATH_PERM_A
	#define XM_PERMUTE_1Y _VECTORMATH_PERM_B
	#define XM_PERMUTE_1Z _VECTORMATH_PERM_C
	#define XM_PERMUTE_1W _VECTORMATH_PERM_D


	#define _VECTORMATH_PERM_XYZA ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_A })
	#define _VECTORMATH_PERM_XYAA ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_A, _VECTORMATH_PERM_A })
	#define _VECTORMATH_PERM_ZBWX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Z, _VECTORMATH_PERM_B, _VECTORMATH_PERM_W, _VECTORMATH_PERM_X })
	#define _VECTORMATH_PERM_XCYX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_C, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_X })
	#define _VECTORMATH_PERM_XYAB ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_A, _VECTORMATH_PERM_B })
	#define _VECTORMATH_PERM_ZWCD ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Z, _VECTORMATH_PERM_W, _VECTORMATH_PERM_C, _VECTORMATH_PERM_D })
	#define _VECTORMATH_PERM_XZBX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_B, _VECTORMATH_PERM_X })     
	#define _VECTORMATH_PERM_CXXX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_C, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X })
	#define _VECTORMATH_PERM_YAXX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Y, _VECTORMATH_PERM_A, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X })
	#define _VECTORMATH_PERM_XAZC ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_A, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_C })
	#define _VECTORMATH_PERM_YXWZ ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Y, _VECTORMATH_PERM_X, _VECTORMATH_PERM_W, _VECTORMATH_PERM_Z })
	#define _VECTORMATH_PERM_YBWD ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Y, _VECTORMATH_PERM_B, _VECTORMATH_PERM_W, _VECTORMATH_PERM_D })
	#define _VECTORMATH_PERM_XYCX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_C, _VECTORMATH_PERM_X })
	#define _VECTORMATH_PERM_YCXY ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Y, _VECTORMATH_PERM_C, _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y })
	#define _VECTORMATH_PERM_CXYC ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_C, _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_C })
	#define _VECTORMATH_PERM_ZAYX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Z, _VECTORMATH_PERM_A, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_X })
	#define _VECTORMATH_PERM_BZXX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_B, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X })
	#define _VECTORMATH_PERM_XZYA ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_A })
	#define _VECTORMATH_PERM_ZXXB ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Z, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X, _VECTORMATH_PERM_B })
	#define _VECTORMATH_PERM_YXXC ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Y, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X, _VECTORMATH_PERM_C })
	#define _VECTORMATH_PERM_BBYX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_B, _VECTORMATH_PERM_B, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_X })

	//--------------------------------functions for Cry_Xenon_Math.h----------------------------------------------

	XINLINE XMVECTOR _XMLoadVector4A(const float* __restrict pSource)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV V;
		V.v[0] = pSource[0];
		V.v[1] = pSource[1];
		V.v[2] = pSource[2];
		V.v[3] = pSource[3];
		VERIFY_XMVECTOR(V,4)
		return V.vec;
#else
	#ifdef __SPU__
		return *((vec_float4*)pSource);
	#else
		return __lvx(pSource, 0);
	#endif
#endif
	}

	XINLINE XMVECTOR _XMLoadVector3A(const float* __restrict pSource)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV V;
		V.v[0] = pSource[0];
		V.v[1] = pSource[1];
		V.v[2] = pSource[2];
		VERIFY_XMVECTOR(V,3)
		return V.vec;
#else
	#ifdef __SPU__
		return *((vec_float4*)pSource);
	#else
		return __lvx(pSource, 0);
	#endif
#endif
	}

	XINLINE void _XMStoreVector4A(float* __restrict pDestination, XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V(V);
		pDestination[0] = _V.v[0];
		pDestination[1] = _V.v[1];
		pDestination[2] = _V.v[2];
		pDestination[3] = _V.v[3];
		VERIFY_XMVECTOR(_V,4)
#else
	#ifdef __SPU__
		*((vec_float4*)pDestination) = V;
	#else
		__stvx(V, pDestination, 0);
	#endif
#endif
	}

	XINLINE void _XMStoreVector3A(float* __restrict pDestination, XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V(V);
		pDestination[0] = _V.v[0];
		pDestination[1] = _V.v[1];
		pDestination[2] = _V.v[2];
		VERIFY_XMVECTOR(V,3)
#else
	#ifdef __SPU__
		XMVECTOR_CONV _V(V);
		pDestination[0] = _V.v[0];
		pDestination[1] = _V.v[1];
		pDestination[2] = _V.v[2];
	#else
		XMVECTOR AlignedV = __vsldoi(V, V, 3 << 2);
		__stvrx(AlignedV, pDestination, 12);
	#endif
#endif
	}

	XINLINE XMVECTOR _XMLoadVector4(const float* __restrict pSource)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV V;
		V.v[0] = pSource[0];
		V.v[1] = pSource[1];
		V.v[2] = pSource[2];
		V.v[3] = pSource[3];
		VERIFY_XMVECTOR(V,4)
		return V.vec;
#else
		return (XMVECTOR)(qword)LoadVecUnaligned((uint8*)pSource);
#endif
	}

	XINLINE XMVECTOR _XMLoadVector3(const float* pSource)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV V;
		V.v[0] = pSource[0];
		V.v[1] = pSource[1];
		V.v[2] = pSource[2];
		VERIFY_XMVECTOR(V,3)
		return V.vec;
#else
	#ifdef __SPU__
		return (XMVECTOR)(qword)LoadVecUnaligned((uint8*)pSource);
	#else
		XMVECTOR VL;
		XMVECTOR VR;
		VR = __lvrx(pSource, 12);
		VL = __lvlx(pSource, 0);
		VR = __vsldoi(VR, VR, 1 << 2);
		return __vor(VL, VR);
	#endif
#endif
	}

	XINLINE XMVECTOR _XMLoadVector2(const float* pSource)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV V;
		V.v[0] = pSource[0];
		V.v[1] = pSource[1];
		VERIFY_XMVECTOR(V,2)
		return V.vec;
#else
	#ifdef __SPU__
		return (XMVECTOR){pSource[0], pSource[1], pSource[2], 0.f};
	#else
		XMVECTOR VL;
		XMVECTOR VR;
		VR = __lvrx(pSource, 8);
		VL = __lvlx(pSource, 0);
		VR = __vsldoi(VR, VR, 2 << 2);
		return __vor(VL, VR);
	#endif
#endif
	}

	XINLINE void _XMStoreVector4(float* pDestination, XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V(V);
		pDestination[0] = _V.v[0];
		pDestination[1] = _V.v[1];
		pDestination[2] = _V.v[2];
		pDestination[3] = _V.v[3];
		VERIFY_XMVECTOR(V,4)
#else
		StoreVecUnaligned((qword)V, (uint8*)pDestination);
#endif
	}

	XINLINE void _XMStoreVector3(float* pDestination, XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V(V);
		pDestination[0] = _V.v[0];
		pDestination[1] = _V.v[1];
		pDestination[2] = _V.v[2];
		VERIFY_XMVECTOR(V,3)
#else
	#ifdef __SPU__
		XMVECTOR_CONV _V(V);
		pDestination[0] = _V.v[0];
		pDestination[1] = _V.v[1];
		pDestination[2] = _V.v[2];
	#else
		XMVECTOR X, Y, Z;
		X = __vspltw(V, 0);
		Y = __vspltw(V, 1);
		Z = __vspltw(V, 2);
		__stvewx(X, pDestination, 0);
		__stvewx(Y, pDestination, 4);
		__stvewx(Z, pDestination, 8);
	#endif
#endif
	}

	XINLINE void _XMStoreFloat(float* pDestination, XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V(V);
		pDestination[0] = _V.v[0];
		VERIFY_XMVECTOR(V,1)
#else
#ifdef __SPU__
		XMVECTOR_CONV _V(V);
		pDestination[0] = _V.v[0];
#else
		XMVECTOR X;
		X = __vspltw(V, 0);
		__stvewx(X, pDestination, 0);
#endif
#endif
	}

	#define XMLoadVector4A(p) _XMLoadVector4A((float*)p)
	#define XMLoadVector3A(p) _XMLoadVector3A((float*)p)
	#define XMLoadVector4(p) _XMLoadVector4((float*)p)
	#define XMLoadVector3(p) _XMLoadVector3((float*)p)
	#define XMLoadVector2(p) _XMLoadVector2((float*)p)
	#define XMStoreVector4A(p,v) _XMStoreVector4A((float*)p, v)
	#define XMStoreVector3A(p,v) _XMStoreVector3A((float*)p, v)
	#define XMStoreVector4(p,v) _XMStoreVector4((float*)p, v)
	#define XMStoreVector3(p,v) _XMStoreVector3((float*)p, v)
	#define XMStoreFloat(p,v)	_XMStoreFloat((float*)p, v)

	XINLINE XMVECTOR XMVectorClamp(XMVECTOR V, XMVECTOR Min, XMVECTOR Max)
	{
		return __vminfp(Max, __vmaxfp(Min, V));
	}

	// 3D Vector; 32 bit floating point components, keep it POD since it used in anon. unions
	struct XMFLOAT3
	{
		float x;
		float y;
		float z;
	};

	// 4D Vector; 32 bit floating point components, keep it POD since it used in anon. unions
	struct XMFLOAT4
	{
		float x, y, z, w;
	};

	struct XMFLOAT4A
	{
		union
		{
			struct  
			{
				float x, y, z, w;
			};
			XMVECTOR v;
		};
	} ALIGN16 __passinreg_vec;

	struct XMMATRIX
	{
#ifdef EMULATE_XENON_INTRINSICS
		union
		{
#endif
			XMVECTOR r[4];
#ifdef EMULATE_XENON_INTRINSICS
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			struct
			{
				float m00, m01, m02, m03;
				float m10, m11, m12, m13;
				float m20, m21, m22, m23;
				float m30, m31, m32, m33;
			};
			float m[4][4];
		};
#endif
	} ALIGN16 __passinreg_vec;

	XINLINE void XMStoreFloat4AIndexed(XMFLOAT4A* const __restrict pDestination, XMVECTOR V, unsigned int offset)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V(V);
		pDestination[0] = _V.v[0];
		pDestination[1] = _V.v[1];
		pDestination[2] = _V.v[2];
		pDestination[3] = _V.v[3];
		VERIFY_XMVECTOR(_V,4)
#else
#ifdef __SPU__
		*((vec_float4*)&pDestination[offset]) = V;
#else
		__stvx(V, pDestination, offset << 4);
#endif
#endif
	}

	XINLINE void XMStoreFloat4AIndexed(XMFLOAT4* const __restrict pDestination, XMVECTOR V, unsigned int offset)
	{
		XMStoreFloat4AIndexed((XMFLOAT4A*)pDestination, V, offset);
	}

	//offset is vector offset
	XINLINE XMVECTOR XMLoadFloat4AIndexed(XMFLOAT4A* const __restrict pSrc, unsigned int offset)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV V;
		V.v[0] = pSrc[0];
		V.v[1] = pSrc[1];
		V.v[2] = pSrc[2];
		V.v[3] = pSrc[3];
		VERIFY_XMVECTOR(V,4)
			return V.vec;
#else
	#ifdef __SPU__
		return *(vec_float4*)&pSrc[offset];
	#else
		return __lvx(pSrc, offset<<4);
	#endif
#endif
	}

	XINLINE XMVECTOR XMLoadFloat4AIndexed(const XMFLOAT4* __restrict pSource, const unsigned int cOff)
	{
		return XMLoadFloat4AIndexed((XMFLOAT4A*)pSource, cOff);
	}

	//--------------------------------XMVECTOR funcs----------------------------------------------

	XINLINE XMVECTOR XMVectorZero()
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result(XMVectorReplicate(0.0f));
		return Result.vec;		
#else
		return (XMVECTOR){0.f,0.f,0.f,0.f};
#endif
	}
#ifdef __SPU__
	#define __vmulfp(a,b) (vec_madd(a, b, XMVectorZero()))
#endif

	XINLINE XMVECTOR XMVectorSetInt(unsigned int x, unsigned int y, unsigned int z, unsigned int w)
	{
		XMVECTOR_CONV V;
		V.u[0] = x;
		V.u[1] = y;
		V.u[2] = z;
		V.u[3] = w;
		return V.vec;
	}

	XINLINE XMVECTOR VecDot4(XMVECTOR vec0, XMVECTOR vec1)
	{
 		XMVECTOR result = vec_madd(vec0, vec1, XMVectorZero());
    result = vec_madd( vec_sld(vec0, vec0, 4), vec_sld(vec1, vec1, 4), result);
 		return vec_add(vec_sld( result, result, 8 ), result);
	}

	XINLINE XMVECTOR VecDot3(XMVECTOR vec0, XMVECTOR vec1)
	{
		XMVECTOR result = vec_madd( vec0, vec1, XMVectorZero());
		result = vec_madd(vec_sld( vec0, vec0, 4), vec_sld(vec1, vec1, 4), result);
		return vec_madd( vec_sld(vec0, vec0, 8), vec_sld(vec1, vec1, 8), result);
	}

	XINLINE XMVECTOR VecDot2(XMVECTOR vec0, XMVECTOR vec1)
	{
		XMVECTOR result = vec_madd( vec0, vec1, XMVectorZero());
		return vec_madd(vec_sld( vec0, vec0, 4), vec_sld(vec1, vec1, 4), result);
	}

	XINLINE XMVECTOR XMLoadFloat3(const XMFLOAT3* pSource)
	{
		return XMLoadVector3(&pSource->x);
	}

	XINLINE XMVECTOR XMLoadFloat4A(const XMFLOAT4* pSource)
	{
		return XMLoadVector4A((XMFLOAT4A*)&pSource->x);
	}

	XINLINE XMVECTOR XMLoadFloat4A(const XMFLOAT4A* pSource)
	{
		return XMLoadVector4A(&pSource->x);
	}

	XINLINE XMVECTOR XMLoadFloat4(const XMFLOAT4* pSource)
	{
		return XMLoadVector4(&pSource->x);
	}

	XINLINE XMVECTOR XMVectorSplatX(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V(V);
		Result.v[0] = Result.v[1] = Result.v[2] = Result.v[3] = _V.v[0];
		VERIFY_XMVECTOR(Result,4)
		return Result.vec;
#else
		return __vspltw(V, 0);
#endif
	}

	XINLINE XMVECTOR XMVectorSplatY(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V(V);
		XMVECTOR_CONV Result;
		Result.v[0] = Result.v[1] = Result.v[2] = Result.v[3] = _V.v[1];
		VERIFY_XMVECTOR(Result,4)
		return Result.vec;
#else
		return __vspltw(V, 1);
#endif
	}

	XINLINE XMVECTOR XMVectorSplatZ(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V(V);
		Result.v[0] = Result.v[1] = Result.v[2] = Result.v[3] = _V.v[2];
		VERIFY_XMVECTOR(Result,4)
		return Result.vec;
#else
		return __vspltw(V, 2);
#endif
	}

	XINLINE XMVECTOR XMVectorSplatW(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V(V);
		Result.v[0] = Result.v[1] = Result.v[2] = Result.v[3] = _V.v[3];
		VERIFY_XMVECTOR(Result,4)
		return Result.vec;
#else
		return __vspltw(V, 3);
#endif
	}

	XINLINE XMVECTOR XMVectorNegate(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V(V);
		Result.v[0] = -_V.v[0];
		Result.v[1] = -_V.v[1];
		Result.v[2] = -_V.v[2];
		Result.v[3] = -_V.v[3];
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
	#ifdef __SPU__
		return negatef4(V);
	#else
		qword SignMask = __vspltisw(-1);
		SignMask = __vslw(SignMask, SignMask);
		return (XMVECTOR)__vxor((qword)V, SignMask);
	#endif
#endif
	}

	XINLINE XMVECTOR XMVectorAdd(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		Result.v[0] = _V1.v[0] + _V2.v[0];
		Result.v[1] = _V1.v[1] + _V2.v[1];
		Result.v[2] = _V1.v[2] + _V2.v[2];
		Result.v[3] = _V1.v[3] + _V2.v[3];
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		return __vaddfp(V1, V2);
#endif
	}

	XINLINE XMVECTOR XMVectorSubtract(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		XMVECTOR_CONV Result;
		Result.v[0] = _V1.v[0] - _V2.v[0];
		Result.v[1] = _V1.v[1] - _V2.v[1];
		Result.v[2] = _V1.v[2] - _V2.v[2];
		Result.v[3] = _V1.v[3] - _V2.v[3];
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		return __vsubfp(V1, V2);
#endif
	}

	XINLINE XMVECTOR XMVectorMultiply(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		Result.v[0] = _V1.v[0] * _V2.v[0];
		Result.v[1] = _V1.v[1] * _V2.v[1];
		Result.v[2] = _V1.v[2] * _V2.v[2];
		Result.v[3] = _V1.v[3] * _V2.v[3];
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		return __vmulfp(V1, V2);
#endif
	}

	XINLINE XMVECTOR XMVectorReciprocal(XMVECTOR V1)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V1(V1);
		Result.v[0] = 1.0f / _V1.v[0];
		Result.v[1] = 1.0f / _V1.v[1];
		Result.v[2] = 1.0f / _V1.v[2];
		Result.v[3] = 1.0f / _V1.v[3];
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		return __vrefp(V1);
#endif
	}

	XINLINE XMVECTOR XMVectorMin(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		Result.v[0] = (_V1.v[0] < _V2.v[0]) ? _V1.v[0] : _V2.v[0];
		Result.v[1] = (_V1.v[1] < _V2.v[1]) ? _V1.v[1] : _V2.v[1];
		Result.v[2] = (_V1.v[2] < _V2.v[2]) ? _V1.v[2] : _V2.v[2];
		Result.v[3] = (_V1.v[3] < _V2.v[3]) ? _V1.v[3] : _V2.v[3];
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		return __vminfp(V1, V2);
#endif
	}

	XINLINE XMVECTOR XMVectorMax(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		Result.v[0] = (_V1.v[0] > _V2.v[0]) ? _V1.v[0] : _V2.v[0];
		Result.v[1] = (_V1.v[1] > _V2.v[1]) ? _V1.v[1] : _V2.v[1];
		Result.v[2] = (_V1.v[2] > _V2.v[2]) ? _V1.v[2] : _V2.v[2];
		Result.v[3] = (_V1.v[3] > _V2.v[3]) ? _V1.v[3] : _V2.v[3];
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		return __vmaxfp(V1, V2);
#endif
	}

	XINLINE XMVECTOR XMVectorMergeZW(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		Result.u[0] = _V1.u[2];
		Result.u[1] = _V2.u[2];
		Result.u[2] = _V1.u[3];
		Result.u[3] = _V2.u[3];
		VERIFY_XMVECTOR(Result,4)
		return Result.vec;
#else
		return __vmrglw(V1, V2);
#endif
	}

	XINLINE XMVECTOR XMVectorMergeXY(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		XMVECTOR_CONV Result;
		Result.u[0] = _V1.u[0];
		Result.u[1] = _V2.u[0];
		Result.u[2] = _V1.u[1];
		Result.u[3] = _V2.u[1];
		VERIFY_XMVECTOR(Result,4)
		return Result.vec;
#else
		return __vmrghw(V1, V2);
#endif
	}

	XINLINE XMVECTOR XMVectorScale(XMVECTOR V, __vecreg float ScaleFactor)
	{
#ifdef EMULATE_XENON_INTRINSICS
		if(!IsXenonNumberValid(ScaleFactor))snPause();
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V(V);
		Result.v[0] = _V.v[0] * ScaleFactor;
		Result.v[1] = _V.v[1] * ScaleFactor;
		Result.v[2] = _V.v[2] * ScaleFactor;
		Result.v[3] = _V.v[3] * ScaleFactor;
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
	#ifdef __SPU__
		XMVECTOR Scale = spu_splats(ScaleFactor);
	#else
		XMVECTOR Scale = __lvlx(&ScaleFactor, 0);
	#endif
		Scale = __vspltw(Scale, 0);
		return __vmulfp(V, Scale);
#endif
	}

	XINLINE XMVECTOR XMVectorReplicate(__vecreg float Value)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		Result.v[0] = Result.v[1] = Result.v[2] = Result.v[3] = Value;
		VERIFY_XMVECTOR(Result,4)
		return Result.vec;
#else
	#ifdef __SPU__
		return spu_splats(Value);
	#else
		XMVECTOR V;
		V = __lvlx(&Value, 0);
		V = __vspltw(V, 0);
		return V;
	#endif
#endif
	}

	XINLINE void XMStoreFloat4A(XMFLOAT4A* pDestination, XMVECTOR V)
	{
		XMStoreVector4A(&pDestination->x, V);
	}

	XINLINE XMVECTOR XMVectorAbs(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR NegativeV;
		XMVECTOR_CONV Result;
		NegativeV = XMVectorNegate(V);
		Result.vec = XMVectorMax(V, NegativeV);
		VERIFY_XMVECTOR(Result,3)
			return Result.vec;
#else
	#ifdef __SPU__
		return fabsf4(V);
	#else
		qword SignMask;
		SignMask = __vspltisw(-1);
		SignMask = __vslw(SignMask, SignMask);
		return __vandc(V, SignMask);
	#endif
#endif
	}

	XINLINE unsigned int XMVector4EqualR(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		unsigned int CR = 0;
		if (_V1.v[0] == _V2.v[0] && 
			_V1.v[1] == _V2.v[1] &&
			_V1.v[2] == _V2.v[2] &&
			_V1.v[3] == _V2.v[3])
			CR |= XM_CRMASK_CR6TRUE;
		else if (_V1.v[0] != _V2.v[0] && 
			_V1.v[1] != _V2.v[1] &&
			_V1.v[2] != _V2.v[2] &&
			_V1.v[3] != _V2.v[3])
			CR |= XM_CRMASK_CR6FALSE;
		return CR;
#else
		return vec_all_eq(V1, V2);
#endif
	}
	#define XMVector4Equal XMVector4EqualR

	XINLINE bool XMVector3Equal(XMVECTOR C1, XMVECTOR C2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV V1(C1);
		XMVECTOR_CONV V2(C2);
		if (V1.v[0] == V2.v[0] && 
			V1.v[1] == V2.v[1] &&
			V1.v[2] == V2.v[2])
			return true;
		return false;
#else
		XMVECTOR dummy = XMVectorSplatX(C1);
		XMVECTOR C1_4 = vec_perm(C1, dummy, _VECTORMATH_PERM_XYZA);
		XMVECTOR C2_4 = vec_perm(C2, dummy, _VECTORMATH_PERM_XYZA);
		return XMVector4Equal(C1_4, C2_4);
#endif
	}

	XINLINE bool XMVector4NotEqual(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		return XMComparisonAnyFalse(XMVector4EqualR(V1, V2));
#else
		return vec_all_ne(V1, V2);
#endif
	}

	XINLINE bool XMVector4Less(XMVECTOR C1, XMVECTOR C2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV V1(C1);
		XMVECTOR_CONV V2(C2);
		if (V1.v[0] > V2.v[0] && 
			V1.v[1] > V2.v[1] &&
			V1.v[2] > V2.v[2] &&
			V1.v[3] > V2.v[3])
			return false;
		return true;
#else
		return vec_all_gt(C2, C1);
#endif
	}

	XINLINE XMVECTOR XMVectorLess(XMVECTOR C1, XMVECTOR C2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Control;
		XMVECTOR_CONV _V1(C1);
		XMVECTOR_CONV _V2(C2);
		Control.u[0] = (_V1.u[0] < _V2.u[0]) ? 0xFFFFFFFF : 0;
		Control.u[1] = (_V1.u[1] < _V2.u[1]) ? 0xFFFFFFFF : 0;
		Control.u[2] = (_V1.u[2] < _V2.u[2]) ? 0xFFFFFFFF : 0;
		Control.u[3] = (_V1.u[3] < _V2.u[3]) ? 0xFFFFFFFF : 0;
		return Control.vec;
#else
		return (XMVECTOR)__vcmpgtfp(C2, C1);
#endif
	}

	XINLINE XMVECTOR XMVectorLessOrEqual(XMVECTOR C1, XMVECTOR C2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Control;
		XMVECTOR_CONV _V1(C1);
		XMVECTOR_CONV _V2(C2);
		Control.u[0] = (_V1.u[0] <= _V2.u[0]) ? 0xFFFFFFFF : 0;
		Control.u[1] = (_V1.u[1] <= _V2.u[1]) ? 0xFFFFFFFF : 0;
		Control.u[2] = (_V1.u[2] <= _V2.u[2]) ? 0xFFFFFFFF : 0;
		Control.u[3] = (_V1.u[3] <= _V2.u[3]) ? 0xFFFFFFFF : 0;
		return Control.vec;
#else
		return (XMVECTOR)__vcmpgefp(C2, C1);
#endif
	}

	XINLINE bool XMVector4Greater(XMVECTOR C1, XMVECTOR C2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV V1(C1);
		XMVECTOR_CONV V2(C2);
		if (V1.v[0] > V2.v[0] && 
			V1.v[1] > V2.v[1] &&
			V1.v[2] > V2.v[2] &&
			V1.v[3] > V2.v[3])
			return false;
		return true;
#else
		return vec_all_gt(C1, C2);
#endif
	}

	XINLINE bool XMVector3NearEqual(XMVECTOR C1, XMVECTOR C2, XMVECTOR Epsilon0)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV V1(C1);
		XMVECTOR_CONV V2(C2);
		XMVECTOR_CONV Epsilon(Epsilon0);
		XMVECTOR Delta0;
		Delta0 = XMVectorSubtract(V1.vec, V2.vec);
		Delta0 = XMVectorAbs(Delta0);
		XMVECTOR_CONV Delta(Delta0);
		return (Delta.v[0] <= Epsilon.v[0] && Delta.v[1] <= Epsilon.v[1] &&	Delta.v[2] <= Epsilon.v[2]);
#else
		XMVECTOR C1_4 = vec_perm(C1, Epsilon0, _VECTORMATH_PERM_XYZA);
		XMVECTOR C2_4 = vec_perm(C2, Epsilon0, _VECTORMATH_PERM_XYZA);		
		return XMVector4Less(XMVectorAbs(XMVectorSubtract(C1_4, C2_4)), Epsilon0);
#endif
	}

	XINLINE unsigned int XMVector3GreaterR(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		unsigned int CR = 0;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		if (_V1.v[0] > _V2.v[0] && _V1.v[1] > _V2.v[1] && _V1.v[2] > _V2.v[2])
			CR |= XM_CRMASK_CR6TRUE;
		else 
			if (_V1.v[0] <= _V2.v[0] && _V1.v[1] <= _V2.v[1] && _V1.v[2] < _V2.v[2])
				CR |= XM_CRMASK_CR6FALSE;
		return CR;
#else
		XMVECTOR dummyV1 = XMVectorSplatX(V1);
		XMVECTOR dummyV2 = XMVectorSplatX(V2);
		XMVECTOR V1_4 = vec_perm(V1, dummyV1, _VECTORMATH_PERM_XYZA);
		XMVECTOR V2_4 = vec_perm(V2, dummyV2, _VECTORMATH_PERM_XYZA);
		return XMVector4Greater(V1_4, V2_4);
#endif
	}


	XINLINE bool XMVector3Greater(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		return XMComparisonAllTrue(XMVector3GreaterR(V1, V2));
#else
		return XMVector3GreaterR(V1, V2);
#endif
	}

	XINLINE unsigned int XMVector2GreaterR(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		unsigned int CR = 0;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		if (_V1.v[0] > _V2.v[0] && _V1.v[1] > _V2.v[1])
			CR |= XM_CRMASK_CR6TRUE;
		else 
			if (_V1.v[0] <= _V2.v[0] && _V1.v[1] <= _V2.v[1])
				CR |= XM_CRMASK_CR6FALSE;
		return CR;
#else
		XMVECTOR dummyV1 = XMVectorSplatX(V1);
		XMVECTOR dummyV2 = XMVectorSplatX(V2);
		XMVECTOR V1_4 = vec_perm(V1, dummyV1, _VECTORMATH_PERM_XYAA);
		XMVECTOR V2_4 = vec_perm(V2, dummyV2, _VECTORMATH_PERM_XYAA);
		return XMVector4Less(V2_4, V1_4);
#endif
	}

	XINLINE bool XMVector2Greater(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		return XMComparisonAllTrue(XMVector2GreaterR(V1, V2));
#else
		return XMVector2GreaterR(V1, V2);
#endif
	}

	XINLINE bool XMVector2Less(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		return XMComparisonAllTrue(XMVector2GreaterR(V2, V1));
#else
		return XMVector2Greater(V2, V1);
#endif
	}


	XINLINE bool XMVector3Less(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		return XMComparisonAllTrue(XMVector2GreaterR(V2, V1));
#else
		return XMVector3Greater(V2, V1);
#endif
	}

	XINLINE bool XMVector3LessOrEqual(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		return XMComparisonAllTrue(XMVector2GreaterR(V2, V1));
#else
		return !XMVector3Greater(V1, V2);
#endif
	}

	XINLINE XMVECTOR XMVector3Dot(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		XMVECTOR_CONV Result;	Result.v[0] =	Result.v[1] =	Result.v[2] =	Result.v[3] = _V1.v[0] * _V2.v[0] + _V1.v[1] * _V2.v[1] + _V1.v[2] * _V2.v[2];
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		XMVECTOR mul	= vec_madd(V1, V2, XMVectorZero());
		XMVECTOR mulx = XMVectorSplatX(mul);
		XMVECTOR muly = XMVectorSplatY(mul);
		XMVECTOR mulz = XMVectorSplatZ(mul);
		return vec_add(vec_add(mulx, muly), mulz);
#endif
	}

	XINLINE XMVECTOR XMVector4Dot(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		XMVECTOR_CONV Result;	Result.v[0] = Result.v[1] =	Result.v[2] =	Result.v[3] = _V1.v[0] * _V2.v[0] + _V1.v[1] * _V2.v[1] + _V1.v[2] * _V2.v[2] + _V1.v[3] * _V2.v[3];
		VERIFY_XMVECTOR(Result,4)
		return Result.vec;
#else
		XMVECTOR mul	= vec_madd(V1, V2, XMVectorZero());
		XMVECTOR mulx = XMVectorSplatX(mul);
		XMVECTOR muly = XMVectorSplatY(mul);
		XMVECTOR mulz = XMVectorSplatZ(mul);
		XMVECTOR mulw = XMVectorSplatW(mul);
		return vec_add(vec_add(mulx, muly), vec_add(mulz, mulw));
#endif
	}

	XINLINE XMVECTOR XMVector3Cross(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		XMVECTOR_CONV Result;
    Result.v[0] = V1.v[1] * V2.v[2] - V1.v[2] * V2.v[1];
    Result.v[1] = V1.v[2] * V2.v[0] - V1.v[0] * V2.v[2];
    Result.v[2] = V1.v[0] * V2.v[1] - V1.v[1] * V2.v[0];
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		const vec_uchar16 perm_YZXY = (vec_uchar16)(XMVECTORI){_VECTORMATH_PERM_Y, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y};
		vec_uchar16 perm_ZXYY = vec_sld( perm_YZXY, perm_YZXY, 4 );
    XMVECTOR crossV1 = vec_perm(V1, V1, perm_YZXY);
    XMVECTOR crossV2 = vec_perm(V2, V2, perm_ZXYY);
    XMVECTOR crossR = __vmulfp(crossV1, crossV2);
    crossV1 = vec_perm(V1, V1, perm_ZXYY);
    crossV2 = vec_perm(V2, V2, perm_YZXY);
    crossR = __vnmsubfp(crossV1, crossV2, crossR);
		return crossR;
#endif
	}

	XINLINE XMVECTOR XMVector3LengthSq(XMVECTOR V)
	{
		return XMVector3Dot(V, V);
	}

	XINLINE XMVECTOR XMVectorReciprocalSqrtEst(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		unsigned int i;
		XMVECTOR_CONV _V(V);
		Result.v[0] = 0.0f;
		for (i = 0; i < 4; i++)
		{
/*			if (V.v[i] == 0.0f)
				Result.u[i] = 0x7F800000;
			else if (V.v[i] == -0.0f)
				Result.u[i] = 0xFF800000;
			else if (V.v[i] < 0.0f)
				Result.u[i] = 0x7FFFFFFF;
*/
			if(_V.v[i] == 0.0f || _V.v[i] == -0.0f || _V.v[i] < 0.0f)
			{
				snPause();
				Result.v[i] = 0.f;
			}
			else
			{
				Result.v[i] = 1.0f / sqrtf(_V.v[i]);
			}
		}
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		return __vrsqrtefp(V);
#endif
	}

	XINLINE XMVECTOR XMVectorReciprocalSqrt(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		return  XMVectorReciprocalSqrtEst(V);
#else
	#ifdef __SPU__
		return rsqrtf4(V);
	#else
		XMVECTOR ReciprocalSqrt;
		XMVECTOR OneHalfV;
		XMVECTOR Reciprocal;
		XMVECTOR Scale;
		Scale = (XMVECTOR)__vspltisw(1);
		ReciprocalSqrt = __vrsqrtefp(V);
		Scale = __vcfsx(Scale, 1);
		OneHalfV = __vmulfp(V, Scale);
		Reciprocal = __vmulfp(ReciprocalSqrt, ReciprocalSqrt);
		Scale = __vnmsubfp(OneHalfV, Reciprocal, Scale);
		return __vmaddfp(ReciprocalSqrt, Scale, ReciprocalSqrt);
	#endif
#endif
	}

	XINLINE XMVECTOR XMVector2Dot(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		XMVECTOR_CONV Result;	Result.v[0] =	Result.v[1] =	Result.v[2] =	Result.v[3] = _V1.v[0] * _V2.v[0] + _V1.v[1] * _V2.v[1];
		VERIFY_XMVECTOR(Result,2)
		return Result.vec;
#else
		XMVECTOR D1, D2, Zero;
		Zero = (XMVECTOR)__vspltisw(0);
		D1 = __vsldoi(Zero, V1, 2 << 2);
		D2 = __vsldoi(Zero, V2, 2 << 2);
		return __vmsum4fp(D1, D2);
#endif
	}

	XINLINE XMVECTOR XMVector2LengthSq(XMVECTOR V)
	{
		return XMVector2Dot(V, V);
	}

	XINLINE XMVECTOR XMVectorEqualInt(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Control;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		Control.u[0] = (_V1.u[0] == _V2.u[0]) ? 0xFFFFFFFF : 0;
		Control.u[1] = (_V1.u[1] == _V2.u[1]) ? 0xFFFFFFFF : 0;
		Control.u[2] = (_V1.u[2] == _V2.u[2]) ? 0xFFFFFFFF : 0;
		Control.u[3] = (_V1.u[3] == _V2.u[3]) ? 0xFFFFFFFF : 0;
		return Control.vec;
#else
		return __vcmpequw(V1, V2);
#endif
	}

	XINLINE XMVECTOR XMVectorSelect(XMVECTOR V1, XMVECTOR V2, XMVECTOR Control)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		XMVECTOR_CONV _Control(Control);
		Result.u[0] = (_V1.u[0] & ~_Control.u[0]) | (_V2.u[0] & _Control.u[0]);
		Result.u[1] = (_V1.u[1] & ~_Control.u[1]) | (_V2.u[1] & _Control.u[1]);
		Result.u[2] = (_V1.u[2] & ~_Control.u[2]) | (_V2.u[2] & _Control.u[2]);
		Result.u[3] = (_V1.u[3] & ~_Control.u[3]) | (_V2.u[3] & _Control.u[3]);
		return Result.vec;
#else
		return (XMVECTOR)__vsel(V1, V2, Control);
#endif
	}

	XINLINE XMVECTOR XMVectorEqual(XMVECTOR V1, XMVECTOR V2)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Control;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		Control.u[0] = (_V1.v[0] == _V2.v[0]) ? 0xFFFFFFFF : 0;
		Control.u[1] = (_V1.v[1] == _V2.v[1]) ? 0xFFFFFFFF : 0;
		Control.u[2] = (_V1.v[2] == _V2.v[2]) ? 0xFFFFFFFF : 0;
		Control.u[3] = (_V1.v[3] == _V2.v[3]) ? 0xFFFFFFFF : 0;
		return Control.vec;
#else
		return __vcmpeqfp(V1, V2);
#endif
	}

	XINLINE XMVECTOR XMVectorSqrt(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V(V);
		Result.x = sqrtf(_V.x);
		if(!IsXenonNumberValid(Result.x))snPause();
		Result.y = sqrtf(_V.y);
		Result.z = sqrtf(_V.z);
		Result.w = sqrtf(_V.w);
		return Result.vec;
#else
	#ifdef __SPU__
		return sqrtf4(V);
	#else
		XMVECTOR OneHalfV;
		XMVECTOR Reciprocal;
		XMVECTOR Scale;
		XMVECTOR RsqNotNaN;
		XMVECTOR ScaleNotNaN;
		vec_uint4 Select;
		XMVECTOR Result;
		Scale = (XMVECTOR)__vspltisw(1);
		Result = __vrsqrtefp(V);
		Scale = __vcfsx(Scale, 1);
		OneHalfV = __vmulfp(V, Scale);
		Reciprocal = __vmulfp(Result, Result);
		RsqNotNaN = __vcmpeqfp(Result, Result); // TRUE if V >= -0.0f
		Scale = __vnmsubfp(OneHalfV, Reciprocal, Scale);
		Result = __vmaddfp(Result, Scale, Result);
		ScaleNotNaN = __vcmpeqfp(Scale, Scale); // TRUE if V > 0.0f and V < +Infinity
		Select = (vec_uint4)__vxor(ScaleNotNaN, RsqNotNaN); // Select result if Scale is not NaN or V < -0.0f
		Result = __vmulfp(V, Result);
		Select = (vec_uint4)__vsel(Result, V, Select);
		return (XMVECTOR)(qword)Select;
	#endif
#endif
	}

	XINLINE XMVECTOR XMVectorSqrtEst(XMVECTOR V)
	{
		//Needs proper (faster) impl
		return XMVectorSqrt(V);
	}

	XINLINE XMVECTOR XMVector3Length(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		Result.vec = XMVector3LengthSq(V);
		Result.vec = XMVectorSqrt(Result.vec);
		return Result.vec;
#else
	#ifdef __SPU__
		return sqrtf4(VecDot3(V,V));
	#else
		XMVECTOR D;
		XMVECTOR Rsq;
		XMVECTOR Rcp;
		XMVECTOR RT;
		XMVECTOR Result;
		XMVECTOR Zero;
		XMVECTOR Length;
		XMVECTOR H;
		H = (XMVECTOR)__vspltisw(1);
		D = __vmsum3fp(V, V);
		H = __vcfsx(H, 1);
		Rsq = __vrsqrtefp(D);
		RT = __vmulfp(D, H);
		Rcp = __vmulfp(Rsq, Rsq);
		Zero = (XMVECTOR)__vspltisw(0);
		H = __vnmsubfp(RT, Rcp, H);
		Rsq = __vmaddfp(Rsq, H, Rsq);
		Result = (XMVECTOR)(qword)__vcmpeqfp(D, Zero);
		Length = __vmulfp(D, Rsq);
		return (XMVECTOR)__vsel(Length, D, Result);
	#endif
#endif
	}

	XINLINE XMVECTOR XMVector3LengthEst(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		Result.vec = XMVector3LengthSq(V);
		Result.vec = XMVectorSqrt(Result.vec);
		return Result.vec;
#else
		XMVECTOR D;
		XMVECTOR Rsq;
		XMVECTOR Result;
		XMVECTOR Zero;
		XMVECTOR Length;
		D = __vmsum3fp(V, V);
		Zero = (XMVECTOR)__vspltisw(0);
		Rsq = __vrsqrtefp(D);
		Result = (XMVECTOR)(qword)__vcmpeqfp(D, Zero);
		Length = __vmulfp(D, Rsq);
		return (XMVECTOR)__vsel(Length, D, Result);
#endif
	}

	XINLINE XMVECTOR XMVector2Length(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		Result.vec = XMVector2LengthSq(V);
		Result.vec = XMVectorSqrt(Result.vec);
		return Result.vec;
#else
	#ifdef __SPU__
		return sqrtf4(VecDot3(V,V));
	#else
		XMVECTOR D;
		XMVECTOR Rsq;
		XMVECTOR Rcp;
		XMVECTOR RT;
		XMVECTOR Result;
		XMVECTOR Zero;
		XMVECTOR Length;
		XMVECTOR H;
		Zero = (XMVECTOR)__vspltisw(0);
		D = __vsldoi(Zero, V, 2 << 2);
		H = (XMVECTOR)__vspltisw(1);
		D = __vmsum4fp(D, D);
		H = __vcfsx(H, 1);
		Rsq = __vrsqrtefp(D);
		RT = __vmulfp(D, H);
		Rcp = __vmulfp(Rsq, Rsq);
		H = __vnmsubfp(RT, Rcp, H);
		Rsq = __vmaddfp(Rsq, H, Rsq);
		Result = __vcmpeqfp(D, Zero);
		Length = __vmulfp(D, Rsq);
		return (XMVECTOR)__vsel(Length, D, Result);
	#endif
#endif
	}

	XINLINE XMVECTOR XMVectorMultiplyAdd(XMVECTOR V1, XMVECTOR V2, XMVECTOR V3)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		XMVECTOR_CONV _V1(V1);
		XMVECTOR_CONV _V2(V2);
		XMVECTOR_CONV _V3(V3);
		Result.v[0] = _V1.v[0] * _V2.v[0] + _V3.v[0];
		Result.v[1] = _V1.v[1] * _V2.v[1] + _V3.v[1];
		Result.v[2] = _V1.v[2] * _V2.v[2] + _V3.v[2];
		Result.v[3] = _V1.v[3] * _V2.v[3] + _V3.v[3];
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		return __vmaddfp(V1, V2, V3);
#endif
	}

	XINLINE XMVECTOR XMVector3Normalize(XMVECTOR V)
	{
		XMVECTOR_CONV Result;
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV v(V);
		float invLen = 1.f/ sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
		Result.x = v.x * invLen;
		Result.y = v.y * invLen;
		Result.z = v.z * invLen;
		VERIFY_XMVECTOR(Result,3)
		return Result.vec;
#else
		XMVECTOR dot = VecDot3(V, V);
		dot = vec_splat( dot, 0 );
		return vec_madd(V, rsqrtf4(dot), XMVectorZero());
#endif
	}

	XINLINE XMVECTOR XMVector3Orthogonal(XMVECTOR V)
	{
		static const XMVECTORF32 vNegativeEpsilon = {-0.001f, -0.001f, -0.001f, -0.001f};

		static const XMVECTORF32 vUp		= {0.0f, 0.0f, 1.0f, 0.0f};
		static const XMVECTORF32 vRight = {1.0f, 0.0f, 0.0f, 0.0f};

		XMVECTOR vCmp = XMVectorSubtract(XMVectorAbs(V), vUp);

		XMVECTOR vSelectControl = (XMVECTOR)__vcmpgtfp(vCmp, vNegativeEpsilon);

		//This will be 0xFFFFFFFF if V was aligned with vUp
		XMVECTOR vSelectControlZOnly = XMVectorSplatZ(vSelectControl);
	
		XMVECTOR vCrossTarget = XMVectorSelect(vUp, vRight, vSelectControlZOnly);

		return XMVector3Cross(vCrossTarget, V);
	}

	XINLINE XMVECTOR XMVector4LengthSq(XMVECTOR V)
	{
		return XMVector4Dot(V, V);
	}

	XINLINE XMVECTOR XMVector4Length(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		Result.vec = XMVector4LengthSq(V);
		Result.vec = XMVectorSqrt(Result.vec);
		return Result.vec;
#else
	#ifdef __SPU__
		return sqrtf4(VecDot4(V,V));
	#else
		XMVECTOR D;
		XMVECTOR Rsq;
		XMVECTOR Rcp;
		XMVECTOR RT;
		XMVECTOR Result;
		qword Zero;
		XMVECTOR Length;
		XMVECTOR H;
		H = (XMVECTOR)__vspltisw(1);
		D = __vmsum4fp(V, V);
		H = __vcfsx(H, 1);
		Rsq = __vrsqrtefp(D);
		RT = __vmulfp(D, H);
		Rcp = __vmulfp(Rsq, Rsq);
		Zero = __vspltisw(0);
		H = __vnmsubfp(RT, Rcp, H);
		Rsq = __vmaddfp(Rsq, H, Rsq);
		Result = __vcmpeqfp(D, (XMVECTOR)Zero);
		Length = __vmulfp(D, Rsq);
		return (XMVECTOR)__vsel(Length, D, Result);
	#endif
#endif
	}

	XINLINE XMVECTOR XMVector3NormalizeEst(XMVECTOR V)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Result;
		Result.vec = XMVector3LengthSq(V);
		Result.vec = XMVectorReciprocalSqrt(Result.vec);
		Result.vec = XMVectorMultiply(V, Result.vec);
		return Result.vec;
#else
		XMVECTOR D;
		XMVECTOR Rsq;
		D = __vmsum3fp(V, V);
		Rsq = __vrsqrtefp(D);
		return __vmulfp(V, Rsq);
#endif
	}

	XINLINE XMVECTOR XMVectorLerp(XMVECTOR V0, XMVECTOR V1, __vecreg float t)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMVECTOR_CONV Lerp;
		XMVECTOR_CONV Scale;
		XMVECTOR_CONV Length;
		Scale.vec		= XMVectorReplicate(t);
		Length.vec	= XMVectorSubtract(V1, V0);
		Lerp.vec		= XMVectorMultiplyAdd(Length.vec, Scale.vec, V0);
		return Lerp.vec;
#else
		XMVECTOR Lerp;
		XMVECTOR Length;
		Length = __vsubfp(V1, V0);
	#ifdef __SPU__
		Lerp = spu_splats(t);
	#else
		Lerp = __lvlx(&t, 0);
		Lerp = __vspltw(Lerp, 0);
	#endif
		return __vmaddcfp(Length, Lerp, V0);
#endif
	}

	XINLINE XMMATRIX XMMatrixTranspose(XMMATRIX M)
	{
#ifdef EMULATE_XENON_INTRINSICS
		XMMATRIX P;
		XMMATRIX MT;
		P.r[0] = XMVectorMergeXY(M.r[0], M.r[2]); // m00m20m01m21
		P.r[1] = XMVectorMergeXY(M.r[1], M.r[3]); // m10m30m11m31
		P.r[2] = XMVectorMergeZW(M.r[0], M.r[2]); // m02m22m03m23
		P.r[3] = XMVectorMergeZW(M.r[1], M.r[3]); // m12m32m13m33
		MT.r[0] = XMVectorMergeXY(P.r[0], P.r[1]); // m00m10m20m30
		MT.r[1] = XMVectorMergeZW(P.r[0], P.r[1]); // m01m11m21m31
		MT.r[2] = XMVectorMergeXY(P.r[2], P.r[3]); // m02m12m22m32
		MT.r[3] = XMVectorMergeZW(P.r[2], P.r[3]); // m03m13m23m33
		return MT;
#else
		XMMATRIX PMat;
		XMMATRIX MTMat;
		PMat.r[0] = __vmrghw(M.r[0], M.r[2]);
		PMat.r[1] = __vmrghw(M.r[1], M.r[3]);
		PMat.r[2] = __vmrglw(M.r[0], M.r[2]);
		PMat.r[3] = __vmrglw(M.r[1], M.r[3]);
		MTMat.r[0] = __vmrghw(PMat.r[0], PMat.r[1]);
		MTMat.r[1] = __vmrglw(PMat.r[0], PMat.r[1]);
		MTMat.r[2] = __vmrghw(PMat.r[2], PMat.r[3]);
		MTMat.r[3] = __vmrglw(PMat.r[2], PMat.r[3]);
		return MTMat;
#endif
	}

	XINLINE XMVECTOR XMVectorPermute(XMVECTOR V1, XMVECTOR V2, XMVECTOR Control)
	{
#ifdef EMULATE_XENON_INTRINSICS
		uint8* aByte[4];
		unsigned int VectorIndex;
		unsigned int ComponentIndex;
		unsigned int i;
		XMVECTOR_CONV Result;
		aByte[0] = (uint8*)&V1;
		aByte[1] = (uint8*)&V2;
		aByte[2] = (uint8*)&Control;
		aByte[3] = (uint8*)&Result;
		for (i = 0; i < 16; i++)
		{
			VectorIndex = aByte[2][i] / 16;
			ComponentIndex = aByte[2][i] % 16;
			aByte[3][i] = aByte[VectorIndex][ComponentIndex];
		}
		return Result.vec;
#else
		return (XMVECTOR)__vperm(V1, V2, Control);
#endif
	}


	XINLINE XMVECTOR XMVectorPermute(XMVECTOR V1, XMVECTOR V2, XMVECTORI Control)
	{
		return XMVectorPermute(V1, V2, (XMVECTOR)Control);
	}

	static XMMATRIX XMMatrixInverse(XMVECTOR* pDeterminant, XMMATRIX  M)
	{
#ifdef EMULATE_XENON_INTRINSICS
		float	tmp[12];
		XMMATRIX m = M;
		tmp[0] = m.m22 * m.m33;
		tmp[1] = m.m32 * m.m23;
		tmp[2] = m.m12 * m.m33;
		tmp[3] = m.m32 * m.m13;
		tmp[4] = m.m12 * m.m23;
		tmp[5] = m.m22 * m.m13;
		tmp[6] = m.m02 * m.m33;
		tmp[7] = m.m32 * m.m03;
		tmp[8] = m.m02 * m.m23;
		tmp[9] = m.m22 * m.m03;
		tmp[10]= m.m02 * m.m13;
		tmp[11]= m.m12 * m.m03;
		M.m00 = tmp[0]*m.m11 + tmp[3]*m.m21 + tmp[ 4]*m.m31;
		M.m00-= tmp[1]*m.m11 + tmp[2]*m.m21 + tmp[ 5]*m.m31;
		M.m01 = tmp[1]*m.m01 + tmp[6]*m.m21 + tmp[ 9]*m.m31;
		M.m01-= tmp[0]*m.m01 + tmp[7]*m.m21 + tmp[ 8]*m.m31;
		M.m02 = tmp[2]*m.m01 + tmp[7]*m.m11 + tmp[10]*m.m31;
		M.m02-= tmp[3]*m.m01 + tmp[6]*m.m11 + tmp[11]*m.m31;
		M.m03 = tmp[5]*m.m01 + tmp[8]*m.m11 + tmp[11]*m.m21;
		M.m03-= tmp[4]*m.m01 + tmp[9]*m.m11 + tmp[10]*m.m21;
		M.m10 = tmp[1]*m.m10 + tmp[2]*m.m20 + tmp[ 5]*m.m30;
		M.m10-= tmp[0]*m.m10 + tmp[3]*m.m20 + tmp[ 4]*m.m30;
		M.m11 = tmp[0]*m.m00 + tmp[7]*m.m20 + tmp[ 8]*m.m30;
		M.m11-= tmp[1]*m.m00 + tmp[6]*m.m20 + tmp[ 9]*m.m30;
		M.m12 = tmp[3]*m.m00 + tmp[6]*m.m10 + tmp[11]*m.m30;
		M.m12-= tmp[2]*m.m00 + tmp[7]*m.m10 + tmp[10]*m.m30;
		M.m13 = tmp[4]*m.m00 + tmp[9]*m.m10 + tmp[10]*m.m20;
		M.m13-= tmp[5]*m.m00 + tmp[8]*m.m10 + tmp[11]*m.m20;
		tmp[ 0] = m.m20*m.m31;
		tmp[ 1] = m.m30*m.m21;
		tmp[ 2] = m.m10*m.m31;
		tmp[ 3] = m.m30*m.m11;
		tmp[ 4] = m.m10*m.m21;
		tmp[ 5] = m.m20*m.m11;
		tmp[ 6] = m.m00*m.m31;
		tmp[ 7] = m.m30*m.m01;
		tmp[ 8] = m.m00*m.m21;
		tmp[ 9] = m.m20*m.m01;
		tmp[10] = m.m00*m.m11;
		tmp[11] = m.m10*m.m01;
		M.m20 = tmp[ 0]*m.m13 + tmp[ 3]*m.m23 + tmp[ 4]*m.m33;
		M.m20-= tmp[ 1]*m.m13 + tmp[ 2]*m.m23 + tmp[ 5]*m.m33;
		M.m21 = tmp[ 1]*m.m03 + tmp[ 6]*m.m23 + tmp[ 9]*m.m33;
		M.m21-= tmp[ 0]*m.m03 + tmp[ 7]*m.m23 + tmp[ 8]*m.m33;
		M.m22 = tmp[ 2]*m.m03 + tmp[ 7]*m.m13 + tmp[10]*m.m33;
		M.m22-= tmp[ 3]*m.m03 + tmp[ 6]*m.m13 + tmp[11]*m.m33;
		M.m23 = tmp[ 5]*m.m03 + tmp[ 8]*m.m13 + tmp[11]*m.m23;
		M.m23-= tmp[ 4]*m.m03 + tmp[ 9]*m.m13 + tmp[10]*m.m23;
		M.m30 = tmp[ 2]*m.m22 + tmp[ 5]*m.m32 + tmp[ 1]*m.m12;
		M.m30-= tmp[ 4]*m.m32 + tmp[ 0]*m.m12 + tmp[ 3]*m.m22;
		M.m31 = tmp[ 8]*m.m32 + tmp[ 0]*m.m02 + tmp[ 7]*m.m22;
		M.m31-= tmp[ 6]*m.m22 + tmp[ 9]*m.m32 + tmp[ 1]*m.m02;
		M.m32 = tmp[ 6]*m.m12 + tmp[11]*m.m32 + tmp[ 3]*m.m02;
		M.m32-= tmp[10]*m.m32 + tmp[ 2]*m.m02 + tmp[ 7]*m.m12;
		M.m33 = tmp[10]*m.m22 + tmp[ 4]*m.m02 + tmp[ 9]*m.m12;
		M.m33-= tmp[ 8]*m.m12 + tmp[11]*m.m22 + tmp[ 5]*m.m02;
		float det=(m.m00*M.m00+m.m10*M.m01+m.m20*M.m02+m.m30*M.m03);
		((XMVECTOR_CONV*)pDeterminant)->x = ((XMVECTOR_CONV*)pDeterminant)->y = ((XMVECTOR_CONV*)pDeterminant)->z = ((XMVECTOR_CONV*)pDeterminant)->w = det;
		float idet=(float)1.0/det;
		M.m00*=idet; M.m01*=idet; M.m02*=idet; M.m03*=idet;
		M.m10*=idet; M.m11*=idet; M.m12*=idet; M.m13*=idet;
		M.m20*=idet; M.m21*=idet; M.m22*=idet; M.m23*=idet;
		M.m30*=idet; M.m31*=idet; M.m32*=idet; M.m33*=idet;
		return M;
#else
		//implementation taken from mat_aos.h
		XMVECTOR in0, in1, in2, in3;
		XMVECTOR tmp0, tmp1, tmp2, tmp3;
		XMVECTOR cof0, cof1, cof2, cof3;
		XMVECTOR t0, t1, t2, t3;
		XMVECTOR t01, t02, t03, t12, t23;
		XMVECTOR t1r, t2r;
		XMVECTOR t01r, t02r, t03r, t12r, t23r;
		XMVECTOR t1r3, t1r3r;
		XMVECTOR det, det0, det1, det2, det3, invdet;
    XMVECTOR vzero = XMVectorZero();
		in0 = M.r[0];
		in1 = M.r[1];
		in2 = M.r[2];
		in3 = M.r[3];
		tmp0 = vec_perm(in0, in1, _VECTORMATH_PERM_XAZC);	/* A E C G */
		tmp1 = vec_perm(in2, in3, _VECTORMATH_PERM_XAZC);	/* I M K O */
		tmp2 = vec_perm(in0, in1, _VECTORMATH_PERM_YBWD);	/* B F D H */
		tmp3 = vec_perm(in2, in3, _VECTORMATH_PERM_YBWD);	/* J N L P */
		t0 = vec_perm(tmp0, tmp1, _VECTORMATH_PERM_XYAB);	/* A E I M */
		t1 = vec_perm(tmp3, tmp2, _VECTORMATH_PERM_XYAB);	/* J N B F */
		t2 = vec_perm(tmp0, tmp1, _VECTORMATH_PERM_ZWCD);	/* C G K O */
		t3 = vec_perm(tmp3, tmp2, _VECTORMATH_PERM_ZWCD);	/* L P D H */
		t23 = vec_madd(t2, t3, vzero);		/* CL GP KD OH */
		t23 = vec_perm(t23, t23, _VECTORMATH_PERM_YXWZ);	/* GP CL OH KD */
		cof0 = vec_nmsub(t1, t23, vzero);		/* -(JGP NCL FOH BKD) */
		cof1 = vec_nmsub(t0, t23, vzero);		/* -(AGP ECL IOH MKD) */
		t23r = vec_sld(t23, t23, 8);			/* OH KD GP CL */
		cof0 = vec_madd(t1, t23r, cof0);		/* JOH NKD BGP FCL + cof0 */
		cof1 = vec_madd(t0, t23r, cof1);		/* AOH EKD IGP MCL + cof1 */
		cof1 = vec_sld(cof1, cof1, 8);		/* IGP MCL AOH EKD - IOH MKD AGP ECL */
		t12 = vec_madd(t1, t2, vzero);		/* JC NG BK FO */
		t12 = vec_perm(t12, t12, _VECTORMATH_PERM_YXWZ);	/* NG JC FO BK */
		cof0 = vec_madd(t3, t12, cof0);		/* LNG PJC DFO HBK + cof0 */
		cof3 = vec_madd(t0, t12, vzero);		/* ANG EJC IFO MBK */
		t12r = vec_sld(t12, t12, 8);			/* FO BK NG JC */
		cof0 = vec_nmsub(t3, t12r, cof0);		/* cof0 - LFO PBK DNG HJC */
		cof3 = vec_nmsub(t0, t12r, cof3);		/* cof3 - AFO EBK ING MJC */
		cof3 = vec_sld(cof3, cof3, 8);		/* ING MJC AFO EBK - IFO MBK ANG EJC */
		t1r = vec_sld(t1, t1, 8);			/* B F J N */
		t2r = vec_sld(t2, t2, 8);			/* K O C G */
		t1r3 = vec_madd(t1r, t3, vzero);		/* BL FP JD NH */
		t1r3 = vec_perm(t1r3, t1r3, _VECTORMATH_PERM_YXWZ);	/* FP BL NH JD */
		cof0 = vec_madd(t2r, t1r3, cof0);		/* KFP OBL CNH GJD + cof0 */
		cof2 = vec_madd(t0, t1r3, vzero);		/* AFP EBL INH MJD */
		t1r3r = vec_sld(t1r3, t1r3, 8);		/* NH JD FP BL */
		cof0 = vec_nmsub(t2r, t1r3r, cof0);		/* cof0 - KNH OJD CFP GBL */
		cof2 = vec_nmsub(t0, t1r3r, cof2);		/* cof2 - ANH EJD IFP MBL */
		cof2 = vec_sld(cof2, cof2, 8);		/* IFP MBL ANH EJD - INH MJD AFP EBL */
		t01 = vec_madd(t0, t1, vzero);		/* AJ EN IB MF */
		t01 = vec_perm(t01, t01, _VECTORMATH_PERM_YXWZ);	/* EN AJ MF IB */
		cof2 = vec_nmsub(t3, t01, cof2);		/* cof2 - LEN PAJ DMF HIB */
		cof3 = vec_madd(t2r, t01, cof3);		/* KEN OAJ CMF GIB + cof3 */ 
		t01r = vec_sld(t01, t01, 8);			/* MF IB EN AJ */
		cof2 = vec_madd(t3, t01r, cof2);		/* LMF PIB DEN HAJ + cof2 */
		cof3 = vec_nmsub(t2r, t01r, cof3);		/* cof3 - KMF OIB CEN GAJ */
		t03 = vec_madd(t0, t3, vzero);		/* AL EP ID MH */
		t03 = vec_perm(t03, t03, _VECTORMATH_PERM_YXWZ);	/* EP AL MH ID */
		cof1 = vec_nmsub(t2r, t03, cof1);		/* cof1 - KEP OAL CMH GID */
		cof2 = vec_madd(t1, t03, cof2);		/* JEP NAL BMH FID + cof2 */
		t03r = vec_sld(t03, t03, 8);			/* MH ID EP AL */
		cof1 = vec_madd(t2r, t03r, cof1);		/* KMH OID CEP GAL + cof1 */
		cof2 = vec_nmsub(t1, t03r, cof2);		/* cof2 - JMH NID BEP FAL */ 
		t02 = vec_madd(t0, t2r, vzero);		/* AK EO IC MG */
		t02 = vec_perm(t02, t02, _VECTORMATH_PERM_YXWZ);	/* E0 AK MG IC */
		cof1 = vec_madd(t3, t02, cof1);		/* LEO PAK DMG HIC + cof1 */
		cof3 = vec_nmsub(t1, t02, cof3);		/* cof3 - JEO NAK BMG FIC */
		t02r = vec_sld(t02, t02, 8);			/* MG IC EO AK */
		cof1 = vec_nmsub(t3, t02r, cof1);		/* cof1 - LMG PIC DEO HAK */
		cof3 = vec_madd(t1, t02r, cof3);		/* JMG NIC BEO FAK + cof3 */
		det  = vec_madd(t0, cof0, vzero);
		det0 = vec_splat(det, 0);
		det1 = vec_splat(det, 1);
		det2 = vec_splat(det, 2);
		det3 = vec_splat(det, 3);
		det  = vec_add(det0, det1);
		det2 = vec_add(det2, det3);
		det  = vec_add(det, det2);
		invdet = recipf4(det);
		XMMATRIX ret;
		ret.r[0] = vec_madd(cof0, invdet, vzero);
		ret.r[1] = vec_madd(cof1, invdet, vzero);
		ret.r[2] = vec_madd(cof2, invdet, vzero);
		ret.r[3] = vec_madd(cof3, invdet, vzero);
		return ret;
#endif
	}

/*#ifdef EMULATE_XENON_INTRINSICS
	XINLINE XMVECTOR operator *(const XMVECTOR& V0, const XMVECTOR& V1)
	{
		return XMVectorMultiply(V0, V1);
	}

	XINLINE XMVECTOR operator *(const XMVECTOR& V0, const float S)
	{
		return XMVectorScale(V0, S);
	}

	XINLINE XMVECTOR operator -(const XMVECTOR& V0, const XMVECTOR& V1)
	{
		return XMVectorSubtract(V0, V1);
	}

	XINLINE XMVECTOR operator +(const XMVECTOR& V0, const XMVECTOR& V1)
	{
		return XMVectorAdd(V0, V1);
	}
#endif
*/

	XINLINE XMVECTOR XMVectorRound(XMVECTOR V)
	{
		return __vrfin(V);
	}

	XINLINE XMVECTOR XMVectorSet(float x, float y, float z, float w)
	{
		XMVECTOR_CONV c;
		c.x = x;	c.y = y;	c.z = z;	c.w = w;
		return c.vec;
	}

	//--------------------------------_XMUBYTE4 funcs----------------------------------------------

	typedef struct _XMUBYTE4
	{
		union
		{
			struct
			{
				uint8 x;
				uint8 y;
				uint8 z;
				uint8 w;
			};
			unsigned int v;
		};
		ILINE _XMUBYTE4() {};
		ILINE _XMUBYTE4(uint8 _x, uint8 _y, uint8 _z, uint8 _w) : x(_x),y(_y),z(_z),w(_w){}
		ILINE _XMUBYTE4(unsigned int _v) : v(_v){}
		ILINE _XMUBYTE4(const uint8 *pArray) : x(pArray[0]),y(pArray[1]),z(pArray[2]),w(pArray[3]){}
		ILINE _XMUBYTE4(float _x, float _y, float _z, float _w);
		ILINE _XMUBYTE4(const float *pArray);
		ILINE _XMUBYTE4& operator= (const _XMUBYTE4& UByte4){v=UByte4.v;return *this;}
	} XMUBYTE4;

	XINLINE XMVECTOR XMLoadUByte4(const XMUBYTE4* pSource)
	{
		XMVECTOR_CONV N;
		N.u[0] = pSource->x;
		N.u[1] = pSource->y;
		N.u[2] = pSource->z;
		N.u[3] = pSource->w;
		return __vcfux(N.vec, 0);
	}

	XINLINE void XMStoreUByte4(XMUBYTE4* pDestination, XMVECTOR V)
	{
		XMVECTOR_CONV N;
		const XMVECTOR MaxVec = {255.0f, 255.0f, 255.0f, 255.0f};
		N.vec = XMVectorClamp(V, XMVectorZero(), MaxVec);
		N.vec = XMVectorRound(N.vec);
		N.vec = (vec_float4)(qword)vec_ctu(N.vec, 0);//convert float to signed int(clamped before)
		pDestination->x = (uint8)N.u[0];
		pDestination->y = (uint8)N.u[1];
		pDestination->z = (uint8)N.u[2];
		pDestination->w = (uint8)N.u[3];
	}

	ILINE XMUBYTE4::_XMUBYTE4(float _x, float _y, float _z, float _w)
	{
		XMStoreUByte4(this, XMVectorSet(_x, _y, _z, _w));
	}

	ILINE XMUBYTE4::_XMUBYTE4(const float *pArray)
	{
		XMStoreUByte4(this, XMLoadFloat4((XMFLOAT4*)pArray));
	}

	#define __vector4 vec_float4

	// On PS3 these casts are necessary, on Xenon they cause errors
	// See Cry_Xenon_Math.h for the corresponding implementations
	#define XMCAST_VEC_FLOAT(a) (XMVECTOR)(a)
	#define XMCAST_VEC_UINT(a) (XMVECTORI)(a)
	#define XMCAST_VEC_UCHAR(a) (vec_uchar16)(a)

#endif//XENON_INTRINSICS
#endif//CRY_PS3_MATH
#endif//PS3OPT
#endif//PS3
