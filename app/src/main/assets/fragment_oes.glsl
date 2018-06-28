#extension GL_OES_EGL_image_external : require
precision mediump float;
uniform samplerExternalOES sTexture;
varying vec2 aCoord;
uniform float alpha;
void main() {
    vec3 rgb = texture2D(sTexture, aCoord);
    gl_FragColor = vec4(rgb, alpha);
}
