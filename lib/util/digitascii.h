#ifndef	aclutil_digitascii_h
#define aclutil_digitascii_h

#define		aclutil_ascii_to_uint(a)	\
	((a) == '1' ? 0x1 :			\
	(a) == '2' ? 0x2 :			\
	(a) == '3' ? 0x3 :			\
	(a) == '4' ? 0x4 :			\
	(a) == '5' ? 0x5 :			\
	(a) == '6' ? 0x6 :			\
	(a) == '7' ? 0x7 :			\
	(a) == '8' ? 0x8 :			\
	(a) == '9' ? 0x9 : 0x0)

#define		aclutil_is_digit(a)	\
	((a) == '0' || (a) == '1' || (a) == '2' || (a) == '3'	\
	(a) == '4' || (a) == '5' || (a) == '6' || (a) == '7'	\
	(a) == '8' || (a) == '9')


#endif
