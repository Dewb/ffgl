//
//  Teleidoscope.cpp
//  Teleidoscope
//
//  Created by Michael Dewberry on 11/24/18.
//

#include <stdio.h>
#include "FFGL.h"
#include "FFGLLib.h"

#include "Teleidoscope.h"
#include "../../lib/ffgl/utilities/utilities.h"

#define FFPARAM_Divisions  (0)
#define FFPARAM_OutputRotation     (1)
#define FFPARAM_Zoom     (2)
#define FFPARAM_InputRotation (3)
#define FFPARAM_CorrectAspect (4)
#define FFPARAM_Edge (5)
#define FFPARAM_EdgeHue (6)
#define FFPARAM_EdgeSaturation (7)
#define FFPARAM_EdgeBrightness (8)

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo (
                                   Teleidoscope::CreateInstance,        // Create method
                                   "DBTS",                                // Plugin unique ID
                                   "Teleidoscope",                    // Plugin name
                                   1,                                       // API major version number
                                   500,                                // API minor version number
                                   1,                                    // Plugin major version number
                                   000,                                // Plugin minor version number
                                   FF_EFFECT,                            // Plugin type
                                   "Advanced kaleidoscope",            // Plugin description
                                   "by Dewb"                // About
);

static const std::string vertexShaderCode =
STRINGIFY(
    void main()
    {
        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_FrontColor = gl_Color;
    }
);


static const std::string fragmentShaderCode =
STRINGIFY(
    uniform sampler2D inputTexture;
    uniform float textureAspect;

    uniform float Divisions;
    uniform float OutputRotation;
    uniform float Zoom;
    uniform float InputRotation;
    uniform float CorrectAspect;
    uniform float Edge;
    uniform vec4 EdgeColor;

    float PI = 3.14159262;
    
    void main()
    {
        vec2 orig_coord = gl_TexCoord[0].st;
        vec2 center = vec2(0.5, 0.5);
        
        if (CorrectAspect > 0.0) {
            orig_coord.x = orig_coord.x * textureAspect;
            center.x = center.x * textureAspect;
        }
        
        vec2 center_vec = orig_coord - center;
        float radius = length(center_vec);
        //radius = mod(radius, 0.5);
        
        float angle = atan(center_vec.y, center_vec.x) - (OutputRotation * PI * 2.0) + PI * 2.0;
        float angle_offset = PI * 2.0 / floor(Divisions*12.0);
        angle = mod(angle, angle_offset);
        
        if (angle > angle_offset / 2.0) {
            angle = angle_offset - angle;
        }
        
        float d = mod(angle, angle_offset / 2.0);
        if (d > angle_offset / 4.0) {
            d = angle_offset / 2.0 - d;
        }
        d = 1.0 - d / (angle_offset / 4.0);
        
        angle = angle + (InputRotation * PI * 2.0);
        
        vec2 new_coord = radius * Zoom * vec2(cos(angle), sin(angle)) + center;
        
        if (CorrectAspect > 0.0) {
            new_coord.x = new_coord.x / textureAspect;
        }
        
        vec4 color = texture2D(inputTexture, new_coord);
        gl_FragColor = mix(color, EdgeColor, (pow(d, 6.0)) * 0.5 * Edge);
    }

    );

Teleidoscope::Teleidoscope()
:CFreeFrameGLPlugin(),
m_initResources(1),
m_inputTextureLocation(-1),
m_textureAspectLocation(-1),
m_DivisionsLocation(-1),
m_OutputRotationLocation(-1),
m_ZoomLocation(-1),
m_InputRotationLocation(-1),
m_CorrectAspectLocation(-1),
m_EdgeLocation(-1),
m_EdgeColorLocation(-1)
{
    // Input properties
    SetMinInputs(1);
    SetMaxInputs(1);
    
    // Parameters
    m_Divisions = 0.5f;
    SetParamInfo(FFPARAM_Divisions, "Divisions", FF_TYPE_STANDARD, m_Divisions);
    m_OutputRotation = 0.0f;
    SetParamInfo(FFPARAM_OutputRotation, "Output Rotation", FF_TYPE_STANDARD, m_OutputRotation);
    m_Zoom = 0.0f;
    SetParamInfo(FFPARAM_Zoom, "Zoom", FF_TYPE_STANDARD, m_Zoom);
    m_InputRotation = 0.0f;
    SetParamInfo(FFPARAM_InputRotation, "Input Rotation", FF_TYPE_STANDARD, m_InputRotation);
    m_CorrectAspect = 1.0f;
    SetParamInfo(FFPARAM_CorrectAspect, "Correct Aspect", FF_TYPE_BOOLEAN, m_CorrectAspect);
    m_Edge = 0.0f;
    SetParamInfo(FFPARAM_Edge, "Edge", FF_TYPE_STANDARD, m_Edge);
    m_EdgeHue = 0.0f;
    SetParamInfo(FFPARAM_EdgeHue, "Edge Color", FF_TYPE_HUE, m_EdgeHue);
    m_EdgeSaturation = 0.0f;
    SetParamInfo(FFPARAM_EdgeSaturation, "Edge Saturation", FF_TYPE_SATURATION, m_EdgeSaturation);
    m_EdgeBrightness = 0.0f;
    SetParamInfo(FFPARAM_EdgeBrightness, "Edge Brightness", FF_TYPE_BRIGHTNESS, m_EdgeBrightness);
    
}

Teleidoscope::~Teleidoscope()
{
    
}

