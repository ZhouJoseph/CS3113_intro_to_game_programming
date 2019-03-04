#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>

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
#define WINDOW_HEIGHT 360


SDL_Window* displayWindow;

GLuint LoadTexture(const char* filePath) {
    int w, h, comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(image);
    return retTexture;
}

float getTranslateY(bool forward, float translateX){
    return forward ? pow((0.01 - pow(translateX,2)),0.5) : -pow((0.01 - pow(translateX,2)),0.5);
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    //setup
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    GLuint emojiEye = LoadTexture(RESOURCE_FOLDER"emoji1.png");
    GLuint emojiThought = LoadTexture(RESOURCE_FOLDER"emoji2.png");
    GLuint emojiTongue = LoadTexture(RESOURCE_FOLDER"emoji3.png");
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::ortho(-1.62f, 1.62f, -1.0f, 1.0f, -1.0f, 1.0f);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    
    ShaderProgram untexturedProgram;
    untexturedProgram.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    untexturedProgram.SetProjectionMatrix(projectionMatrix);
    untexturedProgram.SetViewMatrix(viewMatrix);
    glUseProgram(untexturedProgram.programID);
    untexturedProgram.SetColor(0.0f,0.0f, 0.0f, 0.8f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0f, 1.0f, 1.0f,1.0f);
    
    SDL_Event event;
    
    float lastFrameTicks = 0.0f;
    float angle = 0.0f;
    float translateX = 0.0f;
    bool forward = true;
    
    
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        
//      Here starts the drawing
        
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        angle += elapsed;
        translateX += (forward ? elapsed : -elapsed);
        
        if(translateX > 0.1){
            forward = false;
        }else if(translateX < -0.1){
            forward = true;
        }
        
//      draw triangle
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::rotate(modelMatrix, -angle, glm::vec3(0.0f, 0.0f, 1.0f));
        untexturedProgram.SetModelMatrix(modelMatrix);
        float triangleVertices[] = {0.433f, -0.25f, 0.0f, 0.5f, -0.433f, -0.25f};
        glEnableVertexAttribArray(untexturedProgram.positionAttribute);
        glVertexAttribPointer(untexturedProgram.positionAttribute, 2, GL_FLOAT, false, 0, triangleVertices);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(untexturedProgram.positionAttribute);
        
//      Vertices & texCoords
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        
//      Thinking
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, -0.4f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        glBindTexture(GL_TEXTURE_2D,emojiThought);
    
        
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0,6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
//      Tongue
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translateX, getTranslateY(forward,translateX)+0.6f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.6f, 0.6f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        
        glBindTexture(GL_TEXTURE_2D, emojiTongue);
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0,6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
//      Eyes
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-translateX-0.5f, getTranslateY(forward,-translateX)-0.4f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        
        glBindTexture(GL_TEXTURE_2D, emojiEye);
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0,6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
//      Here ends the drawing
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
