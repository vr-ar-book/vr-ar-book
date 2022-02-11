
#pragma once

namespace ControlManager
{
    struct SConfiguration
    {
        enum EDisplayRotation
        {
            Rotation0   = 0,
            Rotation90  = 1,
            Rotation180 = 2,
            Rotation270 = 3,
        };

        void* m_pEnv;
        void* m_pContext;

        EDisplayRotation m_Rotation;
        int m_Width;
        int m_Height;
    };
} // namespace ControlManager

namespace ControlManager
{
    void OnStart(const SConfiguration& _rConfiguration);
    void OnExit();
    void Update();

    void OnPause();
    void OnResume();

    void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height);

    void OnDraw();
} // namespace ControlManager
