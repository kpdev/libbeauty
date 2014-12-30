; ModuleID = 'test17_ll.ll'
; Tests phi node onto itself. The phi is also a pointer.
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@global1 = common global i32 253, align 4

; Function Attrs: nounwind uwtable
define i32* @test17_ll(i32* %var1) #0 {
  %n = getelementptr i32* @global1, i32 0
  br label %1

; <label>:1                                       ; preds = %1
  %n2 = phi i32* [ %n, %0 ], [ %n3, %1 ]
  %n3 = getelementptr i32* %n2, i32 1
  %2 = icmp slt i32* %n3, %var1
  br i1 %2, label %1, label %3

; <label>:3                                       ; preds = %1, %0
  ret i32* %n3
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"Ubuntu clang version 3.5.0-4ubuntu2 (tags/RELEASE_350/final) (based on LLVM 3.5.0)"}
