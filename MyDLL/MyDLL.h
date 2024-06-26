// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the MYDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// MYDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MYDLL_EXPORTS
#define MYDLL_API __declspec(dllexport)
#else
#define MYDLL_API __declspec(dllimport)
#endif

// This class is exported from the dll
class MYDLL_API CMyDLL {
public:
	CMyDLL(void);
	// TODO: add your methods here.
};

extern MYDLL_API int nMyDLL;

MYDLL_API int fnMyDLL(void);

MYDLL_API float mult(float a, float b);

MYDLL_API float add(float a, float b);
