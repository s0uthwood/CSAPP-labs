> Solved with radare2

## Phase 1

```
[0x00400da0]> s sym.phase_1
[0x00400ee0]> pdf
            ; CALL XREF from dbg.main @ 0x400e3a(x)
┌ 28: sym.phase_1 (int64_t arg1);
│           ; arg int64_t arg1 @ rdi
│           0x00400ee0      4883ec08       sub rsp, 8
│           0x00400ee4      be00244000     mov esi, str.Border_relations_with_Canada_have_never_been_better. ; 0x402400 ; "Border relations with Canada have never been better." ; int64_t arg2
│           0x00400ee9      e84a040000     call sym.strings_not_equal
│           0x00400eee      85c0           test eax, eax
│       ┌─< 0x00400ef0      7405           je 0x400ef7
│       │   0x00400ef2      e843050000     call sym.explode_bomb
│       │   ; CODE XREF from sym.phase_1 @ 0x400ef0(x)
│       └─> 0x00400ef7      4883c408       add rsp, 8
└           0x00400efb      c3             ret
```

A simple strcmp

```
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
```

## Phase 2

```
[0x00400da0]> s sym.phase_2
[0x00400efc]> pdf
            ; CALL XREF from dbg.main @ 0x400e56(x)
┌ 69: sym.phase_2 (const char *s);
│           ; arg const char *s @ rdi
│           ; var int64_t var_4h @ rsp+0x4
│           ; var int64_t var_18h @ rsp+0x18
│           0x00400efc      55             push rbp
│           0x00400efd      53             push rbx
│           0x00400efe      4883ec28       sub rsp, 0x28
│           0x00400f02      4889e6         mov rsi, rsp                ; int64_t arg2
│           0x00400f05      e852050000     call sym.read_six_numbers
│           0x00400f0a      833c2401       cmp dword [rsp], 1
│       ┌─< 0x00400f0e      7420           je 0x400f30
│       │   0x00400f10      e825050000     call sym.explode_bomb
..
│      ││   ; CODE XREFS from sym.phase_2 @ 0x400f2c(x), 0x400f3a(x)
│    ┌┌───> 0x00400f17      8b43fc         mov eax, dword [rbx - 4]
│    ╎╎││   0x00400f1a      01c0           add eax, eax
│    ╎╎││   0x00400f1c      3903           cmp dword [rbx], eax
│   ┌─────< 0x00400f1e      7405           je 0x400f25
│   │╎╎││   0x00400f20      e815050000     call sym.explode_bomb
│   │╎╎││   ; CODE XREF from sym.phase_2 @ 0x400f1e(x)
│   └─────> 0x00400f25      4883c304       add rbx, 4
│    ╎╎││   0x00400f29      4839eb         cmp rbx, rbp
│    └────< 0x00400f2c      75e9           jne 0x400f17
│    ┌────< 0x00400f2e      eb0c           jmp 0x400f3c
│    │╎││   ; CODE XREF from sym.phase_2 @ 0x400f0e(x)
│    │╎││   ; CODE XREF from sym.phase_2 @ +0x19(x)
│    │╎└└─> 0x00400f30      488d5c2404     lea rbx, [var_4h]
│    │╎     0x00400f35      488d6c2418     lea rbp, [var_18h]
│    │└───< 0x00400f3a      ebdb           jmp 0x400f17
│    │      ; CODE XREF from sym.phase_2 @ 0x400f2e(x)
│    └────> 0x00400f3c      4883c428       add rsp, 0x28
│           0x00400f40      5b             pop rbx
│           0x00400f41      5d             pop rbp
└           0x00400f42      c3             ret
```

`read_six_numbers()` read from input string `char *s` with format `%d %d %d %d %d %d`, and saved numbers in an array on stack.

The first `cmp` compare the first number with const int `1`. After that, the program fall into a loop, which compare `array[cur]` with `array[cur - 1] * 2`.

Therefore, our input should be:

```
Phase 1 defused. How about the next one?
1 2 4 8 16 32
That's number 2.  Keep going!
```

## Phase 3

