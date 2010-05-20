#include "Renderer.hpp"

//=====================================================================================================================================
// Vars                                                                                                                               =
//=====================================================================================================================================
int Renderer::screenshotJpegQuality = 90;
bool Renderer::textureCompression = false;
int  Renderer::maxTextureUnits = -1;
bool Renderer::mipmapping = true;
int  Renderer::maxAnisotropy = 8;
float Renderer::quadVertCoords [][2] = { {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0} };
