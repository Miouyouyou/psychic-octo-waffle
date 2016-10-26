precision mediump float;

attribute vec2 xy;
attribute vec2 st;

varying vec2 out_st;

void main() {
  gl_Position = vec4(xy, 0.5, 1.0);
  out_st = st;
}
