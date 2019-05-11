attribute vec4 position;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	vec4 p = viewMatrix * modelMatrix  * position;
	gl_Position = projectionMatrix * p;
}

//attribute vec4 position;
//attribute vec4 lerpColor;
//
//uniform mat4 modelviewMatrix;
//uniform mat4 projectionMatrix;
//
//varying vec4 vertexColor;
//
//void main() {
//    vec4 p = modelviewMatrix  * position;
//    vertexColor = lerpColor;
//    gl_Position = projectionMatrix * p;
//}
