
.include "sys/sms_arch.s"
  ;.include "base/ram.s"
;.include "base/macros.s"
  ;.include "res/defines.s"

.rombankmap
  bankstotal 32
  banksize $4000
  banks 32
.endro

.emptyfill $FF

.background "mkr.gg"

; unbackground some space we need

.unbackground $2FA0 $2FFF

;.unbackground $7BF0 $7C6F

.unbackground $BA48 $BC4F

; free space for extensions in main menu code
.unbackground $19000 $1BFFF

; free space for new messages (e.g. long item names)
;.unbackground $66B00 $66FFF

; free space for long item names
.unbackground $7FB64 $7FD73

;===============================================
; Update header after building
;===============================================
.smstag

;===============================================
; New startup screen
;===============================================

.define tileFontGraphicPointer $7A40
.define tileFontBank $1F
.define splashScreenTimer $C0EA
.define splashScreenTimerInitValue 600  ; number of frames
.define waitForVblank $78
.define buttonsPressed $C018
.define buttonsTriggered $C019
;.define splashScreenW 20
;.define splashScreenH 18
.define splashScreenDimensions $1214
;.define splashScreenShownFlag $DF00
.define splashScreenShownMemString $DF00     ; bottommost part of the space
                                             ; the game allocates for the
                                             ; stack;
                                             ; it's no big deal if this
                                             ; string gets overwritten anyways
.define splashScreenShownStringVal "RAYE"

.bank 0 slot 0
.section "soft reset check" free

  splashScreenShownString:
    .asc splashScreenShownStringVal
  splashScreenShownStringEnd:
  
  .define splashScreenShownStringLen splashScreenShownStringEnd-splashScreenShownString
  
  softResetCheck:
    ld hl,splashScreenShownMemString
    ld de,splashScreenShownString
    ld b,splashScreenShownStringEnd-splashScreenShownString
    
    @checkLoop:
      ; check if next char in memory matches src string
      ld a,(hl)
      ld c,a
      ld a,(de)
      cp c
      jr z,@loopEnd
      
      ; mismatch: return 0
      xor a
      ret
      
      ; match: move to next character
      @loopEnd:
      inc hl
      inc de
      djnz @checkLoop
    
    ; match: return nonzero
    ld a,$FF
    ret

.ends

.bank 2 slot 1
.section "startup splash screen" free
    
  copySoftResetString:
    ; copy soft reset check string to memory
    ld hl,splashScreenShownMemString
    ld de,splashScreenShownString
    ld b,splashScreenShownStringEnd-splashScreenShownString
    -:
      ld a,(de)
      ld (hl),a
      inc hl
      inc de
      djnz -
    ret
  
  doStartupSplashScreen:
  
    ; skip splash screen if soft reset
    
    ; if soft reset check returns nonzero, skip splash screen
    call softResetCheck
    or a
    jp nz,@done
    
    call copySoftResetString
    
;    ld a,(splashScreenShownFlag)
;    or a
;    jp nz,@done
;    dec a
;    ld (splashScreenShownFlag),a
    
  
    ; clear screen?
    call $0745
    
    ; vram fill?
    ld hl,$0000
    ld de,$7800
    ld bc,$0380
    call $0771
    
    ; load tiletext font
    ld a,tileFontBank
    ld ($FFFF),a
    ld hl,(tileFontGraphicPointer)
    ld de,$0E00         ; dstaddr
    call $091D
    
    ; load ... whatever palette is at 7C000, it works fine
    ; for our purposes
    ld hl,$8000
    ld de,$D440
    ld bc,$0022
    ldir
    
    ; write tilemap
    ld a,:splashScreenTilemap
    ld ($FFFF),a
    ld hl,splashScreenTilemap
    ld de,$78CC
    ld bc,splashScreenDimensions
    call $07C3
    
    ; init countdown timer
    ld hl,splashScreenTimerInitValue
    ld (splashScreenTimer),hl
    
    ; enable display?
    call $004F
    
    ; fade in
    call $0A69
;    call $0AA4
    
    ; ?
;    ld a,$01
;    ld ($C01B),a
    
    @waitLoop:
      
      ; wait a frame
      call waitForVblank
      
      ; end screen if start triggered
      ld a,(buttonsTriggered)
      bit 7,a
      jr nz,@finishScreen
    
      ; tick countdown timer
      ld hl,(splashScreenTimer)
      ld a,h
      or l
      ; break loop if done
      jr z,@finishScreen
      dec hl
      ld (splashScreenTimer),hl
      
      jr @waitLoop
    
    @finishScreen:
    
    ; fade out
    call $0A81
;    call $0AA8
    
    @done:
    ; make up work
    jp $0745
  
.ends

.bank 2 slot 1
.org $0003
.section "trigger startup splash screen" overwrite
  call doStartupSplashScreen
.ends

.slot 2
.section "splash screen tilemap" superfree
  splashScreenTilemap:
    .incbin "../out/splash_screen/msg_0x0.bin"
.ends

.bank 2 slot 1
.section "startup init soft reset string check" free
  softResetStartupCheck:
    ; make up work
    ld sp,$DFF0
    
    call softResetCheck
    or a
    jr nz,@preserveString
    
    ; make up work
    ld hl,$C018
    ld de,$C019
    jp $022A
    
    @preserveString:
    ld hl,$C018
    ld de,$C019
    ld bc,$1FD7
    ld (hl),$00
    ldir
    call copySoftResetString
    jp $0231
    
.ends

.bank 0 slot 0
.org $0221
.section "startup init soft reset string check trigger" overwrite
  ld a,:softResetStartupCheck
  ld ($FFFE),a
  di    ; have to do this; vblank will swap the slot 1 page
  jp softResetStartupCheck
.ends

;===============================================
; Adjust title screen cursor position 8 pixels
; left so longer English strings are centered
;===============================================
.bank 3 slot 1
.org $01DF
.section "titlecursor" overwrite
  ld d,$58  ; x-position (orig: 60)
.ends

;===============================================
; Adjust title screen "flashing" tilemaps to
; cover the longer English strings
;===============================================
.bank 3 slot 1
.org $0216
.section "titleflash" overwrite
  ; "new game" state
;  ld hl,$4526   ; srcaddr
;  ld a,($CF5A)  ; ?
  ld de,$7BD6   ; dstaddr (orig: 7B9A)
  dec a
  jr z,+
    ; "continue" state
    ld de,$7C56   ; dstaddr (orig: 7C1A)
    +:
  ld bc,$010A   ; dimensions: 10x1 (orig: 5x2)
.ends

