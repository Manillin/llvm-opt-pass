define i32 @testSR(i32 noundef %0) {
  %2 = mul i32 %0, 8
  %3 = add i32 %2, 1
  %4 = mul i32 8, %0
  %d = udiv i32 %2, 32
  %5 = add i32 %4, 1
  %6 = mul i32 15, %0
  ret i32 %6
}