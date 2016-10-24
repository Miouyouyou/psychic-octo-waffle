precision mediump float;

uniform sampler2D sid;

varying vec2 st;

void main() { gl_FragColor = texture2D(sid, st); }