```
[0x00400da0]> s sym.phase_3
[0x00400f43]> pdf
            ; CALL XREF from dbg.main @ 0x400e72(x)
┌ 132: sym.phase_3 (const char *s);
│           ; arg const char *s @ rdi
│           ; var uint32_t var_8h @ rsp+0x8
│           ; var uint32_t var_ch @ rsp+0xc
│           0x00400f43      4883ec18       sub rsp, 0x18
│           0x00400f47      488d4c240c     lea rcx, [var_ch]
│           0x00400f4c      488d542408     lea rdx, [var_8h]           ;   ...
│           0x00400f51      becf254000     mov esi, 0x4025cf           ; "%d %d" ; const char *format
│           0x00400f56      b800000000     mov eax, 0
│           0x00400f5b      e890fcffff     call sym.imp.__isoc99_sscanf ; int sscanf(const char *s, const char *format,   ...)
│           0x00400f60      83f801         cmp eax, 1                  ; 1
│       ┌─< 0x00400f63      7f05           jg 0x400f6a
│       │   0x00400f65      e8d0040000     call sym.explode_bomb
│       │   ; CODE XREF from sym.phase_3 @ 0x400f63(x)
│       └─> 0x00400f6a      837c240807     cmp dword [var_8h], 7
│       ┌─< 0x00400f6f      773c           ja case.default.0x400f75
│       │   0x00400f71      8b442408       mov eax, dword [var_8h]
│       │   ;-- switch
│       │   0x00400f75      ff24c5702440.  jmp qword [rax*8 + 0x402470] ; "|\x0f@" ; switch table (8 cases) at 0x402470
│       │   ;-- case 0:                                                ; from 0x00400f75
│       │   ; CODE XREF from sym.phase_3 @ 0x400f75(x)
│       │   0x00400f7c      b8cf000000     mov eax, 0xcf               ; 207
│      ┌──< 0x00400f81      eb3b           jmp 0x400fbe
│      ││   ;-- case 2:                                                ; from 0x00400f75
│      ││   ; CODE XREF from sym.phase_3 @ 0x400f75(x)
│      ││   0x00400f83      b8c3020000     mov eax, 0x2c3              ; 707
│     ┌───< 0x00400f88      eb34           jmp 0x400fbe
│     │││   ;-- case 3:                                                ; from 0x00400f75
│     │││   ; CODE XREF from sym.phase_3 @ 0x400f75(x)
│     │││   0x00400f8a      b800010000     mov eax, 0x100              ; 256
│    ┌────< 0x00400f8f      eb2d           jmp 0x400fbe
│    ││││   ;-- case 4:                                                ; from 0x00400f75
│    ││││   ; CODE XREF from sym.phase_3 @ 0x400f75(x)
│    ││││   0x00400f91      b885010000     mov eax, 0x185              ; 389
│   ┌─────< 0x00400f96      eb26           jmp 0x400fbe
│   │││││   ;-- case 5:                                                ; from 0x00400f75
│   │││││   ; CODE XREF from sym.phase_3 @ 0x400f75(x)
│   │││││   0x00400f98      b8ce000000     mov eax, 0xce               ; 206
│  ┌──────< 0x00400f9d      eb1f           jmp 0x400fbe
│  ││││││   ;-- case 6:                                                ; from 0x00400f75
│  ││││││   ; CODE XREF from sym.phase_3 @ 0x400f75(x)
│  ││││││   0x00400f9f      b8aa020000     mov eax, 0x2aa              ; 682
│ ┌───────< 0x00400fa4      eb18           jmp 0x400fbe
│ │││││││   ;-- case 7:                                                ; from 0x00400f75
│ │││││││   ; CODE XREF from sym.phase_3 @ 0x400f75(x)
│ │││││││   0x00400fa6      b847010000     mov eax, 0x147              ; 327
│ ────────< 0x00400fab      eb11           jmp 0x400fbe
│ │││││││   ;-- default:                                               ; from 0x400f75
│ │││││││   ; CODE XREFS from sym.phase_3 @ 0x400f6f(x), 0x400f75(x)
│ ││││││└─> 0x00400fad      e888040000     call sym.explode_bomb
..
│ │││││││   ;-- case 1:                                                ; from 0x00400f75
│ │││││││   ; CODE XREF from sym.phase_3 @ 0x400f75(x)
│ │││││││   0x00400fb9      b837010000     mov eax, 0x137              ; 311
│ │││││││   ; XREFS: CODE 0x00400f81  CODE 0x00400f88  CODE 0x00400f8f  CODE 0x00400f96  CODE 0x00400f9d  CODE 0x00400fa4
│ │││││││   ; XREFS: CODE 0x00400fab  CODE 0x00400fb7
│ └└└└└└└─> 0x00400fbe      3b44240c       cmp eax, dword [var_ch]
│       ┌─< 0x00400fc2      7405           je 0x400fc9
│       │   0x00400fc4      e871040000     call sym.explode_bomb
│       │   ; CODE XREF from sym.phase_3 @ 0x400fc2(x)
│       └─> 0x00400fc9      4883c418       add rsp, 0x18
└           0x00400fcd      c3             ret
```

