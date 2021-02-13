#pragma once

#define MA_CONCAT(a, b) a ## b
#define MA_CONCAT2(a, b) MA_CONCAT(a, b)

#define THIRTYSECOND_ARGUMENT(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, ...) a32
#define COUNT_ARGUMENTS(...) THIRTYSECOND_ARGUMENT(dummy, ## __VA_ARGS__, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define REMOVE_LAST_ARGUMENT(...) MA_CONCAT2(MA_REMOVE_LAST_ARG_, COUNT_ARGUMENTS(__VA_ARGS__))(__VA_ARGS__)

#define MA_REMOVE_LAST_ARG_1(a1) \

#define MA_REMOVE_LAST_ARG_2(a1, a2) \
  a1
#define MA_REMOVE_LAST_ARG_3(a1, a2, a3) \
  a1, a2
#define MA_REMOVE_LAST_ARG_4(a1, a2, a3, a4) \
  a1, a2, a3
#define MA_REMOVE_LAST_ARG_5(a1, a2, a3, a4, a5) \
  a1, a2, a3, a4
#define MA_REMOVE_LAST_ARG_6(a1, a2, a3, a4, a5, a6) \
  a1, a2, a3, a4, a5
#define MA_REMOVE_LAST_ARG_7(a1, a2, a3, a4, a5, a6, a7) \
  a1, a2, a3, a4, a5, a6
#define MA_REMOVE_LAST_ARG_8(a1, a2, a3, a4, a5, a6, a7, a8) \
  a1, a2, a3, a4, a5, a6, a7
#define MA_REMOVE_LAST_ARG_9(a1, a2, a3, a4, a5, a6, a7, a8, a9) \
  a1, a2, a3, a4, a5, a6, a7, a8
#define MA_REMOVE_LAST_ARG_10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9
#define MA_REMOVE_LAST_ARG_11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10
#define MA_REMOVE_LAST_ARG_12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11
#define MA_REMOVE_LAST_ARG_13(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12
#define MA_REMOVE_LAST_ARG_14(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13
#define MA_REMOVE_LAST_ARG_15(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14
#define MA_REMOVE_LAST_ARG_16(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15
#define MA_REMOVE_LAST_ARG_17(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16
#define MA_REMOVE_LAST_ARG_18(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17
#define MA_REMOVE_LAST_ARG_19(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18
#define MA_REMOVE_LAST_ARG_20(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19
#define MA_REMOVE_LAST_ARG_21(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20
#define MA_REMOVE_LAST_ARG_22(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21
#define MA_REMOVE_LAST_ARG_23(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22
#define MA_REMOVE_LAST_ARG_24(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23
#define MA_REMOVE_LAST_ARG_25(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24
#define MA_REMOVE_LAST_ARG_26(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25
#define MA_REMOVE_LAST_ARG_27(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26
#define MA_REMOVE_LAST_ARG_28(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27
#define MA_REMOVE_LAST_ARG_29(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28
#define MA_REMOVE_LAST_ARG_30(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30) \
  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29
