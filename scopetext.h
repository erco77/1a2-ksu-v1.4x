// scopetext.h -- display text messages on your oscilloscope using a single analog out.
//
//    >>
//    >> See README-scopetext.txt for info on how to use this code in your application.
//    >>
//

#ifndef SCOPETEXT_H
#define SCOPETEXT_H

// User definable values.
//     Change these three values as needed for your PIC chip + scope timing requirements.
//
#define SCOPETEXT_OUTPUT    DAC1CON1bits.DAC1R   // analog output port to write to
#define SCOPETEXT_YOFFSET   50                   // how high above gnd the text appears
#define SCOPETEXT_YSCALE    10                   // font height scale

// Constants
#define SCOPETEXT_HEIGHT    5                    // height of font in bit rows
#define SCOPETEXT_WIDTH     8                    // width of font in bit columns

// Struct for a single font character
typedef struct {
    char c;                                      // the character being represented
    unsigned char glyph[SCOPETEXT_HEIGHT];       // the character's glyph
} SCOPETEXT_FontChar;

// [PRIVATE] Individual chars in font
//     A quick, memory + speed efficient font.
//     You can add other chars, just keep #rows consistent.
//
static SCOPETEXT_FontChar G_font[] = {
    { ' ',
      {
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000
      },
    },{ '0',
      {
        0b11111110,
        0b11000110,
        0b11000110,
        0b11000110,
        0b11111110
      },
    },{ '1',
      {
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000,
        0b00110000
      },
    },{ '2',
      {
        0b11111110,
        0b00000110,
        0b11111110,
        0b11000000,
        0b11111110
      },
    },{ '3',
      {
        0b01111110,
        0b00000110,
        0b01111110,
        0b00000110,
        0b01111110
      },
    },{ '4',
      {
        0b11000110,
        0b11000110,
        0b11111110,
        0b00000110,
        0b00000110
      },
    },{ '5',
      {
        0b11111110,
        0b11000000,
        0b11111110,
        0b00000110,
        0b11111110
      },
    },{ '6',
      {
        0b11000000,
        0b11000000,
        0b11111110,
        0b11000110,
        0b11111110
      },
    },{ '7',
      {
        0b11111110,
        0b00000110,
        0b00001100,
        0b00011000,
        0b00011000
      },
    },{ '8',
      {
        0b11111110,
        0b11000110,
        0b11111110,
        0b11000110,
        0b11111110
      },
    },{ '9',
      {
        0b11111110,
        0b11000110,
        0b11111110,
        0b00000110,
        0b00000110
      },
    },{ 'a',
      {
        0b00111000,
        0b11000110,
        0b11111110,
        0b11000110,
        0b11000110
      },
    },{ 'b',
      {
        0b11111100,
        0b01100110,
        0b01111100,
        0b01100110,
        0b11111100
      },
    },{ 'c',
      {
        0b00111110,
        0b11000000,
        0b11000000,
        0b11000000,
        0b00111110
      },
    },{ 'd',
      {
        0b11111000,
        0b01100110,
        0b01100110,
        0b01100110,
        0b11111000
      },
    },{ 'e',
      {
        0b11111100,
        0b11000000,
        0b11111100,
        0b11000000,
        0b11111100
      },
    },{ 'f',
      {
        0b11111100,
        0b11000000,
        0b11111000,
        0b11000000,
        0b11000000
      },
    },{ '.',
      {
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00110000
      },
    },{ '-',
      {
        0b00000000,
        0b00000000,
        0b01111100,
        0b00000000,
        0b00000000
      },
    },{ '+',
      {
        0b00000000,
        0b00011000,
        0b01111110,
        0b00011000,
        0b00000000
      },
    },{ ':',
      {
        0b00000000,
        0b00110000,
        0b00000000,
        0b00110000,
        0b00000000
      },
    },{ '=',
      {
        0b00000000,
        0b01111100,
        0b00000000,
        0b01111100,
        0b00000000
      },
    },{ ',',
      {
        0b00000000,
        0b00000000,
        0b00000000,
        0b00111000,
        0b01100000
      },
    },{
      // 2nd last char is "error" character: all bits set
      0xff,
      {
        0b11111111,
        0b11111111,
        0b11111111,
        0b11111111,
        0b11111111,
      },
    },{
       // Last char must be all zeroes
       0,
       {
          0,0,0,0,0
       },
    }
};

// [PRIVATE] Return the glyph index for the font character 'c'.
int SCOPETEXT_FontForChar(unsigned char c) {
    int i;
    for ( i=0; G_font[i].c; i++ )
        if ( G_font[i].c == c )
            return i;
    return i-1;     // error char is 2nd from last
}

// Show string on scope.
//    String can only contain chars defined in G_font[], e.g.:
//    "0123456789abcdef+-.=:, "
//    See scopetext-examples/*.c for example use.
//
void SCOPETEXT_Print(char *s) {
    const unsigned char voff = 0;   // voltage value if font bit is off
    unsigned char       von  = 0;   // voltage value if font bit is ON
    unsigned char onoff;
    while ( *s ) {
        int i = SCOPETEXT_FontForChar(*s++);
        for ( int x=0; x<SCOPETEXT_WIDTH; x++ ) {
            for ( int y=0; y<SCOPETEXT_HEIGHT; y++ ) {
                onoff = (G_font[i].glyph[y] << x) & 0x80;
                von = SCOPETEXT_YOFFSET + ((SCOPETEXT_HEIGHT-y)*SCOPETEXT_YSCALE);
                SCOPETEXT_OUTPUT = onoff ? von : voff;
                //__delay_us(...);           // add optional delay for wider pixels
            }
        }
        SCOPETEXT_OUTPUT = voff;             // turn off pixels between chars
    }
}

// Convert byte 'val' to 3 digit zero padded decimal string in 's'.
//    Returns pointer to null so caller can continue to append to 's'.
//    For example use, see scopetext-examples/dec-convert.c
//
char* SCOPETEXT_AsDec(unsigned char val, char *s) {
    *s++ = '0' + (val/100);      // hundreds digit
    *s++ = '0' + ((val%100)/10); // tens digit
    *s++ = '0' + (val%10);       // ones digit
    *s   = 0;                    // null terminate string
    return s;                    // return ptr to null
}

// [PRIVATE] Return single ASCII hex digit for 'val'.
//     Assumes 0 <= val <= 0x0f.
//     Returns e.g. '0'-'9','a'-'f'.
//
char SCOPETEXT_HexDigit(unsigned char val) {
    return ( val < 0x0a ) ? ('0' + val) : ('a' + (val - 0x0a));
}

// Return 2 digit zero padded hex string for byte 'val' in string 's'.
//    For example use, see scopetext-examples/hex-convert.c
//
char* SCOPETEXT_AsHex(unsigned char val, char *s) {
    *s++ = SCOPETEXT_HexDigit(val >> 4);      // MSN
    *s++ = SCOPETEXT_HexDigit(val & 0x0f);    // LSN
    *s   = 0;                                 // null terminate string
    return s;
}

#endif // SCOPETEXT_H
