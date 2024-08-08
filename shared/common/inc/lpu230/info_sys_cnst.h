#ifndef _INFO_SYS_CNST_SET_201012290001H_
#define _INFO_SYS_CNST_SET_201012290001H_

/*
#ifndef	offsetof
#define offsetof(s, m)   (UINT32)&(((s *)0)->m)
#endif
*/
//the size of structure member
//s - structure name, m - member name
#define	sizeofstructmember(s,m)				sizeof(((s *)0 )->m)

#endif	//_INFO_SYS_CNST_SET_201012290001H_
