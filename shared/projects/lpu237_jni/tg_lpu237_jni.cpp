// tg_lpu237_jni.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "tg_lpu237_jni.h"
#include "Lpu237Dll.h"
#include <vector>
#include "JavaInfo.h"

using namespace std;

/**
* local data type
*/

typedef	struct{
	JNIEnv *env;
	jobject obj;
}type_java_data;

/**
* local function prototype
*/
static void WINAPI _call_java_callback(void *);

static HANDLE _get_dev_handle( HANDLE h_dev = NULL );
static type_java_data *_get_java_data( bool b_reset, JNIEnv *env = NULL, jobject obj = NULL );
static DWORD _get_buffer_index( DWORD dw_new_index = -1 );
static jvalue JNU_CallMethodByName( 	JNIEnv *env, jboolean *hasException, jobject obj, const char *name, const char *descriptor, ...);
/**
* local function body
*/

jvalue JNU_CallMethodByName( 	JNIEnv *env, jboolean *hasException, jobject obj, const char *name, const char *descriptor, ...)
{
	va_list args;
	jclass clazz = NULL;
	jmethodID mid = NULL;
	jvalue result = {0,};
	if (env->EnsureLocalCapacity(2) == JNI_OK) {
		clazz = env->GetObjectClass(obj);
		mid = env->GetMethodID( clazz, name,	descriptor);
		if (mid) {
			const char *p = descriptor;
			/* skip over argument types to find out the return type */
			while (*p != ')') p++;
			/* skip ')' */
			p++;
			va_start(args, descriptor);
			switch (*p) {
				case 'V':
					env->CallVoidMethodV( obj, mid, args);
					break;
				case '[':
				case 'L':
					result.l = env->CallObjectMethodV( obj, mid, args);
					break;
				case 'Z':
					result.z = env->CallBooleanMethodV(obj, mid, args);
					break;
				case 'B':
					result.b = env->CallByteMethodV( obj, mid, args);
					break;
				case 'C':
					result.c = env->CallCharMethodV( obj, mid, args);
					break;
				case 'S':
					result.s = env->CallShortMethodV( obj, mid, args);
					break;
				case 'I':
					result.i = env->CallIntMethodV( obj, mid, args);
					break;
				case 'J':
					result.j = env->CallLongMethodV( obj, mid, args);
					break;
				case 'F':
					result.f = env->CallFloatMethodV( obj, mid, args);
					break;
				case 'D':
					result.d = env->CallDoubleMethodV( obj, mid, args);
					break;
				default:
					env->FatalError("illegal descriptor");
			}//end switch
			va_end(args);
		}
		env->DeleteLocalRef(clazz);
	}
	if (hasException) {
		*hasException = env->ExceptionCheck();
	}
	return result;
}

#define	_JBYTE_ARRAY_ID_LEN		0
#define	_JBYTE_ARRAY_ID_ISO1_BUF		1
#define	_JBYTE_ARRAY_ID_ISO2_BUF		2
#define	_JBYTE_ARRAY_ID_ISO3_BUF		3

