#include "gax.h"
#include "speech_internal.h"


// macro functions

#undef  STEP
#define STEP( B_TIMES_TWO, MIC, INVA )  \
    temp1    = GSM_ADD( *LARc++, MIC ) << 10;   \
    temp1    = GSM_SUB( temp1, B_TIMES_TWO );   \
    temp1    = GSM_MULT_R( INVA, temp1 );       \
    *LARpp++ = GSM_ADD( temp1, temp1 );

#define saturate(x) ((x) < MIN_WORD ? MIN_WORD : (x) > MAX_WORD ? MAX_WORD: (x))
    

// GSM 6.10 tables
// sourced from libgsm, https://quut.com/gsm/

const s16 speech_FAC[8] = {

    // normalized direct mantissa used to compute xM/xmax
    18431, 20479, 22527, 24575,
    26623, 28671, 30719, 32767

};

const s16 speech_QLB[4] = {

    // quantization levels of LTP gain quantizer 
    3277, 11469, 21299, 32767

};


// inlined functions

static inline s16 speech_sub(s16 a, s16 b) {
    
    // implementation of gsm_sub
    // https://github.com/timothytylee/libgsm/src/add.c#L29
    
    s32 diff = (s32)a - (s32)b;
    return saturate(diff);
    
}

static inline s16 speech_asr(s16 a, s32 n) {
    
    // implementation of gsm_asr
    // https://github.com/timothytylee/libgsm/src/add.c#L183
    
    if (n >= 16)  return -(a < 0);
    if (n <= -16) return 0;
    if (n < 0)    return a << -n;
    
    return a >> n;

}


// functions

// void GAXSpeech_internal0
// https://decomp.me/scratch/ZjU9Y - beanieaxolotl, christianttt
// accuracy -> 100%

void GAXSpeech_internal0(s16* LARc, s16* LARpp) {

    // implementation of Decoding_of_the_coded_Log_Area_Ratios
    // https://github.com/timothytylee/libgsm/blob/master/src/short_term.c#L23
    
    s16 temp1;
    s32 ltmp;

	STEP(      0,  -32,  13107 );
	STEP(      0,  -32,  13107 );
	STEP(   4096,  -16,  13107 );
	STEP(  -5120,  -16,  13107 );

	STEP(    188,   -8,  19223 );
	STEP(  -3584,   -8,  17476 );
	STEP(   -682,   -4,  31454 );
	STEP(  -2288,   -4,  29708 );

}

// void GAXSpeech_internal1
// https://decomp.me/scratch/kUbtb - beanieaxolotl, christianttt
// accuracy -> 100%

void GAXSpeech_internal1(s16* LARp) {

    // implementation of LARp_to_rp
    // https://github.com/timothytylee/libgsm/blob/master/src/short_term.c#L144
	
    int i;
    s16 temp;

    for (i = 1; i <= 8; i++, LARp++) {
        
        if (*LARp < 0) {
			
            temp = (*LARp == MIN_WORD) ? MAX_WORD : -(*LARp);

            *LARp = -((temp < 11059) ? (temp << 1)
                     : (temp < 20070) ? (temp + 11059)
                     : GSM_ADD(temp >> 2, 26112));
        } else {
            temp = *LARp;
            
            *LARp = ((temp < 11059) ? (temp << 1)
                     : (temp < 20070) ? (temp + 11059)
                     : GSM_ADD(temp >> 2, 26112));
        }

    }

}

void GAXSpeech_internal3() {
	
}

// void GAXSpeech_internal5
// https://decomp.me/scratch/0j2jj - beanieaxolotl
// accuracy -> 100%

void GAXSpeech_internal5(s16* xMc, s16 mant, s16 exp, s16* xMp) {

    // implementation of APCM_inverse_quantization
    // https://github.com/timothytylee/libgsm/src/rpe.c#L349
    
    int i;
    s16 temp, temp1, temp2, temp3;
    s32 ltmp;

    temp1 = speech_FAC[mant];
    temp2 = speech_sub(6, exp);
    temp3 = speech_asl(1, speech_sub(temp2, 1));

    for (i = 13; i--;) {

        temp = (*xMc++ << 1) - 7;
        temp <<= 12;
        
        temp   = GSM_MULT_R(temp1, temp);
        temp   = GSM_ADD(temp, temp3);
        *xMp++ = speech_asr(temp, temp2);
        
    }
    
}

