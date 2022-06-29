#pragma once

// warning C4172: returning address of local variable or temporary
// potential pointer dangling? seems fine for now.
#pragma warning( disable : 4172 )  

// generate 'pseudo-random' xor key based on file, date and line.
#define GET_XOR_KEYUI8  ( ( CONST_HASH( __FILE__ __TIMESTAMP__ ) + __LINE__ ) % UINT8_MAX )
#define GET_XOR_KEYUI16 ( ( CONST_HASH( __FILE__ __TIMESTAMP__ ) + __LINE__ ) % UINT16_MAX )
#define GET_XOR_KEYUI32 ( ( CONST_HASH( __FILE__ __TIMESTAMP__ ) + __LINE__ ) % UINT32_MAX )



// todo - dex;  the issue with this code is that the xor key will be static since it's in the actual function call
//              the macro needs to pass a key instead using __COUNTER__ or __LINE__ as the first template arg, the rest will be handled by the compiler (like size).
//              sadly, this increasess compile time by a fuck ton.




#define XOR( s ) ( s )
