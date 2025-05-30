struct Buffers {
  sampler2D color;
};

in vec2 fragTexcoords;

layout(std140) uniform uboCameraMatrices {
  mat4 viewMat;
  mat4 invViewMat;
  mat4 projectionMat;
  mat4 invProjectionMat;
  mat4 viewProjectionMat;
  vec3 cameraPos;
};

uniform Buffers uniSceneBuffers;

layout(location = 0) out vec4 fragColor;

void main() {
  vec3 color = texture(uniSceneBuffers.color, fragTexcoords).rgb;

  fragColor = vec4(color, 1.0);
}