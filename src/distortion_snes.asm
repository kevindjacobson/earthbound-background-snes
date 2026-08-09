.include "hdr.asm"

.ramsection ".earthbound_distortion_work" bank $7e slot 2
eb_frequency dw
eb_compression dw
eb_phase dw
eb_vertical dw
eb_base_scroll dw
eb_effect_type dw
.ends

.section ".earthbound_distortion_text" superfree

.accu 16
.index 16
.16bit

; void eb_distortion_build_snes(output, type, frequency, amplitude,
;                               compression, phase, base_scroll)
eb_distortion_build_snes:
    php
    phb
    phx
    phy

    sep #$20
    lda #$7e
    pha
    plb
    rep #$30

    lda 12,s
    sta eb_effect_type
    lda 14,s
    sta eb_frequency
    lda 18,s
    sta eb_compression
    lda 20,s
    and #$00ff
    xba
    sta eb_phase
    lda 22,s
    sta eb_base_scroll
    and #$00ff
    xba
    sta eb_vertical

    lda 16,s
    sep #$20
    sta.l $00211b
    lda #0
    sta.l $00211b
    rep #$20

    ldy #0
_eb_line_loop:
    lda eb_phase
    xba
    and #$00ff
    tax
    sep #$20
    lda.l eb_sine_table,x
    sta.l $00211c
    rep #$20

    lda eb_effect_type
    cmp #3
    beq _eb_vertical
    cmp #2
    bne _eb_horizontal_add
    tya
    and #2
    beq _eb_horizontal_add
    lda eb_base_scroll
    sec
    sbc.l $002135
    bra _eb_store

_eb_horizontal_add:
    lda.l $002135
    clc
    adc eb_base_scroll
    bra _eb_store

_eb_vertical:
    lda eb_vertical
    clc
    adc eb_compression
    sta eb_vertical
    xba
    and #$00ff
    clc
    adc.l $002135

_eb_store:
    sta (10,s),y
    lda eb_phase
    clc
    adc eb_frequency
    sta eb_phase
    iny
    iny
    cpy #448
    bcc _eb_line_loop

    ply
    plx
    plb
    plp
    rtl

eb_sine_table:
    .db $00,$03,$06,$09,$0C,$0F,$12,$15,$18,$1C,$1F,$22,$25,$28,$2B,$2E
    .db $30,$33,$36,$39,$3C,$3F,$41,$44,$47,$49,$4C,$4E,$51,$53,$55,$58
    .db $5A,$5C,$5E,$60,$62,$64,$66,$68,$6A,$6C,$6D,$6F,$70,$72,$73,$75
    .db $76,$77,$78,$79,$7A,$7B,$7C,$7C,$7D,$7E,$7E,$7F,$7F,$7F,$7F,$7F
    .db $7F,$7F,$7F,$7F,$7F,$7F,$7E,$7E,$7D,$7C,$7C,$7B,$7A,$79,$78,$77
    .db $76,$75,$73,$72,$70,$6F,$6D,$6C,$6A,$68,$66,$64,$62,$60,$5E,$5C
    .db $5A,$58,$55,$53,$51,$4E,$4C,$49,$47,$44,$41,$3F,$3C,$39,$36,$33
    .db $30,$2E,$2B,$28,$25,$22,$1F,$1C,$18,$15,$12,$0F,$0C,$09,$06,$03
    .db $00,$FD,$FA,$F7,$F4,$F1,$EE,$EB,$E8,$E4,$E1,$DE,$DB,$D8,$D5,$D2
    .db $D0,$CD,$CA,$C7,$C4,$C1,$BF,$BC,$B9,$B7,$B4,$B2,$AF,$AD,$AB,$A8
    .db $A6,$A4,$A2,$A0,$9E,$9C,$9A,$98,$96,$94,$93,$91,$90,$8E,$8D,$8B
    .db $8A,$89,$88,$87,$86,$85,$84,$84,$83,$82,$82,$81,$81,$81,$81,$81
    .db $81,$81,$81,$81,$81,$81,$82,$82,$83,$84,$84,$85,$86,$87,$88,$89
    .db $8A,$8B,$8D,$8E,$90,$91,$93,$94,$96,$98,$9A,$9C,$9E,$A0,$A2,$A4
    .db $A6,$A8,$AB,$AD,$AF,$B2,$B4,$B7,$B9,$BC,$BF,$C1,$C4,$C7,$CA,$CD
    .db $D0,$D2,$D5,$D8,$DB,$DE,$E1,$E4,$E8,$EB,$EE,$F1,$F4,$F7,$FA,$FD

.ends
