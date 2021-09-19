
`_VPLY`: Play audio
```
_VPLY(filename, ??)
```

`_WKST`: ?? Some sort of wait?
```
_WKST(F858,eq,0):
```

`_STCC`: ?? Number is generally low (< 50)
```
_STCC(int):
```

`_WNTY`: Some sort of Boolean operation.
Seems that argument is 1 when this is the line is at the end of a page, otherwise 0
```
_WNTY([0:1])
```

`_STCH`: Some sort of image load routine.
```
_STCH(8,img2511,413,225,1000,0,3102,,CC,,,)
_STCH(9,img2511,314,238,1000,0,3018,,CC,11,11,)
```

`_STCP`: ??
```
_STCP(uint, uint):
```

`_WKAD`: ?? Flag modification?
```
_WKAD(F823,1):
```

`_FADS`: Fade in/out?
```
_FADS(1,0,0): ??
```

`_STZ4`: ??
```
_STZ4(15,720,720,720,720,0,0,0)
_STZ4(16,-1000,1000,-1000,1000,0,0,0)
```

`_STMA`: ??
```
_STMA(23,0)
_STMA(20,24014)
```

`_STRT`: ?
```
_STRT(0,0,0,0,0)
_STRT(32,0,0,0,0)
_STRT(1,0,0,0,0)
_STRT(0,0,0,0,0)
```

`_PGST`: Move relative page number?
```
_PGST(316)
_PGST(-1)
```

`_STBR`: ?
```
_STBR(35,24)
_STBR(39,17,,1)
```

`_SEPL`: ?
```
_SEPL(0,0,WOH_SE12027,,`017:100)
_SEPL(1,0,MOON_SE02070,,`017:100)
_SEPL(2,0,MOON_SEETC01B,,`017:100)
```

`_ZM`: Text from memory addr?
$ address seems to be an index into the script_text file
Address can be followed by modifiers such as `@k`, `@e`, `@n`, `@x`
```
_ZMb961f($043578)
_ZMb9b02($043579^@n)
_ZMe3102(ボリュームテスト)
_ZMe3103(音量テスト^　BGM100^　SE100^　)
```

`_ZZ`: Load tag?
```
_ZZe5d01(SEL12_01_CIEL12_3_2B)
_ZZe5e01(SEL12_01_CIEL12_4)
_ZZe5f01(SEL12_01_CIEL12_4B)
_ZZe6001(SEL12_01_CIEL12_5)
```

`_ZY`: ?
```
_ZYe4800(LV01b00)
_ZYe4a00(LV01c00)
_ZYe4c00(LV01d00)
_ZYa7100(LV00000)
_ZYa7600(LV00000)
```

Non _Z command freqency:
```
 481982 _WKST
  62293 _STCC
  58052 _WNTY
  54049 _STCH
  53605 _STCP
  45272 _WKAD
  34192 _FADS // Fade
  32138 _STZ4
  24237 _STMA
  21688 _STRT
  17042 _PGST
  16550 _VPLY
  14935 _STBR
  14360 _SEPL
  14319 _WTTM // Wait timer
  12853 _STEC
  11530 _CLO3 // Clear text
   8757 _SNX4
   7166 _STGS
   5048 _STCF
   4771 _SCH2 // Image display
   3362 _SEFD
   2922 _FANX
   1765 _JUMP
   1636 _SNX2
   1342 _SEVL
   1222 _SQK2
   1171 _MSAD
   1164 _IF__
   1153 _MPLY
    953 _CLO4
    933 _SBAS
    865 _SHOK
    840 _SNX3
    823 _SELR
    775 _MVOL
    722 _MFAD
    712 _STPV
    591 _CLO5
    583 _STRA
    549 _RDED
    543 _RTM_
    529 _RDST
    529 _RDAS
    526 _RTST
    447 _STRR
    397 _ATSV
    396 _SHAF
    205 _SSW2
    195 _SPMA
    194 _STNS
    170 _SELD
    170 _GOTS
    131 _SQS2
    120 _SHK2
    101 _GOTO
    100 _SGMA
     95 _SACL
     78 _SOXY
     71 _VCWT
     57 _FADE
     53 _IFNX
     52 _IFP4
     48 _TOMW
     48 _TOMP
     41 _SPRD
     40 _SPRC
     39 _STCG
     31 _FCSP
     30 _MSQK
     28 _WTSV
     26 _SELA
     26 _FCLN
     25 _STTR
     20 _WKOR
     16 _VPL2
     15 _SESP
      8 _STCL
      5 _STQK
      5 _CLO2
      3 _SHS2
      2 _XSTP
      2 _XRSM
      2 _TRPY
      2 _IFSC
      1 _STCP)21,0)
      1 _SSS2
      1 _SELS
      1 _RTMN
      1 _RTM2
      1 _IFUR
```