;===============================================
; Adjust base text speed (English text is
; generally 2-3 times longer, so speedup
; is desirable)
;===============================================
.bank 0 slot 0
.org $2442
.section "textspeed" overwrite
  ld a,$02  ; frame delay between characters (orig: 4)
.ends

;===============================================
; Extend length of character names from 3 to 6
;===============================================

; redirect character name pointers to expanded tilemaps
; we've placed at hardcoded locations

.bank $1F slot 2
.org $2F9D
.section "charname_ptrtable" overwrite
  .dw $BAE4   ; hikaru name pointer
  .dw $BAEA   ; fuu name pointer
  .dw $BAF0   ; umi name pointer
.ends

; alter size of uploaded tilemap in menus

.bank 0 slot 0
.org $2F1E
.section "charname_size" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

; alter size of uploaded tilemap in battles
; (same strings are used)

.bank 7 slot 1

.org $0E84
.section "charname_size_battle1" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $0F9D
.section "charname_size_battle2" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $111A
.section "charname_size_battl3" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $1164
.section "charname_size_battle4" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $14A0
.section "charname_size_battle5" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $1584
.section "charname_size_battle6" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $178E
.section "charname_size_battle7" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $17C7
.section "charname_size_battle8" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $17EA
.section "charname_size_battle9" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $27D9
.section "charname_size_battle10" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $29AE
.section "charname_size_battle11" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

.org $2AA6
.section "charname_size_battle12" overwrite
  ld bc,$0106   ; tilemap size = 6x1 (orig: 3x1)
.ends

;===============================================
; Expand main menu tilemap with character names
;===============================================

; redirect character name pointers to expanded tilemaps
; we've placed at hardcoded locations

.bank $6 slot 1
.org $0132
.section "menu_charnames_ptr" overwrite
  ld hl,$AAA0   ; srcaddr (orig: A7A2)
  ld de,$7B0C   ; dstaddr (orig: 7B10)
  ld bc,$0113   ; dimensions (orig: 17x1, new: 20x1)
.ends

; move menu cursor x-position to left to avoid overlapping
; new name positions

.bank $6 slot 1
.org $01FB
.section "menu_charnames_cursorpos" overwrite
  .db $30,$61   ; x, y (orig: 38)
  .db $68,$61   ; x, y (orig: 60)
  .db $A0,$61   ; x, y (orig: A8)
.ends

;===============================================
; Allow more tiles for world map labels
;===============================================

.bank $2 slot 1
.org $058B
.section "map_label_pos" overwrite
;  ld hl,$90AD   ; srcaddr (orig: A7A2)
  ld de,$5400   ; dstaddr (orig: 5800)
.ends

;===============================================
; Adjust item menu cursor positions
;===============================================

.bank $6 slot 1
.org $0209
.section "tt_item_menu_options_cursor" overwrite
  .db $38,$91   ; x, y (orig: 38)
  .db $68,$91   ; x, y (orig: 70)
  .db $98,$91   ; x, y (orig: A0)
.ends

;===============================================
; Change field magic menu to vertical layout
; so magic labels can be longer
;===============================================

; Label dimensions
.bank $6 slot 1
.org $04D0
.section "field_magic_label_dimensions 1" overwrite
  ld bc,$0112   ; h/w (orig: 0209)
.ends

; Label dimensions/position for "use on whom" prompt
.bank $6 slot 1
.org $058E
.section "field_magic_label_dimensions 2" overwrite
  ld de,$7C14   ; dstaddr (orig: 7BD8)
  ld bc,$010F   ; h/w (orig: 0209)
.ends

; Label positions
.bank $6 slot 1
.org $0452
.section "field_magic_label_positions" overwrite
  .dw $7C0E     ; dstaddr (orig: 7C0E)
  .dw $7C8E     ; dstaddr (orig: 7C20)
.ends

; Left -> Up
.bank $6 slot 1
.org $05B4
.section "field_magic_buttons 1" overwrite
  bit 0,a
.ends

; Right -> Down
.bank $6 slot 1
.org $05BE
.section "field_magic_buttons 2" overwrite
  bit 1,a
.ends

; Redirect cursor positions to new entries,
; separate from original (which were shared with
; item menu)

.bank $6 slot 1
.org $049B
.section "field_magic_cursor_pos table_pointer 1" overwrite
  ld hl,fieldMagicCursorTable
.ends

.bank $6 slot 1
.org $05CA
.section "field_magic_cursor_pos table_pointer 2" overwrite
  ld hl,fieldMagicCursorTable
.ends

.bank $6 slot 1
.section "field_magic_cursor_pos_table" free
  fieldMagicCursorTable:
    .db $38,$81   ; x, y (orig: 38, 89)
    .db $38,$91   ; x, y (orig: 80, 89)
.ends

; Cursor positions
; (originally shared with item menu)
;.bank $6 slot 1
;.org $0201
;.section "field_magic_cursor_pos" overwrite
;  .db $38,$81   ; x, y (orig: 38, 89)
;  .db $38,$91   ; x, y (orig: 80, 89)
;.ends

;===============================================
; Switch key item screen to vertical display
;===============================================

; Table of new item dst addresses (originally
; shared with magic use menu at 18452)
.bank $6 slot 1
.section "key_item_pos_table" free
  keyItemPosTable:
    .dw $7C0E   ; dstaddr (orig: 7C0E)
    .dw $7C4E   ; dstaddr (orig: 7C20)
    .dw $7C8E   ; dstaddr (orig: 7C8E)
    .dw $7CCE   ; dstaddr (orig: 7CA0)
.ends

; Use new table when laying out screen
.bank $6 slot 1
.org $079C
.section "key_item_layout" overwrite
  ld hl,keyItemPosTable
.ends

; Switch key item names to 1-line
.bank $6 slot 1
.org $07A8
.section "key_item_layout 1line" overwrite
  ld bc,$010C
.ends

;===============================================
; Switch field item screen to vertical display
;===============================================

; Table of new item dst addresses for each
; subindex per page of item list
.bank $6 slot 1
.org $082B
.section "field_item_pos_table" overwrite
  fieldItemPosTable:
    .dw $7C10   ; dstaddr (orig: 7C0E)
    .dw $7C50   ; dstaddr (orig: 7C20)
    .dw $7C90   ; dstaddr (orig: 7C8E)
    .dw $7CD0   ; dstaddr (orig: 7CA0)
.ends

; Table of new cursor positions for each
; subindex per page of item list
.bank $6 slot 1
.org $0201
.section "field_item_cursor_table" overwrite
    .db $38,$81   ; x, y (orig: 38, 89)
    .db $38,$89   ; x, y (orig: 80, 89)
    .db $38,$91   ; x, y (orig: 38, 99)
    .db $38,$99   ; x, y (orig: 80, 99)
