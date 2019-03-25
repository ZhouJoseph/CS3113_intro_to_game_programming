#pragma once

#ifndef Necessity_h
#define Necessity_h

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <time.h>


#include "ShaderProgram.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//GLM Library
#include <math.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Resource_Folder * Not a good practice *
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 427
#include <vector>
#endif /* Necessity_h */