// void GAXSpeech_internal6
// https://decomp.me/scratch/ncC8W - Warm Mosquito (anon)
// accuracy -> 97.62%

static inline void APCM_quantization_xmaxc_to_exp_mant(s16 xmaxc,
                                                       s16* exp_out,
                                                       s16* mant_out) {
    s16 exp = 0;
    s16 mant;

    if (xmaxc > 15) exp = SASR(xmaxc, 3) - 1;
    mant = xmaxc - (exp << 3);

    if (mant == 0) {
        exp  = -4;
        mant = 7;
    } else {
        while (mant <= 7) {
            mant = (mant << 1) | 1;
            exp--;
        }
        mant -= 8;
    }

    *exp_out  = exp;
    *mant_out = mant;
}

static inline void RPE_grid_positioning(s16 Mc,
                                        register s16* xMp,
                                        register s16* ep) {
    int i = 13;
    
    switch (Mc) {
        case 3: *ep++ = 0;
        case 2: do {
                        *ep++ = 0;
        case 1:         *ep++ = 0;
        case 0:         *ep++ = *xMp++;
                } while (--i);
    } 
    while (++Mc < 4) *ep++ = 0;
}

void GAXSpeech_internal6(struct speech_state *S,
                         s16 xmaxcr,
                         s16 Mcr,
                         s16* xMcr,
                         s16* erp) {
    s16 exp, mant;

    APCM_quantization_xmaxc_to_exp_mant(xmaxcr, &exp, &mant);
    GAXSpeech_internal5(xMcr, mant, exp, xMcr);
    RPE_grid_positioning(Mcr, xMcr, erp);
}

void GAXSpeech_mix() {
	
}

// void GAXSpeech_open
// https://decomp.me/scratch/B3lkx - beanieaxolotl
// accuracy -> 100%

void GAXSpeech_open(GAXSpeech_unk* param_1) {
    
    speech_create(&param_1->unk04);
    
    param_1->unk798 = -1;
    param_1->unk79C =  0;
    param_1->unk7A0 =  0;
    
}

void GAXSpeech_render() {
	
}


// void SLTSF (Short/Long Term Synthesis Filtering)
// accuracy -> 56.83% (down from 35.99%)

void SLTSF(GAX_unk0* S, s16 Ncr, u16 bcr, s16* erp, s16* drp) {
    s16 Nr;
    s16 brp;
    s16 *drp_nr;
    u32 *dst, *src;
    int k;

    if ((u16)(Ncr - 40) > 80)
        Nr = S->unk252;
    else
        Nr = Ncr;
    S->unk252 = Nr;

    brp = speech_QLB[bcr];

    dst = (u32*)((u8*)drp - 0xf0);
    src = (u32*)((u8*)drp - 0xa0);
    drp_nr = drp - Nr;

    for (k = 39; k >= 0; k--) {
        s32 drpp = GSM_MULT_R(brp, *drp_nr++);
        *drp++ = *erp++ + drpp;
    }

    for (k = 14; k >= 0; k--) {
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
    }
}

// s16 speech_asl
// https://decomp.me/scratch/bS89k - beanieaxolotl
// accuracy -> 100%

s16 speech_asl(s16 a, s32 n) {

    // implementation of gsm_asl
    // https://github.com/timothytylee/libgsm/src/add.c#L161
    
    if (n >= 16)  return 0;
    if (n <= -16) return -(a < 0);
    if (n < 0)    return speech_asr(a, -n);
    
    return a << n;
    
}

// void speech_create
// https://decomp.me/scratch/qne2u - beanieaxolotl
// accuracy -> 100%

void speech_create(GAX_unk0* speech_unk) {
    
    if (speech_unk != NULL) {
        
        GAX_clear_mem((u32)speech_unk, 0x268); // clear / prepare memory
        speech_unk->unk252 = 40;
        
        CpuSet(STSF, 
               &speech_unk->unk530, 
               REG_BASE|0x55);      // REG_BLDY
        CpuSet(SLTSF, 
               &speech_unk->unk684, 
               REG_BASE|0x2D);      // REG_BG2Y
        CpuSet(GAXSpeech_mix, 
               &speech_unk->unk738, 
               (u32)&REG_BG1HOFS);

        speech_unk->unk788 = &speech_unk->unk531;
        speech_unk->unk78C = &speech_unk->unk685;
        speech_unk->unk790 = &speech_unk->unk739;
        
    }
    
}

void speech_decode() {
	
}

void STSF() {
	
}