.ends

; use long item names
.bank $6 slot 1
.org $0923
.section "field_item_dimensions" overwrite
  ld hl,longItemNamePtrTable ; src pointer table (orig: B240)
  rst $18
  ld bc,longNameDimensions         ; h/w (orig: 0208)
.ends

; "up" reduces item subindex by 1
;.bank $6 slot 1
;.org $0A56
;.section "field_item_controls 1" overwrite
;  nop
;.ends

; "down" increases item subindex by 1
;.bank $6 slot 1
;.org $0A6C
;.section "field_item_controls 2" overwrite
;  nop
;.ends

; reprogram menu controls
.bank $6 slot 1
.org $0A47
.section "field_item_controls" overwrite
  
  .define curPageNum $C89C
  .define curItemSubidx $C981
  .define maxPageNum $C89D
  
  ; at entry, B = current item subindex
  ; (in original routine: 0 = UL, 1 = UR, 2 = LL, 3 = LR)
  
  ;======================
  ; up pressed?
  ;======================
  @checkUp:
  bit 0,a
  jr z,@checkDown
    
  ; ORIGINAL
    ; if current page number is zero, and item subindex
    ; is less than 2, don't allow UP to move to the
    ; previous page
;    ld a,(curPageNum)
;    and a
;    jr nz,+
;      ld a,b
;      cp $02
;      ret c
;    +:
;    ; subindex -= 2
;    dec b
;    dec b
;    jr ctrlCheckDone
  
    ; don't decrement subindex if on first item of first page
    ld a,(curPageNum)
    and a
    ; branch if current page number nonzero
    jr nz,+
      ; get curidx
      ld a,b
      ; ret if zero
      and a
      ret z
    
    +:
    dec b
    
    jr ctrlCheckDone
  
  ;======================
  ; down pressed?
  ;======================
  @checkDown:
  bit 1,a
  jr z,@checkLeft
  
  ; ORIGINAL
;    ld a,(curPageNum)
;    ld c,a
;    ; get max page num?
;    ld a,(maxPageNum)
;    sub c
;    jr nz,+
;      ld a,b
;      cp $02
;      ret nc
;    +:
;    inc b
;    inc b
;    jr ctrlCheckDone
  
;    ld a,(curPageNum)
;    ld c,a
;    ld a,(maxPageNum)
;    sub c
;    ; branch if not on last page
;    jr nz,+
;      ; get curidx
;      ld a,b
;      ; ret if 03
;      cp $03
;      ret z
;      ; ret if 01
;      dec a
;      ret z
    
    +:
    inc b
    
    jr ctrlCheckDone
  
  ;======================
  ; left pressed?
  ;======================
  @checkLeft:
  bit 2,a
  jr z,@checkRight
  
  ; ORIGINAL
;    ld a,b
;    and a
;    ret z
;    cp $02
;    ret z
;    dec b
    
  
    ; don't decrement subindex if on first item of first page
    ld a,(curPageNum)
    and a
    ; ret if current page number zero
    ret z
    
    ; move back a page
    dec b
    dec b
    dec b
    dec b
    
    
    jr ctrlCheckDone
  
  ;======================
  ; right pressed?
  ;======================
  @checkRight:
  bit 3,a
  ret z
  
  ; ORIGINAL
;    ; ret if current index is 03 or 01
;    ld a,b
;    cp $03
;    ret z
;    dec a
;    ret z
;    ; increment subindex
;    inc b
    
    ; ret if on last page
;    ld a,(curPageNum)
;    ld c,a
;    ld a,(maxPageNum)
;    sub c
;    ret z
    
    ; move forward a page
    inc b
    inc b
    inc b
    inc b
    
    jr ctrlCheckDone
  
.ends

; set up relative jump target due to WLA-DX limitations
.bank $6 slot 1
.org $0A86
.section "field_item_controls ctrlcheckdone" overwrite
  ctrlCheckDone:
  bit 7,b
.ends

; trigger range check after paging down
.bank $6 slot 1
.org $0AFA
.section "field_item_controls trigger range check" overwrite
  call fieldItemMenuRangeCheck
.ends

.bank $6 slot 1
.section "field_item_controls range check" free
  fieldItemMenuRangeCheck:
    
    ; check if on last page
    ld a,(maxPageNum)
    ld c,a
    ld a,(curPageNum)
    cp c
    jr nz,@done
    
      ; make sure target index is valid
      
      ; compute absolute item index
      ; get pagenum * 4
      add a,a
      add a,a
      ld c,a
      ; get total number of items
      ld a,(ix+$2B)
      ; subtract pagenum * 4 to get number of items
      ; on last page; store in C
      sub c
      ld c,a
      ld a,(curItemSubidx)
      cp c
      ; branch if in valid range
      jr c,@done
        
        ; force to nearest valid index
        ld a,c
        dec a
        ld (curItemSubidx),a
    
    ; make up work
    @done:
    ld a,(curItemSubidx)
    ld hl,$4201
    ret
    
.ends

;===============================================
; Extended spell names in battle menu
;===============================================

; don't draw window around MP meter
.bank $7 slot 1
.org $1AFF
.section "no spell menu MP meter border" overwrite
  nop
  nop
  nop
.ends

; when opening menu
.bank $7 slot 1
.org $1B18
.section "battle spell menu dst/size 1" overwrite
  ld de,$7B0E   ; dstaddr (orig: 7A8E)
  ld bc,$0212   ; h/w (orig: 0409)
  
  ; get count of spells learned
  ld a,(ix+$2B)
  ; copy only tilemap for first spell if it's the only
  ; one unlocked
  cp $01
  jr nz,+
    ld b,$01
  +:
  call $07C3
  
.ends

; when loading new page
.bank $7 slot 1
.org $1C28
.section "battle spell menu dst/size 2" overwrite
  ld de,$7B0E   ; dstaddr (orig: 7A8E)
  ld bc,$0212   ; h/w (orig: 0409)
.ends

; move MP text
.bank $7 slot 1
.org $1B05
.section "move spell menu MP text" overwrite
  ld de,$7AD8   ; dstaddr (orig: 7AE8)
  ld bc,$0104   ; ? (orig: 0104)
.ends

; move MP meter
.bank $7 slot 1
.org $1B2D
.section "move spell menu MP meter" overwrite
  ld de,$7AE0   ; dstaddr (orig: 7B68)
  ld bc,$0108   ; ? (orig: 0108)
