// Copyright 2019 Google LLC
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stddef.h>

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/shift_test.cc"
#include "hwy/foreach_target.h"  // IWYU pragma: keep
#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"

HWY_BEFORE_NAMESPACE();
namespace hwy {
namespace HWY_NAMESPACE {

template <bool kSigned>
struct TestLeftShifts {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T t, D d) {
    if (kSigned) {
      // Also test positive values
      TestLeftShifts</*kSigned=*/false>()(t, d);
    }

    using TI = MakeSigned<T>;
    using TU = MakeUnsigned<T>;
    const size_t N = Lanes(d);
    auto expected = AllocateAligned<T>(N);
    HWY_ASSERT(expected);

    // Values to shift
    const auto values = Iota(d, kSigned ? -TI(N) : TI(0));
    constexpr size_t kMaxShift = (sizeof(T) * 8) - 1;

    // 0
    HWY_ASSERT_VEC_EQ(d, values, ShiftLeft<0>(values));
    HWY_ASSERT_VEC_EQ(d, values, ShiftLeftSame(values, 0));

    // 1
    for (size_t i = 0; i < N; ++i) {
      const T value =
          kSigned ? static_cast<T>(static_cast<T>(i) - static_cast<T>(N))
                  : static_cast<T>(i);
      expected[i] = static_cast<T>(static_cast<TU>(value) << 1);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftLeft<1>(values));
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftLeftSame(values, 1));

    // max
    for (size_t i = 0; i < N; ++i) {
      const T value =
          kSigned ? static_cast<T>(static_cast<T>(i) - static_cast<T>(N))
                  : static_cast<T>(i);
      expected[i] = static_cast<T>(static_cast<TU>(value) << kMaxShift);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftLeft<kMaxShift>(values));
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftLeftSame(values, kMaxShift));
  }
};

template <bool kSigned>
struct TestVariableLeftShifts {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T t, D d) {
    if (kSigned) {
      // Also test positive values
      TestVariableLeftShifts</*kSigned=*/false>()(t, d);
    }

    using TI = MakeSigned<T>;
    using TU = MakeUnsigned<T>;
    const size_t N = Lanes(d);
    auto expected = AllocateAligned<T>(N);
    HWY_ASSERT(expected);

    const auto v0 = Zero(d);
    const auto v1 = Set(d, 1);
    const auto values = Iota(d, kSigned ? -TI(N) : TI(0));  // value to shift

    constexpr size_t kMaxShift = (sizeof(T) * 8) - 1;
    const auto max_shift = Set(d, kMaxShift);
    const auto small_shifts = And(Iota(d, 0), max_shift);
    const auto large_shifts = Sub(max_shift, small_shifts);

    // Same: 0
    HWY_ASSERT_VEC_EQ(d, values, Shl(values, v0));

    // Same: 1
    for (size_t i = 0; i < N; ++i) {
      const T value =
          kSigned ? static_cast<T>(static_cast<T>(i) - static_cast<T>(N))
                  : static_cast<T>(i);
      expected[i] = static_cast<T>(static_cast<TU>(value) << 1);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shl(values, v1));

    // Same: max
    for (size_t i = 0; i < N; ++i) {
      const T value =
          kSigned ? static_cast<T>(static_cast<T>(i) - static_cast<T>(N))
                  : static_cast<T>(i);
      expected[i] = static_cast<T>(static_cast<TU>(value) << kMaxShift);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shl(values, max_shift));

    // Variable: small
    for (size_t i = 0; i < N; ++i) {
      const T value =
          kSigned ? static_cast<T>(static_cast<T>(i) - static_cast<T>(N))
                  : static_cast<T>(i);
      expected[i] = static_cast<T>(static_cast<TU>(value) << (i & kMaxShift));
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shl(values, small_shifts));

    // Variable: large
    for (size_t i = 0; i < N; ++i) {
      expected[i] =
          static_cast<T>(static_cast<TU>(1) << (kMaxShift - (i & kMaxShift)));
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shl(v1, large_shifts));
  }
};

struct TestUnsignedRightShifts {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const size_t N = Lanes(d);
    auto expected = AllocateAligned<T>(N);
    HWY_ASSERT(expected);

    const auto values = Iota(d, 0);

    const T kMax = LimitsMax<T>();
    constexpr size_t kMaxShift = (sizeof(T) * 8) - 1;

