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

SDL_Window* displayWindow;

struct Coord{
    Coord(float x, float y,float z):x(x),y(y),z(z){}
    Coord(float x, float y):x(x),y(y),z(0.0){}
    float x;
    float y;
    float z;
};

struct GameObject{

    GameObject(float w, float h, float x, float y, float v_x,float v_y):
    width(w),height(h),coord(x,y),horizontalVelocity(v_x),verticleVelocity(v_y),
    vertices{-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5}
    {}

    void Draw(ShaderProgram& program)
    {
        /* Translate and Scale */
        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(coord.x,coord.y,coord.z));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(width,height,0.0f));
        program.SetModelMatrix(modelMatrix);
        
        
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
    }
    
    Coord coord;
    float width;
    float height;
    float verticleVelocity;
    float horizontalVelocity;
    float vertices[12];
};

struct Ball:GameObject{

    Ball(float w, float h, float x, float y, float v_x,float v_y):GameObject(w,h,x,y,v_x,v_y){
        srand(static_cast<unsigned int>(time(NULL)));
        bool directionPositive[2] = {true,false};
        seedX = rand() % 360;
        seedY = rand() % 360;
        auto directionHorizontal = directionPositive[rand()%2];
        auto directionVerticle = directionPositive[rand()%2];
        
        if((seedX > 65 && seedX < 115) || (seedX > 245 && seedX < 295)){
            seedX = directionHorizontal ? 45 : 225;
        }
        if((seedY < 25 || seedY > 335) || (seedY > 155 && seedY < 205)){
            seedY = directionVerticle ? 45 : 225;
        }
        horizontalVelocity = 1.0f;
        verticleVelocity = 1.0f;
    }

    int seedX;
    int seedY;
};


ShaderProgram setUpProgram(){
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f);
    
    ShaderProgram untexturedProgram;
    untexturedProgram.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    untexturedProgram.SetProjectionMatrix(projectionMatrix);
    untexturedProgram.SetViewMatrix(viewMatrix);
    glUseProgram(untexturedProgram.programID);
    untexturedProgram.SetColor(0.0f,0.0f, 0.0f, 0.8f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0f, 1.0f, 1.0f,1.0f);
    return untexturedProgram;
}


void ProcessEvents(bool& done, SDL_Event& event){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
}


