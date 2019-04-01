//
//  SheetSprite.cpp
//  NYUCodebase
//
//  Created by 周凯旋 on 3/15/19.
//  Copyright © 2019 Ivan Safrin. All rights reserved.
//

#include "SheetSprite.hpp"

SheetSprite::SheetSprite(){};
SheetSprite::SheetSprite(unsigned int textureID, float u, float v,float width, float height,float size):
textureID(textureID), u(u), v(v), size(size), width(width),height(height){}

void SheetSprite::Draw(ShaderProgram& program) const{
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLfloat texCoords[] = {
        u, v + height,
        u + width, v,
        u, v,
        u + width, v,
        u, v + height,
        u + width, v + height
    };
    float aspect = width / height;
    float vertices[] = {
        -0.5f * size * aspect, -0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, 0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, -0.5f * size ,
        0.5f * size * aspect, -0.5f * size};
    
    /* Draw Arr Here */
    glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0,6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}


