@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba32sint, write>;

fn textureDimensions_8efd47() {
  var res : u32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_8efd47();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_8efd47();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_8efd47();
}