First read 2 number from input (format is `%d %d`), and save them in `[var_8h], [var_ch]`.

Number 1 in `[var_8h]` is used for a switch-case, and Number 2 in `[var_ch]` is used to compare with eax which set in case.

Valid input:

```
0 207
1 311
2 707
3 256
4 389
5 206
6 682
7 327
```

For example:

```
That's number 2.  Keep going!
7 327
Halfway there!
```

## Phase 4

```
[0x00400da0]> s sym.phase_4
[0x0040100c]> pdf
            ; CALL XREF from dbg.main @ 0x400e8e(x)
┌ 86: sym.phase_4 (const char *s);
│           ; arg const char *s @ rdi
│           ; var uint32_t var_8h @ rsp+0x8
│           ; var uint32_t var_ch @ rsp+0xc
│           0x0040100c      4883ec18       sub rsp, 0x18
│           0x00401010      488d4c240c     lea rcx, [var_ch]
│           0x00401015      488d542408     lea rdx, [var_8h]           ;   ...
│           0x0040101a      becf254000     mov esi, 0x4025cf           ; "%d %d" ; const char *format
│           0x0040101f      b800000000     mov eax, 0
│           0x00401024      e8c7fbffff     call sym.imp.__isoc99_sscanf ; int sscanf(const char *s, const char *format,   ...)
│           0x00401029      83f802         cmp eax, 2                  ; 2
│       ┌─< 0x0040102c      7507           jne 0x401035
│       │   0x0040102e      837c24080e     cmp dword [var_8h], 0xe
│      ┌──< 0x00401033      7605           jbe 0x40103a
│      ││   ; CODE XREF from sym.phase_4 @ 0x40102c(x)
│      │└─> 0x00401035      e800040000     call sym.explode_bomb
│      │    ; CODE XREF from sym.phase_4 @ 0x401033(x)
│      └──> 0x0040103a      ba0e000000     mov edx, 0xe                ; 14 ; int64_t arg3
│           0x0040103f      be00000000     mov esi, 0                  ; int64_t arg2
│           0x00401044      8b7c2408       mov edi, dword [var_8h]     ; int64_t arg1
│           0x00401048      e881ffffff     call sym.func4
│           0x0040104d      85c0           test eax, eax
│       ┌─< 0x0040104f      7507           jne 0x401058
│       │   0x00401051      837c240c00     cmp dword [var_ch], 0
│      ┌──< 0x00401056      7405           je 0x40105d
│      ││   ; CODE XREF from sym.phase_4 @ 0x40104f(x)
│      │└─> 0x00401058      e8dd030000     call sym.explode_bomb
│      │    ; CODE XREF from sym.phase_4 @ 0x401056(x)
│      └──> 0x0040105d      4883c418       add rsp, 0x18
└           0x00401061      c3             ret


[0x0040100c]> s sym.func4
[0x00400fce]> pdf
            ; CALL XREFS from sym.func4 @ 0x400fe9(x), 0x400ffe(x)
            ; CALL XREF from sym.phase_4 @ 0x401048(x)
┌ 62: sym.func4 (signed int arg1, int64_t arg2, int64_t arg3);
│           ; arg signed int arg1 @ rdi
│           ; arg int64_t arg2 @ rsi
│           ; arg int64_t arg3 @ rdx
│           0x00400fce      4883ec08       sub rsp, 8
│           0x00400fd2      89d0           mov eax, edx                ; arg3
│           0x00400fd4      29f0           sub eax, esi                ; arg2
│           0x00400fd6      89c1           mov ecx, eax
│           0x00400fd8      c1e91f         shr ecx, 0x1f
│           0x00400fdb      01c8           add eax, ecx
│           0x00400fdd      d1f8           sar eax, 1
│           0x00400fdf      8d0c30         lea ecx, [rax + rsi]
│           0x00400fe2      39f9           cmp ecx, edi                ; arg1
│       ┌─< 0x00400fe4      7e0c           jle 0x400ff2
│       │   0x00400fe6      8d51ff         lea edx, [rcx - 1]
│       │   0x00400fe9      e8e0ffffff     call sym.func4
│       │   0x00400fee      01c0           add eax, eax
│      ┌──< 0x00400ff0      eb15           jmp 0x401007
│      ││   ; CODE XREF from sym.func4 @ 0x400fe4(x)
│      │└─> 0x00400ff2      b800000000     mov eax, 0
│      │    0x00400ff7      39f9           cmp ecx, edi                ; arg1
│      │┌─< 0x00400ff9      7d0c           jge 0x401007
│      ││   0x00400ffb      8d7101         lea esi, [rcx + 1]
│      ││   0x00400ffe      e8cbffffff     call sym.func4
│      ││   0x00401003      8d440001       lea eax, [rax + rax + 1]
│      ││   ; CODE XREFS from sym.func4 @ 0x400ff0(x), 0x400ff9(x)
│      └└─> 0x00401007      4883c408       add rsp, 8
└           0x0040100b      c3             ret
```

