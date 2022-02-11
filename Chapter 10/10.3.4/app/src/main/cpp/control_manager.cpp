
#include "jni_interface.h"

#include "control_manager.h"

#include "arcore_c_api.h"

#include "GLES3/gl3.h"

#include <assert.h>
#include <unordered_set>
#include <string>
#include <vector>

#ifndef GL_OES_EGL_image_external
#define GL_OES_EGL_image_external 1
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#define GL_TEXTURE_BINDING_EXTERNAL_OES   0x8D67
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES 0x8D68
#endif /* GL_OES_EGL_image_external */

using namespace ControlManager;

namespace
{
    const float k_Uvs[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    constexpr char k_VertexShader[] = R"(
        #version  320 es

        layout(location = 0) in vec2 in_UV;

        layout(location = 0) out vec2 out_UV;

        void main()
        {
            vec2 Vertices[4];

            Vertices[0] = vec2(-1.0f, -1.0f);
            Vertices[1] = vec2( 1.0f, -1.0f);
            Vertices[2] = vec2(-1.0f,  1.0f);
            Vertices[3] = vec2( 1.0f,  1.0f);

            out_UV = in_UV;

            gl_Position = vec4(Vertices[gl_VertexID], 0.0f, 1.0f);
        }
    )";

    constexpr char k_FragmentShader[] = R"(
        #version 320 es

        #extension GL_OES_EGL_image_external_essl3 : require

        precision mediump float;

        layout(location = 0) uniform samplerExternalOES in_ExtOESTexture;

        layout(location = 0) in vec2 in_UV;

        layout(location = 0) out vec4 out_Output;

        void main()
        {
            out_Output = texture(in_ExtOESTexture, in_UV);// + vec4(in_UV, 0.0f, 1.0f);
        }
    )";

    static GLuint LoadShader(GLenum _Type, const char* _pSource)
    {
        GLuint Shader = glCreateShader(_Type);

        if (!Shader)
        {
            return Shader;
        }

        glShaderSource(Shader, 1, &_pSource, nullptr);

        glCompileShader(Shader);

        GLint CompileStatus = 0;

        glGetShaderiv(Shader, GL_COMPILE_STATUS, &CompileStatus);

        if (!CompileStatus)
        {
            GLint InfoLogLength = 0;

            glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &InfoLogLength);

            if (!InfoLogLength)
            {
                return Shader;
            }

            char* pBuffer = reinterpret_cast<char*>(malloc(InfoLogLength));

            if (!pBuffer)
            {
                return Shader;
            }

            glGetShaderInfoLog(Shader, InfoLogLength, nullptr, pBuffer);

            free(pBuffer);

            glDeleteShader(Shader);

            Shader = 0;
        }

        return Shader;
    }

    static unsigned int CreateProgram(const char* _pVertexSource, const char* _pFragmentSource)
    {
        unsigned int VertexShader = LoadShader(GL_VERTEX_SHADER, _pVertexSource);

        if (!VertexShader)
        {
            return 0;
        }

        unsigned int FragmentShader = LoadShader(GL_FRAGMENT_SHADER, _pFragmentSource);

        if (!FragmentShader)
        {
            return 0;
        }

        unsigned int Program = glCreateProgram();

        if (Program)
        {
            glAttachShader(Program, VertexShader);

            glAttachShader(Program, FragmentShader);

            glLinkProgram(Program);

            GLint LinkStatus= GL_FALSE;

            glGetProgramiv(Program, GL_LINK_STATUS, &LinkStatus);

            if (LinkStatus != GL_TRUE)
            {
                glDeleteProgram(Program);

                Program = 0;
            }
        }

        return Program;
    }

    unsigned int g_ShaderProgram;
    unsigned int g_TextureID;
    unsigned int g_AttributeUVs;

    static constexpr int s_NumberOfVertices = 4;
    bool g_IsUVsInitialized = false;
    float g_TransformedUVs[s_NumberOfVertices * 2];
} // namespace

namespace
{
    class CControlManager
    {
    public:

        static CControlManager& GetInstance()
        {
            static CControlManager s_Singleton;
            return s_Singleton;
        }

    public:

        CControlManager();
       ~CControlManager();

    public:

        void OnStart(const SConfiguration& _rConfiguration);
        void OnExit();
        void Update();

        void OnPause();
        void OnResume();

        void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height);

        void OnDraw();

    private:

        typedef std::unordered_set<ArAnchor*> CTrackedObjects;

    private:

        ArSession* m_pARSession;
        ArFrame* m_pARFrame;
        CTrackedObjects m_TrackedObjects;
    };
} // namespace

