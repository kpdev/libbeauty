; ModuleID = 'test18_ll.ll'
; Must compile with clang -O2 so th load and the add get merged.
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@mem1 = internal global i32 291, align 4

; Function Attrs: nounwind uwtable
define i32 @test18_ll(i32 %value1) {
  %1 = load volatile i32* @mem1, align 4 
  %2 = add nsw i32 %1, %value1
  ret i32 %2
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

