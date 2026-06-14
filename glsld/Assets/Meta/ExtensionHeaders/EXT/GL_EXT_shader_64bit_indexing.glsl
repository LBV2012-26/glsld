// GL_EXT_shader_64bit_indexing

// Pragma: #pragma shader_64bit_indexing
// Pragma: #pragma promote_uint32_indices
//
// Arrays gain a length64() method that returns int64_t.
// No standalone function declarations added.
// 64-bit integer types (int64_t, uint64_t) allowed for array indices.
// Cooperative matrix load/store and cooperative vector overloads
// with uint64_t element/offset parameters are added in their
// respective extension files.
