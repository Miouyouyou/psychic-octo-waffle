precision mediump float;

uniform sampler2D sid;

varying vec2 out_st;

void main() { gl_FragColor = texture2D(sid, out_st); }