FFResult Teleidoscope::InitGL(const FFGLViewportStruct *vp)
{
    
    m_initResources = 0;
    
    
    //initialize gl shader
    m_shader.Compile(vertexShaderCode,fragmentShaderCode);
    
    //activate our shader
    m_shader.BindShader();
    
    //to assign values to parameters in the shader, we have to lookup
    //the "location" of each value.. then call one of the glUniform* methods
    //to assign a value
    m_inputTextureLocation = m_shader.FindUniform("inputTexture");
    m_textureAspectLocation = m_shader.FindUniform("textureAspect");

    m_DivisionsLocation = m_shader.FindUniform("Divisions");
    m_OutputRotationLocation = m_shader.FindUniform("OutputRotation");
    m_ZoomLocation = m_shader.FindUniform("Zoom");
    m_InputRotationLocation = m_shader.FindUniform("InputRotation");
    m_CorrectAspectLocation = m_shader.FindUniform("CorrectAspect");
    m_EdgeLocation = m_shader.FindUniform("Edge");
    m_EdgeColorLocation = m_shader.FindUniform("EdgeColor");

    //the 0 means that the 'inputTexture' in
    //the shader will use the texture bound to GL texture unit 0
    glUniform1i(m_inputTextureLocation, 0);
    
    m_width = vp->width;
    m_height = vp->height;
    
    m_shader.UnbindShader();
    
    return FF_SUCCESS;
}

unsigned int Teleidoscope::Resize(const FFGLViewportStruct *vp) {
    m_width = vp->width;
    m_height = vp->height;
}


FFResult Teleidoscope::DeInitGL()
{
    m_shader.FreeGLResources();
    
    
    return FF_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Methods
////////////////////////////////////////////////////////////////////////////////////////////////////



FFResult Teleidoscope::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{
    if (pGL->numInputTextures<1)
        return FF_FAIL;
    
    if (pGL->inputTextures[0]==NULL)
        return FF_FAIL;
    
    //activate our shader
    m_shader.BindShader();
    
    FFGLTextureStruct &Texture = *(pGL->inputTextures[0]);
    
    //get the max s,t that correspond to the
    //width,height of the used portion of the allocated texture space
    FFGLTexCoords maxCoords = GetMaxGLTexCoords(Texture);
    
    glUniform1f(m_DivisionsLocation, m_Divisions);
    glUniform1f(m_OutputRotationLocation, m_OutputRotation);
    glUniform1f(m_ZoomLocation, m_Zoom);
    glUniform1f(m_InputRotationLocation, m_InputRotation);
    glUniform1f(m_CorrectAspectLocation, m_CorrectAspect);
    glUniform1f(m_EdgeLocation, m_Edge);
    
    double rgb1[3];
    double hue1 = (m_EdgeHue == 1.0) ? 0.0 : m_EdgeHue;
    HSVtoRGB( hue1, m_EdgeSaturation, m_EdgeBrightness, &rgb1[0], &rgb1[1], &rgb1[2]);
    glUniform3f(m_EdgeColorLocation, rgb1[0], rgb1[1], rgb1[2]);
    
    glUniform1f(m_textureAspectLocation, (m_width * 1.0 / m_height * 1.0));
    
    //activate texture unit 1 and bind the input texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture.Handle);
    
    //draw the quad that will be painted by the shader/textures
    //note that we are sending texture coordinates to texture unit 1..
    //the vertex shader and fragment shader refer to this when querying for
    //texture coordinates of the inputTexture
    glBegin(GL_QUADS);
    
    //lower left
    glMultiTexCoord2f(GL_TEXTURE0, 0,0);
    glVertex2f(-1,-1);
    
    //upper left
    glMultiTexCoord2f(GL_TEXTURE0, 0, maxCoords.t);
    glVertex2f(-1,1);
    
    //upper right
    glMultiTexCoord2f(GL_TEXTURE0, maxCoords.s, maxCoords.t);
    glVertex2f(1,1);
    
    //lower right
    glMultiTexCoord2f(GL_TEXTURE0, maxCoords.s, 0);
    glVertex2f(1,-1);
    glEnd();
    
    //unbind the input texture
    glBindTexture(GL_TEXTURE_2D,0);
    
    
    //unbind the shader
    m_shader.UnbindShader();
    
    return FF_SUCCESS;
}

float Teleidoscope::GetFloatParameter(unsigned int dwIndex)
{
    switch (dwIndex)
    {
        case FFPARAM_Divisions:
            return m_Divisions;
        case FFPARAM_OutputRotation:
            return m_OutputRotation;
        case FFPARAM_Zoom:
            return m_Zoom;
        case FFPARAM_InputRotation:
            return m_InputRotation;
        case FFPARAM_CorrectAspect:
            return m_CorrectAspect;
        case FFPARAM_Edge:
            return m_Edge;
        case FFPARAM_EdgeHue:
            return m_EdgeHue;
        case FFPARAM_EdgeSaturation:
            return m_EdgeSaturation;
        case FFPARAM_EdgeBrightness:
            return m_EdgeBrightness;
        default:
            return 0.0;
    }
}

FFResult Teleidoscope::SetFloatParameter(unsigned int dwIndex, float value)
{
    switch (dwIndex)
    {
        case FFPARAM_Divisions:
            m_Divisions = value;
            break;
        case FFPARAM_OutputRotation:
            m_OutputRotation = value;
            break;
        case FFPARAM_Zoom:
            m_Zoom = value;
            break;
        case FFPARAM_InputRotation:
            m_InputRotation = value;
            break;
        case FFPARAM_CorrectAspect:
            m_CorrectAspect = value;
            break;
        case FFPARAM_Edge:
            m_Edge = value;
            break;
        case FFPARAM_EdgeHue:
            m_EdgeHue = value;
            break;
        case FFPARAM_EdgeSaturation:
            m_EdgeSaturation = value;
            break;
        case FFPARAM_EdgeBrightness:
            m_EdgeBrightness = value;
            break;
        default:
            return FF_FAIL;
    }
    
    return FF_SUCCESS;
}