namespace
{
    CControlManager::CControlManager()
        : m_pARSession    (0)
        , m_pARFrame      (0)
        , m_TrackedObjects()
    {
    }

    // -----------------------------------------------------------------------------

    CControlManager::~CControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CControlManager::OnStart(const SConfiguration& _rConfiguration)
    {
        // -----------------------------------------------------------------------------
        // AR session and frame
        // -----------------------------------------------------------------------------
        ArStatus Status;

        Status = ArSession_create(_rConfiguration.m_pEnv, _rConfiguration.m_pContext, &m_pARSession);

        assert(Status == AR_SUCCESS);

        assert(m_pARSession != 0);

        ArConfig* ARConfig = 0;

        ArConfig_create(m_pARSession, &ARConfig);

        assert(ARConfig != 0);

        Status = ArSession_checkSupported(m_pARSession, ARConfig);

        assert(Status == AR_SUCCESS);

        Status = ArSession_configure(m_pARSession, ARConfig);

        assert(Status == AR_SUCCESS);

        ArConfig_destroy(ARConfig);

        ArFrame_create(m_pARSession, &m_pARFrame);

        assert(m_pARFrame != 0);

        ArSession_setDisplayGeometry(m_pARSession, _rConfiguration.m_Rotation, _rConfiguration.m_Width, _rConfiguration.m_Height);

        // -----------------------------------------------------------------------------
        // OpenGLES
        // -----------------------------------------------------------------------------
        g_ShaderProgram = CreateProgram(k_VertexShader, k_FragmentShader);

        glGenTextures(1, &g_TextureID);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, g_TextureID);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenBuffers(1, &g_AttributeUVs);

        glBindBuffer(GL_ARRAY_BUFFER, g_AttributeUVs);

        glBufferData(GL_ARRAY_BUFFER, sizeof(k_Uvs), &k_Uvs, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        ArSession_setCameraTextureName(m_pARSession, g_TextureID);
    }

    // -----------------------------------------------------------------------------

    void CControlManager::OnExit()
    {
        m_TrackedObjects.clear();

        // -----------------------------------------------------------------------------
        // AR session and frame
        // -----------------------------------------------------------------------------
        ArSession_destroy(m_pARSession);

        ArFrame_destroy(m_pARFrame);

        // -----------------------------------------------------------------------------
        // OpenGLES
        // -----------------------------------------------------------------------------
        glDeleteProgram(g_ShaderProgram);

        glDeleteTextures(1, &g_TextureID);
    }

    // -----------------------------------------------------------------------------

    void CControlManager::Update()
    {
        ArSession_update(m_pARSession, m_pARFrame);
    }

    // -----------------------------------------------------------------------------

    void CControlManager::OnPause()
    {
        ArSession_pause(m_pARSession);
    }

    // -----------------------------------------------------------------------------

    void CControlManager::OnResume()
    {
        ArSession_resume(m_pARSession);
    }

    // -----------------------------------------------------------------------------

    void CControlManager::OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
        ArSession_setDisplayGeometry(m_pARSession, _DisplayRotation, _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void CControlManager::OnDraw()
    {
        int32_t HasGeometryChanged = 0;

        ArFrame_getDisplayGeometryChanged(m_pARSession, m_pARFrame, &HasGeometryChanged);

        if (HasGeometryChanged != 0 || g_IsUVsInitialized == false)
        {
            ArFrame_transformDisplayUvCoords(m_pARSession, m_pARFrame, s_NumberOfVertices * 2, k_Uvs, g_TransformedUVs);

            glBindBuffer(GL_ARRAY_BUFFER, g_AttributeUVs);

            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_TransformedUVs), &g_TransformedUVs);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            g_IsUVsInitialized = true;
        }

        glUseProgram(g_ShaderProgram);

        glDisable(GL_DEPTH_TEST);

        glDisable(GL_BLEND);

        glDisable(GL_CULL_FACE);

        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, g_TextureID);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, g_AttributeUVs);

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

        glUseProgram(0);
    }
} // namespace

namespace ControlManager
{
    void OnStart(const SConfiguration& _rConfiguration)
    {
        CControlManager::GetInstance().OnStart(_rConfiguration);
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CControlManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CControlManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void OnPause()
    {
        CControlManager::GetInstance().OnPause();
    }

    // -----------------------------------------------------------------------------

    void OnResume()
    {
        CControlManager::GetInstance().OnResume();
    }

    // -----------------------------------------------------------------------------

    void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
        CControlManager::GetInstance().OnDisplayGeometryChanged(_DisplayRotation, _Width, _Height);
    }

    void OnDraw()
    {
        CControlManager::GetInstance().OnDraw();
    }
} // namespace ControlManager
