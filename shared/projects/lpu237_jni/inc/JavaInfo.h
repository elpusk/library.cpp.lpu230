#pragma once

#include "tg_lpu237_jni.h"
#include <map>

using namespace std;

class CJavaInfo
{
public:
	typedef	map<int, jbyteArray>	type_map_jbyteArray;

public:
	CJavaInfo(const char *s_class_name)
	{
		do{
			 m_pp_jvm = NULL;
			 m_p_env = NULL;
			 m_p_cls = NULL;
			 //
			jsize n_vm = 0;
			JNI_GetCreatedJavaVMs( NULL, 0, &n_vm );
			if( n_vm<=0 )
				continue;
			//
			m_v_java_vm.resize( n_vm, 0 );
			
			m_pp_jvm = &m_v_java_vm[0];
			JNI_GetCreatedJavaVMs( m_pp_jvm,n_vm, &n_vm );

			if( (*m_pp_jvm)->AttachCurrentThread( (void**)&m_p_env, NULL ) != 0 )
				continue;

			if( s_class_name == NULL )
				continue;
			m_p_cls = m_p_env->FindClass( s_class_name );
			//m_p_cls = m_p_env->FindClass( "kr/co/elpusk/javapos/msr/Lpu237MSRService" );
			if( m_p_cls == NULL )
				continue;
			//
			jfieldID fid = m_p_env->GetStaticFieldID( m_p_cls, "n_uid","I" );
			if( fid == NULL )
				continue;
		}while(0);
	}

	~CJavaInfo(void)
	{
		do{
			if( m_p_env == NULL )
				continue;
			//
			type_map_jbyteArray::iterator it = begin(m_map_jbyteArray);

			for( ; it != end(m_map_jbyteArray); it++ ){
				if( it->second )
					m_p_env->DeleteLocalRef(  it->second  );
			}//end for


			if( m_p_cls ){
				m_p_env->DeleteLocalRef(m_p_cls);
			}

			(*m_pp_jvm)->DetachCurrentThread();
		}while(0);
	}

	JNIEnv *get_env()
	{
		return m_p_env;
	}

	jclass get_class()
	{
		return m_p_cls;
	}

	bool new_jbyteArray( int n_key, jsize n_size )
	{
		jbyteArray p_array(NULL);
		bool b_result(false);

		do{
			if( m_p_env == NULL )
				continue;
			if( n_size<=0 )
				continue;
			if( m_map_jbyteArray.find( n_key ) != end(m_map_jbyteArray) )
				continue;
			//
			p_array = m_p_env->NewByteArray( n_size );
			if( p_array == NULL )
				continue;
			//
			m_map_jbyteArray[n_key] = p_array;
			b_result = true;
		}while(0);

		return b_result;
	}

	jbyteArray get_jbyteArray( int n_key )
	{
		jbyteArray p_array(NULL);
		do{
			if( m_p_env == NULL )
				continue;

			type_map_jbyteArray::iterator it = m_map_jbyteArray.find( n_key );
			if( it == end(m_map_jbyteArray) )
				continue;
			//
			p_array = it->second;
		}while(0);

		return p_array;
	}

	bool delete_jbyteArray( int n_key )
	{
		bool b_result(false);

		do{
			if( m_p_env == NULL )
				continue;

			type_map_jbyteArray::iterator it = m_map_jbyteArray.find( n_key );
			if( it == end(m_map_jbyteArray) )
				continue;
			//
			m_p_env->DeleteLocalRef( it->second );

			m_map_jbyteArray.erase(it);
			b_result = true;
		}while(0);

		return b_result;
	}

	bool jbyteArray_SetByteArrayRegion(  int n_key, jsize n_start, jsize n_len, const jbyte *s_src )
	{
		bool b_result(false);

		do{
			if( m_p_env == NULL )
				continue;
			if( n_start < 0 )
				continue;
			if( n_len <= 0 )
				continue;
			if( s_src == NULL )
				continue;

			type_map_jbyteArray::iterator it = m_map_jbyteArray.find( n_key );
			if( it == end(m_map_jbyteArray) )
				continue;
			//
			m_p_env->SetByteArrayRegion( it->second, n_start, n_len, s_src );
			//
			b_result = true;
		}while(0);

		return b_result;
	}

	bool call_CallStaticVoidVoidMethod( const char *name_method )
	{
		bool b_result(false);

		do{
			if( m_p_env == NULL )
				continue;
			if( m_p_cls == NULL )
				continue;
			if( name_method == NULL )
				continue;
			//
			jmethodID mid = m_p_env->GetStaticMethodID( m_p_cls, name_method, "()V");
			if( mid == NULL )
				continue;
			m_p_env->CallStaticVoidMethod( m_p_cls, mid );

			jthrowable exc = m_p_env->ExceptionOccurred();
			if( exc ){
				jclass newExcCls;
				m_p_env->ExceptionDescribe();
				m_p_env->ExceptionClear();
				newExcCls = m_p_env->FindClass( "java/lang/IllegalArgumentException" );
				if (newExcCls == NULL) {
					// Unable to find the exception class, give up.
					continue;
				}
				m_p_env->ThrowNew( newExcCls, "thrown from C code");
				continue;
			}

			b_result = true;
		}while(0);

		return b_result;
	}

	bool SetStaticByteArrayField( const char *name_fied, int n_key )
	{
		bool b_result(false);

		do{
			if( m_p_env == NULL )
				continue;
			if( m_p_cls == NULL )
				continue;
			if( name_fied == NULL )
				continue;

			jbyteArray p_array = get_jbyteArray( n_key );
			if( p_array == NULL )
				continue;
			//
			jfieldID fid = m_p_env->GetStaticFieldID( m_p_cls, name_fied, "[B");
			if( fid == NULL )
				continue;
			m_p_env->SetStaticObjectField( m_p_cls, fid, p_array );

			b_result = true;
		}while(0);

		return b_result;
	}

private:
	vector<JavaVM*> m_v_java_vm;
	JavaVM **m_pp_jvm;
	JNIEnv *m_p_env;
	jclass m_p_cls;

	type_map_jbyteArray m_map_jbyteArray;
private:
	//don't call this methods.
	CJavaInfo(void);
	CJavaInfo( const CJavaInfo & );
	CJavaInfo operator= ( const CJavaInfo & );

};

