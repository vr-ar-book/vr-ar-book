
#include "jni_interface.h"

#include <jni.h>

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------
#define JNI_CURRENT_VERSION JNI_VERSION_1_6

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------
namespace
{
    class CJNIInterface
    {
    public:

        static CJNIInterface& GetInstance()
        {
            static CJNIInterface s_Singleton;
            return s_Singleton;
        }

    public:

        void InitializeJNI(JavaVM* _pJavaVM, jint _Version);

        void FindClassesAndMethods();

        JNIEnv* GetJavaEnvironment();

        void SetActivity(jobject _Activity);

        void SetContext(jobject _pContext);
        jobject GetContext();

        int GetDeviceRotation();

        bool CheckPermission(const std::string& _rPermission);

        void AcquirePermissions(const std::string* _pPermissions, unsigned int _NumberOfPermissions);

    public:

        CJNIInterface();
        ~CJNIInterface();

    private:

        jobject m_pContext;
        JavaVM* m_pCurrentJavaVM;
        jint m_CurrentJavaVersion;
        jobject m_GameActivityThiz;
        jclass m_GameActivityID;
        jclass m_PermissionHelperID;
        jclass m_JavaStringClass;
        jobject m_GlobalClassLoader;
        jmethodID m_FindClassMethod;
        jmethodID m_GetDeviceRotationMethod;
        jmethodID m_CheckPermissionMethod;
        jmethodID m_AcquirePermissionMethod;
    };
}

namespace
{
    CJNIInterface::CJNIInterface()
        : m_pContext                      (0)
        , m_pCurrentJavaVM                (0)
        , m_CurrentJavaVersion            (0)
        , m_GameActivityThiz              (0)
        , m_GameActivityID                (0)
        , m_PermissionHelperID            (0)
        , m_JavaStringClass               (0)
        , m_GlobalClassLoader             (0)
        , m_FindClassMethod               (0)
        , m_GetDeviceRotationMethod       (0)
    {

    };

    // -----------------------------------------------------------------------------

    CJNIInterface::~CJNIInterface()
    {

    };

    // -----------------------------------------------------------------------------

    void CJNIInterface::InitializeJNI(JavaVM* _pJavaVM, jint _Version)
    {
        if (m_pCurrentJavaVM == 0)
        {
            m_pCurrentJavaVM     = _pJavaVM;
            m_CurrentJavaVersion = _Version;

            JNIEnv* pEnvironment = GetJavaEnvironment();

            jclass MainClass        = pEnvironment->FindClass("com/example/native_activity/GameActivity");
            jclass ClassClass       = pEnvironment->FindClass("java/lang/Class");
            jclass ClassLoaderClass = pEnvironment->FindClass("java/lang/ClassLoader");

            jmethodID GetClassLoaderMethod = pEnvironment->GetMethodID(ClassClass, "getClassLoader", "()Ljava/lang/ClassLoader;");

            jobject LocalClassLoader = pEnvironment->CallObjectMethod(MainClass, GetClassLoaderMethod);

            m_GlobalClassLoader = pEnvironment->NewGlobalRef(LocalClassLoader);
            m_FindClassMethod   = pEnvironment->GetMethodID(ClassLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");

            m_PermissionHelperID = (jclass)pEnvironment->NewGlobalRef(pEnvironment->FindClass("com/example/native_activity/PermissionHelper"));

            m_CheckPermissionMethod = pEnvironment->GetStaticMethodID(m_PermissionHelperID, "CheckPermission", "(Ljava/lang/String;)Z");

            m_AcquirePermissionMethod = pEnvironment->GetStaticMethodID(m_PermissionHelperID, "AcquirePermissions", "([Ljava/lang/String;)V");

            jclass LocalStringClass = pEnvironment->FindClass("java/lang/String");

            m_JavaStringClass = (jclass)pEnvironment->NewGlobalRef(LocalStringClass);
        }
    }

    // -----------------------------------------------------------------------------

    void CJNIInterface::FindClassesAndMethods()
    {
        JNIEnv* pEnvironment = GetJavaEnvironment();

        jclass LocalGameActivityClass = pEnvironment->FindClass("com/example/native_activity/GameActivity");

        m_GameActivityID = (jclass)pEnvironment->NewGlobalRef(LocalGameActivityClass);

        m_GetDeviceRotationMethod = pEnvironment->GetMethodID(m_GameActivityID, "GetDeviceRotation", "()I");
    }

    // -----------------------------------------------------------------------------

    JNIEnv* CJNIInterface::GetJavaEnvironment()
    {
        JNIEnv* pEnvironment = nullptr;

        jint GetResult = m_pCurrentJavaVM->GetEnv((void **)&pEnvironment, JNI_CURRENT_VERSION);

        if (GetResult == JNI_EDETACHED)
        {
            jint AttachResult = m_pCurrentJavaVM->AttachCurrentThread(&pEnvironment, NULL);

            if (AttachResult == JNI_ERR)
            {
                return nullptr;
            }
        }
        else if (GetResult != JNI_OK)
        {
            return nullptr;
        }

        return pEnvironment;
    }

    // -----------------------------------------------------------------------------

    void CJNIInterface::SetContext(jobject _pContext)
    {
        m_pContext = _pContext;
    };

    // -----------------------------------------------------------------------------

    jobject CJNIInterface::GetContext()
    {
        return m_pContext;
    };

    // -----------------------------------------------------------------------------

    void CJNIInterface::SetActivity(jobject _Activity)
    {
        m_GameActivityThiz = _Activity;
    }

    // -----------------------------------------------------------------------------

    int CJNIInterface::GetDeviceRotation()
    {
        JNIEnv* pEnvironment = GetJavaEnvironment();

        jint DeviceRotation = (jint)pEnvironment->CallIntMethod(m_GameActivityThiz, m_GetDeviceRotationMethod);

        return DeviceRotation;
    }

    // -----------------------------------------------------------------------------

    bool CJNIInterface::CheckPermission(const std::string& _rPermission)
    {
        JNIEnv* pEnvironment = GetJavaEnvironment();

        jstring Argument = pEnvironment->NewStringUTF(_rPermission.c_str());

        bool Result = pEnvironment->CallStaticBooleanMethod(m_PermissionHelperID, m_CheckPermissionMethod, Argument);

        pEnvironment->DeleteLocalRef(Argument);

        return Result;
    }

    // -----------------------------------------------------------------------------

    void CJNIInterface::AcquirePermissions(const std::string* _pPermissions, unsigned int _NumberOfPermissions)
    {
        JNIEnv* pEnvironment = GetJavaEnvironment();

        jobjectArray PermissionsArray = (jobjectArray)pEnvironment->NewObjectArray(_NumberOfPermissions, m_JavaStringClass, 0);

        for (int i = 0; i < _NumberOfPermissions; ++i)
        {
            jstring JavaString = pEnvironment->NewStringUTF(_pPermissions[i].c_str());

            pEnvironment->SetObjectArrayElement(PermissionsArray, i, JavaString);

            pEnvironment->DeleteLocalRef(JavaString);
        }

        pEnvironment->CallStaticVoidMethod(m_PermissionHelperID, m_AcquirePermissionMethod, PermissionsArray);

        pEnvironment->DeleteLocalRef(PermissionsArray);
    }
}

namespace JNI
{
    void* GetJavaEnvironment()
    {
        return CJNIInterface::GetInstance().GetJavaEnvironment();
    }

