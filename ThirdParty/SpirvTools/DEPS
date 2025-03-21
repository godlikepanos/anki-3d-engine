use_relative_paths = True

vars = {
  'github': 'https://github.com',

  'abseil_revision': '310e6f4f0f202da13720fdd6cb0095e10a98fe1c',

  'effcee_revision': '874b47102c57a8979c0f154cf8e0eab53c0a0502',

  'googletest_revision': '3af834740f58ef56058e6f8a1be51f62bc3a6515',

  # Use protobufs before they gained the dependency on abseil
  'protobuf_revision': 'v21.12',

  're2_revision': 'c84a140c93352cdabbfb547c531be34515b12228',

  'spirv_headers_revision': 'd5ee9ed2bbe96756a781bffb19c51d62a468049a',
}

deps = {
  'external/abseil_cpp':
      Var('github') + '/abseil/abseil-cpp.git@' + Var('abseil_revision'),

  'external/effcee':
      Var('github') + '/google/effcee.git@' + Var('effcee_revision'),

  'external/googletest':
      Var('github') + '/google/googletest.git@' + Var('googletest_revision'),

  'external/protobuf':
      Var('github') + '/protocolbuffers/protobuf.git@' + Var('protobuf_revision'),

  'external/re2':
      Var('github') + '/google/re2.git@' + Var('re2_revision'),

  'external/spirv-headers':
      Var('github') +  '/KhronosGroup/SPIRV-Headers.git@' +
          Var('spirv_headers_revision'),
}