void WINAPI _call_java_callback(void *p_data)
{
	do{
		DWORD dw_index = _get_buffer_index();
		if( dw_index == -1 )
			continue;
		//
		DWORD dw_result = LPU237_DLL_RESULT_ERROR;
		jbyte iso[3][120] = {0,};
		jbyte c_iso_len[3] = { 0, };

		for( DWORD i=0; i<3; i++ ){
			dw_result = CLpu237Dll::get_instance()->LPU237_get_data( dw_index, i+1,(unsigned char*)iso[i] );
			if( dw_result == LPU237_DLL_RESULT_ERROR )
				break;
			else if( dw_result == LPU237_DLL_RESULT_ERROR_MSR  )
				break;
			else if( dw_result == LPU237_DLL_RESULT_CANCEL )
				break;
			else{
				c_iso_len[i] = (jbyte)dw_result;
			}
		}//end for

		//
		CJavaInfo javainfo( "kr/co/elpusk/javapos/msr/Lpu237MSRService" );

		if( !javainfo.new_jbyteArray( _JBYTE_ARRAY_ID_LEN, 3 ) )
			continue;
		if( !javainfo.jbyteArray_SetByteArrayRegion( _JBYTE_ARRAY_ID_LEN, 0, 3, c_iso_len ) )
			continue;
		//
		bool b_error = false;

		for( int i = _JBYTE_ARRAY_ID_ISO1_BUF; i<=_JBYTE_ARRAY_ID_ISO3_BUF; i++ ){
			if( !javainfo.new_jbyteArray( i, 120 ) ){
				b_error = true;
				break;//exit for
			}
			if( c_iso_len[i-1] > 0 )
				javainfo.jbyteArray_SetByteArrayRegion( i, 0, c_iso_len[i-1], iso[i-1] );
		}//end for

		if( b_error )
			continue;

		//length
		if( !javainfo.SetStaticByteArrayField( "out_size", _JBYTE_ARRAY_ID_LEN ) )
			continue;
		//ISO1
		if( !javainfo.SetStaticByteArrayField( "out_iso1", _JBYTE_ARRAY_ID_ISO1_BUF ) )
			continue;
		//ISO2
		if( !javainfo.SetStaticByteArrayField( "out_iso2", _JBYTE_ARRAY_ID_ISO2_BUF ) )
			continue;
		//ISO3
		if( !javainfo.SetStaticByteArrayField( "out_iso3", _JBYTE_ARRAY_ID_ISO3_BUF ) )
			continue;

		if( !javainfo.call_CallStaticVoidVoidMethod( "lpu237CallbackReadDone" ) )
			continue;
		//
	}while(0);
}

HANDLE _get_dev_handle( HANDLE h_dev /*= NULL*/ )
{
	static HANDLE h_msr = NULL;

	if( h_dev != NULL && h_dev != INVALID_HANDLE_VALUE ){
		h_msr = h_dev;
	}

	return h_msr;
}

type_java_data *_get_java_data( bool b_reset, JNIEnv *env /*= NULL*/, jobject obj /*= NULL*/ )
{
	static type_java_data java_data = {0,};

	do{
		if( b_reset ){
			if( java_data.obj )
				java_data.env->DeleteWeakGlobalRef( java_data.obj);
			//
			java_data.env = NULL;
			java_data.obj = NULL;

			continue;
		}

		if( env != NULL && obj != NULL ){
			if( java_data.obj )
				java_data.env->DeleteWeakGlobalRef( java_data.obj);

			java_data.env = env;
			java_data.obj = env->NewWeakGlobalRef(obj);
		}
	}while(0);

	return &java_data;
}

DWORD _get_buffer_index( DWORD dw_new_index /*= -1*/ )
{
	static DWORD dw_index = -1;

	if( dw_new_index != -1 )
		dw_index = dw_new_index;
	//
	return dw_index;
}
/**
* exported function body
*/


