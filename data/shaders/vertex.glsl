#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMat = transpose(inverse(mat3(model)));
    Normal = normalize(normalMat * aNormal);
    TexCoord = aTex;
    gl_Position = proj * view * vec4(FragPos, 1.0);
}
