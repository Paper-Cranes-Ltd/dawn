#version 310 es

void select_dfab3b() {
  bool arg_2 = true;
  vec3 res = (arg_2 ? vec3(1.0f) : vec3(1.0f));
}

vec4 vertex_main() {
  select_dfab3b();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

void select_dfab3b() {
  bool arg_2 = true;
  vec3 res = (arg_2 ? vec3(1.0f) : vec3(1.0f));
}

void fragment_main() {
  select_dfab3b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void select_dfab3b() {
  bool arg_2 = true;
  vec3 res = (arg_2 ? vec3(1.0f) : vec3(1.0f));
}

void compute_main() {
  select_dfab3b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}