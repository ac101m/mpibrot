#version 330 core


// Orbit texture
uniform sampler2D texture0;


// Inputs from vertex shader
in VS_OUT {
  vec2 fragUV;
} fsIn;


// Fragment colour output
out vec4 gl_FragColor;


void main() {
  vec3 fragColor = texture(texture0, fsIn.fragUV).rgb;
  gl_FragColor = vec4(fragColor, 1.0);
}
