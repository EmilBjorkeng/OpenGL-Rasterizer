#version 440 core

#define MAX_TEXTURES 16
#define MAX_LIGHTS 16

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
flat in int TexID;
flat in vec3 DiffuseColor;
flat in float Opacity;

uniform sampler2D textures[MAX_TEXTURES];

uniform bool useLighting;
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];
uniform float lightIntensities[MAX_LIGHTS];

uniform float lightFarPlanes[MAX_LIGHTS];
uniform samplerCube depthMaps[MAX_LIGHTS];

uniform float ambientLight;
uniform vec3 ambientLightColor;

out vec4 FragColor;

float ShadowCalculation(vec3 fragPos, int lightIndex) {
    vec3 fragToLight = fragPos - lightPositions[lightIndex];

    float closestDepth = texture(depthMaps[lightIndex], fragToLight).r;
    closestDepth *= lightFarPlanes[lightIndex];

    float currentDepth = length(fragToLight);

    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPositions[lightIndex] - fragPos);
    float cosTheta = max(dot(normal, lightDir), 0.0);
    float bias = 0.005 + 0.05 * (1.0 - cosTheta);

    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main() {
    // Base color and alpha
    vec3 color = DiffuseColor;
    float alpha = Opacity;

    // Apply texture if available
    if (TexID >= 0) {
        vec4 texColor = texture(textures[TexID], TexCoord);
        color *= texColor.rgb;
        alpha *= texColor.a;
    }

    vec3 finalColor;
    if (useLighting) {
        vec3 diffuse = vec3(0.0);
        vec3 norm = normalize(Normal);

        for (int i = 0; i < numLights; ++i) {
            vec3 lightDir = normalize(lightPositions[i] - FragPos);
            float ndotl = max(dot(norm, lightDir), 0.0);

            // Distance attenuation
            float distance = length(lightPositions[i] - FragPos);
            if (distance < 1e-6) distance = 1e-6;
            float attenuation = 1.0 / max(distance * distance, 1e-6);

            float shadow = ShadowCalculation(FragPos, i);
            diffuse += lightColors[i] * lightIntensities[i] * ndotl * attenuation * (1.0 - shadow);
            //diffuse += lightColors[i] * lightIntensities[i] * ndotl * attenuation;
        }
        diffuse *= color;

        vec3 ambient = color * ambientLightColor * ambientLight;
        finalColor = diffuse + ambient;
    }
    else {
        // Fullbright
        finalColor = color;
    }

    FragColor = vec4(clamp(finalColor, 0.0, 1.0), alpha);
}
