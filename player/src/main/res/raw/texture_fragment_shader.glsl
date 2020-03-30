precision mediump float;

varying vec2 v_TextureCoordinates;

uniform sampler2D y_Sampler;
uniform sampler2D u_Sampler;
uniform sampler2D v_Sampler;

void main(){
    float y,u,v;
    y = texture2D(y_Sampler,v_TextureCoordinates).r;
    u = texture2D(u_Sampler,v_TextureCoordinates).r- 0.5;
    v = texture2D(v_Sampler,v_TextureCoordinates).r- 0.5;

    vec3 rgb;
    rgb.r = y + 1.403 * v;
    rgb.g = y - 0.344 * u - 0.714 * v;
    rgb.b = y + 1.770 * u;

    gl_FragColor = vec4(rgb,1);
}