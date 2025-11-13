#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D tex0;
uniform int useTex;
uniform int isEmitter;
uniform vec3 objectColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform vec3 viewPos;

void main() {
    vec3 baseColor = objectColor;
    if (useTex == 1) {
        baseColor = texture(tex0, TexCoord).rgb;
    }

    // ambient term (soft)
    vec3 ambient = 0.22 * baseColor;

    // lighting calculations
    vec3 norm = normalize(Normal);
    vec3 lightDir = lightPos - FragPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);

    // attenuation (softer falloff for a more visible sun)
    float constant = 1.0;
    float linear = 0.07;
    float quadratic = 0.017;
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

    float diffFactor = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffFactor * baseColor * lightColor * lightIntensity * attenuation;

    // specular (softer)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfway = normalize(lightDir + viewDir);
    float specFactor = pow(max(dot(norm, halfway), 0.0), 8.0);
    vec3 specular = specFactor * lightColor * lightIntensity * attenuation * 0.5;

    vec3 result = ambient + diffuse + specular;

    if (isEmitter == 1) {
        // Emissive/sun rendering: make it very bright so it's clearly visible.
        vec3 emissive = lightColor * lightIntensity * 4.0 + baseColor * 0.2;
        FragColor = vec4(emissive, 1.0);
    } else {
        FragColor = vec4(result, 1.0);
    }
}
