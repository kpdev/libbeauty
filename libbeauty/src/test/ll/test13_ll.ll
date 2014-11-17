; ModuleID = 'test13_ll.ll'
target datalayout = "e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @test13_ll(i32 %var1) #0 {
  %1 = alloca i32, align 4
  %n = alloca i32, align 4
  store i32 %var1, i32* %1, align 4
  store i32 0, i32* %n, align 4
  %2 = load i32* %1, align 4
  ret i32 %2
}

attributes #0 = { nounwind uwtable }