Still, read 2 numbers from input by `sscanf()`.

The `func4()` should be:

```c
int func4(int a1, long long a2, long long a3){
    int ecx = a3 - a2;
    ecx += ecx >> 0x1f;
    ecx = (ecx >> 1) + a2;
    if (ecx > a1) return func4(a1, a2, a3 - 1) * 2;
    if (ecx < a1) return func4(a1, a2, a3 + 1) * 2 + 1;
    return 0;
}
```

Our target is `func(input[0], 0, 14) == 0`, therefore `input[0]` should be `0` or `1`.

Since `cmp dword [var_ch], 0`, `input[1]` should be `0`.

```
Halfway there!
1 0
So you got that one.  Try this one.
```

## Phase 5

```
[0x00400da0]> s sym.phase_5
[0x00401062]> pdf
            ; CALL XREF from dbg.main @ 0x400eaa(x)
┌ 137: sym.phase_5 (int64_t arg1);
│           ; arg int64_t arg1 @ rdi
│           ; var int64_t var_10h @ rsp+0x10
│           ; var int64_t var_16h @ rsp+0x16
│           ; var int64_t canary @ rsp+0x18
│           0x00401062      53             push rbx
│           0x00401063      4883ec20       sub rsp, 0x20
│           0x00401067      4889fb         mov rbx, rdi                ; arg1
│           0x0040106a      64488b042528.  mov rax, qword fs:[0x28]
│           0x00401073      4889442418     mov qword [canary], rax
│           0x00401078      31c0           xor eax, eax
│           0x0040107a      e89c020000     call sym.string_length
│           0x0040107f      83f806         cmp eax, 6                  ; 6
│       ┌─< 0x00401082      744e           je 0x4010d2
│       │   0x00401084      e8b1030000     call sym.explode_bomb
..
│      ││   ; CODE XREFS from sym.phase_5 @ 0x4010ac(x), 0x4010d7(x)
│    ┌┌───> 0x0040108b      0fb60c03       movzx ecx, byte [rbx + rax]
│    ╎╎││   0x0040108f      880c24         mov byte [rsp], cl
│    ╎╎││   0x00401092      488b1424       mov rdx, qword [rsp]
│    ╎╎││   0x00401096      83e20f         and edx, 0xf                ; 15
│    ╎╎││   0x00401099      0fb692b02440.  movzx edx, byte [rdx + obj.array.3449] ; [0x4024b0:1]=109 ; "maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"
│    ╎╎││   0x004010a0      88540410       mov byte [rsp + rax + 0x10], dl
│    ╎╎││   0x004010a4      4883c001       add rax, 1
│    ╎╎││   0x004010a8      4883f806       cmp rax, 6                  ; 6
│    └────< 0x004010ac      75dd           jne 0x40108b
│     ╎││   0x004010ae      c644241600     mov byte [var_16h], 0
│     ╎││   0x004010b3      be5e244000     mov esi, str.flyers         ; 0x40245e ; "flyers" ; int64_t arg2
│     ╎││   0x004010b8      488d7c2410     lea rdi, [var_10h]          ; int64_t arg1
│     ╎││   0x004010bd      e876020000     call sym.strings_not_equal
│     ╎││   0x004010c2      85c0           test eax, eax
│    ┌────< 0x004010c4      7413           je 0x4010d9
│    │╎││   0x004010c6      e86f030000     call sym.explode_bomb
..
│   ││╎││   ; CODE XREF from sym.phase_5 @ 0x401082(x)
│   ││╎││   ; CODE XREF from sym.phase_5 @ +0x27(x)
│   ││╎└└─> 0x004010d2      b800000000     mov eax, 0
│   ││└───< 0x004010d7      ebb2           jmp 0x40108b
│   ││      ; CODE XREF from sym.phase_5 @ 0x4010c4(x)
│   ││      ; CODE XREF from sym.phase_5 @ +0x6e(x)
│   └└────> 0x004010d9      488b442418     mov rax, qword [canary]
│           0x004010de      644833042528.  xor rax, qword fs:[0x28]
│       ┌─< 0x004010e7      7405           je 0x4010ee
│       │   0x004010e9      e842faffff     call sym.imp.__stack_chk_fail ; void __stack_chk_fail(void)
│       │   ; CODE XREF from sym.phase_5 @ 0x4010e7(x)
│       └─> 0x004010ee      4883c420       add rsp, 0x20
│           0x004010f2      5b             pop rbx
└           0x004010f3      c3             ret
```