///////////////////////////////////
/*
 * Class:     kr_co_elpusk_javapos_msr_Lpu237MSRService
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_kr_co_elpusk_javapos_msr_Lpu237MSRService_initIDs
  (JNIEnv *env, jclass cls)
{
}


/*
 * Class:     kr_co_elpusk_javapos_msr_Lpu237Jni
 * Method:    lpu237_ini
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_kr_co_elpusk_javapos_msr_Lpu237MSRService_lpu237_1ini
  (JNIEnv *env, jobject obj)
{
	jboolean jresult = JNI_TRUE;
	do{
		//load dll
		CLpu237Dll *p_dll = CLpu237Dll::get_instance(  L".\\tg_lpu237_dll.dll" );
	}while(0);

	return jresult;
}

/*
 * Class:     kr_co_elpusk_javapos_msr_Lpu237MSRService
 * Method:    lpu237_end
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_kr_co_elpusk_javapos_msr_Lpu237MSRService_lpu237_1end
  (JNIEnv *env, jobject obj)
{
	jboolean jresult = JNI_TRUE;
	do{
		//load dll
		CLpu237Dll::get_instance()->Unload();
	}while(0);

	return jresult;
}


/*
 * Class:     kr_co_elpusk_javapos_msr_Lpu237Jni
 * Method:    lpu237_enable_read
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_kr_co_elpusk_javapos_msr_Lpu237MSRService_lpu237_1enable_1read
  (JNIEnv *env, jobject obj, jboolean b_enable)
{
	do{
		HANDLE h_dev = _get_dev_handle();

		if( h_dev == NULL || h_dev == INVALID_HANDLE_VALUE )
			continue;

		DWORD dw_result = LPU237_DLL_RESULT_SUCCESS;
	
		if( b_enable ){
			dw_result = CLpu237Dll::get_instance()->LPU237_enable( h_dev );
			if( dw_result != LPU237_DLL_RESULT_SUCCESS )
				continue;
		}
		else{
			_get_buffer_index( -1 ); // reset buffer index.

			CLpu237Dll::get_instance()->LPU237_disable(h_dev);
			if( dw_result != LPU237_DLL_RESULT_SUCCESS )
				continue;
		}
	}while(0);
}

/*
 * Class:     kr_co_elpusk_javapos_msr_Lpu237MSRService
 * Method:    lpu237_wait_read
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_kr_co_elpusk_javapos_msr_Lpu237MSRService_lpu237_1wait_1read
  (JNIEnv *env, jobject obj)
{
	do{
		HANDLE h_dev = _get_dev_handle();

		if( h_dev == NULL || h_dev == INVALID_HANDLE_VALUE )
			continue;

		DWORD dw_result = LPU237_DLL_RESULT_SUCCESS;
	
		//_get_java_data( false,env, obj ); //save java data.
		dw_result = CLpu237Dll::get_instance()->LPU237_wait_swipe_with_callback( h_dev, _call_java_callback, NULL );
		if( dw_result == LPU237_DLL_RESULT_ERROR )
			continue;
		//
		_get_buffer_index( dw_result );//save result buffer index.
	}while(0);
}

/*
 * Class:     kr_co_elpusk_javapos_msr_Lpu237Jni
 * Method:    lpu237_open
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_kr_co_elpusk_javapos_msr_Lpu237MSRService_lpu237_1open
  (JNIEnv *env, jobject obj)
{
	jboolean b_jresult = JNI_FALSE;

	do{
		TCHAR s_path[_MAX_PATH] = {0,};

		DWORD dw_result = CLpu237Dll::get_instance()->LPU237_get_list( (LPTSTR)s_path );
		if( dw_result == LPU237_DLL_RESULT_ERROR )
			continue;
		//
		HANDLE h_dev = CLpu237Dll::get_instance()->LPU237_open( s_path );
		if( h_dev == NULL || h_dev == INVALID_HANDLE_VALUE )
			continue;
		//
		_get_dev_handle( h_dev );//save handle

		b_jresult = JNI_TRUE;
	}while(0);
	return b_jresult;
}

/*
 * Class:     kr_co_elpusk_javapos_msr_Lpu237Jni
 * Method:    lpu237_close
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_kr_co_elpusk_javapos_msr_Lpu237MSRService_lpu237_1close
  (JNIEnv *env, jobject obj)
{
	jboolean b_jresult = JNI_TRUE;

	do{
		HANDLE h_dev = _get_dev_handle();
		if( h_dev == NULL || h_dev == INVALID_HANDLE_VALUE )
			continue;
		
		//_get_java_data(true);

		DWORD dw_result = CLpu237Dll::get_instance()->LPU237_close( h_dev );
		if( dw_result == LPU237_DLL_RESULT_ERROR )
			continue;

		_get_dev_handle( 0 );
	}while(0);

	return b_jresult;
}

/*
 * Class:     kr_co_elpusk_javapos_msr_Lpu237MSRService
 * Method:    lpu237_cancel_wait
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_kr_co_elpusk_javapos_msr_Lpu237MSRService_lpu237_1cancel_1wait
  (JNIEnv *env, jobject obj)
{
	do{
		HANDLE h_dev = _get_dev_handle();
		if( h_dev == NULL || h_dev == INVALID_HANDLE_VALUE )
			continue;

		CLpu237Dll::get_instance()->LPU237_cancel_wait_swipe( h_dev );
	}while(0);
}