precision mediump float;

attribute vec4 xyst;

varying vec2 st;

void main() {
  gl_Position = vec4(xyst.xy, 0.5, 1.0);
  st = xyst.zw;
}
