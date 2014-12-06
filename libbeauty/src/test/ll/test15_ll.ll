; ModuleID = 'test15_ll.bc'
; Function to test xor 8 bit

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@mem8a = common global i8 253, align 4
@mem8b = common global i8 8, align 4

; Function Attrs: optsize zeroext
define i8 @test15_ll() #0 {
  %a = load i8* @mem8a, align 4
  %b = load i8* @mem8b, align 4
  %c = xor i8 %a, %b
  ret i8 %c
}

attributes #0 = { optsize zeroext "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
