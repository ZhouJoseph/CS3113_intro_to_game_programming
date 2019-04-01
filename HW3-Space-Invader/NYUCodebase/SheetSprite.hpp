//
//  SheetSprite.hpp
//  NYUCodebase
//
//  Created by 周凯旋 on 3/15/19.
//  Copyright © 2019 Ivan Safrin. All rights reserved.
//

#ifndef SheetSprite_hpp
#define SheetSprite_hpp

#include <stdio.h>
#include "ShaderProgram.h"

struct SheetSprite{
    SheetSprite();
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size);
    
    void Draw(ShaderProgram& program) const;
    float u;
    float v;
    float size;
    float width;
    float height;
    unsigned int textureID;
};

#endif /* SheetSprite_hpp */