First, `string_length(input) == 6`.

Fall into a loop:

```c
for (int i = 0; i != 6; i++){
    ecx = input[i];
    edx = ecx & 0xf;
    edx = maduiersnfotvbyl[edx];
    var_10h[i] = edx;
}
strings_not_equal(var_10h[i], "flyers");
```

So our input should be:

```
0x?9 0x?f 0x?e 0x?5 0x?6 0x?7
```

For easy to input, we can simply set `?` as `6`.

```
So you got that one.  Try this one.
ionefg
Good work!  On to the next...
```

## Phase 6

```
[0x00400da0]> s sym.phase_6
[0x004010f4]> pdf
            ; CALL XREF from dbg.main @ 0x400ec6(x)
┌ 272: sym.phase_6 (const char *s);
│           ; arg const char *s @ rdi
│           ; var int64_t var_18h @ rsp+0x18
│           ; var int64_t var_20h @ rsp+0x20
│           ; var int64_t var_28h @ rsp+0x28
│           ; var int64_t var_50h @ rsp+0x50
│           0x004010f4      4156           push r14
│           0x004010f6      4155           push r13
│           0x004010f8      4154           push r12
│           0x004010fa      55             push rbp
│           0x004010fb      53             push rbx
│           0x004010fc      4883ec50       sub rsp, 0x50
│           0x00401100      4989e5         mov r13, rsp
│           0x00401103      4889e6         mov rsi, rsp                ; int64_t arg2
│           0x00401106      e851030000     call sym.read_six_numbers
│           0x0040110b      4989e6         mov r14, rsp
│           0x0040110e      41bc00000000   mov r12d, 0
│           ; CODE XREF from sym.phase_6 @ 0x401151(x)
│       ┌─> 0x00401114      4c89ed         mov rbp, r13
│       ╎   0x00401117      418b4500       mov eax, dword [r13]
│       ╎   0x0040111b      83e801         sub eax, 1
│       ╎   0x0040111e      83f805         cmp eax, 5                  ; 5
│      ┌──< 0x00401121      7605           jbe 0x401128
│      │╎   0x00401123      e812030000     call sym.explode_bomb
│      │╎   ; CODE XREF from sym.phase_6 @ 0x401121(x)
│      └──> 0x00401128      4183c401       add r12d, 1
│       ╎   0x0040112c      4183fc06       cmp r12d, 6                 ; 6
│      ┌──< 0x00401130      7421           je 0x401153
│      │╎   0x00401132      4489e3         mov ebx, r12d
│      │╎   ; CODE XREF from sym.phase_6 @ 0x40114b(x)
│     ┌───> 0x00401135      4863c3         movsxd rax, ebx
│     ╎│╎   0x00401138      8b0484         mov eax, dword [rsp + rax*4]
│     ╎│╎   0x0040113b      394500         cmp dword [rbp], eax
│    ┌────< 0x0040113e      7505           jne 0x401145
│    │╎│╎   0x00401140      e8f5020000     call sym.explode_bomb
│    │╎│╎   ; CODE XREF from sym.phase_6 @ 0x40113e(x)
│    └────> 0x00401145      83c301         add ebx, 1
│     ╎│╎   0x00401148      83fb05         cmp ebx, 5                  ; 5
│     └───< 0x0040114b      7ee8           jle 0x401135
│      │╎   0x0040114d      4983c504       add r13, 4
│      │└─< 0x00401151      ebc1           jmp 0x401114
│      │    ; CODE XREF from sym.phase_6 @ 0x401130(x)
│      └──> 0x00401153      488d742418     lea rsi, [var_18h]
│           0x00401158      4c89f0         mov rax, r14
│           0x0040115b      b907000000     mov ecx, 7
│           ; CODE XREF from sym.phase_6 @ 0x40116d(x)
│       ┌─> 0x00401160      89ca           mov edx, ecx
│       ╎   0x00401162      2b10           sub edx, dword [rax]
│       ╎   0x00401164      8910           mov dword [rax], edx
│       ╎   0x00401166      4883c004       add rax, 4
│       ╎   0x0040116a      4839f0         cmp rax, rsi
│       └─< 0x0040116d      75f1           jne 0x401160
│           0x0040116f      be00000000     mov esi, 0
│       ┌─< 0x00401174      eb21           jmp 0x401197
│       │   ; CODE XREFS from sym.phase_6 @ 0x40117f(x), 0x4011a9(x)
│     ┌┌──> 0x00401176      488b5208       mov rdx, qword [rdx + 8]
│     ╎╎│   0x0040117a      83c001         add eax, 1
│     ╎╎│   0x0040117d      39c8           cmp eax, ecx
│     └───< 0x0040117f      75f5           jne 0x401176
│     ┌───< 0x00401181      eb05           jmp 0x401188
│     │╎│   ; CODE XREF from sym.phase_6 @ 0x40119d(x)
│    ┌────> 0x00401183      bad0326000     mov edx, obj.node1          ; 0x6032d0 ; "L\x01"
│    ╎│╎│   ; CODE XREF from sym.phase_6 @ 0x401181(x)
│    ╎└───> 0x00401188      4889547420     mov qword [rsp + rsi*2 + 0x20], rdx
│    ╎ ╎│   0x0040118d      4883c604       add rsi, 4
│    ╎ ╎│   0x00401191      4883fe18       cmp rsi, 0x18               ; 24
│    ╎┌───< 0x00401195      7414           je 0x4011ab
│    ╎│╎│   ; CODE XREF from sym.phase_6 @ 0x401174(x)
│    ╎│╎└─> 0x00401197      8b0c34         mov ecx, dword [rsp + rsi]
│    ╎│╎    0x0040119a      83f901         cmp ecx, 1                  ; 1
│    └────< 0x0040119d      7ee4           jle 0x401183
│     │╎    0x0040119f      b801000000     mov eax, 1
│     │╎    0x004011a4      bad0326000     mov edx, obj.node1          ; 0x6032d0 ; "L\x01"
│     │└──< 0x004011a9      ebcb           jmp 0x401176
│     │     ; CODE XREF from sym.phase_6 @ 0x401195(x)
│     └───> 0x004011ab      488b5c2420     mov rbx, qword [var_20h]
│           0x004011b0      488d442428     lea rax, [var_28h]
│           0x004011b5      488d742450     lea rsi, [var_50h]
│           0x004011ba      4889d9         mov rcx, rbx
│           ; CODE XREF from sym.phase_6 @ 0x4011d0(x)
│       ┌─> 0x004011bd      488b10         mov rdx, qword [rax]
│       ╎   0x004011c0      48895108       mov qword [rcx + 8], rdx
│       ╎   0x004011c4      4883c008       add rax, 8
│       ╎   0x004011c8      4839f0         cmp rax, rsi
│      ┌──< 0x004011cb      7405           je 0x4011d2
│      │╎   0x004011cd      4889d1         mov rcx, rdx
│      │└─< 0x004011d0      ebeb           jmp 0x4011bd
│      │    ; CODE XREF from sym.phase_6 @ 0x4011cb(x)
│      └──> 0x004011d2      48c742080000.  mov qword [rdx + 8], 0
│           0x004011da      bd05000000     mov ebp, 5
│           ; CODE XREF from sym.phase_6 @ 0x4011f5(x)
│       ┌─> 0x004011df      488b4308       mov rax, qword [rbx + 8]
│       ╎   0x004011e3      8b00           mov eax, dword [rax]
│       ╎   0x004011e5      3903           cmp dword [rbx], eax
│      ┌──< 0x004011e7      7d05           jge 0x4011ee
│      │╎   0x004011e9      e84c020000     call sym.explode_bomb
│      │╎   ; CODE XREF from sym.phase_6 @ 0x4011e7(x)
│      └──> 0x004011ee      488b5b08       mov rbx, qword [rbx + 8]
│       ╎   0x004011f2      83ed01         sub ebp, 1
│       └─< 0x004011f5      75e8           jne 0x4011df
│           0x004011f7      4883c450       add rsp, 0x50
│           0x004011fb      5b             pop rbx
│           0x004011fc      5d             pop rbp
│           0x004011fd      415c           pop r12
│           0x004011ff      415d           pop r13
│           0x00401201      415e           pop r14
└           0x00401203      c3             ret
```

