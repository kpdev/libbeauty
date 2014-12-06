; ModuleID = 'test14_ll.bc'
; Function to test bitcast

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@mem32 = common global i32 0, align 4

; Function Attrs: optsize zeroext
define i32 @test14_ll() #0 {
  %mem8 = bitcast i32* @mem32 to i8*
  store i32 8225, i32* @mem32, align 4
  store i8 18, i8* %mem8, align 1
  ret i32 0
}

attributes #0 = { optsize zeroext "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
