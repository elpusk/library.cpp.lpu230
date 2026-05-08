// dllmain.h : 모듈 클래스의 선언입니다.

class COposLpu230LockModule : public ATL::CAtlDllModuleT< COposLpu230LockModule >
{
public :
	DECLARE_LIBID(LIBID_OposLpu230LockLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_OPOSLPU230LOCK, "{499E5C6B-4135-417F-A7D0-F67FF554F421}")
};

extern class COposLpu230LockModule _AtlModule;
