uniform sampler2D DiffuseMap, NormalMap, SpecularMap, ShininessMap;
uniform sampler2D AmbientMap, EmissiveMap, ReflectionMap;
VERSE_FS_IN vec4 texCoord0, texCoord1, color;
VERSE_FS_IN vec3 eyeNormal, eyeTangent, eyeBinormal;
VERSE_SRCIPT_DEF

#ifdef VERSE_GLES3
layout(location = 0) VERSE_FS_OUT vec4 fragData0;
layout(location = 1) VERSE_FS_OUT vec4 fragData1;
layout(location = 2) VERSE_FS_OUT vec4 fragData2;
layout(location = 3) VERSE_FS_OUT vec4 fragData3;
#endif

void main()
{
    vec2 uv0 = texCoord0.xy, uv1 = texCoord1.xy;
    vec4 diffuse = VERSE_TEX2D(DiffuseMap, uv0) * color;
    vec4 normalValue = VERSE_TEX2D(NormalMap, uv0);
    vec4 emission = VERSE_TEX2D(EmissiveMap, uv1);
    vec3 specular = VERSE_TEX2D(SpecularMap, uv0).rgb;
    vec3 metalRough = VERSE_TEX2D(ShininessMap, uv0).rgb;

    VERSE_SCRIPT_FUNC(0);
    if (diffuse.a < 0.1) discard;

    // Compute eye-space normal
    vec3 eyeNormal2 = eyeNormal;
    if (normalValue.a > 0.1)
    {
        vec3 tsNormal = normalize(2.0 * normalValue.rgb - vec3(1.0));
        eyeNormal2 = normalize(mat3(eyeTangent, eyeBinormal, eyeNormal) * tsNormal);
    }

    // MRT output
    VERSE_SCRIPT_FUNC(1);
#ifdef VERSE_GLES3
    fragData0/*NormalBuffer*/ = vec4(eyeNormal2.xyz, metalRough.r);
    fragData1/*DiffuseMetallicBuffer*/ = vec4(diffuse.rgb, metalRough.b);
    fragData2/*SpecularRoughnessBuffer*/ = vec4(specular, metalRough.g);
    fragData3/*EmissionBuffer*/ = vec4(emission);
#else
    gl_FragData[0]/*NormalBuffer*/ = vec4(eyeNormal2.xyz, metalRough.r);
    gl_FragData[1]/*DiffuseMetallicBuffer*/ = vec4(diffuse.rgb, metalRough.b);
    gl_FragData[2]/*SpecularRoughnessBuffer*/ = vec4(specular, metalRough.g);
    gl_FragData[3]/*EmissionBuffer*/ = vec4(emission);
#endif
}