.ends

; move up/down indicators
.bank $7 slot 1
.org $2118
.section "move battle spell menu up/down arrows" overwrite
  ld (ix+35),$C8
.ends

; move cursor at menu start
.bank $7 slot 1
.org $1A83
.section "move battle spell menu cursor 1" overwrite
  ld (ix+32),$61        ; y-pos (orig: 59)
  ld (ix+35),$30        ; x-pos (orig: 38)
.ends

; move cursor regularly
.bank $7 slot 1
.org $1C37
.section "move battle spell menu cursor 2" overwrite
  ld h,$61      ; top row y-pos (orig: 59)
  jr +
    ld h,$69    ; bottom row y-pos (orig: 69)
  +:
  ld (ix+$20),h
  ld (ix+$23),$30       ; x-pos (orig: 38)
  
.ends

; move cursor after "not enough MP" message
.bank $7 slot 1
.org $1C8C
.section "move battle spell menu cursor 3" overwrite
  ld (ix+32),$61        ; y-pos (orig: 59)
  ld (ix+35),$30        ; x-pos (orig: 38)
.ends

;===============================================
; New "long" item name table
;===============================================

.define longItemNameLen 14
.define longItemNameCount 33

; Item names
.bank $1F slot 2
.section "long_item_name_table" free
  longItemNames:
    .incbin "../out/items_long/msg_0x66B00.bin"
.ends

; Pointer table for each item
;.bank $6 slot 1
.bank $1F slot 2
.section "long_item_name_ptr_table" free
  longItemNamePtrTable:
    .rept longItemNameCount index count
      .dw (longItemNames) + (longItemNameLen * count)
    .endr
.ends

;===============================================
; Use long item names where possible
;===============================================
  
.define longNameDimensions $010E
  
.bank $6 slot 1
.org $0C6B
.section "tt_item_on_whom_longname 1" overwrite
  ld hl,longItemNamePtrTable ; src pointer table (orig: B240)
  rst $18
  ld de,$7C16         ; dstaddr (orig: 7BDA)
  ld bc,longNameDimensions         ; h/w (orig: 0208)
.ends

.bank $6 slot 1
.org $0DAF
.section "tt_item_on_whom_longname 2" overwrite
  ld hl,longItemNamePtrTable ; src pointer table (orig: B240)
  rst $18
  ld de,$7C16         ; dstaddr (orig: 7BDA)
  ld bc,longNameDimensions         ; h/w (orig: 0208)
.ends

.bank $6 slot 1
.org $09E7
.section "tt_item_discarded_longname" overwrite
  ld hl,longItemNamePtrTable ; src pointer table (orig: B240)
  rst $18
  ld de,$7C12         ; dstaddr (orig: 7C10)
  ld bc,longNameDimensions         ; h/w (orig: 0208)
.ends
  
.bank $6 slot 1
.org $0D69
.section "tt_item_in_use_longname 1" overwrite
  ld hl,longItemNamePtrTable ; src pointer table (orig: B240)
  rst $18
  ld de,$7C12         ; dstaddr (orig: 7BD0)
  ld bc,longNameDimensions         ; h/w (orig: 0208)
.ends

.bank $6 slot 1
.org $0E17
.section "tt_item_in_use_longname 2" overwrite
  ld hl,longItemNamePtrTable ; src pointer table (orig: B240)
  rst $18
  ld de,$7C0E         ; dstaddr (orig: 7BD0)
  ld bc,longNameDimensions         ; h/w (orig: 0208)
.ends

.bank $7 slot 1
.org $29C9
.section "tt_battle_useonwhom_longname" overwrite
  ld hl,longItemNamePtrTable ; src pointer table (orig: B200)
  rst $18
  ld de,$7AD6         ; dstaddr (orig: 7A90)
  ld bc,longNameDimensions         ; h/w (orig: 0208)
.ends

.bank $7 slot 1
.org $2416
.section "tt_battle_itemused_longname" overwrite
  ld hl,longItemNamePtrTable ; src pointer table (orig: B200)
  rst $18
  ld de,$7ACE         ; dstaddr (orig: 7A90)
  ld bc,longNameDimensions         ; h/w (orig: 0208)
.ends

;===============================================
; Switch spell cast text to full-box
;===============================================

.bank $7 slot 1
.org $1FEF
.section "spellcast fullbox 1" overwrite
  ld de,$7A8E         ; dstaddr (orig: 7A8E)
  ld bc,$0412         ; h/w (orig: 0212)
.ends

.bank $7 slot 1
.org $2005
.section "spellcast fullbox 2" overwrite
  ld de,$7A8E         ; dstaddr (orig: 7B0E)
  ld bc,$0412         ; h/w (orig: 0212)
.ends

;===============================================
; Load world map label graphics from bank 4
; instead of bank 0x10 (due to the larger
; translated graphics being unable to fit in
; the original bank)
;===============================================

.bank $2 slot 1
.org $058E
.section "worldmap graphics 1" overwrite
  call loadWorldMapLabelGrp
.ends

.bank $2 slot 1
.section "worldmap graphics 2" free
  loadWorldMapLabelGrp:
    
    ; load bank 4, containing new label graphics
    ld a,$04
    ld ($FFFF),a
    
    ; make up work
    call $091D
    
    ; load original bank ($10)
    ld a,$10
    ld ($FFFF),a
    ret
    
.ends

;===============================================
; Long item name drawing routine
;===============================================

;.bank $2 slot 1
.bank $0 slot 0
.section "draw long item name" free
  ; A = item ID
  ; DE = dst VRAM address
  drawLongItemName:
    ; if ID == FF, target ID $20 instead (blank)
    inc a
    jr nz,+
      ld a,$21
    +:
    dec a
  
    push de
    push bc
      
      ; switch to long item name bank
      ld b,a    ; B = item ID
      ld a,($FFFF)
      ld c,a    ; C = old bank number
      ld a,:longItemNamePtrTable
      ld ($FFFF),a      ; switch bank
      
      push bc
      
        ld a,b  ; get item ID
        ld hl,longItemNamePtrTable ; src pointer table (orig: B240)
        ; do 16-bit table lookup
        rst $18
;        ld de,$7C0E         ; dstaddr (orig: 7C10)
        ld bc,longNameDimensions         ; h/w (orig: 0208)
        ; copy tilemap to VRAM
        call $07C3
        
      pop bc
      
      ; restore old bank
      ld a,c
      ld ($FFFF),a
    
    pop bc
    pop de
    ret
    
.ends

;===============================================
; Use long item names in shop menus
;===============================================

