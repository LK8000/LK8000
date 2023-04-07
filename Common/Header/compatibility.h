#ifndef __COMPATIBILITY_H_
#define __COMPATIBILITY_H_


#if defined(WIN32_PLATFORM_HPC2000)

#elif defined(WIN32_PLATFORM_HPCPRO)

#elif defined(WIN32_PLATFORM_PSPC)
// Pocket PC

	#if (_WIN32_WCE == 300)
	// Pocket PC 2000
                #define OLDPPC

		// App keys
		#define VK_APP1     0xC1
		#define VK_APP2     0xC2
		#define VK_APP3     0xC3
		#define VK_APP4     0xC4
		#define VK_APP5     0xC5
		#define VK_APP6     0xC6
		// #define VK_APP7     0xC7
		// #define VK_APP8     0xC8

		#ifndef NOCLEARTYPE
		#define NOCLEARTYPE
		#endif

		#define NOTIME_H

	#elif (WIN32_PLATFORM_PSPC == 310)
	// Pocket PC 2002

		#ifndef NOCLEARTYPE
		#define NOCLEARTYPE
		#endif

	#elif (WIN32_PLATFORM_PSPC == 400)
	// Pocket PC 2003
#include <assert.h>
	// 110106 apply cleartype on CE 5 as well
	#ifndef NOCLEARTYPE
	#define NOCLEARTYPE
	#endif


	#else
	// Some other Pocket PC
#include <assert.h>

	// NOT EXPLICITLY SUPPORTED
	// 110106 apply cleartype on CE 5 as well
	#ifndef NOCLEARTYPE
	#define NOCLEARTYPE
	#endif

	#endif

#elif defined(WIN32_PLATFORM_WFSP)
// Smartphone
// NOT SUPPORTED
#else
// etc.
#endif


#endif // __COMPATIBILITY_H_