    // Shift by 0
    HWY_ASSERT_VEC_EQ(d, values, ShiftRight<0>(values));
    HWY_ASSERT_VEC_EQ(d, values, ShiftRightSame(values, 0));

    // Shift by 1
    for (size_t i = 0; i < N; ++i) {
      expected[i] = static_cast<T>(static_cast<T>(i & kMax) >> 1);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftRight<1>(values));
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftRightSame(values, 1));

    // max
    for (size_t i = 0; i < N; ++i) {
      expected[i] = static_cast<T>(static_cast<T>(i & kMax) >> kMaxShift);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftRight<kMaxShift>(values));
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftRightSame(values, kMaxShift));
  }
};

struct TestVariableUnsignedRightShifts {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const size_t N = Lanes(d);
    auto expected = AllocateAligned<T>(N);
    HWY_ASSERT(expected);

    const auto v0 = Zero(d);
    const auto v1 = Set(d, 1);
    const auto values = Iota(d, 0);

    const T kMax = LimitsMax<T>();
    const auto max = Set(d, kMax);

    constexpr size_t kMaxShift = (sizeof(T) * 8) - 1;
    const auto max_shift = Set(d, kMaxShift);
    const auto small_shifts = And(Iota(d, 0), max_shift);
    const auto large_shifts = Sub(max_shift, small_shifts);

    // Same: 0
    HWY_ASSERT_VEC_EQ(d, values, Shr(values, v0));

    // Same: 1
    for (size_t i = 0; i < N; ++i) {
      expected[i] = static_cast<T>(static_cast<T>(i & kMax) >> 1);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shr(values, v1));

    // Same: max
    for (size_t i = 0; i < N; ++i) {
      expected[i] = static_cast<T>(static_cast<T>(i & kMax) >> kMaxShift);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shr(values, max_shift));

    // Variable: small
    for (size_t i = 0; i < N; ++i) {
      expected[i] = ConvertScalarTo<T>(T(i) >> (i & kMaxShift));
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shr(values, small_shifts));

    // Variable: Large
    for (size_t i = 0; i < N; ++i) {
      expected[i] = ConvertScalarTo<T>(kMax >> (kMaxShift - (i & kMaxShift)));
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shr(max, large_shifts));
  }
};

template <int kAmount, typename T>
T RightShiftNegative(T val) {
  // C++ shifts are implementation-defined for negative numbers, and we have
  // seen divisions replaced with shifts, so resort to bit operations.
  using TU = hwy::MakeUnsigned<T>;
  TU bits;
  CopySameSize(&val, &bits);

  const TU shifted = TU(bits >> kAmount);

  const TU all = TU(~TU(0));
  const size_t num_zero = sizeof(TU) * 8 - 1 - kAmount;
  const TU sign_extended = static_cast<TU>((all << num_zero) & LimitsMax<TU>());

  bits = shifted | sign_extended;
  CopySameSize(&bits, &val);
  return val;
}

class TestSignedRightShifts {
 public:
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const size_t N = Lanes(d);
    auto expected = AllocateAligned<T>(N);
    HWY_ASSERT(expected);
    constexpr T kMin = LimitsMin<T>();
    constexpr T kMax = LimitsMax<T>();
    constexpr size_t kMaxShift = (sizeof(T) * 8) - 1;

    // First test positive values, negative are checked below.
    const auto v0 = Zero(d);
    const auto values = And(Iota(d, 0), Set(d, kMax));

    // Shift by 0
    HWY_ASSERT_VEC_EQ(d, values, ShiftRight<0>(values));
    HWY_ASSERT_VEC_EQ(d, values, ShiftRightSame(values, 0));

    // Shift by 1
    for (size_t i = 0; i < N; ++i) {
      expected[i] = static_cast<T>(static_cast<T>(i & kMax) >> 1);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftRight<1>(values));
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftRightSame(values, 1));

    // max
    HWY_ASSERT_VEC_EQ(d, v0, ShiftRight<kMaxShift>(values));
    HWY_ASSERT_VEC_EQ(d, v0, ShiftRightSame(values, kMaxShift));

    // Even negative value
    Test<0>(kMin, d, __LINE__);
    Test<1>(kMin, d, __LINE__);
    Test<2>(kMin, d, __LINE__);
    Test<kMaxShift>(kMin, d, __LINE__);

    const T odd = ConvertScalarTo<T>(kMin + 1);
    Test<0>(odd, d, __LINE__);
    Test<1>(odd, d, __LINE__);
    Test<2>(odd, d, __LINE__);
    Test<kMaxShift>(odd, d, __LINE__);
  }

 private:
  template <int kAmount, typename T, class D>
  void Test(T val, D d, int line) {
    const auto expected = Set(d, RightShiftNegative<kAmount>(val));
    const auto in = Set(d, val);
    const char* file = __FILE__;
    AssertVecEqual(d, expected, ShiftRight<kAmount>(in), file, line);
    AssertVecEqual(d, expected, ShiftRightSame(in, kAmount), file, line);
  }
};