.bank 0 slot 0
.org $2B86
.section "long item names shop menus 1" overwrite
  
  ; DE = dstaddr (left column)
  
  ; A = item ID
  ld a,(hl)
  dec a
  call drawLongItemName
  jp $2B9D
  
.ends

.bank 0 slot 0
.org $2B9F
.section "long item names shop menus 2" overwrite
  
  ; DE = dstaddr (left column); needs to be moved down
  ; a row
  
  ; A = item ID
  ld a,(hl)
  dec a
  
  ; move dstaddr to next row
  push bc
    ex de,hl
    ld bc,$0040
    add hl,bc
    ex de,hl
  pop bc
  
  call drawLongItemName
  jp $2BBC
  
.ends

.bank 0 slot 0
.org $2B76
.section "long item names shop menus 3" overwrite
  
  ; adjust starting position of list down a row
  ; (original: $0004)
  ld bc,$0044
  
.ends

;===============================================
; Fix shop text
;===============================================

; shift tilemaps down a row to cover bottom row of
; new item layout

.bank $2 slot 1
.org $183D
.section "shop text pos 1" overwrite
  ld de,$7ACE   ; dstaddr (orig: 7A8E)
.ends

.bank $2 slot 1
.org $1A47
.section "shop text pos 2" overwrite
  ld de,$7ACE   ; dstaddr (orig: 7A8E)
.ends

.bank $2 slot 1
.org $1C0B
.section "shop text pos 3" overwrite
  ld de,$7ACE   ; dstaddr (orig: 7A8E)
.ends

; shift item description tilemaps

; standard
.bank $2 slot 1
.org $180C
.section "shop text pos 4" overwrite
  ld de,$7ACE   ; dstaddr (orig: 7A8E)
.ends

; fruit shop
.bank $2 slot 1
.org $1BDA
.section "shop text pos 5" overwrite
  ld de,$7ACE   ; dstaddr (orig: 7A8E)
.ends

; adjust other stuff

.bank $2 slot 1
.org $187F
.section "shop gem counts buy 1" overwrite
  ; item cost tilemap dstaddr (orig: 7B4E)
  ld de,$7B1C
  call $27D4
  
  ; player gem count tilemap dstaddr (orig: 7C62)
  ld de,$7C6A
.ends

; update gem count after item bought
.bank $2 slot 1
.org $18C7
.section "shop gem counts buy 2" overwrite
  ; player gem count tilemap dstaddr (orig: 7C62)
  ld de,$7C6A
.ends

.bank $2 slot 1
.org $1A74
.section "shop gem counts sell 1" overwrite
  ; item cost tilemap dstaddr (orig: 7B4E)
  ld de,$7B24
  call $27D4
  
  ; player gem count tilemap dstaddr (orig: 7C62)
  ld de,$7C6A
.ends

; update gem count after item bought
.bank $2 slot 1
.org $1AAD
.section "shop gem counts sell 2" overwrite
  ; player gem count tilemap dstaddr (orig: 7C62)
  ld de,$7C6A
.ends

.bank $2 slot 1
.org $1C4D
.section "fruit shop gem counts buy 1" overwrite
  ; item cost tilemap dstaddr (orig: 7B4E)
  ld de,$7B1E
  call $27D4
  
  ; player gem count tilemap dstaddr (orig: 7C62)
  ld de,$7C6A
.ends

; update gem count after item bought
.bank $2 slot 1
.org $1C95
.section "fruit shop gem counts buy 2" overwrite
  ; player gem count tilemap dstaddr (orig: 7C62)
  ld de,$7C6A
.ends

.bank $2 slot 1
.org $185C
.section "long item names shop buy" overwrite
  ; A is currently item ID
  dec a
  ld de,$7AD2   ; dstaddr
  call drawLongItemName
  jp $586F
  
.ends

.bank $2 slot 1
.org $1A58
.section "long item names shop sell" overwrite
  ; A is currently item ID
  dec a
  ld de,$7AD2   ; dstaddr
  call drawLongItemName
  jp $5A6B
  
.ends

.bank $2 slot 1
.org $1C2A
.section "long item names fruit shop buy" overwrite
  ; A is currently item ID
  dec a
  ld de,$7AD2   ; dstaddr
  call drawLongItemName
  jp $5C3D
  
.ends

; New cursor position table
.bank $2 slot 1
.org $1E9E
.section "shop cursor positions" overwrite
  
  .db $38,$58   ; x/y (orig: 38, 58)
  .db $38,$60   ; x/y (orig: 80, 58)
  .db $38,$68   ; x/y (orig: 38, 68)
  .db $38,$70   ; x/y (orig: 80, 68)
  .db $38,$78   ; x/y (orig: 38, 78)
  .db $38,$80   ; x/y (orig: 80, 78)
  .db $38,$88   ; x/y (orig: 38, 88)
  .db $38,$90   ; x/y (orig: 80, 88)
  
.ends

; Cursor control

;.bank $2 slot 1
;.org $1E24
;.section "shop cursor control 1" overwrite
  
  ; only scroll up one item when up pressed
;  nop
  
;.ends

;.bank $2 slot 1
;.org $1E3A
;.section "shop cursor control 2" overwrite
  
  ; only scroll down one item when down pressed
;  nop
  
;.ends

.bank $2 slot 1
.org $1E1F
.section "shop cursor control 3" overwrite
  
  ; scroll up when subindex < 2
  cp $02
  ; only scroll up one item when up pressed
  jp c,singleScrollPageUp
  jp shopCursorUpCheck
  ; a "dec a" has been overwritten here, so now we only scroll
  ; up one item instead of two
  
.ends

.bank $2 slot 1
.org $1E35
.section "shop cursor control 4" overwrite
  
  ; scroll down when subindex >= 8
  cp $08
  ; only scroll up one item when up pressed
  jp nc,singleScrollPageDown
  jp shopCursorDownCheck
  ; an "inc a" has been overwritten here, so now we only scroll
  ; down one item instead of two
  
.ends

.bank $2 slot 1
.org $1F12
.section "fruit shop cursor control 1" overwrite
  
  cp $04
  jp nc,shopCursorControlDone
  ; get rid of an "inc a" so we scroll up only one item
  ; when up is pressed
  nop
  
.ends

.bank $2 slot 1
.org $1F01
.section "fruit shop cursor control 2" overwrite
  
  cp $02
  jr c,shopCursorControlDone
  ; get rid of a "dec a" so we scroll down only one item
  ; when down is pressed
  nop
  
.ends

.bank $2 slot 1
.org $1EF8
.section "fruit shop cursor control 3" overwrite

  ; no left/right handling
  jp $5F00