void Update(Ball& ball, GameObject& leftPaddle, GameObject& rightPaddle, ShaderProgram& untexturedProgram, float& lastFrameTicks, bool& reverse){
    glClear(GL_COLOR_BUFFER_BIT);
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(ball.coord.y > 1.5f - ball.height/2 || ball.coord.y < -1.5f + ball.height/2){
        ball.coord.y = (ball.coord.y > 1.5f - ball.height/2) ? 1.5f - ball.height/2 - 0.005f : -1.5f + ball.height/2 + 0.0005f;
        ball.verticleVelocity *= -1.0f;
    }

    if(keys[SDL_SCANCODE_UP]){
        rightPaddle.coord.y += rightPaddle.verticleVelocity * elapsed;
        if(rightPaddle.coord.y >= 1.5f - rightPaddle.height/2){
            rightPaddle.coord.y = 1.5f - rightPaddle.height/2;
        }
    }else if (keys[SDL_SCANCODE_DOWN]){
        rightPaddle.coord.y -= rightPaddle.verticleVelocity * elapsed;
        if(rightPaddle.coord.y <= -1.5f + rightPaddle.height/2){
            rightPaddle.coord.y = -1.5f + rightPaddle.height/2;
        }
    }
    
    
    if (keys[SDL_SCANCODE_W]){
        leftPaddle.coord.y += leftPaddle.verticleVelocity * elapsed;
        if(leftPaddle.coord.y >= 1.5 - leftPaddle.height/2){
            leftPaddle.coord.y = 1.5 - leftPaddle.height/2;
        }
    }else if (keys[SDL_SCANCODE_S]){
        leftPaddle.coord.y -= leftPaddle.verticleVelocity * elapsed;
        if(leftPaddle.coord.y <= -1.5 + leftPaddle.height/2){
            leftPaddle.coord.y = -1.5 + leftPaddle.height/2;
        }
    }
    /* How far is the ball with the paddle on X/Y axis? */
    auto lpx = abs(ball.coord.x - leftPaddle.coord.x) - (ball.width + leftPaddle.width)/2;
    auto lpy = abs(ball.coord.y - leftPaddle.coord.y) - (ball.height + leftPaddle.height)/2;
    auto rpx = abs(ball.coord.x - rightPaddle.coord.x) - (ball.width + rightPaddle.width)/2;
    auto rpy = abs(ball.coord.y - rightPaddle.coord.y) - (ball.height + rightPaddle.height)/2;
    
    
    
    
    if(reverse){
        if ((lpx < 0 && lpy < 0) || (rpx < 0 && rpy < 0)){
            auto ballTopY = ball.coord.y - ball.height/2;
            auto ballBtmY = ball.coord.y + ball.height/2;
            auto leftTopY = leftPaddle.coord.y - leftPaddle.height/2;
            auto leftBtmY = leftPaddle.coord.y + leftPaddle.height/2;
            auto rightTopY = rightPaddle.coord.y - rightPaddle.height/2;
            auto rightBtmY = rightPaddle.coord.y + rightPaddle.height/2;
            if(lpx < 0 && lpy < 0){
                if(ballBtmY - leftTopY > 0 && ballBtmY - leftTopY < 0.02){
                    ball.verticleVelocity *= -1.0f;
                }else if (ballTopY - leftBtmY < 0 && ballTopY - leftBtmY > -0.02){
                    ball.verticleVelocity *= -1.0f;
                }else{
                    ball.horizontalVelocity *= -1.0f;
                }
            }else{
                if(ballBtmY - rightTopY > 0 && ballBtmY - rightTopY < 0.02){
                    ball.verticleVelocity *= -1.0f;
                }else if (ballTopY - rightBtmY < 0 && ballTopY - rightBtmY > -0.02){
                    ball.verticleVelocity *= -1.0f;
                }else{
                    ball.horizontalVelocity *= -1.0f;
                }
            }
        }
        reverse = false;
    }
    
    if(!((lpx < 0 && lpy < 0) || (rpx < 0 && rpy < 0))){
        reverse = true;
    }
    

    ball.coord.x += cos(ball.seedX * M_PI/180)* elapsed * ball.horizontalVelocity;
    ball.coord.y += sin(ball.seedY * M_PI/180)* elapsed * ball.verticleVelocity;
    
    
    if(ball.coord.x < -2.0 + ball.width/2){
        std::cout << "Right paddle wins" << std::endl;
        leftPaddle = GameObject(0.01f,0.6f,-1.6f,0.0f,0.0f,1.0f);
        rightPaddle = GameObject(0.01f,0.6f,1.6f,0.0f,0.0f,1.0f);
        ball = Ball(0.025f,0.025f,0.0f,0.0f,1.3f,1.3f);
    }else if(ball.coord.x > 2.0 - ball.width/2){
        std::cout << "Left paddle wins" << std::endl;
        leftPaddle = GameObject(0.01f,0.6f,-1.6f,0.0f,0.0f,1.0f);
        rightPaddle = GameObject(0.01f,0.6f,1.6f,0.0f,0.0f,1.0f);
        ball = Ball(0.025f,0.025f,0.0f,0.0f,1.3f,1.3f);
    }
    
    leftPaddle.Draw(untexturedProgram);
    rightPaddle.Draw(untexturedProgram);
    ball.Draw(untexturedProgram);
    SDL_GL_SwapWindow(displayWindow);
}

void Render(){}

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
    ShaderProgram untexturedProgram = setUpProgram();
    
    SDL_Event event;
    float lastFrameTicks = 0.0f;
    bool done = false;
    
//    width,height,coord,horizontalVelocity,verticleVelocity,
    GameObject leftPaddle(0.01f,0.6f,-1.6f,0.0f,0.0f,1.0f);
    GameObject rightPaddle(0.01f,0.6f,1.6f,0.0f,0.0f,1.0f);
    Ball ball(0.025f,0.025f,0.0f,0.0f,1.3f,1.3f);
    bool reverse = true;
    
    while (!done) {

        
//      Here starts the drawing
        
        ProcessEvents(done,event);
        Update(ball,leftPaddle,rightPaddle,untexturedProgram,lastFrameTicks,reverse);
        Render();
        
//      Here ends the drawing
        
    }
    
    SDL_Quit();
    return 0;
}
