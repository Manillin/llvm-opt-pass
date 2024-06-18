define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) {
  %b = add nsw i32 1  , 1
  %a = add nsw i32 %b , 1
  %c = sub nsw i32 %a , 1   ; sottrazione candidata
  %3 = mul nsw i32 %c , 4
  %4 = add nsw i32 %c , 3
  ret i32 %4
}