`read_six_numbers()` from input string.

A loop:

```c
for (int r12d = 0; r12d != 6; r12d++) {
    if (*r13 - 1 > 5) explode_bomb();
    for (int ebx = r12d; ebx <= 5; ebx++) {
        if (*r13 == rsp[ebx]) explode_bomb();
    }
    r13 += 1;
}
```

Therefore, 6 numbers must be a permutation of [0, 1, 2, 3, 4, 5].

Calculations before check.

`num[i] = 7 - num[i]`:

```c
rax = rsp;
do {
    *rax = 7 - *rax;
    rax += 4;
} while(rax != rsp + 0x18);
```

Re-sort the Nodes.

```c
for (int esi = 0; esi < 0x18; esi += 4) {
    ecx = rsp[esi];
    if (ecx <= 1) {
        edx = node1;
    }
    else {
        eax = 1;
        edx = node1;
        do {
            rdx = rdx.next;
            eax += 1;
        } while (eax != ecx);
    }
    var_20h[rsi * 2] = rdx;
}
```

Node is a One-way LinkedList

```
node1
    dd 14Ch 
    dd 1
    dq offset node2

node2
    dd 0A8h
    dd 2
    dq offset node3

node3
    dd 39Ch
    dd 3
    dq offset node4

node4
    dd 2B3h
    dd 4
    dq offset node5

node5
    dd 1DDh
    dd 5
    dq offset node6

node6
    dd 1BBh
    dd 6
    dq offset 0
```

