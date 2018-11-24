//
//  Header.h
//  Teleidoscope
//
//  Created by Michael Dewberry on 11/24/18.
//

#ifndef Header_h
#define Header_h

#include "FFGLShader.h"
#include "FFGLPluginSDK.h"
#include <string>

class Teleidoscope : public CFreeFrameGLPlugin
{
public:
    Teleidoscope();
    ~Teleidoscope();
    
    ///////////////////////////////////////////////////
    // FreeFrame plugin methods
    ///////////////////////////////////////////////////
    
    FFResult SetFloatParameter(unsigned int dwIndex, float value) override;
    float GetFloatParameter(unsigned int index) override;
    FFResult ProcessOpenGL(ProcessOpenGLStruct* pGL) override;
    FFResult InitGL(const FFGLViewportStruct *vp) override;
    FFResult DeInitGL() override;
    unsigned int Resize(const FFGLViewportStruct *vp) override;

    
    ///////////////////////////////////////////////////
    // Factory method
    ///////////////////////////////////////////////////
    
    static FFResult __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance)
    {
        *ppOutInstance = new Teleidoscope();
        if (*ppOutInstance != NULL)
            return FF_SUCCESS;
        return FF_FAIL;
    }
    
    
protected:
    // Parameters
    float m_Divisions;
    float m_OutputRotation;
    float m_Zoom;
    float m_InputRotation;
    float m_CorrectAspect;
    float m_Edge;
    float m_EdgeHue;
    float m_EdgeSaturation;
    float m_EdgeBrightness;
    int m_initResources;
    
    GLuint m_width, m_height;
    
    FFGLShader m_shader;
    GLint m_inputTextureLocation;
    GLint m_DivisionsLocation;
    GLint m_OutputRotationLocation;
    GLint m_ZoomLocation;
    GLint m_InputRotationLocation;
    GLint m_CorrectAspectLocation;
    GLint m_EdgeLocation;
    GLint m_EdgeColorLocation;
    GLint m_textureAspectLocation;
};

#endif /* Header_h */
