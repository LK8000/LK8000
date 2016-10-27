/*
 * Holux interface to Windows CE by Paolo
 */
#if defined(PNA) && defined(UNDER_CE)

extern bool DeviceIsGM130;

extern bool Init_GM130(void);
extern void DeInit_GM130(void);

extern int  GM130BarAltitude(void);
extern float  GM130BarPressure(void);
extern int  GM130PowerLevel(void);
extern int  GM130PowerStatus(void);
extern int  GM130PowerFlag(void);
extern void GM130MaxBacklight(void);

extern void GM130MaxSoundVolume(void);

#if 0 // unused
extern void GM130PlaySound(int level, int duration, int count, int interval);
#endif
#if GM130TEMPERATURE
extern int  GM130GetTemperature(void);
#endif

#endif // PNA only
