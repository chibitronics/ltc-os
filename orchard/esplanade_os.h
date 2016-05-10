#ifndef __ESPLANADE_OS__
#define __ESPLANADE_OS__

#define export_symbol(def) __attribute__((section(".osdata"))) def
#define export_symbol_bss(def) __attribute__((section(".osbss"))) def

#endif /* __ESPLANADE_OS__ */