    // -----------------------------------------------------------------------------

    void* GetContext()
    {
        return CJNIInterface::GetInstance().GetContext();
    }

    // -----------------------------------------------------------------------------

    int GetDeviceRotation()
    {
        return CJNIInterface::GetInstance().GetDeviceRotation();
    }

    // -----------------------------------------------------------------------------

    bool CheckPermission(const std::string& _rPermission)
    {
        return CJNIInterface::GetInstance().CheckPermission(_rPermission);
    }

    // -----------------------------------------------------------------------------

    void AcquirePermissions(const std::string* _pPermissions, unsigned int _NumberOfPermissions)
    {
        CJNIInterface::GetInstance().AcquirePermissions(_pPermissions, _NumberOfPermissions);
    }
} // namespace JNI

// -----------------------------------------------------------------------------
// Native interface from JAVA to C++
// -----------------------------------------------------------------------------
extern "C"
{
    JNIEXPORT void JNICALL Java_com_example_native_1activity_GameActivity_nativeInitializeInterface(JNIEnv* _pEnv, jobject _LocalThiz, jobject _Context)
    {
        CJNIInterface::GetInstance().SetActivity(_pEnv->NewGlobalRef(_LocalThiz));

        CJNIInterface::GetInstance().SetContext(_pEnv->NewGlobalRef(_Context));

        CJNIInterface::GetInstance().FindClassesAndMethods();
    }

    // -----------------------------------------------------------------------------

    JNIEXPORT void JNICALL Java_com_example_native_1activity_PermissionHelper_nativeOnAcquirePermissions(JNIEnv* _pEnv, jobject _LocalThiz, jobjectArray _Permissions, jintArray _GrantResults)
    {
        jint* pGrantResults = _pEnv->GetIntArrayElements(_GrantResults, NULL);

        _pEnv->ReleaseIntArrayElements(_GrantResults, pGrantResults, 0);
    }
}; // extern "C"

// -----------------------------------------------------------------------------
// Startup function
// -----------------------------------------------------------------------------
JNIEXPORT jint JNI_OnLoad(JavaVM* _pJavaVM, void* _pReserved)
{
    CJNIInterface::GetInstance().InitializeJNI(_pJavaVM, JNI_CURRENT_VERSION);

    return JNI_CURRENT_VERSION;
}