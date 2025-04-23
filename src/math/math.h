#pragma once
#include "../includes.h"

void math_orthographic_f4x4(float* matrix, float left_f, float right_f, float bottom_f, float top_f, float near_f, float far_f);
void math_translate_f4x4(float* matrix, float x, float y, float z);
void math_identity_f4x4(float* matrix, float v);
void math_multiply_f4x4(float* dest, const float* mat1, const float* mat2);
void math_scale_f4x4(float* matrix, float sx, float sy, float sz);