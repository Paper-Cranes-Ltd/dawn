@group(1) @binding(0) var arg_0 : texture_depth_2d;

fn textureLoad_9ed19e() {
  var arg_1 = vec2<u32>();
  var arg_2 = 1u;
  var res : f32 = textureLoad(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_9ed19e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_9ed19e();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_9ed19e();
}