struct TestVariableSignedRightShifts {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    using TU = MakeUnsigned<T>;
    const size_t N = Lanes(d);
    auto expected = AllocateAligned<T>(N);
    HWY_ASSERT(expected);

    constexpr T kMin = LimitsMin<T>();
    constexpr T kMax = LimitsMax<T>();

    constexpr size_t kMaxShift = (sizeof(T) * 8) - 1;

    // First test positive values, negative are checked below.
    const auto v0 = Zero(d);
    const auto positive = And(Iota(d, 0), Set(d, kMax));

    // Shift by 0
    HWY_ASSERT_VEC_EQ(d, positive, ShiftRight<0>(positive));
    HWY_ASSERT_VEC_EQ(d, positive, ShiftRightSame(positive, 0));

    // Shift by 1
    for (size_t i = 0; i < N; ++i) {
      expected[i] = static_cast<T>(static_cast<T>(i & kMax) >> 1);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftRight<1>(positive));
    HWY_ASSERT_VEC_EQ(d, expected.get(), ShiftRightSame(positive, 1));

    // max
    HWY_ASSERT_VEC_EQ(d, v0, ShiftRight<kMaxShift>(positive));
    HWY_ASSERT_VEC_EQ(d, v0, ShiftRightSame(positive, kMaxShift));

    const auto max_shift = Set(d, kMaxShift);
    const auto small_shifts = And(Iota(d, 0), max_shift);
    const auto large_shifts = Sub(max_shift, small_shifts);

    const auto negative = Iota(d, kMin);

    // Test varying negative to shift
    for (size_t i = 0; i < N; ++i) {
      const T val = ConvertScalarTo<T>(static_cast<TU>(kMin) + i);
      expected[i] =
          (val < 0) ? RightShiftNegative<1>(val) : ConvertScalarTo<T>(val >> 1);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shr(negative, Set(d, 1)));

    // Shift MSB right by small amounts
    for (size_t i = 0; i < N; ++i) {
      const size_t amount = i & kMaxShift;
      const TU shifted = static_cast<TU>(~((1ull << (kMaxShift - amount)) - 1));
      CopySameSize(&shifted, &expected[i]);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shr(Set(d, kMin), small_shifts));

    // Shift MSB right by large amounts
    for (size_t i = 0; i < N; ++i) {
      const size_t amount = kMaxShift - (i & kMaxShift);
      const TU shifted = static_cast<TU>(~((1ull << (kMaxShift - amount)) - 1));
      CopySameSize(&shifted, &expected[i]);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), Shr(Set(d, kMin), large_shifts));
  }
};

HWY_NOINLINE void TestAllShifts() {
  ForUnsignedTypes(ForPartialVectors<TestLeftShifts</*kSigned=*/false>>());
  ForSignedTypes(ForPartialVectors<TestLeftShifts</*kSigned=*/true>>());
  ForUnsignedTypes(ForPartialVectors<TestUnsignedRightShifts>());
  ForSignedTypes(ForPartialVectors<TestSignedRightShifts>());
}

HWY_NOINLINE void TestAllVariableShifts() {
  ForUnsignedTypes(
      ForPartialVectors<TestVariableLeftShifts</*kSigned=*/false>>());
  ForSignedTypes(ForPartialVectors<TestVariableLeftShifts</*kSigned=*/true>>());
  ForUnsignedTypes(ForPartialVectors<TestVariableUnsignedRightShifts>());
  ForSignedTypes(ForPartialVectors<TestVariableSignedRightShifts>());
}

struct TestRoundingShiftRight {
  template <int kShiftAmt, class D>
  static HWY_INLINE void VerifyRoundingShiftRight(
      D d, const TFromD<D>* HWY_RESTRICT expected, Vec<D> v,
      const char* filename, const int line) {
    AssertVecEqual(d, expected, RoundingShiftRight<kShiftAmt>(v), filename,
                   line);
    AssertVecEqual(d, expected, RoundingShiftRightSame(v, kShiftAmt), filename,
                   line);
  }

  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    using TU = MakeUnsigned<T>;
    const auto iota0 = Iota(d, T{0});

