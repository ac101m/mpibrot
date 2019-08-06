#version 330 core


// Vertex attribute inputs
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertUV;
layout (location = 2) in vec3 vertNml;
layout (location = 3) in vec3 vertTan;
layout (location = 4) in vec3 vertBitan;


// Outputs from the vertex shader
out VS_OUT {
  vec3 fragPos;
  vec2 fragUV;
} vsOut;


void main() {
  vsOut.fragUV = vertUV;
  gl_Position = vec4(vertPos, 1.0);
}
