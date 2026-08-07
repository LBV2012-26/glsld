#version 460 core

// Conditional preprocessor robustness test matrix
// Goals:
// 1) Ensure no crash/out-of-bounds on incomplete expressions.
// 2) Ensure nested condition directives remain balanced/tolerant.
// 3) Provide obvious inactive regions for future inactive_regions_ validation.

#define A 1
#define B 0
#define C 3
#define FOO(x) ((x) + 1)
#define EMPTY

// ===== Group 1: Normal complete expressions =====
#if 1
int g1_active_1;
#else
int g1_inactive_1;
#endif

#if 0
int g1_inactive_2;
#else
int g1_active_2;
#endif

#if defined(A)
int g1_active_3;
#endif

#if defined(UNDEF_SYMBOL)
int g1_inactive_3;
#else
int g1_active_4;
#endif

#if A && !B
int g1_active_5;
#endif

#if (A + C * 2) > 3
int g1_active_6;
#endif

#if FOO(1) > 1
int g1_active_7;
#endif

// ===== Group 2: Incomplete expression tails (must not crash) =====
#if
int g2_body_1;
#endif

#if 1 +
int g2_body_2;
#endif

#if (1 + 2
int g2_body_3;
#endif

#if defined(
int g2_body_4;
#endif

#if defined(A
int g2_body_5;
#endif

#if defined(A) &&
int g2_body_6;
#endif

#if ? 1 : 0
int g2_body_7;
#endif

#if 1 ?
int g2_body_8;
#endif

#if 1 ? 2 :
int g2_body_9;
#endif

#if 1 ||
int g2_body_10;
#endif

#if 1 &&
int g2_body_11;
#endif

#if ~
int g2_body_12;
#endif

#if !
int g2_body_13;
#endif

#if ( )
int g2_body_14;
#endif

// ===== Group 3: Broken nesting forms (must not crash) =====
#if 1
int g3_a;
#elif
int g3_b;
#else
int g3_c;
#endif

#if 0
int g3_d;
#else
int g3_e;
#elif 1
int g3_f;
#endif

#if 1
int g3_g;
#else
int g3_h;
#else
int g3_i;
#endif

#if 1
int g3_j;
#endif
#endif

#elif 1
int g3_k;

// ===== Group 4: Nested blocks for future inactive_regions_ checks =====
// Expected inactive region candidates are marked with comments.
#if 0
// INACTIVE_EXPECT_BEGIN: R1
int r1_0;
#if 1
int r1_1;
#else
int r1_2;
#endif
// INACTIVE_EXPECT_END: R1
#else
int r1_active;
#endif

#if 1
int r2_active_0;
#if 0
// INACTIVE_EXPECT_BEGIN: R2
int r2_inactive_0;
int r2_inactive_1;
// INACTIVE_EXPECT_END: R2
#endif
int r2_active_1;
#endif

#if 0
// INACTIVE_EXPECT_BEGIN: R3
int r3_inactive_0;
#if 0
int r3_inactive_1;
#endif
// INACTIVE_EXPECT_END: R3
#endif

// ===== Group 5: Macro + condition corner cases =====
#if EMPTY
int g5_a;
#endif

#if defined EMPTY
int g5_b;
#endif

#if defined(EMPTY)
int g5_c;
#endif

#undef EMPTY

#if defined(EMPTY) || UNDEFINED_IDENTIFIER
int g5_d;
#endif

#if defined(A) ? 1 : 0
int g5_e;
#endif

#if (A << 2) > (C <<)
int g5_f;
#endif

// ===== Group 6: Line continuation and directive body capture =====
#if 1 + \
2
int g6_a;
#endif

#if defined(A) && \
(1 + )
int g6_b;
#endif

// ===== Group 7: Keep parser alive =====
void main() {
    int keep_alive = 1;
}