    const size_t N = Lanes(d);
    auto expected = AllocateAligned<T>(N);
    HWY_ASSERT(expected);

    Store(iota0, d, expected.get());
    VerifyRoundingShiftRight<0>(d, expected.get(), iota0, __FILE__, __LINE__);

    const auto v1 = Set(d, T{1});
    const auto iota1 = Add(iota0, v1);
    Store(iota1, d, expected.get());
    VerifyRoundingShiftRight<0>(d, expected.get(), iota1, __FILE__, __LINE__);

    const auto v2 = Set(d, T{2});
    const auto iota2 = Add(iota0, v2);
    Store(iota2, d, expected.get());
    VerifyRoundingShiftRight<0>(d, expected.get(), iota2, __FILE__, __LINE__);

    const auto iota3 = Add(iota0, Set(d, T{3}));
    Store(iota3, d, expected.get());
    VerifyRoundingShiftRight<0>(d, expected.get(), iota3, __FILE__, __LINE__);

    const auto seq4 = Add(iota0, SignBit(d));
    Store(seq4, d, expected.get());
    VerifyRoundingShiftRight<0>(d, expected.get(), seq4, __FILE__, __LINE__);

    const auto seq5 = Add(seq4, v1);
    Store(seq5, d, expected.get());
    VerifyRoundingShiftRight<0>(d, expected.get(), seq5, __FILE__, __LINE__);

    const auto v0 = Zero(d);
    Store(AverageRound(iota1, v0), d, expected.get());
    VerifyRoundingShiftRight<1>(d, expected.get(), iota1, __FILE__, __LINE__);

    Store(AverageRound(iota2, v0), d, expected.get());
    VerifyRoundingShiftRight<1>(d, expected.get(), iota2, __FILE__, __LINE__);

    Store(AverageRound(seq4, v0), d, expected.get());
    VerifyRoundingShiftRight<1>(d, expected.get(), seq4, __FILE__, __LINE__);

    Store(AverageRound(seq5, v0), d, expected.get());
    VerifyRoundingShiftRight<1>(d, expected.get(), seq5, __FILE__, __LINE__);

    const auto seq6 = And(
        Xor(iota1,
            Set(d, static_cast<T>(0x70FB991A05AC6B24ULL & LimitsMax<TU>()))),
        Set(d, static_cast<T>(~static_cast<T>(0x10))));
    Store(ShiftRight<5>(seq6), d, expected.get());
    VerifyRoundingShiftRight<5>(d, expected.get(), seq6, __FILE__, __LINE__);

    const auto seq7 =
        Or(Xor(iota2,
               Set(d, static_cast<T>(0x6ED498B16EC87C63ULL & LimitsMax<TU>()))),
           Set(d, static_cast<T>(0x04)));
    Store(Add(ShiftRight<3>(seq7), v1), d, expected.get());
    VerifyRoundingShiftRight<3>(d, expected.get(), seq7, __FILE__, __LINE__);

    const auto seq8 = And(
        Xor(iota1,
            Set(d, static_cast<T>(0x186958FE04C94D77ULL & LimitsMax<TU>()))),
        Set(d, static_cast<T>(~static_cast<T>(0x08))));
    Store(ShiftRight<4>(seq8), d, expected.get());
    VerifyRoundingShiftRight<4>(d, expected.get(), seq8, __FILE__, __LINE__);

    const auto seq9 =
        Or(Xor(iota2,
               Set(d, static_cast<T>(0x7FC4E62077CC7655ULL & LimitsMax<TU>()))),
           v2);
    Store(Add(ShiftRight<2>(seq9), v1), d, expected.get());
    VerifyRoundingShiftRight<2>(d, expected.get(), seq9, __FILE__, __LINE__);
  }
};

HWY_NOINLINE void TestAllRoundingShiftRight() {
  ForIntegerTypes(ForPartialVectors<TestRoundingShiftRight>());
}

struct TestVariableRoundingShr {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const size_t N = Lanes(d);
    constexpr size_t kNumOfBits = sizeof(T) * 8;
    const auto iota1 = Iota(d, T{1});

    const auto v0 = Zero(d);
    const auto v1 = Set(d, T{1});
    const auto sign_bit = SignBit(d);

