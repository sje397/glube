#version 120
attribute vec3 vertex;
attribute vec3 normalVec;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

varying vec3 position;
varying vec3 norm;

void main() {
    position = vertex;
    norm = normalVec;
    //norm = vec3(0, 0, 1);
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