.ends

.bank $2 slot 1
.org $19DF
.section "shop cursor control 5" overwrite
  
  ; number of subitems to move back on down-page change
  sub $07
  
.ends

.bank $2 slot 1
.org $19FF
.section "shop cursor control 6" overwrite
  
  ; number of subitems to move forward on up-page change
  add a,$07
  
.ends

.bank $2 slot 1
.section "shop cursor control paging checks" free

  shopCursorUpCheck:
    ; do not allow scrolling up if another command is pending
    push bc
      ld b,a
      and $F0
      ld a,b
    pop bc
    
    jr nz,+
      dec a
      ld (ix+$2B),a
    +:
    
    jp $5E28
    
  shopCursorDownCheck:
    ; do not allow scrolling down if another command is pending
    push bc
      ld b,a
      and $F0
      ld a,b
    pop bc
    
    jr nz,+
      inc a
      ld (ix+$2B),a
    +:
    
    jp $5E3E
  
  ; if left pressed
  
  shopCursorControlLeftCheck:
    ; disallow if not selling
    ld a,($CFBB)
    or a
    ret z
  
    ; flag page for scrolling up
;    ld (ix+$2C),$01     ; direction = 01 (up)
;    ld (ix+$2D),$01     ; flag page change
    
    ; update cursor position
    call shopCursorControlDone
    
    ld a,(ix+$2B)       ; prep subindex for addition by 7
;    sub $09
    set 6,a
    ld (ix+$2B),a
    
    ret
  
  shopCursorControlRightCheck:
    ; disallow if not selling
    ld a,($CFBB)
    or a
    ret z
  
    ; flag page for scrolling down
;    ld (ix+$2C),$02     ; direction = 02 (down)
;    ld (ix+$2D),$01     ; flag page change
    
    ; update cursor position
    call shopCursorControlDone
    
    ld a,(ix+$2B)       ; prep subindex for subtraction by 7
;    add a,$09
    set 7,a
    ld (ix+$2B),a
    
    ret
  
  singleScrollPageUp:
    ; disallow if not selling
    ld a,($CFBB)
    or a
    ret z
  
;    ld (ix+$2D),$01
    
    ; ret if on page 0
    ld a,($CFEA)
    or a
    jp z,$5E90
    
    ; update cursor position
    call shopCursorControlDone
    
    ld a,(ix+$2B)
    set 4,a
    ld (ix+$2B),a
    
    ret
  
  singleScrollPageDown:
    ; disallow if not selling
    ld a,($CFBB)
    or a
    ret z
  
;    ld (ix+$2D),$01
    
    ; ret if on page 3
    ld a,($CFEA)
    cp $03
    jp z,$5E90
    
    ; update cursor position
    call shopCursorControlDone
    
    ld a,(ix+$2B)
    set 5,a
    ld (ix+$2B),a
;    jp $5E90
    
    ret
  
  checkForShopLrPaging:
;    ld a,($C018)        ; get buttons triggered
;    bit 1,a             ; down
;    jp nz,$59CA
;    bit 0,a             ; up
;    jp nz,$59EB
;    bit 3,a             ; right
;    jp nz,$59CA
;    bit 2,a             ; left
;    jp nz,$59EB
;    ret

;    ld a,($CFBC)        ; get scroll direction
;    bit 1,a             ; down
;    jp nz,$59CA
;    bit 0,a             ; up
;    jp nz,$59EB
;    ret

    ; very very annoyingly, there seems to be a race condition
    ; somewhere in the original code, probably based on the
    ; input polling, that causes the paging flag
    ; to get reset before being handled.
    ; this manifests in the original game as inputs occasionally
    ; being "eaten", but causes more serious problems with
    ; our additional logic.
    ; thus I've used the item subindex as a more stable flag
    
    ; first do our extra check
    ld hl,$CFBD         ; ready paging flag for access
    ld a,($CFBB)        ; get subindex
    
    ; bit 4 = single scroll up
    ; bit 5 = single scroll down
    ; bit 6 = page scroll up
    ; bit 7 = page scroll down
    
    bit 7,a
    jp z,+
      and $0F
      add a,$07
      ld ($CFBB),a
      jp $59D0         ; if >= 09, page down
    +:
    
    bit 6,a
    jp z,+          ; if negative, page up
      and $0F
      sub $07
      ld ($CFBB),a
      jp $59F1
    +:
    
    bit 5,a
    jp z,+
      and $0F
      ld ($CFBB),a
      jp $59D0
    +:
    
    bit 4,a
    jp z,+
      and $0F
      ld ($CFBB),a
      jp $59F1
    +:

    ; now do the regular check
;    ld a,($CFBC)        ; get scroll direction
;    bit 1,a             ; down
;    jp nz,$59CA
;    bit 0,a             ; up
;    jp nz,$59EB
    
    ret
  
.ends

.bank $2 slot 1
.org $1E4B
.section "shop cursor control 7" overwrite
  
  ; if left pressed
  
  ; do nothing if on page 0
  ld a,($CFEA)
;  ld a,($C11F)
  or a
  jr z,shopCursorControlDone
  
  ; flag page for scrolling up
  jp shopCursorControlLeftCheck
;  call shopCursorControlLeftCheck
;  jr shopCursorControlDone
  
.ends

.bank $2 slot 1
.org $1E5C
.section "shop cursor control 8" overwrite
  
  ; if right pressed
  
  ; do nothing if on page 3
  ld a,($CFEA)
;  ld a,($C11F)
  cp $03
  jr nc,shopCursorControlDone
  
  ; flag page for scrolling up
  jp shopCursorControlRightCheck
;  call shopCursorControlRightCheck
;  jr shopCursorControlDone
  
.ends

.bank $2 slot 1
.org $1E90
.section "shop cursor control done" overwrite
  
  shopCursorControlDone:
    ld a,(ix+$2B)
  
.ends

; new checks for paging
.bank $2 slot 1
.org $19A1
.section "shop cursor control paging flag checks" overwrite
  
  jp checkForShopLrPaging
  
.ends

.bank $2 slot 1
.org $0FF0
.section "fortune teller update caller" overwrite
  call installFortuneTellerInputUpdate
.ends

.bank $2 slot 1
.section "fortune teller update" free
  installFortuneTellerInputUpdate:
    ld hl,fortuneTellerInputUpdate
    jp $5EB1