    for (size_t i = 0; i < kNumOfBits; i += N) {
      auto shift_amt = Iota(d, static_cast<T>(i & (kNumOfBits - 1)));

      HWY_IF_CONSTEXPR(HWY_MAX_LANES_D(D) > kNumOfBits) {
        shift_amt = And(shift_amt, Set(d, static_cast<T>(kNumOfBits - 1)));
      }

      const auto half_bit = ShiftRight<1>(Shl(v1, shift_amt));

      const auto in_0 = AndNot(half_bit, Or(Shl(iota1, shift_amt), v1));
      const auto in_1 = Or(in_0, half_bit);
      const auto in_2 = Xor(in_0, sign_bit);
      const auto in_3 = Xor(in_1, sign_bit);

      const auto round_decr = VecFromMask(d, Ne(half_bit, v0));

      const auto expected_0 = Shr(in_0, shift_amt);
      const auto expected_1 = Sub(Shr(in_1, shift_amt), round_decr);
      const auto expected_2 = Shr(in_2, shift_amt);
      const auto expected_3 = Sub(Shr(in_3, shift_amt), round_decr);

      HWY_ASSERT_VEC_EQ(d, expected_0, RoundingShr(in_0, shift_amt));
      HWY_ASSERT_VEC_EQ(d, expected_1, RoundingShr(in_1, shift_amt));
      HWY_ASSERT_VEC_EQ(d, expected_2, RoundingShr(in_2, shift_amt));
      HWY_ASSERT_VEC_EQ(d, expected_3, RoundingShr(in_3, shift_amt));
    }
  }
};

HWY_NOINLINE void TestAllVariableRoundingShr() {
  ForIntegerTypes(ForPartialVectors<TestVariableRoundingShr>());
}

struct TestMaskedShiftOrZero {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const MFromD<D> all_true = MaskTrue(d);
    const Vec<D> v0 = Zero(d);
    const auto v1 = Iota(d, 1);
    const MFromD<D> first_five = FirstN(d, 5);

    HWY_ASSERT_VEC_EQ(d, ShiftLeft<1>(v1), MaskedShiftLeftOrZero<1>(all_true, v1));
    HWY_ASSERT_VEC_EQ(d, ShiftRight<1>(v1), MaskedShiftRightOrZero<1>(all_true, v1));

    const Vec<D> v1_exp_left = IfThenElse(first_five, ShiftLeft<1>(v1), v0);
    HWY_ASSERT_VEC_EQ(d, v1_exp_left, MaskedShiftLeftOrZero<1>(first_five, v1));

    const Vec<D> v1_exp_right = IfThenElse(first_five, ShiftRight<1>(v1), v0);
    HWY_ASSERT_VEC_EQ(d, v1_exp_right, MaskedShiftRightOrZero<1>(first_five, v1));
  }
};
struct TestMaskedShiftRightOr {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    // Test MaskedShiftRightOr
    const auto v1 = Iota(d, 1);
    const auto v2 = Iota(d, 2);
    const MFromD<D> first_five = FirstN(d, 5);

    const Vec<D> expected = IfThenElse(first_five, ShiftRight<1>(v2), v1);
    HWY_ASSERT_VEC_EQ(d, expected, MaskedShiftRightOr<1>(v1, first_five, v2));
  }
};

struct TestMaskedShrOr {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    // Test MaskedShrOr
    const auto v1 = Iota(d, 1);
    const auto v2 = Iota(d, 2);
    const auto shifts = Set(d, 1);
    const MFromD<D> first_five = FirstN(d, 5);

    const Vec<D> expected = IfThenElse(first_five, ShiftRight<1>(v2), v1);
    HWY_ASSERT_VEC_EQ(d, expected, MaskedShrOr(v1, first_five, v2, shifts));
  }
};

HWY_NOINLINE void TestAllMaskedShift() {
  ForIntegerTypes(ForPartialVectors<TestMaskedShiftOrZero>());
  ForIntegerTypes(ForPartialVectors<TestMaskedShiftRightOr>());
  ForSignedTypes(ForPartialVectors<TestMaskedShrOr>());
}

struct TestMultiShift {
  uint64_t byte_swap_64(uint64_t x) {
    // Equivalent to std::byteswap for uint64 (in C++23)
    return  ((x << 56) & 0xff00000000000000ULL) |
            ((x << 40) & 0x00ff000000000000ULL) |
            ((x << 24) & 0x0000ff0000000000ULL) |
            ((x << 8)  & 0x000000ff00000000ULL) |
            ((x >> 8)  & 0x00000000ff000000ULL) |
            ((x >> 24) & 0x0000000000ff0000ULL) |
            ((x >> 40) & 0x000000000000ff00ULL) |
            ((x >> 56) & 0x00000000000000ffULL);
  }

