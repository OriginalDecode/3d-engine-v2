#pragma once
#include <Core/Defines.h>

LPE_DEFINE_HANDLE( VkInstance );

namespace Graphics
{
    class VlkInstance
    {
    public:
        VlkInstance() = default;
        ~VlkInstance();

        void Init();
        void Release();

        VkInstance get() const { return m_Instance; }

    private:
        VkInstance m_Instance = nullptr;
    };

}; //namespace Graphics