;    ld (iy+$00),l
;    ld (iy+$01),h
;    ld (iy+$02),$02
;    ld (iy+$2B),$00
;    ret

  fortuneTellerCursorTable:
    .db $38,$58   ; x/y (orig: 38, 58)
    .db $80,$58   ; x/y (orig: 80, 58)
    .db $38,$68   ; x/y (orig: 38, 68)
    .db $80,$68   ; x/y (orig: 80, 68)
;    .db $38,$78   ; x/y (orig: 38, 78)
;    .db $38,$80   ; x/y (orig: 80, 78)
;    .db $38,$88   ; x/y (orig: 38, 88)
;    .db $38,$90   ; x/y (orig: 80, 88)
    

  fortuneTellerInputUpdate:
    ld (ix+$03),$12
    ld hl,$9603
    call $1A40
    xor a
    ld (ix+$2E),a
    ld a,(ix+$2B)
    or a
    jr nz,+
      ld (ix+$2B),$01
    +:
    call fortuneTellerInputDone
    call $1A66
    ld a,(ix+$2E)
    or a
    jp nz,$1942
    ; check buttons pressed
    ld a,($C019)
    ld b,a
    ld a,(ix+$2B)
    ; button 1
    bit 5,b
;    jr nz,@button1
    jp nz,$5F44
    ; directions
    bit 0,b
    jr nz,@up
    bit 1,b
    jr nz,@down
    bit 2,b
    jr nz,@left
    bit 3,b
    jr nz,@right
    ret 
    
    @up:
      cp $03
      jr c,fortuneTellerInputDone
      dec a
      dec a
      ld (ix+$2B),a
      ld a,$BC
      call $04B0
      jr fortuneTellerInputDone
    
    @down:
      cp $03
      jp nc,fortuneTellerInputDone
      inc a
      inc a
      ld (ix+$2B),a
      ld a,$BC
      call $04B0
      jr fortuneTellerInputDone
    
    @left:
      bit 0,a
      jp nz,fortuneTellerInputDone
      dec (ix+$2B)
      ld a,$BC
      call $04B0
      jr fortuneTellerInputDone
    
    @right:
      bit 0,a
      jp z,fortuneTellerInputDone
      inc (ix+$2B)
      ld a,$BC
      call $04B0
      jr fortuneTellerInputDone
    
    @button1:
;      jp $5F44
;      jr fortuneTellerInputDone
      
  fortuneTellerInputDone:
    ; update cursor position
    ld a,(ix+$2B)
;    ld hl,$5E9C
    ld hl,fortuneTellerCursorTable-2
    jp $5E96
;    rst $18
;    ld (ix+$23),l
;    ld (ix+$20),h
;    ret
  
.ends

;===============================================
; Update various tiletext sizes and destinations
;===============================================

.bank $6 slot 1
.org $033B
.section "tt_party_leader_name" overwrite
  ld de,$7C12   ; dstaddr (orig: 7C58)
.ends

.bank $6 slot 1
.org $023A
.section "tt_main_menu" overwrite
  ld de,$7BCE   ; dstaddr (orig: 7BD2)
  ld bc,$0511   ; h/w (orig: 050E)
.ends

.bank $6 slot 1
.org $0863
.section "tt_item_menu_label" overwrite
  ld de,$7BDC   ; dstaddr (orig: 7BDC)
  ld bc,$0105   ; h/w (orig: 0104)
.ends

.bank $6 slot 1
.org $0872
.section "tt_item_menu_options" overwrite
  ld bc,$0110   ; h/w (orig: 0110)
.ends

  ; settings for "use [magic/item] on whom" message
  .define newUseThingDstAddr    $7BCE   ; dstaddr (orig: 7BD2)
  .define newUseThingDimensions $0410   ; h/w (orig: 040E)
  
  .bank $6 slot 1
  .org $057E
  .section "tt_magic_on_whom 1" overwrite
    ld de,newUseThingDstAddr
    ld bc,newUseThingDimensions
  .ends

  .bank $6 slot 1
  .org $0C5A
  .section "tt_magic_on_whom 2" overwrite
    ld de,newUseThingDstAddr
    ld bc,newUseThingDimensions
  .ends

  .bank $6 slot 1
  .org $0D9E
  .section "tt_magic_on_whom 3" overwrite
    ld de,newUseThingDstAddr
    ld bc,newUseThingDimensions
  .ends

.bank $6 slot 1
.org $0801
.section "tt_no_items" overwrite
  ld de,$7C0E   ; dstaddr (orig: 7C4E)
  ld bc,$0312   ; h/w (orig: 0112)
.ends

.bank $6 slot 1
.org $0C1C
.section "tt_item_cantuse" overwrite
  ld de,$7C0E   ; dstaddr (orig: 7C4E)
  ld bc,$0312   ; h/w (orig: 0112)
.ends

.bank $6 slot 1
.org $0521
.section "tt_field_not_enough_mp" overwrite
  ld de,$7C0E   ; dstaddr (orig: 7C12)
  ld bc,$0212   ; h/w (orig: 020D)
.ends

.bank $6 slot 1
.org $0446
.section "tt_field_cant_use_magic" overwrite
  ld de,$7C0E   ; dstaddr (orig: 7C0E)
  ld bc,$0312   ; h/w (orig: 0212)
.ends

.bank $6 slot 1
.org $09D6
.section "tt_item_discarded" overwrite
  ld de,$7C0E   ; dstaddr (orig: 7C4E)
  ld bc,$0312   ; h/w (orig: 0112)
.ends

  .bank $6 slot 1
  .org $0D58
  .section "tt_item_used 1" overwrite
    ld de,$7C0E   ; dstaddr (orig: 7C20)
    ld bc,$0312   ; h/w (orig: 0106)
  .ends
  
  .bank $6 slot 1
  .org $0E06
  .section "tt_item_used 2" overwrite
    ld de,$7C0E   ; dstaddr (orig: 7C20)
    ld bc,$0312   ; h/w (orig: 0106)
  .ends

.bank $6 slot 1
.org $0D12
.section "tt_spiritfeather" overwrite
  ld de,$7C0E   ; dstaddr (orig: 7C4E)
  ld bc,$0312   ; h/w (orig: 0112)
.ends

.bank $7 slot 1
.org $240A
.section "tt_battle_itemused" overwrite
  ld de,$7A8E   ; dstaddr (orig: 7A8E)
  ld bc,$0412   ; h/w (orig: 0212)
.ends

.bank $7 slot 1
.org $207A
.section "tt_battle_notenoughmp" overwrite
  ld de,$7A8E   ; dstaddr (orig: 7A8E)
  ld bc,$0412   ; h/w (orig: 0409)
.ends

