#version 330 core

in vec3 vNormal;
in vec2 vUV;

uniform sampler2D tex;

uniform vec3 lightDirection;
uniform vec3 lightColor;

out vec4 FragColor;

void main() {
  vec3 totalLight = vec3(0,0,0);

  float dif = dot(vNormal, lightDirection);
  // totalLight = clamp(totalLight + dif * lightColor, 0, 1);
  // vec4 color = vec4(vec3(1,1,1).xyz * totalLight, 1);
  // FragColor = vec4(color.rgb * texture(tex, vUV).rgb, 1);
  FragColor = texture(tex, vUV);
}