Re-linked.

```c
rcx = *var_20h;
for (i = 1; i < 6; i++) {
    rdx = var_20h[i];
    rcx.next = rdx;
    rcx = rdx;
}
```

Final check: nodes should be sorted by value.

```c
rbx = *var_20h;
for (ebp = 5; ebp > 0; ebp--) {
    rax = rbx.next;
    eax = rax.value;
    if (rbx.value < eax) explode_bomb();
    rbx = rbx.next;
}
```

node3 > node4 > node5 > node6 > node1 > node2

```
Good work!  On to the next...
4 3 2 1 6 5
Congratulations! You've defused the bomb!
```

## Secret Phase

```
[0x004010f4]> s sym.secret_phase
[0x00401242]> pdf
            ; CALL XREF from sym.phase_defused @ 0x401630(x)
┌ 81: sym.secret_phase ();
│           0x00401242      53             push rbx
│           0x00401243      e856020000     call sym.read_line
│           0x00401248      ba0a000000     mov edx, 0xa                ; int base
│           0x0040124d      be00000000     mov esi, 0                  ; char * *endptr
│           0x00401252      4889c7         mov rdi, rax                ; const char *str
│           0x00401255      e876f9ffff     call sym.imp.strtol         ; long strtol(const char *str, char * *endptr, int base)
│           0x0040125a      4889c3         mov rbx, rax
│           0x0040125d      8d40ff         lea eax, [rax - 1]
│           0x00401260      3de8030000     cmp eax, 0x3e8              ; 1000
│       ┌─< 0x00401265      7605           jbe 0x40126c
│       │   0x00401267      e8ce010000     call sym.explode_bomb
│       │   ; CODE XREF from sym.secret_phase @ 0x401265(x)
│       └─> 0x0040126c      89de           mov esi, ebx                ; int64_t arg2
│           0x0040126e      bff0306000     mov edi, obj.n1             ; 0x6030f0 ; "$" ; int64_t arg1
│           0x00401273      e88cffffff     call sym.fun7
│           0x00401278      83f802         cmp eax, 2                  ; 2
│       ┌─< 0x0040127b      7405           je 0x401282
│       │   0x0040127d      e8b8010000     call sym.explode_bomb
│       │   ; CODE XREF from sym.secret_phase @ 0x40127b(x)
│       └─> 0x00401282      bf38244000     mov edi, str.Wow__Youve_defused_the_secret_stage_ ; 0x402438 ; "Wow! You've defused the secret stage!" ; const char *s
│           0x00401287      e884f8ffff     call sym.imp.puts           ; int puts(const char *s)
│           0x0040128c      e833030000     call sym.phase_defused
│           0x00401291      5b             pop rbx
└           0x00401292      c3             ret

[0x00401242]> s sym.fun7
[0x00401204]> pdf
            ; CALL XREFS from sym.fun7 @ 0x401217(x), 0x40122d(x)
            ; CALL XREF from sym.secret_phase @ 0x401273(x)
┌ 62: sym.fun7 (int64_t arg1, signed int64_t arg2);
│           ; arg int64_t arg1 @ rdi
│           ; arg signed int64_t arg2 @ rsi
│           0x00401204      4883ec08       sub rsp, 8
│           0x00401208      4885ff         test rdi, rdi               ; arg1
│       ┌─< 0x0040120b      742b           je 0x401238
│       │   0x0040120d      8b17           mov edx, dword [rdi]        ; arg1
│       │   0x0040120f      39f2           cmp edx, esi                ; arg2
│      ┌──< 0x00401211      7e0d           jle 0x401220
│      ││   0x00401213      488b7f08       mov rdi, qword [rdi + 8]    ; int64_t arg1
│      ││   0x00401217      e8e8ffffff     call sym.fun7
│      ││   0x0040121c      01c0           add eax, eax
│     ┌───< 0x0040121e      eb1d           jmp 0x40123d
│     │││   ; CODE XREF from sym.fun7 @ 0x401211(x)
│     │└──> 0x00401220      b800000000     mov eax, 0
│     │ │   0x00401225      39f2           cmp edx, esi                ; arg2
│     │┌──< 0x00401227      7414           je 0x40123d
│     │││   0x00401229      488b7f10       mov rdi, qword [rdi + 0x10] ; int64_t arg1
│     │││   0x0040122d      e8d2ffffff     call sym.fun7
│     │││   0x00401232      8d440001       lea eax, [rax + rax + 1]
│    ┌────< 0x00401236      eb05           jmp 0x40123d
│    ││││   ; CODE XREF from sym.fun7 @ 0x40120b(x)
│    │││└─> 0x00401238      b8ffffffff     mov eax, 0xffffffff         ; -1
│    │││    ; CODE XREFS from sym.fun7 @ 0x40121e(x), 0x401227(x), 0x401236(x)
│    └└└──> 0x0040123d      4883c408       add rsp, 8
└           0x00401241      c3             ret
```