.bank $7 slot 1
.org $2985
.section "tt_battle_mprecovered" overwrite
  ld de,$7ACE   ; dstaddr (orig: 7ACE)
  ld bc,$0312   ; h/w (orig: 0212)
  call $07C3
  ld hl,$7AD0   ; charname dstaddr (orig: 7B10)
.ends

  .bank $7 slot 1
  .org $27C6
  .section "tt_battle_revived 1" overwrite
    ld de,$7ACE   ; dstaddr (orig: 7ACE)
    ld bc,$0312   ; h/w (orig: 0212)
  .ends

  .bank $7 slot 1
  .org $27D6
  .section "tt_battle_revived 2" overwrite
    ld de,$7AD0 ; charname dstaddr (orig: 7B10)
  .ends

;===
; "enemy appeared"
;===

  .bank $7 slot 1
  .org $0513
  .section "tt_battle_appeared 1" overwrite
    
    ; "   appeared"
    ; this is a "ld hl,$XXXX" opcode, which is hardcoded
    ; due to pointer shenanigans
;    ld hl,$A16F
    .db $21
    
  .ends

  .bank $7 slot 1
  .org $0516
  .section "tt_battle_appeared 2" overwrite
    
    ; "   appeared"
    ; this is a "ld hl,XXXX" opcode, which is hardcoded
    ; due to pointer shenanigans
;    ld hl,$A16F
    ld de,$7ACE   ; dstaddr (orig: 7ADE)
    ld bc,$0312   ; h/w (orig: 0208)
    call $07C3
    
    ; enemy name
    ld hl,($C85D)
    ld de,$7AD4   ; dstaddr (orig: 7ACE)
    ld bc,$010C   ; h/w (orig: 0208)
    call $07C3
    
  .ends


.bank $7 slot 1
.org $05C2
.section "tt_battle_enemy_standard" overwrite
  
  ld hl,($C85D)
  ld de,$7AD4   ; dstaddr (orig: 7ACE)
  jr +
    ld hl,($C85B)
    ld de,$7AD4   ; dstaddr (orig: 7A8E)
  +:
  ld bc,$010C   ; h/w (orig: 0208)
  
.ends


.bank $7 slot 1
.org $0FC1
.section "tt_battle_enemy_attack_missed" overwrite
  
  ld de,$7ACE   ; dstaddr (orig: 7ACE)
  ld bc,$0312   ; h/w (orig: 0212)
  
.ends


.bank $7 slot 1
.org $0F9A
.section "tt_battle_enemy_sleepwave_hit_allyname" overwrite
  
  ld de,$7ACE   ; dstaddr (orig: 7ACE)
  
.ends

.bank $7 slot 1
.org $0FDD
.section "tt_battle_enemy_sleepwave_missed" overwrite
  
  ld de,$7ACE   ; dstaddr (orig: 7ACE)
  ld bc,$0312   ; h/w (orig: 0212)
  
.ends

.bank $7 slot 1
.org $178B
.section "tt_battle_ally_hitvitals" overwrite
  
  ld de,$7ACE   ; dstaddr (orig: 7B0E)
  
.ends

.bank $7 slot 1
.org $1581
.section "tt_battle_ally_autorevived" overwrite
  
  ld de,$7AD0   ; dstaddr (orig: 7B10)
  
.ends

.bank $7 slot 1
.org $17C4
.section "tt_battle_ally_halfhealth" overwrite
  
  ld de,$7AD0   ; dstaddr (orig: 7B0E)
  
.ends

.bank $7 slot 1
.org $17E7
.section "tt_battle_ally_halfmagic" overwrite
  
  ld de,$7AD0   ; dstaddr (orig: 7AD0)
  
.ends

.bank $7 slot 1
.org $31DA
.section "tt_battle_enemydefeated name" overwrite
  
  ld de,$7AD4   ; dstaddr (orig: 7A8E)
  ld bc,$010C   ; h/w (orig: 0208)
  
.ends

.bank $7 slot 1
.org $320B
.section "tt_battle_enemydefeated gems" overwrite
  
  ld de,$7B4E   ; dstaddr (orig: 7B5A)
  ld bc,$0103   ; h/w (orig: 0103)
  
.ends

.bank $7 slot 1
.org $31C9
.section "tt_battle_enemydefeated 1" overwrite
  
  ld de,$7A8E   ; dstaddr (orig: 7A8E)
  ld bc,$0312   ; h/w (orig: 0212)
  
.ends

.bank $7 slot 1
.org $31F3
.section "tt_battle_enemydefeated 2" overwrite
  
  ld hl,$B77C   ; srcaddr (orig: B76A)
  ld de,$7B4E   ; dstaddr (orig: 7B0E)
  ld bc,$0112   ; h/w (orig: 0212)
  
.ends

.bank $7 slot 1
.org $276E
.section "tt_battle_critrate_up" overwrite
  
  ld de,$7ACE   ; dstaddr (orig: 7ACE)
  ld bc,$0312   ; h/w (orig: 0212)
  
.ends

.bank $7 slot 1
.org $2651
.section "tt_battle_instakill" overwrite
  
  ld de,$7ACE   ; dstaddr (orig: 7ACE)
  ld bc,$0312   ; h/w (orig: 0212)
  
.ends

.bank $7 slot 1
.org $25E8
.section "tt_battle_ally_hprecover" overwrite
  
  ld hl,$7AD0   ; dstaddr (orig: 7B10)
  
.ends

.bank $7 slot 1
.org $1161
.section "tt_battle_ally_stronger" overwrite
  
  ld de,$7AD0   ; dstaddr (orig: 7AD0)
  
.ends

.bank $7 slot 1
.org $1117
.section "tt_battle_ally_levelup" overwrite
  
  ld de,$7ACE   ; dstaddr (orig: 7AD0)
  
.ends

.bank $7 slot 1
.org $149D
.section "tt_battle_flee_unfleeable allyname 1" overwrite
  
  ld de,$7ACE   ; dstaddr (orig: 7B0E)
  
.ends

.bank $7 slot 1
.org $2AA3
.section "tt_battle_flee_unfleeable allyname 2" overwrite
  
  ld de,$7ACE   ; dstaddr (orig: 7B0E)
  
.ends

.bank $7 slot 1
.org $204D
.section "tt_battle_flamechain_effect" overwrite
  
  ld de,$7ACE   ; dstaddr (orig: 7A8E)
  ld bc,$0312   ; h/w (orig: 0312)
  
.ends

.bank $7 slot 1
.org $0E81
.section "tt_battle_ally_vitals 1hko ally name" overwrite
  
  ld de,$7B0E   ; dstaddr (orig: 7A10)
  
.ends
