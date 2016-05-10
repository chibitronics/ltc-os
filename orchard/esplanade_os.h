#ifndef __ESPLANADE_OS__
#define __ESPLANADE_OS__

#define bl_symbol(def) __attribute__((section(".bldata"))) def
#define bl_symbol_bss(def) __attribute__((section(".blbss"))) def

#endif /* __ESPLANADE_OS__ */
