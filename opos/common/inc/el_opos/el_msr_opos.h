

#if !defined(_EL_MSR_OPOS_H_)
#define      _EL_MSR_OPOS_H_

/////////////////////////////////////////////////////////////////////
// "CapDataEncryption", "DataEncryptionAlgorithm" Property Constants
//   (added in 1.12)
/////////////////////////////////////////////////////////////////////

// const LONG MSR_DE_NONE          = 0x00000001; //defined OposMsr.h
// const LONG MSR_DE_3DEA_DUKPT    = 0x00000002; //defined OposMsr.h
// Note: Service-specific values begin at 0x01000000.

const LONG EL_MSR_DE_AES_DUKPT    = 0x01000000;

#endif	//_EL_MSR_OPOS_H_