  template <class T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const Repartition<uint8_t, decltype(d)> du8;
    const size_t N = Lanes(d);
    if (N < 2) return;

    // Generate a vector where all bytes in a block are different
    const uint64_t initial_even = 0x0102030405060708ul;
    const uint64_t initial_odd = 0x1020304050607080ul;
    const auto v1 = Dup128VecFromValues(d, initial_even, initial_odd);

    // Test byte aligned shifts
    // First 8 values define the transformation for even lanes
    // Second 8 values define the transformation for odd lanes
    auto indices = Dup128VecFromValues(du8,
      0, 8, 16, 24, 32, 40, 48, 56,  // Return every byte to its original location
      56, 48, 40, 32, 24, 16, 8, 0  // Reverse byte order
    );
    auto expected = AllocateAligned<T>(N);
    HWY_ASSERT(expected);

    for (size_t i = 0; i < N; i += 2) {
      expected[i] = ConvertScalarTo<T>(initial_even);
      expected[i + 1] = ConvertScalarTo<T>(byte_swap_64(initial_odd));
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), MultiShift(v1, indices));

    // Test bit level shifts, different amounts for each byte
    const auto v2 = Set(d, 0x0102010201020102ul);
    indices = Dup128VecFromValues(du8,  // Let j = i % 8
      0, 9, 18, 27, 36, 45, 54, 63,  // Shifting right: r[i].byte[j] = (v[j..j+1] >> j) & 0xff
      0, 7, 14, 21, 28, 35, 42, 49  // Shifting left: r[i].byte[j] = ((v[j-1..j] << j) >> 8) & 0xff
    );

    for (size_t i = 0; i < N; ++i) {
      const T v_i = ExtractLane(v2, i);
      T shift_result = 0;
      for(size_t j = 0; j < 8; j++) {
        uint8_t idx = ExtractLane(indices, (i * 8) + j);
        T rot_result = (v_i >> idx) | (v_i << (64 - idx));
        shift_result |= (rot_result & 0xff) << (j * 8);
      }
      expected[i] = ConvertScalarTo<T>(shift_result);
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), MultiShift(v2, indices));

    // Combine byte-level reordering with bit level shift
    indices = Dup128VecFromValues(du8,
      4, 12, 20, 28, 36, 44, 52, 60,  // Shift each byte right 4 bits
      60, 52, 44, 36, 28, 20, 12, 4  // Shift each byte right 4 bits then reverse byte order
    );

    for (size_t i = 0; i < N; i += 2) {
      expected[i] = ConvertScalarTo<T>((initial_even >> 4) | (initial_even << (64 - 4)));

      uint64_t unreversed_val = ConvertScalarTo<T>((initial_odd >> 4) | (initial_odd << (64 - 4)));
      expected[i + 1] = ConvertScalarTo<T>(byte_swap_64(unreversed_val));
    }
    HWY_ASSERT_VEC_EQ(d, expected.get(), MultiShift(v1, indices));
  }
};

HWY_NOINLINE void TestAllMultiShift() {
#if HWY_HAVE_INTEGER64
  const ForGEVectors<128, TestMultiShift> test64;
  test64(uint64_t());
  test64(int64_t());
#endif
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
}  // namespace HWY_NAMESPACE
}  // namespace hwy
HWY_AFTER_NAMESPACE();

#if HWY_ONCE

namespace hwy {
HWY_BEFORE_TEST(HwyShiftTest);
HWY_EXPORT_AND_TEST_P(HwyShiftTest, TestAllShifts);
HWY_EXPORT_AND_TEST_P(HwyShiftTest, TestAllVariableShifts);
HWY_EXPORT_AND_TEST_P(HwyShiftTest, TestAllRoundingShiftRight);
HWY_EXPORT_AND_TEST_P(HwyShiftTest, TestAllVariableRoundingShr);
HWY_EXPORT_AND_TEST_P(HwyShiftTest, TestAllMaskedShift);
HWY_EXPORT_AND_TEST_P(HwyShiftTest, TestAllMultiShift);
HWY_AFTER_TEST();
}  // namespace hwy

#endif
