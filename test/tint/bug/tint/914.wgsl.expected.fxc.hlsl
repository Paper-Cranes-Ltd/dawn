ByteAddressBuffer firstMatrix : register(t0, space0);
ByteAddressBuffer secondMatrix : register(t1, space0);
RWByteAddressBuffer resultMatrix : register(u2, space0);
cbuffer cbuffer_uniforms : register(b3, space0) {
  uint4 uniforms[1];
};

float mm_readA(uint row, uint col) {
  bool tint_tmp = (row < uniforms[0].x);
  if (tint_tmp) {
    tint_tmp = (col < uniforms[0].y);
  }
  if ((tint_tmp)) {
    const float result = asfloat(firstMatrix.Load((4u * ((row * uniforms[0].y) + col))));
    return result;
  }
  return 0.0f;
}

float mm_readB(uint row, uint col) {
  bool tint_tmp_1 = (row < uniforms[0].y);
  if (tint_tmp_1) {
    tint_tmp_1 = (col < uniforms[0].z);
  }
  if ((tint_tmp_1)) {
    const float result = asfloat(secondMatrix.Load((4u * ((row * uniforms[0].z) + col))));
    return result;
  }
  return 0.0f;
}

void mm_write(uint row, uint col, float value) {
  bool tint_tmp_2 = (row < uniforms[0].x);
  if (tint_tmp_2) {
    tint_tmp_2 = (col < uniforms[0].z);
  }
  if ((tint_tmp_2)) {
    const uint index = (col + (row * uniforms[0].z));
    resultMatrix.Store((4u * index), asuint(value));
  }
}

groupshared float mm_Asub[64][64];
groupshared float mm_Bsub[64][64];

struct tint_symbol_1 {
  uint3 local_id : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 global_id : SV_DispatchThreadID;
};

void main_inner(uint3 local_id, uint3 global_id, uint local_invocation_index) {
  {
    [loop] for(uint idx = local_invocation_index; (idx < 4096u); idx = (idx + 256u)) {
      const uint i = (idx / 64u);
      const uint i_1 = (idx % 64u);
      mm_Asub[i][i_1] = 0.0f;
      mm_Bsub[i][i_1] = 0.0f;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  const uint tileRow = (local_id.y * 4u);
  const uint tileCol = (local_id.x * 4u);
  const uint globalRow = (global_id.y * 4u);
  const uint globalCol = (global_id.x * 4u);
  const uint numTiles = (((uniforms[0].y - 1u) / 64u) + 1u);
  float acc[16] = (float[16])0;
  float ACached = 0.0f;
  float BCached[4] = (float[4])0;
  {
    [loop] for(uint index = 0u; (index < 16u); index = (index + 1u)) {
      acc[index] = 0.0f;
    }
  }
  const uint ColPerThreadA = (64u / 16u);
  const uint tileColA = (local_id.x * ColPerThreadA);
  const uint RowPerThreadB = (64u / 16u);
  const uint tileRowB = (local_id.y * RowPerThreadB);
  {
    [loop] for(uint t = 0u; (t < numTiles); t = (t + 1u)) {
      {
        [loop] for(uint innerRow = 0u; (innerRow < 4u); innerRow = (innerRow + 1u)) {
          {
            [loop] for(uint innerCol = 0u; (innerCol < ColPerThreadA); innerCol = (innerCol + 1u)) {
              const uint inputRow = (tileRow + innerRow);
              const uint inputCol = (tileColA + innerCol);
              const float tint_symbol_2 = mm_readA((globalRow + innerRow), ((t * 64u) + inputCol));
              mm_Asub[inputRow][inputCol] = tint_symbol_2;
            }
          }
        }
      }
      {
        [loop] for(uint innerRow = 0u; (innerRow < RowPerThreadB); innerRow = (innerRow + 1u)) {
          {
            [loop] for(uint innerCol = 0u; (innerCol < 4u); innerCol = (innerCol + 1u)) {
              const uint inputRow = (tileRowB + innerRow);
              const uint inputCol = (tileCol + innerCol);
              const float tint_symbol_3 = mm_readB(((t * 64u) + inputRow), (globalCol + innerCol));
              mm_Bsub[innerCol][inputCol] = tint_symbol_3;
            }
          }
        }
      }
      GroupMemoryBarrierWithGroupSync();
      {
        [loop] for(uint k = 0u; (k < 64u); k = (k + 1u)) {
          {
            [loop] for(uint inner = 0u; (inner < 4u); inner = (inner + 1u)) {
              BCached[inner] = mm_Bsub[k][(tileCol + inner)];
            }
          }
          {
            [loop] for(uint innerRow = 0u; (innerRow < 4u); innerRow = (innerRow + 1u)) {
              ACached = mm_Asub[(tileRow + innerRow)][k];
              {
                [loop] for(uint innerCol = 0u; (innerCol < 4u); innerCol = (innerCol + 1u)) {
                  const uint index = ((innerRow * 4u) + innerCol);
                  acc[index] = (acc[index] + (ACached * BCached[innerCol]));
                }
              }
            }
          }
        }
      }
      GroupMemoryBarrierWithGroupSync();
    }
  }
  {
    [loop] for(uint innerRow = 0u; (innerRow < 4u); innerRow = (innerRow + 1u)) {
      {
        [loop] for(uint innerCol = 0u; (innerCol < 4u); innerCol = (innerCol + 1u)) {
          const uint index = ((innerRow * 4u) + innerCol);
          mm_write((globalRow + innerRow), (globalCol + innerCol), acc[index]);
        }
      }
    }
  }
}

[numthreads(16, 16, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_id, tint_symbol.global_id, tint_symbol.local_invocation_index);
  return;
}
