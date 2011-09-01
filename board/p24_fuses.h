#ifndef _P24_FUSES_H
#define _P24_FUSES_H

#if (defined(TARGET_EKOBB_R1) || defined (TARGET_EKOBB_R2) || defined (TARGET_EKOBB_R3))

#include "p24f16ka102_fuses.h"

#elif (defined (TARGET_P24DEVKIT_SURE))

#include "p24fj256gb106_fuses.h"

#endif

#endif
