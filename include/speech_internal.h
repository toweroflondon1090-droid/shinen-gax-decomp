// defines //

// https://github.com/timothytylee/libgsm/blob/master/inc/private.h

#define MIN_WORD -32768
#define MAX_WORD 32767

#define GSM_ADD(a, b) ((a) + (b))
#define GSM_SUB(a, b)    ((a) - (b))
#define GSM_MULT_R(a, b) ((a) * (b) + 0x4000) >> 0xF


// structs //

typedef struct GAX_unk0 {
    // speech related
    u8 unk0[594];
    u16 unk252;
    u8 unk254[732];
    u8 unk530;
    u8 unk531;
    u8 unk532[338];
    u8 unk684;
    u8 unk685;
    u8 unk686[178];
    u8 unk738;
    u8 unk739;
    u8 unk73A[78];
    void* unk788;
    void* unk78C;
    void* unk790;
} GAX_unk0;


typedef struct speech_state speech_state;
typedef void (*speech_filter_fn)(struct speech_state*, s16*, int, s16*, s8*);

struct speech_state {
    s16 dp0[280];
    s16 LARpp[2][8];
    s16 j;
    u8 pad[0x788 - 0x252];
    speech_filter_fn filter;
};

typedef struct GAXSpeech_unk {
    u32      unk00;
    GAX_unk0 unk04;
    u32      unk798;
    u32      unk79C;
    u32      unk7A0;
} GAXSpeech_unk;

void GAXSpeech_internal0(s16* LARc, s16* LARpp);
void GAXSpeech_internal1(s16* LARp);
void GAXSpeech_internal3(struct speech_state* S, s16* LARc, s16* s, s8* d);
void GAXSpeech_internal5(s16* xMc, s16 mant, s16 exp, s16* xMp);
void GAXSpeech_internal6(struct speech_state *S, s16 xmaxcr, s16 Mcr, s16* xMcr, s16* erp);
void GAXSpeech_mix();
s16 speech_asl(s16 a, s32 n);
void speech_create(GAX_unk0* speech_unk);
void speech_decode();
void STSF();
void SLTSF(GAX_unk0* S, s16 Ncr, u16 bcr, s16* erp, s16* drp);