Readline and convert to number.

The number should smaller than 1001, and the result of `fun7()` should be 2.

Arg1 is a root of a tree.

```
n1
    dq 24h
    dq offset n21
    dq offset n22
    dq 0

n21
    dq 8
    dq offset n31
    dq offset n32
    dq 0

n22
    dq 32h
    dq offset n33
    dq offset n34
    dq 0

n32
    dq 16h
    dq offset n43
    dq offset n44
    dq 0

n33
    dq 2Dh
    dq offset n45
    dq offset n46
    dq 0

n31
    dq 6
    dq offset n41
    dq offset n42
    dq 0

n34
    dq 6Bh
    dq offset n47
    dq offset n48
    dq 0

n45
    dq 28h
    dq 0
    dq 0
    dq 0

n41
    dq 1
    dq 0
    dq 0
    dq 0

n47
    dq 63h
    dq 0
    dq 0
    dq 0

n44
    dq 23h
    dq 0
    dq 0
    dq 0

n42
    dq 7
    dq 0
    dq 0
    dq 0

n43
    dq 14h
    dq 0
    dq 0
    dq 0

n46
    dq 2Fh
    dq 0
    dq 0
    dq 0

n48
    dq 3E9h
    dq 0
    dq 0
    dq 0
```

The tree:

```

                      0x24
          v------------|------------v
          8                        0x32
    v-----|------v            v-----|------v
    6           0x16         0x2d         0x6b
v---|---v    v---|---v    v---|---v    v---|---v
1       7   0x14    0x23 0x28    0x2f 0x63   0x3e9
```

In fun7:

```c
rdi = root;
rsi = input_num;
edx = root.value;
if (edx > rsi) {
    rdi = rdi.lchild;
    return 2 * fun7(rdi, rsi);
}
eax = 0
if (edx == esi) {
    return eax;
}
else {
    rdi = rdi.rchild;
    return 1 + 2 * fun7(rdi, rsi);
}
```

The target is 2, so we need:

```
ret 0
ret 1 + 2 * 0
ret 2 * (1 + 2 * 0)
```

Therefore, the input should be 22.

```
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
1 2 4 8 16 32
That's number 2.  Keep going!
7 327
Halfway there!
0 0 DrEvil
So you got that one.  Try this one.
ionefg
Good work!  On to the next...
4 3 2 1 6 5
Curses, you've found the secret phase!
But finding it and solving it are quite different...
22
Wow! You've defused the secret stage!
Congratulations! You've defused the bomb!
```

